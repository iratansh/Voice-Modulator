#include <math.h>
#include <complex.h>
#include <fftw3.h> 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <omp.h>
#include <pthread.h>

#ifndef NOISE_FLOOR
#define NOISE_FLOOR 0.001f
#endif
#ifndef TARGET_RMS
#define TARGET_RMS 0.3f
#endif
#ifndef GAIN_SMOOTH_FACTOR
#define GAIN_SMOOTH_FACTOR 0.001f 
#endif
#ifndef RMS_SMOOTH_FACTOR 
#define RMS_SMOOTH_FACTOR 0.01f
#endif

#define FRAME_SIZE 1024  // Reduced from 2048 for lower latency
#define OVERLAP_RATIO 4
#define HOP_SIZE (FRAME_SIZE / OVERLAP_RATIO)
#define BUFFER_SIZE (FRAME_SIZE * 8)

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