#include "phase_vocoder.h"

// Global variables for threads and resources
static pthread_t input_thread, processing_thread, output_thread;
static float input_buffer[FRAME_SIZE];
static float output_buffer[FRAME_SIZE];
static float* overlap_buffer = NULL;

// Circular buffer functions
CircularBuffer* create_circular_buffer(size_t size) {
    CircularBuffer* cb = malloc(sizeof(CircularBuffer));
    cb->buffer = calloc(size, sizeof(float));
    cb->size = size;
    cb->read_pos = 0;
    cb->write_pos = 0;
    pthread_mutex_init(&cb->lock, NULL);
    return cb;
}

int circular_buffer_write(CircularBuffer* cb, float* data, size_t length) {
    pthread_mutex_lock(&cb->lock);
    for (size_t i = 0; i < length; i++) {
        cb->buffer[cb->write_pos] = data[i];
        cb->write_pos = (cb->write_pos + 1) % cb->size;
    }
    pthread_mutex_unlock(&cb->lock);
    return 0;
}

int circular_buffer_read(CircularBuffer* cb, float* data, size_t length) {
    pthread_mutex_lock(&cb->lock);
    for (size_t i = 0; i < length; i++) {
        data[i] = cb->buffer[cb->read_pos];
        cb->read_pos = (cb->read_pos + 1) % cb->size;
    }
    pthread_mutex_unlock(&cb->lock);
    return 0;
}

#if SIMD_AVAILABLE
void apply_window_simd(float* input, float* window, size_t length) {
    size_t i;
    for (i = 0; i + 4 <= length; i += 4) {
        __m128 in = _mm_load_ps(&input[i]);
        __m128 win = _mm_load_ps(&window[i]);
        _mm_store_ps(&input[i], _mm_mul_ps(in, win));
    }
    // Handle remaining elements
    for (; i < length; i++) {
        input[i] *= window[i];
    }
}
#else
void apply_window_simd(float* input, float* window, size_t length) {
    for (size_t i = 0; i < length; i++) {
        input[i] *= window[i];
    }
}
#endif

// Pre-computed window function
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

// Optimized FFT bin processing
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

// Optimized phase vocoder implementation
int phase_vocoder(const float *input, float *output, size_t length, float pitch_factor) {
    if (input == NULL || output == NULL || length == 0 || pitch_factor <= 0) {
        return -1;
    }

    // Use pre-computed window
    float *window = get_window();
    if (!window) return -1;

    // Static FFT resources
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

    // Static phase processing buffers
    static float *prev_phase = NULL;
    static float *phase_accum = NULL;
    
    if (!prev_phase) {
        prev_phase = calloc(FRAME_SIZE / 2 + 1, sizeof(float));
        phase_accum = calloc(FRAME_SIZE / 2 + 1, sizeof(float));
    }

    // Initialize overlap buffer if needed
    if (!overlap_buffer) {
        overlap_buffer = calloc(FRAME_SIZE, sizeof(float));
    }

    memset(output, 0, sizeof(float) * length);

    // Process frames
    #pragma omp parallel for
    // Add explicit size_t cast to prevent unsigned comparison issues
    for (size_t frame_start = 0; frame_start <= (size_t)(length - FRAME_SIZE); frame_start += HOP_SIZE) {
        memcpy((float *)fft_in, input + frame_start, FRAME_SIZE * sizeof(float));
        apply_window_simd((float *)fft_in, window, FRAME_SIZE);

        fftwf_execute(forward_plan);
        process_fft_bins(fft_out, prev_phase, phase_accum, pitch_factor);
        fftwf_execute(inverse_plan);

        // Overlap-add with normalization
        float norm_factor = 1.0f / FRAME_SIZE;
        for (size_t i = 0; i < FRAME_SIZE; i++) {
            size_t idx = frame_start + i;
            if (idx < length) {
                float processed = ((float *)fft_in)[i] * window[i] * norm_factor;
                output[idx] = overlap_buffer[i] + processed;
                
                if (i + HOP_SIZE < FRAME_SIZE) {
                    overlap_buffer[i] = overlap_buffer[i + HOP_SIZE];
                } else {
                    overlap_buffer[i] = 0;
                }
            }
        }
    }

    return 0;
}

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