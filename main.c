#include "gui.h"

int main(int argc, char **argv) {
    // Initialize GUI widgets structure
    GUIWidgets widgets = {0};  // Zero-initialize all fields

    // Initialize modulation parameters with defaults
    ModulationParams mod_params = {
        .pitch_factor = 1.0f,
        .speed_factor = 1.0f,
        .echo_intensity = 0.0f,
        .reverb_intensity = 0.0f,
        .echo_delay = 0,
        .sample_rate = 44100
    };

    // Initialize the GUI
    if (init_gui(&argc, &argv, &widgets, &mod_params) < 0) {
        fprintf(stderr, "Failed to initialize GUI\n");
        return 1;
    }

    // Initialize the audio pipeline
    if (init_audio_pipeline(&mod_params) < 0) {
        fprintf(stderr, "Failed to initialize audio pipeline\n");
        return 1;
    }

    printf("Voice Modulator started. Use the GUI controls to adjust parameters.\n");
    printf("Press Ctrl+C to exit.\n");

    // Start the GUI main loop
    start_gui();

    // Cleanup
    cleanup_audio_pipeline();

    return 0;
}
