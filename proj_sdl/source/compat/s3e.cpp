#include "s3e.h"
#include "IwGx.h"
#include "Iw2D.h"
#include <SDL/SDL_opengl.h>
#include <string>
#include <stb_image.h>
#include <stdio.h>
#include <unistd.h>

const float FColorRGB = 1./255.;

#define ABGR_RED2F(c)      (((c)&0x000000ff)*FColorRGB)
#define ABGR_GREEN2F(c)    ((((c)>>8)&0x000000ff)*FColorRGB)
#define ABGR_BLUE2F(c)	   ((((c)>>16)&0x000000ff)*FColorRGB)
#define ABGR_ALPHA2F(c)    ((((c)>>24)&0x000000ff)*FColorRGB)

const char *resourceRoot () {

#ifdef __QNXNTO__
	static char cwd[FILENAME_MAX] = { 0 };

	if (!*cwd) {
	  getcwd(cwd, FILENAME_MAX - 1);
	  strcat(cwd, "/app/native/data/");
	}
	
	return cwd;
#else
	return "";
#endif
}

const char *resourcePath (const char *filename) {

  static char path[FILENAME_MAX] = { 0 };
  
  if (filename) {
    snprintf(path, FILENAME_MAX, "%s%s", resourceRoot(), filename); return path;
  } else
    return resourceRoot();
}

const char *writePath (const char *file) {

#if defined __QNXNTO__
	// Let's write it in the current working directory's data folder
	static char cwd[FILENAME_MAX] = { 0 };
	static char path[FILENAME_MAX] = { 0 };

	if (!*cwd) {
		getcwd(cwd, FILENAME_MAX - 1);
		strcat(cwd, "/data/");
	}

	if (file) {
		snprintf(path, FILENAME_MAX, "%s%s", cwd, file); return path;
	} else
		return cwd;
#else
	return file;
#endif
}

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
    unsigned char *idata = stbi_load( resourcePath(from_file), &width, &height, &channels, force_channels );
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

void Iw2DClearScreen(const uint32 color) {
  glClearColor(ABGR_RED2F(color), ABGR_GREEN2F(color), ABGR_BLUE2F(color), ABGR_ALPHA2F(color));
  glClear(GL_COLOR_BUFFER_BIT);
}

void Iw2DFinishDrawing() {
  SDL_GL_SwapBuffers();
}
