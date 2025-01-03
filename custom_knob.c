#include "custom_knob.h"
#include <math.h>
#include <stdlib.h>

// Global list of knobs
GList *knobs = NULL;

/**
 * Callback to draw a knob widget. This callback is registered with the
 * GtkWidget passed to add_knob() and is called whenever the widget needs
 * to be redrawn.
 *
 * This function draws a circle for the knob and a red line for the indicator.
 *
 * @param widget The GtkWidget being drawn.
 * @param cr The cairo context to draw with.
 * @param user_data The KnobData associated with the widget.
 *
 * @return FALSE, to indicate that the event has been handled.
 */
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

/**
 * Callback for the "motion-notify-event" signal. This callback is called when
 * the user moves the mouse while a knob is being dragged.
 *
 * This callback updates the angle of the knob based on the mouse position and
 * redraws the widget.
 *
 * @param widget The GtkWidget associated with the event.
 * @param event The GdkEventMotion associated with the event.
 * @param user_data The KnobData associated with the widget.
 *
 * @return TRUE to indicate that the event has been handled.
 */
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

/**
 * Callback for the "button-press-event" signal. This callback is called when
 * the user presses a mouse button while the pointer is over the widget.
 *
 * This callback checks if the click is within the radius of the knob and if
 * so, flags the knob as being dragged. It returns TRUE if the event is
 * handled, and FALSE otherwise.
 *
 * @param widget The GtkWidget associated with the event.
 * @param event The GdkEventButton associated with the event.
 * @param user_data The KnobData associated with the widget.
 *
 * @return TRUE if the event is handled, FALSE otherwise.
 */
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

/**
 * Callback for the "button-release-event" signal. This callback is called when
 * the user releases a mouse button while the pointer is over the widget.
 *
 * This callback unflags the knob as being dragged, indicating the end of a
 * drag operation. It returns TRUE to indicate that the event has been handled.
 *
 * @param widget The GtkWidget associated with the event.
 * @param event The GdkEventButton associated with the event.
 * @param user_data The KnobData associated with the widget.
 *
 * @return TRUE to indicate that the event has been handled.
 */
gboolean on_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    KnobData *knob_data = (KnobData *)user_data;
    knob_data->is_dragging = FALSE;
    return TRUE;  // Event handled
}

/**
 * Create a new knob at the specified position and return its KnobData.
 *
 * The returned KnobData is linked into the list of knobs and is ready to be
 * used with the other functions in this module.
 *
 * @param x The x-coordinate of the knob's center.
 * @param y The y-coordinate of the knob's center.
 *
 * @return The newly-created KnobData.
 */
KnobData *add_knob(double x, double y) {
    KnobData *knob = g_new0(KnobData, 1);
    knob->x = x;
    knob->y = y;
    knob->angle = 0.0;
    knob->is_dragging = FALSE;
    knobs = g_list_append(knobs, knob);
    return knob;
}

/**
 * Update the properties of a knob given its index in the list of knobs.
 *
 * @param knob_index The index of the knob in the list of knobs.
 * @param x The new x-coordinate of the knob's center.
 * @param y The new y-coordinate of the knob's center.
 * @param angle The new angle of the knob in radians.
 */
void update_knob(int knob_index, double x, double y, double angle) {
    GList *node = g_list_nth(knobs, knob_index);
    if (node) {
        KnobData *knob_data = (KnobData *)node->data;
        knob_data->x = x;
        knob_data->y = y;
        knob_data->angle = angle;
    }
}

/**
 * Activates the GTK application by creating a window with two knob widgets.
 *
 * This function sets up the main application window, initializes two
 * drawing areas for the knob widgets, and connects the necessary event
 * signals for handling user interactions with the knobs. The window is
 * displayed with a fixed size of 600x300 pixels.
 *
 * @param app The GtkApplication associated with the window.
 * @param user_data User data passed to the function.
 */

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
