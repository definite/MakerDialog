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
 * UI relative definitions and functions.
 *
 * This module declares the UI relative functions,
 * and a interface for toolkit handlers.
 *
 * Toolkit handler is a bridge between MakerDialog and UI toolkit
 * such as Gtk or Qt.
 */

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
    MAKER_DIALOG_RESPONSE_NONE=-1,		//!< Returned if an action widget has no response id, or if the dialog gets programmatically hidden or destroyed.
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
 * Response id for UI components.
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
 * Data structure of MakerDialog toolkit handler.
 *
 * Note that normally you don't have to directly use the members here.
 * The values of members will be filled by calling corresponding toolkit handler.
 *
 * These members are listed here for convenience for
 * developer of toolkit handlers.
 */
typedef struct {
    /**
     * Callback function to get the value from UI component.
     * Called by maker_dialog_ui_get_value().
     */
    GValue * (* component_get_value)(MakerDialogUi *dlgUi, const gchar *key);

    /**
     *  Callback function to set a value to a property, so the corresponding UI component can show the value.
     *  Called by maker_dialog_ui_set_value().
     */
    void (* component_set_value)(MakerDialogUi *dlgUi, const gchar *key, GValue *value);

    /**
     *  Callback function to construct the "real" toolkit dialog UI.
     *  Called by maker_dialog_ui_construct().
     */
    gpointer (* dialog_construct)(MakerDialogUi *dlgUi, gpointer parentWindow, gboolean modal);

     /**
      *  Callback function to execute a dialog UI until the dialog either emits
      *  the "response" signal, or is destroyed.
      *  Called by maker_dialog_ui_run().
      */
     gint (* dialog_run)(MakerDialogUi *dlgUi);

     /**
      *  Callback function to show the dialog UI.
      *  Called by maker_dialog_ui_show().
      */
     void (* dialog_show)(MakerDialogUi *dlgUi);

     /**
      *  Callback function to hide the dialog UI.
      *  Called by maker_dialog_ui_hide().
      */
    void (* dialog_hide)(MakerDialogUi *dlgUi);

    /**
     *  Callback function to destroy the dialog UI.
     *  Called by maker_dialog_ui_destroy().
     */
    void (* dialog_destroy)(MakerDialogUi *dlgUi);
} MakerDialogToolkitHandler;

/**
 * UI instance for MakerDialog.
 */
struct _MakerDialogUi{
    MakerDialog	*mDialog;		//!< Referring MakerDialog.
    gpointer dialogObj;			//!< The toolkit dialog object.
    MakerDialogToolkitHandler *toolkitHandler; //!< The toolkit handler which connects to UI toolkit front-end.
};

/**
 * Initialize a MakerDialogUi, the UI front-end of MakerDialog.
 *
 * This function initializes an UI front-end using the given toolkit handler
 * for the MakerDialog.
 * During initialization, the new MakerDialogUi is associated to the #mDialog.
 * Thus, maker_dialog_destroy() can free the associated MakerDialogUi as well.
 *
 * This function is meant for toolkit handler developers.
 * For Gtk or Qt users, it is more convenient to call
 * maker_dialog_ui_use_gtk() or maker_dialog_ui_use_qt4() than using this
 * function directly.
 *
 * @param mDialog A MakeDialog.
 * @param toolkitHandler The toolkit handler for that front end.
 * @return A newly allocated MakerDialogUi instance; NULL if failed.
 */
MakerDialogUi *maker_dialog_ui_init(MakerDialog *mDialog, MakerDialogToolkitHandler *toolkitHandler);


/**
 * Construct an UI dialog object (such as GtkDialog or QDialog) for later use.
 *
 * @param mDialog A MakerDialog.
 * @param parentWindow The parent window which can invoke this dialog. Can be
 * NULL.
 * @param modal Whether the dialog is modal.
 * @return TRUE if succeeded; FALSE otherwise.
 */
gboolean maker_dialog_ui_construct(MakerDialog *mDialog, gpointer parentWindow, gboolean modal);

/**
 * Blocks in a recursive main loop until the dialog either emits the "response" signal, or is destroyed.
 *
 * This function aspires gtk_dialog_run().
 * Show the dialog by using dialog_show().
 * @param mDialog A MakerDialog.
 * @return The respond ID.
 */
gint maker_dialog_ui_run(MakerDialog *mDialog);

/**
 * Show the dialog by using dialog_show().
 *
 * @param mDialog A MakerDialog.
 */
void maker_dialog_ui_show(MakerDialog *mDialog);

/**
 * Hide the dialog.
 *
 * @param mDialog A MakerDialog.
 */
void maker_dialog_ui_hide(MakerDialog *mDialog);

/**
 * Destroy and free the UI.
 *
 * @param mDialog A MakerDialog.
 */
void maker_dialog_ui_destroy(MakerDialog *mDialog);

