#ifndef __compat_iw2d_h__
#define __compat_iw2d_h__

#include "s3e.h"
#include "IwColour.h"
#include "IwFColour.h"

class CIwMat2D
{
};

class CIw2DImage
{
 public:
    float GetWidth();
    float GetHeight();
    CIwMat2D* GetMaterial();
    ~CIw2DImage() {};
};

enum CIw2DFontAlign
{
    IW_2D_FONT_ALIGN_TOP,
    IW_2D_FONT_ALIGN_LEFT,
    IW_2D_FONT_ALIGN_CENTRE,
    IW_2D_FONT_ALIGN_BOTTOM,
    IW_2D_FONT_ALIGN_RIGHT,
};

class CIw2DFont
{
};

void Iw2DInit();
void Iw2DTerminate();
void Iw2DSurfaceShow();

#endif /* __compat_iw2d_h__ */
