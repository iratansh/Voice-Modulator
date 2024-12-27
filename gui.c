#include <gtk/gtk.h>
#include "custom_knob.h"
#include <math.h>
#include <stdio.h>

// Data structure to hold widgets for GUI
typedef struct {
    GtkWidget *knob_pitch;
    GtkWidget *knob_speed;
    GtkWidget *knob_echo;
    GtkWidget *knob_reverb;
} GUIWidgets;

/**
 * Callback function for the "Reset" button.
 * Resets all the knobs to their default positions.
 */
void on_reset_clicked(GtkWidget *widget, gpointer user_data) {
    GUIWidgets *widgets = (GUIWidgets *)user_data;

    // Update labels to reflect default values
    gtk_label_set_text(GTK_LABEL(widgets->knob_pitch), "Pitch: Default");
    gtk_label_set_text(GTK_LABEL(widgets->knob_speed), "Speed: Default");
    gtk_label_set_text(GTK_LABEL(widgets->knob_echo), "Echo: Default");
    gtk_label_set_text(GTK_LABEL(widgets->knob_reverb), "Reverb: Default");
}

/**
 * Initializes the GTK application and sets up the main window.
 */
int init_gui(int *argc, char ***argv) {
    gtk_init(argc, argv);

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GUI Test");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 500);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    // Create a vertical box layout
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Add a header label
    GtkWidget *header_label = gtk_label_new("<b>GUI Test</b>");
    gtk_label_set_use_markup(GTK_LABEL(header_label), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), header_label, FALSE, FALSE, 10);

    // Create a horizontal box layout for the knobs
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

    // Add knobs to the horizontal box
    GtkWidget *knob_pitch = gtk_drawing_area_new();
    gtk_widget_set_size_request(knob_pitch, 150, 150);
    gtk_box_pack_start(GTK_BOX(hbox), knob_pitch, TRUE, TRUE, 0);

    GtkWidget *knob_speed = gtk_drawing_area_new();
    gtk_widget_set_size_request(knob_speed, 150, 150);
    gtk_box_pack_start(GTK_BOX(hbox), knob_speed, TRUE, TRUE, 0);

    GtkWidget *knob_echo = gtk_drawing_area_new();
    gtk_widget_set_size_request(knob_echo, 150, 150);
    gtk_box_pack_start(GTK_BOX(hbox), knob_echo, TRUE, TRUE, 0);

    GtkWidget *knob_reverb = gtk_drawing_area_new();
    gtk_widget_set_size_request(knob_reverb, 150, 150);
    gtk_box_pack_start(GTK_BOX(hbox), knob_reverb, TRUE, TRUE, 0);

    // Add knobs to the custom_knob list
    KnobData *knob_data_pitch = add_knob(75, 75);
    KnobData *knob_data_speed = add_knob(75, 75); // For knob_speed
    KnobData *knob_data_echo = add_knob(75, 75); // For knob_echo
    KnobData *knob_data_reverb = add_knob(75, 75); // For knob_reverb

    g_signal_connect(knob_pitch, "draw", G_CALLBACK(on_draw_knob), knob_data_pitch);
    g_signal_connect(knob_pitch, "button-press-event", G_CALLBACK(on_button_press), knob_data_pitch);
    g_signal_connect(knob_pitch, "button-release-event", G_CALLBACK(on_button_release), knob_data_pitch);
    g_signal_connect(knob_pitch, "motion-notify-event", G_CALLBACK(on_motion_notify_knob), knob_data_pitch);

    g_signal_connect(knob_speed, "draw", G_CALLBACK(on_draw_knob), knob_data_speed);
    g_signal_connect(knob_speed, "button-press-event", G_CALLBACK(on_button_press), knob_data_speed);
    g_signal_connect(knob_speed, "button-release-event", G_CALLBACK(on_button_release), knob_data_speed);
    g_signal_connect(knob_speed, "motion-notify-event", G_CALLBACK(on_motion_notify_knob), knob_data_speed);

    g_signal_connect(knob_echo, "draw", G_CALLBACK(on_draw_knob), knob_data_echo);
    g_signal_connect(knob_echo, "button-press-event", G_CALLBACK(on_button_press), knob_data_echo);
    g_signal_connect(knob_echo, "button-release-event", G_CALLBACK(on_button_release), knob_data_echo);
    g_signal_connect(knob_echo, "motion-notify-event", G_CALLBACK(on_motion_notify_knob), knob_data_echo);

    g_signal_connect(knob_reverb, "draw", G_CALLBACK(on_draw_knob), knob_data_reverb);
    g_signal_connect(knob_reverb, "button-press-event", G_CALLBACK(on_button_press), knob_data_reverb);
    g_signal_connect(knob_reverb, "button-release-event", G_CALLBACK(on_button_release), knob_data_reverb);
    g_signal_connect(knob_reverb, "motion-notify-event", G_CALLBACK(on_motion_notify_knob), knob_data_reverb);


    gtk_widget_add_events(knob_pitch, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
    gtk_widget_add_events(knob_speed, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
    gtk_widget_add_events(knob_echo, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
    gtk_widget_add_events(knob_reverb, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);

    // Add Reset and Exit buttons
    GtkWidget *reset_button = gtk_button_new_with_label("Reset");
    gtk_box_pack_start(GTK_BOX(vbox), reset_button, FALSE, FALSE, 0);
    g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_clicked), NULL);

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

// Testing GUI - No Voice Modulation yet
int main(int argc, char **argv) {
    // Initialize the GUI
    if (init_gui(&argc, &argv) != 0) {
        printf("Error: Failed to initialize the GUI.\n");
        return -1;
    }

    // Start the GUI
    start_gui();

    return 0;
}




// #include "gui.h"
// #include <gtk/gtk.h>
// #include <math.h>
// #include <stdio.h>

// // Data structure to hold widgets for GUI
// typedef struct {
//     GtkWidget *knob_pitch;
//     GtkWidget *knob_speed;
//     GtkWidget *knob_echo;
//     GtkWidget *knob_reverb;
// } GUIWidgets;

// /**
//  * Callback function for the "Reset" button.
//  * Resets all the knobs to their default positions.
//  */
// void on_reset_clicked(GtkWidget *widget, gpointer user_data) {
//     GUIWidgets *widgets = (GUIWidgets *)user_data;

//     // Update knobs to reflect default values
//     update_knob(widgets->knob_pitch, M_PI);  // Reset angle to midpoint for default value
//     update_knob(widgets->knob_speed, M_PI);
//     update_knob(widgets->knob_echo, -M_PI);
//     update_knob(widgets->knob_reverb, -M_PI);

//     gtk_widget_queue_draw(widgets->knob_pitch);  // Force redraw for each knob
//     gtk_widget_queue_draw(widgets->knob_speed);
//     gtk_widget_queue_draw(widgets->knob_echo);
//     gtk_widget_queue_draw(widgets->knob_reverb);
// }

// /**
//  * Initializes the GTK application and sets up the main window.
//  */
// int init_gui(int *argc, char ***argv) {
//     gtk_init(argc, argv);

//     // Create the main window
//     GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
//     gtk_window_set_title(GTK_WINDOW(window), "GUI Test");
//     gtk_window_set_default_size(GTK_WINDOW(window), 600, 500); // Adjust dimensions to fit all elements
//     gtk_window_set_resizable(GTK_WINDOW(window), FALSE);       // Disable resizing

//     // Create a vertical box layout
//     GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
//     gtk_container_add(GTK_CONTAINER(window), vbox);

//     // Add a header label
//     GtkWidget *header_label = gtk_label_new("<b>GUI Test</b>");
//     gtk_label_set_use_markup(GTK_LABEL(header_label), TRUE);   // Enable markup for bold text
//     gtk_box_pack_start(GTK_BOX(vbox), header_label, FALSE, FALSE, 10);

//     // Allocate memory for GUI widgets
//     GUIWidgets *widgets = g_new0(GUIWidgets, 1);

//     // Create knobs for testing
//     widgets->knob_pitch = create_knob("Pitch", 200, 100);
//     gtk_box_pack_start(GTK_BOX(vbox), widgets->knob_pitch, FALSE, FALSE, 0);

//     widgets->knob_speed = create_knob("Speed", 200, 100);
//     gtk_box_pack_start(GTK_BOX(vbox), widgets->knob_speed, FALSE, FALSE, 0);

//     widgets->knob_echo = create_knob("Echo", 200, 100);
//     gtk_box_pack_start(GTK_BOX(vbox), widgets->knob_echo, FALSE, FALSE, 0);

//     widgets->knob_reverb = create_knob("Reverb", 200, 100);
//     gtk_box_pack_start(GTK_BOX(vbox), widgets->knob_reverb, FALSE, FALSE, 0);

//     // Add Reset button
//     GtkWidget *reset_button = gtk_button_new_with_label("Reset");
//     gtk_box_pack_start(GTK_BOX(vbox), reset_button, FALSE, FALSE, 0);
//     g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_clicked), widgets);

//     // Add exit button
//     GtkWidget *exit_button = gtk_button_new_with_label("Exit");
//     gtk_box_pack_start(GTK_BOX(vbox), exit_button, FALSE, FALSE, 0);
//     g_signal_connect(exit_button, "clicked", G_CALLBACK(gtk_main_quit), NULL);

//     // Connect the destroy signal for window close
//     g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

//     // Show all widgets
//     gtk_widget_show_all(window);

//     return 0;
// }

// /**
//  * Starts the GTK main loop.
//  */
// void start_gui() {
//     gtk_main();
// }

// /**
//  * Main entry point for the program.
//  */
// int main(int argc, char **argv) {
//     // Initialize the GUI
//     if (init_gui(&argc, &argv) != 0) {
//         printf("Error: Failed to initialize the GUI.\n");
//         return -1;
//     }

//     // Start the GUI
//     start_gui();

//     return 0;
// }


// // #include "gui.h"
// // #include "voice_modulator.h"
// // #include <gtk/gtk.h>
// // #include <math.h>
// // #include <stdio.h>

// // // Data structure to hold widgets for GUI
// // typedef struct {
// //     GtkWidget *knob_pitch;
// //     GtkWidget *knob_speed;
// //     GtkWidget *knob_echo;
// //     GtkWidget *knob_reverb;
// //     ModulationParams *params;
// // } GUIWidgets;

// // /**
// //  * Callback function for the "Reset" button.
// //  * Resets all the knobs to their default values.
// //  */
// // void on_reset_clicked(GtkWidget *widget, gpointer user_data) {
// //     GUIWidgets *widgets = (GUIWidgets *)user_data;

// //     // Reset modulation parameters to default values
// //     widgets->params->pitch_factor = 1.0f;      // Default unmodified pitch
// //     widgets->params->speed_factor = 1.0f;      // Default unmodified speed
// //     widgets->params->echo_intensity = 0.0f;    // No echo
// //     widgets->params->reverb_intensity = 0.0f;  // No reverb

// //     // Update knobs to reflect default values
// //     update_knob(widgets->knob_pitch, M_PI);  // Reset angle to midpoint for default value
// //     update_knob(widgets->knob_speed, M_PI);
// //     update_knob(widgets->knob_echo, -M_PI);
// //     update_knob(widgets->knob_reverb, -M_PI);

// //     gtk_widget_queue_draw(widgets->knob_pitch);  // Force redraw for each knob
// //     gtk_widget_queue_draw(widgets->knob_speed);
// //     gtk_widget_queue_draw(widgets->knob_echo);
// //     gtk_widget_queue_draw(widgets->knob_reverb);
// // }

// // /**
// //  * Initializes the GTK application and sets up the main window.
// //  */
// // int init_gui(int *argc, char ***argv, ModulationParams *params) {
// //     gtk_init(argc, argv);

// //     // Create the main window
// //     GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
// //     gtk_window_set_title(GTK_WINDOW(window), "Voice Modulator");
// //     gtk_window_set_default_size(GTK_WINDOW(window), 600, 500); // Adjust dimensions to fit all elements
// //     gtk_window_set_resizable(GTK_WINDOW(window), FALSE);       // Disable resizing

// //     // Create a vertical box layout
// //     GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
// //     gtk_container_add(GTK_CONTAINER(window), vbox);

// //     // Add a header label
// //     GtkWidget *header_label = gtk_label_new("<b>Voice Modulator</b>");
// //     gtk_label_set_use_markup(GTK_LABEL(header_label), TRUE);   // Enable markup for bold text
// //     gtk_box_pack_start(GTK_BOX(vbox), header_label, FALSE, FALSE, 10);

// //     // Allocate memory for GUI widgets and set the modulation parameters
// //     GUIWidgets *widgets = g_new0(GUIWidgets, 1);
// //     widgets->params = params;

// //     // Create knobs for each modulation parameter
// //     widgets->knob_pitch = create_knob("Pitch", 200, 100);
// //     gtk_box_pack_start(GTK_BOX(vbox), widgets->knob_pitch, FALSE, FALSE, 0);
// //     g_signal_connect(widgets->knob_pitch, "adjusted", G_CALLBACK(on_knob_adjusted), widgets);

// //     widgets->knob_speed = create_knob("Speed", 200, 100);
// //     gtk_box_pack_start(GTK_BOX(vbox), widgets->knob_speed, FALSE, FALSE, 0);
// //     g_signal_connect(widgets->knob_speed, "adjusted", G_CALLBACK(on_knob_adjusted), widgets);

// //     widgets->knob_echo = create_knob("Echo", 200, 100);
// //     gtk_box_pack_start(GTK_BOX(vbox), widgets->knob_echo, FALSE, FALSE, 0);
// //     g_signal_connect(widgets->knob_echo, "adjusted", G_CALLBACK(on_knob_adjusted), widgets);

// //     widgets->knob_reverb = create_knob("Reverb", 200, 100);
// //     gtk_box_pack_start(GTK_BOX(vbox), widgets->knob_reverb, FALSE, FALSE, 0);
// //     g_signal_connect(widgets->knob_reverb, "adjusted", G_CALLBACK(on_knob_adjusted), widgets);

// //     // Add Reset button
// //     GtkWidget *reset_button = gtk_button_new_with_label("Reset");
// //     gtk_box_pack_start(GTK_BOX(vbox), reset_button, FALSE, FALSE, 0);
// //     g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_clicked), widgets);

// //     // Add exit button
// //     GtkWidget *exit_button = gtk_button_new_with_label("Exit");
// //     gtk_box_pack_start(GTK_BOX(vbox), exit_button, FALSE, FALSE, 0);
// //     g_signal_connect(exit_button, "clicked", G_CALLBACK(gtk_main_quit), NULL);

// //     // Connect the destroy signal for window close
// //     g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

// //     // Show all widgets
// //     gtk_widget_show_all(window);

// //     return 0;
// // }

// // /**
// //  * Starts the GTK main loop.
// //  */
// // void start_gui() {
// //     gtk_main();
// // }

// // /**
// //  * Cleans up resources allocated by the GTK application.
// //  */
// // void cleanup_gui() {
// //     // Perform any necessary cleanup, if needed
// // }


// // int main(int argc, char **argv) {
// //     // Create default modulation parameters
// //     ModulationParams params = {
// //         .pitch_factor = 1.0f,      // Default pitch factor
// //         .speed_factor = 1.0f,      // Default speed factor
// //         .echo_intensity = 0.0f,    // No echo
// //         .echo_delay = 0,           // No delay
// //         .reverb_intensity = 0.0f,  // No reverb
// //         .sample_rate = 44100       // Default sample rate
// //     };

// //     // Initialize the audio pipeline
// //     if (init_audio_pipeline(&params) != 0) {
// //         printf("Error: Failed to initialize the audio pipeline.\n");
// //         return -1;
// //     }

// //     // Initialize the GUI
// //     if (init_gui(&argc, &argv, &params) != 0) {
// //         printf("Error: Failed to initialize the GUI.\n");
// //         cleanup_audio_pipeline();
// //         return -1;
// //     }

// //     // Start the GUI
// //     start_gui();

// //     // Clean up the GUI and audio pipeline when the program exits
// //     cleanup_gui();
// //     cleanup_audio_pipeline();

// //     return 0;
// // }
