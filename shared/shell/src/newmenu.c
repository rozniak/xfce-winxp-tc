#include <glib.h>
#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "../public/fsop.h"
#include "../public/newmenu.h"

//
// PRIVATE ENUMS
//
enum
{
    WINTC_SH_NEW_OP_NEW_FOLDER = 80,
    WINTC_SH_NEW_OP_NEW_SHORTCUT,
    WINTC_SH_NEW_OP_FIRST_TEMPLATE
};

//
// PRIVATE STRUCTURES
//
typedef struct _WinTCShNewTemplate
{
    gchar* filename;
    gchar* name;
} WinTCShNewTemplate;

//
// FORWARD DECLARATIONS
//
static void wintc_sh_new_menu_update_templates(void);

static void clear_new_template(
    WinTCShNewTemplate* template
);

//
// STATIC DATA
//
static GList* S_LIST_TEMPLATES = NULL;

//
// PUBLIC FUNCTIONS
//
GMenuModel* wintc_sh_new_menu_get_menu(void)
{
    static GMenuModel* s_menu = NULL;

    if (s_menu)
    {
        return s_menu;
    }

    wintc_sh_new_menu_update_templates();

    // Create the menu
    //
    GtkBuilder* builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/shell/menunew.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    s_menu =
        G_MENU_MODEL(
            g_object_ref(
                gtk_builder_get_object(builder, "menu-model")
            )
        );

    // Populate templates
    //
    GMenu* section_new_templates =        
        G_MENU(
            gtk_builder_get_object(
                builder,
                "section-new-templates"
            )
        );

    gint view_op_id = WINTC_SHEXT_OP_NEW + 2;

    for (GList* iter = S_LIST_TEMPLATES; iter; iter = iter->next)
    {
        WinTCShNewTemplate* template =
            (WinTCShNewTemplate*) iter->data;

        // Cap off menu items
        //
        if (!WINTC_SHEXT_OP_IS_NEW_OP(view_op_id))
        {
            break;
        }

        // Create the menu item
        //
        GIcon*     icon      = g_themed_icon_new(template->filename);
        GMenuItem* menu_item = g_menu_item_new(NULL, NULL);

        g_menu_item_set_label(menu_item, template->name);
        g_menu_item_set_icon(menu_item, icon);

        g_menu_item_set_action_and_target(
            menu_item,
            "control.view-op",
            "i",
            view_op_id
        );

        g_menu_append_item(
            section_new_templates,
            menu_item
        );

        view_op_id++;

        g_object_unref(icon);
        g_object_unref(menu_item);
    }

    g_object_unref(builder);

    return s_menu;
}

gboolean wintc_sh_new_menu_create_file(
    const gchar* path,
    gint         op_id,
    guint*       hash_new_file,
    GError**     error
)
{
    //
    // FIXME: Localisation needed in the names this func uses
    //

    // Handle Folder and Shortcut as special cases
    //
    // FIXME: Not doing shortcuts just yet because it requires a wizard
    //
    gboolean is_folder = op_id == WINTC_SH_NEW_OP_NEW_FOLDER;

    GFile*              file;
    guint               hash;
    GError*             local_error = NULL;
    gchar*              name;
    const gchar*        name_type;
    gchar*              new_path;
    gboolean            success;
    WinTCShNewTemplate* template;

    // FIXME: Drop out of shortcut handling for now
    //
    if (op_id == WINTC_SH_NEW_OP_NEW_SHORTCUT)
    {
        g_critical(
            "%s",
            "shell: new - shortcuts not implemented"
        );
        return FALSE;
    }

    if (is_folder)
    {
        name_type = "Folder";
    }
    else
    {
        template =
            (WinTCShNewTemplate*)
            g_list_nth_data(
                S_LIST_TEMPLATES,
                op_id - WINTC_SH_NEW_OP_FIRST_TEMPLATE
            );

        name_type = template->name;
    }

    for (gint attempt = 0; attempt < 100; attempt++)
    {
        if (attempt)
        {
            name =
                g_strdup_printf(
                    "New %s (%d)",
                    name_type,
                    attempt
                );
        }
        else
        {
            name =
                g_strdup_printf(
                    "New %s",
                    name_type
                );
        }

        new_path = g_build_path(G_DIR_SEPARATOR_S, path, name, NULL);
        hash     = g_str_hash(new_path);
        file     = g_file_new_for_path(new_path);

        if (is_folder)
        {
            success =
                g_file_make_directory(
                    file,
                    NULL,
                    &local_error
                );
        }
        else
        {
            gchar* template_path = g_build_path(
                                       G_DIR_SEPARATOR_S,
                                       WINTC_ASSETS_DIR,
                                       "templates",
                                       template->filename,
                                       NULL
                                   );
            GFile* template_file = g_file_new_for_path(template_path);

            success =
                g_file_copy (
                    template_file,
                    file,
                    G_FILE_COPY_NONE,
                    NULL,
                    NULL,
                    NULL,
                    &local_error
                );

            g_free(template_path);
            g_object_unref(template_file);
        }

        g_free(new_path);
        g_free(name);
        g_object_unref(file);

        if (success)
        {
            WINTC_SAFE_REF_SET(hash_new_file, hash);
            return TRUE;
        }
        else
        {
            if (local_error->code == G_IO_ERROR_EXISTS)
            {
                g_clear_error(&local_error);
                continue;
            }
            else
            {
                g_propagate_error(error, local_error);
                return FALSE;
            }
        }
    }

    // FIXME: Set error here
    return FALSE;
}

//
// PRIVATE FUNCTIONS
//
static void clear_new_template(
    WinTCShNewTemplate* template
)
{
    g_free(template->filename);
    g_free(template->name);
    g_free(template);
}

static void wintc_sh_new_menu_update_templates(void)
{
    GList* templates =
        wintc_sh_fs_get_names_as_list(
            WINTC_ASSETS_DIR G_DIR_SEPARATOR_S "templates",
            FALSE,
            G_FILE_TEST_IS_REGULAR,
            FALSE,
            NULL
        );

    g_clear_list(
        &(S_LIST_TEMPLATES),
        (GDestroyNotify) clear_new_template
    );

    for (GList* iter = templates; iter; iter = iter->next)
    {
        const gchar* filename = (gchar*) iter->data;

        // Pull out the MIME type parts from filename
        //
        const gchar* p_mime_base_end = strstr(filename, "-");

        if (!p_mime_base_end)
        {
            g_warning(
                "shell: fs - doesn't look like valid MIME: %s",
                filename
            );

            continue;
        }

        // Build the file path...
        //
        gchar* mime_base   = wintc_substr(filename, p_mime_base_end);
        gchar* mime_spec   = wintc_substr(p_mime_base_end + 1, NULL);
        gchar* mime_specfn = g_strdup_printf("%s.xml", mime_spec);

        gchar* mime_path =
            g_build_path(
                G_DIR_SEPARATOR_S,
                G_DIR_SEPARATOR_S,
                "usr",
#ifdef WINTC_PKGMGR_BSDPKG
                "local",
#endif
                "share",
                "mime",
                mime_base,
                mime_specfn,
                NULL
            );

        // Attempt to parse the MIME XML
        //
        xmlDocPtr xml_mime = xmlParseFile(mime_path);

        g_free(mime_base);
        g_free(mime_spec);
        g_free(mime_specfn);
        g_free(mime_path);

        if (!xml_mime)
        {
            WINTC_LOG_DEBUG(
                "shell: fs - could not get XML for %s",
                filename
            );

            continue;
        }

        // Retrieve the name
        //
        xmlNodePtr xml_root = xmlDocGetRootElement(xml_mime);

        for (xmlNodePtr node = xml_root->children; node; node = node->next)
        {
            // We're looking for <comment>
            //
            xmlChar* node_lang;

            if (node->type != XML_ELEMENT_NODE)
            {
                continue;
            }

            if (g_strcmp0((gchar*) node->name, "comment") != 0)
            {
                continue;
            }

            if ((node_lang = xmlNodeGetLang(node)))
            {
                xmlFree(node_lang);
                continue;
            }

            // Create the new template item
            //
            WinTCShNewTemplate* template =
                g_new(WinTCShNewTemplate, 1);

            xmlChar* mime_name = xmlNodeGetContent(node);

            template->filename = g_strdup(filename);
            template->name     = g_strdup((gchar*) mime_name);

            xmlFree(mime_name);

            S_LIST_TEMPLATES =
                g_list_prepend(
                    S_LIST_TEMPLATES,
                    template
                );

            break;
        }

        xmlFreeDoc(xml_mime);
    }

    if (S_LIST_TEMPLATES)
    {
        S_LIST_TEMPLATES =
            g_list_reverse(S_LIST_TEMPLATES);
    }

    g_list_free_full(
        templates,
        (GDestroyNotify) g_free
    );
}
