/**
 * \file SDLGLTexture.cpp
 * \brief SDL/OpenGL Texture functions.
 *
 * These are various functions for loading and unloading textures into
 * VRAM in an SDL/OpenGL environment.
 */

#include <string>
#include <iostream>
#include "SDLGLTexture.hpp"

/**
 * \brief Frees a texture from video memory.
 * \param texID The OpenGL texture ID to free from memory.
 */
void SDLGL_FreeTexture( long &texID ) {
  if ( texID != 0 )
    glDeleteTextures( 1, (GLuint*)(&texID) );
  texID = 0;
}

/**
 * \brief Loads a texture from an image file into video memory.
 * \param fileName The image file name.
 * \param filtering True if you want linear filtering.
 * \return The OpenGL texture ID.
 */
long SDLGL_LoadTextureFromFile( std::string fileName, bool filtering ) {
  unsigned long ignorex, ignorey;
  return SDLGL_LoadTextureFromFileBestFit( fileName, filtering, ignorex, ignorey );
}

/**
 * \brief Moves a texture from one memory block to the other.
 * \param src Source surface.
 * \param dest Destination surface.
 *
 * Moves the contents of src to dest, provided that dest is equal or greater in
 * size. It will put it in the upper-left-hand corner (0,0).  Make sure both
 * surfaces are 32-bit with alpha channels.
 */
static void MoveTexture( SDL_Surface *src, SDL_Surface *dest ) {
  Sint32 x, y;
  Uint32 *srcPixels, *destPixels;

  if ( src && dest ) {
    if ( dest->w >= src->w && dest->h >= src->h ) {
      // You need to lock surfaces before handling their raw pixels.
      SDL_LockSurface( dest );
      SDL_LockSurface( src );
      for ( y = 0; y < src->h; y++ ) {
        // The source's pixels are easy: a row
        // start is pixels+y*src->w.
        srcPixels = (Uint32*)src->pixels + y*src->w;
        // Destination's pixel rowstarts are dest->pixels + y*dest->w.
        destPixels = (Uint32*)dest->pixels + y*dest->w;
        for ( x = 0; x < src->w; x++ ) {
          *destPixels = *srcPixels;
          destPixels++;
          srcPixels++;
        }
      }
      // We've done what we need to do.  Time to clean up.
      SDL_UnlockSurface( src );
      SDL_UnlockSurface( dest );
    }
  }
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
 * \brief Loads a texture from an image file and fits into a GL-compatible size.
 * \param fileName The name of the image file.
 * \param filtering True for linear filtering, false for nearest-neighbor.
 * \param realW Returns the real width of the texture.
 * \param realH Returns the real height of the texture.
 * \return The OpenGL texture ID.
 */
long SDLGL_LoadTextureFromFileBestFit( std::string fileName, bool filtering, unsigned long &realW, unsigned long &realH ) {
  GLuint theTexture;
  SDL_Surface *loadSurface, *theSurface, *convertedSurface;
  Uint32 rmask, gmask, bmask, amask;

  loadSurface = IMG_Load( fileName.c_str() );

  if ( loadSurface ) {
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
      rmask = 0xff000000;
      gmask = 0x00ff0000;
      bmask = 0x0000ff00;
      amask = 0x000000ff;
    #else
      rmask = 0x000000ff;
      gmask = 0x0000ff00;
      bmask = 0x00ff0000;
      amask = 0xff000000;
    #endif
    theSurface = SDL_CreateRGBSurface( SDL_SWSURFACE | SDL_SRCALPHA, NextPowerOfTwo(loadSurface->w), NextPowerOfTwo(loadSurface->h), 32, rmask, gmask, bmask, amask );
    SDL_FillRect( theSurface, NULL, SDL_MapRGBA( theSurface->format, 0,0,0,255 ) );
    convertedSurface = SDL_ConvertSurface( loadSurface, theSurface->format, SDL_SWSURFACE | SDL_SRCALPHA );

    MoveTexture( convertedSurface, theSurface );
  }
  else
    theSurface = NULL;

  if ( theSurface ) {
    glGenTextures( 1, &theTexture);

    glBindTexture( GL_TEXTURE_2D, theTexture );

    if ( filtering ) {
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    } else {
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    }

    realW = loadSurface->w;
    realH = loadSurface->h;
    glTexImage2D( GL_TEXTURE_2D, 0, 4, theSurface->w, theSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, theSurface->pixels );

    SDL_FreeSurface( theSurface );
    SDL_FreeSurface( loadSurface );
    SDL_FreeSurface( convertedSurface );

    return theTexture;
  }
  else
    return 0; // GLspeak for "no texture"
}

