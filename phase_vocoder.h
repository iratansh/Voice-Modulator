#include <math.h>
#include <complex.h>
#include <fftw3.h> 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <omp.h>
#include <pthread.h>

#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>
#define SIMD_AVAILABLE 1
#else
#define SIMD_AVAILABLE 0
#endif

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
CircularBuffer* create_circular_buffer(size_t size);
int circular_buffer_write(CircularBuffer* cb, float* data, size_t length);
int circular_buffer_read(CircularBuffer* cb, float* data, size_t length);
void cleanup_phase_vocoder();