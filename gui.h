#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>
#include "voice_modulator.h"


int init_gui(int* argc, char*** argv, ModulationParams* params);
GtkWidget* create_slider(const char* label, double min, double max, double step, double initial,
                         void (*update_callback)(GtkRange* range, gpointer user_data),
                         gpointer user_data);
void on_pitch_slider_change(GtkRange* range, gpointer user_data);
void on_speed_slider_change(GtkRange* range, gpointer user_data);
void on_echo_slider_change(GtkRange* range, gpointer user_data);
void on_reverb_slider_change(GtkRange* range, gpointer user_data);
void start_gui();
void cleanup_gui();

#endif
