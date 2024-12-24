#include "custom_knob.h"

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    KnobData *knob_data = (KnobData *)user_data;

    // Use the provided x and y as the knob center
    double center_x = knob_data->x;
    double center_y = knob_data->y;

    // Draw knob circle
    cairo_arc(cr, center_x, center_y, KNOB_RADIUS, 0, 2 * M_PI);
    cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
    cairo_fill_preserve(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_stroke(cr);

    // Draw indicator
    double indicator_x = center_x + KNOB_RADIUS * cos(knob_data->angle);
    double indicator_y = center_y - KNOB_RADIUS * sin(knob_data->angle);
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    cairo_move_to(cr, center_x, center_y);
    cairo_line_to(cr, indicator_x, indicator_y);
    cairo_stroke(cr);

    return FALSE;
}

static gboolean on_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer user_data) {
    KnobData *knob_data = (KnobData *)user_data;

    if (knob_data->is_dragging) {
        double dx = event->x - knob_data->x;
        double dy = knob_data->y - event->y;

        knob_data->angle = atan2(dy, dx);
        knob_data->value = ((knob_data->angle + M_PI) / (2 * M_PI)) * (VALUE_MAX - VALUE_MIN) + VALUE_MIN;

        gtk_widget_queue_draw(widget); // Redraw the widget
    }
    return TRUE;
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Knob Example");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400); // Increased size to accommodate multiple knobs

    // Create multiple knobs
    KnobData *knob1 = g_new0(KnobData, 1);
    knob1->x = 100; // Set position for the first knob
    knob1->y = 100;

    KnobData *knob2 = g_new0(KnobData, 1);
    knob2->x = 300; // Set position for the second knob
    knob2->y = 100;

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 400, 400);
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    // Connect signals for the knobs
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), knob1); // Modify to loop for multiple knobs
    g_signal_connect(drawing_area, "button-press-event", G_CALLBACK(on_button_press), knob1);
    g_signal_connect(drawing_area, "button-release-event", G_CALLBACK(on_button_release), knob1);
    g_signal_connect(drawing_area, "motion-notify-event", G_CALLBACK(on_motion_notify), knob1);

    gtk_widget_add_events(drawing_area, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);

    gtk_widget_show_all(window);
}
