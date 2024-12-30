#include "phase_vocoder.h"
#include <string.h> 
#include <stdio.h>
#include <pthread.h>
#include "portaudio.h"

#ifndef FRAME_SIZE
#define FRAME_SIZE 256
#endif
#ifndef OVERLAP_RATIO
#define OVERLAP_RATIO 4  
#endif
#ifndef HOP_SIZE
#define HOP_SIZE (FRAME_SIZE / OVERLAP_RATIO)
#endif
#define BUFFER_SIZE (FRAME_SIZE * 4) 

// Data structure to hold voice modulation parameters
typedef struct {
    float pitch_factor;      // Pitch shifting
    float speed_factor;      // Speed adjustment
    float echo_intensity;    // Intensity of the echo effect (0.0 - 1.0)
    float reverb_intensity;  // Intensity of the reverb effect (0.0 - 1.0)
    size_t echo_delay;       // Echo delay 
    size_t sample_rate;      // Audio sample rate 
} ModulationParams;

// Struct for thread synchronization
typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t input_ready;
    pthread_cond_t process_ready;
    pthread_cond_t output_ready;
    int input_ready_flag;
    int process_ready_flag;
    int output_ready_flag;
} ThreadSync;

// Function prototypes
int capture_audio_input();
int send_audio_output();
void cleanup_audio_pipeline();
void cleanup_audio_io();
void* audio_input_thread(void* arg);
void* audio_processing_thread(void* arg);
void* audio_output_thread(void* arg);
int init_audio_io(size_t sample_rate);
int init_audio_pipeline(ModulationParams* params);