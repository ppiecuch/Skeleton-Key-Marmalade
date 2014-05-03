/*
    BMF_font: An OpenGL Bitmap Font Library
    Copyright (C) 2003-2005  Trenkwalder Markus

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Trenkwalder Markus
    trenki2@gmx.net
*/

#ifndef BMF_FONT_H
#define BMF_FONT_H

#define BMF_VERSION 101

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#include <SDL_opengl.h>

# define EXPORT

#ifdef __cplusplus
extern "C" {
#endif

/* Internal structure containing font information */
typedef struct BMF_Font BMF_Font;

/* Open a .bmf font file and create a font structure that can be used
   by the other functions. If an error occurs NULL is returned.
 */
extern EXPORT BMF_Font * BMF_Load(const char* filename);

/* Read a .bmf font file from memory. */
extern BMF_Font* BMF_LoadM(const unsigned char* data);

/* Free all resources used by the font */
extern EXPORT void BMF_Free(BMF_Font* font);


/* The function takes the font that will be used within the
   Begin() and End() pair to output text with BMF_Print().
   All the necessary GL settings are made.
   The default rendering state is set.

   The default rendering state is:
     - white and opaque color (r=1, g=1, b=1, a=1)
     - output position x=0, y=0
     - scale factor of 1 for x any y
     - no rotation
     - text alignment: bottom left
*/
extern EXPORT void BMF_Begin(BMF_Font* font);

/* Ends a BMF_Begin()/End() block and restores the GL attibutes
   that where changed by the call to BMF_Begin().
*/
extern EXPORT void BMF_End(void);

/* Print text. Use this function like the standard C printf() function.
   The currently set font, color, position, scaling, rotation, alignment ...
   is used.
   After the text has been written, the output position is advanced by
   the text width.
   If called outside BMF_Begin()/BMF_End() pairs nothing is done.
*/
extern EXPORT void BMF_Print(const char* fmt, ...);

/* some shortcut macros */

#define BMF_Printf(font, parameter) \
	BMF_Begin(font);\
		BMF_Print(parameter);\
	BMF_End();

#define BMF_Printfp(font,x,y,parameter) \
	BMF_Begin(font);\
		BMF_SetPos(x,y);\
		BMF_Print(parameter);\
	BMF_End();

#define BMF_Printfc(font,r,g,b,a,parameter) \
	BMF_Begin(font);\
		BMF_SetColor(r,g,b,a);\
		BMF_Print(parameter);\
	BMF_End();


#define BMF_ALIGN_TOP		1
#define BMF_ALIGN_BOTTOM	2
#define BMF_ALIGN_LEFT		3
#define BMF_ALIGN_RIGHT		4
#define BMF_ALIGN_CENTER	5

/* Set the parameters that will be used by the following calls to BMF_Print().
   If called outside a BMF_Begin() and BMF_End() pair, these functions modify
   the default rendering state that will be used when a call to BMF_Begin() is
   issued.
*/
extern EXPORT void BMF_SetColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
extern EXPORT void BMF_SetPos(GLfloat x, GLfloat y);
extern EXPORT void BMF_SetScale(GLfloat xs, GLfloat ys);
extern EXPORT void BMF_SetRotation(GLfloat degrees);
extern EXPORT void BMF_SetVAlign(int style);
extern EXPORT void BMF_SetHAlign(int style);


/* Returns the width of the text for a given font.
   The currently set scaling factor is taken into account.
*/
extern EXPORT GLfloat BMF_GetTextWidth(BMF_Font* font, const char* text);

/* Returns the height of the font.
   The currently set scalig factor is taken into account.
*/
extern EXPORT GLfloat BMF_GetFontHeight(BMF_Font* font);

/* Returns the value lineskip.
   The currently set scaling factor is taken into account.
*/
extern EXPORT GLfloat BMF_GetLineSkip(BMF_Font* font);


#ifdef __cplusplus
}
#endif

#endif /* BMF_FONT_H */
