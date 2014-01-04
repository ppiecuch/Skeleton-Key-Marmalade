#ifndef WAV_H
#define WAV_H

#include "utils.h"

typedef struct {
	uint frequency;
	uint bits;
	uint channels;
	uint size;
	void* data;
} RawSound;	

#ifdef __cplusplus
extern "C" {
#endif

    RawSound* wav_load(const char* filename);
    void wav_free(RawSound* sound);

#ifdef __cplusplus
}
#endif

#endif

