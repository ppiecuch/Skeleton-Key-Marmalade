/**
 * \file SDLGLFont.cpp
 * \brief Bitmap font class.
 *
 * Defines a bitmap font class.  A bitmap font is represented with 95
 * characters that represent ASCII characters 32 to 126 (0x20 to 0x7E).
 * Characters that fall out of this range should be represented with 0x20, or
 * space.
 */

#include "SDLGLFont.hpp"
#include "SDLGLTexture.hpp"

#include <assert.h>
#include <vector>

/* Prerequisites:
  To support alpha-blended textures, you need to call the following GL functions
  somewhere earlier in your program:

  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glAlphaFunc(GL_GREATER,0.1f);
*/

/**
 * \brief Creates an SDLGLFont object from the PNG of the given filename.
 * \param fileName The file name of the PNG with the bitmap font.
 */
SDLGLFont::SDLGLFont( std::string fileName ) {
  fontLoaded = LoadFontFromFile(fileName);
}

/**
 * \brief Destructor for fonts.
 */
SDLGLFont::~SDLGLFont( ) {
  if ( fontLoaded == true )
    SDLGL_FreeTexture( fontTexture );
}

/**
 * \brief Finds the next power of two for an integer.
 * \param v The integer to find the next power of two for.
 * \return The next power of two of v.
 *
 * This code is directly copy/pasted from:
 * http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
 */
static unsigned int NextPowerOfTwo( unsigned int v ) {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}

/**
 * \brief Loads a font from a file.
 * \param fileName File name of the PNG containing the bitmap font.
 * \return True if the font loaded.
 */
bool SDLGLFont::LoadFontFromFile( std::string fileName ) {
  // Linear filtering will make the SDLGLFont look crappy.
  fontTexture = SDLGL_LoadTextureFromFileBestFit( fileName,
                                                  false,
                                                  fontX,
                                                  fontY );

  // If the font doesn't load, we've got a problem.
  if ( fontTexture )
    fontLoaded = true;
  else
    assert(0);

  // charX and charY are the width and height of a single character.
  charX = fontX / 95; // 95 = number of characters in the SDLGLFont
  charY = fontY;      // character height is the same.

  // texX and texY are the width and height of the font in the OpenGL texture
  // buffer, the dimensions of which must be a power of two.  As a result, we
  // waste a little memory and use the next power of two from the actual
  // texture size.
  texX = NextPowerOfTwo( fontX );
  texY = NextPowerOfTwo( fontY );

  return fontLoaded;
}

/**
 * \brief Draws the string.  Use this if you are already in a glOrtho state.
 * \param x The x coordinate on the screen to draw from (upper-left corner).
 * \param y The y coordinate on the screen to draw from (upper-left corner).
 * \param theString The string to draw on the screen.
 */
void SDLGLFont::DrawString( float x, float y, std::string theString ) {
  float charPieceX = ((float)fontX/(float)texX)/95.0f;

  // These are the coordinate arrays we're going to send to the GPU.
  GLfloat vertices[theString.length()*2*4];
  GLfloat texcoords[theString.length()*2*4];
  short indices[] = {0, 1, 2, 0, 2, 3};

  // Set the current GL texture to the font we want to use.
  glBindTexture( GL_TEXTURE_2D, fontTexture );

  // Populate the vertices & texcoords arrays.
  for ( int count = 0; count < theString.length(); count++ ) {
    // These must be defined in a counter-clockwise order.
    // Upper-left
    vertices[count*2*4+0] = x + charX*count;
    vertices[count*2*4+1] = y;
    texcoords[count*2*4+0] = charPieceX*(theString[count]-32);
    texcoords[count*2*4+1] = 0.0f;
    // Lower-left
    vertices[count*2*4+2] = x + charX*count;
    vertices[count*2*4+3] = y + charY;
    texcoords[count*2*4+2] = charPieceX*(theString[count]-32);
    texcoords[count*2*4+3] = (float)fontY/(float)texY;
    // Lower-right
    vertices[count*2*4+4] = x + charX*(count+1);
    vertices[count*2*4+5] = y + charY;
    texcoords[count*2*4+4] = charPieceX*(theString[count]-32+1);
    texcoords[count*2*4+5] = (float)fontY/(float)texY;
    // Upper-right
    vertices[count*2*4+6] = x + charX*(count+1);
    vertices[count*2*4+7] = y;
    texcoords[count*2*4+6] = charPieceX*(theString[count]-32+1);
    texcoords[count*2*4+7] = 0.0f;
  }

  // Set up the GL state so vertex and texcoord arrays can be uploaded.
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  // Send the arrays to the GPU.
  glVertexPointer(2, GL_FLOAT, 0, vertices);
  glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

  // draw the letters
  glDrawElements(GL_TRIANGLES, theString.length()*2, GL_UNSIGNED_SHORT, indices);

  // deactivate vertex arrays after drawing
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

/**
 * \brief Draws the string.  Use if you need a glOrtho overlay made for you.
 * \param x The x coordinate on the screen to draw from (upper-left corner).
 * \param y The y coordinate on the screen to draw from (upper-left corner).
 * \param theString The string to draw on the screen.
 * \param xResolution The total resolution of the glOrtho overlay to make.
 * \param yResolution The total resolution of the glOrtho overlay to make.
 */
void SDLGLFont::DrawString( float x, float y, std::string theString, int xResolution, int yResolution ) {
  // Saves the old ModelView and Projection matrices, then puts you
  // into a GLOrtho environment.  Activates textures.
  GLfloat modelview[16];
  GLfloat projection[16];

  // Get the current Modelview and Projection matrices so we can put them
  // back when we're done here.
  glGetFloatv(GL_MODELVIEW_MATRIX,modelview);
  glGetFloatv(GL_PROJECTION_MATRIX,projection);

  // Blank out the projection matrix.
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // Set the projection matrix to orthoganal to make a "HUD-style" overlay.
  glOrtho(0, xResolution, yResolution, 0, -1, 1);

  // Blank out the Modelview matrix.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Depth test is inconvenient, texturing is necessary.
  glDisable( GL_DEPTH_TEST );
  glEnable( GL_TEXTURE_2D );

  // Now that everything's in order, actually draw the string.
  DrawString( x, y, theString );

  // Reloads the old Modelview and Projection matrices.  Deactivates textures.
  glDisable( GL_TEXTURE_2D);
  glEnable( GL_DEPTH_TEST );

  // Put the Projection and Modelview matrices back.
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(projection);
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(modelview);
}
