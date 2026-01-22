#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdarg.h>
#include <sys/param.h>
#include <wintc/comgtk.h>

#include "../public/wizpage.h"
#include "../public/wizwnd.h"

#define WIZARD97_WATERMARK_WIDTH  164
#define WIZARD97_WATERMARK_HEIGHT 314

#define WIZARD97_HEADERPIC_SIZE 49

#define WIZARD97_PAGE_NUM_FINAL 171717

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_CAN_CANCEL,
    PROP_CAN_PREV,
    PROP_CAN_NEXT,
    PROP_CAN_HELP,
    PROP_FINAL_PAGE,
    N_PROPERTIES
};

//
// FORWARD DECLARATIONS
//
static void wintc_wizard97_window_dispose(
    GObject* object
);
static void wintc_wizard97_window_finalize(
    GObject* object
);
static void wintc_wizard97_window_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_wizard97_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static gboolean wintc_wizard97_window_cancel(
    WinTCWizard97Window* wiz_wnd,
    guint                current_page
);
static void wintc_wizard97_window_constructing_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num,
    GtkBuilder*          builder
);
static gboolean wintc_wizard97_window_finish(
    WinTCWizard97Window* wiz_wnd,
    guint                current_page
);
static guint wintc_wizard97_window_get_next_page(
    WinTCWizard97Window* wiz_wnd,
    guint                current_page
);
static void wintc_wizard97_window_help(
    WinTCWizard97Window* wiz_wnd,
    guint                current_page
);

static GtkBuilder* wintc_wizard97_window_create_builder(
    WinTCWizard97Window* wiz_wnd
);
static GtkWidget* wintc_wizard97_window_create_page(
    WinTCWizard97Window* wiz_wnd,
    const gchar*         resource_path,
    guint                page_num
);
static GtkWidget* wintc_wizard97_window_get_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num
);
static void wintc_wizard97_window_go_to_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num,
    gboolean             push_history
);
static gboolean wintc_wizard97_window_is_on_final_page(
    WinTCWizard97Window* wiz_wnd
);

static void action_back(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_cancel(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_help(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_next(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);

static gboolean on_window_map_event(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);

//
// STATIC DATA
//
static GParamSpec* wintc_wizard97_window_properties[N_PROPERTIES] = { 0 };

static const GActionEntry S_ACTIONS[] = {
    {
        .name           = "back",
        .activate       = action_back,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "cancel",
        .activate       = action_cancel,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "help",
        .activate       = action_help,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "next",
        .activate       = action_next,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    }
};
static const gchar* S_ACTION_BINDINGS[] = {
    "back",   "can-prev",
    "cancel", "can-cancel",
    "help",   "can-help",
    "next",   "can-next"
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCWizard97WindowPrivate
{
    // State
    //
    guint   current_page;
    GSList* history;

    // UI stuff
    //
    GtkWidget* box_page_area;

    GtkWidget* box_tmpl_ext_page_area;
    GtkWidget* box_tmpl_int_page_area;
    GtkWidget* img_ext_watermark;
    GtkWidget* img_int_header;
    GtkWidget* label_int_subtitle;
    GtkWidget* label_int_title;

    // Stuff we maintain an additional ref to
    //
    GdkPixbuf* pixbuf_watermark;
    GdkPixbuf* pixbuf_header;

    GList* list_pages;

    GtkWidget* box_tmpl_ext_page;
    GtkWidget* box_tmpl_int_page;
} WinTCWizard97WindowPrivate;

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(
    WinTCWizard97Window,
    wintc_wizard97_window,
    GTK_TYPE_WINDOW
)

static void wintc_wizard97_window_class_init(
    WinTCWizard97WindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    klass->cancel              = wintc_wizard97_window_cancel;
    klass->constructing_page   = wintc_wizard97_window_constructing_page;
    klass->finish              = wintc_wizard97_window_finish;
    klass->help                = wintc_wizard97_window_help;
    klass->get_next_page       = wintc_wizard97_window_get_next_page;
    object_class->dispose      = wintc_wizard97_window_dispose;
    object_class->finalize     = wintc_wizard97_window_finalize;
    object_class->get_property = wintc_wizard97_window_get_property;
    object_class->set_property = wintc_wizard97_window_set_property;

    // Install properties
    //
    wintc_wizard97_window_properties[PROP_CAN_CANCEL] =
        g_param_spec_boolean(
            "can-cancel",
            "CanCancel",
            "Determines whether it is possible to cancel the wizard.",
            TRUE,
            G_PARAM_READABLE
        );
    wintc_wizard97_window_properties[PROP_CAN_PREV] =
        g_param_spec_boolean(
            "can-prev",
            "CanPrev",
            "Determines whether it is possible go backward in the wizard.",
            TRUE,
            G_PARAM_READABLE
        );
    wintc_wizard97_window_properties[PROP_CAN_NEXT] =
        g_param_spec_boolean(
            "can-next",
            "CanNext",
            "Determines whether it is possible to proceed in the wizard.",
            TRUE,
            G_PARAM_READABLE
        );
    wintc_wizard97_window_properties[PROP_CAN_HELP] =
        g_param_spec_boolean(
            "can-help",
            "CanHelp",
            "Determines whether it is possible to get help.",
            FALSE,
            G_PARAM_READABLE
        );
    wintc_wizard97_window_properties[PROP_FINAL_PAGE] =
        g_param_spec_boolean(
            "final-page",
            "FinalPage",
            "Determines whether the wizard is on a final page.",
            FALSE,
            G_PARAM_READABLE
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_wizard97_window_properties
    );

    // Load styles
    //
    GtkCssProvider* css_default   = gtk_css_provider_new();
    GtkCssProvider* css_default_p = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css_default,
        "/uk/oddmatics/wintc/wizard97/default.css"
    );
    gtk_css_provider_load_from_resource(
        css_default_p,
        "/uk/oddmatics/wintc/wizard97/default_p.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_default),
        GTK_STYLE_PROVIDER_PRIORITY_FALLBACK
    );
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_default_p),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

static void wintc_wizard97_window_init(
    WINTC_UNUSED(WinTCWizard97Window* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_wizard97_window_dispose(
    GObject* object
)
{
    WinTCWizard97WindowPrivate* priv =
        wintc_wizard97_window_get_instance_private(
            WINTC_WIZARD97_WINDOW(object)
        );

    g_clear_object(&(priv->pixbuf_watermark));
    g_clear_object(&(priv->pixbuf_header));
    g_clear_list(
        &(priv->list_pages),
        (GDestroyNotify) g_object_unref
    );

    (G_OBJECT_CLASS(wintc_wizard97_window_parent_class))
        ->dispose(object);
}

static void wintc_wizard97_window_finalize(
    GObject* object
)
{
    WinTCWizard97WindowPrivate* priv =
        wintc_wizard97_window_get_instance_private(
            WINTC_WIZARD97_WINDOW(object)
        );

    g_slist_free(
        g_steal_pointer(&(priv->history))
    );

    (G_OBJECT_CLASS(wintc_wizard97_window_parent_class))
        ->finalize(object);
}

static void wintc_wizard97_window_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCWizard97WindowPrivate* priv =
        wintc_wizard97_window_get_instance_private(
            WINTC_WIZARD97_WINDOW(object)
        );

    switch (prop_id)
    {
        case PROP_CAN_CANCEL:
            g_value_set_boolean(value, TRUE);
            break;

        case PROP_CAN_PREV:
            g_value_set_boolean(value, !!(priv->history));
            break;

        case PROP_CAN_NEXT:
            g_value_set_boolean(value, TRUE);
            break;

        case PROP_CAN_HELP:
            g_value_set_boolean(value, FALSE);
            break;

        case PROP_FINAL_PAGE:
            g_value_set_boolean(
                value,
                wintc_wizard97_window_is_on_final_page(
                    WINTC_WIZARD97_WINDOW(object)
                )
            );

            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_wizard97_window_set_property(
    GObject*    object,
    guint       prop_id,
    WINTC_UNUSED(const GValue* value),
    GParamSpec* pspec
)
{
    switch (prop_id)
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static gboolean wintc_wizard97_window_cancel(
    WINTC_UNUSED(WinTCWizard97Window* wiz_wnd),
    WINTC_UNUSED(guint                current_page)
)
{
    return TRUE;
}

static void wintc_wizard97_window_constructing_page(
    WINTC_UNUSED(WinTCWizard97Window* wiz_wnd),
    WINTC_UNUSED(guint                page_num),
    WINTC_UNUSED(GtkBuilder*          builder)
) {}

static gboolean wintc_wizard97_window_finish(
    WINTC_UNUSED(WinTCWizard97Window* wiz_wnd),
    WINTC_UNUSED(guint                current_page)
)
{
    return TRUE;
}

static guint wintc_wizard97_window_get_next_page(
    WinTCWizard97Window* wiz_wnd,
    guint                current_page
)
{
    WinTCWizard97WindowPrivate* priv =
        wintc_wizard97_window_get_instance_private(wiz_wnd);

    if (current_page + 1 >= g_list_length(priv->list_pages))
    {
        return WIZARD97_PAGE_NUM_FINAL;
    }

    return current_page + 1;
}

static void wintc_wizard97_window_help(
    WINTC_UNUSED(WinTCWizard97Window* wiz_wnd),
    WINTC_UNUSED(guint                current_page)
) {}

//
// PUBLIC FUNCTIONS
//
void wintc_wizard97_window_class_setup_from_resources(
    WinTCWizard97WindowClass* wizard_class,
    const gchar*              resource_watermark,
    const gchar*              resource_header,
    ...
)
{
    va_list ap;
    gchar*  next_res;

    wizard_class->resource_watermark = g_strdup(resource_watermark);
    wizard_class->resource_header    = g_strdup(resource_header);

    va_start(ap, resource_header);

    next_res = va_arg(ap, gchar*);

    while (next_res)
    {
        wizard_class->list_resources_pages =
            g_list_append(
                wizard_class->list_resources_pages,
                g_strdup(next_res)
            );

        next_res = va_arg(ap, gchar*);
    }

    va_end(ap);
}

void wintc_wizard97_window_init_wizard(
    WinTCWizard97Window* wiz_wnd
)
{
    WinTCWizard97WindowClass* wizard_class =
        WINTC_WIZARD97_WINDOW_GET_CLASS(wiz_wnd);

    WinTCWizard97WindowPrivate* priv =
        wintc_wizard97_window_get_instance_private(
            WINTC_WIZARD97_WINDOW(wiz_wnd)
        );

    GtkBuilder* builder;
    GError*     error = NULL;

    g_type_ensure(WINTC_TYPE_WIZARD97_PAGE);

    // General GtkWindow setup
    //
    gtk_window_set_resizable(GTK_WINDOW(wiz_wnd), FALSE);

    // Add actions
    //
    GSimpleActionGroup* action_group = g_simple_action_group_new();

    g_action_map_add_action_entries(
        G_ACTION_MAP(action_group),
        S_ACTIONS,
        G_N_ELEMENTS(S_ACTIONS),
        wiz_wnd
    );

    for (gsize i = 0; i < G_N_ELEMENTS(S_ACTION_BINDINGS); i += 2)
    {
        const gchar* action_name   = S_ACTION_BINDINGS[i];
        const gchar* property_name = S_ACTION_BINDINGS[i + 1];

        GAction* action =
            g_action_map_lookup_action(
                G_ACTION_MAP(action_group),
                action_name
            );

        g_object_bind_property(
            wiz_wnd,
            property_name,
            action,
            "enabled",
            G_BINDING_SYNC_CREATE
        );
    }

    gtk_widget_insert_action_group(
        GTK_WIDGET(wiz_wnd),
        "win",
        G_ACTION_GROUP(action_group)
    );

    g_object_unref(action_group);

    // External page template
    //
    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/wizard97/wizext.ui"
        );

    wintc_builder_get_objects(
        builder,
        "main-box",      &(priv->box_tmpl_ext_page),
        "box-page",      &(priv->box_tmpl_ext_page_area),
        "img-watermark", &(priv->img_ext_watermark),
        NULL
    );

    g_object_ref(priv->box_tmpl_ext_page);

    g_clear_object(&builder);

    // Internal page template
    //
    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/wizard97/wizint.ui"
        );

    wintc_builder_get_objects(
        builder,
        "main-box",       &(priv->box_tmpl_int_page),
        "box-page",       &(priv->box_tmpl_int_page_area),
        "img-header",     &(priv->img_int_header),
        "label-subtitle", &(priv->label_int_subtitle),
        "label-title",    &(priv->label_int_title),
        NULL
    );

    g_object_ref(priv->box_tmpl_int_page);

    g_clear_object(&builder);

    // Iterate over internal pages
    //
    GList* iter = wizard_class->list_resources_pages;

    for (guint i = 0; iter; iter = iter->next, i++)
    {
        GtkWidget* page =
            wintc_wizard97_window_create_page(
                wiz_wnd,
                (gchar*) iter->data,
                i
            );

        if (!page)
        {
            i--;
            continue;
        }

        if (
            wintc_wizard97_page_get_is_exterior_page(
                WINTC_WIZARD97_PAGE(page)
            )
        )
        {
            gtk_widget_set_size_request(
                page,
                317,
                193
            );
        }
        else
        {
            gtk_widget_set_size_request(
                page,
                317,
                143
            );
        }

        priv->list_pages =
            g_list_append(
                priv->list_pages,
                page
            );
    }

    // Pixbufs
    //
    if (wizard_class->resource_watermark)
    {
        GdkPixbuf* pixbuf_watermark_src =
            gdk_pixbuf_new_from_resource(
                wizard_class->resource_watermark,
                &error
            );

        if (pixbuf_watermark_src)
        {
            priv->pixbuf_watermark =
                gdk_pixbuf_scale_simple(
                    pixbuf_watermark_src,
                    WIZARD97_WATERMARK_WIDTH,
                    WIZARD97_WATERMARK_HEIGHT,
                    GDK_INTERP_NEAREST
                );

            g_object_unref(pixbuf_watermark_src);

            gtk_image_set_from_pixbuf(
                GTK_IMAGE(priv->img_ext_watermark),
                priv->pixbuf_watermark
            );
        }
        else
        {
            gtk_widget_set_size_request(
                priv->img_ext_watermark,
                WIZARD97_WATERMARK_WIDTH,
                WIZARD97_WATERMARK_HEIGHT
            );

            wintc_log_error_and_clear(&error);
        }
    }

    if (wizard_class->resource_header)
    {
        GdkPixbuf* pixbuf_header_src =
            gdk_pixbuf_new_from_resource(
                wizard_class->resource_header,
                &error
            );

        if (pixbuf_header_src)
        {
            priv->pixbuf_header =
                gdk_pixbuf_scale_simple(
                    pixbuf_header_src,
                    WIZARD97_HEADERPIC_SIZE,
                    WIZARD97_HEADERPIC_SIZE,
                    GDK_INTERP_NEAREST
                );

            g_object_unref(pixbuf_header_src);

            gtk_image_set_from_pixbuf(
                GTK_IMAGE(priv->img_int_header),
                priv->pixbuf_header
            );
        }
        else
        {
            gtk_widget_set_size_request(
                priv->img_int_header,
                WIZARD97_HEADERPIC_SIZE,
                WIZARD97_HEADERPIC_SIZE
            );

            wintc_log_error_and_clear(&error);
        }
    }

    // Set up the window
    //
    builder = gtk_builder_new();

    gtk_builder_expose_object(
        builder,
        "wizard",
        G_OBJECT(wiz_wnd)
    );

    gtk_builder_add_from_resource(
        builder,
        "/uk/oddmatics/wintc/wizard97/wizwnd.ui",
        NULL
    );

    priv->box_page_area =
        GTK_WIDGET(
            gtk_builder_get_object(builder, "box-page-area")
        );

    gtk_container_add(
        GTK_CONTAINER(wiz_wnd),
        GTK_WIDGET(
            gtk_builder_get_object(builder, "main-box")
        )
    );

    g_clear_object(&builder);

    // Unify page sizes - load each page, figure out the largest size, and make
    // that our size request so the window doesn't grow/shrink between pages
    //
    GtkRequisition req;
    GtkRequisition req_largest;

    iter = priv->list_pages;

    for (guint i = 0; iter; iter = iter->next, i++)
    {
        wintc_wizard97_window_go_to_page(
            wiz_wnd,
            i,
            FALSE
        );

        gtk_widget_get_preferred_size(
            priv->box_page_area,
            NULL,
            &req
        );

        req_largest.width  = MAX(req_largest.width, req.width);
        req_largest.height = MAX(req_largest.height, req.height);
    }

    gtk_widget_set_size_request(
        priv->box_page_area,
        req_largest.width,
        req_largest.height
    );

    // Connect to map event to finish start up after being shown
    //
    g_signal_connect(
        wiz_wnd,
        "map-event",
        G_CALLBACK(on_window_map_event),
        NULL
    );
}

//
// PRIVATE FUNCTIONS
//
static GtkBuilder* wintc_wizard97_window_create_builder(
    WinTCWizard97Window* wiz_wnd
)
{
    GtkBuilder* builder = gtk_builder_new();

    gtk_builder_expose_object(
        builder,
        "wizard",
        G_OBJECT(wiz_wnd)
    );

    return builder;
}

static GtkWidget* wintc_wizard97_window_create_page(
    WinTCWizard97Window* wiz_wnd,
    const gchar*         resource_path,
    guint                page_num
)
{
    WinTCWizard97WindowClass* wiz_class =
        WINTC_WIZARD97_WINDOW_GET_CLASS(wiz_wnd);

    GtkBuilder* builder = wintc_wizard97_window_create_builder(wiz_wnd);
    GError*     error   = NULL;
    GtkWidget*  ret     = NULL;

    if (
        gtk_builder_add_from_resource(
            builder,
            resource_path,
            &error
        )
    )
    {
        ret = GTK_WIDGET(gtk_builder_get_object(builder, "page"));

        if (!WINTC_IS_WIZARD97_PAGE(ret))
        {
            g_critical(
                "wizard97: the resource at %s doesn't have a 'page'",
                resource_path
            );
        }

        wiz_class->constructing_page(
            wiz_wnd,
            page_num,
            builder
        );

        g_object_ref(ret);
    }
    else
    {
        wintc_log_error_and_clear(&error);
    }

    g_object_unref(builder);

    return ret;
}

static GtkWidget* wintc_wizard97_window_get_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num
)
{
    WinTCWizard97WindowPrivate* priv =
        wintc_wizard97_window_get_instance_private(
            WINTC_WIZARD97_WINDOW(wiz_wnd)
        );

    return GTK_WIDGET(
        g_list_nth_data(
            priv->list_pages,
            page_num
        )
    );
}

static void wintc_wizard97_window_go_to_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num,
    gboolean             push_history
)
{
    WinTCWizard97WindowPrivate* priv =
        wintc_wizard97_window_get_instance_private(
            WINTC_WIZARD97_WINDOW(wiz_wnd)
        );

    // Remove any existing page from view
    //
    GtkWidget* page_current =
        wintc_wizard97_window_get_page(
            wiz_wnd,
            priv->current_page
        );

    if (gtk_widget_get_parent(page_current))
    {
        gtk_container_remove(
            GTK_CONTAINER(gtk_widget_get_parent(page_current)),
            page_current
        );
    }

    wintc_container_clear(
        GTK_CONTAINER(priv->box_page_area),
        FALSE
    );

    // Retrieve the boxes we need
    //
    GtkWidget* box_template;
    GtkWidget* box_template_page_area;
    GtkWidget* page_next;

    page_next =
        wintc_wizard97_window_get_page(
            wiz_wnd,
            page_num
        );

    if (
        wintc_wizard97_page_get_is_exterior_page(
            WINTC_WIZARD97_PAGE(page_next)
        )
    )
    {
        box_template           = priv->box_tmpl_ext_page;
        box_template_page_area = priv->box_tmpl_ext_page_area;
    }
    else
    {
        box_template           = priv->box_tmpl_int_page;
        box_template_page_area = priv->box_tmpl_int_page_area;

        gtk_label_set_text(
            GTK_LABEL(priv->label_int_title),
            wintc_wizard97_page_get_title(
                WINTC_WIZARD97_PAGE(page_next)
            )
        );
        gtk_label_set_text(
            GTK_LABEL(priv->label_int_subtitle),
            wintc_wizard97_page_get_subtitle(
                WINTC_WIZARD97_PAGE(page_next)
            )
        );
    }

    // Pack the page now
    //
    gtk_box_pack_start(
        GTK_BOX(box_template_page_area),
        page_next,
        TRUE,
        TRUE,
        0
    );

    gtk_box_pack_start(
        GTK_BOX(priv->box_page_area),
        box_template,
        TRUE,
        TRUE,
        0
    );

    gtk_widget_show_all(box_template);

    // Update state
    //
    if (push_history)
    {
        priv->history =
            g_slist_prepend(
                priv->history,
                GUINT_TO_POINTER(priv->current_page)
            );
    }

    priv->current_page = page_num;

    // Ping everything
    //
    for (gint i = PROP_CAN_PREV; i < N_PROPERTIES; i++)
    {
        g_object_notify_by_pspec(
            G_OBJECT(wiz_wnd),
            wintc_wizard97_window_properties[i]
        );
    }
}

static gboolean wintc_wizard97_window_is_on_final_page(
    WinTCWizard97Window* wiz_wnd
)
{
    WinTCWizard97WindowPrivate* priv =
        wintc_wizard97_window_get_instance_private(wiz_wnd);

    WinTCWizard97WindowClass* wiz_class =
        WINTC_WIZARD97_WINDOW_GET_CLASS(wiz_wnd);

    if (
        wintc_wizard97_page_get_is_final_page(
            WINTC_WIZARD97_PAGE(
                wintc_wizard97_window_get_page(wiz_wnd, priv->current_page)
            )
        ) ||
        wiz_class->get_next_page(
            wiz_wnd,
            priv->current_page
        ) == WIZARD97_PAGE_NUM_FINAL
    )
    {
        return TRUE;
    }


    return FALSE;
}

//
// CALLBACKS
//
static void action_back(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    WinTCWizard97WindowPrivate* priv =
        wintc_wizard97_window_get_instance_private(
            WINTC_WIZARD97_WINDOW(user_data)
        );

    // Check what page we should go to
    //
    guint prev_page;

    if (!priv->history)
    {
        g_critical("%s", "wizard97: attempted to go back with no history");
        return;
    }

    prev_page = GPOINTER_TO_UINT(priv->history->data);

    priv->history =
        g_slist_delete_link(
            priv->history,
            priv->history
        );

    // Go!
    //
    wintc_wizard97_window_go_to_page(
        WINTC_WIZARD97_WINDOW(user_data),
        prev_page,
        FALSE
    );
}

static void action_cancel(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    WinTCWizard97Window*      wiz_wnd   = WINTC_WIZARD97_WINDOW(user_data);
    WinTCWizard97WindowClass* wiz_class =
        WINTC_WIZARD97_WINDOW_GET_CLASS(wiz_wnd);

    WinTCWizard97WindowPrivate* priv =
        wintc_wizard97_window_get_instance_private(
            wiz_wnd
        );

    if (wiz_class->cancel(wiz_wnd, priv->current_page))
    {
        gtk_window_close(GTK_WINDOW(user_data));
    }
}

static void action_help(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    WinTCWizard97Window*      wiz_wnd   = WINTC_WIZARD97_WINDOW(user_data);
    WinTCWizard97WindowClass* wiz_class =
        WINTC_WIZARD97_WINDOW_GET_CLASS(wiz_wnd);

    WinTCWizard97WindowPrivate* priv =
        wintc_wizard97_window_get_instance_private(
            wiz_wnd
        );

    wiz_class->help(wiz_wnd, priv->current_page);
}

static void action_next(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    WinTCWizard97Window*      wiz_wnd   = WINTC_WIZARD97_WINDOW(user_data);
    WinTCWizard97WindowClass* wiz_class =
        WINTC_WIZARD97_WINDOW_GET_CLASS(wiz_wnd);

    WinTCWizard97WindowPrivate* priv =
        wintc_wizard97_window_get_instance_private(
            wiz_wnd
        );

    if (wintc_wizard97_window_is_on_final_page(wiz_wnd))
    {
        if (!wiz_class->finish(wiz_wnd, priv->current_page))
        {
            return;
        }

        gtk_window_close(GTK_WINDOW(wiz_wnd));
    }
    else
    {
        wintc_wizard97_window_go_to_page(
            WINTC_WIZARD97_WINDOW(user_data),
            wiz_class->get_next_page(wiz_wnd, priv->current_page),
            TRUE
        );
    }
}

static gboolean on_window_map_event(
    GtkWidget* widget,
    WINTC_UNUSED(GdkEvent* event),
    WINTC_UNUSED(gpointer user_data)
)
{
    WinTCWizard97Window* wiz_wnd = WINTC_WIZARD97_WINDOW(widget);

    // Load the first page
    //
    wintc_wizard97_window_go_to_page(
        wiz_wnd,
        0,
        FALSE
    );

    return FALSE;
}
