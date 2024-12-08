#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "intapi.h"
#include "pagegen.h"

//
// FORWARD DECLARATIONS
//
static void wintc_cpl_sysdm_page_general_constructed(
    GObject* object
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCCplSysdmPageGeneral
{
    WinTCShextUIController __parent__;
} WinTCCplSysdmPageGeneral;

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCCplSysdmPageGeneral,
    wintc_cpl_sysdm_page_general,
    WINTC_TYPE_SHEXT_UI_CONTROLLER
)

static void wintc_cpl_sysdm_page_general_class_init(
    WinTCCplSysdmPageGeneralClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_cpl_sysdm_page_general_constructed;
}

static void wintc_cpl_sysdm_page_general_init(
    WINTC_UNUSED(WinTCCplSysdmPageGeneral* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_cpl_sysdm_page_general_constructed(
    GObject* object
)
{
    GtkBuilder* builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/cpl-sysdm/page-gen.ui"
        );

    GtkWidget* label_skuname = NULL;
    GtkWidget* label_skued   = NULL;
    GtkWidget* label_skuver  = NULL;

    wintc_lc_builder_preprocess_widget_text(builder);

    wintc_builder_get_objects(
        builder,
        "label-skuname", &label_skuname,
        "label-skued",   &label_skued,
        "label-skuver",  &label_skuver,
        NULL
    );

    gtk_label_set_text(
        GTK_LABEL(label_skuname),
        wintc_build_get_sku_name()
    );
    gtk_label_set_text(
        GTK_LABEL(label_skued),
        wintc_build_get_sku_edition()
    );
    gtk_label_set_text(
        GTK_LABEL(label_skuver),
        wintc_build_get_tagline()
    );

    // Insert page into host
    //
    WinTCIShextUIHost* ui_host =
        wintc_shext_ui_controller_get_ui_host(
            WINTC_SHEXT_UI_CONTROLLER(object)
        );

    wintc_ishext_ui_host_get_ext_widget(
        ui_host,
        WINTC_CPL_SYSDM_HOSTEXT_PAGE,
        GTK_TYPE_BOX,
        builder
    );

    g_object_unref(builder);

    // Chain up
    //
    (G_OBJECT_CLASS(wintc_cpl_sysdm_page_general_parent_class))
        ->constructed(object);
}
