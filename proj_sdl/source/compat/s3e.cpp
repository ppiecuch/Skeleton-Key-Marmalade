#include "s3e.h"
#include "IwGx.h"
#include "Iw2D.h"
#include <SDL/SDL_opengl.h>
#include <string>
#include <vector>
#include <map>
#include <stb_image.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include "utf8.h"

#if defined __QNXNTO__
# include <bps/bps.h>
# include <scoreloop/scoreloopcore.h>
# include <AL/al.h>
# include "_bb_simple_audio_engine.h"

#define _audio audio::SimpleAudioEngine::sharedEngine()

#endif

struct CIwGxFont
{
  bool utf8;
  const char * image;
  const char * charmap;
};

#include "fonts/font_algerian_16.h"
#include "fonts/font_deutsch_26.h"
#include "fonts/font_gabriola_22b.h"
#include "fonts/font_algerian_20.h"
#include "fonts/font_gabriola_14.h"
#include "fonts/font_algerian_24.h"
#include "fonts/font_gabriola_16b.h"


using std::string;
using std::vector;
using std::map;
using std::pair;

template <class A> A clamp(const A x, const A min_v, const A max_v) { return x<min_v?  min_v: (x>max_v? max_v: x); }

const float FColorRGB = 1./255.;

#define ABGR_RED2F(c)      (((c)&0x000000ff)*FColorRGB)
#define ABGR_GREEN2F(c)    ((((c)>>8)&0x000000ff)*FColorRGB)
#define ABGR_BLUE2F(c)	   ((((c)>>16)&0x000000ff)*FColorRGB)
#define ABGR_ALPHA2F(c)    ((((c)>>24)&0x000000ff)*FColorRGB)

int32 s3ePointerGetInt(s3ePointerProperty property) 
{
  if (property == S3E_POINTER_MULTI_TOUCH_AVAILABLE) return 1;
}

struct s3eCallbackInfo {
  s3eCallback fn;
  void *userData;
} _ptrTouchEvent = {NULL, NULL},
  _ptrTouchMotionEvent = {NULL, NULL},
  _ptrButtonEvent = {NULL, NULL},
  _ptrMotionEvent = {NULL, NULL};

s3eResult s3ePointerRegister(s3ePointerCallback cbid, s3eCallback fn, void* userData)
{
  switch(cbid)
    {
    case S3E_POINTER_TOUCH_EVENT: _ptrTouchEvent = (s3eCallbackInfo){fn, userData}; return S3E_RESULT_SUCCESS;
    case S3E_POINTER_TOUCH_MOTION_EVENT: _ptrTouchMotionEvent = (s3eCallbackInfo){fn, userData}; return S3E_RESULT_SUCCESS;
    case S3E_POINTER_BUTTON_EVENT: _ptrButtonEvent = (s3eCallbackInfo){fn, userData}; return S3E_RESULT_SUCCESS;
    case S3E_POINTER_MOTION_EVENT: _ptrMotionEvent = (s3eCallbackInfo){fn, userData}; return S3E_RESULT_SUCCESS;
    }
}

s3eResult s3ePointerUnRegister(s3ePointerCallback cbid, s3eCallback fn)
{
  switch(cbid)
    {
    case S3E_POINTER_TOUCH_EVENT: _ptrTouchEvent = (s3eCallbackInfo){NULL, NULL}; return S3E_RESULT_SUCCESS;
    case S3E_POINTER_TOUCH_MOTION_EVENT: _ptrTouchMotionEvent = (s3eCallbackInfo){NULL, NULL}; return S3E_RESULT_SUCCESS;
    case S3E_POINTER_BUTTON_EVENT: _ptrButtonEvent = (s3eCallbackInfo){NULL, NULL}; return S3E_RESULT_SUCCESS;
    case S3E_POINTER_MOTION_EVENT: _ptrMotionEvent = (s3eCallbackInfo){NULL, NULL}; return S3E_RESULT_SUCCESS;
    }
}

s3eResult s3ePointerUpdate()
{
}

int32 s3eFileGetSize(s3eFile* file) {
  struct stat st; 
  if (fstat(fileno(file), &st) == 0)
    return st.st_size;
  return 0;
}

s3eBool s3eAudioIsCodecSupported(s3eEnum codec) {
  if (codec == S3E_AUDIO_CODEC_PCM) return true;
  return false;
}

static uint _last_audio_source = 0;
static uint _default_freq = 44100;
static string _audio_error;

s3eResult s3eAudioPlay(const char* filename, uint32 repeatCount) { 
  _last_audio_source = _audio->playEffect(filename, repeatCount); 
  return S3E_RESULT_SUCCESS; 
}
s3eResult s3eAudioPause() { alSourcePause(_last_audio_source); return S3E_RESULT_SUCCESS; }
s3eResult s3eAudioResume(){ alSourcePlay(_last_audio_source); return S3E_RESULT_SUCCESS; }
void s3eAudioStop() { alSourceStop(_last_audio_source); }
s3eBool s3eAudioIsPlaying() { ALint sourceState; alGetSourcei(_last_audio_source, AL_SOURCE_STATE, &sourceState); return (sourceState == AL_PLAYING); }
void s3eAudioSetInt(s3eEnum f, int v) {
  if (f == S3E_AUDIO_VOLUME) _audio->setEffectsVolume( clamp(v, 0, 255)/255. );
}

typedef pair<ALuint, ALuint> ChannelInfo;
static vector<ChannelInfo> _channels;

s3eResult s3eSoundChannelPlay(int channel, int16* start, uint32 numSamples, int32 repeat, int32 loopfrom) {
  if (_channels.size() > channel) {
    const ChannelInfo &ch = _channels[channel];
    alBufferData(ch.second, AL_FORMAT_STEREO16, start, numSamples, _default_freq);
    alSourcei(ch.first, AL_LOOPING, repeat ? AL_TRUE : AL_FALSE);
    alSourcePlay(ch.first);
    return S3E_RESULT_SUCCESS;
  } else {
    _audio_error = f_ssprintf("s3eSoundChannelPlay: invalid channel %d (of %d).", channel, _channels.size());
    fprintf (stderr, "*** %s\n", _audio_error.c_str());
    return S3E_RESULT_ERROR;
  }
}
int s3eSoundGetFreeChannel() {
  ALuint source;		
  ALuint buffer;
  alGenBuffers (1, &buffer);
  if (alGetError () != AL_NO_ERROR)
    {
      _audio_error = "*** s3eSoundGetFreeChannel: failed with alGenBuffers.";
      fprintf (stderr, "*** %s\n", _audio_error.c_str());
      return -1;
    }
  alGenSources(1, &source);
  alSourcei(source, AL_BUFFER, buffer);

  _channels.push_back(ChannelInfo(source, buffer));

  return source;
}
const char* s3eSoundGetErrorString() { return _audio_error.c_str(); }
void s3eSoundSetInt(s3eEnum f, int v) {
  if (f == S3E_SOUND_VOLUME) _audio->setBackgroundMusicVolume( clamp(v, 0, 255)/255. );
  else if (f == S3E_SOUND_DEFAULT_FREQ) _default_freq = v;
}

// -----  IwResManager -----

class CcIw2DFont : public CIwGxFont, public CIw2DFont
{
};

static map<string, const CIwGxFont*> _fonts;
static CcIw2DFont *_current_font;
static uint32 _current_color;

void IwResManagerInit()
{
  _fonts["font_algerian_16"] = &font_algerian_16;
  _fonts["font_algerian_20"] = &font_algerian_20;
  _fonts["font_algerian_24"] = &font_algerian_24;
  _fonts["font_deutsch_26"] = &font_deutsch_26;
  _fonts["font_gabriola_14"] = &font_gabriola_14;
  _fonts["font_gabriola_16b"] = &font_gabriola_16b;
  _fonts["font_gabriola_22b"] = &font_gabriola_22b;
}

// ----- Iw2D -----

class CcIw2DImage : public CIw2DImage
{
private:
  CIwIVec2 size;
  std::string file;
  std::string error;
public:
  CcIw2DImage(const char* from_file) {

    file = from_file;

    int width, height, channels;
    const int force_channels = 0;
    
    // try to read raw data:
    unsigned char *idata = stbi_load( from_file, &width, &height, &channels, force_channels );
    if( idata == NULL ) {
      fprintf(stderr, "[CcIw2DImage] Failed to get raw data from the file %s - image not supported or not an image (%s).\n", 
	      from_file, stbi_failure_reason());
      error = stbi_failure_reason();
      return;
    }
  }
  virtual float GetWidth() { return size.x; }
  virtual float GetHeight()  { return size.y; }

  virtual ~CcIw2DImage() {};
  // --
  const char *GetErrorString() { error.empty()?NULL:error.c_str(); }
};

uint32 Iw2DGetSurfaceHeight() {
  #if defined __BB10__
  return 1280;
  #elif defined __PLAYBOOK__
  return 600;
  #else
  #error Cannot determine device.
  #endif
}
uint32 Iw2DGetSurfaceWidth() {
  #if defined __BB10__
  return 768;
  #elif defined __PLAYBOOK__
  return 1024;
  #else
  #error Cannot determine device.
  #endif
}

void Iw2DSetFont(const CIw2DFont *f) {
  _current_font = (CcIw2DFont*)f;
}

void Iw2DSetColour(const uint32 color) {
  _current_color = color;
}

void Iw2DClearScreen(const uint32 color) {
  glClearColor(ABGR_RED2F(color), ABGR_GREEN2F(color), ABGR_BLUE2F(color), ABGR_ALPHA2F(color));
  glClear(GL_COLOR_BUFFER_BIT);
}

void Iw2DFinishDrawing() {
  SDL_GL_SwapBuffers();
}
