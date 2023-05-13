# Localization
Here are some notes for how to go about localization in this repository.

## Programs - supporting translations
For programs you will want to do the following:

- In the `CMakeLists.txt`, add a call to the `wintc_create_meta_h()` function, this will generate the `src/meta.h` file that defines the `PKG_NAME` constant based on the project name that we can include in a moment

```cmake
... blah blah
pkg_check_modules(...)

wintc_create_meta_h() # <-- put it here!

add_executable(
    ...
)
```

- Next, in `main.c`, you will need to add the `<glib/gi18n.h>` and `<locale.h>` headers, and set up the environment locale in your main method:

```c
...includes...

#include <glib/gi18n.h>
#include <locale.h>

...more includes...


int main(
    int   argc,
    char* argv[]
)
{
    // Set up locales
    //
    setlocale(LC_ALL, "");
    bindtextdomain(PKG_NAME, "/usr/share/locale");
    textdomain(PKG_NAME);
    
    ...rest of main()...
}
```

- Now in the actual application code, anywhere you use a string for text to display, wrap it in `_(...)` so that it gets translated, you will need to add the `<glib/gi18n.h>` header for this to work:

```c
...includes...

#include <glib/gi18n.h>

...more includes...


example_method()
{
    ...blah blah...
    
    gtk_window_set_title(
        GTK_WINDOW(self),
        _("Run") // Translated string!
    );
    
    ...etc...
}
```

- Finally, for common text strings such as `Browse`, `OK`, and `Cancel`, or place names such as `My Computer`, you can find these in the `wintc-shllang` library -- use `wintc_get_control_text()` and `wintc_get_place_name()` respectively (you will need to add the header of course)

```c
...includes...

#include <wintc-shllang.h>

...more includes...


example_method()
{
    ...yip yap...
    
    gtk_label_new(
        wintc_get_control_text(WINTC_CTLTXT_OPEN, WINTC_PUNC_ITEMIZATION)
    );
    
    ...etc...
}
```

## Libraries - supporting translations
For libraries you must do the following:

- In the `CMakeLists.txt`, add a call to the `wintc_create_config_h()` function, this will generate the `src/config.h` file that defines the `GETTEXT_PACKAGE` constant based on the project name that we can include in a moment

```cmake
... blah blah
pkg_check_modules(...)

wintc_create_config_h() # <-- put it here!

add_library(
    ...
)
```

- Things are practically identical to programs, except you will want to use `config.h` (for the constant) and the `<glib/gi18n-lib.h>` header so that you use your library's translations rather than the host program's

```c
#include "config.h" // Must come first, because it has the GETTEXT_PACKAGE constant

...includes...

#include <glib/gi18n-lib.h>

...more includes...
```

## Programs - use common control text and place names in GtkBuilder/GLADE XML
Of course when developing programs it's much more convenient (and tidier) to define the UI layout in XML, and construct it using a `GtkBuilder`.

**Do not use common text like 'OK', 'Cancel', or 'My Documents' in the XML**, instead you can access the constants defined in `libwintc-shllang` by using either `%CTL_<name>%` or `%PLACE_<name>%`. You can also add punctuation to control text by specifying a punctuation constant first with `%PUNC_<name>%`.

For example:
- `%CTL_BROWSE%` will be rewritten to `Browse`
- `%PLACE_DOCUMENTS%` will be rewritten to `My Documents`
- `%PUNC_MOREINPUT%%CTL_OPEN%` will be rewritten to `Open...`

With this in your XML, call `wintc_preprocess_builder_widget_text()` (in `libwintc-shllang`) to have those placeholders rewritten.

Here's an example from `notepad`:
```c
GtkBuilder* builder;

builder =
    gtk_builder_new_from_resource(
        "/uk/oddmatics/wintc/notepad/notepad.ui"
    );
    
wintc_preprocess_builder_widget_text(builder);

... use the builder now ...
```

## Tip: `_()` vs. `N_()`
If you ever look at the source code for `wintc-shllang` you might notice some usage of `_()` and some of `N_()`.

`N_()` does not cause any translation at runtime, it is basically just a marker to say, "hey, put this string in the POT file". This is useful if you have strings in an array or something, composed dynamically (in which case `_()` wouldn't work because you'd need a translation for every possible composed string).

## Creating a .pot file ready for translations
The `.pot` file is basically the source for translations to be based off of. You will need to export all the translatable strings from the source code to a `.pot` file in order to use the translation functions.

Use `xgettext --keyword=_ --language=C -o po/locale.pot src/*` within the component's directory (eg.` /shell/run`) to generate the POT file (eg. `/shell/run/po/locale.pot`).

You may want `xgettext --keyword=N_ --language=C -o po/locale.pot src/*` if you're using `N_()` to mark strings, it's up to you what you're doing.

## Use the locale tools to generate strings from Windows MUI sources
Tools exist in this repository to try and do a lot of the brunt work for you, rather than you having to go out and refer to Windows yourself.

In the `<repo root>/tools/locale` directory you'll find a number of scripts. There is a `README.MD` in that same directory that explains how to use them.

The gist of it is, you'll want to use `muiprep.sh` to prepare your sources (aka, a script that extracts everything ready for the next script to read) and then `transpot.sh` to actually translate your POT file into all the available languages.
