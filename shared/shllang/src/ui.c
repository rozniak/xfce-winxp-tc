#include "config.h"

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <wintc-comgtk.h>

#include "controls.h"
#include "places.h"
#include "punctuation.h"
#include "ui.h"

//
// MACROS
//
#define SHLLANG_MAP_CTLTXT(str, val)  \
    g_hash_table_insert(s_ctltxt_map, str, GINT_TO_POINTER(val))
#define SHLLANG_MAP_PLACE(str, val) \
    g_hash_table_insert(s_place_map, str, GINT_TO_POINTER(val))
#define SHLLANG_MAP_PUNC(str, val) \
    g_hash_table_insert(s_punc_map, str, GINT_TO_POINTER(val))

//
// STATIC DATA
//
static gboolean s_initialized = FALSE;

static GHashTable* s_ctltxt_map = NULL;
static GHashTable* s_place_map  = NULL;
static GHashTable* s_punc_map   = NULL;

static GRegex* s_langstr_regex = NULL;

//
// FORWARD DECLARATIONS
//
static gboolean initialize_text_maps(void);

static void process_widget_text(
    void* widget,
    void* user_data
);

static const gchar* translate_widget_text(
    const gchar* widget_text
);

//
// PUBLIC FUNCTIONS
//
void wintc_preprocess_builder_widget_text(
    GtkBuilder* builder
)
{
    GSList* widgets = gtk_builder_get_objects(builder);

    initialize_text_maps();

    g_slist_foreach(
        widgets,
        (GFunc) process_widget_text,
        NULL
    );

    g_slist_free(widgets);
}

//
// PRIVATE FUNCTIONS
//
static gboolean initialize_text_maps(void)
{
    if (s_initialized)
    {
        return TRUE;
    }

    // Init regex
    //
    GError* regex_err = NULL;

    s_langstr_regex =
        g_regex_new(
            "^(%(?<punc>PUNC_[A-Z]+)%)?%(?<str>[A-Z_]+)%$",
            0,
            0,
            &regex_err
        );

    if (regex_err != NULL)
    {
        wintc_log_error_and_clear(&regex_err);
        return FALSE;
    }

    // Init map
    //
    s_ctltxt_map  = g_hash_table_new(g_str_hash, g_str_equal);
    s_place_map   = g_hash_table_new(g_str_hash, g_str_equal);
    s_punc_map    = g_hash_table_new(g_str_hash, g_str_equal);

    // Insert punctuation
    //
    SHLLANG_MAP_PUNC("PUNC_NONE",        WINTC_PUNC_NONE);
    SHLLANG_MAP_PUNC("PUNC_MOREINPUT",   WINTC_PUNC_MOREINPUT);
    SHLLANG_MAP_PUNC("PUNC_ITEMIZATION", WINTC_PUNC_ITEMIZATION);

    // Insert strings
    //
    SHLLANG_MAP_CTLTXT("CTL_OK",     WINTC_CTLTXT_OK);
    SHLLANG_MAP_CTLTXT("CTL_CANCEL", WINTC_CTLTXT_CANCEL);
    SHLLANG_MAP_CTLTXT("CTL_YES",    WINTC_CTLTXT_YES);
    SHLLANG_MAP_CTLTXT("CTL_NO",     WINTC_CTLTXT_NO);
    SHLLANG_MAP_CTLTXT("CTL_ABORT",  WINTC_CTLTXT_ABORT);
    SHLLANG_MAP_CTLTXT("CTL_RETRY",  WINTC_CTLTXT_RETRY);
    SHLLANG_MAP_CTLTXT("CTL_IGNORE", WINTC_CTLTXT_IGNORE);
    SHLLANG_MAP_CTLTXT("CTL_HELP",   WINTC_CTLTXT_HELP);

    SHLLANG_MAP_CTLTXT("CTL_FILEMENU",   WINTC_CTLTXT_FILEMENU);
    SHLLANG_MAP_CTLTXT("CTL_EDITMENU",   WINTC_CTLTXT_EDITMENU);
    SHLLANG_MAP_CTLTXT("CTL_VIEWMENU",   WINTC_CTLTXT_VIEWMENU);
    SHLLANG_MAP_CTLTXT("CTL_INSERTMENU", WINTC_CTLTXT_INSERTMENU);
    SHLLANG_MAP_CTLTXT("CTL_FORMATMENU", WINTC_CTLTXT_FORMATMENU);
    SHLLANG_MAP_CTLTXT("CTL_TOOLSMENU",  WINTC_CTLTXT_TOOLSMENU);
    SHLLANG_MAP_CTLTXT("CTL_WINDOWMENU", WINTC_CTLTXT_WINDOWMENU);
    SHLLANG_MAP_CTLTXT("CTL_HELPMENU",   WINTC_CTLTXT_HELPMENU);

    SHLLANG_MAP_CTLTXT("CTL_NEW",        WINTC_CTLTXT_NEW);
    SHLLANG_MAP_CTLTXT("CTL_BROWSE",     WINTC_CTLTXT_BROWSE);
    SHLLANG_MAP_CTLTXT("CTL_OPEN",       WINTC_CTLTXT_OPEN);
    SHLLANG_MAP_CTLTXT("CTL_SAVE",       WINTC_CTLTXT_SAVE);
    SHLLANG_MAP_CTLTXT("CTL_SAVEAS",     WINTC_CTLTXT_SAVEAS);
    SHLLANG_MAP_CTLTXT("CTL_SAVEALL",    WINTC_CTLTXT_SAVEALL);
    SHLLANG_MAP_CTLTXT("CTL_CLOSE",      WINTC_CTLTXT_CLOSE);
    SHLLANG_MAP_CTLTXT("CTL_CLOSEALL",   WINTC_CTLTXT_CLOSEALL);
    SHLLANG_MAP_CTLTXT("CTL_PROPERTIES", WINTC_CTLTXT_PROPERTIES);

    SHLLANG_MAP_CTLTXT("CTL_PRINT",        WINTC_CTLTXT_PRINT);
    SHLLANG_MAP_CTLTXT("CTL_PRINTPREVIEW", WINTC_CTLTXT_PRINTPREVIEW);
    SHLLANG_MAP_CTLTXT("CTL_PAGESETUP",    WINTC_CTLTXT_PAGESETUP);

    SHLLANG_MAP_CTLTXT("CTL_EXIT", WINTC_CTLTXT_EXIT);

    SHLLANG_MAP_CTLTXT("CTL_UNDO",       WINTC_CTLTXT_UNDO);
    SHLLANG_MAP_CTLTXT("CTL_REDO",       WINTC_CTLTXT_REDO);
    SHLLANG_MAP_CTLTXT("CTL_CUT",        WINTC_CTLTXT_CUT);
    SHLLANG_MAP_CTLTXT("CTL_COPY",       WINTC_CTLTXT_COPY);
    SHLLANG_MAP_CTLTXT("CTL_PASTE",      WINTC_CTLTXT_PASTE);
    SHLLANG_MAP_CTLTXT("CTL_DELETE",     WINTC_CTLTXT_DELETE);
    SHLLANG_MAP_CTLTXT("CTL_FIND",       WINTC_CTLTXT_FIND);
    SHLLANG_MAP_CTLTXT("CTL_FINDNEXT",   WINTC_CTLTXT_FINDNEXT);
    SHLLANG_MAP_CTLTXT("CTL_REPLACE",    WINTC_CTLTXT_REPLACE);
    SHLLANG_MAP_CTLTXT("CTL_REPLACEALL", WINTC_CTLTXT_REPLACEALL);
    SHLLANG_MAP_CTLTXT("CTL_GOTO",       WINTC_CTLTXT_GOTO);
    SHLLANG_MAP_CTLTXT("CTL_SELECTALL",  WINTC_CTLTXT_SELECTALL);

    SHLLANG_MAP_CTLTXT("CTL_ZOOM",       WINTC_CTLTXT_ZOOM);
    SHLLANG_MAP_CTLTXT("CTL_ZOOMIN",     WINTC_CTLTXT_ZOOMIN);
    SHLLANG_MAP_CTLTXT("CTL_ZOOMOUT",    WINTC_CTLTXT_ZOOMOUT);
    SHLLANG_MAP_CTLTXT("CTL_ZOOMFIT",    WINTC_CTLTXT_ZOOMFIT);
    SHLLANG_MAP_CTLTXT("CTL_ZOOMFULL",   WINTC_CTLTXT_ZOOMFULL);
    SHLLANG_MAP_CTLTXT("CTL_FULLSCREEN", WINTC_CTLTXT_FULLSCREEN);
    SHLLANG_MAP_CTLTXT("CTL_STATUSBAR",  WINTC_CTLTXT_STATUSBAR);

    SHLLANG_MAP_CTLTXT("CTL_FONT",              WINTC_CTLTXT_FONT);
    SHLLANG_MAP_CTLTXT("CTL_FONTSTYLE",         WINTC_CTLTXT_FONTSTYLE);
    SHLLANG_MAP_CTLTXT("CTL_FONTSTYLE_BOLD",    WINTC_CTLTXT_FONTSTYLE_BOLD);
    SHLLANG_MAP_CTLTXT("CTL_FONTSTYLE_ITALIC",  WINTC_CTLTXT_FONTSTYLE_ITALIC);
    SHLLANG_MAP_CTLTXT("CTL_FONTSTYLE_REGULAR", WINTC_CTLTXT_FONTSTYLE_REGULAR);
    SHLLANG_MAP_CTLTXT("CTL_FONTSIZE",          WINTC_CTLTXT_FONTSIZE);
    SHLLANG_MAP_CTLTXT("CTL_WORDWRAP",          WINTC_CTLTXT_WORDWRAP);

    SHLLANG_MAP_CTLTXT("CTL_OPTIONS",   WINTC_CTLTXT_OPTIONS);
    SHLLANG_MAP_CTLTXT("CTL_CUSTOMIZE", WINTC_CTLTXT_CUSTOMIZE);

    SHLLANG_MAP_CTLTXT("CTL_NEWWINDOW",   WINTC_CTLTXT_NEWWINDOW);
    SHLLANG_MAP_CTLTXT("CTL_NEWTAB",      WINTC_CTLTXT_NEWTAB);
    SHLLANG_MAP_CTLTXT("CTL_CLOSEWINDOW", WINTC_CTLTXT_CLOSEWINDOW);
    SHLLANG_MAP_CTLTXT("CTL_CLOSETAB",    WINTC_CTLTXT_CLOSETAB);

    SHLLANG_MAP_CTLTXT("CTL_HELPTOPICS",   WINTC_CTLTXT_HELPTOPICS);
    SHLLANG_MAP_CTLTXT("CTL_ABOUTPROGRAM", WINTC_CTLTXT_ABOUTPROGRAM);

    SHLLANG_MAP_PLACE("PLACE_APPDATA",      WINTC_PLACE_APPDATA);
    SHLLANG_MAP_PLACE("PLACE_DESKTOP",      WINTC_PLACE_DESKTOP);
    SHLLANG_MAP_PLACE("PLACE_DOWNLOADS",    WINTC_PLACE_DOWNLOADS);
    SHLLANG_MAP_PLACE("PLACE_FAVORITES",    WINTC_PLACE_FAVORITES);
    SHLLANG_MAP_PLACE("PLACE_DOCUMENTS",    WINTC_PLACE_DOCUMENTS);
    SHLLANG_MAP_PLACE("PLACE_MUSIC",        WINTC_PLACE_MUSIC);
    SHLLANG_MAP_PLACE("PLACE_PICTURES",     WINTC_PLACE_PICTURES);
    SHLLANG_MAP_PLACE("PLACE_RECENTS",      WINTC_PLACE_RECENTS);
    SHLLANG_MAP_PLACE("PLACE_RECYCLEBIN",   WINTC_PLACE_RECYCLEBIN);
    SHLLANG_MAP_PLACE("PLACE_VIDEO",        WINTC_PLACE_VIDEO);
    SHLLANG_MAP_PLACE("PLACE_ADMINTOOLS",   WINTC_PLACE_ADMINTOOLS);
    SHLLANG_MAP_PLACE("PLACE_DRIVES",       WINTC_PLACE_DRIVES);
    SHLLANG_MAP_PLACE("PLACE_NETHOOD",      WINTC_PLACE_NETHOOD);
    SHLLANG_MAP_PLACE("PLACE_CONTROLPANEL", WINTC_PLACE_CONTROLPANEL);
    SHLLANG_MAP_PLACE("PLACE_CONNECTIONS",  WINTC_PLACE_CONNECTIONS);
    SHLLANG_MAP_PLACE("PLACE_PRINTERS",     WINTC_PLACE_PRINTERS);

    s_initialized = TRUE;

    return TRUE;
}

//
// CALLBACKS
//
static void process_widget_text(
    void* widget,
    WINTC_UNUSED(void* user_data)
)
{
    if (GTK_IS_LABEL(widget))
    {
        GtkLabel* label = GTK_LABEL(widget);

        gtk_label_set_text(
            label,
            translate_widget_text(
                gtk_label_get_text(label)
            )
        );
    }
    else if (GTK_IS_MENU_ITEM(widget))
    {
        if (GTK_IS_SEPARATOR_MENU_ITEM(widget))
        {
            return;
        }

        GtkMenuItem* menu_item = GTK_MENU_ITEM(widget);

        gtk_menu_item_set_label(
            menu_item,
            translate_widget_text(
                gtk_menu_item_get_label(menu_item)
            )
        );
    }
    else if (GTK_IS_BUTTON(widget))
    {
        GtkButton* button = GTK_BUTTON(widget);

        gtk_button_set_label(
            button,
            translate_widget_text(
                gtk_button_get_label(button)
            )
        );
    }
}

static const gchar* translate_widget_text(
    const gchar* widget_text
)
{
    WinTCControlTextId enum_ctl    = 0;
    WinTCShellPlace    enum_place  = 0;
    WinTCPunctuationId enum_punc   = WINTC_PUNC_NONE;
    GMatchInfo*        match_info;
    const gchar*       ret         = widget_text;
    gpointer           tmp_ptr_out = NULL;
    gchar*             text_punc   = NULL;
    gchar*             text_str    = NULL;

    if (!g_regex_match(s_langstr_regex, widget_text, 0, &match_info))
    {
        return ret;
    }

    text_punc = g_match_info_fetch_named(match_info, "punc");
    text_str  = g_match_info_fetch_named(match_info, "str");

    // See if we match any punctuation
    //
    if (text_punc != NULL && strlen(text_punc))
    {
        if (
            g_hash_table_lookup_extended(
                s_punc_map,
                text_punc,
                NULL,
                &tmp_ptr_out
            )
        )
        {
            enum_punc = (WinTCPunctuationId) GPOINTER_TO_INT(tmp_ptr_out);
        }
        else
        {
            g_warning(
                "Unknown punctuation in %s",
                widget_text
            );
        }
    }

    // Attempt to match either control text or a place
    //
    if (
        g_hash_table_lookup_extended(
            s_ctltxt_map,
            text_str,
            NULL,
            &tmp_ptr_out
        )
    )
    {
        enum_ctl = (WinTCControlTextId) GPOINTER_TO_INT(tmp_ptr_out);

        ret = wintc_get_control_text(enum_ctl, enum_punc);
    }
    else if (
        g_hash_table_lookup_extended(
            s_place_map,
            text_str,
            NULL,
            &tmp_ptr_out
        )
    )
    {
        if (enum_punc != WINTC_PUNC_NONE)
        {
            g_warning(
                "Cannot use punctuation with place names in %s",
                widget_text
            );
        }

        enum_place = (WinTCShellPlace) GPOINTER_TO_INT(tmp_ptr_out);

        ret = wintc_get_place_name(enum_place);
    }
    else
    {
        g_warning(
            "Translation failure for %s",
            widget_text
        );
    }

    g_free(text_punc);
    g_free(text_str);

    g_match_info_unref(match_info);

    return ret;
}
