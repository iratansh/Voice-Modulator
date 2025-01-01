#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include "voice_modulator.h"
#include "custom_knob.h"

typedef struct {
    // Knob widgets
    GtkWidget *knob_pitch;
    GtkWidget *knob_speed;
    GtkWidget *knob_echo;
    GtkWidget *knob_reverb;
    
    // Value display labels
    GtkWidget *label_pitch;
    GtkWidget *label_speed;
    GtkWidget *label_echo;
    GtkWidget *label_reverb;
    
    // Value displays
    GtkWidget *value_pitch;
    GtkWidget *value_speed;
    GtkWidget *value_echo;
    GtkWidget *value_reverb;
    
    // Current parameter values
    float pitch;    // Range: 0.25 to 4.0
    float speed;    // Range: 0.5 to 2.0
    float echo;     // Range: 0.0 to 1.0
    float reverb;   // Range: 0.0 to 1.0
    
    // Pointer to modulation parameters
    ModulationParams *mod_params;
} GUIWidgets;

void on_reset_clicked(GtkWidget *widget, gpointer user_data);
void on_knob_adjusted(GtkWidget *widget, GdkEvent *event, gpointer user_data);
int init_gui(int *argc, char ***argv, GUIWidgets *widgets, ModulationParams *mod_params);
void start_gui();