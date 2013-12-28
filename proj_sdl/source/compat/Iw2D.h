#ifndef __compat_iw2d_h__
#define __compat_iw2d_h__

#include "s3e.h"
#include "IwColour.h"
#include "IwFColour.h"
#include "IwGx.h"

class CIw2DImage
{
 public:
    virtual float GetWidth() = 0;
    virtual float GetHeight() = 0;
};

enum CIw2DFontAlign
{
    IW_2D_FONT_ALIGN_TOP,
    IW_2D_FONT_ALIGN_LEFT,
    IW_2D_FONT_ALIGN_CENTRE,
    IW_2D_FONT_ALIGN_BOTTOM,
    IW_2D_FONT_ALIGN_RIGHT,
};

class CIw2DFont {
  // --
};

inline static void Iw2DInit() { }
inline static void Iw2DTerminate() { }
inline static void Iw2DSurfaceShow() { }
void Iw2DFinishDrawing();
// always full screen
uint32 Iw2DGetSurfaceHeight();
uint32 Iw2DGetSurfaceWidth();
void Iw2DSetTransformMatrix(const CIwMat2D &m);
void Iw2DDrawString(const char* text, CIwFVec2 topLeft, CIwFVec2 size, CIw2DFontAlign horzAlign, CIw2DFontAlign vertAlign);
void Iw2DDrawImage(CIw2DImage* image, CIwFVec2 topLeft, CIwFVec2 size);
void Iw2DSetFont(const CIw2DFont *f);
void Iw2DSetColour(const uint32 color);
#ifdef __S3E__
inline static void Iw2DClearScreen(const uint32 color) {
  Iw2DSetColour(IGDistorter::getInstance()->colorBlackInt);
  Iw2DFillRect(CIwSVec2(0,0), CIwSVec2(Iw2DGetSurfaceWidth(), Iw2DGetSurfaceHeight()));
}
#else
void Iw2DClearScreen(const uint32 color);
#endif
CIw2DImage* Iw2DCreateImageResource(const char* resource);
CIw2DFont* Iw2DCreateFontResource(const char* resource);

#endif /* __compat_iw2d_h__ */
