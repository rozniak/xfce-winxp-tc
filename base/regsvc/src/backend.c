#include <glib.h>
#include <sqlite3.h>
#include <wintc/comgtk.h>

#include "backend.h"

#define WINTC_COMPONENT_REGISTRY "registry"

#define BAIL_ON_FAIL(query)                        \
    rc = (query);                                  \
    if (rc == SQLITE_ERROR || rc == SQLITE_MISUSE) \
    {                                              \
        handle_sqlite_problem(rc);                 \
        sqlite3_finalize(stmt);                    \
        return FALSE;                              \
    }

//
// PRIVATE CONSTANTS
//
static const gchar* S_SQL_INIT =
"CREATE TABLE IF NOT EXISTS RegKey ("
"    Id       INTEGER PRIMARY KEY,"
"    ParentId INTEGER,"
"    Name     VARCHAR(255) NOT NULL,"
"    FOREIGN KEY(ParentId) REFERENCES RegKey(Id),"
"    UNIQUE(ParentId, Name)"
");"
"CREATE TABLE IF NOT EXISTS RegKeyValue ("
"    Id        INTEGER PRIMARY KEY,"
"    OwnerKey  INTEGER,"
"    Name      VARCHAR(255),"
"    ItemType  INTEGER,"
"    ItemValue BLOB,"
"    FOREIGN KEY(OwnerKey) REFERENCES RegKey(Id),"
"    UNIQUE(OwnerKey, Name)"
");"
"INSERT OR REPLACE INTO RegKey VALUES (1, NULL, 'HKEY_CURRENT_USER');";

//
// STATIC DATA
//
static sqlite3* s_db = NULL;

//
// FORWARD DECLARATIONS
//
static gboolean db_create_regkey(
    const gchar* key_name,
    gint         parent_id,
    gint*        out_id
);
static gboolean db_get_regkey_id(
    const gchar* key_name,
    gint         parent_id,
    gint*        out_id
);
static gboolean db_traverse_to_key(
    const gchar* key_path,
    gint*        out_id
);

static void handle_sqlite_problem(
    gint rc
);

static gboolean is_valid_key_path(
    GSList* components
);

static gboolean is_valid_value_name(
    const gchar* value_name
);

static GSList* path_to_components(
    const gchar* key_path
);

//
// INTERNAL FUNCTIONS
//
void backend_close(void)
{
    if (!s_db)
    {
        return;
    }

    WINTC_LOG_DEBUG("%s", "regsvc: sqlite closing profile");

    sqlite3_close(s_db);
    s_db = NULL;
}

gboolean backend_init(void)
{
    GError* error = NULL;
    gchar*  path;
    gint    rc;

    if (s_db)
    {
        return TRUE;
    }

    // Open DB handle
    //
    if (!wintc_profile_ensure_exists(WINTC_COMPONENT_REGISTRY, &error))
    {
        wintc_log_error_and_clear(&error);
        return FALSE;
    }

    path = wintc_profile_get_path(WINTC_COMPONENT_REGISTRY, "ntuser.db");
    rc   = sqlite3_open(path, &s_db);

    WINTC_LOG_DEBUG("regsvc: sqlite opening %s", path);

    g_free(path);

    if (rc)
    {
        g_critical(
            "regsvc: failed to open profile sqlite %s",
            sqlite3_errmsg(s_db)
        );
        sqlite3_close(s_db);
        return FALSE;
    }

    // Ensure tables initialised
    //
    char* error_sql = NULL;

    rc =
        sqlite3_exec(
            s_db,
            S_SQL_INIT,
            NULL,
            NULL,
            &error_sql
        );

    if (rc != SQLITE_OK)
    {
        g_critical(
            "regsvc: failed to init tables %s",
            error_sql
        );

        sqlite3_free(error_sql);

        return FALSE;
    }

    return TRUE;
}

gboolean backend_create_key(
    const gchar* key_path
)
{
    GSList* components = path_to_components(key_path);

    WINTC_LOG_DEBUG("regsvc: backend create key %s", key_path);

    if (!is_valid_key_path(components))
    {
        WINTC_LOG_DEBUG(
            "regsvc: backend create key: key/value not valid '%s'",
            key_path
        );

        g_slist_free_full(components, g_free);

        return FALSE;
    }

    // Iterate over to create the key
    //
    gint     last_id = 0;
    gboolean success = TRUE;

    for (GSList* iter = components; iter; iter = iter->next)
    {
        const gchar* key_name = (gchar*) iter->data;
        gint         this_id  = 0;

        WINTC_LOG_DEBUG("regsvc: create key checking for part '%s'", key_name);

        // Check if key exists first
        //
        if (!db_get_regkey_id(key_name, last_id, &this_id))
        {
            success = FALSE;
            break;
        }

        // If needed, create the key
        //
        if (!this_id)
        {
            WINTC_LOG_DEBUG("%s", "regsvc: need to create that key");

            if (!db_create_regkey(key_name, last_id, &this_id))
            {
                success = FALSE;
                break;
            }
        }

        last_id = this_id;
    }

    g_slist_free_full(components, g_free);

    return success;
}

gboolean backend_get_key_value(
    const gchar*           key_path,
    const gchar*           value_name,
    WinTCRegistryValueType value_type,
    void*                  value_data
)
{
    gint key_id = 0;
    gint rc;

    if (value_data == NULL)
    {
        g_critical("%s", "regsvc: backend nowhere to store kv");
        return FALSE;
    }

    if (
        !is_valid_value_name(value_name) ||
        !db_traverse_to_key(key_path, &key_id)
    )
    {
        WINTC_LOG_DEBUG(
            "regsvc: backend get kv: key/value not valid '%s->%s'",
            key_path,
            value_name
        );
        return FALSE;
    }

    // Retrieve key stuff
    //
    sqlite3_stmt* stmt = NULL;

    BAIL_ON_FAIL(
        sqlite3_prepare(
            s_db,
            "SELECT ItemType, ItemValue FROM RegKeyValue WHERE OwnerKey = ? AND Name = ?;",
            -1,
            &stmt,
            NULL
        )
    );

    BAIL_ON_FAIL(sqlite3_bind_int(stmt, 1, key_id));
    BAIL_ON_FAIL(sqlite3_bind_text(stmt, 2, value_name, -1, SQLITE_STATIC));

    BAIL_ON_FAIL(sqlite3_step(stmt));

    // Analyze result
    //
    WinTCRegistryValueType found_type = WINTC_REG_INVALID;
    gboolean               success    = FALSE;

    if (rc == SQLITE_ROW)
    {
        found_type = (WinTCRegistryValueType) sqlite3_column_int(stmt, 0);

        if (found_type == value_type)
        {
            switch (value_type)
            {
                case WINTC_REG_DWORD:
                    *((gint*) value_data) =
                        sqlite3_column_int(stmt, 1);
                    break;

                case WINTC_REG_QWORD:
                    *((gint64*) value_data) =
                        sqlite3_column_int64(stmt, 1);
                    break;

                case WINTC_REG_SZ:
                    *((gchar**) value_data) =
                        g_strdup((const gchar*) sqlite3_column_text(stmt, 1));
                    break;

                default:
                    g_critical(
                        "regsvc: backend unknown key type %d",
                        value_type
                    );
                    break;
            }

            success = TRUE;
        }
    }

    sqlite3_finalize(stmt);

    return success;
}

gboolean backend_set_key_value(
    const gchar*           key_path,
    const gchar*           value_name,
    WinTCRegistryValueType value_type,
    void*                  value_data
)
{
    gint key_id = 0;
    gint rc;

    if (value_data == NULL)
    {
        g_critical("%s", "regsvc: backend no data provided to set kv");
        return FALSE;
    }

    if (
        !is_valid_value_name(value_name) ||
        !db_traverse_to_key(key_path, &key_id)
    )
    {
        WINTC_LOG_DEBUG(
            "regsvc: backend set kv: key/value not valid '%s->%s'",
            key_path,
            value_name
        );
        return FALSE;
    }

    WINTC_LOG_DEBUG("regsvc: traversed to key id %d", key_id);

    // Check if we already have a key value
    //
    gint          value_id = 0;
    sqlite3_stmt* stmt     = NULL;

    BAIL_ON_FAIL(
        sqlite3_prepare(
            s_db,
            "SELECT Id FROM RegKeyValue WHERE OwnerKey = ? AND Name = ?;",
            -1,
            &stmt,
            NULL
        )
    );

    BAIL_ON_FAIL(sqlite3_bind_int(stmt, 1, key_id));
    BAIL_ON_FAIL(sqlite3_bind_text(stmt, 2, value_name, -1, SQLITE_STATIC));

    BAIL_ON_FAIL(sqlite3_step(stmt));

    if (rc == SQLITE_ROW)
    {
        value_id = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    // Set new value
    //
    gint blob_bind_pos;

    if (value_id)
    {
        BAIL_ON_FAIL(
            sqlite3_prepare(
                s_db,
                "UPDATE RegKeyValue SET ItemType = ?, ItemValue = ? WHERE Id = ?;",
                -1,
                &stmt,
                NULL
            )
        );

        BAIL_ON_FAIL(sqlite3_bind_int(stmt, 1, value_type));
        BAIL_ON_FAIL(sqlite3_bind_int(stmt, 3, value_id));

        blob_bind_pos = 2;
    }
    else
    {
        BAIL_ON_FAIL(
            sqlite3_prepare(
                s_db,
                "INSERT INTO RegKeyValue VALUES (NULL, ?, ?, ?, ?);",
                -1,
                &stmt,
                NULL
            )
        );

        BAIL_ON_FAIL(sqlite3_bind_int(stmt, 1, key_id));
        BAIL_ON_FAIL(
            sqlite3_bind_text(
                stmt,
                2,
                value_name,
                -1,
                SQLITE_STATIC
            )
        );
        BAIL_ON_FAIL(sqlite3_bind_int(stmt, 3, value_type));

        blob_bind_pos = 4;
    }

    switch (value_type)
    {
        case WINTC_REG_DWORD:
            BAIL_ON_FAIL(
                sqlite3_bind_int(
                    stmt,
                    blob_bind_pos,
                    *((gint*) value_data)
                )
            );
            break;

        case WINTC_REG_QWORD:
            BAIL_ON_FAIL(
                sqlite3_bind_int64(
                    stmt,
                    blob_bind_pos,
                    *((gint64*) value_data)
                )
            );
            break;

        case WINTC_REG_SZ:
            BAIL_ON_FAIL(
                sqlite3_bind_text(
                    stmt,
                    blob_bind_pos,
                    *((gchar**) value_data),
                    -1,
                    SQLITE_STATIC
                )
            );
            break;

        default:
            g_critical("regsvc: sqlite no binding for %d", value_type);
            break;
    }

    BAIL_ON_FAIL(sqlite3_step(stmt));

    sqlite3_finalize(stmt);

    return TRUE;
}

//
// PRIVATE FUNCTIONS
//
static gboolean db_create_regkey(
    const gchar* key_name,
    gint         parent_id,
    gint*        out_id
)
{
    gint          rc;
    sqlite3_stmt* stmt = NULL;

    WINTC_SAFE_REF_SET(out_id, 0);

    BAIL_ON_FAIL(
        sqlite3_prepare(
            s_db,
            "INSERT INTO RegKey VALUES (NULL, ?, ?);",
            -1,
            &stmt,
            NULL
        )
    );

    if (!parent_id)
    {
        BAIL_ON_FAIL(sqlite3_bind_null(stmt, 1));
    }
    else
    {
        BAIL_ON_FAIL(sqlite3_bind_int(stmt, 1, parent_id));
    }

    BAIL_ON_FAIL(
        sqlite3_bind_text(
            stmt,
            2,
            key_name,
            -1,
            SQLITE_STATIC
        )
    );

    BAIL_ON_FAIL(sqlite3_step(stmt));

    return db_get_regkey_id(
        key_name,
        parent_id,
        out_id
    );
}

static gboolean db_get_regkey_id(
    const gchar* key_name,
    gint         parent_id,
    gint*        out_id
)
{
    gint          rc;
    sqlite3_stmt* stmt = NULL;

    WINTC_SAFE_REF_SET(out_id, 0);

    BAIL_ON_FAIL(
        sqlite3_prepare(
            s_db,
            "SELECT Id FROM RegKey WHERE ParentId IS ? AND Name = ?;",
            -1,
            &stmt,
            NULL
        )
    );

    if (!parent_id)
    {
        BAIL_ON_FAIL(sqlite3_bind_null(stmt, 1));
    }
    else
    {
        BAIL_ON_FAIL(sqlite3_bind_int(stmt, 1, parent_id));
    }

    BAIL_ON_FAIL(sqlite3_bind_text(stmt, 2, key_name, -1, SQLITE_STATIC));

    BAIL_ON_FAIL(sqlite3_step(stmt));

    if (rc == SQLITE_ROW)
    {
        WINTC_SAFE_REF_SET(out_id, sqlite3_column_int(stmt, 0));
    }

    sqlite3_finalize(stmt);
    return TRUE;
}

static gboolean db_traverse_to_key(
    const gchar* key_path,
    gint*        out_id
)
{
    GSList* components = path_to_components(key_path);

    WINTC_SAFE_REF_SET(out_id, 0);

    if (!is_valid_key_path(components))
    {
        // FIXME: Respond invalid path
        g_slist_free_full(components, g_free);
        return FALSE;
    }

    // Attempt to traverse to the key
    //
    gint     last_id = 0;
    gboolean success = TRUE;

    for (GSList* iter = components; iter; iter = iter->next)
    {
        const gchar* key_name = (gchar*) iter->data;
        gint         this_id  = 0;

        if (!db_get_regkey_id(key_name, last_id, &this_id))
        {
            success = FALSE;
            break;
        }

        last_id = this_id;
    }

    if (success)
    {
        WINTC_SAFE_REF_SET(out_id, last_id);
    }

    g_slist_free_full(components, g_free);

    return success;
}

static void handle_sqlite_problem(
    gint rc
)
{
    switch (rc)
    {
        case SQLITE_ERROR:
            WINTC_LOG_DEBUG("regsvc: sqlite error: %s", sqlite3_errmsg(s_db));
            break;

        case SQLITE_MISUSE:
            g_critical("%s", "regsvc: sqlite misuse detected");
            break;

        default:
            g_critical("regsvc: unknown sqlite problem %d", rc);
            break;
    }
}

static gboolean is_valid_key_path(
    GSList* components
)
{
    if (g_slist_length(components) == 0)
    {
        return FALSE;
    }

    // Root must be HKCU
    //
    if (
        g_strcmp0(
            (gchar*) g_slist_nth_data(components, 0),
            "HKEY_CURRENT_USER"
        ) != 0
    )
    {
        return FALSE;
    }

    // Max length of individual components is 255
    //
    for (GSList* iter = components; iter; iter = iter->next)
    {
        if (strlen((gchar*) iter->data) > 255)
        {
            return FALSE;
        }
    }

    return TRUE;
}

static gboolean is_valid_value_name(
    const gchar* value_name
)
{
    // Can't contain backslashes
    //
    if (strchr(value_name, '\\'))
    {
        return FALSE;
    }

    // Max length of string is 255
    //
    if (strlen(value_name) > 255)
    {
        return FALSE;
    }

    return TRUE;
}

static GSList* path_to_components(
    const gchar* key_path
)
{
    GSList* components = NULL;
    gchar** split      = g_strsplit(key_path, "\\", -1);

    // Move strings into list
    //
    for (gint i = 0; split[i] != NULL; i++)
    {
        components = g_slist_append(components, split[i]);
    }

    g_free(split); // The list now owns the strings

    // Naughty hack... if the first component is HKCU, expand it to
    // HKEY_CURRENT_USER
    //
    GSList* first = g_slist_nth(components, 0);

    if (
        g_strcmp0(
            (gchar*) first->data,
            "HKCU"
        ) == 0
    )
    {
        g_free(first->data);
        first->data = g_strdup("HKEY_CURRENT_USER");
    }

    return components;
}
