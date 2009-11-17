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
 *
 * @todo Implement functions that uses maxSizeInPixel.
 * @todo Implement functions that uses maxSizeInChar.
 * @todo Display associate image files.
 */
#ifndef MAKER_DIALOG_H_
#define MAKER_DIALOG_H_
#include <glib.h>
#include <glib-object.h>
#include "MakerDialogUtil.h"
#include "MakerDialogTypes.h"

typedef struct _MakerDialog MakerDialog;

#include "MakerDialogProperty.h"
#include "MakerDialogUi.h"
#include "MakerDialogConfig.h"
#include "MakerDialogConfigKeyFile.h"

/**
 * Inter-process communication module.
 * To be implement in the future.
 * @todo Implement Inter-process communication instance.
 */
typedef gpointer MakerDialogIpc;

/**
 * Data structure of a MakerDialog.
 *
 */
struct _MakerDialog{
    gchar *title;				//!< Title of the dialog, which will be shown in title bar.
    MakerDialogPropertyTable *propertyTable;	//!< Hash table that stores property context.
    guint buttonSpecCount;			//!< Number of button specs.
    MakerDialogButtonSpec *buttonSpecs;		//!< Button specs.
    MakerDialogDimension maxSizeInPixel;	//!< The maximum size in pixel. Default is (-1, -1).
    MakerDialogDimension maxSizeInChar;		//!< The maximum size in characters. Default is (-1, -1).
    MakerDialogAlignment labelAlignment;	//!< The alignment for label. Default is (0, 0.5);
    MakerDialogAlignment componentAlignment;	//!< The alignment for UI component. Default is (0, 0.5);
    /// @cond
    GNode *pageRoot;				//!< Store pages and keys under it. Depth 1 is root, point to NULL; Depth 2 stores pages; Depth 3 stores keys.
    MakerDialogUi *ui;				//!< UI instance.
    MakerDialogConfig *config;			//!< Configure instance.
    MakerDialogIpc *ipc;			//!< Inter-process communication instance.
    gpointer	userData;			//!< Custom user data.
    /// @endcond
};

/*=== Start Function Definition  ===*/
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
 *       maker_dialog_property_context_new( propertySpec, NULL));
 * @endcode
 * Where @code propertySpec @endcode is the property spec.
 *
 * @param mDialog A MakerDialog.
 * @param ctx The property context to be added.
 *
 * @see maker_dialog_property_table_insert().
 */
void maker_dialog_add_property(MakerDialog *mDialog, MakerDialogPropertyContext *ctx);

/**
 * Destroy the MakerDialog.
 * @param mDialog A MakerDialog.
 *
 * Free all associate memory.
 * Title in the MakerDialog will also be freed.
 */
void maker_dialog_destroy(MakerDialog *mDialog);

/**
 * Get the value of a property.
 *
 * This function either returns the value of a property,
 * or NULL if no such property or no value has been set.
 *
 * The returned value is still useful for property context, so DO NOT free it.
 *
 * @param mDialog A MakerDialog.
 * @param key A property key.
 * @return Value of the property; or NULL if no such property, or value have not been set.
 */
MkdgValue *maker_dialog_get_value(MakerDialog *mDialog, const gchar *key);

/**
 * Get the property context.
 *
 * The returned value is still useful for property context, so DO NOT free it.
 *
 * @param mDialog A MakerDialog.
 * @param key A property key.
 * @return Context of the property; or NULL if no such property.
 */
MakerDialogPropertyContext *maker_dialog_get_property_context(MakerDialog *mDialog, const gchar *key);

/**
 * Apply a property value by calling the apply callback function.
 *
 * This function applies the current property value by calling
 * the applyFunc() defined in property context.
 *
 * If validateFunc() is also defined, then the value will be checked with it,
 * if it does not pass, this function returns FALSE.
 *
 * If applyFunc() is not defined, this function returns FALSE as well.
 *
 * The difference between maker_dialog_apply_value(), maker_dialog_set_value(), and
 * maker_dialog_ui_update_value() are:
 *
 * - maker_dialog_apply_value() applies the property value to the system by calling the applyFunc();
 * - maker_dialog_set_value() copies argument value to the property value and UI widget value.
 * - maker_dialog_ui_update_value() copies UI component value to property value.
 *
 * @param mDialog A MakerDialog.
 * @param key A property key.
 * @return TRUE if succeed, FALSE if the property value does not pass validation, or applyFunc() does not exist.
 * @see maker_dialog_set_value()
 * @see maker_dialog_ui_update_value()
 */
gboolean maker_dialog_apply_value(MakerDialog *mDialog, const gchar *key);

/**
 * Set the value to the property and corresponding UI component.
 *
 * This function:
 * -# validates \a value, if validateFunc() exists,
 * -# if \a value is not valid, then it return FALSE.
 * -# otherwise copies the argument value to the property value
 * -# update UI widget if it exists.
 *
 * If \a value is NULL, then default value will be used.
 *
 *
 * @param mDialog A MakerDialog.
 * @param key A property key.
 * @param value Argument value to be set. \c NULL for using default.
 * @return TRUE if succeed, FALSE if the property value does not pass validation.
 * @see maker_dialog_apply_value()
 * @see maker_dialog_ui_update_value()
 * @see maker_dialog_property_set_default()
 * @see maker_dialog_property_set_value_fast()
 *
 */
gboolean maker_dialog_set_value(MakerDialog *mDialog, const gchar *key, MkdgValue *value);

/**
 * Find the page node by page name.
 *
 * Find the page node by page name.
 * @param mDialog A MakerDialog.
 * @param pageName Page name to be found.
 * @return GNode that contains the page name; NULL if no such node.
 */
GNode *maker_dialog_find_page_node(MakerDialog *mDialog, const gchar *pageName);


/**
 * Find the group node by page name and group name.
 *
 * Find the group node by page name and group name.
 * @param mDialog A MakerDialog.
 * @param pageName Matching page name.
 * @param groupName Matching group name.
 * @return GNode that contains the group name under given page name; NULL if no such node.
 */
GNode *maker_dialog_find_group_node(MakerDialog *mDialog, const gchar *pageName, const gchar *groupName);


/*=== End Function Definition  ===*/


#endif /* MAKER_DIALOG_H_ */

