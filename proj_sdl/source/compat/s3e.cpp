#include "s3e.h"
#include "IwGx.h"
#include "Iw2D.h"
#include <stdio.h>
#include <unistd.h>

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
  CIwIVec2 pos;
  CIwIVec2 size;
  CIwIVec2 anchor;
public:
  virtual float GetWidth() { return size.x; }
  virtual float GetHeight()  { return size.y; }
  virtual CIwMat2D* GetMaterial();
  virtual ~CcIw2DImage() {};
};

