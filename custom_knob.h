#include <gtk/gtk.h>
#include <math.h>

#define KNOB_RADIUS 30 // Define the radius of the knob
#define VALUE_MIN 0
#define VALUE_MAX 100

typedef struct {
    double x;             // Center x-position of the knob
    double y;             // Center y-position of the knob
    double angle;         // Current angle in radians
    gboolean is_dragging; // Whether the knob is being dragged
} KnobData;

gboolean on_draw_knob(GtkWidget *widget, cairo_t *cr, gpointer user_data);
KnobData *add_knob(double x, double y);
gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean on_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean on_motion_notify_knob(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
void activate(GtkApplication *app, gpointer user_data);
void update_knob(int knob_index, double x, double y, double angle);