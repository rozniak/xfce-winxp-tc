/** @file */

#ifndef __SHLANG_PUNCTUATION_H__
#define __SHLANG_PUNCTUATION_H__

//
// PUBLIC ENUMS
//

/**
 * Specifies punctuation formats for translating text.
 */
typedef enum {
    /** No punctuation necessary. */
    WINTC_PUNC_NONE,
    /**
     * 'More input' typically refers to ellipsis, for use on buttons and menus
     * where more details are required from the user (eg. 'Browse...')
     */
    WINTC_PUNC_MOREINPUT,
    /**
     * 'Itemization' gives context to a label (a colon in English), used next to a
     * field or list (eg. 'Username:')
     */
    WINTC_PUNC_ITEMIZATION
} WinTCPunctuationId;

#endif
