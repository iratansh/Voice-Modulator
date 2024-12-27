#include "phase_vocoder.h"
#include <string.h> 
#include <stdio.h>
#include <pthread.h>
#include "portaudio.h"

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
    pthread_cond_t cond;
    int data_ready;
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