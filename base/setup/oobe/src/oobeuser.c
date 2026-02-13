#include <glib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <wintc/comgtk.h>

#include "oobeuser.h"

//
// FORWARD DECLARATIONS
//
static gboolean wintc_oobe_user_is_eligible_user(
    struct passwd* pwent
);

static void wintc_oobe_user_merge_xfconf_xml(
    const gchar* user_home,
    const gchar* channel
);

static xmlNodePtr xml_node_find_in_children(
    xmlNodePtr   xml_node,
    const gchar* tag,
    const gchar* name
);
static xmlAttr* xml_node_find_property(
    xmlNodePtr   xml_node,
    const gchar* property_name
);
static xmlChar* xml_node_get_property_value(
    xmlNodePtr   xml_node,
    const gchar* property_name
);
static gboolean xml_version_patch(
    gchar*   xml_src,
    gboolean upgrade
);
static xmlNodePtr xml_xfconf_traverse_to_property_group(
    xmlNodePtr   xml_root,
    const gchar* group_name
);
static void xml_xfconf_set_child_property_value(
    xmlNodePtr xml_node_group,
    const gchar* property_name,
    const gchar* new_value
);

//
// PUBLIC FUNCTIONS
//
gboolean wintc_oobe_user_apply_all(
    WINTC_UNUSED(GError** error)
)
{
    struct passwd* pwent = NULL;

    while ((pwent = getpwent()))
    {
        if (!wintc_oobe_user_is_eligible_user(pwent))
        {
            WINTC_LOG_DEBUG(
                "oobe: skipping config on %s (%u)",
                pwent->pw_name,
                pwent->pw_uid
            );

            continue;
        }

        WINTC_LOG_DEBUG(
            "oobe: applying config for %s (%u)",
            pwent->pw_name,
            pwent->pw_uid
        );

        wintc_oobe_user_merge_xfconf_xml(
            pwent->pw_dir,
            "xfwm4"
        );
    }

    endpwent();

    return TRUE;
}

//
// PRIVATE FUNCTIONS
//
static gboolean wintc_oobe_user_is_eligible_user(
    struct passwd* pwent
)
{
    static const gchar* k_hidden_users[] = {
        "nobody", "nobody4", "noaccess"
    };
    static const gchar* k_hidden_shells[] = {
        "/false", "/nologin"
    };

    //
    // These checks are the same as would normally be found in
    // /etc/lightdm/users.conf
    //
    gsize i;

    if (pwent->pw_uid < 500)
    {
        return FALSE;
    }

    for (i = 0; i < G_N_ELEMENTS(k_hidden_users); i++)
    {
        if (g_strcmp0(pwent->pw_name, k_hidden_users[i]) == 0)
        {
            return FALSE;
        }
    }

    for (i = 0; i < G_N_ELEMENTS(k_hidden_shells); i++)
    {
        if (g_str_has_suffix(pwent->pw_shell, k_hidden_shells[i]))
        {
            return FALSE;
        }
    }

    return TRUE;
}

static void wintc_oobe_user_merge_xfconf_xml(
    const gchar* user_home,
    const gchar* channel
)
{
    GError*  error = NULL;

    GBytes* channel_src_bytes = NULL;
    gchar*  channel_src_path  = NULL;
    gchar*  channel_dest_data = NULL;
    gchar*  channel_dest_dir  = NULL;
    gchar*  channel_dest_path = NULL;

    // Retrieve the local resource for this data
    //
    channel_src_path =
        g_strdup_printf(
            "/uk/oddmatics/wintc/oobe/xfconf-%s.xml",
            channel
        );

    channel_src_bytes =
        g_resources_lookup_data(
            channel_src_path,
            G_RESOURCE_LOOKUP_FLAGS_NONE,
            &error
        );

    if (!channel_src_bytes)
    {
        wintc_log_error_and_clear(&error);

        g_critical(
            "oobe: requested deployment of channel %s, except we have no data",
            channel
        );

        goto cleanup;
    }

    // Depending on if the user already has channel conf, do the deployment
    //
    channel_dest_dir =
        g_strdup_printf(
            "%s/.config/xfce4/xfconf/xfce-perchannel-xml",
            user_home
        );
    channel_dest_path =
        g_strdup_printf(
            "%s/%s.xml",
            channel_dest_dir,
            channel
        );

    if (
        g_file_get_contents(
            channel_dest_path,
            &channel_dest_data,
            NULL,
            &error
        )
    )
    {
        if (!xml_version_patch(channel_dest_data, FALSE))
        {
            g_critical(
                "oobe: failed to downgrade XML versions for channel '%s'",
                channel
            );

            goto cleanup;
        }

        xmlDocPtr channel_dest_xml =
            xmlParseDoc((xmlChar*) channel_dest_data);
        xmlDocPtr channel_src_xml  =
            xmlParseDoc((xmlChar*) g_bytes_get_data(channel_src_bytes, NULL));

        xmlNodePtr node_dest_group =
            xml_xfconf_traverse_to_property_group(
                xmlDocGetRootElement(channel_dest_xml),
                "general"
            );

        xml_xfconf_set_child_property_value(
            node_dest_group,
            "theme",
            "Windows XP style (Silver)"
        );

        xmlChar* new_xml = NULL;

        xmlDocDumpMemory(channel_dest_xml, &new_xml, NULL);

        xml_version_patch((gchar*) new_xml, TRUE);

        if (
            !g_file_set_contents(
                channel_dest_path,
                (gchar*) new_xml,
                -1,
                &error
            )
        )
        {
            wintc_log_error_and_clear(&error);
        }

        xmlFree(new_xml);

        xmlFreeDoc(channel_dest_xml);
        xmlFreeDoc(channel_src_xml);
    }
    else
    {
        // File not found is fine, anything else is a real problem
        //
        if (error->code != G_FILE_ERROR_NOENT)
        {
            wintc_log_error_and_clear(&error);

            g_critical(
                "oobe: failed to read existing channel data from %s",
                channel_dest_path
            );

            goto cleanup;
        }

        g_clear_error(&error);

        // Go ahead and just create the file based on our resource
        //
        if (g_mkdir_with_parents(channel_dest_dir, S_IRWXU) < 0)
        {
            g_critical(
                "oobe: failed to create %s (%d)",
                channel_dest_dir,
                errno
            );

            goto cleanup;
        }

        if (
            !g_file_set_contents(
                channel_dest_path,
                g_bytes_get_data(channel_src_bytes, NULL),
                -1,
                &error
            )
        )
        {
            wintc_log_error_and_clear(&error);

            g_critical(
                "oobe: failed to write xfconf xml to %s",
                channel_dest_path
            );

            goto cleanup;
        }

        WINTC_LOG_DEBUG(
            "oobe: wrote new config for channel %s to %s",
            channel,
            channel_dest_path
        );
    }

cleanup:
    g_bytes_unref(channel_src_bytes);
    g_free(channel_src_path);
    g_free(channel_dest_data);
    g_free(channel_dest_dir);
    g_free(channel_dest_path);
}

static xmlNodePtr xml_node_find_in_children(
    xmlNodePtr   xml_node,
    const gchar* tag,
    const gchar* name
)
{
    xmlNodePtr iter;

    for (iter = xml_node->children; iter; iter = iter->next)
    {
        if (g_strcmp0((gchar*) iter->name, tag) != 0)
        {
            continue;
        }

        xmlChar* iter_name = xml_node_get_property_value(iter, "name");
        gboolean found     = g_strcmp0((gchar*) iter_name, name) == 0;

        xmlFree(iter_name);

        if (!found)
        {
            continue;
        }

        break;
    }

    return iter;
}

static xmlAttr* xml_node_find_property(
    xmlNodePtr   xml_node,
    const gchar* property_name
)
{
    xmlAttr* attrib;

    for (attrib = xml_node->properties; attrib; attrib = attrib->next)
    {
        if (g_strcmp0((gchar*) attrib->name, property_name) != 0)
        {
            continue;
        }

        break;
    }

    return attrib;
}

static xmlChar* xml_node_get_property_value(
    xmlNodePtr   xml_node,
    const gchar* property_name
)
{
    xmlAttr* attrib =
        xml_node_find_property(xml_node, property_name);

    if (!attrib)
    {
        return NULL;
    }

    return xmlNodeGetContent(attrib->children);
}

static gboolean xml_version_patch(
    gchar*   xml_src,
    gboolean upgrade
)
{
    gchar* p_version = strstr(xml_src, "<?xml version=\"1.");

    if (!p_version)
    {
        g_critical("%s", "oobe: cannot find XML version");
        return FALSE;
    }

    p_version += strlen("<?xml version=\"1.");

    if (upgrade)
    {
        *p_version = '1';
    }
    else
    {
        *p_version = '0';
    }

    return TRUE;
}

static xmlNodePtr xml_xfconf_traverse_to_property_group(
    xmlNodePtr   xml_root,
    const gchar* group_name
)
{
    // Look for the property group now
    //
    if (!group_name)
    {
        return xml_root;
    }

    return xml_node_find_in_children(
        xml_root,
        "property",
        group_name
    );
}

static void xml_xfconf_set_child_property_value(
    xmlNodePtr   xml_node_group,
    const gchar* property_name,
    const gchar* new_value
)
{
    xmlNodePtr node_prop =
        xml_node_find_in_children(
            xml_node_group,
            "property",
            property_name
        );

    if (!node_prop)
    {
        g_warning("oobe: couldn't find property '%s'", property_name);
        return;
    }

    // Safe to set property
    //
    xmlAttr* attrib =
        xml_node_find_property(node_prop, "value");

    if (!attrib)
    {
        g_warning(
            "oobe: couldn't find value attribute on property '%s'",
            property_name
        );
        return;
    }

    xmlNodeSetContent(attrib->children, (xmlChar*)  new_value);
}
