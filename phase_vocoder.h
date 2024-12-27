#include <math.h>
#include <complex.h>
#include <fftw3.h> 
#include <stdlib.h>
#include <string.h>

#define FRAME_SIZE 1024   // Size of each frame
#define HOP_SIZE 256      // Hop size for overlapping frames

int phase_vocoder(const float* input, float* output, size_t length, float pitch_factor);
