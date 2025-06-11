#include "balloon.h"

struct _Balloon {
    GtkWindow parent_instance;

    BalloonType alert_type; 
    GtkWidget* target_widget; 

    gboolean show_right;

    GtkWidget* icon;
    GtkWidget* title;
    GtkWidget* message;
};

G_DEFINE_TYPE(Balloon, balloon, GTK_TYPE_WINDOW)

enum {
    PROP_0,
    PROP_ALERT_TYPE,
    PROP_TARGET_WIDGET,
    N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

GType balloon_type_get_type(void) {
    static GType type = 0;
    if (type == 0) {
        static const GEnumValue values[] = {
            { BALLOON_TYPE_ERROR, "BALLOON_TYPE_ERROR", "error" },
            { BALLOON_TYPE_WARNING, "BALLOON_TYPE_WARNING", "warning" },
            { 0, NULL, NULL }
        };
        type = g_enum_register_static("BalloonType", values);
    }
    return type;
}


static void balloon_set_property(GObject *object, guint prop_id, 
                            const GValue *value, GParamSpec *pspec);
static void balloon_get_property(GObject *object, guint prop_id,
                            GValue *value, GParamSpec *pspec);
static void balloon_constructed(GObject *object);
static void balloon_finalize(GObject *object);

static void on_target_widget_destroyed(gpointer data, GObject *destroyed_object);
gboolean can_show_right(GtkWidget *target_widget);
static void configure_base_balloon(GtkWidget *widget);
static void configure_balloon_style(Balloon *self);

static void balloon_class_init(BalloonClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    
    object_class->set_property = balloon_set_property;
    object_class->constructed = balloon_constructed;
    object_class->get_property = balloon_get_property;
    object_class->finalize = balloon_finalize;
    
    properties[PROP_ALERT_TYPE] = g_param_spec_enum(
        "alert-type",
        "Alert Type",
        "The type of alert (error or warning)",
        balloon_type_get_type(), 
        BALLOON_TYPE_WARNING,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
    );

    properties[PROP_TARGET_WIDGET] = g_param_spec_object(
        "target-widget",
        "Target Widget",
        "The widget that this balloon is associated with",
        GTK_TYPE_WIDGET,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
    );

    g_object_class_install_properties(object_class, N_PROPERTIES, properties);
}


static void
balloon_set_property(GObject *object, guint prop_id, 
                            const GValue *value, GParamSpec *pspec)
{
    Balloon *self = BALLOON_WIDGET(object);
    
    switch (prop_id) {
        case PROP_ALERT_TYPE:
            self->alert_type = g_value_get_enum(value);
            break;
        case PROP_TARGET_WIDGET:
            self->target_widget = g_value_get_object(value);
            if (self->target_widget) {
                g_object_weak_ref(G_OBJECT(self->target_widget), 
                                 (GWeakNotify)on_target_widget_destroyed, self);
            }
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void
balloon_get_property(GObject *object, guint prop_id,
                            GValue *value, GParamSpec *pspec)
{
    Balloon *self = BALLOON_WIDGET(object);
    switch (prop_id) {
        case PROP_ALERT_TYPE:
            g_value_set_enum(value, self->alert_type);
            break;
        case PROP_TARGET_WIDGET:
            g_value_set_object(value, self->target_widget);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void balloon_finalize(GObject *object) {
    Balloon *self = BALLOON_WIDGET(object);
    if (self->target_widget) {
        g_object_weak_unref(G_OBJECT(self->target_widget), 
                           (GWeakNotify)on_target_widget_destroyed, self);
    }
    G_OBJECT_CLASS(balloon_parent_class)->finalize(object);
}

static void balloon_init(Balloon *self) {
    self->alert_type = BALLOON_TYPE_WARNING;
    self->target_widget = NULL;
    self->show_right = FALSE;

    gtk_window_set_decorated(GTK_WINDOW(self), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(self), FALSE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(self), TRUE);
    gtk_window_set_keep_above(GTK_WINDOW(self), TRUE);

    // Enable transparency
    gtk_widget_set_app_paintable(GTK_WIDGET(self), TRUE);
    GdkScreen *screen = gtk_widget_get_screen(GTK_WIDGET(self));
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
    if (visual != NULL) {
        gtk_widget_set_visual(GTK_WIDGET(self), visual);
    }
}

GtkWidget *
balloon_new(BalloonType type, GtkWidget* target_widget) {
    g_return_val_if_fail(type == BALLOON_TYPE_ERROR || type == BALLOON_TYPE_WARNING, NULL);
    g_return_val_if_fail(GTK_IS_WIDGET(target_widget), NULL);

    Balloon *balloon = g_object_new(BALLOON_TYPE_CONTAINER, 
                                    "type", GTK_WINDOW_POPUP,
                                    "alert-type", type,
                                    "target-widget", target_widget,
                                    NULL);
    
    balloon->show_right = can_show_right(target_widget);

    return GTK_WIDGET(balloon);
}


static gboolean draw_arrow(GtkWidget *widget, cairo_t *cr, gpointer data) {
    gboolean show_right = (gboolean) data;

    int x_start;
    int x_end;

    int height = 20;
    int width = 12;

    if (show_right) {
        x_start = 10;
        x_end = x_start + width;
    } else {
        x_start = 300;
        x_end = x_start - width;
    }
    
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgba(cr, 1.0, 1.0, 0.882, 1.0);
    cairo_move_to(cr, x_start, 0);
    cairo_line_to(cr, x_end, height);
    cairo_line_to(cr, x_start, height);
    cairo_close_path(cr);
    cairo_fill(cr);

    cairo_set_line_width(cr, 0.8);
    cairo_set_source_rgba(cr, 0, 0, 0, 1.0); 
    cairo_move_to(cr, x_start, 0);      

    cairo_line_to(cr, x_end, height);     
    cairo_move_to(cr, x_start, 0);      
    cairo_line_to(cr, x_start, height);    

    cairo_stroke(cr);

    return FALSE;
}

GtkWidget* balloon_create_arrow_widget(gboolean show_right) {
    GtkWidget *area = gtk_drawing_area_new();
    gtk_widget_set_hexpand(area, TRUE);
    gtk_widget_set_vexpand(area, FALSE);
    gtk_widget_set_size_request(area, 20, 20);
    g_signal_connect(area, "draw", G_CALLBACK(draw_arrow), show_right);
    return area;
}

gboolean can_show_right(GtkWidget *target_widget) {
    GdkDisplay *display = gdk_display_get_default();
    GdkMonitor *monitor = gdk_display_get_primary_monitor(display);
    int display_width = 0;

    if (monitor) {
        GdkRectangle geometry;
        gdk_monitor_get_geometry(monitor, &geometry);
        display_width = geometry.width;
    }

    GtkAllocation allocation;
    gtk_widget_get_allocation(target_widget, &allocation);

    int x, y;
    GdkWindow* window = gtk_widget_get_window(target_widget);
    gdk_window_get_origin(window, &x, &y);

    return (display_width - x) > 305;
}

static void configure_base_balloon(GtkWidget *widget) {
    Balloon *self = BALLOON_WIDGET(widget);

    if (!self->target_widget || !GTK_IS_WIDGET(self->target_widget)) {
        g_warning("Balloon target widget is not set correctly or doesn't exist. This balloon will not be positioned correctly.");
    } 

    gboolean show_right = can_show_right(self->target_widget);

    GtkWidget* overlay = gtk_overlay_new();

    GtkBox *box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
    gtk_style_context_add_class(gtk_widget_get_style_context(box), "balloon-tooltip");
    gtk_widget_set_valign(GTK_WIDGET(box), GTK_ALIGN_START);
    gtk_widget_set_margin_top(GTK_WIDGET(box), 19);

    GtkBox *header = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
    self->icon = gtk_image_new();
    self->title = gtk_label_new("");
    gtk_widget_set_valign(GTK_WIDGET(self->title), GTK_ALIGN_START);
    gtk_widget_set_halign(GTK_WIDGET(self->title), GTK_ALIGN_START);
    gtk_style_context_add_class(gtk_widget_get_style_context(self->title), "error-popup-title");

    gtk_box_pack_start(GTK_BOX(header), GTK_WIDGET(self->icon), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(header), GTK_WIDGET(self->title), TRUE, TRUE, 0);

    GtkBox* content = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_widget_set_margin_top(GTK_WIDGET(content), 5);
    gtk_style_context_add_class(gtk_widget_get_style_context(content), "error-popup-content");

    self->message = gtk_label_new("");
    gtk_label_set_max_width_chars(GTK_LABEL(self->message), 50);
    gtk_label_set_xalign(GTK_LABEL(self->message), 0.0f);
    gtk_label_set_line_wrap(GTK_LABEL(self->message), TRUE);
    gtk_label_set_line_wrap_mode(GTK_LABEL(self->message), PANGO_WRAP_WORD); 
    gtk_label_set_justify(GTK_LABEL(self->message), GTK_JUSTIFY_LEFT);        
    gtk_widget_set_halign(self->message, GTK_ALIGN_START);
    gtk_widget_set_valign(self->message, GTK_ALIGN_START);

    gtk_box_pack_start(GTK_BOX(content), GTK_WIDGET(self->message), TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(header), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(content), TRUE, TRUE, 0);

    GtkWidget* arrow = balloon_create_arrow_widget(show_right);
    gtk_widget_set_halign(arrow, GTK_ALIGN_START);
    gtk_widget_set_valign(arrow, GTK_ALIGN_START);

    gtk_container_add(GTK_CONTAINER(overlay), GTK_WIDGET(box));
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), arrow);

    gtk_container_add(GTK_CONTAINER(self), overlay);
    
    gtk_widget_show_all(GTK_WIDGET(self));
}

static void configure_balloon_style(Balloon *self) {
    configure_base_balloon(self);
    switch (self->alert_type) {
        case BALLOON_TYPE_ERROR:
            gtk_image_set_from_file(GTK_IMAGE(self->icon), "src/res/error.png");
            gtk_label_set_text(GTK_LABEL(self->title), "Did you forget your password?");

            gtk_label_set_text(GTK_WIDGET(self->message), "Please type your password again.\n"
                                                          "Be sure to use the correct uppercase and lowercase letters.");
            break;
        case BALLOON_TYPE_WARNING:
            gtk_image_set_from_file(GTK_IMAGE(self->icon), "src/res/exclamation.png");
            gtk_label_set_text(GTK_LABEL(self->title), "Caps Lock is On");
            gtk_label_set_text(self->message, "Having Caps Lock on may cause you to enter your password incorrectly.\n\n"
                                                          "You should press Caps Lock to turn it off before entering your password.");
            break;
        default:
            g_warning("Unknown alert type: %d", self->alert_type);
            break;
    }
}

static void balloon_constructed(GObject *object) {
    Balloon *self = BALLOON_WIDGET(object);
    configure_balloon_style(self);


    G_OBJECT_CLASS(balloon_parent_class)->constructed(object);
}

static void on_target_widget_destroyed(gpointer data, GObject *destroyed_object) {
    Balloon *self = BALLOON_WIDGET(data);
    if (self->target_widget == destroyed_object) {
        self->target_widget = NULL;
        gtk_widget_destroy(GTK_WIDGET(self));
    }
}