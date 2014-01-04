#include "s3e.h"
#include "IwGx.h"
#include "Iw2D.h"
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

#if defined __QNXNTO__
# include <bps/bps.h>
# include <bps/navigator.h>
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
  return 1024;
  #else
  #error Cannot determine device.
  #endif
}

static uint32 _GetDevHeight() {
  #if defined __BB10__
  return 768;
  #elif defined __PLAYBOOK__
  return 600;
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
static void _setupLetterBox() {
  float targetAspectRatio = virtual_width/virtual_height;
 
  // figure out the largest area that fits in this resolution at the desired aspect ratio
  int width = screen_width;
  int height = (int)(width / targetAspectRatio + 0.5f);

  if (height > screen_height ) {
    //It doesn't fit our height, we must switch to pillarbox then
    height = screen_height ;
    width = height * targetAspectRatio + 0.5f;
  }
 
  // set up the new viewport centered in the backbuffer
  const int vp_x = (screen_width  / 2) - (width / 2);
  const int vp_y = (screen_height / 2) - (height/ 2);
 
  glViewport(vp_x, vp_y, width,height);

  /// Now that our viewport is set we should set our 2d perspective
 
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, screen_width, screen_height, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
}

static void _beginLetterBox() {
  
  /// So now we should push the transformations before actually drawing anything
  
  // Push in scale transformations
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glScalef(scale_x, scale_y, 1.0f);
}

static void _endLetterBox() {
  // This pops those matrices for the scale transformations.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glPopMatrix();
  // Now to finish we should end our 2D perspective
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();   
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
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
	const int x = event.button.x, y = _GetDevHeight() - event.button.y;
      }; break;
    case SDL_MOUSEBUTTONUP:
      {
	const int x = event.button.x, y = _GetDevHeight() - event.button.y;
      }; break;
    case SDL_MOUSEMOTION: 
      {
	const int x = event.button.x, y = _GetDevHeight() - event.button.y;
      }; break;
    }
}

void s3eKeyboardUpdate() {   keys = SDL_GetKeyState(NULL); }
s3eEnum s3eKeyboardGetState(s3eKeys key) { }
s3eBool s3eDeviceCheckQuitRequest() { return done; }

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

class CcIw2DFont : public CIw2DFont
{
  int num;
  uint16 asciimap[256];
  map<uint, uint16> utf8map;
  const CIwGxFont &descr;
  uint texture;
  float maxs, maxt;
  float char_w, char_h;
public:
  CcIw2DFont(const CIwGxFont &font) : texture(0), maxs(0), maxt(0), num(0), char_w(0), char_h(0), descr(font)
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

    int width, height, channels;
    const int force_channels = 0;

    // try to read raw data:
    const char * path = resourceExists(font.image)?resourcePath(font.image):resourcePath(f_ssprintf("fonts/%s", font.image));
    unsigned char *idata = stbi_load( path, &width, &height, &channels, force_channels );
    if( idata == NULL ) {
      fprintf(stderr, "[CcIw2DFont] Failed to get raw data from the file %s - image not supported or not an image (%s).\n", 
	      path, stbi_failure_reason());
      return;
    }

    // create texture:
    maxs = (float)width/(float)_nextpot(width);
    maxt = (float)height/(float)_nextpot(height);

    char_w = (float)width/(float)num;
    char_h = height;

    texture = SOIL_create_OGL_texture(idata, width, height, channels, SOIL_CREATE_NEW_ID, SOIL_FLAG_POWER_OF_TWO|SOIL_FLAG_MULTIPLY_ALPHA);

    printf("CcIw2DFont image %s: texture %d, dim. %dx%d, tex. %fx%f\n", font.image, texture, width, height, maxs, maxt);
  }

  uint GetTexture() const { return texture; }
  float GetMaxS() const { return maxs; }
  float GetMaxT() const { return maxt; }
  float GetCharW() const { return char_w; }
  float GetCharH() const { return char_h; }
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
    "levels.db",
    "saved_game.db",
    "settings.db", NULL };
  for(char **db = (char **)_sqdb; *db != NULL; db++) {
    if (resourceExists(*db) && !_fileExists(writePath(*db))) {
      fprintf(stderr, "*** Copying to writeable location: %s.\n", *db);
      if (_cp(writePath(*db), resourcePath(*db)) < 0)
	fprintf(stderr, "*** Failed to copy %s.\n*** from: %s\n*** to: %s\n", *db, resourcePath(*db), writePath(*db));
    }
  }
}

// ----- Iw2D -----

class CcIw2DImage : public CIw2DImage
{
private:
  CIwIVec2 size;
  std::string file;
  std::string error;
  uint texture;
  float maxt, maxs;
public:
  CcIw2DImage(const char* from_file) : texture(0), maxs(0), maxt(0) {

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

    size.x = width;
    size.y = height;

    maxs = (float)width/(float)_nextpot(width);
    maxt = (float)height/(float)_nextpot(height);

    texture = SOIL_create_OGL_texture2(idata, width, height, channels, SOIL_CREATE_NEW_ID, SOIL_FLAG_POWER_OF_TWO|SOIL_FLAG_MULTIPLY_ALPHA);

    printf("CcIw2DImage image %s: texture %d, dim. %dx%d, tex. %fx%f\n", from_file, texture, width, height, maxs, maxt);
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
};

CIw2DImage* Iw2DCreateImageResource(const char* resource)
{
  const char *_search[] = { "", "graphics/", "graphics/menu/", "graphics/backgrounds/", NULL };
  const char *path = NULL;
  for(char **p=(char**)_search; *p != NULL; p++)
    if (resourceExists(f_ssprintf("%s%s.png", *p, resource))) {
      path = resourcePath(f_ssprintf("%s%s.png", *p, resource));
      break;
    }
  if (path)
    return new CcIw2DImage(path);
  else
    return NULL;
}

CIw2DFont* Iw2DCreateFontResource(const char* resource)
{
  if (_fonts.count(resource)) {
    return _fonts[resource];
  }
  fprintf(stderr, "*** Font resource %s not found.\n", resource);
  return NULL;
}

uint32 Iw2DGetSurfaceHeight() { return _GetDevHeight(); }

uint32 Iw2DGetSurfaceWidth() { return _GetDevWidth(); }

void Iw2DSetTransformMatrix(const CIwMat2D &m) {
  _current_matrix = m;
  glMultMatrixf(AffineTransform::matrix(_current_matrix)());
}

void Iw2DSetFont(const CIw2DFont *f) {
  _current_font = (CcIw2DFont*)f;
}

void Iw2DSetColour(const uint32 color) {
  _current_color = color;
}

void Iw2DDrawString(const char* text, CIwFVec2 topLeft, CIwFVec2 size, CIw2DFontAlign horzAlign, CIw2DFontAlign vertAlign) {

  float x = topLeft.x;
  float y = topLeft.y;
  const float w = _current_font->GetCharW();
  const float h = _current_font->GetCharH();
  const float uwid = _current_font->GetMaxS()/_current_font->GetNumChars();
  const float vwid = _current_font->GetMaxT();

  string line = text;

  const string::iterator end_it = utf8::find_invalid(line.begin(), line.end());
  if (end_it != line.end()) {
    fprintf( stderr, "[Iw2DDrawString] Invalid UTF-8 encoding detected.\n" );
    fprintf( stderr, "[Iw2DDrawString] This part is fine and will be processed: %s.\n", string(line.begin(), end_it).c_str() );
  }
  utf8::iterator<string::iterator> it (line.begin(), line.begin(), end_it);
  int index = 0; while(it.base()!=end_it) {
    uint32_t ch = *it;

    float uofs = 0;
    float vofs = 0;

    struct _v2c4 {
      GLfloat v[2];
      GLfloat t[2];
      uint32_t c;
    } vertices[] = {
      { {x, y}, {uofs, vofs}, _current_color },
      { {x,y+h}, {uofs, vofs + vwid}, _current_color },
      { {x+w,y}, {uofs + uwid, vofs}, _current_color },
      { {x+w,y+h}, {uofs + uwid, vofs + vwid}, _current_color },
    };
    glBindTexture(GL_TEXTURE_2D, _current_font->GetTexture());
    glVertexPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->v);
    glTexCoordPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->t);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(_v2c4), &vertices->c);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }
}

void Iw2DDrawImage(CIw2DImage* image, CIwFVec2 topLeft, CIwFVec2 size) {

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
    { {x,y+h}, {uofs, vofs + vwid}, _current_color },
    { {x+w,y}, {uofs + uwid, vofs}, _current_color },
    { {x+w,y+h}, {uofs + uwid, vofs + vwid}, _current_color },
  };
  glBindTexture(GL_TEXTURE_2D, img->GetTexture());
  glVertexPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->v);
  glTexCoordPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->t);
  glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(_v2c4), &vertices->c);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); 
}

void Iw2DClearScreen(const uint32 color) {
  glClearColor(ABGR_RED2F(color), ABGR_GREEN2F(color), ABGR_BLUE2F(color), ABGR_ALPHA2F(color));
  glClear(GL_COLOR_BUFFER_BIT);
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
   Uint32 bits = 32, flags = SDL_OPENGL, width =  _GetDevWidth(), height = _GetDevHeight();

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
