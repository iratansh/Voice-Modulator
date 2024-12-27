#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include "voice_modulator.h"
#include "phase_vocoder.h"

// Data structure to hold widgets and modulation parameters
typedef struct {
    GtkWidget *knob_pitch;
    GtkWidget *knob_speed;
    GtkWidget *knob_echo;
    GtkWidget *knob_reverb;
    float pitch;
    float speed;
    float echo;
    float reverb;
} GUIWidgets;

void on_reset_clicked(GtkWidget *widget, gpointer user_data);
void on_knob_adjusted(GtkWidget *widget, GdkEvent *event, gpointer user_data);
int init_gui(int *argc, char ***argv, GUIWidgets *widgets);
void start_gui();