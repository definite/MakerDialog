/*
 * Copyright © 2009  Red Hat, Inc. All rights reserved.
 * Copyright © 2009  Ding-Yi Chen <dchen at redhat.com>
 *
 *  This file is part of MakerDialog.
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
 *  along with MakerDialog.  If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * @file MakerDialogUi.h
 * User interface module for MakerDialog.
 *
 * This module defines the UI relative data structure, functions,
 * and an interface for bridging between MakerDialog and UI toolkit
 * such as Gtk or Qt.
 *
 * In MakerDialog, a property is a configuration option which associate with a
 * value. According to property specification, MakerDialog generates
 * corresponding UI component to manipulate the property value.
 */
/**
 * Separator used in Widget ID.
 */
#define WIDGET_ID_SEPARATOR "+"

/**
 * Describe the dimension (size) of a visible component.
 *
 * Note that -1 in either width or height means it can expend indefinitely.
 */
typedef struct _MakerDialogDimension{
    gint width;		//!< Width of a visible component.
    gint height;	//!< Height of a visible component.
}MakerDialogDimension;

/**
 * Describe the alignment (size) of a visible component.
 *
 */
typedef struct _MakerDialogAlignment{
    gfloat x;	//!< Alignment in X axis. 0 for Left; 0.5 for Center; 1.0 for Right.
    gfloat y;	//!< Alignment in Y axis. 0 for Top; 0.5 for Center; 1.0 for Right.
}MakerDialogAlignment;

/**
 * Predefined values for use as response ids in MakerDialog.
 * All predefined values are negative, where application developer can uses positive
 * values for application-defined response ids.
 *
 * These ids are actually coming from GTK_RESPONSE ids.
 */
typedef enum{
    MAKER_DIALOG_RESPONSE_NIL=-1,		//!< Returned if the dialog gets programmatically hidden or destroyed, or serves as button definition terminator.
    MAKER_DIALOG_RESPONSE_REJECT=-2,		//!< Developers can use this id in their own application. Not internally used by MakerDialog.
    MAKER_DIALOG_RESPONSE_ACCEPT=-3,		//!< Developers can use this id in their own application. Not internally used by MakerDialog.
    MAKER_DIALOG_RESPONSE_DELETE_EVENT=-4,	//!< Returned if the dialog is deleted.
    MAKER_DIALOG_RESPONSE_OK=-5,		//!< Returned when an "OK" button is pressed.
    MAKER_DIALOG_RESPONSE_CANCEL=-6,		//!< Returned when a "Cancel" button is pressed.
    MAKER_DIALOG_RESPONSE_CLOSE=-7,		//!< Returned when a "Close" button is pressed.
    MAKER_DIALOG_RESPONSE_YES=-8,		//!< Returned when a "Yes" button is pressed.
    MAKER_DIALOG_RESPONSE_NO=-9,		//!< Returned when a "No" button is pressed.
    MAKER_DIALOG_RESPONSE_APPLY=-10,		//!< Returned when a "Apply" button is pressed.
    MAKER_DIALOG_RESPONSE_HELP=11,		//!< Returned when a "Help" button is pressed.
} MakerDialogResponsePredefined;

/**
 * Response id for UI widgets.
 */
typedef gint MakerDialogResponse;

/**
 * Specification of buttons.
 *
 * The buttons will be appeared on the bottom of the dialog, label with the
 * button text provide.
 *
 * If using the predefined button, the button text can be set to NULL to
 * use the default text provide by the toolkit.
 */
typedef struct _MakerDialogButtonSpec{
    MakerDialogResponse responseId;	//!< Response Id when this button is pressed.
    const char *buttonText;		//!< The text on the button. Can be NULL.
} MakerDialogButtonSpec;

typedef struct _MakerDialogUi MakerDialogUi;

/**
 * Data structure of MakerDialog toolkit interface.
 *
 * Note that normally you don't have to directly use the members here.
 * The values of members will be filled by calling corresponding toolkit interface.
 *
 * These members are listed here for convenience for
 * developer of toolkit interfaces.
 */
typedef struct {
    /**
     * Callback function to get the value from UI widget.
     *
     * Called by maker_dialog_get_value().
     * @param dlgUi		A MakerDialog UI instance.
     * @param key		Key of a property.
     * @return The value held by UI.
     */
    MkdgValue * (* widget_get_value)(MakerDialogUi *dlgUi, const gchar *key);

    /**
     * Callback function to set a value to an UI widget.
     *
     * Called by maker_dialog_set_value().
     * @param dlgUi		A MakerDialog UI instance.
     * @param key		Key of a property.
     * @param value		Value to be set to UI widget.
     */
    void (* widget_set_value)(MakerDialogUi *dlgUi, const gchar *key, MkdgValue *value);

    /**
     * Callback function to control the widget.
     *
     * Called by maker_dialog_ui_update().
     *
     * @param dlgUi		A MakerDialog UI instance.
     * @param key		Key of a property.
     * @param control		Widget control flag. See #MAKER_DIALOG_WIDGET_CONTROL
     */
    void (* widget_control)(MakerDialogUi *dlgUi, const gchar *key, MakerDialogWidgetControl control);

    /**
     * Callback function to construct the actual toolkit dialog UI.
     * Called by maker_dialog_ui_construct().
     *
     * @param dlgUi		A MakerDialog UI instance.
     * @param parentWindow	Parent window of the MakerDialog.
     * @param modal		Whether the dialog prevent interaction with other window in the same application.
     * @return The actual UI instance.
     */
    gpointer (* dialog_construct)(MakerDialogUi *dlgUi, gpointer parentWindow, gboolean modal);

     /**
      * Callback function to execute a dialog UI until the dialog either emits
      * the "response" signal, or is destroyed.
      *
      * Called by maker_dialog_ui_run().
      * @param dlgUi		A MakerDialog UI instance.
      * @return Respond Id.
      */
    MakerDialogResponse (* dialog_run)(MakerDialogUi *dlgUi);

     /**
      * Callback function to show the dialog UI.
      *
      * Called by maker_dialog_ui_show().
      * @param dlgUi		A MakerDialog UI instance.
      */
     void (* dialog_show)(MakerDialogUi *dlgUi);

     /**
      * Callback function to hide the dialog UI.
      *
      * Called by maker_dialog_ui_hide().
      * @param dlgUi		A MakerDialog UI instance.
      */
    void (* dialog_hide)(MakerDialogUi *dlgUi);

    /**
     * Callback function to destroy the dialog UI.
     *
     * Called by maker_dialog_ui_destroy().
     * @param dlgUi		A MakerDialog UI instance.
     */
    void (* dialog_destroy)(MakerDialogUi *dlgUi);

    /**
     * Callback function to get the UI widget.
     *
     * Called by maker_dialog_ui_get_widget().
     * @param dlgUi		A MakerDialog UI instance.
     * @param key		Key of a property.
     */
    gpointer (* get_widget)(MakerDialogUi *dlgUi, const gchar *key);
} MakerDialogToolkitInterface;

/**
 * Data structure of MakerDialog UI.
 */
struct _MakerDialogUi{
    MakerDialog	*mDialog;		//!< "Parent" MakerDialog.
    gpointer dlgObj;			//!< The toolkit dialog object.
    MakerDialogToolkitInterface *toolkitInterface; //!< The toolkit interface which connects to UI toolkit front-end.
};

/**
 * Initialize a MakerDialogUi, the UI front-end of MakerDialog.
 *
 * This function initializes an UI front-end using the given toolkit interface
 * for the MakerDialog.
 * During initialization, the new MakerDialogUi is associated to the \a mDialog.
 * Thus, maker_dialog_destroy() can free the associated MakerDialogUi as well.
 *
 * This function is meant for toolkit interface developers.
 * For Gtk or Qt users, it is more convenient to call
 * maker_dialog_ui_use_gtk() or maker_dialog_ui_use_qt4() than using this
 * function directly.
 *
 * @param mDialog 		A MakeDialog.
 * @param toolkitInterface 	The toolkit interface for that front end.
 * @return A newly allocated MakerDialogUi instance; NULL if failed.
 */
MakerDialogUi *maker_dialog_ui_init(MakerDialog *mDialog, MakerDialogToolkitInterface *toolkitInterface);

/**
 * Construct an UI dialog object (such as GtkDialog or QDialog) for later use.
 *
 * @param dlgUi 	A MakerDialog UI instance.
 * @param parentWindow The parent window which can invoke this dialog. Can be
 * NULL.
 * @param modal Whether the dialog is modal.
 * @return TRUE if succeeded; FALSE otherwise.
 */
gboolean maker_dialog_ui_construct(MakerDialogUi *dlgUi, gpointer parentWindow, gboolean modal);

/**
 * Destroy and free the UI.
 *
 * @param dlgUi 	A MakerDialog UI instance.
 */
void maker_dialog_ui_destroy(MakerDialogUi *dlgUi);

/**
 * Blocks in a recursive main loop until the dialog either emits the "response" signal, or is destroyed.
 *
 * This function aspires gtk_dialog_run().
 * Show the dialog by using dialog_show().
 * @param dlgUi 	A MakerDialog UI instance.
 * @return The respond ID.
 */
gint maker_dialog_ui_run(MakerDialogUi *dlgUi);

/**
 * Show the dialog by using dialog_show().
 *
 * @param dlgUi 	A MakerDialog UI instance.
 */
void maker_dialog_ui_show(MakerDialogUi *dlgUi);

/**
 * Hide the dialog.
 *
 * @param dlgUi 	A MakerDialog UI instance.
 */
void maker_dialog_ui_hide(MakerDialogUi *dlgUi);

/**
 * Update the property value using the value in UI widget.
 *
 * This function copies UI widget value to property value.
 *
 * If validateFunc() is also defined, then the argument value will be checked with it,
 * if it does not pass, this function returns FALSE.
 *
 * If widget_get_value() in ::MakerDialogToolkitInterface is not defined,
 * this function returns FALSE as well.
 *
 * @param dlgUi 	A MakerDialog UI instance.
 * @param ctx     A property context to be update.
 * @return TRUE if succeed, FALSE if the property value does not pass
 *  validation, or widget_get_value() does not exist.
 * @see maker_dialog_apply_value()
 * @see maker_dialog_set_value()
 */
gboolean maker_dialog_ui_update(MakerDialogUi *dlgUi, MakerDialogPropertyContext *ctx);

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
 * @return The corresponding UI widget; or \c NULL if get_widget() in MakerDialogToolkitInterface is not implemented.
 */
gpointer maker_dialog_ui_get_widget(MakerDialogUi *dlgUi, const gchar *key);

