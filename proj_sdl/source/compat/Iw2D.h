#ifndef __compat_iw2d_h__
#define __compat_iw2d_h__

#include "s3e.h"
#include "IwColour.h"
#include "IwFColour.h"
#include "IwGx.h"

class CIw2DImage
{
 public:
    virtual float GetWidth();
    virtual float GetHeight();
    virtual CIwMat2D* GetMaterial();
    virtual ~CIw2DImage() {};
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
void Iw2DFinishDrawing();
uint32 Iw2DGetSurfaceHeight();
uint32 Iw2DGetSurfaceWidth();
void Iw2DSetTransformMatrix(const CIwMat2D &m);
void Iw2DDrawString(const char* string, CIwFVec2 topLeft, CIwFVec2 size, CIw2DFontAlign horzAlign, CIw2DFontAlign vertAlign);
void Iw2DDrawImage(CIw2DImage* image, CIwFVec2 topLeft, CIwFVec2 size);
void Iw2DSetFont(const CIw2DFont *f);
void Iw2DSetColour(const uint32 color);
// only for full size clearing:
void Iw2DFillRect(CIwFVec2 topLeft, CIwFVec2 size);

CIw2DImage* Iw2DCreateImageResource(const char* resource);
CIw2DFont* Iw2DCreateFontResource(const char* resource);

#endif /* __compat_iw2d_h__ */
