#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdarg.h>
#include <wintc/comgtk.h>

#include "../public/wizwnd.h"

//
// FORWARD DECLARATIONS
//
static void wintc_wizard97_window_dispose(
    GObject* object
);

static GtkBuilder* wintc_wizard97_window_create_builder(
    WinTCWizard97Window* wiz_wnd
);
static GtkWidget* wintc_wizard97_window_create_page(
    WinTCWizard97Window* wiz_wnd,
    const gchar*         resource_path
);
static void wintc_wizard97_window_go_to_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct __WinTCWizard97WindowPrivate
{
    // State
    //
    guint current_page;

    // UI stuff
    //
    GtkWidget* box_page_area;

    // Stuff we maintain an additional ref to
    //
    GdkPixbuf* pixbuf_watermark;
    GdkPixbuf* pixbuf_header;
    GtkWidget* box_ext_page;
    GList*     list_int_pages;
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

    object_class->dispose = wintc_wizard97_window_dispose;
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
    g_clear_object(&(priv->box_ext_page));
    g_clear_list(
        &(priv->list_int_pages),
        (GDestroyNotify) g_object_unref
    );

    (G_OBJECT_CLASS(wintc_wizard97_window_parent_class))
        ->dispose(object);
}

//
// PUBLIC FUNCTIONS
//
void wintc_wizard97_window_class_setup_from_resources(
    WinTCWizard97WindowClass* wizard_class,
    const gchar*              resource_watermark,
    const gchar*              resource_header,
    const gchar*              resource_ext_page,
    ...
)
{
    va_list ap;
    gchar*  next_res;

    wizard_class->resource_watermark = g_strdup(resource_watermark);
    wizard_class->resource_header    = g_strdup(resource_header);
    wizard_class->resource_ext_page  = g_strdup(resource_ext_page);

    va_start(ap, resource_ext_page);

    next_res = va_arg(ap, gchar*);

    while (next_res)
    {
        wizard_class->list_resources_int_pages =
            g_list_append(
                wizard_class->list_resources_int_pages,
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

    GError* error = NULL;

    // General GtkWindow setup
    //
    gtk_window_set_resizable(GTK_WINDOW(wiz_wnd), FALSE);

    // External page, if we have one!
    //
    if (wizard_class->resource_ext_page)
    {
        priv->box_ext_page =
            wintc_wizard97_window_create_page(
                wiz_wnd,
                wizard_class->resource_ext_page
            );
    }

    // Iterate over internal pages
    //
    GList* iter = wizard_class->list_resources_int_pages;

    for (; iter; iter = iter->next)
    {
        GtkWidget* box =
            wintc_wizard97_window_create_page(
                wiz_wnd,
                (gchar*) iter->data
            );

        if (box)
        {
            priv->list_int_pages =
                g_list_append(
                    priv->list_int_pages,
                    box
                );
        }
    }

    // Pixbufs
    //
    if (wizard_class->resource_watermark)
    {
        priv->pixbuf_watermark =
            gdk_pixbuf_new_from_resource(
                wizard_class->resource_watermark,
                &error
            );

        if (!(priv->pixbuf_watermark))
        {
            wintc_log_error_and_clear(&error);
        }
    }

    if (wizard_class->resource_header)
    {
        priv->pixbuf_header =
            gdk_pixbuf_new_from_resource(
                wizard_class->resource_header,
                &error
            );

        if (!(priv->pixbuf_header))
        {
            wintc_log_error_and_clear(&error);
        }
    }

    // Set up the window
    //
    GtkBuilder* builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/wizard97/wizwnd.ui"
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

    g_object_unref(builder);

    // Load the first page
    //
    wintc_wizard97_window_go_to_page(
        wiz_wnd,
        priv->box_ext_page ? 0 : 1
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
    const gchar*         resource_path
)
{
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
        GSList* objects = gtk_builder_get_objects(builder);

        if (!GTK_IS_BOX(objects->data))
        {
            g_critical(
                "%s",
                "wizard97: root ext page widget is not a GtkBox"
            );
        }

        ret = GTK_WIDGET(objects->data);
        g_object_ref(ret);
    }
    else
    {
        wintc_log_error_and_clear(&error);
    }

    g_object_unref(builder);

    return ret;
}

static void wintc_wizard97_window_go_to_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page
)
{
    WinTCWizard97WindowPrivate* priv =
        wintc_wizard97_window_get_instance_private(
            WINTC_WIZARD97_WINDOW(wiz_wnd)
        );

    GtkWidget*   box_target = NULL;
    const gchar* wrapper_resource;

    if (page == 0)
    {
        if (priv->box_ext_page)
        {
            box_target       = priv->box_ext_page;
            wrapper_resource = "/uk/oddmatics/wintc/wizard97/wizext.ui";
        }
    }
    else
    {
        box_target =
            GTK_WIDGET(
                g_list_nth_data(
                    priv->list_int_pages,
                    page - 1
                )
            );

        wrapper_resource = "/uk/oddmatics/wintc/wizard97/wizint.ui";
    }

    if (!box_target)
    {
        g_critical("wizard97: invalid page selected: %u", page);
        return;
    }

    // Remove any existing page from view
    //
    GList* children =
        gtk_container_get_children(
            GTK_CONTAINER(priv->box_page_area)
        );

    if (g_list_length(children))
    {
        gtk_container_remove(
            GTK_CONTAINER(box_target),
            GTK_WIDGET(children->data)
        );
    }

    g_list_free(children);

    // Construct the page within the appropriate page wrapper
    //
    GtkWidget*  box_main;
    GtkWidget*  box_wrapper;
    GtkBuilder* builder = gtk_builder_new_from_resource(wrapper_resource);

    wintc_builder_get_objects(
        builder,
        "main-box", &box_main,
        "box-page", &box_wrapper,
        NULL
    );

    if (page == 0) // Exterior page
    {
        GtkImage* img_watermark =
            GTK_IMAGE(gtk_builder_get_object(builder, "img-watermark"));

        gtk_image_set_from_pixbuf(
            img_watermark,
            priv->pixbuf_watermark
        );

        gtk_widget_set_size_request(
            box_target,
            317,
            193
        );
    }
    else // Interior page
    {
        GtkImage* img_header =
            GTK_IMAGE(gtk_builder_get_object(builder, "img-header"));

        gtk_image_set_from_pixbuf(
            img_header,
            priv->pixbuf_header
        );

        gtk_widget_set_size_request(
            box_target,
            317,
            143
        );
    }

    gtk_box_pack_start(
        GTK_BOX(box_wrapper),
        box_target,
        TRUE,
        TRUE,
        0
    );

    // Pack the page now
    //
    gtk_box_pack_start(
        GTK_BOX(priv->box_page_area),
        box_main,
        TRUE,
        TRUE,
        0
    );

    gtk_widget_show_all(box_target);

    g_object_unref(builder);
}
