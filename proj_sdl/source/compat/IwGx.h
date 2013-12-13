#ifndef __compat_iwgx_h__
#define __compat_iwgx_h__

#include "s3e.h"
#include "IwColour.h"
#include "IwFColour.h"

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

class CIwMat2D
{
 public:
  void SetIdentity();
  void Scale(float sc);
};

#endif /* __compat_iwgx_h__ */
