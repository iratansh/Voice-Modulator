#include <math.h>
#include <complex.h>
#include <fftw3.h> 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <immintrin.h>
#include <omp.h>

#define FRAME_SIZE 2048
#define OVERLAP_RATIO 4  
#define HOP_SIZE (FRAME_SIZE / OVERLAP_RATIO)

typedef struct {
    float* buffer;
    size_t size;
    size_t read_pos;
    size_t write_pos;
    pthread_mutex_t lock;
} CircularBuffer;

int phase_vocoder(const float* input, float* output, size_t length, float pitch_factor);
