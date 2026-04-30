#include <gtk/gtk.h>
#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/msgina.h>

#include "balloon.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_0,
    PROP_ALERT_TYPE,
    PROP_TARGET_WIDGET,
    N_PROPERTIES
};

//
// FORWARD DECLARATIONS
//
static void wintc_welcome_balloon_constructed(
    GObject* object
);
static void wintc_welcome_balloon_finalize(
    GObject* object
);
static void wintc_welcome_balloon_get_property(
    GObject*    object, 
    guint       prop_id,
    GValue*     value, 
    GParamSpec* pspec
);
static void wintc_welcome_balloon_set_property(
    GObject*      object, 
    guint         prop_id, 
    const GValue* value, 
    GParamSpec*   pspec
);

static gboolean on_draw_area_arrow_draw(
    GtkWidget* widget,
    cairo_t*   cr,
    gpointer   data
);
static void on_target_widget_destroyed(
    gpointer data, 
    GObject *destroyed_object
);

//
// STATIC DATA
//
static GParamSpec* wintc_welcome_balloon_properties[N_PROPERTIES];

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCWelcomeBalloonClass
{
    GtkBoxClass __parent__;
};

struct _WinTCWelcomeBalloon {
    GtkBox __parent__;

    BalloonType alert_type; 
    GtkWidget* target_widget; 

    GtkWidget* icon;
    GtkWidget* title;
    GtkWidget* message;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCWelcomeBalloon,
    wintc_welcome_balloon,
    GTK_TYPE_BOX
)

GType balloon_type_get_type(void)
{
    static GType type = 0;

    if (type == 0)
    {
        static const GEnumValue values[] = {
            { BALLOON_TYPE_ERROR,   "BALLOON_TYPE_ERROR",   "error" },
            { BALLOON_TYPE_WARNING, "BALLOON_TYPE_WARNING", "warning" },
            { 0, NULL, NULL }
        };

        type = g_enum_register_static("BalloonType", values);
    }

    return type;
}

static void wintc_welcome_balloon_class_init(
    WinTCWelcomeBalloonClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);
    
    object_class->set_property = wintc_welcome_balloon_set_property;
    object_class->constructed  = wintc_welcome_balloon_constructed;
    object_class->get_property = wintc_welcome_balloon_get_property;
    object_class->finalize     = wintc_welcome_balloon_finalize;
    
    wintc_welcome_balloon_properties[PROP_ALERT_TYPE] =
        g_param_spec_enum(
            "alert-type",
            "Alert Type",
            "The type of alert (error or warning)",
            balloon_type_get_type(), 
            BALLOON_TYPE_WARNING,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
        );

    wintc_welcome_balloon_properties[PROP_TARGET_WIDGET] =
        g_param_spec_object(
            "target-widget",
            "Target Widget",
            "The widget that this balloon is associated with",
            GTK_TYPE_WIDGET,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_welcome_balloon_properties
    );
}

static void wintc_welcome_balloon_init(
    WinTCWelcomeBalloon* welcome_balloon
)
{
    welcome_balloon->alert_type    = BALLOON_TYPE_WARNING;
    welcome_balloon->target_widget = NULL;

    gtk_orientable_set_orientation(
        GTK_ORIENTABLE(welcome_balloon),
        GTK_ORIENTATION_VERTICAL
    );

    // Construct the interface for the balloon
    //
    GtkBuilder* builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/logonui/balloon.ui"
        );

    GtkWidget* arrow   = NULL;
    GtkWidget* overlay = NULL;

    wintc_builder_get_objects(
        builder,
        "img-icon",        &(welcome_balloon->icon),
        "label-title",     &(welcome_balloon->title),
        "label-message",   &(welcome_balloon->message),
        "overlay-main",    &overlay,
        "draw-area-arrow", &arrow,
        NULL
    );

    g_signal_connect(
        arrow,
        "draw",
        G_CALLBACK(on_draw_area_arrow_draw),
        NULL
    );

    gtk_box_pack_start(
        GTK_BOX(welcome_balloon),
        overlay,
        TRUE,
        TRUE,
        0
    );

    gtk_widget_show_all(GTK_WIDGET(welcome_balloon));

    g_object_unref(builder);
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_welcome_balloon_constructed(
    GObject* object
)
{
    G_OBJECT_CLASS(wintc_welcome_balloon_parent_class)
        ->constructed(object);

    WinTCWelcomeBalloon* welcome_balloon = WINTC_WELCOME_BALLOON(object);

    if (
        !welcome_balloon->target_widget ||
        !GTK_IS_WIDGET(welcome_balloon->target_widget)
    )
    {
        g_warning(
            "%s",
            "logonui: Balloon target widget is not set correctly or doesn't "
            "exist. This balloon will not be positioned correctly."
        );
    } 

    switch (welcome_balloon->alert_type)
    {
        case BALLOON_TYPE_ERROR:
            gtk_image_set_from_resource(
                GTK_IMAGE(welcome_balloon->icon),
                "/uk/oddmatics/wintc/logonui/error.png"
            );

            gtk_label_set_text(
                GTK_LABEL(welcome_balloon->title),
                "Did you forget your password?"
            );
            gtk_label_set_text(
                GTK_LABEL(welcome_balloon->message),
                "Please type your password again.\n"
                "Be sure to use the correct uppercase and lowercase letters."
            );

            break;

        case BALLOON_TYPE_WARNING:
            gtk_image_set_from_resource(
                GTK_IMAGE(welcome_balloon->icon),
                "/uk/oddmatics/wintc/logonui/exclamation.png"
            );

            gtk_label_set_text(
                GTK_LABEL(welcome_balloon->title),
                "Caps Lock is On"
            );
            gtk_label_set_text(
                GTK_LABEL(welcome_balloon->message),
                "Having Caps Lock on may cause you to enter your password "
                "incorrectly.\n\n"
                "You should press Caps Lock to turn it off before entering "
                "your password."
            );

            break;

        default:
            g_warning("Unknown alert type: %d", welcome_balloon->alert_type);
            break;
    }
}

static void wintc_welcome_balloon_finalize(
    GObject* object
)
{
    WinTCWelcomeBalloon* welcome_balloon = WINTC_WELCOME_BALLOON(object);

    if (welcome_balloon->target_widget)
    {
        g_object_weak_unref(
            G_OBJECT(welcome_balloon->target_widget),
            (GWeakNotify) on_target_widget_destroyed,
            welcome_balloon
        );
    }

    G_OBJECT_CLASS(wintc_welcome_balloon_parent_class)->finalize(object);
}

static void wintc_welcome_balloon_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCWelcomeBalloon *welcome_balloon = WINTC_WELCOME_BALLOON(object);

    switch (prop_id)
    {
        case PROP_ALERT_TYPE:
            g_value_set_enum(value, welcome_balloon->alert_type);
            break;

        case PROP_TARGET_WIDGET:
            g_value_set_object(value, welcome_balloon->target_widget);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_welcome_balloon_set_property(
    GObject*      object,
    guint         prop_id, 
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCWelcomeBalloon *welcome_balloon = WINTC_WELCOME_BALLOON(object);
    
    switch (prop_id)
    {
        case PROP_ALERT_TYPE:
            welcome_balloon->alert_type = g_value_get_enum(value);
            break;

        case PROP_TARGET_WIDGET:
            welcome_balloon->target_widget = g_value_get_object(value);

            if (welcome_balloon->target_widget)
            {
                g_object_weak_ref(
                    G_OBJECT(welcome_balloon->target_widget),
                    (GWeakNotify) on_target_widget_destroyed,
                    welcome_balloon
                );
            }

            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_welcome_balloon_new_with_type(
    BalloonType type,
    GtkWidget*  target_widget
)
{
    g_return_val_if_fail(
        type == BALLOON_TYPE_ERROR || type == BALLOON_TYPE_WARNING,
        NULL
    );
    g_return_val_if_fail(
        GTK_IS_WIDGET(target_widget),
        NULL
    );

    WinTCWelcomeBalloon* balloon =
        g_object_new(
            wintc_welcome_balloon_get_type(), 
            "alert-type",    type,
            "target-widget", target_widget,
            NULL
        );

    return GTK_WIDGET(balloon);
}

//
// CALLBACKS
//
static gboolean on_draw_area_arrow_draw(
    WINTC_UNUSED(GtkWidget* widget),
    cairo_t* cr,
    WINTC_UNUSED(gpointer data)
)
{
    // FIXME: In future the baloon should be able to show the arrow on the
    //        left or right side
    //
    gboolean show_right = TRUE;

    int x_start;
    int x_end;

    int height = 20;
    int width  = 12;

    if (show_right)
    {
        x_start = 10;
        x_end   = x_start + width;
    }
    else
    {
        x_start = 300;
        x_end   = x_start - width;
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

static void on_target_widget_destroyed(
    gpointer data,
    GObject* destroyed_object
)
{
    WinTCWelcomeBalloon* welcome_balloon = WINTC_WELCOME_BALLOON(data);

    if (welcome_balloon->target_widget == GTK_WIDGET(destroyed_object))
    {
        welcome_balloon->target_widget = NULL;
        gtk_widget_destroy(GTK_WIDGET(welcome_balloon));
    }
}
