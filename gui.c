#include "gui.h"
#include "voice_modulator.h"
#include <gtk/gtk.h>
#include <math.h>

// Data structure to hold widgets for GUI
typedef struct {
    GtkWidget *knob_pitch;
    GtkWidget *knob_speed;
    GtkWidget *knob_echo;
    GtkWidget *knob_reverb;
    ModulationParams *params;
} GUIWidgets;

/**
 * Callback function for the "Reset" button.
 * Resets all the knobs to their default values.
 */
void on_reset_clicked(GtkWidget *widget, gpointer user_data) {
    GUIWidgets *widgets = (GUIWidgets *)user_data;

    // Reset modulation parameters to default values
    widgets->params->pitch_factor = 1.0f;      // Default unmodified pitch
    widgets->params->speed_factor = 1.0f;      // Default unmodified speed
    widgets->params->echo_intensity = 0.0f;    // No echo
    widgets->params->reverb_intensity = 0.0f;  // No reverb

    // Update knobs to reflect default values
    update_knob(widgets->knob_pitch, M_PI);  // Reset angle to midpoint for default value
    update_knob(widgets->knob_speed, M_PI);
    update_knob(widgets->knob_echo, -M_PI);
    update_knob(widgets->knob_reverb, -M_PI);

    gtk_widget_queue_draw(widgets->knob_pitch);  // Force redraw for each knob
    gtk_widget_queue_draw(widgets->knob_speed);
    gtk_widget_queue_draw(widgets->knob_echo);
    gtk_widget_queue_draw(widgets->knob_reverb);
}

/**
 * Initializes the GTK application and sets up the main window.
 */
int init_gui(int *argc, char ***argv, ModulationParams *params) {
    gtk_init(argc, argv);

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Voice Modulator");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 500); // Adjust dimensions to fit all elements
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);       // Disable resizing

    // Create a vertical box layout
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Add a header label
    GtkWidget *header_label = gtk_label_new("<b>Voice Modulator</b>");
    gtk_label_set_use_markup(GTK_LABEL(header_label), TRUE);   // Enable markup for bold text
    gtk_box_pack_start(GTK_BOX(vbox), header_label, FALSE, FALSE, 10);

    // Allocate memory for GUI widgets and set the modulation parameters
    GUIWidgets *widgets = g_new0(GUIWidgets, 1);
    widgets->params = params;

    // Create knobs for each modulation parameter
    widgets->knob_pitch = create_knob("Pitch", 200, 100);
    gtk_box_pack_start(GTK_BOX(vbox), widgets->knob_pitch, FALSE, FALSE, 0);
    g_signal_connect(widgets->knob_pitch, "adjusted", G_CALLBACK(on_knob_adjusted), widgets);

    widgets->knob_speed = create_knob("Speed", 200, 100);
    gtk_box_pack_start(GTK_BOX(vbox), widgets->knob_speed, FALSE, FALSE, 0);
    g_signal_connect(widgets->knob_speed, "adjusted", G_CALLBACK(on_knob_adjusted), widgets);

    widgets->knob_echo = create_knob("Echo", 200, 100);
    gtk_box_pack_start(GTK_BOX(vbox), widgets->knob_echo, FALSE, FALSE, 0);
    g_signal_connect(widgets->knob_echo, "adjusted", G_CALLBACK(on_knob_adjusted), widgets);

    widgets->knob_reverb = create_knob("Reverb", 200, 100);
    gtk_box_pack_start(GTK_BOX(vbox), widgets->knob_reverb, FALSE, FALSE, 0);
    g_signal_connect(widgets->knob_reverb, "adjusted", G_CALLBACK(on_knob_adjusted), widgets);

    // Add Reset button
    GtkWidget *reset_button = gtk_button_new_with_label("Reset");
    gtk_box_pack_start(GTK_BOX(vbox), reset_button, FALSE, FALSE, 0);
    g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_clicked), widgets);

    // Add exit button
    GtkWidget *exit_button = gtk_button_new_with_label("Exit");
    gtk_box_pack_start(GTK_BOX(vbox), exit_button, FALSE, FALSE, 0);
    g_signal_connect(exit_button, "clicked", G_CALLBACK(gtk_main_quit), NULL);

    // Connect the destroy signal for window close
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Show all widgets
    gtk_widget_show_all(window);

    return 0;
}

/**
 * Starts the GTK main loop.
 */
void start_gui() {
    gtk_main();
}

/**
 * Cleans up resources allocated by the GTK application.
 */
void cleanup_gui() {
    // Perform any necessary cleanup, if needed
}
