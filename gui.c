#include "gui.h"
#include "voice_modulator.h"
#include "custom_knob.h"
#include <math.h>

// Data Structure to hold widgets for GUI
typedef struct {
    KnobData *knob_pitch;
    KnobData *knob_speed;
    KnobData *knob_echo;
    KnobData *knob_reverb;
    ModulationParams *params;
} GUIWidgets;

/**
 * Initializes the GTK application and sets up the main window.
 */
int init_gui(int *argc, char ***argv, ModulationParams *params) {
    gtk_init(argc, argv);

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Voice Modulator");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    // Create a vertical layout box
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Initialize GUI widget data
    GUIWidgets *widgets = g_new0(GUIWidgets, 1);
    widgets->params = params;

    // Create drawing area for knobs
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 600, 300);
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area, TRUE, TRUE, 0);

    // Initialize knobs
    widgets->knob_pitch = g_new0(KnobData, 1);
    widgets->knob_speed = g_new0(KnobData, 1);
    widgets->knob_echo = g_new0(KnobData, 1);
    widgets->knob_reverb = g_new0(KnobData, 1);

    add_knob(100, 150);  // Pitch knob
    add_knob(200, 150);  // Speed knob
    add_knob(300, 150);  // Echo knob
    add_knob(400, 150);  // Reverb knob

    // Connect drawing area signals
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), NULL);
    g_signal_connect(drawing_area, "motion-notify-event", G_CALLBACK(on_motion_notify), NULL);
    g_signal_connect(drawing_area, "button-press-event", G_CALLBACK(on_button_press), NULL);
    g_signal_connect(drawing_area, "button-release-event", G_CALLBACK(on_button_release), NULL);

    gtk_widget_add_events(drawing_area, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);

    // Add an exit button
    GtkWidget *exit_button = gtk_button_new_with_label("Exit");
    gtk_box_pack_start(GTK_BOX(vbox), exit_button, FALSE, FALSE, 0);
    g_signal_connect(exit_button, "clicked", G_CALLBACK(gtk_main_quit), NULL);

    // Show all widgets
    gtk_widget_show_all(window);

    return 0;
}

/**
 * Starts the GTK main loop.
 * This function runs the GTK main event loop, handling all user interactions.
 */
void start_gui() {
    gtk_main();
}

/**
 * Cleans up resources allocated by the GTK application.
 * This function should be called before exiting the program.
 */
void cleanup_gui() {
    // Free knob list resources
    g_list_free_full(knobs, g_free);
}
