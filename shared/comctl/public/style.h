/** @file */

#ifndef __COMCTL_STYLE_H__
#define __COMCTL_STYLE_H__

/**
 * @def WINTC_CTL_BUTTON_BOX_CSS_CLASS
 *
 * The CSS class for applying WinTC standard button box styles, intended for
 * GtkBox widgets.
 */
#define WINTC_CTL_BUTTON_BOX_CSS_CLASS "wintc-button-box"

//
// PUBLIC FUNCTIONS
//

/**
 * Installs the common controls stylesheet to the default display, the styles
 * have fallback priority and can be overridden by themes and programs.
 */
void wintc_ctl_install_default_styles(void);

#endif
