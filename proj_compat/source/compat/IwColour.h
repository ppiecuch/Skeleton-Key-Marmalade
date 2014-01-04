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

#ifndef IW_COLOUR_H
#define IW_COLOUR_H

// Includes
#include "s3eTypes.h"

#ifdef __ARMCC_VERSION
    #pragma anon_unions
#endif

// Forward declarations

//-----------------------------------------------------------------------------
// Helper macros
//-----------------------------------------------------------------------------
#define IW_COLOUR_565_TO_555(a) \
        ((((a) >> 1) & 0x7fe0) + ((a) & 0x1f))

#define IW_COLOUR_8888_TO_1555(r,g,b,a) \
        (((r>>3)| ((g >>3)<<5) | ((b>>3)<<10)) | (a ? 0x8000 : 0))

typedef uint16 IwPixel;
#define IW_RGB(r,g,b) (IwPixel)(((r)>>3<<11) | ((g)>>2<<5) | ((b)>>3))

#define IW_COLOUR_MUL_ABC_SHIFT(c, c1, c2, s) \
        (c).r = ((c1).r * (c2).r) >> (s); \
        (c).g = ((c1).g * (c2).g) >> (s); \
        (c).b = ((c1).b * (c2).b) >> (s); \
        (c).a = ((c1).a * (c2).a) >> (s)

#define IW_COLOUR_MUL_SHIFT_R(c1, c2, s)    (((c1).r * (c2).r) >> (s))
#define IW_COLOUR_MUL_SHIFT_G(c1, c2, s)    (((c1).g * (c2).g) >> (s))
#define IW_COLOUR_MUL_SHIFT_B(c1, c2, s)    (((c1).b * (c2).b) >> (s))
#define IW_COLOUR_MUL_SHIFT_A(c1, c2, s)    (((c1).a * (c2).a) >> (s))

/**
 * @addtogroup iwgxgroup
 * @{
 */


/**
 * @defgroup colours Colours
 *
 * @note For more information on IwGx Colour, see the
 * @ref colour "Colour" section of the <i>IwGx API Documentation</i>.
 * @{
 */

//-----------------------------------------------------------------------------
// CIwColour
//-----------------------------------------------------------------------------
/**
 * Colour object.
 * See the @ref colour "Colour" module overview for more information.
 *
 * @autoexp{&lt;r\,x&gt;\,&lt;g,x&gt;\,&lt;b,x&gt;\,&lt;a,x&gt;}
 * @par Required Header Files
 * IwColour.h
 */
class CIwColour
{
public:

    /**
     * Sets all 4 components from a single input.
     * @param rgba 32-bit value to set.
     * @see SetOpaque, SetGrey
     * @par Required Header Files
     * IwColour.h
     */
    void Set(uint32 rgba)
    {
#ifdef IW_ENDIAN_BIG
        r = (rgba ) & 0xff;
        g = (rgba >> 8) & 0xff;
        b = (rgba >> 16) & 0xff;
        a = (rgba >> 24) & 0xff;
#else
        *(uint32*)this = rgba;
#endif
    }

    /**
     * Sets all 4 components.
     * @param _r 8-bit r value to set.
     * @param _g 8-bit g value to set.
     * @param _b 8-bit b value to set.
     * @param _a 8-bit a value to set.
     * @see SetOpaque, SetGrey
     * @par Required Header Files
     * IwColour.h
     */
    inline void Set(uint8 _r, uint8 _g, uint8 _b, uint8 _a)
    {
        r = _r;
        g = _g;
        b = _b;
        a = _a;
    }

    /**
     * Sets r,g,b components with full opacity.
     * @param _r 8-bit r value to set.
     * @param _g 8-bit g value to set.
     * @param _b 8-bit b value to set.
     * @see SetOpaque, SetGrey
     * @par Required Header Files
     * IwColour.h
     */
    inline void Set(uint8 _r, uint8 _g, uint8 _b)
    {
        r = _r;
        g = _g;
        b = _b;
        a = 0xff;
    }

    /**
     * Sets all 4 components from a single input.
     * @param col Colour structure to set.
     * @see SetOpaque, SetGrey
     * @par Required Header Files
     * IwColour.h
     */
    inline void Set(CIwColour other) { *(uint32*)this = *(uint32*)&other; }

    /**
     * Gets the colour as a uint32 value.
     * @see Set
     * @par Required Header Files
     * IwColour.h
     */
    inline uint32 Get() const
    {
#ifdef IW_ENDIAN_BIG
        return  ((uint32)r) |
                ((uint32)g << 8) |
                ((uint32)b << 16) |
                ((uint32)a << 24);
#else
        return *(uint32*)this;
#endif
    }

    /**
     * Sets all 4 components from a single input, and make alpha value fully opaque.
     * @param rgba 24-bit value to set.
     * @see Set, SetGrey
     * @par Required Header Files
     * IwColour.h
     */
    inline void SetOpaque(uint32 rgb)
    {
        Set(rgb | (uint32)0xff << 24);
    }

    /**
     * Sets all 4 components from a single input, by specifying a grayscale value.
     * @param g Greyscale value to set (0..255).
     * @see Set, SetOpaque
     * @par Required Header Files
     * IwColour.h
     */
    inline void SetGrey(uint8 g) { SetOpaque(g | g << 8 | g << 16); }

    //-----------------------------------------------------------------------------
    // Operator overloads
    //-----------------------------------------------------------------------------
    /**
     * Sets all 4 components from a single input.
     * @param rgba 32-bit colour value to set.
     * @result Reference to the colour object itself.
     */
    CIwColour operator= (uint32 rgba)
    {
        Set(rgba);
        return *this;
    }

    /**
     * Tests if colour is equivalent to another 32-bit value.
     * @param col 32-bit colour value to test against.
     * @result true only if the values are equivalent.
     */
    bool operator == (uint32 rgba) const
    {
        return Get() == rgba;
    }

    /**
     * Tests if colour is equivalent to another CIwColour object.
     * @param other CIwColour object to test against.
     * @result true only if the values are equivalent.
     */
    bool operator == (const CIwColour& other) const
    {
        return *(uint32*)this == *(uint32*)&other;
    }

    /**
     * Tests if colour is not equivalent to another 32-bit value.
     * @param col 32-bit colour value to test against.
     * @result true only if the values are not equivalent.
     */
    bool operator != (uint32 rgba) const
    {
        return Get() != rgba;
    }

    /**
     * Tests if colour is not equivalent to another CIwColour object.
     * @param other CIwColour object to test against.
     * @result true only if the values are NOT equivalent.
     */
    bool operator != (const CIwColour& other) const
    {
        return *(uint32*)this != *(uint32*)&other;
    }

    /**
     * Component-wise multiplication by another CIwColour object,
     * where 0x100 is equivalent to unity.
     * @param other CIwColour object to scale by.
     * @result Reference to the colour object itself.
     */
    CIwColour operator *= (const CIwColour& other)
    {
        IW_COLOUR_MUL_ABC_SHIFT(*this, *this, other, 7);
        return *this;
    }

    /**
     * Saturated component-wise addition with another CIwColour object.
     * @param other CIwColour object to add.
     * @result Result colour object itself.
     */
    CIwColour operator + (CIwColour other)
    {
        CIwColour ret;
        ret.r = MIN(0xff, r + other.r);
        ret.g = MIN(0xff, g + other.g);
        ret.b = MIN(0xff, b + other.b);
        ret.a = MIN(0xff, a + other.a);
        return ret;
    }

    /**
     * Component-wise addition with another CIwColour object.
     * @param other CIwColour object to add.
     * @result Reference to the colour object itself.
     */
    CIwColour operator += (const CIwColour& other)
    {
        r = MIN(0xff, r + other.r);
        g = MIN(0xff, g + other.g);
        b = MIN(0xff, b + other.b);
        a = MIN(0xff, a + other.a);
        return *this;
    }

    /**
     * Component-wise subtraction by another CIwColour object.
     * @param other CIwColour object to subtract.
     * @result Reference to the colour object itself.
     */
    CIwColour operator -= (const CIwColour& other)
    {
        r = MAX(0x00, r - other.r);
        g = MAX(0x00, g - other.g);
        b = MAX(0x00, b - other.b);
        a = MAX(0x00, a - other.a);
        return *this;
    }

    // Members
#ifndef __GNUC__
    union {
        struct
        {
#endif
            uint8   r;  //!< 8-bit red component.
            uint8   g;  //!< 8-bit green component.
            uint8   b;  //!< 8-bit blue component.
            uint8   a;  //!< 8-bit alpha component.
#ifndef __GNUC__
        };
        uint32 force_dword_align;
    };
#endif
}
#ifndef __ARMCC_VERSION
IW_ALIGNED(4)   //armcc doesn't like using the __align keyword on types
#endif
;

// Default colour to use for debug drawing.
#define IW_GX_DEBUG_COLOUR_DEFAULT IW_GX_COLOUR_MAGENTA

/** @} */
/** @} */

#endif
