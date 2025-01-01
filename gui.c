#include "gui.h"

// Helper function to convert angle to parameter value
static float angle_to_value(double angle, float min_val, float max_val) {
    // Convert angle from [-π, π] to [0, 1]
    float normalized = (angle + M_PI) / (2 * M_PI);
    // Map to parameter range
    return min_val + normalized * (max_val - min_val);
}

// Helper function to update parameter display
static void update_parameter_display(GtkWidget *label, const char *param_name, float value) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s: %.2f", param_name, value);
    gtk_label_set_text(GTK_LABEL(label), buffer);
}

// Fixed knob adjustment callback to match the header declaration
void on_knob_adjusted(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    // Only handle motion events
    if (event->type != GDK_MOTION_NOTIFY) return;
    
    GUIWidgets *widgets = (GUIWidgets *)user_data;
    KnobData *knob_data = g_object_get_data(G_OBJECT(widget), "knob-data");
    
    if (!knob_data || !knob_data->is_dragging) return;
    
    // Get motion event data
    GdkEventMotion *motion_event = (GdkEventMotion *)event;
    
    // Calculate angle based on mouse position
    double dx = motion_event->x - knob_data->x;
    double dy = knob_data->y - motion_event->y;
    double new_angle = atan2(dy, dx);
    
    // Detect full rotation and reset if necessary
    if (fabs(new_angle - knob_data->angle) > M_PI) {
        // Reset to initial position
        knob_data->angle = 0;
        new_angle = 0;
    } else {
        knob_data->angle = new_angle;
    }

    // Update the appropriate parameter based on which knob was adjusted
    if (widget == widgets->knob_pitch) {
        widgets->pitch = angle_to_value(new_angle, 0.25, 4.0);
        update_parameter_display(widgets->value_pitch, "Pitch", widgets->pitch);
        widgets->mod_params->pitch_factor = widgets->pitch;
    } else if (widget == widgets->knob_speed) {
        widgets->speed = angle_to_value(new_angle, 0.5, 2.0);
        update_parameter_display(widgets->value_speed, "Speed", widgets->speed);
        widgets->mod_params->speed_factor = widgets->speed;
    } else if (widget == widgets->knob_echo) {
        widgets->echo = angle_to_value(new_angle, 0.0, 1.0);
        update_parameter_display(widgets->value_echo, "Echo", widgets->echo);
        widgets->mod_params->echo_intensity = widgets->echo;
    } else if (widget == widgets->knob_reverb) {
        widgets->reverb = angle_to_value(new_angle, 0.0, 1.0);
        update_parameter_display(widgets->value_reverb, "Reverb", widgets->reverb);
        widgets->mod_params->reverb_intensity = widgets->reverb;
    }
    
    gtk_widget_queue_draw(widget);
}

// Modified reset function to prevent segfault
void on_reset_clicked(GtkWidget *widget, gpointer user_data) {
    GUIWidgets *widgets = (GUIWidgets *)user_data;
    if (!widgets) return;
    
    // Reset all parameters to default values
    widgets->pitch = 1.0f;
    widgets->speed = 1.0f;
    widgets->echo = 0.0f;
    widgets->reverb = 0.0f;
    
    // Update modulation parameters
    if (widgets->mod_params) {
        widgets->mod_params->pitch_factor = widgets->pitch;
        widgets->mod_params->speed_factor = widgets->speed;
        widgets->mod_params->echo_intensity = widgets->echo;
        widgets->mod_params->reverb_intensity = widgets->reverb;
    }
    
    // Update displays
    if (widgets->value_pitch) 
        update_parameter_display(widgets->value_pitch, "Pitch", widgets->pitch);
    if (widgets->value_speed)
        update_parameter_display(widgets->value_speed, "Speed", widgets->speed);
    if (widgets->value_echo)
        update_parameter_display(widgets->value_echo, "Echo", widgets->echo);
    if (widgets->value_reverb)
        update_parameter_display(widgets->value_reverb, "Reverb", widgets->reverb);
    
    // Reset knob positions
    GtkWidget *knobs[] = {
        widgets->knob_pitch,
        widgets->knob_speed,
        widgets->knob_echo,
        widgets->knob_reverb
    };
    
    for (int i = 0; i < 4; i++) {
        if (knobs[i]) {
            KnobData *knob_data = g_object_get_data(G_OBJECT(knobs[i]), "knob-data");
            if (knob_data) {
                knob_data->angle = 0;
                gtk_widget_queue_draw(knobs[i]);
            }
        }
    }
}

// Modified init_gui function
int init_gui(int *argc, char ***argv, GUIWidgets *widgets, ModulationParams *mod_params) {
    gtk_init(argc, argv);
    
    // Initialize parameter values
    widgets->pitch = 1.0f;
    widgets->speed = 1.0f;
    widgets->echo = 0.0f;
    widgets->reverb = 0.0f;
    widgets->mod_params = mod_params;
    
    // Create main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Voice Modulator");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 400);
    
    // Disable window resizing
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    
    // Set window to be non-resizable but allow minimize
    GdkGeometry geometry;
    geometry.min_width = 800;
    geometry.min_height = 400;
    geometry.max_width = 800;
    geometry.max_height = 400;
    gtk_window_set_geometry_hints(GTK_WINDOW(window), NULL, &geometry, 
                                GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE);
    
    // Create main vertical box
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    // Create header
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), "<span size='x-large' weight='bold'>Voice Modulator</span>");
    gtk_box_pack_start(GTK_BOX(vbox), header, FALSE, FALSE, 10);
    
    // Create horizontal box for knobs
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 10);
    
    // Create knob containers
    GtkWidget **knobs[] = {&widgets->knob_pitch, &widgets->knob_speed, 
                          &widgets->knob_echo, &widgets->knob_reverb};
    GtkWidget **values[] = {&widgets->value_pitch, &widgets->value_speed, 
                           &widgets->value_echo, &widgets->value_reverb};
    const char *labels[] = {"Pitch", "Speed", "Echo", "Reverb"};
    
    for (int i = 0; i < 4; i++) {
        GtkWidget *knob_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        gtk_box_pack_start(GTK_BOX(hbox), knob_vbox, TRUE, TRUE, 10);
        
        // Add label
        GtkWidget *label = gtk_label_new(labels[i]);
        gtk_box_pack_start(GTK_BOX(knob_vbox), label, FALSE, FALSE, 0);
        
        // Create and add knob
        *knobs[i] = gtk_drawing_area_new();
        gtk_widget_set_size_request(*knobs[i], 100, 100);
        gtk_box_pack_start(GTK_BOX(knob_vbox), *knobs[i], TRUE, TRUE, 0);
        
        // Create value display
        *values[i] = gtk_label_new("");
        update_parameter_display(*values[i], labels[i], 
            (i == 0) ? 1.0f : (i == 1) ? 1.0f : 0.0f); // Set default values
        gtk_box_pack_start(GTK_BOX(knob_vbox), *values[i], FALSE, FALSE, 0);
        
        // Initialize knob data with default angle
        KnobData *knob_data = g_new0(KnobData, 1);
        knob_data->x = 50;
        knob_data->y = 50;
        knob_data->angle = 0.0;
        knob_data->is_dragging = FALSE;
        g_object_set_data_full(G_OBJECT(*knobs[i]), "knob-data", knob_data, g_free);
        
        // Connect signals
        g_signal_connect(*knobs[i], "draw", G_CALLBACK(on_draw_knob), knob_data);
        g_signal_connect(*knobs[i], "button-press-event", G_CALLBACK(on_button_press), knob_data);
        g_signal_connect(*knobs[i], "button-release-event", G_CALLBACK(on_button_release), knob_data);
        g_signal_connect(*knobs[i], "motion-notify-event", G_CALLBACK(on_knob_adjusted), widgets);
        
        gtk_widget_add_events(*knobs[i], 
            GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
    }
    
    // Add Reset button
    GtkWidget *reset_button = gtk_button_new_with_label("Reset All");
    gtk_box_pack_end(GTK_BOX(vbox), reset_button, FALSE, FALSE, 10);
    g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_clicked), widgets);
    
    // Connect window close event
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    // Show all widgets
    gtk_widget_show_all(window);
    
    return 0;
}

void start_gui() {
    gtk_main();
}