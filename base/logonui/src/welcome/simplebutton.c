#include "simplebutton.h"

#define DEFAULT_PRESS_DELAY 150

//
// PRIVATE ENUMS
//
enum {
    PROP_0,
    PROP_ACTIVATED_PIXBUF,
    PROP_IDLE_PIXBUF,
    PROP_PRESS_DELAY,
    N_PROPERTIES
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _SimpleButton {
    GtkButton parent_instance;

    GdkPixbuf *idle_pixbuf;
    GdkPixbuf *activated_pixbuf;
    guint press_delay;
    guint timeout_id;
};

struct _SimpleButtonClass {
    GtkButtonClass parent_class;
};


//
// FORWARD DECLARATIONS
//
static void simple_button_finalize(GObject* object);
static void simple_button_set_property(GObject *object, guint prop_id, 
                            const GValue *value, GParamSpec *pspec);
static void simple_button_get_property(GObject *object, guint prop_id,
                            GValue *value, GParamSpec *pspec);
static void on_simple_button_clicked(GtkButton *button);

static void simple_button_set_image_from_pixbuf(SimpleButton *self, GdkPixbuf *pixbuf);
static gboolean simple_button_reset_image(gpointer user_data);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    SimpleButton, 
    simple_button, 
    GTK_TYPE_BUTTON
)

static GParamSpec *properties[N_PROPERTIES] = { NULL, };


static void simple_button_class_init(SimpleButtonClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkButtonClass *button_class = GTK_BUTTON_CLASS(klass);

    object_class->set_property = simple_button_set_property;
    object_class->get_property = simple_button_get_property;
    object_class->finalize = simple_button_finalize;

    button_class->clicked = on_simple_button_clicked;

    
    properties[PROP_ACTIVATED_PIXBUF] = g_param_spec_object(
        "activated-pixbuf",
        "ActivatedPixbuf",
        "The pixbuf displayed when the button is activated",
        GDK_TYPE_PIXBUF,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
    );

    properties[PROP_IDLE_PIXBUF] = g_param_spec_object(
        "idle-pixbuf",
        "IdlePixbuf",
        "The pixbuf displayed when the button is idle",
        GDK_TYPE_PIXBUF,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
    );

    properties[PROP_PRESS_DELAY] = g_param_spec_uint(
        "press-delay",
        "PressDelay",
        "Delay in milliseconds before reverting to idle pixbuf",
        0, G_MAXUINT, DEFAULT_PRESS_DELAY,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT
    );

    g_object_class_install_properties(object_class, N_PROPERTIES, properties);
}

static void simple_button_init(SimpleButton *self) {
    gtk_style_context_add_class(
        gtk_widget_get_style_context(GTK_WIDGET(self)),
        "plain-button"
    );
    
    self->press_delay = DEFAULT_PRESS_DELAY;
    self->timeout_id = 0;
}

static void simple_button_set_property(GObject *object, guint prop_id, 
                            const GValue *value, GParamSpec *pspec) {
    SimpleButton *self = SIMPLE_BUTTON(object);

    switch (prop_id) {
        case PROP_ACTIVATED_PIXBUF:
            if (self->activated_pixbuf) {
                g_object_unref(self->activated_pixbuf);
            }
            self->activated_pixbuf = g_value_dup_object(value);
            break;
        case PROP_IDLE_PIXBUF:
            if (self->idle_pixbuf) {
                g_object_unref(self->idle_pixbuf);
            }
            self->idle_pixbuf = g_value_dup_object(value);
            if (self->idle_pixbuf) {
                simple_button_set_image_from_pixbuf(self, self->idle_pixbuf);
            }
            break;
        case PROP_PRESS_DELAY:
            self->press_delay = g_value_get_uint(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void simple_button_get_property(GObject *object, guint prop_id,
                            GValue *value, GParamSpec *pspec) {
    SimpleButton *self = SIMPLE_BUTTON(object);

    switch (prop_id) {
        case PROP_ACTIVATED_PIXBUF:
            g_value_set_object(value, self->activated_pixbuf);
            break;
        case PROP_IDLE_PIXBUF:
            g_value_set_object(value, self->idle_pixbuf);
            break;
        case PROP_PRESS_DELAY:
            g_value_set_uint(value, self->press_delay);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// CLASS VIRTUAL METHODS
//
static void simple_button_finalize(GObject* object) {
    SimpleButton *self = SIMPLE_BUTTON(object);
    
    if (self->timeout_id > 0) {
        g_source_remove(self->timeout_id);
        self->timeout_id = 0;
    }
    
    if (self->idle_pixbuf) {
        g_object_unref(self->idle_pixbuf);
        self->idle_pixbuf = NULL;
    }
    
    if (self->activated_pixbuf) {
        g_object_unref(self->activated_pixbuf);
        self->activated_pixbuf = NULL;
    }
    
    G_OBJECT_CLASS(simple_button_parent_class)->finalize(object);
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* simple_button_new_with_pixbufs(GdkPixbuf *idle_pixbuf, 
                                           GdkPixbuf *activated_pixbuf) {
    return GTK_WIDGET(g_object_new(simple_button_get_type(),
                       "idle-pixbuf", idle_pixbuf,
                       "activated-pixbuf", activated_pixbuf,
                       NULL));
}

//
// PRIVATE FUNCTIONS
//
static void on_simple_button_clicked(GtkButton *button) {
    SimpleButton *self = SIMPLE_BUTTON(button);
    
    if (self->timeout_id > 0) {
        g_source_remove(self->timeout_id);
        self->timeout_id = 0;
    }
    
    if (self->activated_pixbuf) {
        simple_button_set_image_from_pixbuf(self, self->activated_pixbuf);
    }
    
    if (self->press_delay > 0) {
        self->timeout_id = g_timeout_add(self->press_delay, 
                                        simple_button_reset_image, self);
    }
}

static void simple_button_set_image_from_pixbuf(SimpleButton *self, GdkPixbuf *pixbuf) {
    if (!pixbuf) return;
    
    GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);
    gtk_button_set_image(GTK_BUTTON(self), image);
    gtk_widget_show(image);
}

static gboolean simple_button_reset_image(gpointer user_data) {
    SimpleButton *self = SIMPLE_BUTTON(user_data);
    
    if (self->idle_pixbuf) {
        simple_button_set_image_from_pixbuf(self, self->idle_pixbuf);
    }
    
    self->timeout_id = 0;
    return G_SOURCE_REMOVE;
}
