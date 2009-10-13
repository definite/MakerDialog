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
 * @file MakerDialogToolkitHandler.h
 * Toolkit handler for MakerDialog.
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

/**
 * Data structure of MakerDialog toolkit handler.
 *
 * Note that normally you don't have to directly use the fields here.
 * The fields will be filled by calling corresponding toolkit handler.
 *
 * These fields are listed here for convenience for
 * developer of toolkit handlers.
 */
typedef struct _MakerDialogToolkitHandler{
    gpointer dialogObj;				//!< The toolkit dialog object. Initially this is NULL until maker_dialog_construct() is called.

    /**
     * Callback function to get the value from UI component.
     * Called by maker_dialog_get_value().
     */
    GValue * (* component_get_value)(MakerDialog *dlg, const gchar *key);

    /**
     *  Callback function to set a value to a property, so the corresponding UI component can show the value.
     *  Called by maker_dialog_set_value().
     */
    void (* component_set_value)(MakerDialog *dlg, const gchar *key, GValue *value);

    /**
     *  Callback function to construct the "real" toolkit dialog UI.
     *  Called by maker_dialog_construct().
     */
     gpointer (* dialog_construct)(MakerDialog *dlg);

     /**
      *  Callback function to execute a dialog UI until the dialog either emits
      *  the "response" signal, or is destroyed.
      *  Called by maker_dialog_run().
      */
     gint (* dialog_run)(MakerDialog *dlg);

     /**
      *  Callback function to show the dialog UI.
      *  Called by maker_dialog_show().
      */
     void (* dialog_show)(MakerDialog *dlg);

     /**
      *  Callback function to hide the dialog UI.
      *  Called by maker_dialog_hide().
      */
    void (* dialog_hide)(MakerDialog *dlg);

    /**
     *  Callback function to destroy the dialog UI.
     *  Called by maker_dialog_destroy().
     */
    void (* dialog_destroy)(MakerDialog *dlg);
} MakerDialogToolkitHandler;

