#include <gtk/gtk.h>
#include <math.h>

#define KNOB_RADIUS 50.0
#define VALUE_MIN 0
#define VALUE_MAX 100

typedef struct {
    double value;        // Current value of the knob
    double angle;        // Current angle of the knob
    gboolean is_dragging; // Is the knob being dragged?
} KnobData;

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
static gboolean on_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
static gboolean on_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
static void activate(GtkApplication *app, gpointer user_data);