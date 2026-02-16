#include <glib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wintc/comgtk.h>

#include "xfconf.h"

//
// PRIVATE ENUMS
//
enum
{
    XML_XFCONF_FOUND,
    XML_XFCONF_NOT_FOUND,
    XML_XFCONF_TYPE_MISMATCH
};

//
// PRIVATE STRUCTURES
//
typedef struct _XfconfPropNode
{
    xmlChar* name;
    xmlChar* type;
    xmlChar* value;
} XfconfPropNode;

//
// FORWARD DECLARATIONS
//
static gboolean xml_node_check_attribute(
    xmlNode*       node,
    const xmlChar* attribute_name,
    const xmlChar* expected_value
);
static xmlAttr* xml_node_find_attribute(
    xmlNode*       node,
    const xmlChar* property_name
);
static xmlChar* xml_node_get_attribute_value(
    xmlNode*       node,
    const xmlChar* attribute_name
);
static void xml_node_set_attribute_value(
    xmlNode*       node,
    const xmlChar* attribute_name,
    const xmlChar* new_value
);
static gboolean xml_version_patch(
    gchar*   xml_src,
    gboolean upgrade
);
static xmlNode* xml_xfconf_create_property_node(
    xmlNode*        node_parent,
    XfconfPropNode* descriptor
);
static xmlNode* xml_xfconf_find_property_node(
    xmlNode*        node_parent,
    XfconfPropNode* descriptor,
    gint*           result
);
static void xml_xfconf_mirror_property_node(
    xmlNode* node_dest_group,
    xmlNode* node_src_prop
);

//
// PUBLIC FUNCTIONS
//
void wintc_oobe_xfconf_update_channel(
    const gchar* user_home,
    uid_t        user_id,
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
        WINTC_LOG_DEBUG(
            "oobe: merging config to %s",
            channel_dest_path
        );

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

        //
        // The configs look like so:
        //
        // <channel name="XX">                            - ROOT NODE
        //   <property name="XX" type="empty">            - PROPERTY GROUP
        //     <property name="XX" type="YY" value="ZZ"/> - PROPERTY
        //   </property>
        // </channel>
        //

        // First level - iterate over property groups
        //
        xmlNode* node_dest_root =
            xmlDocGetRootElement(channel_dest_xml);
        xmlNode* node_src_root =
            xmlDocGetRootElement(channel_src_xml);

        for (
            xmlNode* node_src_group = node_src_root->children;
            node_src_group;
            node_src_group = node_src_group->next
        )
        {
            // Check this is a property group node
            //
            if (
                node_src_group->type != XML_ELEMENT_NODE ||
                g_strcmp0((gchar*) node_src_group->name, "property") != 0
            )
            {
                continue;
            }

            // Property group must be type 'empty'
            //
            if (
                !xml_node_check_attribute(
                    node_src_group,
                    (xmlChar*) "type",
                    (xmlChar*) "empty"
                )
            )
            {
                continue;
            }

            // Sync up with destination group node
            //
            XfconfPropNode descriptor;
            xmlChar*       group_name;
            xmlNode*       node_dest_group;
            gint           search_result;

            group_name =
                xml_node_get_attribute_value(
                    node_src_group,
                    (xmlChar*) "name"
                );

            descriptor.name  = group_name;
            descriptor.type  = (xmlChar*) "empty";
            descriptor.value = NULL;

            node_dest_group =
                xml_xfconf_find_property_node(
                    node_dest_root,
                    &descriptor,
                    &search_result
                );

            WINTC_LOG_DEBUG(
                "oobe: updating property group %s on channel %s",
                (gchar*) group_name,
                channel
            );

            if (!node_dest_group)
            {
                if (search_result == XML_XFCONF_TYPE_MISMATCH)
                {
                    g_warning(
                        "oobe: skipping property group '%s', type mismatch",
                        (gchar*) group_name
                    );

                    xmlFree(group_name);

                    continue;
                }

                // Create the new node
                //
                node_dest_group =
                    xml_xfconf_create_property_node(
                        node_dest_root,
                        &descriptor
                    );
            }

            xmlFree(group_name);

            // Second level - iterate over properties themselves
            //
            for (
                xmlNode* node_src_prop = node_src_group->children;
                node_src_prop;
                node_src_prop = node_src_prop->next
            )
            {
                // Check this is a property
                //
                if (
                    node_src_prop->type != XML_ELEMENT_NODE ||
                    g_strcmp0((gchar*) node_src_prop->name, "property") != 0
                )
                {
                    continue;
                }

                // Skip empty props
                //
                if (
                    xml_node_check_attribute(
                        node_src_prop,
                        (xmlChar*) "type",
                        (xmlChar*) "empty"
                    )
                )
                {
                    continue;
                }

                // Arrays unsupported
                //
                if (
                    xml_node_check_attribute(
                        node_src_prop,
                        (xmlChar*) "type",
                        (xmlChar*) "array"
                    )
                )
                {
                    g_critical(
                        "%s",
                        "oobe: array in src properties - this is unsupported"
                    );

                    continue;
                }

                // All good, mirror the property
                //
                xml_xfconf_mirror_property_node(
                    node_dest_group,
                    node_src_prop
                );
            }
        }

        // Write out the modifications
        //
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
        if (
            g_mkdir_with_parents(channel_dest_dir, S_IRWXU) < 0 ||
            chown(channel_dest_dir, user_id, -1) < 0
        )
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

        if (chown(channel_dest_path, user_id, -1) < 0)
        {
            g_critical(
                "oobe: failed to chown %s as %d (%d)",
                channel_dest_path,
                user_id,
                errno
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

//
// PRIVATE FUNCTIONS
//
static gboolean xml_node_check_attribute(
    xmlNode*       node,
    const xmlChar* attribute_name,
    const xmlChar* expected_value
)
{
    xmlChar* attribute_value =
        xml_node_get_attribute_value(
            node,
            attribute_name
        );

    gboolean result =
        g_strcmp0(
            (gchar*) attribute_value,
            (gchar*) expected_value
        ) == 0;

    xmlFree(attribute_value);

    return result;
}

static xmlAttr* xml_node_find_attribute(
    xmlNode*       node,
    const xmlChar* property_name
)
{
    xmlAttr* attrib;

    for (attrib = node->properties; attrib; attrib = attrib->next)
    {
        if (g_strcmp0((gchar*) attrib->name, (gchar*) property_name) != 0)
        {
            continue;
        }

        break;
    }

    return attrib;
}

static xmlChar* xml_node_get_attribute_value(
    xmlNode*       node,
    const xmlChar* attribute_name
)
{
    xmlAttr* attrib =
        xml_node_find_attribute(node, attribute_name);

    if (!attrib)
    {
        return NULL;
    }

    return xmlNodeGetContent(attrib->children);
}

static void xml_node_set_attribute_value(
    xmlNode*       node,
    const xmlChar* attribute_name,
    const xmlChar* new_value
)
{
    xmlAttr* attrib =
        xml_node_find_attribute(node, attribute_name);

    if (attrib)
    {
        xmlNewProp(
            node,
            attribute_name,
            new_value
        );
    }
    else
    {
        xmlNodeSetContent(attrib->children, new_value);
    }
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

static xmlNode* xml_xfconf_create_property_node(
    xmlNode*        node_parent,
    XfconfPropNode* descriptor
)
{
    xmlNode* node_property = xmlNewNode(NULL, (xmlChar*) "property");

    node_property->type = XML_ELEMENT_NODE;

    if (descriptor->name)
    {
        xml_node_set_attribute_value(
            node_property,
            (xmlChar*) "name",
            descriptor->name
        );
    }

    if (descriptor->type)
    {
        xml_node_set_attribute_value(
            node_property,
            (xmlChar*) "type",
            descriptor->type
        );
    }

    if (descriptor->value)
    {
        xml_node_set_attribute_value(
            node_property,
            (xmlChar*) "value",
            descriptor->value
        );
    }

    xmlAddChild(node_parent, node_property);

    return node_property;
}

static xmlNode* xml_xfconf_find_property_node(
    xmlNode*        node_parent,
    XfconfPropNode* descriptor,
    gint*           result
)
{
    xmlNode* iter;

    for (
        iter = node_parent->children;
        iter;
        iter = iter->next
    )
    {
        if (
            iter->type != XML_ELEMENT_NODE ||
            g_strcmp0((gchar*) iter->name, "property") != 0
        )
        {
            continue;
        }

        // Check name and type match
        //
        if (
            descriptor->name &&
            !xml_node_check_attribute(
                iter,
                (xmlChar*) "name",
                descriptor->name
            )
        )
        {
            continue;
        }

        if (
            descriptor->type &&
            !xml_node_check_attribute(
                iter,
                (xmlChar*) "type",
                descriptor->type
            )
        )
        {
            *result = XML_XFCONF_TYPE_MISMATCH;
            return NULL;
        }

        // Found it!
        //
        break;
    }

    *result = iter ? XML_XFCONF_FOUND : XML_XFCONF_NOT_FOUND;
    return iter;
}

static void xml_xfconf_mirror_property_node(
    xmlNode* node_dest_group,
    xmlNode* node_src_prop
)
{
    XfconfPropNode descriptor;
    gint           search_result;

    descriptor.name   = xml_node_get_attribute_value(
                            node_src_prop,
                            (xmlChar*) "name"
                        );
    descriptor.type   = xml_node_get_attribute_value(
                            node_src_prop,
                            (xmlChar*) "type"
                        );
    descriptor.value  = xml_node_get_attribute_value(
                            node_src_prop,
                            (xmlChar*) "value"
                        );

    // Either create or update the node
    //
    xmlNode* node_dest_prop =
        xml_xfconf_find_property_node(
            node_dest_group,
            &descriptor,
            &search_result
        );

    WINTC_LOG_DEBUG(
        "oobe: setting '%s' to '%s'",
        (gchar*) descriptor.name,
        (gchar*) descriptor.value
    );

    if (node_dest_prop)
    {
        xml_node_set_attribute_value(
            node_dest_prop,
            (xmlChar*) "value",
            descriptor.value
        );
    }
    else
    {
        if (search_result == XML_XFCONF_TYPE_MISMATCH)
        {
            g_warning(
                "oobe: unable to update property '%s', expected type '%s'",
                (gchar*) descriptor.name,
                (gchar*) descriptor.type
            );
        }
        else
        {
            xml_xfconf_create_property_node(
                node_dest_group,
                &descriptor
            );
        }
    }

    xmlFree(descriptor.name);
    xmlFree(descriptor.type);
    xmlFree(descriptor.value);
}
