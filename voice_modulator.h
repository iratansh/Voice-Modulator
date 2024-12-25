#ifndef VOICE_MODULATOR_H
#define VOICE_MODULATOR_H

#include <stddef.h>
#include <stdint.h>
#include <pthread.h>

// Data structure to hold voice modulation parameters
typedef struct {
    float pitch_factor;      // Pitch shifting
    float speed_factor;      // Speed adjustment
    float echo_intensity;    // Intensity of the echo effect (0.0 - 1.0)
    float reverb_intensity;  // Intensity of the reverb effect (0.0 - 1.0)
    size_t echo_delay;       // Echo delay 
    size_t sample_rate;      // Audio sample rate 
} ModulationParams;

int init_audio_pipeline(ModulationParams* params);
int process_audio(const float* input, float* output, size_t sample_count, ModulationParams* params);
void update_modulation_params(ModulationParams* params);
void cleanup_audio_pipeline();

#endif 

