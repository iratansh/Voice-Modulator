#include "gui.h"
#include "voice_modulator.h" 

typedef struct {
    GtkWidget *knob_pitch;
    GtkWidget *knob_speed;
    GtkWidget *knob_echo;
    GtkWidget *knob_reverb;
    ModulationParams *params;
} GUIWidgets;

/**
 * Callback function for updating modulation parameters when a knob is adjusted.
 */
void on_knob_adjusted(GtkWidget *widget, gpointer user_data) {
    GUIWidgets *widgets = (GUIWidgets *)user_data;

    // Example: Updating the pitch modulation parameter
    if (widget == widgets->knob_pitch) {
        widgets->params->pitch_factor = get_knob_value(widget); // Hypothetical function to get knob value
    } else if (widget == widgets->knob_speed) {
        widgets->params->speed_factor = get_knob_value(widget);
    } else if (widget == widgets->knob_echo) {
        widgets->params->echo_intensity = get_knob_value(widget);
    } else if (widget == widgets->knob_reverb) {
        widgets->params->reverb_intensity = get_knob_value(widget);
    }
}

int init_gui(int *argc, char ***argv, ModulationParams *params) {
    gtk_init(argc, argv);

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Voice Modulator");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    // Create a vertical box layout
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create knobs and labels for each modulation parameter
    GUIWidgets *widgets = g_new0(GUIWidgets, 1);
    widgets->params = params;

    // Pitch knob
    widgets->knob_pitch = create_knob("Pitch", 200, 100); // Hypothetical function
    gtk_box_pack_start(GTK_BOX(vbox), widgets->knob_pitch, FALSE, FALSE, 0);
    g_signal_connect(widgets->knob_pitch, "adjusted", G_CALLBACK(on_knob_adjusted), widgets);

    // Speed knob
    widgets->knob_speed = create_knob("Speed", 200, 100);
    gtk_box_pack_start(GTK_BOX(vbox), widgets->knob_speed, FALSE, FALSE, 0);
    g_signal_connect(widgets->knob_speed, "adjusted", G_CALLBACK(on_knob_adjusted), widgets);

    // Echo knob
    widgets->knob_echo = create_knob("Echo", 200, 100);
    gtk_box_pack_start(GTK_BOX(vbox), widgets->knob_echo, FALSE, FALSE, 0);
    g_signal_connect(widgets->knob_echo, "adjusted", G_CALLBACK(on_knob_adjusted), widgets);

    // Reverb knob
    widgets->knob_reverb = create_knob("Reverb", 200, 100);
    gtk_box_pack_start(GTK_BOX(vbox), widgets->knob_reverb, FALSE, FALSE, 0);
    g_signal_connect(widgets->knob_reverb, "adjusted", G_CALLBACK(on_knob_adjusted), widgets);

    // Add exit button
    GtkWidget *exit_button = gtk_button_new_with_label("Exit");
    gtk_box_pack_start(GTK_BOX(vbox), exit_button, FALSE, FALSE, 0);
    g_signal_connect(exit_button, "clicked", G_CALLBACK(gtk_main_quit), NULL);

    // Show all widgets
    gtk_widget_show_all(window);

    // Main loop
    gtk_main();

    return 0;
}

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

