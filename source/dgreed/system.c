#include "system.h"

#include "memory.h"
#include "darray.h"

#if defined(__APPLE__) && !defined(__S3E__)
# include <mach/mach.h>
# include <mach/mach_time.h>
#else
# include <sys/time.h>
# include <time.h>
#endif

/*
-----------
--- Time --
-----------
*/

static float t_acc = 0.0f, t_scale = 1.0f;
static float t_s = 0.0f, t_d = 0.0f;
static float last_frame_time = 0.0f, last_fps_update = 0.0f;
static uint fps = 0;
float inactive_time = 0.0f;
uint fps_count;

float time_s(void) {
  return t_acc / 1000.0f;
}

float time_ms(void) {
  return t_acc;
}

float time_delta(void) {
  // TODO: fix timestep here?
  // return t_d * 1000.0f;
  return (1000.0f / 60.0f) * t_scale;
}

uint time_fps(void) {
  return fps;
}

void time_scale(float s) {
  t_scale = s;
}

#if defined(__APPLE__) && !defined(__S3E__)

void time_start(void)
{
}

uint time_ms_current(void) {
  static mach_timebase_info_data_t info;
  static bool first = true;
  static uint64_t start_t;
  if(first) {
    mach_timebase_info(&info);
    start_t = mach_absolute_time();
    first = false;
  }
  
  uint64_t t = mach_absolute_time();

  t = t - start_t;
  
  t *= info.numer;
  t /= info.denom;
  
  return t / 1000000;
}

#else

/* The first ticks value of the application */
#ifdef HAVE_CLOCK_GETTIME
static struct timespec start;
#else
static struct timeval start;
#endif /* HAVE_CLOCK_GETTIME */

void time_start(void)
{
	/* Set first ticks value */
#if HAVE_CLOCK_GETTIME
	clock_gettime(CLOCK_MONOTONIC,&start);
#else
	gettimeofday(&start, NULL);
#endif
}

uint time_ms_current(void)
{
#if HAVE_CLOCK_GETTIME
  uint ticks;
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC,&now);
  ticks=(now.tv_sec-start.tv_sec)*1000+(now.tv_nsec-start.tv_nsec)/1000000;
  return(ticks);
#else /*  HAVE_CLOCK_GETTIME */
  uint ticks;
  struct timeval now;
  gettimeofday(&now, NULL);
  ticks=(now.tv_sec-start.tv_sec)*1000+(now.tv_usec-start.tv_usec)/1000;
  return(ticks);
#endif /*  HAVE_CLOCK_GETTIME */
}
#endif

static float _get_t(void) {
  return (float)time_ms_current();
}

void _time_update(float current_time) {
  t_s = current_time - inactive_time;
  t_d = t_s - last_frame_time;
  t_acc += t_d * t_scale;
  if(last_fps_update + 1000.0f < t_s) {
    fps = fps_count;
    fps_count = 0;
    last_fps_update = t_s;
  }
  last_frame_time = t_s;
}
