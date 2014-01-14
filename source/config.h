#define AIRPLAY_DEVICE_ANDROID 0
#define AIRPLAY_DEVICE_IPHONE 1
#define AIRPLAY_DEVICE_BADA 2
#define AIRPLAY_DEVICE_PLAYBOOK 3
#define AIRPLAY_DEVICE_BB10 4

/* set phone to build for here, can be "andoid", "iphone", or "bada" */
#if defined __PLAYBOOK__
#define AIRPLAY_DEVICE AIRPLAY_DEVICE_PLAYBOOK
#elif defined __BB10__
#define AIRPLAY_DEVICE AIRPLAY_DEVICE_BB10
#elif defined __ANDROID__
#define AIRPLAY_DEVICE AIRPLAY_DEVICE_ANDROID
#elif defined __BADA__ || defined SHP
#define AIRPLAY_DEVICE AIRPLAY_DEVICE_BADA
#elif defined __APPLE__
# include <TargetConditionals.h>
# if TARGET_OS_IPHONE
#  define AIRPLAY_DEVICE AIRPLAY_DEVICE_IPHONE
# endif
#else
# error *** Unknown device!
#endif
