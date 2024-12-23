#include "phase_vocoder.h"

#define FRAME_SIZE 1024   // Size of each frame
#define HOP_SIZE 256      // Hop size for overlapping frames

/**
 * Phase vocoder time-stretching algorithm.
 * 
 * This function takes an input audio signal and applies time-stretching using the phase vocoder algorithm.
 * The pitch factor controls the speed of the time-stretching, with values greater than 1 increasing the speed
 * and values less than 1 decreasing the speed.
 * 
 * @param input The input audio signal.
 * @param output The output audio signal.
 * @param length The length of the input signal.
 * @param pitch_factor The pitch factor for time-stretching.
 * @return 0 on success, -1 on failure.
 */
int phase_vocoder(const float* input, float* output, size_t length, float pitch_factor) {
    // Allocate buffers for FFT
    fftwf_complex *fft_in = fftwf_malloc(sizeof(fftwf_complex) * FRAME_SIZE);
    fftwf_complex *fft_out = fftwf_malloc(sizeof(fftwf_complex) * FRAME_SIZE);
    float *window = malloc(sizeof(float) * FRAME_SIZE);
    
    // Generate Hann window
    for (size_t i = 0; i < FRAME_SIZE; i++) {
        window[i] = 0.5 * (1 - cos(2 * M_PI * i / (FRAME_SIZE - 1)));
    }

    // FFTW plans
    fftwf_plan forward_plan = fftwf_plan_dft_r2c_1d(FRAME_SIZE, fft_in, fft_out, FFTW_MEASURE);
    fftwf_plan inverse_plan = fftwf_plan_dft_c2r_1d(FRAME_SIZE, fft_out, fft_in, FFTW_MEASURE);

    // Phase vocoder processing
    float *prev_phase = calloc(FRAME_SIZE / 2 + 1, sizeof(float));
    float *phase_accum = calloc(FRAME_SIZE / 2 + 1, sizeof(float));
    float *magnitude = calloc(FRAME_SIZE / 2 + 1, sizeof(float));

    for (size_t frame_start = 0; frame_start < length; frame_start += HOP_SIZE) {
        // Load input frame and apply window
        for (size_t i = 0; i < FRAME_SIZE; i++) {
            size_t idx = frame_start + i;
            fft_in[i] = (idx < length) ? input[idx] * window[i] : 0;
        }

        // Forward FFT
        fftwf_execute(forward_plan);

        // Process FFT bins
        for (size_t k = 0; k < FRAME_SIZE / 2 + 1; k++) {
            float real = creal(fft_out[k]);
            float imag = cimag(fft_out[k]);
            float phase = atan2(imag, real);
            magnitude[k] = sqrt(real * real + imag * imag);

            // Phase difference and accumulation
            float phase_diff = phase - prev_phase[k];
            prev_phase[k] = phase;
            phase_accum[k] += phase_diff * pitch_factor;

            // Update FFT output
            fft_out[k] = magnitude[k] * cexp(I * phase_accum[k]);
        }

        // Inverse FFT
        fftwf_execute(inverse_plan);

        // Overlap and add to output
        for (size_t i = 0; i < FRAME_SIZE; i++) {
            size_t idx = frame_start + i;
            if (idx < length) {
                output[idx] += fft_in[i] / FRAME_SIZE; // Normalize by frame size
            }
        }
    }

    // Cleanup
    fftwf_destroy_plan(forward_plan);
    fftwf_destroy_plan(inverse_plan);
    fftwf_free(fft_in);
    fftwf_free(fft_out);
    free(window);
    free(prev_phase);
    free(phase_accum);
    free(magnitude);

    return 0;
}
