#include "voice_modulator.h"

// Global variables for threads and resources
static pthread_t input_thread, processing_thread, output_thread;
static int audio_running = 0; // Indicates if the audio pipeline is running
static PaStream *input_stream, *output_stream;
static float input_buffer[FRAME_SIZE];
static float output_buffer[FRAME_SIZE];

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t input_ready;
    pthread_cond_t process_ready;
    pthread_cond_t output_ready;
    int input_ready_flag;
    int process_ready_flag;
    int output_ready_flag;
} ThreadSync;

static ThreadSync sync = {
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .input_ready = PTHREAD_COND_INITIALIZER,
    .process_ready = PTHREAD_COND_INITIALIZER,
    .output_ready = PTHREAD_COND_INITIALIZER,
    .input_ready_flag = 0,
    .process_ready_flag = 0,
    .output_ready_flag = 0
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

    pthread_mutex_lock(&sync.lock);
    sync.input_ready_flag = 1;
    pthread_cond_signal(&sync.input_ready);
    pthread_mutex_unlock(&sync.lock);

    return 0;
}

// Send audio output
int send_audio_output() {
    PaError err = Pa_WriteStream(output_stream, output_buffer, FRAME_SIZE);
    if (err != paNoError) {
        printf("Error writing to output stream: %s\n", Pa_GetErrorText(err));
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
    ModulationParams* params = (ModulationParams*)arg;

    while (audio_running) {
        // Wait for input
        pthread_mutex_lock(&sync.lock);
        while (!sync.input_ready_flag && audio_running) {
            pthread_cond_wait(&sync.input_ready, &sync.lock);
        }
        sync.input_ready_flag = 0;
        pthread_mutex_unlock(&sync.lock);

        // Process audio
        if (params) {
            phase_vocoder(input_buffer, output_buffer, FRAME_SIZE, params->pitch_factor);
        } else {
            memcpy(output_buffer, input_buffer, FRAME_SIZE * sizeof(float));
        }

        // Signal output thread
        pthread_mutex_lock(&sync.lock);
        sync.output_ready_flag = 1;
        pthread_cond_signal(&sync.output_ready);
        pthread_mutex_unlock(&sync.lock);
    }
    return NULL;
}

void* audio_output_thread(void* arg) {
    while (audio_running) {
        // Wait for processed data
        pthread_mutex_lock(&sync.lock);
        while (!sync.output_ready_flag && audio_running) {
            pthread_cond_wait(&sync.output_ready, &sync.lock);
        }
        sync.output_ready_flag = 0;
        pthread_mutex_unlock(&sync.lock);

        if (send_audio_output() < 0) {
            printf("Error: Failed to send audio output.\n");
        }

        // Signal input thread that we're ready for more
        pthread_mutex_lock(&sync.lock);
        sync.input_ready_flag = 1;
        pthread_cond_signal(&sync.input_ready);
        pthread_mutex_unlock(&sync.lock);
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
