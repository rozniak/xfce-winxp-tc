#include <gdk-pixbuf/gdk-pixbuf.h>
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

static gchar*     get_cpu_name(void);
static gdouble    get_cpu_speed(void);
static GdkPixbuf* get_distro_logo(void);
static gdouble    get_total_ram_mb(void);

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

    GtkWidget* label_dist   = NULL;
    GtkWidget* img_distlogo = NULL;

    GtkWidget* label_skuname = NULL;
    GtkWidget* label_skued   = NULL;
    GtkWidget* label_skuver  = NULL;

    GtkWidget* label_reguser = NULL;

    GtkWidget* label_cpuid   = NULL;
    GtkWidget* label_stats   = NULL;

    wintc_lc_builder_preprocess_widget_text(builder);

    wintc_builder_get_objects(
        builder,
        "label-dist",    &label_dist,
        "img-distlogo",  &img_distlogo,
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
    GdkPixbuf*     dist_logo = get_distro_logo();
    struct passwd* user_pwd  = getpwuid(getuid());

    gdouble      cpu_speed     = get_cpu_speed();
    const gchar* cpu_speed_fmt = "%.0f MHz";
    gchar*       cpu_speed_str = NULL;

    gdouble      stat_ram     = get_total_ram_mb();
    const gchar* stat_ram_fmt = "%.0f MB";
    gchar*       stat_ram_str = NULL;

    gchar* stats_str;

    if (cpu_speed > 1000.0f)
    {
        cpu_speed     /= 1000.0f;
        cpu_speed_fmt  = "%.2f GHz";
    }

    if (stat_ram > 1024.0f)
    {
        stat_ram     /= 1024.0f;
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

    // Some systems may not be possible to get a CPU freq. reading, so
    // just report RAM only in that case
    //
    if (cpu_speed < 1.0f)
    {
        stats_str =
            g_strdup_printf(
                "%s of RAM",
                stat_ram_str
            );
    }
    else
    {
        stats_str =
            g_strdup_printf(
                "%s, %s of RAM",
                cpu_speed_str,
                stat_ram_str
            );
    }

    gtk_label_set_text(
        GTK_LABEL(label_skuname),
        wintc_build_query(WINTC_VER_NAME)
    );
    gtk_label_set_text(
        GTK_LABEL(label_skued),
        wintc_build_query(WINTC_VER_SKU)
    );
    gtk_label_set_text(
        GTK_LABEL(label_skuver),
        wintc_build_query(WINTC_VER_SKU_TAGLINE)
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

    if (dist_logo)
    {
        gtk_image_set_from_pixbuf(
            GTK_IMAGE(img_distlogo),
            dist_logo
        );
        g_object_unref(dist_logo);
    }
    else
    {
        gtk_label_set_text(GTK_LABEL(label_dist), "");
    }

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
    static const gchar* k_methods[] = {
        "sh -c \"lscpu | grep 'Model name' | sed -e 's/Model name: //g'\"",
        "sh -c \"sysctl hw.model | cut -d':' -f2\"",
        "sh -c \"cat /proc/cpuinfo | grep 'model name' | head -n 1 | cut -d':' -f2\""
    };

    GError* error  = NULL;
    gchar*  output = NULL;

    for (gulong i = 0; i < G_N_ELEMENTS(k_methods); i++)
    {
        if (
            !wintc_launch_command_sync(
                k_methods[i],
                &output,
                NULL,
                &error
            )
        )
        {
            wintc_log_error_and_clear(&error);
            continue;
        }

        if (output)
        {
            if (strlen(output) == 0)
            {
                g_free(g_steal_pointer(&output));
            }
            else
            {
                output = g_strstrip(output);
                break;
            }
        }
    }

    return output;
}

static gdouble get_cpu_speed(void)
{
    static const gchar* k_methods[] = {
        "sh -c \"lscpu | grep 'CPU max MHz' | sed -e 's/CPU max MHz: //g'\"",
        "sh -c \"lscpu -e=MHZ | grep '[[:digit:]]' | sort -r | head -n 1\"",
        "sh -c \"sysctl dev.cpu.0.freq | cut -d':' -f2\"",
        "sh -c \"cat /proc/cpuinfo | grep 'cpu MHz' | head -n 1 | cut -d':' -f2\""
    };

    GError* error  = NULL;
    gchar*  output = NULL;
    gdouble ret    = 0.0f;

    for (gulong i = 0; i < G_N_ELEMENTS(k_methods); i++)
    {
        if (
            !wintc_launch_command_sync(
                k_methods[i],
                &output,
                NULL,
                &error
            )
        )
        {
            wintc_log_error_and_clear(&error);
            continue;
        }

        ret = g_strtod(output, NULL);
        g_free(output);

        // Was this method successful?
        //
        if (ret > 0.0f)
        {
            break; 
        }
    }

    return ret;
}

static GdkPixbuf* get_distro_logo(void)
{
    gchar* distro = NULL;

    if (
        !wintc_launch_command_sync(
            "sh -c '. /etc/os-release && echo -n $ID'",
            &distro,
            NULL,
            NULL
        )
    )
    {
        return NULL;
    }

    distro = g_strstrip(distro);

    // Attempt to load the logo
    //
    GdkPixbuf* logo;
    gchar*     path = g_strdup_printf(
                          "%s/dist-logo/%s.png",
                          WINTC_ASSETS_DIR,
                          distro
                      );

    logo =
        gdk_pixbuf_new_from_file(
            path,
            NULL
        );

    g_free(distro);
    g_free(path);

    return logo;
}

static gdouble get_total_ram_mb(void)
{
    struct sysinfo stats;

    sysinfo(&stats);

#ifdef WINTC_PKGMGR_BSDPKG
    return (gdouble) (stats.totalram / 1024);
#else
    return (gdouble) ((stats.totalram / 1024) / 1024);
#endif
}
