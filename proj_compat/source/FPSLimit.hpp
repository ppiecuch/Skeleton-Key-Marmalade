/**
 * \file FPSLimit.hpp
 * \brief FPS Limiter.
 *
 * This class automatically limits the frames per second of a game loop, and it
 * provides performance warnings to stdout if the framerate is less than the
 * desired value.
 */

#ifndef __FPSLIMIT_H__
#define __FPSLIMIT_H__

#include "SDL.h"

class FPSLimit {
  public:
    FPSLimit( int desiredFPS );
    void Regulate();
  private:
    Uint32 lastTime;  //!< The SDL timestamp received last call to Regulate.
    Uint32 numMillis; //!< The number of milliseconds per frame.
};

#endif
