#include "custom_knob.h"

typedef struct {
    double x;             // Center x-position of the knob
    double y;             // Center y-position of the knob
    double angle;         // Current angle in radians
    gboolean is_dragging; // Whether the knob is being dragged
} KnobData;

// Shared list of all knobs
GList *knobs = NULL;

// Draw all knobs in the list
static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    GList *node = knobs;
    while (node) {
        KnobData *knob_data = (KnobData *)node->data;

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

        node = node->next;
    }
    return FALSE;
}

// Detect motion and update the knob being dragged
static gboolean on_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer user_data) {
    GList *node = knobs;
    while (node) {
        KnobData *knob_data = (KnobData *)node->data;
        if (knob_data->is_dragging) {
            double dx = event->x - knob_data->x;
            double dy = knob_data->y - event->y;

            knob_data->angle = atan2(dy, dx);
            gtk_widget_queue_draw(widget); // Redraw the widget
            break;
        }
        node = node->next;
    }
    return TRUE;
}

// Detect when a knob is clicked
static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    GList *node = knobs;
    while (node) {
        KnobData *knob_data = (KnobData *)node->data;

        // Check if click is within knob's radius
        double dx = event->x - knob_data->x;
        double dy = event->y - knob_data->y;
        if (sqrt(dx * dx + dy * dy) <= KNOB_RADIUS) {
            knob_data->is_dragging = TRUE;
            return TRUE;
        }
        node = node->next;
    }
    return FALSE;
}

// Detect when the knob drag is released
static gboolean on_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    GList *node = knobs;
    while (node) {
        KnobData *knob_data = (KnobData *)node->data;
        knob_data->is_dragging = FALSE;
        node = node->next;
    }
    return TRUE;
}

// Create and add a new knob
void add_knob(double x, double y) {
    KnobData *knob = g_new0(KnobData, 1);
    knob->x = x;
    knob->y = y;
    knob->angle = 0.0;
    knob->is_dragging = FALSE;
    knobs = g_list_append(knobs, knob);
}

// Main application
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Knob Example");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 400, 400);
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    add_knob(100, 100); // Add first knob
    add_knob(300, 100); // Add second knob

    // Connect signals
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), NULL);
    g_signal_connect(drawing_area, "button-press-event", G_CALLBACK(on_button_press), NULL);
    g_signal_connect(drawing_area, "button-release-event", G_CALLBACK(on_button_release), NULL);
    g_signal_connect(drawing_area, "motion-notify-event", G_CALLBACK(on_motion_notify), NULL);

    gtk_widget_add_events(drawing_area, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
    gtk_widget_show_all(window);
}
