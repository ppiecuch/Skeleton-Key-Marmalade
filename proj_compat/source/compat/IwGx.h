#ifndef __compat_iwgx_h__
#define __compat_iwgx_h__

#include "s3e.h"
#include "IwColour.h"
#include "IwFColour.h"

#include "AffineTransform.h"

template<typename T> class CIwVec2
{
public:
    T x;
    T y;

    CIwVec2(T _x, T _y) : x(_x), y(_y) {};
    CIwVec2() : x(0), y(0) {};
    CIwVec2(const CIwVec2<float> &v) : x(v.x), y(v.y) {};
    CIwVec2(const CIwVec2<int> &v) : x(v.x), y(v.y) {};
    CIwVec2(const CIwVec2<int16> &v) : x(v.x), y(v.y) {};
};

typedef CIwVec2<int> CIwIVec2;
typedef CIwVec2<int16> CIwSVec2;
typedef CIwVec2<float> CIwFVec2;

typedef affinetransform::AffineTransformT<float> AffineTransform;

class CIwMat2D : public AffineTransform
{
 public:
  CIwMat2D() { }
  CIwMat2D(const AffineTransform &m) : AffineTransform(m) { }
  CIwMat2D &SetIdentity() { identity(); return *this; }
  CIwMat2D &Scale(float sc) { scale(sc); return *this; }
  CIwMat2D &Rotate(float rt) { rotate(rt); return *this; }
  CIwMat2D &Rotate(float rt, float cx, float cy) { rotate(rt, cx, cy); return *this; }
  CIwMat2D Rotation(float rt, float cx, float cy) const { return CIwMat2D(rotation(rt, cx, cy)); }
  CIwMat2D &Translate(float mx, float my) { translate(mx, my); return *this; }

  static const CIwMat2D Identity;

};

#endif /* __compat_iwgx_h__ */
