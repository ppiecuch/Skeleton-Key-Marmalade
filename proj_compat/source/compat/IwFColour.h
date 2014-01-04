#pragma once

/*
 * (C) 2001-2012 Marmalade. All Rights Reserved.
 *
 * This document is protected by copyright, and contains information
 * proprietary to Marmalade.
 *
 * This file consists of source code released by Marmalade under
 * the terms of the accompanying End User License Agreement (EULA).
 * Please do not use this program/source code before you have read the
 * EULA and have agreed to be bound by its terms.
 */
 
//-----------------------------------------------------------------------------

#ifndef IW_FCOLOUR_H
#define IW_FCOLOUR_H


class CIwColour;

class CIwFColour
{
    float r;
    float g;
    float b;
    float a;
public:
    CIwFColour() {}
    CIwFColour(float r, float g, float b, float a=1.0f) : r(r), g(g), b(b), a(a) {}
    CIwFColour(const CIwColour &col);

    /** Red. */
    static const CIwFColour    red;

    /** Green. */
    static const CIwFColour    green;

    /** Blue. */
    static const CIwFColour   blue;

    /** White. */
    static const CIwFColour    white;

    /** Black. */
    static const CIwFColour    black;

    /** Yellow. */
    static const CIwFColour    yellow;
    
    /** Cyan. */
    static const CIwFColour    cyan;

    /** Magenta. */
    static const CIwFColour    magenta;
    
    /** Gray. */
    static const CIwFColour    gray;
    
    /** Grey. */
    static const CIwFColour    grey;
    
    /** Clear. */
    static const CIwFColour    clear;

    /** */
    static CIwFColour Lerp(const CIwFColour &start, const CIwFColour &end, const float factor);

    inline CIwFColour operator * (const CIwFColour &other) const
    {
        // do we need to clamp the values?
        return CIwFColour(r*other.r, g*other.g, b*other.b, a*other.a );
    }
    inline CIwFColour& operator *= (const CIwFColour &other)
    {
        r *= other.r;
        g *= other.g;
        b *= other.b;
        a *= other.a;

        return (*this);
    }
    inline CIwFColour operator * (const float &s) const
    {
        return CIwFColour(r*s, g*s, b*s, a*s );
    }
    inline CIwFColour& operator *= (const float &s)
    {
        r *= s;
        g *= s;
        b *= s;
        a *= s;

        return (*this);
    }
    float GetGrayscale() const
    {
        // Use the Y term of YUV conversion (better approximation to percieved brightness than averaging R,G,B)
        return r*0.299f + g*0.587f + b*0.114f;
    }


    operator CIwColour() const;
};

inline CIwFColour operator * (const float &s, const CIwFColour &col)
{
    return col * s;
}

#endif // !IW_FCOLOUR_H
