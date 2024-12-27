#include "voice_modulator.h"

// Global variables for threads and resources
static pthread_t input_thread, processing_thread, output_thread;
static int audio_running = 0; // Indicates if the audio pipeline is running
static PaStream *input_stream, *output_stream;
static float input_buffer[FRAME_SIZE];
static float output_buffer[FRAME_SIZE];

static ThreadSync sync = {
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .cond = PTHREAD_COND_INITIALIZER,
    .data_ready = 0
};

// Function prototypes
int capture_audio_input();
int send_audio_output();
void cleanup_audio_pipeline();
void cleanup_audio_io();
void* audio_input_thread(void* arg);
void* audio_processing_thread(void* arg);
void* audio_output_thread(void* arg);

// Initialize PortAudio streams
int init_audio_io(size_t sample_rate) {
    if (Pa_Initialize() != paNoError) {
        printf("Error: Failed to initialize PortAudio.\n");
        return -1;
    }

    // Configure input stream
    if (Pa_OpenDefaultStream(&input_stream, 1, 0, paFloat32, sample_rate, FRAME_SIZE, NULL, NULL) != paNoError) {
        printf("Error: Failed to open input stream.\n");
        return -1;
    }

    // Configure output stream
    if (Pa_OpenDefaultStream(&output_stream, 0, 1, paFloat32, sample_rate, FRAME_SIZE, NULL, NULL) != paNoError) {
        printf("Error: Failed to open output stream.\n");
        return -1;
    }

    if (Pa_StartStream(input_stream) != paNoError || Pa_StartStream(output_stream) != paNoError) {
        printf("Error: Failed to start audio streams.\n");
        return -1;
    }

    return 0;
}

// Capture audio input
int capture_audio_input() {
    if (Pa_ReadStream(input_stream, input_buffer, FRAME_SIZE) != paNoError) {
        printf("Error: Failed to read from input stream.\n");
        return -1;
    }

    // Signal processing thread
    pthread_mutex_lock(&sync.lock);
    sync.data_ready = 1;
    pthread_cond_signal(&sync.cond);
    pthread_mutex_unlock(&sync.lock);

    return 0;
}

// Send audio output
int send_audio_output() {
    if (Pa_WriteStream(output_stream, output_buffer, FRAME_SIZE) != paNoError) {
        printf("Error: Failed to write to output stream.\n");
        return -1;
    }
    return 0;
}

// Audio input thread
void* audio_input_thread(void* arg) {
    while (audio_running) {
        if (capture_audio_input() < 0) {
            printf("Error: Failed to capture audio input.\n");
        }
    }
    return NULL;
}

// Audio processing thread
void* audio_processing_thread(void* arg) {
    printf("Processing audio buffer...\n");

    ModulationParams* params = (ModulationParams*)arg;
    
    while (audio_running) {
        pthread_mutex_lock(&sync.lock);
        while (!sync.data_ready && audio_running) {
            pthread_cond_wait(&sync.cond, &sync.lock);
        }
        sync.data_ready = 0;
        pthread_mutex_unlock(&sync.lock);

        // Apply phase vocoder for pitch modification
        if (params) {
            phase_vocoder(input_buffer, output_buffer, FRAME_SIZE, params->pitch_factor);
        } else {
            memcpy(output_buffer, input_buffer, FRAME_SIZE * sizeof(float));
        }
    }

    return NULL;
}

// Audio output thread
void* audio_output_thread(void* arg) {
    while (audio_running) {
        if (send_audio_output() < 0) {
            printf("Error: Failed to send audio output.\n");
        }
    }
    return NULL;
}

// Initialize the audio pipeline
int init_audio_pipeline(ModulationParams* params) {
    if (params == NULL) {
        printf("Error: ModulationParams is NULL.\n");
        return -1;
    }

    if (init_audio_io(params->sample_rate) < 0) {
        printf("Error: Failed to initialize audio I/O.\n");
        return -1;
    }

    audio_running = 1;

    if (pthread_create(&input_thread, NULL, audio_input_thread, params) != 0) {
        printf("Error: Failed to create input thread.\n");
        cleanup_audio_pipeline();
        return -1;
    }

    if (pthread_create(&processing_thread, NULL, audio_processing_thread, params) != 0) {
        printf("Error: Failed to create processing thread.\n");
        cleanup_audio_pipeline();
        return -1;
    }

    if (pthread_create(&output_thread, NULL, audio_output_thread, params) != 0) {
        printf("Error: Failed to create output thread.\n");
        cleanup_audio_pipeline();
        return -1;
    }

    return 0;
}

// Cleanup the audio pipeline
void cleanup_audio_pipeline() {
    audio_running = 0;

    pthread_join(input_thread, NULL);
    pthread_join(processing_thread, NULL);
    pthread_join(output_thread, NULL);

    cleanup_audio_io();
}

// Cleanup PortAudio resources
void cleanup_audio_io() {
    if (input_stream) Pa_CloseStream(input_stream);
    if (output_stream) Pa_CloseStream(output_stream);
    Pa_Terminate();
}

// Testing audio pipeline
int main() {
    ModulationParams params = {
        .sample_rate = 44100,
        .pitch_factor = 1.2f // Increase pitch slightly
    };

    if (init_audio_pipeline(&params) < 0) {
        printf("Error: Failed to initialize audio pipeline.\n");
        return -1;
    }

    printf("Audio pipeline running. Press Enter to stop...\n");
    getchar(); // Keep running until user presses Enter

    cleanup_audio_pipeline();
    printf("Audio pipeline stopped.\n");
    return 0;
}
