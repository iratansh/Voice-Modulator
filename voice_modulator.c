#include "voice_modulator.h"

// Global variables for threads and resources
static pthread_t input_thread, processing_thread, output_thread;
static int audio_running = 0; // Indicates if the audio pipeline is running
static PaStream *input_stream, *output_stream;
static float input_buffer[FRAME_SIZE];
static float output_buffer[FRAME_SIZE];
static CircularBuffer* audio_buffer;

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

int init_audio_io(size_t sample_rate) {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        printf("Error: Failed to initialize PortAudio: %s\n", Pa_GetErrorText(err));
        return -1;
    }

    // Print available devices
    int numDevices = Pa_GetDeviceCount();
    printf("Available audio devices:\n");
    for(int i = 0; i < numDevices; i++) {
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
        printf("%d: %s (in: %d, out: %d)\n", 
               i, deviceInfo->name, 
               deviceInfo->maxInputChannels,
               deviceInfo->maxOutputChannels);
    }

    PaDeviceIndex inputDevice = Pa_GetDefaultInputDevice();
    PaDeviceIndex outputDevice = Pa_GetDefaultOutputDevice();
    
    printf("Using input device: %s\n", Pa_GetDeviceInfo(inputDevice)->name);
    printf("Using output device: %s\n", Pa_GetDeviceInfo(outputDevice)->name);

    // Input stream parameters
    PaStreamParameters inputParams = {
        .device = inputDevice,
        .channelCount = 1,
        .sampleFormat = paFloat32,
        .suggestedLatency = 0.005,
        .hostApiSpecificStreamInfo = NULL
    };

    // Output stream parameters
    PaStreamParameters outputParams = {
        .device = outputDevice,
        .channelCount = 1,
        .sampleFormat = paFloat32,
        .suggestedLatency = 0.005,
        .hostApiSpecificStreamInfo = NULL
    };

    printf("Opening input stream...\n");
    err = Pa_OpenStream(&input_stream,
                       &inputParams,
                       NULL,
                       sample_rate,
                       FRAME_SIZE,
                       paClipOff,
                       NULL,
                       NULL);
    if (err != paNoError) {
        printf("Error: Failed to open input stream: %s\n", Pa_GetErrorText(err));
        return -1;
    }

    printf("Opening output stream...\n");
    err = Pa_OpenStream(&output_stream,
                       NULL,
                       &outputParams,
                       sample_rate,
                       FRAME_SIZE,
                       paClipOff,
                       NULL,
                       NULL);
    if (err != paNoError) {
        printf("Error: Failed to open output stream: %s\n", Pa_GetErrorText(err));
        Pa_CloseStream(input_stream);
        return -1;
    }

    // Start streams with debug prints
    printf("Starting input stream...\n");
    err = Pa_StartStream(input_stream);
    if (err != paNoError) {
        printf("Error: Failed to start input stream: %s\n", Pa_GetErrorText(err));
        Pa_CloseStream(input_stream);
        Pa_CloseStream(output_stream);
        return -1;
    }

    printf("Starting output stream...\n");
    err = Pa_StartStream(output_stream);
    if (err != paNoError) {
        printf("Error: Failed to start output stream: %s\n", Pa_GetErrorText(err));
        Pa_StopStream(input_stream);
        Pa_CloseStream(input_stream);
        Pa_CloseStream(output_stream);
        return -1;
    }

    printf("Audio I/O initialized successfully\n");
    return 0;
}

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

int send_audio_output() {
    PaError err = Pa_WriteStream(output_stream, output_buffer, FRAME_SIZE);
    if (err != paNoError) {
        printf("Error writing to output stream: %s\n", Pa_GetErrorText(err));
        return -1;
    }
    return 0;
}

// Update audio_input_thread
void* audio_input_thread(void* arg) {
    while (audio_running) {
        if (Pa_ReadStream(input_stream, input_buffer, FRAME_SIZE) != paNoError) {
            printf("Error: Failed to read from input stream.\n");
            continue;
        }

        pthread_mutex_lock(&sync.lock);
        circular_buffer_write(audio_buffer, input_buffer, FRAME_SIZE);
        sync.input_ready_flag = 1;
        pthread_cond_signal(&sync.input_ready);
        pthread_mutex_unlock(&sync.lock);
    }
    return NULL;
}

void* audio_processing_thread(void* arg) {
    ModulationParams* params = (ModulationParams*)arg;
    float temp_buffer[FRAME_SIZE];
    float processed_buffer[FRAME_SIZE];
    float running_rms = 0.0f;
    float current_gain = 1.0f;
    
    while (audio_running) {
        pthread_mutex_lock(&sync.lock);
        while (!sync.input_ready_flag && audio_running) {
            pthread_cond_wait(&sync.input_ready, &sync.lock);
        }
        
        circular_buffer_read(audio_buffer, temp_buffer, FRAME_SIZE);
        sync.input_ready_flag = 0;
        pthread_mutex_unlock(&sync.lock);

        // Calculate input RMS
        float frame_rms = 0.0f;
        for (int i = 0; i < FRAME_SIZE; i++) {
            frame_rms += temp_buffer[i] * temp_buffer[i];
        }
        frame_rms = sqrtf(frame_rms / FRAME_SIZE);

        // Update running RMS with smoothing
        running_rms = running_rms * (1.0f - RMS_SMOOTH_FACTOR) + 
                     frame_rms * RMS_SMOOTH_FACTOR;

        // Skip processing if input is too quiet (noise gate)
        if (running_rms < NOISE_FLOOR) {
            memset(output_buffer, 0, FRAME_SIZE * sizeof(float));
            pthread_mutex_lock(&sync.lock);
            sync.output_ready_flag = 1;
            pthread_cond_signal(&sync.output_ready);
            pthread_mutex_unlock(&sync.lock);
            continue;
        }

        // Process audio using the phase vocoder
        if (params && phase_vocoder(temp_buffer, processed_buffer, FRAME_SIZE, params->pitch_factor) >= 0) {
            // Calculate desired gain to reach target RMS
            float desired_gain = running_rms > NOISE_FLOOR ? 
                               TARGET_RMS / running_rms : current_gain;
            
            // Smooth gain changes
            current_gain = current_gain * (1.0f - GAIN_SMOOTH_FACTOR) + 
                         desired_gain * GAIN_SMOOTH_FACTOR;
            
            // Apply gain with soft limiting
            for (int i = 0; i < FRAME_SIZE; i++) {
                float sample = processed_buffer[i] * current_gain;
                // Soft limiting to prevent clipping
                output_buffer[i] = sample / (1.0f + fabsf(sample));
            }
        } else {
            printf("Error: Failed to process audio.\n");
            continue;
        }

        pthread_mutex_lock(&sync.lock);
        sync.output_ready_flag = 1;
        pthread_cond_signal(&sync.output_ready);
        pthread_mutex_unlock(&sync.lock);
    }
    return NULL;
}

void* audio_output_thread(void* arg) {
    int frames_output = 0;
    
    while (audio_running) {
        pthread_mutex_lock(&sync.lock);
        while (!sync.output_ready_flag && audio_running) {
            pthread_cond_wait(&sync.output_ready, &sync.lock);
        }
        sync.output_ready_flag = 0;
        pthread_mutex_unlock(&sync.lock);

        PaError err = Pa_WriteStream(output_stream, output_buffer, FRAME_SIZE);
        if (err != paNoError) {
            printf("Error: Failed to write to output stream: %s\n", Pa_GetErrorText(err));
            continue;
        }

        frames_output++;
        if(frames_output % 100 == 0) {  // Print every 100 frames
            printf("Output frame %d\n", frames_output);
        }
    }
    return NULL;
}

// Update init_audio_pipeline
int init_audio_pipeline(ModulationParams* params) {
    if (params == NULL) {
        printf("Error: ModulationParams is NULL.\n");
        return -1;
    }

    // Initialize circular buffer
    audio_buffer = create_circular_buffer(BUFFER_SIZE);
    if (!audio_buffer) {
        printf("Error: Failed to create audio buffer.\n");
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

void cleanup_audio_pipeline() {
    audio_running = 0;

    pthread_join(input_thread, NULL);
    pthread_join(processing_thread, NULL);
    pthread_join(output_thread, NULL);

    cleanup_audio_io();
    cleanup_phase_vocoder();
}

void cleanup_audio_io() {
    if (input_stream) Pa_CloseStream(input_stream);
    if (output_stream) Pa_CloseStream(output_stream);
    Pa_Terminate();
}

int main() {
    ModulationParams params = {
        .sample_rate = 44100,
        .pitch_factor = 1.2f
    };

    printf("Initializing audio pipeline...\n");
    if (init_audio_pipeline(&params) < 0) {
        printf("Error: Failed to initialize audio pipeline.\n");
        return -1;
    }

    printf("Audio pipeline running. Speak into your microphone to hear the pitch-shifted output.\n");
    printf("Press Enter to stop...\n");
    getchar();

    printf("Cleaning up...\n");
    cleanup_audio_pipeline();
    printf("Audio pipeline stopped.\n");
    return 0;
}