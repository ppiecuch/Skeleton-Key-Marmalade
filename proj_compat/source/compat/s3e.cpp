#include "s3e.h"
#include "IwGx.h"
#include "Iw2D.h"
#include "IwResManager.h"
#include <SDL/SDL_opengl.h>
#include <string>
#include <vector>
#include <map>
#include <stb_image.h>
#include <SOIL.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <fcntl.h>
#include <errno.h>
#include "dgreed/utils.h"
#include "dgreed/darray.h"
#include "dgreed/localization.h"
#include "dgreed/async.h"
#include "toolkit.h"
#include "foreach_.h"
#include "utf8.h"

#include "dgreed/utils.h"
#include "dgreed/darray.h"
#include "dgreed/memory.h"

# include <AL/al.h>

#if defined __QNXNTO__
# include <bps/bps.h>
# include <bps/navigator.h>
# include <scoreloop/scoreloopcore.h>
# include "_bb_simple_audio_engine.h"

#include "../../source/ig2d/ig_distorter.h"

#define _audio audio::SimpleAudioEngine::sharedEngine()

#endif

#define ENABLE_ANIM_BG (0)

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

#include "TileSetUtility.h"

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

inline static uint32_t _nextpot(uint32_t x) {
    x = x - 1;
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >>16);
    return x + 1;
}

static uint32 _GetDevWidth() {
  #if defined __BB10__
  return 1280;
  #elif defined __PLAYBOOK__
  return 600;
  #else
  #error Cannot determine device.
  #endif
}

static uint32 _GetDevHeight() {
  #if defined __BB10__
  return 768;
  #elif defined __PLAYBOOK__
  return 1024;
  #else
  #error Cannot determine device.
  #endif
}

/// Both these values must be your real window size, so of course these values can't be static
#define screen_width  _GetDevWidth()
#define screen_height _GetDevHeight()
#define virtual_width  320
#define virtual_height 480

/// The scale considering the screen size and virtual size
const float scale_x = (float)screen_width / (float)virtual_width;
const float scale_y = (float)screen_height / (float)virtual_height;

// This is your target virtual resolution for the game, the size you built your game to
static void _setupLetterbox() {
  float targetAspectRatio = (float)virtual_width/(float)virtual_height;
 
  // figure out the largest area that fits in this resolution at the desired aspect ratio
  int width = screen_width;
  int height = width / targetAspectRatio + 0.5f;

  if (height > screen_height ) {
    //It doesn't fit our height, we must switch to pillarbox then
    height = screen_height ;
    width = height * targetAspectRatio + 0.5f;
  }
 
  // set up the new viewport centered in the backbuffer
  const int vp_x = (screen_width  / 2.) - (width / 2.);
  const int vp_y = (screen_height / 2.) - (height / 2.);
 
  glViewport(vp_x, vp_y, width, height);

  /// Now that our viewport is set we should set our 2d perspective
 
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, screen_width, screen_height, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Push in scale transformations
  glScalef(scale_x, scale_y, 1.0f);
  printf("** Letterbox scale: %f,%f; viewport: %d %d %d %d.\n", scale_x, scale_y, vp_x, vp_y, width, height);
}

static bool _fileExists (const char *path) {
  if (path) {
    struct stat st;
    return (stat(path, &st) == 0);
  } else
    return false;
}

static int _cp(const char *to, const char *from) {
    int fd_to, fd_from;
    char buf[4096];
    ssize_t nread;
    int saved_errno;

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
        return -1;

    fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd_to < 0)
        goto out_error;

    while (nread = read(fd_from, buf, sizeof buf), nread > 0)
    {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);
            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR)
            {
                goto out_error;
            }
        } while (nread > 0);
    }
    if (nread == 0)
    {
        if (close(fd_to) < 0)
        {
            fd_to = -1;
            goto out_error;
        }
        close(fd_from);
        /* Success! */
        return 0;
    }
  out_error:
    saved_errno = errno;
    close(fd_from);
    if (fd_to >= 0)
        close(fd_to);
    errno = saved_errno;
    return -1;
}

int32 s3ePointerGetInt(s3ePointerProperty property) 
{
  if (property == S3E_POINTER_MULTI_TOUCH_AVAILABLE) return 0;
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

/// Global pointer and keyboard state:
static s3eBool done = S3E_FALSE;
static Uint8* keys = NULL;
// end.

s3eResult s3ePointerUpdate()
{
  SDL_Event event = { 0 };
  if (SDL_PollEvent(&event))
    switch(event.type) {
    case SDL_QUIT:
      {
	done = S3E_TRUE; 
      }; break;
    case SDL_MOUSEBUTTONDOWN:
      {
	const int x = event.button.x, y = event.button.y;
	if (_ptrButtonEvent.fn) {
	  s3ePointerEvent ev = { S3E_POINTER_BUTTON_LEFTMOUSE, 1, x, y };
	  _ptrButtonEvent.fn( &ev, _ptrButtonEvent.userData);
	}
      }; break;
    case SDL_MOUSEBUTTONUP:
      {
	const int x = event.button.x, y = event.button.y;
	if (_ptrButtonEvent.fn) {
	  s3ePointerEvent ev = { S3E_POINTER_BUTTON_LEFTMOUSE, 0, x, y };
	  _ptrButtonEvent.fn( &ev, _ptrButtonEvent.userData);
	}
      }; break;
    case SDL_MOUSEMOTION: 
      {
	const int x = event.button.x, y = event.button.y;
	if (_ptrTouchMotionEvent.fn) {
	  s3ePointerMotionEvent ev = { x, y };
	  _ptrTouchMotionEvent.fn( &ev, _ptrTouchMotionEvent.userData);
	}
      }; break;
    }
}

const int s3eKeyCount = 211;

static SDLKey s_s3eToSDLTranslation[s3eKeyCount] =
{
	(SDLKey)0, //s3eKeyFirst
	SDLK_ESCAPE,//	s3eKeyEsc ,				//!< Esc.
	SDLK_TAB,//	s3eKeyTab ,				//!< Tab.
	SDLK_BACKSPACE,//	s3eKeyBackspace ,		//!< Backspace.
	SDLK_RETURN,//  s3eKeyEnter ,			//!< Enter.
	SDLK_LSHIFT,//  s3eKeyShift ,			//!< Key Shift.
	SDLK_LCTRL,//  s3eKeyControl ,			//!< Key Control. 
	(SDLKey)0,//  s3eKeyResevered , 		//!< Reserved, do not use.
	SDLK_SPACE,//  s3eKeySpace ,			//!< Key Space.
	SDLK_LEFT,//  s3eKeyLeft ,			//!< Key Left. 
	SDLK_UP,//  s3eKeyUp ,				//!< Key Up. 
	SDLK_RIGHT,//  s3eKeyRight ,			//!< Key Right. 
	SDLK_DOWN,//  s3eKeyDown ,			//!< Key Down. 
	SDLK_0,//  s3eKey0 ,				//!< Key 0. 
	SDLK_1,//  s3eKey1 ,				//!< Key 1. 
	SDLK_2,//  s3eKey2 ,				//!< Key 2. 
	SDLK_3,//  s3eKey3 ,				//!< Key 3. 
	SDLK_4,//  s3eKey4 ,				//!< Key 4. 
	SDLK_5,//  s3eKey5 ,				//!< Key 5. 
	SDLK_6,//  s3eKey6 ,				//!< Key 6. 
	SDLK_7,//  s3eKey7 ,				//!< Key 7. 
	SDLK_8,//  s3eKey8 ,				//!< Key 8. 
	SDLK_9,//  s3eKey9 ,				//!< Key 9. 
	SDLK_a,//3eKeyA ,				//!< Key A.
	SDLK_b,//  s3eKeyB ,				//!< Key B. 
	SDLK_c,//3eKeyC ,				//!< Key C. 
	SDLK_d,//  s3eKeyD ,				//!< Key D. 
	SDLK_e,//  s3eKeyE ,				//!< Key E. 
	SDLK_f,//  s3eKeyF ,				//!< Key F. 
	SDLK_g,//  s3eKeyG ,				//!< Key G. 
	SDLK_h,//  s3eKeyH ,				//!< Key H. 
	SDLK_i,//  s3eKeyI ,				//!< Key I. 
	SDLK_j,//  s3eKeyJ ,				//!< Key J. 
	SDLK_k,//  s3eKeyK ,				//!< Key K. 
	SDLK_l,//  s3eKeyL ,				//!< Key L. 
	SDLK_m,//  s3eKeyM ,				//!< Key M. 
	SDLK_n,//  s3eKeyN ,				//!< Key N. 
	SDLK_o,//  s3eKeyO ,				//!< Key O.
	SDLK_p,//  s3eKeyP ,				//!< Key P. 
	SDLK_q,//  s3eKeyQ ,				//!< Key Q. 
	SDLK_r,//  s3eKeyR ,				//!< Key R. 
	SDLK_s,//  s3eKeyS ,				//!< Key S. 
	SDLK_t,//  s3eKeyT ,				//!< Key T. 
	SDLK_u,//  s3eKeyU ,				//!< Key U. 
	SDLK_v,//  s3eKeyV ,				//!< Key V. 
	SDLK_w,//  s3eKeyW ,				//!< Key W.
	SDLK_x,//3eKeyX ,				//!< Key X. 
	SDLK_y,//  s3eKeyY ,				//!< Key Y. 
	SDLK_z,//  s3eKeyZ ,				//!< Key Z. 
	SDLK_F1,//  s3eKeyF1 ,				//!< Key F1. 
	SDLK_F2,//3eKeyF2 ,				//!< Key F2.
	SDLK_F3,//  s3eKeyF3 ,				//!< Key F3. 
	SDLK_F4,//  s3eKeyF4 ,				//!< Key F4.
	SDLK_F5,//  s3eKeyF5 ,				//!< Key F5. 
	SDLK_F6,//  s3eKeyF6 ,				//!< Key F6. 
	SDLK_F7,//  s3eKeyF7 ,				//!< Key F7. 
	SDLK_F8,//3eKeyF8 ,				//!< Key F8.
	SDLK_F9,//  s3eKeyF9 ,				//!< Key F9. 
	SDLK_F10,//  s3eKeyF10 ,				//!< Key F10. 
	SDLK_KP0,//3eKeyNumPad0 ,			//!< Key NumPad0.
	SDLK_KP1,//3eKeyNumPad1 ,			//!< Key NumPad1.
	SDLK_KP2,//3eKeyNumPad2 ,			//!< Key NumPad2.
	SDLK_KP3,//3eKeyNumPad3 ,			//!< Key NumPad3.
	SDLK_KP4,//3eKeyNumPad4 ,			//!< Key NumPad4.
	SDLK_KP5,//3eKeyNumPad5 ,			//!< Key NumPad5.
	SDLK_KP6,//3eKeyNumPad6 ,			//!< Key NumPad6.
	SDLK_KP7,//3eKeyNumPad7 ,			//!< Key NumPad7.
	SDLK_KP8,//3eKeyNumPad8 ,			//!< Key NumPad8.
	SDLK_KP9,//3eKeyNumPad9 ,			//!< Key NumPad9.
	SDLK_KP_PLUS,//3eKeyNumPadPlus ,		//!< Key NumPadPlus.
	SDLK_KP_MINUS,//3eKeyNumPadMinus ,		//!< Key NumPadMinus.
	SDLK_KP_ENTER,//3eKeyNumPadEnter ,		//!< Key NumPadEnter.

	//Not too sure about these...

	SDLK_RETURN,//3eKeyRSK ,				//!< Right Soft Key.
	SDLK_ESCAPE,//3eKeyLSK ,				//!< Left Soft Key.
	(SDLKey)0,//3eKeyLS ,				//!< Left Shoulder button.
	(SDLKey)0,//3eKeyRS ,				//!< Right shoulder button.
	SDLK_HASH,//3eKeyHash ,			//!< # Key.
	SDLK_ASTERISK,//3eKeyStar ,			//!< * Key.
	SDLK_SPACE,//3eKeyOk ,				//!< Select Key.
	SDLK_BACKSPACE,//3eKeyCLR ,				//!< CLR key.
	//   ,// Volume
	(SDLKey)0,//3eKeyVolUp ,			//!< Volume Up Key.
	(SDLKey)0,//3eKeyVolDown ,			//!< Volume Down Key.
	//  ,
	///,Misc.
	(SDLKey)0,//3eKeyCamera ,			//!< Camera button.
	(SDLKey)0,//3eKeyMic ,				//!< Microphone button.
	(SDLKey)0,//3eKeyFn ,				//!< Fn button.
	(SDLKey)0,//3eKeySym ,				//!< Sym button.
	////,Call  
	(SDLKey)0,//3eKeyAccept ,			//!< call accept (talk).
	(SDLKey)0,//3eKeyEnd ,				//!< call end (reject).
	SDLK_HOME,//3eKeyHomePage ,		//!< Home key.
	SDLK_WORLD_0,//s3eKeyButton1 ,			//<! Generic Button1.
	SDLK_WORLD_1,//3eKeyButton2 ,			//<! Generic Button2.
	SDLK_WORLD_2,//3eKeyButton3 ,			//<! Generic Button3.
	SDLK_WORLD_3,//3eKeyButton4 ,			//<! Generic Button4.
	SDLK_WORLD_4,//3eKeyButton5 ,			//<! Generic Button5.
	SDLK_WORLD_5,//3eKeyButton6 ,			//<! Generic Button6.
	SDLK_WORLD_6,//3eKeyButton7 ,			//<! Generic Button7.
	SDLK_WORLD_7,//3eKeyButton8 ,			//<! Generic Button8.
	SDLK_F11,//s3eKeyF11 ,				//!< Key F11. 
	SDLK_F12,//3eKeyF12 ,				//!< Key F12. 
	SDLK_LALT,//3eKeyAlt ,				//!< Alt key.
	SDLK_a,//s3eKeyAbsGameA = 200,	//<! Abstract Game keyA.
	SDLK_s,//3eKeyAbsGameB ,		//<! Abstract Game keyB.
	SDLK_d,//3eKeyAbsGameC ,		//<! Abstract Game keyC.
	SDLK_f,//3eKeyAbsGameD ,		//<! Abstract Game keyD.
	SDLK_UP,//3eKeyAbsUp	,			//<! Abstract Up.
	SDLK_DOWN,//3eKeyAbsDown ,			//<! Abstract Down.
	SDLK_LEFT,//3eKeyAbsLeft ,			//<! Abstract Left.
	SDLK_RIGHT,//3eKeyAbsRight ,		//<! Abstract Right.
	SDLK_SPACE,//3eKeyAbsOk ,			//<! Abstract Ok.
	SDLK_RETURN,//3eKeyAbsASK,			//<! Abstract action softkey.
	SDLK_ESCAPE,//s3eKeyAbsBSK,			//<! Abstract backwards softkey.
};

void s3eKeyboardUpdate() { keys = SDL_GetKeyState(NULL); }
s3eEnum s3eKeyboardGetState(s3eKeys key) { keys[s_s3eToSDLTranslation[key]]?S3E_KEY_STATE_PRESSED:S3E_KEY_STATE_RELEASED; }
s3eBool s3eDeviceCheckQuitRequest() { return done; }

s3eFile* s3eFileOpen(const char* filename, const char* mode) { 
  struct stat st;
  if (stat(filename, &st) == 0)
    return fopen(filename, mode);
  if (resourceExists(filename))
    return fopen(resourcePath(filename), mode);
  return NULL;
}

int32 s3eFileGetSize(s3eFile* file) {
  struct stat st; 
  if (fstat(fileno(file), &st) == 0)
    return st.st_size;
  return 0;
}

static int __checkALError(const char *funcName)
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

s3eBool s3eAudioIsCodecSupported(s3eEnum codec) {
  if (codec == S3E_AUDIO_CODEC_PCM) return true;
  else if (codec == S3E_AUDIO_CODEC_MP3) return true;
  return false;
}

static uint _default_freq = 44100;
static string _audio_error;

s3eResult s3eAudioPlay(const char* filename, uint32 repeatCount) { 
  if (_audio->playBackgroundMusic(filename, repeatCount))
    return S3E_RESULT_SUCCESS; 
  else
    return S3E_RESULT_ERROR; 
}
s3eResult s3eAudioPause() { return _audio->pauseBackgroundMusic()?S3E_RESULT_SUCCESS:S3E_RESULT_ERROR; }
s3eResult s3eAudioResume(){ return _audio->resumeBackgroundMusic()?S3E_RESULT_SUCCESS:S3E_RESULT_ERROR; }
void s3eAudioStop() { _audio->stopBackgroundMusic(); }
s3eBool s3eAudioIsPlaying() { return _audio->isBackgroundMusicPlaying()?S3E_RESULT_SUCCESS:S3E_RESULT_ERROR; }
void s3eAudioSetInt(s3eEnum f, int v) {
  if (f == S3E_AUDIO_VOLUME) _audio->setEffectsVolume( clamp(v, 0, 255)/255. );
}

// allocated sound information:
typedef pair<ALuint, int16*> BufferInfo;
typedef pair<ALuint, BufferInfo> ChannelInfo;
static map<ALuint, BufferInfo> _channels;
static vector<BufferInfo> _buffers;
static vector<ALuint> _sources;
// end.

s3eResult s3eSoundChannelPlay(int channel, int16* start, uint32 numSamples, int32 repeat, int32 loopfrom) {
  if (channel < 0) {
    _audio_error = f_ssprintf("s3eSoundChannelPlay: invalid channel %d (of %d allocated).", channel, _channels.size());
    fprintf (stderr, "*** %s\n", _audio_error.c_str());
    return S3E_RESULT_ERROR;
  } else {

    // look for this buffer:
    int b = 0; for(int b=0; b<_buffers.size(); ++b) {
      const BufferInfo &binfo = _buffers[b];
      if (binfo.second == start)
	break;
    }

    BufferInfo binfo;
    if (b == _buffers.size()) { // add new buffer
      ALuint buffer;
      alGenBuffers (1, &buffer);
      if (__checkALError("s3eSoundChannelPlay/alGenBuffers") != AL_NO_ERROR) {
	printf("*** adding new audio buffer %d.\n", buffer);
	alBufferData(buffer, AL_FORMAT_STEREO16, start, numSamples, _default_freq);
	_buffers.push_back(binfo = BufferInfo(buffer, start));
      } else {
	_audio_error = "s3eSoundChannelPlay: failed with alGenBuffers.";
	fprintf (stderr, "*** %s\n", _audio_error.c_str());
	return S3E_RESULT_ERROR;
      }
    }

    alSourcei(channel, AL_BUFFER, binfo.first);
    alSourcei(channel, AL_LOOPING, repeat ? AL_TRUE : AL_FALSE);
    alSourcePlay(channel);

    _channels[channel] = binfo;

    return S3E_RESULT_SUCCESS;
  }
}
int s3eSoundGetFreeChannel() {

  __checkALError("s3eSoundGetFreeChannel"); // clear error message

  // look for free pre-allocated source:
  for(int x=0; x<_sources.size(); ++x) {
    ALint sourceState;
    alGetSourcei(_sources[x], AL_SOURCE_STATE, &sourceState);
    if (sourceState != AL_PLAYING) {
      alSourcei(_sources[x], AL_BUFFER, 0);
      return _sources[x];
    }
  }

  // create and add new source:
  ALuint source;		
  alGenSources(1, &source);
  if (__checkALError("s3eSoundGetFreeChannel/alGenSources") != AL_NO_ERROR)
    {
      _sources.push_back(source);
      return source;
    }
  
  _audio_error = "s3eSoundGetFreeChannel: failed with alGenSources.";
  fprintf (stderr, "*** %s\n", _audio_error.c_str());
  return -1;
}
const char* s3eSoundGetErrorString() { return _audio_error.c_str(); }
void s3eSoundSetInt(s3eEnum f, int v) {
  if (f == S3E_SOUND_VOLUME) _audio->setBackgroundMusicVolume( clamp(v, 0, 255)/255. );
  else if (f == S3E_SOUND_DEFAULT_FREQ) _default_freq = v;
}

// -----  IwResManager -----

class CcIw2DFont : public CIw2DFont
{
  int width, height, num;
  uint16 asciimap[256];
  map<uint, uint16> utf8map;
  const CIwGxFont &descr;
  uint texture;
  std::vector<TileCoord> regions;
public:
  CcIw2DFont(const CIwGxFont &font) : texture(0), num(0), descr(font)
  {
    string line = font.charmap;
    // process utf8 characters
    const string::iterator end_it = utf8::find_invalid(line.begin(), line.end());
    if (end_it != line.end()) {
      fprintf( stderr, "[CcIw2DFont] Invalid UTF-8 encoding detected.\n" );
      fprintf( stderr, "[CcIw2DFont] This part is fine and will be processed: %s.\n", string(line.begin(), end_it).c_str() );
    }
    utf8::iterator<string::iterator> it (line.begin(), line.begin(), end_it);
    int index = 0; while(it.base()!=end_it) {
      uint32_t ch = *it;
      if (ch < 256)
	asciimap[ch] = index;
      else
	utf8map[ch] = index;
      ++index; ++it; // next character
    }
    printf("Character map created for %s with %d characters.\n", font.image, index);

    num = index;

    int channels;
    const int force_channels = 0;

    // try to read raw data:
    const char * path = resourceExists(font.image)?resourcePath(font.image):resourcePath(f_ssprintf("fonts/%s", font.image));
    unsigned char *idata = stbi_load( path, &width, &height, &channels, force_channels );
    // font is in alpha channel
    // bounds are in green channel
    if( idata == NULL ) {
      fprintf(stderr, "[CcIw2DFont] Failed to get raw data from the file %s - image not supported or not an image (%s).\n", 
	      path, stbi_failure_reason());
      return;
    }

    // calculate char glyph regions:
    BufferedImage img(idata, width, height);
    ArrayRegions rc = TileSetUtilityRGBA::inferNumberColumns(img);
    ArrayRegions rr = TileSetUtilityRGBA::inferNumberRows(img);
    regions = TileSetUtilityRGBA::getTiles(img, rc, rr);
    printf("CcIw2DFont image %s: %d regions.\n", font.image, regions.size());
    if (num != regions.size())
      fprintf(stderr, "CcIw2DFont image %s: diffrent number of regions and characters: %d<>%d.\n", font.image, regions.size(), num);

    texture = SOIL_create_OGL_texture(idata, width, height, channels, SOIL_CREATE_NEW_ID, SOIL_FLAG_POWER_OF_TWO|SOIL_FLAG_MULTIPLY_ALPHA);
    free(idata);

    printf("CcIw2DFont image %s: texture %d, dim. %dx%d\n", font.image, texture, width, height);
  }

  uint GetTexture() const { return texture; }
  //  not_found_ch : not found character placeholder
  const TileCoord &GetTextureRegion(uint ch, char not_found_ch = '?') {
    int index = 0; if (ch < 256)
      index = asciimap[ch];
    else if (utf8map.count(ch))
      index = utf8map[ch];
    else index = asciimap[not_found_ch];

    assert(index<regions.size());

    return regions[index];
  }
  TileTextureCoord GetTextureCoord(uint ch, char not_found_ch = '?') {
    int index = 0; if (ch < 256)
      index = asciimap[ch];
    else if (utf8map.count(ch))
      index = utf8map[ch];
    else index = asciimap[not_found_ch];
    const TileCoord &coord = regions[index];
    const float w = width, h = height;
    return (TileTextureCoord){coord.x1/w, coord.y1/h, coord.x2/w, coord.y2/h};
  }
  int GetNumChars() const { return num; }
};

static map<string, CcIw2DFont*> _fonts;
static CcIw2DFont *_current_font;
static uint32 _current_color;
static CIwMat2D _current_matrix;

void IwResManagerInit()
{
  _fonts["font_algerian_16"] = new CcIw2DFont(font_algerian_16);
  _fonts["font_algerian_20"] = new CcIw2DFont(font_algerian_20);
  _fonts["font_algerian_24"] = new CcIw2DFont(font_algerian_24);
  _fonts["font_deutsch_26"] = new CcIw2DFont(font_deutsch_26);
  _fonts["font_gabriola_14"] = new CcIw2DFont(font_gabriola_14);
  _fonts["font_gabriola_16b"] = new CcIw2DFont(font_gabriola_16b);
  _fonts["font_gabriola_22b"] = new CcIw2DFont(font_gabriola_22b);
  // copy sqlite databases to writeable location:
  const char *_sqdb[] = {
    "achievements.db",
    "achievements.db.overwrite",
    "levels.db",
    "levels.db.overwrite",
    "saved_game.db",
    "saved_game.db.overwrite",
    "settings.db",
    "settings.db.overwrite",
    NULL };
  for(char **db = (char **)_sqdb; *db != NULL; db++) {
    if (resourceExists(*db) && !_fileExists(writePath(*db))) {
      fprintf(stderr, "*** Copying to writeable location: %s.\n", *db);
      if (_cp(writePath(*db), resourcePath(*db)) < 0)
	fprintf(stderr, "*** Failed to copy %s.\n*** from: %s\n*** to: %s\n", *db, resourcePath(*db), writePath(*db));
    }
  }
}

// ----- IwGx -----

const CIwMat2D CIwMat2D::Identity;

// ----- Iw2D -----

class CcIw2DImage : public CIw2DImage
{
private:
  CIwIVec2 size;
  std::string file;
  std::string error;
  uint texture;
  float maxt, maxs;
  const bool native;
public:
  CcIw2DImage(const char* from_file, bool native = false) : texture(0), maxs(0), maxt(0), native(native) {

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

    size.x = width; if (native) size.x /= IGDistorter::getInstance()->multiply;
    size.y = height; if (native) size.y /= IGDistorter::getInstance()->multiply;

    texture = SOIL_create_OGL_texture2(idata, width, height, channels, SOIL_CREATE_NEW_ID, SOIL_FLAG_POWER_OF_TWO|SOIL_FLAG_MULTIPLY_ALPHA);
    free(idata);

    maxs = maxt = 1;

    printf("CcIw2DImage image %s: texture %d, dim. %dx%d, img. %dx%d, tex. %fx%f\n", from_file, texture, width, height, size.x, size.y, maxs, maxt);
  }
  virtual float GetWidth() { return size.x; }
  virtual float GetHeight()  { return size.y; }

  virtual ~CcIw2DImage() {
    if (texture) glDeleteTextures( 1, &texture );
  }
  // --
  const char *GetErrorString() const { error.empty()?NULL:error.c_str(); }
  uint GetTexture() const { return texture; }
  float GetMaxS() const { return maxs; }
  float GetMaxT() const { return maxt; }
  bool IsNative() const { return native; }
};

static map<string, CcIw2DImage*> _letterbox_bg;
static CcIw2DImage* _curr_letterbox_bg = NULL;

CIw2DImage* Iw2DCreateImageResource(const char* resource)
{
  const char *_search[] = { 
    "", 
#ifdef __PLAYBOOK__
    "*graphics/game/playbook/",
#endif
    "graphics/game/",
#ifdef __PLAYBOOK__
    "*graphics/backgrounds/playbook/",
#endif
    "graphics/backgrounds/",
    "graphics/game_menu/",
    "graphics/menu/",
#ifdef __PLAYBOOK__
    "graphics/map/playbook/",
#endif
    "graphics/map/",
    "graphics/options/",
    "graphics/achievements/",
    "graphics/select_level/",
    "graphics/instructions/",
    NULL };
  const char *path = NULL; bool native = false;
  for(char **p=(char**)_search; *p != NULL; p++) {
    const char *pp = (**p=='*')?(*p+1):*p; // skip *
    if (resourceExists(f_ssprintf("%s%s.png", pp, resource))) {
      path = resourcePath(f_ssprintf("%s%s.png", pp, resource));
      native = (**p=='*');
      break;
    }
  }
  if (path) {
    return new CcIw2DImage(path, native);
  } else {
    fprintf(stderr, "*** Resource image %s not found.\n", resource);
    return NULL;
  }
}

CIw2DFont* Iw2DCreateFontResource(const char* resource)
{
  if (_fonts.count(resource)) {
    return new CcIw2DFont(*_fonts[resource]); // make a copy based on current cached font
  }
  fprintf(stderr, "*** Font resource %s not found.\n", resource);
  return NULL;
}

uint32 Iw2DGetSurfaceWidth() { return screen_width; }

uint32 Iw2DGetSurfaceHeight() { return screen_height; }

void Iw2DSetTransformMatrix(const CIwMat2D &m) {
  printf("*** Applying new transformation matrix.\n");
  _current_matrix = m;
  glLoadMatrixf(AffineTransform::matrix(_current_matrix)());
}

void Iw2DSetFont(const CIw2DFont *f) {
  _current_font = (CcIw2DFont*)f;
}

void Iw2DSetColour(const uint32 color) {
  _current_color = color;
}

static std::pair<int,int> _text_size(const char* text, std::vector<int> &lines_width) {
  string line = text;

  const string::iterator end_it = utf8::find_invalid(line.begin(), line.end());
  if (end_it != line.end()) {
    fprintf( stderr, "[_text_size] Invalid UTF-8 encoding detected.\n" );
    fprintf( stderr, "[_text_size] This part is fine and will be processed: %s.\n", string(line.begin(), end_it).c_str() );
  }
  utf8::iterator<string::iterator> it (line.begin(), line.begin(), end_it);
  int len = 0, line_len = 0, height = 0, line_height = 0; while(it.base()!=end_it) {
    const uint32_t ch = *it;
    if (ch == '\n') {
      height += line_height; line_height = 0;
      if (line_len > len) len = line_len;
      lines_width.push_back(line_len); line_len = 0;
    } else {
      const TileCoord &box = _current_font->GetTextureRegion(ch);
      line_len += box.x2-box.x1; const int h = box.y2-box.y1; if ( h > line_height ) line_height = h;
    }
    it++;
  }

  if (line_len) lines_width.push_back(line_len); // add remaining line
  height += line_height;

  return std::pair<int,int>(len,height);
}

void Iw2DDrawString(const char* text, CIwSVec2 topLeft, CIwSVec2 size, CIw2DFontAlign horzAlign, CIw2DFontAlign vertAlign) {

  if (!*text) return; // nothing to draw

  std::vector<int> lines_width;
  const std::pair<int,int> &sz = _text_size(text, lines_width);

  int ln = 0;
  float x = topLeft.x, xs = topLeft.x;
  float y = topLeft.y;
  if (horzAlign == IW_2D_FONT_ALIGN_CENTRE) {
    x += (size.x - lines_width[ln]) / 2.;
  }
  if (vertAlign == IW_2D_FONT_ALIGN_CENTRE) {
    y += (size.y - sz.second) / 2.;
  }

  string line = text;
  const string::iterator end_it = utf8::find_invalid(line.begin(), line.end());
  if (end_it != line.end()) {
    fprintf( stderr, "[Iw2DDrawString] Invalid UTF-8 encoding detected.\n" );
    fprintf( stderr, "[Iw2DDrawString] This part is fine and will be processed: %s.\n", string(line.begin(), end_it).c_str() );
  }
  utf8::iterator<string::iterator> it (line.begin(), line.begin(), end_it);
  glBindTexture(GL_TEXTURE_2D, _current_font->GetTexture());
  int index = 0; while(it.base()!=end_it) {
    const uint32_t ch = *it;

    if (ch == '\n') {
      ++ln; y += sz.second; // next line
      // restart x position
      if (horzAlign == IW_2D_FONT_ALIGN_CENTRE) {
	x += (size.x - lines_width[ln]) / 2.;
      } else
	x = xs;
    } else {
      const TileTextureCoord &coord = _current_font->GetTextureCoord(ch);
      const TileCoord &box = _current_font->GetTextureRegion(ch);
    
      const int h = box.y2 - box.y1;
      const int w = box.x2 - box.x1;

      struct _v2c4 {
	GLfloat v[2];
	GLfloat t[2];
	uint32_t c;
      } vertices[] = {
	{ {x, y}, {coord.x1, coord.y1}, _current_color },
	{ {x, y+h}, {coord.x1, coord.y2}, _current_color },
	{ {x+w, y}, {coord.x2, coord.y1}, _current_color },
	{ {x+w, y+h}, {coord.x2, coord.y2}, _current_color },
      };
      glVertexPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->v);
      glTexCoordPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->t);
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(_v2c4), &vertices->c);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      x += w;
    }
    it++;;
  }
}

void Iw2DDrawImage(CIw2DImage* image, CIwSVec2 topLeft, CIwSVec2 size) {

  CcIw2DImage* img = dynamic_cast<CcIw2DImage*>(image);

  const float x = topLeft.x;
  const float y = topLeft.y;
  const float w = size.x;
  const float h = size.y;
  float uofs = 0;
  float vofs = 0;
  const float uwid = img->GetMaxS();
  const float vwid = img->GetMaxT();

  struct _v2c4 {
    GLfloat v[2];
    GLfloat t[2];
    uint32_t c;
  } vertices[] = {
    { {x, y}, {uofs, vofs}, _current_color },
    { {x, y+h}, {uofs, vofs+vwid}, _current_color },
    { {x+w, y}, {uofs+uwid, vofs}, _current_color },
    { {x+w, y+h}, {uofs+uwid, vofs+vwid}, _current_color },
  };
  glBindTexture(GL_TEXTURE_2D, img->GetTexture());
  glVertexPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->v);
  glTexCoordPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->t);
  glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(_v2c4), &vertices->c);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); 
}

void Iw2DClearScreen(const uint32 color) {
#ifdef __PLAYBOOK__

  static float mx = 0, my = 0;
  static int cnt = 0;
  CIwMat2D m;

  if (_curr_letterbox_bg) {
    glClearColor(0., 0., 0., 1.);
    glClear(GL_COLOR_BUFFER_BIT);

    glPushMatrix();
    glLoadIdentity();
    //
    CcIw2DImage* img = _curr_letterbox_bg;
    const float uwid = img->GetMaxS();
    const float vwid = img->GetMaxT();

    const float sh2 = screen_height/2.;
    const float sw2 = screen_width/2.;
    const int maxAlpha = 64;

    const float rotSpeed = 0.005;
    const float alphaChangeSpeed = 0.01;
    const float horizSizeChangeSpeed = 0.01;
    const float vertSizeChangeSpeed = 0.015;

    const uint32_t alpha = (uint8_t(maxAlpha*fabs(sinf(cnt*alphaChangeSpeed))) << 24) | 0x00ffffff;
    const float absmx = fabs(mx);
    const float absmy = fabs(my);
    struct _v2c4 {
      GLfloat v[2];
      GLfloat t[2];
      uint32_t c;
    } vertices[] = {
      // background:
      { {0, 0}, {0, 0}, 0x80ffffff },
      { {0, screen_height}, {0, vwid}, 0x80ffffff },
      { {screen_width, 0}, {uwid, 0}, 0x80ffffff },
      { {screen_width, screen_height}, {uwid, vwid}, 0x80ffffff },
      // moving layer:
      { {-absmx, my}, {0, 0}, alpha },
      { {-absmx, screen_height+absmy}, {0, vwid}, alpha },
      { {screen_width+absmx, my}, {uwid, 0}, alpha },
      { {screen_width+absmx, screen_height+absmy}, {uwid, vwid}, alpha },
    };
    glBindTexture(GL_TEXTURE_2D, img->GetTexture());
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glVertexPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->v);
    glTexCoordPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->t);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(_v2c4), &vertices->c);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); 
    // moving layer:
    glLoadMatrixf(AffineTransform::matrix(m.Rotation(cnt*rotSpeed, sw2, sh2))());
    glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
    //
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);    
    glPopMatrix();
    ++cnt; 
    mx = sw2 + sw2*fabs(sinf(cnt*horizSizeChangeSpeed)); my = 75+10*sinf(cnt*vertSizeChangeSpeed);
  } else
#endif
    {
      glClearColor(ABGR_RED2F(color), ABGR_GREEN2F(color), ABGR_BLUE2F(color), ABGR_ALPHA2F(color));
      glClear(GL_COLOR_BUFFER_BIT);
    }
}

void Iw2DInit() {

  // force/switch to landscape:
#if defined __QNXNTO__
  printf("** Locking to orientation portrait.\n");
  navigator_rotation_lock(true);
#endif

  if (SDL_Init(SDL_INIT_VIDEO)<0) exit(1);

#ifdef SDL_HINT_MERGE_MOUSE_MOTION_EVENTS
    SDL_SetHint(SDL_HINT_MERGE_MOUSE_MOTION_EVENTS, "1");
#endif

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 16);
  
   const bool fullscreen = true;
   Uint32 bits = 32, flags = SDL_OPENGL, width =  screen_width, height = screen_height;

    if (SDL_SetVideoMode(width, height, bits, flags) == NULL) {
      fprintf(stderr, "Failed to create main GL window!\n");
      exit(1);
    }
    SDL_WM_SetCaption("SkeletonKey", "opengl");

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_DONT_CARE);
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND); glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // dgreed utils:
    async_init();
    log_init(NULL, LOG_LEVEL_INFO);
#ifdef DEBUG
    loc_init("main.loc", false);
#else
    loc_init("main.loc", true);
#endif
    // ok.
}

void Iw2DTerminate() {
  SDL_Quit();
  // dgreed utils:
  loc_close();
  log_close();
  async_close();
  // ok.
}

void Iw2DSurfaceShow() {
}

void Iw2DFinishDrawing() {
  SDL_GL_SwapBuffers();
  SDL_Delay(0);
#ifdef DEBUG
  fflush(stdout);
#endif
}

// ----- IwResManager -----

void IwGetResManagerS::LoadGroup(const char *grp) {
    group = grp; 
    // look for playbook background:
#if ENABLE_ANIM_BG
    _curr_letterbox_bg = _letterbox_bg[grp];
    if (_curr_letterbox_bg == NULL) {
      string bg = "playbook_blur_background_wood.png";
      if (strcmp(grp, "achievements.group") == 0) {
	bg = "playbook_blur_background_wood.png";
      } else if (strcmp(grp, "instructions.group") == 0) {
	bg = "playbook_blur_background_wood.png";
      } else if (strcmp(grp, "nag.group") == 0) {
	bg = "playbook_blur_background_wood.png";
      } else if (strcmp(grp, "game.group") == 0) {
	bg = "playbook_blur_background_wood.png";
      } else if (strcmp(grp, "map.group") == 0) {
	bg = "playbook_blur_background_wood.png";
      } else if (strcmp(grp, "options.group") == 0) {
	bg = "playbook_blur_background_wood.png";
      } else if (strcmp(grp, "game_menu.group") == 0) {
	bg = "playbook_blur_background_forest_light.png";
      } else if (strcmp(grp, "menu.group") == 0) {
	bg = "playbook_blur_background_forest_light.png";
      } else if (strcmp(grp, "select_level.group") == 0) {
	bg = "playbook_blur_background_wood.png";
      }

      if (resourceExists(f_ssprintf("graphics/backgrounds/%s", bg.c_str()))) {
	const char *path = resourcePath(f_ssprintf("graphics/backgrounds/%s", bg.c_str()));
	_curr_letterbox_bg = _letterbox_bg[grp] = new CcIw2DImage(path);
      }
    }
#endif
}

// -- Scoreboard support

#define SCORE_SUBMIT_ACTION  1
#define SCORE_RETRIVE_ACTION 2

static void _process_score_queue(void);

static SC_Client_h score_client = 0;
static SC_InitData_t initData;

#define SC_NUM_MODES 3 // 3 skills levels

static struct ScoreGetCmd {
    const int type;
    SC_Error_t error_status;
    SC_ScoreList_h list;
    SC_ScoresController_h controller;
    SCL_Status status;
    SCL_Callback callback;
    ScoreGetCmd():type(SCORE_RETRIVE_ACTION),status(SC_STATUS_NONE),error_status(SC_OK),callback(NULL) { }
} scores[SC_NUM_MODES];
static struct ScoreSubmitCmd {
    const int type;
    SC_Error_t error_status;
    SC_Score_h score;
    void *payload;
    SC_ScoresController_h controller;
    SCL_Status status;
    SCL_Callback callback;
    ScoreSubmitCmd():type(SCORE_SUBMIT_ACTION),status(SC_STATUS_NONE),error_status(SC_OK),payload(NULL),callback(NULL) { }
} score_submit;

#define SCORE_CHECK_ERR(code) if(SC_Error_t _err = (code)) { fprintf(stderr, "^^ %s failed at line %d with error code: %d.\n", __FUNCTION__, __LINE__, _err); return false; }

// inclue Scoreloop leadersboard information:
#include "../sc.info.h"

#define QUEUE_STR_LEN 32
#define QUEUE_FILE writePath("slqueues.db")
static DArray score_queue;
static int live_requests = 0;
typedef struct {
  double result;       // time
  unsigned int mode;   // skill
  double minor_result; // numer of moves
  unsigned int level;  // level number
} QueuedScore;

#ifdef __QNXNTO__
# if BPS_VERSION == 3000000
SC_ScoreFormatter_h SC_Client_GetScoreFormatter(SC_Client_h score_client) {
  SC_ScoreFormatter_h sf;
  SC_Client_GetScoreFormatter(score_client, &sf);
  return sf;
}
# define SC_SCORES_SEARCH_LIST_ALL SC_SCORE_SEARCH_LIST_GLOBAL
# define SC_ScoresSearchList_t SC_ScoreSearchList_t
# define SC_ScoreList_GetCount SC_ScoreList_GetScoresCount
# define SC_ScoreList_GetAt    SC_ScoreList_GetScore
# define SC_ScoresController_LoadScoresAtRank SC_ScoresController_LoadRangeAtRank
# endif
#endif

static void _load_queues(void) {
    if(file_exists(QUEUE_FILE)) {
        FileHandle f = file_open(QUEUE_FILE);
        
        uint32 magic = file_read_uint32(f);
        if(magic != FOURCC('G', 'S', 'Q', '0')) {
            fprintf(stderr, "[scoreloop] Unable to load score queues");
            return;
        }

        file_read(f, &score_queue, sizeof(score_queue));

        size_t sizeof_score = score_queue.item_size;
        
        if(sizeof(QueuedScore) != sizeof_score) {
            fprintf(stderr, "[scoreloop] Score queues data size mismatch");
            score_queue = darray_create(sizeof(QueuedScore), 0);
            return;
        }

        fprintf(stderr, "[scoreloop] Restored %d score requests\n", sizeof_score);

        size_t score_bytes = sizeof_score * score_queue.reserved;
        score_queue.data = MEM_ALLOC(score_bytes);
        file_read(f, score_queue.data, score_bytes);
        
        file_close(f);
    } else {
        score_queue = darray_create(sizeof(QueuedScore), 0);
    }
}

static void _flush_queues(void) {
    FileHandle f = file_create(QUEUE_FILE);
    
    file_write_uint32(f, FOURCC('G', 'S', 'Q', '0'));
    file_write(f, &score_queue, sizeof(score_queue));
    file_write(f, score_queue.data, sizeof(QueuedScore) * score_queue.reserved);
    
    file_close(f);
}

static void _close_queues(void) {
    darray_free(&score_queue);
}


void _controllerCallback(void* userData, SC_Error_t completionStatus);


bool _report_score(ScoreSubmitCmd *cmd, const double aResult, const unsigned int aMode, const double aMinorResult, const unsigned int aLevel) {
    SC_ScoreController_h score_controller;
    SC_Score_h score;
    
    cmd->status = SC_STATUS_ERROR;
    cmd->score = score;

    //Step 1
    SCORE_CHECK_ERR(SC_Client_CreateScore(score_client, &score));
    //Step 2
    //aResult is the main numerical result achieved by a user in the game.
    SCORE_CHECK_ERR(SC_Score_SetResult(score, aResult));
    //Step 3
    //aMinorResult is the score result of the game
    SCORE_CHECK_ERR(SC_Score_SetMinorResult(score, aMinorResult));
    //aMode is the mode of the game
    SCORE_CHECK_ERR(SC_Score_SetMode (score, aMode));
    //aLevel is the level in the game
    SCORE_CHECK_ERR(SC_Score_SetLevel (score, aLevel));
    //Step 4
    // client - assumes the handle to the client exists
    // controllerCallback is the callback to be registered
    SCORE_CHECK_ERR(SC_Client_CreateScoreController (score_client, &score_controller, _controllerCallback, cmd));
    //Step 5
    SCORE_CHECK_ERR(SC_ScoreController_SubmitScore(score_controller, score));
    //Mark a request busy if all steps successed.
    cmd->status = SC_STATUS_BUSY;
    ++live_requests;
    
    return SC_OK;
}

// process single element from report score queue:
static void _process_score_queue(void) {
    if(score_queue.size && live_requests == 0) {
        printf("scoreloop: processing %d requests queue.\n", score_queue.size);
        QueuedScore* scores = DARRAY_DATA_PTR(score_queue, QueuedScore);
        const int i = 0; // process only first one
        score_submit.payload = &scores[i];
        _report_score(&score_submit, scores[i].result, scores[i].mode, scores[i].minor_result, scores[i].level);
    }
}

void _controllerCallback(void* userData, SC_Error_t completionStatus) {
    
    --live_requests; int request_type = *(int*)userData;
    assert(live_requests >= 0);

    if (completionStatus != SC_OK) {
        
        if (request_type == SCORE_SUBMIT_ACTION) {
        } else {
            ScoreGetCmd *cmd = (ScoreGetCmd*)userData;
            cmd->status = SC_STATUS_ERROR;
            cmd->error_status = completionStatus;
        };
        fprintf(stderr, "[scoreloop] controllerCallback failed with status: %s.\n", SC_MapErrorToStr(completionStatus));
        
    } else if (request_type == SCORE_SUBMIT_ACTION) {
        
        ScoreSubmitCmd *cmd = (ScoreSubmitCmd*)userData;
        
        // Find score queue item and remove it
        printf("Score reported to SL successfully: %lld", cmd->score);
        if (cmd->payload) {
            QueuedScore* scores = DARRAY_DATA_PTR(score_queue, QueuedScore);
            for(uint i = 0; i < score_queue.size; ++i) {
                if(&scores[i] == cmd->payload) {
                    darray_remove_fast(&score_queue, i); _flush_queues(); // save current report score queue
                    break;
                }
            }
        }
    } else /* SCORE_RETRIVE_ACTION */ {
        
        fprintf(stderr, "[scoreloop] controllerCallback for retrive action.\n");
        
        ScoreGetCmd *cmd = (ScoreGetCmd*)userData;
        
        cmd->list = SC_ScoresController_GetScores(cmd->controller);
        // process scores first:
        if(cmd->callback) cmd->callback(SC_ScoresController_GetMode(cmd->controller), (SCL_ScoreList)cmd);
        // and change status next:
        cmd->status = SC_STATUS_DONE;
        cmd->error_status = completionStatus;
        if (cmd->list) {
            // client - assumes the handle to the client exists
            unsigned int i, numScores = SC_ScoreList_GetCount(cmd->list);
            for (i = 0; i < numScores; ++i) {
                SC_Score_h score = SC_ScoreList_GetAt(cmd->list, i);
                SC_User_h user = SC_Score_GetUser(score);
                SC_String_h login = user ? SC_User_GetLogin(user) : NULL;
                //logging the details
                fprintf(stderr, "Rank: %d, Result: %d, User: %s.\n", SC_Score_GetRank(score), static_cast<int>(SC_Score_GetResult(score)),
                        login ? SC_String_GetData(login) : "<unknown>");
            }
        } else
            fprintf(stderr, "[scoreloop] controllerCallback - SC_ScoresController_GetScores failed.\n");
    }

    _process_score_queue(); // continue with next submission
}

// requires SC creditentials
bool SCL_InitLeadersboard() {
    const char *aGameId       = SC_GameID;
    const char *aGameSecret   = SC_GameSecret;
    const char *aGameCurrency = "JIA";
    const char *aLanguageCode = "en";
    const char *aGameVersion  = "1.0";

    SC_Error_t errCode;
    
    // Initialize the platform adaptation object
    SC_InitData_Init(&initData);

    // Optionally modify the following fields:
    // initData.currentVersion = SC_INIT_CURRENT_VERSION;
    // initData.minimumRequiredVersion = SC_INIT_VERSION_1_0;
    // Create the client.
    // aGameId, aGameSecret and aCurrency are const char strings that you obtain from Scoreloop.
    // aGameVersion should be your current game version.
    // aLanguageCode specifies the language support for localization in awards,
    // for example, "en" for English, which is the default language.
    errCode = SC_Client_New(&score_client, &initData, aGameId, aGameSecret, aGameVersion, aGameCurrency, aLanguageCode);
    if (errCode != SC_OK) {
      fprintf(stderr, "^^ SC_Client_New failed with error code: %d.\n", errCode);
      if (errCode == SC_DEV_PERMISSION_DENIED) {
	// not enough permissions to initiate Scoreloop module
      }
    } else {
#ifdef SDL_HINT_SET_SC_INIT_DATA
      SDL_SetHint(SDL_HINT_SET_SC_INIT_DATA, f_ssprintf("%p", (void *)&initData));
#endif
      _load_queues(); _process_score_queue(); // load queue of scores to be submitted and start submission
    }
    return (SC_OK == errCode);
}

void SCL_CloseLeadersboard() {
    SC_Client_Release(score_client);
    _close_queues();
}

bool SCL_ReportScore(const double aResult, const unsigned int aMode, const double aMinorResult, const unsigned int aLevel) {
    // put new request into the queue:
    QueuedScore qscore;
    qscore.result = aResult;
    qscore.mode = aMode;
    qscore.minor_result = aMinorResult;
    qscore.level = aLevel;
    
    darray_append(&score_queue, &qscore); _flush_queues(); // save current report score queue
    
    if (score_client)
        _process_score_queue();
    
    return true;
}

bool SCL_SubmitScore(const double aResult, const unsigned int aMode, const double aMinorResult, const unsigned int aLevel) {
    if(score_client && score_submit.status!=SC_STATUS_BUSY) { //! we are reusing global submit structure
        score_submit.payload = NULL;
        return _report_score(&score_submit, aResult, aMode, aMinorResult, aLevel);
    } else
        return false;
}

bool SCL_RequestLeadersboard(const unsigned int aMode, SCL_Callback callback) {

    assert(aMode < SC_NUM_MODES);
    
    if (live_requests) {
      fprintf(stderr, "^^ %s: leaderboard %d skipped - %d request(s) in progress.\n", __FUNCTION__, aMode, live_requests);
      return false; // other request in progress
    }
 
    unsigned int aRange;
    SC_Score_h aScore;
    SC_User_h aUser;

    SC_ScoresSearchList_t aSearchList = SC_SCORES_SEARCH_LIST_ALL;

    scores[aMode].status = SC_STATUS_ERROR;
    scores[aMode].callback = callback;

    SCORE_CHECK_ERR(SC_Client_CreateScoresController (score_client, &scores[aMode].controller, _controllerCallback, &scores[aMode]));
    //Setting the search list option
    // aSearchList points to one of the following searchlists:
    // (SC_SCORE_SEARCH_LIST_GLOBAL, SC_SCORE_SEARCH_LIST_24H or SC_SCORE_SEARCH_LIST_USER_COUNTRY)
    SCORE_CHECK_ERR(SC_ScoresController_SetSearchList (scores[aMode].controller, aSearchList));
    SCORE_CHECK_ERR(SC_ScoresController_SetMode(scores[aMode].controller, aMode));
    //Request the scores by using one of the following methods
    // aRange.offset = 0;
    // aRange.length = 20;
    // errCode = SC_ScoresController_LoadScores(scores_controller, aRange);
    // errCode = SC_ScoresController_LoadScoresAroundScore(scores_controller, aScore, aRange);
    SCORE_CHECK_ERR(SC_ScoresController_LoadScoresAtRank(scores[aMode].controller, 1, 10)); // request top 10
    //All steps fine - mark request busy.
    scores[aMode].status = SC_STATUS_BUSY;
    ++live_requests;
    
    return SC_OK;
}

int SCL_GetLeaderboardLiveRequests() {
    return live_requests;
}

SCL_Status SCL_GetLeadersboardStatus(const unsigned int aMode) {
    assert(aMode < SC_NUM_MODES);
    return scores[aMode].status;
}

SCL_ScoreList SCL_GetLeadersboard(const unsigned int aMode) {
    assert(aMode < SC_NUM_MODES);
    return (SCL_ScoreList*)&scores[aMode];
}

int SCL_GetLeadersboardCount(const SCL_ScoreList score_list) {
    ScoreGetCmd *cmd = (ScoreGetCmd*)score_list;
    return SC_ScoreList_GetCount(cmd->list);
}

double SCL_GetLeadersboardResultAt(const SCL_ScoreList score_list, int pos) {
    ScoreGetCmd *cmd = (ScoreGetCmd*)score_list;
    SC_Score_h score = SC_ScoreList_GetAt(cmd->list, pos);
    return SC_Score_GetResult(score);
}

const char *SCL_GetLeadersboardUserAt(const SCL_ScoreList score_list, int pos) {
    ScoreGetCmd *cmd = (ScoreGetCmd*)score_list;
    SC_Score_h score = SC_ScoreList_GetAt(cmd->list, pos);
    SC_User_h user = SC_Score_GetUser(score);
    SC_String_h login = user ? SC_User_GetLogin(user) : NULL;
    return login ? SC_String_GetData(login) : "<?>";
}

const char *SCL_GetLeadersboardEmailAt(const SCL_ScoreList score_list, int pos) {
    ScoreGetCmd *cmd = (ScoreGetCmd*)score_list;
    if (cmd->list) {
        SC_Score_h score = SC_ScoreList_GetAt(cmd->list, pos);
        SC_User_h user = SC_Score_GetUser(score);
        SC_String_h email = user ? SC_User_GetEmail(user) : NULL;
        return email ? SC_String_GetData(email) : "n/a";
    }
    return "?";
}

int SCL_GetLeadersboardRankAt(const SCL_ScoreList score_list, int pos) {
    ScoreGetCmd *cmd = (ScoreGetCmd*)score_list;
    if (cmd->list) {
        SC_Score_h score = SC_ScoreList_GetAt(cmd->list, pos);
        return SC_Score_GetRank(score);
    }
    return 0;
}

bool SCL_IsMyLeadersboardAt(const SCL_ScoreList score_list, int pos) {
    ScoreGetCmd *cmd = (ScoreGetCmd*)score_list;
    if (cmd->list) {
        SC_Score_h score = SC_ScoreList_GetAt(cmd->list, pos);
        return SC_User_Equals( SC_Session_GetUser(SC_Client_GetSession(score_client)), SC_Score_GetUser(score) );
    }
    return false;
}

bool SCL_SetLeadersboardDirty(const unsigned int aMode) {
    assert(aMode < SC_NUM_MODES);
    if (scores[aMode].status != SC_STATUS_BUSY) {
        scores[aMode].status = SC_STATUS_DIRTY;
        return true;
    } else
        return false;
}
