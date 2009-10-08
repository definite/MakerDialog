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
 * @file MakerDialog.h
 * MakerDialog: Make dialog from given data type.
 *
 * Developers can easily make an configuration dialog with maker-dialog.
 * Maker-dialog generates corresponding UI components for configuration
 * properties, such as data type, default value, valid values.
 * That is, developers no longer need to worry about the implementation
 * details for various toolkit.
 *
 * Developers, however, need to provide either callback functions or dbus
 * messages for value getting and setting to handle such events.
 */
#ifndef MAKER_DIALOG_H_
#define MAKER_DIALOG_H_
#include <glib.h>
#include <glib-object.h>
#include "MakerDialogProperty.h"

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
 * Data structure of a MakerDialog.
 *
 * @todo Implement functions that uses maxSizeInPixel.
 * @todo Implement functions that uses maxSizeInChar.
 */
typedef struct _MakerDialog MakerDialog;

/**
 * Toolkit handler for a MakerDialog.
 *
 * A wrapper structure for toolkit such as Gtk or Qt.
 * Normally this structure will be automatically filled
 * by the toolkit front-end.
 */
typedef struct _MakerDialogToolkitHandler{
    gpointer dialog_obj;				//!< The toolkit dialog object. Initially this is NULL until maker_dialog_construct() is called.
    GValue * (* component_get_value)(MakerDialog *dlg, const gchar *key);	//!< Get the value in UI component. The GValue is newly allocated. So remember to free it.
    void (* component_set_value)(MakerDialog *dlg, const gchar *key, GValue *value);	//!< Set a value to a property, so the corresponding UI component can show the value.
    gpointer (* dialog_construct)(MakerDialog *dlg);	//!< Construct the toolkit dialog. Called by maker_dialog_construct().
    gint (* dialog_run)(MakerDialog *dlg);		//!< Blocks in a recursive main loop until the dialog either emits the "response" signal, or is destroyed.
    void (* dialog_show)(MakerDialog *dlg);		//!< Show the dialog UI.
    void (* dialog_hide)(MakerDialog *dlg);		//!< Hide the dialog UI.
    void (* dialog_destroy)(MakerDialog *dlg);		//!< Destroy (free) the dialog UI.
} MakerDialogToolkitHandler;

/**
 * A dummy toolkit handler.
 *
 * This is the default if no handler is specified.
 */
const MakerDialogToolkitHandler makerDialogHandlerNone;

struct _MakerDialog{
    gchar *title;				//!< Title of the dialog, which will be shown in title bar.
    MakerDialogPropertyTable *propertyTable;	//!< Hash table that stores property context.
    MakerDialogToolkitHandler *handler;		//!< The handler that define how the UI to be handled.
    guint buttonSpecCount;			//!< Number of button specs.
    MakerDialogButtonSpec *buttonSpecs;		//!< Button specs.
    MakerDialogDimension maxSizeInPixel;	//!< The maximum size in pixel. Default is (-1, -1).
    MakerDialogDimension maxSizeInChar;		//!< The maximum size in characters. Default is (-1, -1).
    MakerDialogAlignment labelAlignment;	//!< The alignment for label. Default is (0, 0);
    MakerDialogAlignment componentAlignment;	//!< The alignment for UI component. Default is (0, 0);
};

/*=== Start Function Definition  ===*/
/**
 * Print debug message.
 *
 * @param level Level of the debug message, the higher the level, the more verbose it should be.
 * @param format Printf-like format string.
 * @param ... Parameter of format.
 */
void MAKER_DIALOG_DEBUG_MSG(gint level, const gchar *format, ...);

/**
 * Initialize a MakerDialog.
 *
 * Note this does not create the actual toolkit dialog yet.
 * Use maker_dialog_construct() after this function to create the actual
 * toolkit dialog.
 *
 * @param title Title of the dialog. This string will be duplicated in MakerDialog.
 * @param buttonSpecCount Number of button specs.
 * @param buttonSpecs Button specs.
 * @return A newly allocate MakerDialog.
 *
 * @see maker_dialog_construct().
 */
MakerDialog *maker_dialog_init(const gchar *title,
	guint buttonSpecCount, MakerDialogButtonSpec *buttonSpecs);

/**
 * Add a property context to the maker dialog.
 *
 * This function is actually a convenient wrap of maker_dialog_property_table_insert().
 *
 * If you only have property spec, use following example code:
 * @code
 *    maker_dialog_add_property(dlg,
 *       maker_dialog_property_context_new( propertySpec, NULL, NULL));
 * @endcode
 * Where @code propertySpec @endcode is the property spec.
 *
 * @param dlg A MakerDialog.
 * @param ctx The property context to be added.
 * @param initValue String represented initial value. Non-NULL value overrides the default value defined in spec.
 *
 * @see maker_dialog_property_table_insert().
 */
void maker_dialog_add_property(MakerDialog *dlg, MakerDialogPropertyContext *ctx, const gchar *initValue);

/**
 * Set toolkit of choice.
 *
 * So maker_dialog generates UI components accordingly.
 * @param dlg A MakerDialog.
 * @param handler The handler that define how the UI to be handled.
 * @return TRUE if succeed; FALSE otherwise.
 *
 * Set toolkit of choice. So maker_dialog generates UI components accordingly.
 */
void maker_dialog_set_toolkit_handler(MakerDialog *dlg, MakerDialogToolkitHandler *handler);

/**
 * Construct a toolkit dialog object (such as GtkDialog or QDialog) for later use.
 *
 * @param dlg A MakerDialog.
 *
 * @see maker_dialog_init().
 */
void maker_dialog_construct(MakerDialog *dlg);

/**
 * Blocks in a recursive main loop until the dialog either emits the "response" signal, or is destroyed.
 *
 * This function aspires gtk_dialog_run().
 * Show the dialog by using dialog_show().
 * @param dlg A MakerDialog.
 * @return The respond ID.
 */
gint maker_dialog_run(MakerDialog *dlg);

/**
 * Show the dialog by using dialog_show().
 *
 * @param dlg A MakerDialog.
 */
void maker_dialog_show(MakerDialog *dlg);

/**
 * Hide the dialog.
 *
 * @param dlg A MakerDialog.
 */
void maker_dialog_hide(MakerDialog *dlg);

/**
 * Destroy and free the dialog.
 *
 * Title in dialog will also be freed.
 * @param dlg A MakerDialog.
 */
void maker_dialog_destroy(MakerDialog *dlg);

/**
 * Get the value of a property.
 *
 * The returned value is still useful for property context, so DO NOT free it.
 *
 * @param dlg A MakerDialog.
 * @param key A property key.
 * @return Value of the property; or NULL if no such property.
 */
GValue *maker_dialog_get_value(MakerDialog *dlg, const gchar *key);


/**
 * Apply the value of a property.
 *
 * Apply the value in UI component to property.
 * If setFunc() is defined in property context, then it will be called as well,
 * but validateFunc() will not be called in this function. i.e. No validating.
 *
 * The difference between maker_dialog_apply_value() and maker_dialog_set_value() is:
 * maker_dialog_apply_value() copy the value in UI component to property value;
 * while maker_dialog_set_value() copy the parameter value to property value and UI
 * component.
 *
 * @param dlg A MakerDialog.
 * @param key A property key.
 * @return Value of the property; or NULL if no such property.
 * @see maker_dialog_set_value()
 * @see maker_dialog_validate_and_apply_value()
 * @see maker_dialog_validate_and_set_value()
 */
void maker_dialog_apply_value(MakerDialog *dlg, const gchar *key);

/**
 * Set the value of a property.
 *
 * Value in UI component will be updated.
 * If setFunc() is defined in property context, then it will be called as well,
 * but validateFunc() will not be called in this function. i.e. No validating.
 *
 * @param dlg A MakerDialog.
 * @param key A property key.
 * @param value Value to be set.
 * @see maker_dialog_apply_value()
 * @see maker_dialog_validate_and_apply_value()
 * @see maker_dialog_validate_and_set_value()
 */
void maker_dialog_set_value(MakerDialog *dlg, const gchar *key, GValue *value);

/**
 * Validate and apply the value of a property.
 *
 * Apply the value in UI component to property.
 * First validateFunc() is called to validate the value,
 * if pass, then set the value and call setFunc() if defined.
 * Note this function also returns FALSE if validateFunc() is undefined.
 *
 *
 * @param dlg A MakerDialog.
 * @param key A property key.
 * @return Value of the property; or NULL if no such property.
 * @see maker_dialog_apply_value()
 * @see maker_dialog_set_value()
 * @see maker_dialog_validate_and_set_value()
 */
gboolean maker_dialog_validate_and_apply_value(MakerDialog *dlg, const gchar *key);

/**
 * Validate and set the value of a property.
 *
 * Value in UI component will be updated.
 * First validateFunc() is called to validate the value,
 * if pass, then set the value and call setFunc() if defined.
 * Note this function also returns FALSE if validateFunc() is undefined.
 *
 * @param dlg A MakerDialog.
 * @param key A property key.
 * @param value Value to be set.
 * @return TRUE if succeed; FALSE if validateFunc() is not defined or the value is not valid.
 * @see maker_dialog_apply_value()
 * @see maker_dialog_set_value()
 * @see maker_dialog_validate_and_apply_value()
 */
gboolean maker_dialog_validate_and_set_value(MakerDialog *dlg, const gchar *key, GValue *value);

/**
 * String to boolean.
 *
 * It returns FALSE if:
 *    -# string is NULL or have 0 length.
 *    -# string starts with 'F', 'f', 'N' or 'n'.
 *    -# string can be converted to a numeric 0.
 *
 * Everything else is TRUE.
 *
 * @param str A string.
 * @return Boolean value represented by the string.
 */
gboolean maker_dialog_atob(const gchar *str);

/**
 * Return the index of a string in a string list.
 *
 * @param str String to be found.
 * @param strlist List of string. Must be ended with NULL.
 * @param max_find Max strings to find. -1 for find until NULL.
 * @return Index of the string if str is in strlist before max_find; or -1 if on such string.
 */
gint maker_dialog_find_string(const gchar *str, const gchar **strlist, gint max_find);
/*=== End Function Definition  ===*/


#endif /* MAKER_DIALOG_H_ */

