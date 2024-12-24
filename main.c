#include "gui.h"
#include "voice_modulator.h"

int main(int argc, char* argv[]) {
    ModulationParams params;
    if (init_audio_pipeline(&params) != 0) {
        return 1;
    }
    if (init_gui(&argc, &argv, &params) != 0) {
        return 1;
    }
    start_gui();
    return 0;
}