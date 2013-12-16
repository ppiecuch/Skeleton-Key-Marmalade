/**
 * \file SDLGLFont.hpp
 * \brief Bitmap font class.
 *
 * Defines a bitmap font class.  A bitmap font is represented with 95
 * characters that represent ASCII characters 32 to 126 (0x20 to 0x7E).
 * Characters that fall out of this range should be represented with 0x20, or
 * space.
*/

#ifndef _SDLGLFONT_H_
#define _SDLGLFONT_H_

#include <string>

/**
 * A bitmap font class usable in an SDL/OpenGL environment.
 */
class SDLGLFont {
  public:
    SDLGLFont(std::string);
    ~SDLGLFont();
    void DrawString( float x, float y, std::string theString );
    void DrawString( float x, float y, std::string theString, int xResolution, int yResolution );
  private:
    bool LoadFontFromFile( std::string fileName );

    bool fontLoaded;            //!< True if font is loaded.
    long fontTexture;           //!< Longint corresponding to OpenGL texture id.
    unsigned long fontX, fontY; //!< How big the font texture is (the file).
    unsigned long charX, charY; //!< How big each individual character is.
    unsigned long texX, texY;   //!< How big the actual texture is in VRAM.
};

#endif
