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
 * @file Mkdg.h
 * Mkdg: Make dialog from given data type.
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
#include <glib/gstdio.h>
#include "MkdgUtil.h"
#include "MkdgTypes.h"

typedef struct _Mkdg Mkdg;

#include "MkdgProperty.h"
#include "MkdgPage.h"
#include "MkdgUi.h"
#include "MkdgConfig.h"
#include "MkdgConfigSet.h"
#include "MkdgConfigFile.h"
#include "MkdgConfigKeyFile.h"
#include "MkdgModule.h"
#include "MkdgSpecParser.h"

/**
 * Inter-process communication module.
 *
 * To be implement in the future.
 * @todo Implement Inter-process communication instance.
 */
typedef gpointer MkdgIpc;

/**
 * Enumeration of flags for MakerDialog itself.
 *
 * Enumeration of flags for MakerDialog itself.
 * @since 0.2
 */
typedef enum{
    /// @cond
    MAKER_DIALOG_FLAG_FREE_ALL=0x1,		// Free all content.
    /// @endcond
    MAKER_DIALOG_FLAG_CAN_GUI=0x10,		// The environment has GUI capability. This flags is automatically set.
} MAKER_DIALOG_FLAG;

/**
 * Data structure of a Mkdg.
 *
 * Data structure of a Mkdg.
 */
struct _Mkdg{
    gchar *title;				//!< Title of the dialog, which will be shown in title bar.
    MkdgPropertyTable *propertyTable;	//!< Hash table that stores property context.
    MkdgButtonSpec *buttonSpecs;		//!< Button specs.
    MkdgDimension maxSizeInPixel;	//!< The maximum size in pixel. Default is (-1, -1).
    MkdgDimension maxSizeInChar;		//!< The maximum size in characters. Default is (-1, -1).
    MkdgAlignment labelAlignment;	//!< The alignment for label. Default is (0, 0.5);
    MkdgAlignment componentAlignment;	//!< The alignment for UI component. Default is (0, 0.5);
    /// @cond
    MkdgFlags flags;			//!< Flags of makerDialog
    gint argc;
    gchar **argv;
    GNode *pageRoot;				//!< Store pages and keys under it. Depth 1 is root, point to NULL; Depth 2 stores pages; Depth 3 stores keys.
    MkdgUi *ui;				//!< UI instance.
    MkdgConfig *config;			//!< Configure instance.
    MkdgIpc ipc;				//!< Inter-process communication instance.
    /// @endcond
    gpointer	userData;			//!< Custom user data.
};

/**
 * New an empty Mkdg.
 *
 * This function allocates a new MakerDialog instance and
 * set the struct members to their default value.
 *
 * This function is called for MakerDialog spec parsers
 * and maker_dialog_init(), so no need to call this function directly.
 *
 * Either call maker_dialog_new_from_key_file() to read the MakerDialog spec from
 * a file; or use maker_dialog_init() to define the MakerDialog spec from
 * programs.
 * @return A newly allocated MakerDialog instance.
 *
 * @see maker_dialog_init(), maker_dialog_construct()
 * @since 0.3
 */
Mkdg *maker_dialog_new();

/**
 * Initialize a Mkdg.
 *
 * This function initialize a MakerDialog instance.
 * The title and buttons are specified in this function, however,
 * no property is specified at this point.
 * Run maker_dialog_add_property() to add property context and specification.
 *
 * Buttons, if specified, will be shown at bottom of dialog UI.
 * Use \c MAKER_DIALOG_RESPONSE_NIL as respond id at end of button
 * specification to signified the end of \a buttonSpec.
 *
 * @param title 	Title of the dialog. This string will be duplicated in Mkdg.
 * @param buttonSpecs 	Button specification. Can be \c NULL.
 * @return A newly allocated MakerDialog instance.
 *
 * @see maker_dialog_new(), maker_dialog_construct().
 */
Mkdg *maker_dialog_init(const gchar *title, MkdgButtonSpec *buttonSpecs);

/**
 * Set command line options.
 *
 * Some application would like to process command line options in their own
 * ways. UI toolkits like Gtk2 or Qt also offer further tweak via command line options.
 * @param argc Number of arguments, as in main(int argc, char **argv)
 * @param argv List of arguments, as in main(int argc, char **argv)
 * @since 0.3
 */
void maker_dialog_set_args(Mkdg *mDialog, gint argc, gchar **argv);

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
 * @param mDialog A Mkdg.
 * @param ctx The property context to be added.
 *
 * @see maker_dialog_property_table_insert().
 */
void maker_dialog_add_property(Mkdg *mDialog, MkdgPropertyContext *ctx);

/**
 * Destroy the Mkdg.
 * @param mDialog A Mkdg.
 *
 * Free all associate memory.
 * Title in the MakerDialog will also be freed.
 */
void maker_dialog_destroy(Mkdg *mDialog);

/**
 * Get the value of a property.
 *
 * This function either returns the value of a property,
 * or NULL if no such property or no value has been set.
 *
 * The returned value is still useful for property context, so DO NOT free it.
 *
 * @param mDialog A Mkdg.
 * @param key A property key.
 * @return Value of the property; or NULL if no such property, or value have not been set.
 */
MkdgValue *maker_dialog_get_value(Mkdg *mDialog, const gchar *key);

/**
 * Get the property context.
 *
 * The returned value is still useful for property context, so DO NOT free it.
 *
 * @param mDialog A Mkdg.
 * @param key A property key.
 * @return Context of the property; or NULL if no such property.
 */
MkdgPropertyContext *maker_dialog_get_property_context(Mkdg *mDialog, const gchar *key);

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
 * @param mDialog A Mkdg.
 * @param key A property key.
 * @return TRUE if succeed, FALSE if the property value does not pass validation, or applyFunc() does not exist.
 * @see maker_dialog_set_value()
 * @see maker_dialog_ui_update_value()
 */
gboolean maker_dialog_apply_value(Mkdg *mDialog, const gchar *key);

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
 * @param mDialog A Mkdg.
 * @param key A property key.
 * @param value Argument value to be set. \c NULL for using default.
 * @return TRUE if succeed, FALSE if the property value does not pass validation.
 * @see maker_dialog_apply_value()
 * @see maker_dialog_ui_update_value()
 * @see maker_dialog_property_set_default()
 * @see maker_dialog_property_set_value_fast()
 *
 */
gboolean maker_dialog_set_value(Mkdg *mDialog, const gchar *key, MkdgValue *value);


#endif /* MAKER_DIALOG_H_ */

