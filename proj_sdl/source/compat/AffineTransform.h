#pragma once

#include <math.h>

namespace scalar
{
    inline static float sin(float a) { return ::sinf(a); }
    inline static double sin(double a) { return ::sin(a); }
    inline static float cos(float a) { return ::cosf(a); }
    inline static double cos(double a) { return ::cos(a); }
}

namespace affinetransform
{
  namespace details
  {
    template<typename T> struct vector2
    {
      T x, y;
    };

    template<typename T> struct matrix4
    {
      T m[16];
      matrix4(T c0, T c4, T c8, T c12,
	      T c1, T c5, T c9, T c13,
	      T c2, T c6, T c10, T c14,
	      T c3, T c7, T c11, T c15
	      ) {
	m[0] = c0;
	m[1] = c1;
	m[2] = c2;
	m[3] = c3;
	m[4] = c4;
	m[5] = c5;
	m[6] = c6;
	m[7] = c7;
	m[8] = c8;
	m[9] = c9;
	m[10] = c10;
	m[11] = c11;
	m[12] = c12;
	m[13] = c13;
	m[14] = c14;
	m[15] = c15;
      }
      const T *operator ()() const { return m; } // return pointer to the matrix array
    };
  }

  template<class T, class Vector2 = details::vector2<T>, class Matrix4 = details::matrix4<T> >
	class AffineTransformT
	{
	public:

		typedef Vector2 vector2;
		typedef Matrix4 matrix;
		typedef AffineTransformT<T> affineTransform;

		AffineTransformT(){}
		AffineTransformT(T a_, T b_, T c_, T d_, T x_, T y_):a(a_), b(b_), c(c_), d(d_), x(x_), y(y_){}


		explicit AffineTransformT(const matrix &m)
		{
			a = m.m[0];
			b = m.m[1];
			c = m.m[4];
			d = m.m[5];
			x = m.m[12];
			y = m.m[13];			
		}

		void identity()
		{
			a = T(1);
			b = T(0);
			c = T(0);
			d = T(1);
			x = T(0);
			y = T(0);
		}

		void translate(const vector2 &v)
		{
			x += a * v.x + c * v.y;
			y += b * v.x + d * v.y;
		}

		void scale(const vector2 &v)
		{
			a *= v.x;
			b *= v.x;
			c *= v.y; 
			d *= v.y;
		}

		void scale(const T v)
		{
			a *= v;
			b *= v;
			c *= v; 
			d *= v;
		}

		void rotate(T v)
		{
			T sin_ = scalar::sin(v);
			T cos_ = scalar::cos(v);

			affineTransform rot(cos_, sin_, -sin_, cos_, 0, 0);
			*this = *this * rot;
		}

		void invert()
		{
			affineTransform t = *this;

			T det = T(1) / (t.a * t.d - t.b * t.c);

			a = det * t.d;
			b = -det * t.b;
			c = -det * t.c;
			d = det * t.a;
			x = det * (t.c * t.y - t.d * t.x);
			y = det * (t.b * t.x - t.a * t.y);
		}

		operator matrix() const
		{
			return matrix
				(
				a, b, 0, 0,
				c, d, 0, 0,
				0, 0, 1, 0,
				x, y, 0, 1
				);
		}



		affineTransform operator * (const affineTransform &t2) const
		{
			affineTransform r;
			const affineTransform &t1 = *this;
			r.a = t1.a * t2.a + t1.b * t2.c;
			r.b = t1.a * t2.b + t1.b * t2.d;
			r.c = t1.c * t2.a + t1.d * t2.c;
			r.d = t1.c * t2.b + t1.d * t2.d;
			r.x = t1.x * t2.a + t1.y * t2.c + t2.x;
			r.y = t1.x * t2.b + t1.y * t2.d + t2.y;
			
			return r;
		}

		vector2 transform (const vector2 &v) const
		{
			return vector2(
				a * v.x + c * v.y + x, 
				b * v.x + d * v.y + y);
		}


		T a, b, c, d;
		T x, y;
	};

	typedef AffineTransformT<float> AffineTransform;
}
