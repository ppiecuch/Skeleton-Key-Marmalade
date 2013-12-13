#ifndef __compat_s3e_h__
#define __compat_s3e_h__

#if defined __QNXNTO__
# define I3D_OS_QNX 1
#endif

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_events.h>
#include <SDL/SDL_timer.h>

#ifndef MIN
    #define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
    #define MAX(a,b) (((a)>(b))?(a):(b))
#endif

inline static float ABS(float v) { return fabs(v); }
inline static int ABS(int v) { return abs(v); }

#include "s3eTypes.h"
#include "s3ePointer.h"

enum s3eEnum {
  S3E_DEVICE_PAUSE,
  S3E_DEVICE_UNPAUSE,
  // --
  S3E_KEY_STATE_RELEASED,
  // --
  S3E_AUDIO_CODEC_MIDI,
  S3E_AUDIO_CODEC_MP3,
  S3E_AUDIO_CODEC_AAC,
  S3E_AUDIO_CODEC_AACPLUS,
  S3E_AUDIO_CODEC_QCP,
  S3E_AUDIO_CODEC_PCM,
  S3E_AUDIO_CODEC_SPF,
  S3E_AUDIO_CODEC_AMR,
  S3E_AUDIO_CODEC_MP4,
  // --
  S3E_SOUND_DEFAULT_FREQ,
  S3E_SOUND_VOLUME,
  S3E_AUDIO_VOLUME,
  S3E_IOSBACKGROUNDMUSIC_PLAYBACK_STATE,
  S3E_IOSBACKGROUNDMUSIC_PLAYBACK_PLAYING,
  S3E_IOSBACKGROUNDMUSIC_PLAYBACK_INTERRUPTED
};

enum s3eKeys {
  s3eKeyBack
};

#define IW_FIXED(v) (v)

void s3eDebugOutputString(const char *fmt, ...);

void* s3eMallocBase(int size);
void s3eFreeBase(void* item);

typedef FILE s3eFile;

s3eFile* s3eFileOpen(const char* filename, const char* mode);
s3eResult s3eFileClose(s3eFile* file);
uint32 s3eFileRead(void* buffer, uint32 elemSize, uint32 noElems, s3eFile* file);
uint32 s3eFileWrite(const void* buffer, uint32 elemSize, uint32 noElems, s3eFile* file);
int32 s3eFileGetSize(s3eFile* file);

inline static uint32 s3eTimerGetMs() { return SDL_GetTicks(); }
inline static void s3eDeviceYield(int ms) { }
inline static s3eResult s3eConfigGetString(const char *conf, const char *sect, char *buffer) {
  return S3E_RESULT_SUCCESS;
}
inline static s3eResult s3eDeviceRegister(s3eEnum ev, s3eCallback fn, void* userdata) { }
inline static s3eResult s3eDeviceUnRegister(s3eEnum ev, s3eCallback fn) { }

inline static void s3eKeyboardUpdate() { }
inline static s3eEnum s3eKeyboardGetState(s3eKeys key) { }
inline static s3eBool s3eDeviceCheckQuitRequest() { return false; }

inline static s3eBool s3eExtOSExecAvailable() { return false; }
inline static s3eResult s3eOSExecExecute(const char* url, s3eBool exit) { return S3E_RESULT_ERROR; }
inline static s3eBool s3eIOSBackgroundMusicAvailable() { return true; }
int32 s3eIOSBackgroundMusicGetInt(s3eEnum property);
s3eResult s3eIOSBackgroundMusicPlay();
s3eBool s3eIOSBackgroundAudioIsPlaying();
s3eBool s3eIOSBackgroundAudioSetMix( s3eBool mix );

inline static void s3eAccelerometerStart() { }
inline static void s3eAccelerometerStop() { }
inline static int32 s3eAccelerometerGetX() { return 0; }
inline static int32 s3eAccelerometerGetY() { return 0; }
inline static int32 s3eAccelerometerGetZ() { return 0; }

s3eBool s3eAudioIsCodecSupported(s3eEnum codec);
s3eResult s3eAudioPlay(const char* filename, uint32 repeatCount S3E_DEFAULT(1));
s3eResult s3eAudioPause();
s3eResult s3eAudioResume();
void s3eAudioStop();
s3eBool s3eAudioIsPlaying();
void s3eAudioSetInt(s3eEnum f, int v);

s3eResult s3eSoundChannelPlay(int channel, int16* start, uint32 numSamples, int32 repeat, int32 loopfrom);
int s3eSoundGetFreeChannel();
const char* s3eSoundGetErrorString();
void s3eSoundSetInt(s3eEnum f, int v);

const char *resourceRoot ();
const char *resourcePath (const char *filename);
const char *writePath (const char *file);

#endif /* __compat_s3e_h__ */
