/**
 * \file SDLGLTexture.hpp
 * \brief SDL/OpenGL Texture functions.
 *
 * These are various functions for loading and unloading textures into
 * VRAM in an SDL/OpenGL environment.
 */

#ifndef __SDLGLTEXTURE_HPP__
#define __SDLGLTEXTURE_HPP__

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_image.h>
#include <string>

void SDLGL_FreeTexture( long &texID );

long SDLGL_LoadTextureFromFile( std::string fileName, bool filtering );
long SDLGL_LoadTextureFromFileBestFit( std::string fileName, bool filtering, unsigned long &realW, unsigned long &realH );

#endif
