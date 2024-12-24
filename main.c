#include "gui.h"
#include "voice_modulator.h"

// Function to initialize the ModulationParams structure with default values
void init_modulation_params(ModulationParams *params) {
    if (params == NULL) return;

    params->pitch_factor = 1.0f;          // No pitch modification
    params->speed_factor = 1.0f;          // Normal speed
    params->echo_intensity = 0.0f;        // No echo effect
    params->reverb_intensity = 0.0f;      // No reverb effect
    params->echo_delay = 500;             // Default echo delay in milliseconds
    params->sample_rate = 44100;          // Default sample rate 
}


int main(int argc, char **argv) {
    ModulationParams params;

    // Initialize modulation parameters with default values
    init_modulation_params(&params);

    // Pass the initialized parameters to the GUI
    init_gui(&argc, &argv, &params);

    return 0;
}
