#include <glib.h>
#include <gtk/gtk.h>
#include <pwd.h>
#include <sys/sysinfo.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
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

static gchar* get_cpu_name(void);
static gdouble get_cpu_speed(void);
static gdouble get_total_ram(void);

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

    GtkWidget* label_reguser = NULL;

    GtkWidget* label_cpuid   = NULL;
    GtkWidget* label_stats   = NULL;

    wintc_lc_builder_preprocess_widget_text(builder);

    wintc_builder_get_objects(
        builder,
        "label-skuname", &label_skuname,
        "label-skued",   &label_skued,
        "label-skuver",  &label_skuver,
        "label-reguser", &label_reguser,
        "label-cpuid",   &label_cpuid,
        "label-stats",   &label_stats,
        NULL
    );

    // Update data in the view
    //
    gchar*         cpu_id    = get_cpu_name();
    struct passwd* user_pwd  = getpwuid(getuid());

    gdouble      cpu_speed     = get_cpu_speed();
    const gchar* cpu_speed_fmt = "%.0f MHz";
    gchar*       cpu_speed_str = NULL;

    gdouble      stat_ram     = get_total_ram() / 1000000.0f;
    const gchar* stat_ram_fmt = "%.0f MB";
    gchar*       stat_ram_str = NULL;

    gchar* stats_str;

    if (cpu_speed > 1000.0f)
    {
        cpu_speed     /= 1000.0f;
        cpu_speed_fmt  = "%.2f GHz";
    }

    if (stat_ram > 1000.0f)
    {
        stat_ram     /= 1000.0f;
        stat_ram_fmt  = "%.2f GB";
    }

    cpu_speed_str =
        g_strdup_printf(
            cpu_speed_fmt,
            cpu_speed
        );
    stat_ram_str =
        g_strdup_printf(
            stat_ram_fmt,
            stat_ram
        );
    stats_str =
        g_strdup_printf(
            "%s, %s of RAM",
            cpu_speed_str,
            stat_ram_str
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
    gtk_label_set_text(
        GTK_LABEL(label_reguser),
        user_pwd->pw_name
    );
    gtk_label_set_text(
        GTK_LABEL(label_cpuid),
        cpu_id
    );
    gtk_label_set_text(
        GTK_LABEL(label_stats),
        stats_str
    );

    g_free(cpu_id);
    g_free(cpu_speed_str);
    g_free(stat_ram_str);
    g_free(stats_str);

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

//
// PRIVATE FUNCTIONS
//
static gchar* get_cpu_name(void)
{
    // Will not work on FreeBSD
    //
    GError* error  = NULL;
    gchar*  output = NULL;

    if (
        !wintc_launch_command_sync(
            "sh -c \"lscpu | grep 'Model name' | sed -e 's/Model name: //g'\"",
            &output,
            NULL,
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
    }

    if (output)
    {
        output = g_strstrip(output);
    }

    return output;
}

static gdouble get_cpu_speed(void)
{
    // Will not work on FreeBSD
    //
    GError* error  = NULL;
    gchar*  output = NULL;
    gdouble ret    = 0.0f;

    if (
        !wintc_launch_command_sync(
            "sh -c \"lscpu | grep 'CPU max MHz' | sed -e 's/CPU max MHz: //g'\"",
            &output,
            NULL,
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
    }

    if (output)
    {
        ret = g_strtod(output, NULL);
        g_free(output);
    }

    return ret;
}

static gdouble get_total_ram(void)
{
    struct sysinfo stats;

    sysinfo(&stats);

    return (gdouble) stats.totalram;
}
