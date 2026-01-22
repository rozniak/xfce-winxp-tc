#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/wizard97.h>

#include "application.h"
#include "window.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_CAN_HELP,
    N_PROPERTIES
};

enum
{
    WIZPAGE_INTRO,
    WIZPAGE_INTRO2,
    WIZPAGE_DECISION,
    WIZPAGE_FINISHED
};

enum
{
    DECISION_APPLES,
    DECISION_ORANGES
};

//
// FORWARD DECLARATIONS
//
static void wintc_wizard_window_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);

static void wintc_wizard_window_constructing_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num,
    GtkBuilder*          builder
);
static guint wintc_wizard_window_get_next_page(
    WinTCWizard97Window* wiz_wnd,
    guint                current_page
);
static void wintc_wizard_window_help(
    WinTCWizard97Window* wiz_wnd,
    guint                current_page
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCWizardWindowClass
{
    WinTCWizard97WindowClass __parent__;
};

struct _WinTCWizardWindow
{
    WinTCWizard97Window __parent__;

    // UI stuff
    //
    GtkWidget* radio_apples;
    GtkWidget* label_done_prefer;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCWizardWindow,
    wintc_wizard_window,
    WINTC_TYPE_WIZARD97_WINDOW
)

static void wintc_wizard_window_class_init(
    WinTCWizardWindowClass* klass
)
{
    GObjectClass*             object_class = G_OBJECT_CLASS(klass);
    WinTCWizard97WindowClass* wizard_class =
        WINTC_WIZARD97_WINDOW_CLASS(klass);

    wizard_class->constructing_page = wintc_wizard_window_constructing_page;
    wizard_class->get_next_page     = wintc_wizard_window_get_next_page;
    wizard_class->help              = wintc_wizard_window_help;
    object_class->get_property      = wintc_wizard_window_get_property;

    g_object_class_override_property(
        object_class,
        PROP_CAN_HELP,
        "can-help"
    );

    wintc_wizard97_window_class_setup_from_resources(
        wizard_class,
        "/uk/oddmatics/wintc/play/wizard/watermk.png",
        "/uk/oddmatics/wintc/play/wizard/header.png",
        "/uk/oddmatics/wintc/play/wizard/wizpg1.ui",
        "/uk/oddmatics/wintc/play/wizard/wizpg2.ui",
        "/uk/oddmatics/wintc/play/wizard/wizpg3.ui",
        "/uk/oddmatics/wintc/play/wizard/wizpg4.ui",
        NULL
    );
}

static void wintc_wizard_window_init(
    WinTCWizardWindow* self
)
{
    wintc_wizard97_window_init_wizard(
        WINTC_WIZARD97_WINDOW(self)
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_wizard_window_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    switch (prop_id)
    {
        case PROP_CAN_HELP:
            g_value_set_boolean(value, TRUE);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_wizard_window_constructing_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num,
    GtkBuilder*          builder
)
{
    WinTCWizardWindow* swiz_wnd = WINTC_WIZARD_WINDOW(wiz_wnd);

    switch (page_num)
    {
        case WIZPAGE_DECISION:
            wintc_builder_get_objects(
                builder,
                "radio-apples", &(swiz_wnd->radio_apples),
                NULL
            );

            break;

        case WIZPAGE_FINISHED:
            wintc_builder_get_objects(
                builder,
                "label-done-prefer", &(swiz_wnd->label_done_prefer),
                NULL
            );

            break;

        default: break;
    }
}

static guint wintc_wizard_window_get_next_page(
    WinTCWizard97Window* wiz_wnd,
    guint                current_page
)
{
    WinTCWizardWindow* swiz_wnd = WINTC_WIZARD_WINDOW(wiz_wnd);

    guint next_page = current_page + 1;

    switch (next_page)
    {
        case WIZPAGE_FINISHED:
        {
            guint        idx_prefer;
            const gchar* text_prefer_item;
            gchar*       text_prefer;

            idx_prefer =
                wintc_radio_group_get_selection(
                    gtk_radio_button_get_group(
                        GTK_RADIO_BUTTON(swiz_wnd->radio_apples)
                    )
                );

            switch (idx_prefer)
            {
                case DECISION_APPLES:  text_prefer_item = "apples";  break;
                case DECISION_ORANGES: text_prefer_item = "oranges"; break;
            }

            text_prefer =
                g_strdup_printf(
                    "Decide that %s are better.",
                    text_prefer_item
                );

            gtk_label_set_text(
                GTK_LABEL(swiz_wnd->label_done_prefer),
                text_prefer
            );

            g_free(text_prefer);

            break;
        }

        default: break;
    }

    return
        (WINTC_WIZARD97_WINDOW_CLASS(wintc_wizard_window_parent_class))
            ->get_next_page(wiz_wnd, current_page);
}

static void wintc_wizard_window_help(
    WinTCWizard97Window* wiz_wnd,
    WINTC_UNUSED(guint current_page)
)
{
    wintc_messagebox_show(
        GTK_WINDOW(wiz_wnd),
        "And this is where I'd display the help dialog... IF I HAD ONE!",
        "Help",
        GTK_BUTTONS_OK,
        GTK_MESSAGE_ERROR
    );
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_wizard_window_new(
    WinTCWizardApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_WIZARD_WINDOW,
            "application", GTK_APPLICATION(app),
            "title",       "My Sample Wizard",
            NULL
        )
    );
}
