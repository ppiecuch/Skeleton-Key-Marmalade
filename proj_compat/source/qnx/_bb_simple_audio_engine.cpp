/****************************************************************************
Copyright (c) 2010 cocos2d-x.org

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 ****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include <mm/renderer.h>
#include <bps/soundplayer.h>
#include <vorbis/vorbisfile.h>

#include <map>
#include <string>

#include "_bb_simple_audio_engine.h"

namespace audio
{
struct soundData {
	ALuint buffer;
	ALuint source;
	bool   isLooped;
};

inline const char *resourcePath () {
  return "/app/native/data/";
}

typedef std::map<std::string, soundData *> EffectsMap;
EffectsMap s_effects;

typedef enum {
	PLAYING,
	STOPPED,
	PAUSED,
} playStatus;

static int	 s_audioOid;

static float s_volume                             = 1.0f;
static float s_effectVolume                       = 1.0f;
static bool s_isBackgroundInitialized             = false;
static bool s_hasMMRError                         = false;
static playStatus s_playStatus                    = STOPPED;

static std::string s_currentBackgroundStr;
static mmr_connection_t *s_mmrConnection          = 0;
static mmr_context_t *s_mmrContext                = 0;
static strm_dict_t *s_repeatDictionary            = 0;
static strm_dict_t *s_volumeDictionary            = 0;

static SimpleAudioEngine  *s_engine = 0;

static int checkALError(const char *funcName)
{
  int err = alGetError();
    
  if (err != AL_NO_ERROR)
  {
    switch (err)
    {
    case AL_INVALID_NAME:
      fprintf(stderr, "AL_INVALID_NAME in %s\n", funcName);
      break;
	    
    case AL_INVALID_ENUM:
      fprintf(stderr, "AL_INVALID_ENUM in %s\n", funcName);
      break;
	    
    case AL_INVALID_VALUE:
      fprintf(stderr, "AL_INVALID_VALUE in %s\n", funcName);
      break;
	    
    case AL_INVALID_OPERATION:
      fprintf(stderr, "AL_INVALID_OPERATION in %s\n", funcName);
      break;
	    
    case AL_OUT_OF_MEMORY:
      fprintf(stderr, "AL_OUT_OF_MEMORY in %s\n", funcName);
      break;
    }
  }
  
  return err;
}

static void printALError(int err)
{
	switch (err)
	{
	case AL_NO_ERROR:
		fprintf(stderr, "AL_NO_ERROR");
		break;

	case AL_INVALID_NAME:
		fprintf(stderr, "AL_INVALID_NAME");
		break;

	case AL_INVALID_ENUM:
		fprintf(stderr, "AL_INVALID_ENUM");
		break;

	case AL_INVALID_VALUE:
		fprintf(stderr, "AL_INVALID_VALUE");
		break;

	case AL_INVALID_OPERATION:
		fprintf(stderr, "AL_INVALID_OPERATION");
		break;

	case AL_OUT_OF_MEMORY:
		fprintf(stderr, "AL_OUT_OF_MEMORY");
		break;
	};
}

static void mmrerror(mmr_context_t *ctxt, const char *msg)
{
	const mmr_error_info_t  *err = mmr_error_info( ctxt );
	unsigned 				 errcode = (err) ? err->error_code : -1;
	const char 				*name;

	fprintf(stderr, "%s: error %d \n", msg, errcode);
	s_hasMMRError = true;
}

//
// OGG support
//
static bool isOGGFile(const char *pszFilePath)
{
  FILE *file;
  OggVorbis_File ogg_file;
  int result;

  file = fopen(pszFilePath, "rb");
  result = ov_test(file, &ogg_file, 0, 0);
  ov_clear(&ogg_file);

  return (result == 0);
}

static ALuint createBufferFromOGG(const char *pszFilePath)
{
  ALuint buffer;
  OggVorbis_File ogg_file;
  vorbis_info* info;
  ALenum format;
  int result;
  int section;
  unsigned int size = 0;

  if (ov_fopen(pszFilePath, &ogg_file) < 0)
    {
      ov_clear(&ogg_file);
      fprintf(stderr, "Could not open OGG file %s\n", pszFilePath);
      return -1;
    }

  info = ov_info(&ogg_file, -1);

  if (info->channels == 1)
    format = AL_FORMAT_MONO16;
  else
    format = AL_FORMAT_STEREO16;

  // size = #samples * #channels * 2 (for 16 bit)
  unsigned int data_size = ov_pcm_total(&ogg_file, -1) * info->channels * 2;
  char* data = new char[data_size];

  while (size < data_size)
    {
      result = ov_read(&ogg_file, data + size, data_size - size, 0, 2, 1, &section);
      if (result > 0)
	{
	  size += result;
	}
      else if (result < 0)
	{
	  delete [] data;
	  fprintf(stderr, "OGG file problem %s\n", pszFilePath);
	  return -1;
	}
      else
	{
	  break;
	}
    }

  if (size == 0)
    {
      delete [] data;
      fprintf(stderr, "Unable to read OGG data\n");
      return -1;
    }

  // clear al errors
  checkALError("createBufferFromOGG");

  // Load audio data into a buffer.
  alGenBuffers(1, &buffer);

  if (checkALError("createBufferFromOGG") != AL_NO_ERROR)
    {
      fprintf(stderr, "Couldn't generate a buffer for OGG file\n");
      delete [] data;
      return buffer;
    }

  alBufferData(buffer, format, data, data_size, info->rate);
  checkALError("createBufferFromOGG");
  
  delete [] data;
  ov_clear(&ogg_file);

  return buffer;
}

static void stopBackground(bool bReleaseData)
{
	s_playStatus = STOPPED;

	if (s_mmrContext)
		mmr_stop(s_mmrContext);

	if (bReleaseData)
	{
		if (s_mmrContext)
		{
			mmr_input_detach(s_mmrContext);
			mmr_context_destroy(s_mmrContext);
		}

		if (s_mmrConnection)
			mmr_disconnect(s_mmrConnection);

		if (s_repeatDictionary)
			strm_dict_destroy(s_repeatDictionary);

		if (s_volumeDictionary)
			strm_dict_destroy(s_volumeDictionary);

		s_mmrContext = 0;
		s_mmrConnection = 0;
		s_repeatDictionary = 0;
		s_volumeDictionary = 0;
		s_hasMMRError = false;
		s_currentBackgroundStr = "";
		s_isBackgroundInitialized = false;
	}
}

static void setBackgroundVolume(float volume)
{
	char volume_str[128];

	// set it up the background volume
	strm_dict_t *dictionary = strm_dict_new();

	sprintf(volume_str, "%d", (int)(volume * 100) );
	s_volumeDictionary = strm_dict_set(dictionary, "volume", volume_str);

	if (mmr_output_parameters(s_mmrContext, s_audioOid, s_volumeDictionary) != 0)
	{
		mmrerror(s_mmrContext, "output parameters");
		return;
	}
}

SimpleAudioEngine::SimpleAudioEngine()
{
	alutInit(0, 0);
}

SimpleAudioEngine::~SimpleAudioEngine()
{
	alutExit();
}

SimpleAudioEngine* SimpleAudioEngine::sharedEngine()
{
	if (!s_engine)
		s_engine = new SimpleAudioEngine();

	return s_engine;
}

void SimpleAudioEngine::end()
{
	// clear all the sounds
	EffectsMap::const_iterator end = s_effects.end();
	for (EffectsMap::iterator it = s_effects.begin(); it != end; it++)
	{
		alSourceStop(it->second->source);

		alDeleteBuffers(1, &it->second->buffer);
		alDeleteSources(1, &it->second->source);
		delete it->second;
	}
	s_effects.clear();

	if (s_isBackgroundInitialized)
	{
		s_isBackgroundInitialized = false;
	}

	// and the background too
	stopBackground(true);
}

void SimpleAudioEngine::setResource(const char* pszZipFileName)
{
}

//
// background audio (using mmrenderer)
//
void SimpleAudioEngine::preloadBackgroundMusic(const char* pszFilePath, char* pszKey)
{
	if (!s_isBackgroundInitialized)
	{
		const char 		*mmrname = NULL;
		const char 		*ctxtname = "mmrplayer";
		char 			 cwd[PATH_MAX];
		mode_t 			 mode = S_IRUSR | S_IXUSR;

		if (!pszKey)
			pszKey = const_cast<char*>(pszFilePath);

		getcwd(cwd, PATH_MAX);

		s_mmrConnection = mmr_connect(mmrname);
		if (!s_mmrConnection)
		{
			perror("mmr_connect");
			s_hasMMRError = true;
			return;
		}

		s_mmrContext = mmr_context_create(s_mmrConnection, ctxtname, 0, mode);
		if (!s_mmrContext)
		{
			perror(ctxtname);
			s_hasMMRError = true;
			return;
		}

		if ((s_audioOid = mmr_output_attach(s_mmrContext, "audio:default", "audio")) < 0)
		{
			mmrerror(s_mmrContext, "audio:default");
			return;
		}

		setBackgroundVolume(s_volume);

		std::string path = "";
		if (pszFilePath && *pszFilePath == '/') {
		  // absolut path:
		  path += pszFilePath;
		} else {
		  path += cwd;
		  path += "/";
		  path += resourcePath();
		  path += pszFilePath;
		}
		if (mmr_input_attach(s_mmrContext, path.data(), "autolist") < 0)
		{
			fprintf(stderr, "** preloadBackgroundMusic : unable to load %s\n", path.data());
			mmrerror(s_mmrContext, path.data());
			return;
		}

		s_currentBackgroundStr 	  = pszKey;
		s_isBackgroundInitialized = true;
	}
}

void SimpleAudioEngine::playBackgroundMusic(const char* pszKey, bool bLoop)
{
  if (s_currentBackgroundStr != pszKey) {
    stopBackgroundMusic(true);
  } else {
    if (s_playStatus == PAUSED)
      resumeBackgroundMusic();
    else
      rewindBackgroundMusic();
  }
  
  if (!s_isBackgroundInitialized)
    preloadBackgroundMusic(pszKey);

  if (bLoop) {
    // set it up to loop
    strm_dict_t *dictionary = strm_dict_new();
    s_repeatDictionary = strm_dict_set(dictionary, "repeat", "all");
    
    if (mmr_input_parameters(s_mmrContext, s_repeatDictionary) != 0) {
      mmrerror(s_mmrContext, "input parameters (loop)");
      return;
    }
  }
  
  if (s_hasMMRError || !s_mmrContext)
    return;
  
  if (mmr_play(s_mmrContext) < 0) {
    mmrerror(s_mmrContext, "mmr_play");
    s_hasMMRError = true;
  }
  
  if (!s_hasMMRError)
    s_playStatus = PLAYING;
}

void SimpleAudioEngine::stopBackgroundMusic(bool bReleaseData)
{
  // if we were paused then we need to resume first so that we can play
  if (s_playStatus == PAUSED)
    resumeBackgroundMusic();
  
  stopBackground(bReleaseData);
}

void SimpleAudioEngine::pauseBackgroundMusic()
{
	if (s_mmrContext && mmr_speed_set(s_mmrContext, 0) < 0)
	{
		mmrerror(s_mmrContext, "pause");
	}
	s_playStatus = PAUSED;
}

void SimpleAudioEngine::resumeBackgroundMusic()
{
	if (s_mmrContext && mmr_speed_set(s_mmrContext, 1000) < 0)
	{
		mmrerror(s_mmrContext, "resume");
	}
	s_playStatus = PLAYING;
}

void SimpleAudioEngine::rewindBackgroundMusic()
{
	if (s_mmrContext && mmr_seek(s_mmrContext, "1:0") < 0)
	{
		mmrerror(s_mmrContext, "rewind");
	}
}

bool SimpleAudioEngine::willPlayBackgroundMusic()
{
	return true;
}

bool SimpleAudioEngine::isBackgroundMusicPlaying()
{
	return (s_playStatus == PLAYING) && s_isBackgroundInitialized;
}

float SimpleAudioEngine::getBackgroundMusicVolume()
{
	return s_volume;
}

void SimpleAudioEngine::setBackgroundMusicVolume(float volume)
{
	if (s_volume != volume && volume >= -0.0001 && volume <= 1.0001)
	{
		s_volume = volume;

		setBackgroundVolume(volume);
	}
}

//
// Effect audio (using OpenAL)
//
float SimpleAudioEngine::getEffectsVolume()
{
	return s_effectVolume;
}

void SimpleAudioEngine::setEffectsVolume(float volume)
{
	if (volume != s_effectVolume)
	{
		EffectsMap::const_iterator end = s_effects.end();
		for (EffectsMap::const_iterator it = s_effects.begin(); it != end; it++)
		{
			alSourcef(it->second->source, AL_GAIN, volume);
		}

		s_effectVolume = volume;
	}
}

unsigned int SimpleAudioEngine::playEffect(const char* pszFilePath, bool bLoop)
{
  return playEffect(pszFilePath, NULL, bLoop);
}

unsigned int SimpleAudioEngine::playEffect(const char* pszFilePath, const char* pszKey, bool bLoop)
{
  if (!pszKey)
    pszKey = const_cast<char*>(pszFilePath);
  EffectsMap::iterator iter = s_effects.find(pszKey);
  
  if (iter == s_effects.end())
    {
      preloadEffect(pszFilePath, pszKey);
      
      // let's try again
      iter = s_effects.find(pszFilePath);
      if (iter == s_effects.end())
	{
	  fprintf(stderr, "could not find play sound %s\n", pszFilePath);
	  return -1;
	}
    }
  
  iter->second->isLooped = bLoop;
  alSourcei(iter->second->source, AL_LOOPING, iter->second->isLooped ? AL_TRUE : AL_FALSE);
  alSourcePlay(iter->second->source);
  
  return iter->second->source;
}

void SimpleAudioEngine::stopEffect(unsigned int nSoundId)
{
  alSourceStop(nSoundId);
}

bool SimpleAudioEngine::isEffectPlaying(const char* pszKey) {
  EffectsMap::iterator iter = s_effects.find(pszKey);
  
  if (iter == s_effects.end())
    {
      fprintf(stderr, "could not find play sound %s\n", pszKey);
      return false;
    }

  ALint sourceState;
  alGetSourcei(iter->second->source, AL_SOURCE_STATE, &sourceState);
  return sourceState == AL_PLAYING;
}

bool SimpleAudioEngine::isEffectPlaying() {
  EffectsMap::iterator iter; for(iter = s_effects.begin(); iter != s_effects.end(); ++iter ) {
      ALint sourceState;
      alGetSourcei(iter->second->source, AL_SOURCE_STATE, &sourceState);
      if (sourceState == AL_PLAYING)
	return true;
  }
  return false;
}

void SimpleAudioEngine::preloadEffect(const char* pszFilePath, const char* pszKey)
{
	if (!pszKey)
		pszKey = const_cast<char*>(pszFilePath);

	EffectsMap::iterator iter = s_effects.find(pszKey);

	// check if we have this already
	if (iter == s_effects.end())
	{
		ALuint 		buffer;
		ALuint 		source;
		char 		cwd[PATH_MAX];
		soundData  *data = new soundData;

		getcwd(cwd, PATH_MAX);

		std::string path = "";
		if (pszFilePath && *pszFilePath == '/') {
		  // absolut path:
		  path += pszFilePath;
		} else {
		  path += cwd;
		  path += "/";
		  path += resourcePath();
		  path += pszFilePath;
		}

		buffer = alutCreateBufferFromFile(path.data());

		if (buffer == AL_NONE)
		{
			fprintf(stderr, "Error loading file: '%s'\n", path.data());
			alDeleteBuffers(1, &buffer);
			return;
		}

		alGenSources(1, &source);
		alSourcei(source, AL_BUFFER, buffer);

		data->isLooped = false;
		data->buffer = buffer;
		data->source = source;

		s_effects.insert(EffectsMap::value_type(pszKey, data));
	}
}

void SimpleAudioEngine::unloadEffect(const char* pszKey)
{
	EffectsMap::iterator iter = s_effects.find(pszKey);

	if (iter != s_effects.end())
	{
		alSourceStop(iter->second->source);
		alDeleteSources(1, &iter->second->source);
		alDeleteBuffers(1, &iter->second->buffer);
		delete iter->second;

		int err = alGetError();
		if (err != AL_NO_ERROR)
			printALError(err);

		s_effects.erase(iter);
	}
}
}

static inline char *itokey(int key) {
	static char skey[64];
	return itoa(key, skey, 10);
}

// -- sound support:
extern "C" {
	void sndPrefetchSound(const char *snd) {
		audio::SimpleAudioEngine::sharedEngine()->preloadEffect( snd );
	}
	void sndPrefetchSoundWithKey(const int key, const char *snd) {
		audio::SimpleAudioEngine::sharedEngine()->preloadEffect( snd, itokey(key) );
	}
	void sndPlaySound(const char *snd) {
		audio::SimpleAudioEngine::sharedEngine()->playEffect( snd, false );
	}
	void sndPlaySoundWithKey(const int key) {
		audio::SimpleAudioEngine::sharedEngine()->playEffect( itokey(key), false );
	}
	void sndStopSoundWithKey(const int key) {
		audio::EffectsMap::iterator iter = audio::s_effects.find( itokey(key) );
		if (iter == audio::s_effects.end())
			fprintf(stderr, "** sndStopSoundWithKey: no sound found for the key %d.", key);
		else {
			audio::SimpleAudioEngine::sharedEngine()->stopEffect( iter->second->source );
		}
	}
	float sndSoundGetVolume() {
		return audio::SimpleAudioEngine::sharedEngine()->getEffectsVolume();
	}
	void sndSoundSetVolume(float volume) {
		audio::SimpleAudioEngine::sharedEngine()->setEffectsVolume( volume );
	}
	// -- background music support:
	void sndPrefetchMusic(const char *snd) {
		audio::SimpleAudioEngine::sharedEngine()->preloadBackgroundMusic( snd );
	}
	void sndPrefetchMusicWithKey(const int key, const char *snd) {
		audio::SimpleAudioEngine::sharedEngine()->preloadBackgroundMusic( snd, itokey(key) );
	}
	void sndPlayMusic(const char *snd) {
		audio::SimpleAudioEngine::sharedEngine()->playBackgroundMusic( snd, true );
	}
	void sndPlayMusicWithKey(const int key) {
		audio::SimpleAudioEngine::sharedEngine()->playBackgroundMusic( itokey(key), true );
	}
	void sndMusicStop() {
	}
	void sndMusicPause() {
	}
	void sndMusicResume() {
	}
	float sndMusicGetVolume() {
		return audio::SimpleAudioEngine::sharedEngine()->getBackgroundMusicVolume( );
	}
	void sndMusicSetVolume(float volume) {
		audio::SimpleAudioEngine::sharedEngine()->setBackgroundMusicVolume( volume );
	}
	// --
	void sndRemove(const char *snd) {
		audio::SimpleAudioEngine::sharedEngine()->unloadEffect( snd );
	}
	// --
	void sndBeep() {
		static bool prepare = false;
		if(!prepare) {
			soundplayer_prepare_sound("input_keypress");
			prepare = true;
		}
		soundplayer_play_sound("input_keypress");
	}
	void sndVibrate() {
		fprintf(stderr, "sndVibrate: vibrating not supported.");
	}
}
// done.