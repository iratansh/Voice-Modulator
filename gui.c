#include "gui.h"

/**
 * Initializes the GTK application and sets up the main window.
 * 
 * This function creates the main application window with all necessary controls
 * (knobs/sliders for pitch, speed, echo, and reverb) and connects them to the
 * modulation parameter update functions.
 */
int init_gui(int* argc, char*** argv, ModulationParams* params);

/**
 * Creates a slider control for a modulation parameter.
 * 
 * This function sets up a GTK slider with specified ranges, default value,
 * and label, and connects it to update the corresponding parameter dynamically.
 */
GtkWidget* create_slider(const char* label, double min, double max, double step, double initial,
                         void (*update_callback)(GtkRange* range, gpointer user_data),
                         gpointer user_data);

/**
 * Callback function to update pitch factor from GUI.
 */
void on_pitch_slider_change(GtkRange* range, gpointer user_data);

/**
 * Callback function to update speed factor from GUI.
 */
void on_speed_slider_change(GtkRange* range, gpointer user_data);

/**
 * Callback function to update echo intensity from GUI.
 */
void on_echo_slider_change(GtkRange* range, gpointer user_data);

/**
 * Callback function to update reverb intensity from GUI.
 */
void on_reverb_slider_change(GtkRange* range, gpointer user_data);

/**
 * Starts the GTK main loop.
 * This function runs the GTK main event loop, handling all user interactions.
 */
void start_gui();

/**
 * Cleans up resources allocated by the GTK application.
 * This function should be called before exiting the program.
 */
void cleanup_gui();

