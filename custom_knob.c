#include "custom_knob.h"
#include <math.h>
#include <stdlib.h>

// Shared list of all knobs
GList *knobs = NULL;

// Draw all knobs in the list
gboolean on_draw_knob(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    KnobData *knob_data = (KnobData *)user_data;

    // Draw the knob
    cairo_arc(cr, knob_data->x, knob_data->y, KNOB_RADIUS, 0, 2 * M_PI);
    cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
    cairo_fill_preserve(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_stroke(cr);

    // Draw the indicator
    double indicator_x = knob_data->x + KNOB_RADIUS * cos(knob_data->angle);
    double indicator_y = knob_data->y - KNOB_RADIUS * sin(knob_data->angle);
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    cairo_move_to(cr, knob_data->x, knob_data->y);
    cairo_line_to(cr, indicator_x, indicator_y);
    cairo_stroke(cr);

    return FALSE;
}


// Detect motion and update the knob being dragged
gboolean on_motion_notify_knob(GtkWidget *widget, GdkEventMotion *event, gpointer user_data) {
    KnobData *knob_data = (KnobData *)user_data;

    if (knob_data->is_dragging) {
        double dx = event->x - knob_data->x;
        double dy = knob_data->y - event->y;

        knob_data->angle = atan2(dy, dx);
        gtk_widget_queue_draw(widget); // Redraw the widget
    }
    return TRUE;
}


// Detect when a knob is clicked
gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    KnobData *knob_data = (KnobData *)user_data;

    // Check if the click is within this specific knob's radius
    double dx = event->x - knob_data->x;
    double dy = event->y - knob_data->y;
    if (sqrt(dx * dx + dy * dy) <= KNOB_RADIUS) {
        knob_data->is_dragging = TRUE;
        return TRUE;  // Event handled
    }
    return FALSE;  // Event not handled
}


// Detect when the knob drag is released
gboolean on_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    KnobData *knob_data = (KnobData *)user_data;
    knob_data->is_dragging = FALSE;
    return TRUE;  // Event handled
}


// Create and add a new knob
KnobData *add_knob(double x, double y) {
    KnobData *knob = g_new0(KnobData, 1);
    knob->x = x;
    knob->y = y;
    knob->angle = 0.0;
    knob->is_dragging = FALSE;
    knobs = g_list_append(knobs, knob);
    return knob;
}


// Update an existing knob's position or angle
void update_knob(int knob_index, double x, double y, double angle) {
    GList *node = g_list_nth(knobs, knob_index);
    if (node) {
        KnobData *knob_data = (KnobData *)node->data;
        knob_data->x = x;
        knob_data->y = y;
        knob_data->angle = angle;
    }
}

// Main application
// Main application
void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Knob Example");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 300);

    // Create a horizontal box to arrange knobs side-by-side
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_container_add(GTK_CONTAINER(window), hbox);

    // Create knobs and add them to the box
    GtkWidget *drawing_area1 = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area1, 150, 150);
    gtk_box_pack_start(GTK_BOX(hbox), drawing_area1, TRUE, TRUE, 10);

    GtkWidget *drawing_area2 = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area2, 150, 150);
    gtk_box_pack_start(GTK_BOX(hbox), drawing_area2, TRUE, TRUE, 10);

    // Add knobs at appropriate positions
    KnobData *knob1 = add_knob(75, 75);  // Center the knob within the first drawing area
    KnobData *knob2 = add_knob(75, 75);  // Center the knob within the second drawing area

    // Connect signals for each drawing area
    g_signal_connect(drawing_area1, "draw", G_CALLBACK(on_draw_knob), knob1);
    g_signal_connect(drawing_area1, "button-press-event", G_CALLBACK(on_button_press), NULL);
    g_signal_connect(drawing_area1, "button-release-event", G_CALLBACK(on_button_release), NULL);
    g_signal_connect(drawing_area1, "motion-notify-event", G_CALLBACK(on_motion_notify_knob), knob1);

    gtk_widget_add_events(drawing_area1, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);

    g_signal_connect(drawing_area2, "draw", G_CALLBACK(on_draw_knob), knob2);
    g_signal_connect(drawing_area2, "button-press-event", G_CALLBACK(on_button_press), NULL);
    g_signal_connect(drawing_area2, "button-release-event", G_CALLBACK(on_button_release), NULL);
    g_signal_connect(drawing_area2, "motion-notify-event", G_CALLBACK(on_motion_notify_knob), knob2);

    gtk_widget_add_events(drawing_area2, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);

    gtk_widget_show_all(window);
}
