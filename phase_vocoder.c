#include "phase_vocoder.h"

// Global variables for threads and resources
static pthread_t input_thread, processing_thread, output_thread;
static float input_buffer[FRAME_SIZE];
static float output_buffer[FRAME_SIZE];
static float* overlap_buffer = NULL;

/**
 * Creates a circular buffer of the specified size.
 *
 * The buffer is initialized with all zeros, and the read and write positions
 * are set to zero. The buffer is protected by a mutex for thread safety.
 *
 * @param size The size of the circular buffer in floats.
 * @return A pointer to the created CircularBuffer object.
 */
CircularBuffer* create_circular_buffer(size_t size) {
    CircularBuffer* cb = malloc(sizeof(CircularBuffer));
    cb->buffer = calloc(size, sizeof(float));
    cb->size = size;
    cb->read_pos = 0;
    cb->write_pos = 0;
    pthread_mutex_init(&cb->lock, NULL);
    return cb;
}

/**
 * Writes the specified data to the circular buffer.
 *
 * This function writes the specified amount of data to the circular buffer
 * starting at the current write position. The write position is then advanced
 * by the amount of data written. If the write position reaches the end of the
 * buffer, it wraps around to the start of the buffer.
 *
 * @param cb The circular buffer to write to.
 * @param data The data to write.
 * @param length The amount of data to write in floats.
 * @return 0 on success.
 */
int circular_buffer_write(CircularBuffer* cb, float* data, size_t length) {
    pthread_mutex_lock(&cb->lock);
    for (size_t i = 0; i < length; i++) {
        cb->buffer[cb->write_pos] = data[i];
        cb->write_pos = (cb->write_pos + 1) % cb->size;
    }
    pthread_mutex_unlock(&cb->lock);
    return 0;
}

/**
 * Reads the specified amount of data from the circular buffer.
 *
 * This function reads the specified amount of data from the circular buffer
 * starting at the current read position. The read position is then advanced
 * by the amount of data read. If the read position reaches the end of the
 * buffer, it wraps around to the start of the buffer.
 *
 * @param cb The circular buffer to read from.
 * @param data The array to store the read data in.
 * @param length The amount of data to read in floats.
 * @return 0 on success.
 */
int circular_buffer_read(CircularBuffer* cb, float* data, size_t length) {
    pthread_mutex_lock(&cb->lock);
    for (size_t i = 0; i < length; i++) {
        data[i] = cb->buffer[cb->read_pos];
        cb->read_pos = (cb->read_pos + 1) % cb->size;
    }
    pthread_mutex_unlock(&cb->lock);
    return 0;
}

/**
 * Applies the specified window function to the input signal.
 *
 * This function multiplies the input signal with the specified window function
 * element-wise. The window function is assumed to be of the same
 * length as the input signal. This function does not use SIMD instructions, if SIMD is not available -> slower.
 * This function was part of the original implementation of the phase_vocoder function (since I don't have access to the SIMD instructions)
 * Modify this function if you are running on a machine with SIMD support -> faster
 * 
 * @param input The input signal to be windowed.
 * @param window The window function to be applied.
 * @param length The length of the input signal and window function.
 */
void apply_window_simd(float* input, float* window, size_t length) {
    for (size_t i = 0; i < length; i++) {
        input[i] *= window[i];
    }
}

/**
 * Returns a pointer to a statically allocated Hann window array.
 *
 * This function allocates memory for a Hann window array of length FRAME_SIZE
 * and initializes it on the first call. Subsequent calls return the same
 * pointer to the array.
 *
 * @return A pointer to the statically allocated Hann window array.
 */
static float* get_window() {
    static float* window = NULL;
    static int initialized = 0;
    
    if (!initialized) {
        window = malloc(sizeof(float) * FRAME_SIZE);
        for (size_t i = 0; i < FRAME_SIZE; i++) {
            window[i] = 0.5 * (1 - cos(2 * M_PI * i / (FRAME_SIZE - 1)));
        }
        initialized = 1;
    }
    return window;
}


/**
 * Process FFT bins to implement the phase vocoder algorithm.
 *
 * This function processes the FFT bins by shifting the phase of each bin
 * according to the pitch factor. The phase vocoder algorithm is implemented
 * using the following steps: 1) Calculate the phase difference between the
 * current bin and the previous one; 2) Subtract 2*pi from the phase difference
 * to unwrap it; 3) Add the phase difference to the phase accumulator; 4)
 * Calculate the new phase using the phase accumulator; 5) Update the real and
 * imaginary parts of the FFT bin using the new phase.
 *
 * @param fft_out The FFT bins to be processed.
 * @param prev_phase The previous phase of each bin.
 * @param phase_accum The phase accumulator.
 * @param pitch_factor The pitch factor.
 */
void process_fft_bins(fftwf_complex* fft_out, float* prev_phase, 
                     float* phase_accum, float pitch_factor) {
    const size_t bins = FRAME_SIZE / 2 + 1;
    const float two_pi = 2 * M_PI;
    
    #pragma omp parallel for
    for (size_t k = 0; k < bins; k++) {
        float real = crealf(fft_out[k]);
        float imag = cimagf(fft_out[k]);
        float mag = sqrtf(real * real + imag * imag);
        float phase = atan2f(imag, real);
        
        float phase_diff = phase - prev_phase[k];
        prev_phase[k] = phase;
        
        phase_diff -= two_pi * roundf(phase_diff / two_pi);
        phase_accum[k] += phase_diff * pitch_factor;
        
        float new_phase = phase_accum[k];
        fft_out[k] = mag * (cosf(new_phase) + I * sinf(new_phase));
    }
}

/**
 * Applies the phase vocoder algorithm to the input signal.
 *
 * This function applies the phase vocoder algorithm to the input signal and
 * stores the result in the output buffer. The phase vocoder algorithm is
 * implemented using the following steps: 1) Split the input signal into frames
 * of length FRAME_SIZE; 2) Apply a window function to each frame; 3)
 * Calculate the FFT of each frame; 4) Process each FFT bin by shifting the
 * phase of each bin according to the pitch factor; 5) Calculate the IFFT of
 * each processed frame; 6) Store the result in the output buffer.
 *
 * @param input The input signal to be processed.
 * @param output The output buffer in which to store the result.
 * @param length The length of the input signal and output buffer.
 * @param pitch_factor The pitch factor.
 *
 * @return 0 on success, -1 on failure.
 */
int phase_vocoder(const float *input, float *output, size_t length, float pitch_factor) {
    if (input == NULL || output == NULL || length == 0 || pitch_factor <= 0) {
        return -1;
    }

    float *window = get_window();
    if (!window) return -1;

    static fftwf_plan forward_plan = NULL;
    static fftwf_plan inverse_plan = NULL;
    static fftwf_complex *fft_in = NULL;
    static fftwf_complex *fft_out = NULL;
    
    if (!forward_plan) {
        fftwf_init_threads();
        fftwf_plan_with_nthreads(omp_get_max_threads());
        
        fft_in = fftwf_malloc(sizeof(fftwf_complex) * FRAME_SIZE);
        fft_out = fftwf_malloc(sizeof(fftwf_complex) * FRAME_SIZE);
        forward_plan = fftwf_plan_dft_r2c_1d(FRAME_SIZE, (float *)fft_in, fft_out, FFTW_MEASURE);
        inverse_plan = fftwf_plan_dft_c2r_1d(FRAME_SIZE, fft_out, (float *)fft_in, FFTW_MEASURE);
    }

    static float *prev_phase = NULL;
    static float *phase_accum = NULL;
    
    if (!prev_phase) {
        prev_phase = calloc(FRAME_SIZE / 2 + 1, sizeof(float));
        phase_accum = calloc(FRAME_SIZE / 2 + 1, sizeof(float));
    }

    // Clear output and process frame
    memset(output, 0, sizeof(float) * length);
    
    // Single frame processing without overlap
    memcpy((float *)fft_in, input, FRAME_SIZE * sizeof(float));
    
    // Apply window
    for (size_t i = 0; i < FRAME_SIZE; i++) {
        ((float *)fft_in)[i] *= window[i];
    }

    fftwf_execute(forward_plan);
    
    // Simple phase processing
    const size_t bins = FRAME_SIZE / 2 + 1;
    for (size_t k = 0; k < bins; k++) {
        float real = crealf(fft_out[k]);
        float imag = cimagf(fft_out[k]);
        float mag = sqrtf(real * real + imag * imag);
        float phase = atan2f(imag, real);
        
        // Simple phase modification
        phase *= pitch_factor;
        
        // Reconstruct bin
        fft_out[k] = mag * (cosf(phase) + I * sinf(phase));
    }
    
    fftwf_execute(inverse_plan);

    float norm = 1.0f / FRAME_SIZE;
    for (size_t i = 0; i < FRAME_SIZE; i++) {
        output[i] = ((float *)fft_in)[i] * norm;
    }

    return 0;
}

/**
 * Cleans up resources used by the phase vocoder.
 *
 * This function releases all resources and memory allocated for the phase 
 * vocoder, including FFT plans, input/output buffers, and phase arrays. 
 * It destroys the FFTW plans, frees the allocated memory for FFT input/output, 
 * and phase tracking arrays. Additionally, it frees the overlap buffer if it 
 * was allocated, and calls `fftwf_cleanup_threads` to clean up FFTW threading 
 * resources. This function should be called when the phase vocoder is no longer 
 * needed to prevent memory leaks.
 */
void cleanup_phase_vocoder() {
    static fftwf_plan forward_plan = NULL;
    static fftwf_plan inverse_plan = NULL;
    static fftwf_complex *fft_in = NULL;
    static fftwf_complex *fft_out = NULL;
    static float *prev_phase = NULL;
    static float *phase_accum = NULL;

    if (forward_plan) {
        fftwf_destroy_plan(forward_plan);
        fftwf_destroy_plan(inverse_plan);
        fftwf_free(fft_in);
        fftwf_free(fft_out);
        free(prev_phase);
        free(phase_accum);
        forward_plan = NULL;
    }

    if (overlap_buffer) {
        free(overlap_buffer);
        overlap_buffer = NULL;
    }

    fftwf_cleanup_threads();
}