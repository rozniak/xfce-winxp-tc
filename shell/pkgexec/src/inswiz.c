#include <gio/gdesktopappinfo.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/setupapi.h>
#include <wintc/shlang.h>
#include <wintc/wizard97.h>

#include "inswiz.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_PATH,
    N_PROPERTIES,
};

enum
{
    WIZPAGE_INTRO,
    WIZPAGE_PROGRESS,
    WIZPAGE_FINISH
};

//
// FORWARD DECLARATIONS
//
static void wintc_pkg_exec_install_wizard_constructed(
    GObject* object
);
static void wintc_pkg_exec_install_wizard_dispose(
    GObject* object
);
static void wintc_pkg_exec_install_wizard_finalize(
    GObject* object
);
static void wintc_pkg_exec_install_wizard_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_pkg_exec_install_wizard_constructing_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num,
    GtkBuilder*          builder
);
static void wintc_pkg_exec_install_wizard_presenting_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num
);

//
// STATIC DATA
//
static GParamSpec*
    wintc_pkg_exec_install_wizard_properties[N_PROPERTIES] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCPkgExecInstallWizardClass
{
    WinTCWizard97WindowClass __parent__;
};

struct _WinTCPkgExecInstallWizard
{
    WinTCWizard97Window __parent__;

    // State
    //
    gchar*           package_name;
    gchar*           path;
    WinTCPkgSession* session;

    GtkWidget* label_pkgname;
    GtkWidget* label_pkgname2;
    GtkWidget* progress_pkg;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCPkgExecInstallWizard,
    wintc_pkg_exec_install_wizard,
    WINTC_TYPE_WIZARD97_WINDOW
)

static void wintc_pkg_exec_install_wizard_class_init(
    WinTCPkgExecInstallWizardClass* klass
)
{
    GObjectClass*             object_class = G_OBJECT_CLASS(klass);
    WinTCWizard97WindowClass* wizard_class =
        WINTC_WIZARD97_WINDOW_CLASS(klass);

    object_class->constructed  = wintc_pkg_exec_install_wizard_constructed;
    object_class->dispose      = wintc_pkg_exec_install_wizard_dispose;
    object_class->finalize     = wintc_pkg_exec_install_wizard_finalize;
    object_class->set_property = wintc_pkg_exec_install_wizard_set_property;
    wizard_class->constructing_page =
        wintc_pkg_exec_install_wizard_constructing_page;
    wizard_class->presenting_page   =
        wintc_pkg_exec_install_wizard_presenting_page;


    // Set up properties
    //
    wintc_pkg_exec_install_wizard_properties[PROP_PATH] =
        g_param_spec_string(
            "path",
            "Path",
            "The path to the package to install.",
            NULL,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_pkg_exec_install_wizard_properties
    );

    // Configure wizard
    //
    wintc_wizard97_window_class_setup_from_resources(
        wizard_class,
        WINTC_WIZARD97_STYLE_IE5,
        "/uk/oddmatics/wintc/pkg-exec/watermk.png",
        "/uk/oddmatics/wintc/pkg-exec/header.png",
        "/uk/oddmatics/wintc/pkg-exec/inswizp1.ui",
        "/uk/oddmatics/wintc/pkg-exec/inswizp2.ui",
        "/uk/oddmatics/wintc/pkg-exec/inswizp3.ui",
        NULL
    );
}

static void wintc_pkg_exec_install_wizard_init(
    WinTCPkgExecInstallWizard* self
)
{
    self->package_name = g_strdup("Debian Package");

    wintc_wizard97_window_init_wizard(
        WINTC_WIZARD97_WINDOW(self)
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_pkg_exec_install_wizard_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_pkg_exec_install_wizard_parent_class))
        ->constructed(object);

    //WinTCPkgExecInstallWizard* inswiz =
    //    WINTC_PKG_EXEC_INSTALL_WIZARD(object);
}

static void wintc_pkg_exec_install_wizard_dispose(
    GObject* object
)
{
    WinTCPkgExecInstallWizard* inswiz =
        WINTC_PKG_EXEC_INSTALL_WIZARD(object);

    g_clear_object(&(inswiz->session));

    (G_OBJECT_CLASS(wintc_pkg_exec_install_wizard_parent_class))
        ->dispose(object);
}

static void wintc_pkg_exec_install_wizard_finalize(
    GObject* object
)
{
    WinTCPkgExecInstallWizard* inswiz =
        WINTC_PKG_EXEC_INSTALL_WIZARD(object);

    g_free(g_steal_pointer(&(inswiz->package_name)));

    (G_OBJECT_CLASS(wintc_pkg_exec_install_wizard_parent_class))
        ->finalize(object);
}

static void wintc_pkg_exec_install_wizard_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCPkgExecInstallWizard* inswiz =
        WINTC_PKG_EXEC_INSTALL_WIZARD(object);

    switch (prop_id)
    {
        case PROP_PATH:
            inswiz->path = g_value_dup_string(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_pkg_exec_install_wizard_constructing_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num,
    GtkBuilder*          builder
)
{
    WinTCPkgExecInstallWizard* inswiz =
        WINTC_PKG_EXEC_INSTALL_WIZARD(wiz_wnd);

    switch (page_num)
    {
        case WIZPAGE_INTRO:
            wintc_builder_get_objects(
                builder,
                "label-pkgname", &(inswiz->label_pkgname),
                NULL
            );

            wintc_widget_printf(
                inswiz->label_pkgname,
                inswiz->package_name
            );

            break;

        case WIZPAGE_PROGRESS:
            wintc_builder_get_objects(
                builder,
                "progress-pkg", &(inswiz->progress_pkg),
                NULL
            );

            break;

        case WIZPAGE_FINISH:
            wintc_builder_get_objects(
                builder,
                "label-pkgname2", &(inswiz->label_pkgname2),
                NULL
            );

            wintc_widget_printf(
                inswiz->label_pkgname2,
                inswiz->package_name
            );

            break;

        default: break;
    }
}

static void wintc_pkg_exec_install_wizard_presenting_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num
)
{
    WinTCPkgExecInstallWizard* inswiz =
        WINTC_PKG_EXEC_INSTALL_WIZARD(wiz_wnd);

    if (page_num != WIZPAGE_PROGRESS)
    {
        return;
    }

    gtk_progress_bar_set_fraction(
        GTK_PROGRESS_BAR(inswiz->progress_pkg),
        0.5f
    );
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_pkg_exec_install_wizard_new(
    const gchar* path
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_PKG_EXEC_INSTALL_WIZARD,
            "icon-name", "package-x-generic",
            "path",      path,
            "title",     "Package Installation Wizard",
            NULL
        )
    );
}
