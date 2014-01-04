/**
 * \file FPSLimit.cpp
 * \brief FPS Limiter.
 *
 * This class automatically limits the frames per second of a game loop, and it
 * provides performance warnings to stdout if the framerate is less than the
 * desired value.
 */

#include "FPSLimit.hpp"

static const int NUM_MILLISECONDS_IN_SECOND = 1000;

/**
 * \brief Constructor for FPSLimit class.
 * \param desiredFPS The desired framerate to regulate to.
 */
FPSLimit::FPSLimit( int desiredFPS ) {
  // Calculate the number of milliseconds per frame.
  numMillis = NUM_MILLISECONDS_IN_SECOND/desiredFPS;
  // Set lastTime to the current timestamp so that we have a place to start from.
  lastTime = SDL_GetTicks();
}

/**
 * \brief Call every frame at the end to regulate the framerate.
 */
void FPSLimit::Regulate() {
  // Figure out how many milliseconds have elapsed since last call.
  Uint32 timeDiff = SDL_GetTicks() - lastTime;

  // If the amount of time is less than the number of milliseconds per frame, we
  // have time left over that we need to sleep for.  If not, we need to make a
  // performance warning since our loop went over how much time it has.
  if ( timeDiff < numMillis )
    SDL_Delay( numMillis - timeDiff );
  else
    printf("Performance warning: Frame took %dms (expected <%dms).\n", timeDiff, numMillis);

  // Set this for the next time we call Regulate().
  lastTime = SDL_GetTicks();
}
