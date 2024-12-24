#include "custom_knob.h"

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    KnobData *knob_data = (KnobData *)user_data;

    int width, height;
    gtk_widget_get_size_request(widget, &width, &height);

    // Center coordinates
    double center_x = width / 2.0;
    double center_y = height / 2.0;

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

static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    KnobData *knob_data = (KnobData *)user_data;
    knob_data->is_dragging = TRUE;
    return TRUE;
}

static gboolean on_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    KnobData *knob_data = (KnobData *)user_data;
    knob_data->is_dragging = FALSE;
    return TRUE;
}

static gboolean on_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer user_data) {
    KnobData *knob_data = (KnobData *)user_data;

    if (knob_data->is_dragging) {
        int width, height;
        gtk_widget_get_size_request(widget, &width, &height);
        double center_x = width / 2.0;
        double center_y = height / 2.0;

        double dx = event->x - center_x;
        double dy = center_y - event->y;

        knob_data->angle = atan2(dy, dx);
        knob_data->value = ((knob_data->angle + M_PI) / (2 * M_PI)) * (VALUE_MAX - VALUE_MIN) + VALUE_MIN;

        gtk_widget_queue_draw(widget); // Redraw the widget
    }
    return TRUE;
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Knob Example");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

    KnobData *knob_data = g_new0(KnobData, 1);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 200, 200);
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), knob_data);
    g_signal_connect(drawing_area, "button-press-event", G_CALLBACK(on_button_press), knob_data);
    g_signal_connect(drawing_area, "button-release-event", G_CALLBACK(on_button_release), knob_data);
    g_signal_connect(drawing_area, "motion-notify-event", G_CALLBACK(on_motion_notify), knob_data);

    gtk_widget_add_events(drawing_area, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);

    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.example.knob", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
