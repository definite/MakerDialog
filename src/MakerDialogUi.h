/*
 * Copyright © 2009  Red Hat, Inc. All rights reserved.
 * Copyright © 2009  Ding-Yi Chen <dchen at redhat.com>
 *
 *  This file is part of Mkdg.
 *
 *  MakerDialog is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  MakerDialog is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Mkdg.  If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * @file MkdgUi.h
 * UI interface for Mkdg.
 *
 * This interface bridges between  MakerDialog and UI toolkits
 * such as Gtk or Qt.
 *
 * Through the UI interface,
 * MakerDialog generates corresponding UI components
 * to manipulate the property values.
 */
/**
 * Separator used in Widget ID.
 */
#ifndef MKDG_UI_H_
#define MKDG_UI_H_
#define WIDGET_ID_SEPARATOR "+"

/**
 * Describe the dimension (size) of a visible component.
 *
 * Note that -1 in either width or height means it can expend indefinitely.
 */
typedef struct _MkdgDimension{
    gint width;		//!< Width of a visible component.
    gint height;	//!< Height of a visible component.
}MkdgDimension;

/**
 * Describe the alignment of a visible component.
 *
 * Alignment of a visible component.
 */
typedef struct _MkdgAlignment{
    gfloat x;	//!< Alignment in X axis. 0 for Left; 0.5 for Center; 1.0 for Right.
    gfloat y;	//!< Alignment in Y axis. 0 for Top; 0.5 for Center; 1.0 for Right.
}MkdgAlignment;

/**
 * Predefined values for use as response ids in Mkdg.
 * All predefined values are negative, where application developer can uses positive
 * values for application-defined response ids.
 *
 * These ids are actually coming from GTK_RESPONSE ids.
 */
typedef enum{
    MKDG_RESPONSE_INVALID=-1,		//!< The respond is invalid. Use in parser.
    MKDG_RESPONSE_NIL=-2,		//!< Returned if the dialog gets programmatically hidden or destroyed, or serves as button definition terminator.
    MKDG_RESPONSE_REJECT=-3,		//!< Developers can use this id in their own application. Not internally used by Mkdg.
    MKDG_RESPONSE_ACCEPT=-4,		//!< Developers can use this id in their own application. Not internally used by Mkdg.
    MKDG_RESPONSE_DELETE_EVENT=-5,	//!< Returned if the dialog is deleted.
    MKDG_RESPONSE_OK=-6,		//!< Returned when an "OK" button is pressed.
    MKDG_RESPONSE_CANCEL=-7,		//!< Returned when a "Cancel" button is pressed.
    MKDG_RESPONSE_CLOSE=-8,		//!< Returned when a "Close" button is pressed.
    MKDG_RESPONSE_YES=-9,		//!< Returned when a "Yes" button is pressed.
    MKDG_RESPONSE_NO=-10,		//!< Returned when a "No" button is pressed.
    MKDG_RESPONSE_APPLY=-11,		//!< Returned when a "Apply" button is pressed.
    MKDG_RESPONSE_FILE=-12,		//!< Returned when a "File" button is pressed.
    MKDG_RESPONSE_NEW=-13,		//!< Returned when a "New" button is pressed.
    MKDG_RESPONSE_OPEN=-14,		//!< Returned when a "Open" button is pressed.
    MKDG_RESPONSE_SAVE=-15,		//!< Returned when a "Save" button is pressed.
    MKDG_RESPONSE_SAVE_AS=-16,		//!< Returned when a "Save As" button is pressed.
    MKDG_RESPONSE_PRINT=-17,		//!< Returned when a "Print" button is pressed.
    MKDG_RESPONSE_QUIT=-18,		//!< Returned when a "Apply" button is pressed.
    MKDG_RESPONSE_EDIT=-19,		//!< Returned when a "Edit" button is pressed.
    MKDG_RESPONSE_UNDO=-20,		//!< Returned when a "Undo" button is pressed.
    MKDG_RESPONSE_REDO=-21,		//!< Returned when a "Redo" button is pressed.
    MKDG_RESPONSE_REFRESH=-22,		//!< Returned when a "Refresh" button is pressed.
    MKDG_RESPONSE_CUT=-23,		//!< Returned when a "Cut" button is pressed.
    MKDG_RESPONSE_COPY=-24,		//!< Returned when a "Copy" button is pressed.
    MKDG_RESPONSE_PASTE=-25,		//!< Returned when a "Paste" button is pressed.
    MKDG_RESPONSE_SELECT_ALL=-26,	//!< Returned when a "Select all" button is pressed.
    MKDG_RESPONSE_FIND=-27,		//!< Returned when a "Find" button is pressed.
    MKDG_RESPONSE_FIND_AND_REPLACE=-28,	//!< Returned when a "Find and replace" button is pressed.
    MKDG_RESPONSE_HELP=-29,		//!< Returned when a "Help" button is pressed.
    MKDG_RESPONSE_GOTO_TOP=-30,		//!< Returned when a "Top" button is pressed.
    MKDG_RESPONSE_GOTO_BOTTOM=-31,	//!< Returned when a "Bottom" button is pressed.
    MKDG_RESPONSE_GOTO_FIRST=-32,	//!< Returned when a "First" button is pressed.
    MKDG_RESPONSE_GOTO_LAST=-33,	//!< Returned when a "Last" button is pressed.
    MKDG_RESPONSE_GO_UP=-34,		//!< Returned when a "Up" button is pressed.
    MKDG_RESPONSE_GO_DOWN=-35,		//!< Returned when a "Down" button is pressed.
    MKDG_RESPONSE_GO_BACK=-36,		//!< Returned when a "Back" button is pressed.
    MKDG_RESPONSE_GO_FORWARD=-37,	//!< Returned when a "Forward" button is pressed.
} MKDG_RESPONSE;

/**
 * Response id for UI widgets.
 */
typedef gint MkdgResponse;

/**
 * Specification of buttons.
 *
 * The buttons will be appeared on the bottom of the dialog, label with the
 * button text provide.
 *
 * If using the predefined button, the button text can be set to NULL to
 * use the default text provide by the toolkit.
 */
typedef struct _MkdgButtonSpec{
    MkdgResponse responseId;	//!< Response Id when this button is pressed.
    const char *buttonText;		//!< The text on the button. Can be NULL.
} MkdgButtonSpec;

typedef struct _MkdgUi MkdgUi;

/**
 * MakerDialog UI callback functions to be implemented.
 *
 * These callback functions connect to UI tookit
 * to perform corresponding tasks.
 * These callback functions are called automatically, so no need
 * to call them directly.
 */
typedef struct {
    /**
     * Callback function to get the value from UI widget.
     *
     * Called by mkdg_get_value().
     * @param dlgUi		A MakerDialog UI instance.
     * @param key		Key of a property.
     * @return The value held by UI.
     */
    MkdgValue * (* widget_get_value)(MkdgUi *dlgUi, const gchar *key);

    /**
     * Callback function to set a value to an UI widget.
     *
     * Called by mkdg_set_value().
     * @param dlgUi		A MakerDialog UI instance.
     * @param key		Key of a property.
     * @param value		Value to be set to UI widget.
     */
    void (* widget_set_value)(MkdgUi *dlgUi, const gchar *key, MkdgValue *value);

    /**
     * Callback function to control the widget.
     *
     * Called by mkdg_ui_update().
     *
     * @param dlgUi		A MakerDialog UI instance.
     * @param key		Key of a property.
     * @param control		Widget control flag. See #MKDG_WIDGET_CONTROL
     */
    void (* widget_control)(MkdgUi *dlgUi, const gchar *key, MkdgWidgetControl control);

    /**
     * Callback function to construct the actual toolkit dialog UI.
     * Called by mkdg_ui_construct().
     *
     * @param dlgUi		A MakerDialog UI instance.
     * @param parentWindow	Parent window of the Mkdg.
     * @param modal		Whether the dialog prevent interaction with other window in the same application.
     * @return The actual UI instance.
     */
    gpointer (* dialog_construct)(MkdgUi *dlgUi, gpointer parentWindow, gboolean modal);

     /**
      * Callback function to execute a dialog UI until the dialog either emits
      * the "response" signal, or is destroyed.
      *
      * Called by mkdg_ui_run().
      * @param dlgUi		A MakerDialog UI instance.
      * @return Respond Id.
      */
    MkdgResponse (* dialog_run)(MkdgUi *dlgUi);

     /**
      * Callback function to show the dialog UI.
      *
      * Called by mkdg_ui_show().
      * @param dlgUi		A MakerDialog UI instance.
      */
     void (* dialog_show)(MkdgUi *dlgUi);

     /**
      * Callback function to hide the dialog UI.
      *
      * Called by mkdg_ui_hide().
      * @param dlgUi		A MakerDialog UI instance.
      */
    void (* dialog_hide)(MkdgUi *dlgUi);

    /**
     * Callback function to destroy the dialog UI.
     *
     * Called by mkdg_ui_destroy().
     * @param dlgUi		A MakerDialog UI instance.
     */
    void (* dialog_destroy)(MkdgUi *dlgUi);

    /**
     * Callback function to get the UI widget.
     *
     * Called by mkdg_ui_get_widget().
     * @param dlgUi		A MakerDialog UI instance.
     * @param key		Key of a property.
     */
    gpointer (* get_widget)(MkdgUi *dlgUi, const gchar *key);
} MkdgToolkitInterface;

/**
 * Data structure of MakerDialog UI.
 */
struct _MkdgUi{
    Mkdg	*mDialog;		//!< "Parent" Mkdg.
    gpointer dlgObj;			//!< The toolkit dialog object.
    MkdgToolkitInterface *toolkitInterface; //!< The toolkit interface which connects to UI toolkit front-end.
};

/**
 * Initialize a MkdgUi, the UI front-end of Mkdg.
 *
 * This function initializes an UI front-end using the given toolkit interface
 * for the Mkdg.
 * During initialization, the new MkdgUi is associated to the \a mDialog.
 * Thus, mkdg_destroy() can free the associated MkdgUi as well.
 *
 * This function is meant for toolkit interface developers.
 * For Gtk or Qt users, it is more convenient to call
 * mkdg_ui_use_gtk() or mkdg_ui_use_qt4() than using this
 * function directly.
 *
 * @param mDialog 		A MakeDialog.
 * @param toolkitInterface 	The toolkit interface for that front end.
 * @return A newly allocated MkdgUi instance; NULL if failed.
 */
MkdgUi *mkdg_ui_init(Mkdg *mDialog, MkdgToolkitInterface *toolkitInterface);

/**
 * Construct an UI dialog object (such as GtkDialog or QDialog) for later use.
 *
 * @param dlgUi 	A MakerDialog UI instance.
 * @param parentWindow The parent window which can invoke this dialog. Can be
 * NULL.
 * @param modal Whether the dialog is modal.
 * @return TRUE if succeeded; FALSE otherwise.
 */
gboolean mkdg_ui_construct(MkdgUi *dlgUi, gpointer parentWindow, gboolean modal);

/**
 * Destroy and free the UI.
 *
 * @param dlgUi 	A MakerDialog UI instance.
 */
void mkdg_ui_destroy(MkdgUi *dlgUi);

/**
 * Blocks in a recursive main loop until the dialog either emits the "response" signal, or is destroyed.
 *
 * This function aspires gtk_dialog_run().
 * Show the dialog by using dialog_show().
 * @param dlgUi 	A MakerDialog UI instance.
 * @return The respond ID.
 */
gint mkdg_ui_run(MkdgUi *dlgUi);

/**
 * Show the dialog by using dialog_show().
 *
 * @param dlgUi 	A MakerDialog UI instance.
 */
void mkdg_ui_show(MkdgUi *dlgUi);

/**
 * Hide the dialog.
 *
 * @param dlgUi 	A MakerDialog UI instance.
 */
void mkdg_ui_hide(MkdgUi *dlgUi);

/**
 * Update the property value using the value in UI widget.
 *
 * This function copies UI widget value to property value.
 *
 * If validateFunc() is also defined, then the argument value will be checked with it,
 * if it does not pass, this function returns FALSE.
 *
 * If widget_get_value() in ::MkdgToolkitInterface is not defined,
 * this function returns FALSE as well.
 *
 * @param dlgUi 	A MakerDialog UI instance.
 * @param ctx     A property context to be update.
 * @return TRUE if succeed, FALSE if the property value does not pass
 *  validation, or widget_get_value() does not exist.
 * @see mkdg_apply_value()
 * @see mkdg_set_value()
 */
gboolean mkdg_ui_update(MkdgUi *dlgUi, MkdgPropertyContext *ctx);

/**
 * Get the corresponding widget.
 *
 * This function gets the UI widget that represent the value of a property.
 * So further control over the widget is possible.
 *
 * However, it may make the program depends on one toolkit and harder to
 * port to other toolkit, so use it with care.
 * @param dlgUi 	A MakerDialog UI instance.
 * @param key		Key of a MakerDialog property.
 * @return The corresponding UI widget; or \c NULL if get_widget() in MkdgToolkitInterface is not implemented.
 */
gpointer mkdg_ui_get_widget(MkdgUi *dlgUi, const gchar *key);

/**
 * Parse a button response ID from a string.
 *
 * This function parses two kinds of strings in to response ids.
 * - Pre-defined respond id in #MkdgResponsePredefined without the "MKDG_RESPONSE_" prefix, such as "OK", "CLOSE", "SAVE_AS"; or
 * - string that can converted into positive integer.
 *
 * All others string will return \c MKDG_RESPONSE_INVALID.
 *
 * @param idStr 	The string to be parsed.
 * @return Corresponding response id; or \c MKDG_RESPONSE_INVALID if parse failed.
 */
MkdgResponse mkdg_parse_button_response_id(const gchar *idStr);

#endif /* MKDG_UI_H_ */

