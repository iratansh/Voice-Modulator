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

/**
 * Callback function for the "Reset" button.
 * Resets all the knobs to their default positions.
 */
void on_reset_clicked(GtkWidget *widget, gpointer user_data) {
    GUIWidgets *widgets = (GUIWidgets *)user_data;

    // Reset modulation parameters to default values
    widgets->pitch = 1.0f;  // Default pitch
    widgets->speed = 1.0f;  // Default speed
    widgets->echo = 0.0f;   // Default echo
    widgets->reverb = 0.0f; // Default reverb

    // Force redraw for each knob
    gtk_widget_queue_draw(widgets->knob_pitch);
    gtk_widget_queue_draw(widgets->knob_speed);
    gtk_widget_queue_draw(widgets->knob_echo);
    gtk_widget_queue_draw(widgets->knob_reverb);

    printf("All knobs reset to default values.\n");
}

/**
 * Callback for knob adjustments.
 * Updates the corresponding modulation parameter.
 */
void on_knob_adjusted(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    GUIWidgets *widgets = (GUIWidgets *)user_data;

    if (widget == widgets->knob_pitch) {
        widgets->pitch = ((double)rand() / RAND_MAX) * 2.0; // Simulate pitch adjustment
        printf("Pitch adjusted to %.2f\n", widgets->pitch);
    } else if (widget == widgets->knob_speed) {
        widgets->speed = ((double)rand() / RAND_MAX) * 2.0; // Simulate speed adjustment
        printf("Speed adjusted to %.2f\n", widgets->speed);
    } else if (widget == widgets->knob_echo) {
        widgets->echo = ((double)rand() / RAND_MAX) * 1.0; // Simulate echo adjustment
        printf("Echo adjusted to %.2f\n", widgets->echo);
    } else if (widget == widgets->knob_reverb) {
        widgets->reverb = ((double)rand() / RAND_MAX) * 1.0; // Simulate reverb adjustment
        printf("Reverb adjusted to %.2f\n", widgets->reverb);
    }
}

/**
 * Initializes the GTK application and sets up the main window.
 */
int init_gui(int *argc, char ***argv, GUIWidgets *widgets) {
    gtk_init(argc, argv);

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Voice Modulator GUI");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 500);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    // Create a vertical box layout
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Add a header label
    GtkWidget *header_label = gtk_label_new("<b>Voice Modulator</b>");
    gtk_label_set_use_markup(GTK_LABEL(header_label), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), header_label, FALSE, FALSE, 10);

    // Initialize knobs
    widgets->knob_pitch = gtk_drawing_area_new();
    widgets->knob_speed = gtk_drawing_area_new();
    widgets->knob_echo = gtk_drawing_area_new();
    widgets->knob_reverb = gtk_drawing_area_new();

    // Set default sizes for knobs
    gtk_widget_set_size_request(widgets->knob_pitch, 150, 150);
    gtk_widget_set_size_request(widgets->knob_speed, 150, 150);
    gtk_widget_set_size_request(widgets->knob_echo, 150, 150);
    gtk_widget_set_size_request(widgets->knob_reverb, 150, 150);

    // Add knobs to the layout
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(hbox), widgets->knob_pitch, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), widgets->knob_speed, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), widgets->knob_echo, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), widgets->knob_reverb, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

    // Connect signals for knobs
    g_signal_connect(widgets->knob_pitch, "draw", G_CALLBACK(on_knob_adjusted), widgets);
    g_signal_connect(widgets->knob_speed, "draw", G_CALLBACK(on_knob_adjusted), widgets);
    g_signal_connect(widgets->knob_echo, "draw", G_CALLBACK(on_knob_adjusted), widgets);
    g_signal_connect(widgets->knob_reverb, "draw", G_CALLBACK(on_knob_adjusted), widgets);

    // Add Reset and Exit buttons
    GtkWidget *reset_button = gtk_button_new_with_label("Reset");
    gtk_box_pack_start(GTK_BOX(vbox), reset_button, FALSE, FALSE, 0);
    g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_clicked), widgets);

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