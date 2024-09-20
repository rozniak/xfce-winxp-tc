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

#define WINTC_CTL_BUTTON_CSS_CLASS "wintc-button"

#define WINTC_CTL_MARGINB_MD_CSS_CLASS "wintc-mb-md"
#define WINTC_CTL_MARGINL_MD_CSS_CLASS "wintc-ml-md"
#define WINTC_CTL_MARGINR_MD_CSS_CLASS "wintc-mr-md"
#define WINTC_CTL_MARGINT_MD_CSS_CLASS "wintc-mt-md"

#define WINTC_CTL_MARGINB_LG_CSS_CLASS "wintc-mb-lg"
#define WINTC_CTL_MARGINL_LG_CSS_CLASS "wintc-ml-lg"
#define WINTC_CTL_MARGINR_LG_CSS_CLASS "wintc-mr-lg"
#define WINTC_CTL_MARGINT_LG_CSS_CLASS "wintc-mt-lg"

//
// PUBLIC FUNCTIONS
//

/**
 * Installs the common controls stylesheet to the default display, the styles
 * have fallback priority and can be overridden by themes and programs.
 */
void wintc_ctl_install_default_styles(void);

#endif
