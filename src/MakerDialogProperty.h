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
 * @file MakerDialogProperty.h
 * Property Specification and Manipulation.
 *
 * In MakerDialog, a property is a configuration option which associate with a
 * value. According to property specification, MakerDialog generates
 * corresponding UI component to manipulate the property value.
 */
#ifndef MAKER_DIALOG_PROPERTY_H_
#define MAKER_DIALOG_PROPERTY_H_
#include <glib.h>
#include <glib-object.h>

/**
 * An end-of-property-spec-list definition.
 *
 * This defines an empty property spec, which can be put in property spec arrays
 * as a terminator for following code:
 * @code
 * MakerDialogPropertySpec specList[]={
 *     {....},
 *     {....}.
 *     ......,
 *    MAKER_DIALOG_PROPERTY_SPEC_ENDER
 * };
 *
 * for(i=0; specList[i]->validType!=MKDG_TYPE_INVALID;i++){
 * ...
 * }
 * @endcode
 */
#define MAKER_DIALOG_PROPERTY_SPEC_ENDER {\
    NULL, MKDG_TYPE_INVALID, 0,\
    NULL, NULL, NULL, NULL, 0.0, 0.0, 0.0, 0,\
    NULL, NULL, NULL, NULL,\
    NULL}\

/**
 * Enumeration of MakerDialog flags.
 *
 * Flags for a configuration property. These flags, along with the property
 * type, determine how the UI is represented.
 */
typedef enum {
    MAKER_DIALOG_PROPERTY_FLAG_CAN_FREE    		=0x1, //!< The property spec can be freed. This flag is automatically set.
    MAKER_DIALOG_PROPERTY_FLAG_INVISIBLE   		=0x2, //!< The property should not be appeared in UI.
    MAKER_DIALOG_PROPERTY_FLAG_INSENSITIVE		=0x4, //!< The property should be insensitive. e.g. Gray-out in UI.
    MAKER_DIALOG_PROPERTY_FLAG_FIXED_SET 		=0x8, //!< The property choose only among predefined valid values.
    MAKER_DIALOG_PROPERTY_FLAG_PREFER_RADIO_BUTTONS 	=0x10, //!< Use radio buttons if possible. Need to set ::MAKER_DIALOG_PROPERTY_FLAG_FIXED_SET as well.
    MAKER_DIALOG_PROPERTY_FLAG_HAS_TRANSLATION		=0x20, //!< The values of a property is associated.
} MAKER_DIALOG_PROPERTY_FLAG;

/**
 * Data structure for storing property flags.
 */
typedef guint MakerDialogPropertyFlags;


/**
 * A MakerDialogPropertySpec determine how UI components be generated.
 *
 * As the name suggests, this data structure stores the specification of a
 * configuration property.
 */
typedef struct _MakerDialogPropertySpec{
    const gchar *key;			//!< String that identify the property.
    GType valueType;			//!< Data type of the property value.
    MakerDialogPropertyFlags flags; 	//!< Flags for a configuration property.
    const gchar *defaultValue;		//!< Default value represent in string. Can be \c NULL.
    const gchar **validValues;		//!< Valid values represent in strings. Can be \c NULL.
    /**
     * Option for parsing \a defaultValue and \a validValues.
     * For example "8" can be passed to integer property as base. \c NULL for using default (intuitive) parse.
     */
    const gchar *parseOption;
    const gchar *toStringFormat;	 //!< printf()-like format string used in to_string functions(). \c NULL for using default  (intuitive) format.

    gdouble min;			//!< Minimum value of a number. Irrelevant to other data type.
    gdouble max;			//!< Maximum value of a number. Or max characters of an input entry.
    gdouble step;			//!< Increment added or subtracted by spinning the widget.
    gint   decimalDigits;		//!< Number of digits after decimal digits.

    const gchar *pageName;		//!< Page that this property belongs to. It will appear as a tab label in GUI. Can be NULL.
    const gchar *label;			//!< Label of this property.
    const gchar *translationContext;	//!< Translation message context as for dgettext().
    const gchar *tooltip;		//!< Tooltip to be shown when mouse hover over the property.

    gpointer userData;			//!< For storing custom data structure.
} MakerDialogPropertySpec;

/**
 * A MakerDialogPropertyContext is a property context which associates property specification,
 * a value, and a referencing object.
 */
typedef struct _MakerDialogPropertyContext MakerDialogPropertyContext;

/**
 * Prototype of callback function for validating value.
 *
 * This callback function will be called when value validation is required.
 * TRUE should be returned is value is valid, FALSE otherwise.
 *
 * @param ctx A context of a MakerDialog context.
 * @param value Value to be set of the property.
 * @return TRUE for pass; FALSE for fail.
 * @see MakerDialogApplyCallbackFunc(), ::MakerDialogPropertySpec .
 */
typedef gboolean (* MakerDialogValidateCallbackFunc)(MakerDialogPropertySpec *spec, GValue *value);

/**
 * Prototype of callback function for applying value.
 *
 * This callback function will be called when applying the current value of
 * property to the system.
 *
 * @param ctx A context of a MakerDialog context.
 * @param value Value to be set of the property.
 *
 * @see MakerDialogValidateCallbackFunc(), ::MakerDialogPropertySpec .
 */
typedef void (* MakerDialogApplyCallbackFunc)(MakerDialogPropertyContext *ctx, GValue *value);

/**
 * Property context flags.
 */
typedef enum{
    MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_HAS_VALUE	=0x1, //!< Whether the value is set.
    MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNSAVED		=0x2, //!< The value is modified but unsaved.
    MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED	=0x4, //!< The value has not been applied. i.e. value has not passed to property context applyFunc().
    MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_VALUE_CHANGED	=0x8, //!< Value changed
} MakerDialogPropertyContextFlag;

/**
 * Data structure that holds property context flags.
 */
typedef guint MakerDialogPropertyContextFlags;

/**
 * A MakerDialogPropertyContext is a property context which associates property specification,
 * a value, and a referencing object.
 *
 * When a value is changed though UI, validateFunc() will be invoked to validate whether the values is acceptable.
 * If it passes, then the value will be set and the referencing object can be
 * tuned by calling the setFunc().
 */
struct _MakerDialogPropertyContext{
    MakerDialogPropertySpec 	*spec;		//!< Corresponding property spec.
    GValue 			value;		//!< Current value of the property.
    gint			valueIndex;	//!< Index of the value in validValues. -1 if value is not in validValues, or validValues does not exist.
    gpointer 			userData;	//!< For storing user data
    MakerDialogValidateCallbackFunc 	validateFunc;	//!< Function to be called for value validation.
    MakerDialogApplyCallbackFunc 	applyFunc;	//!< Function to be called for applying value.
    MakerDialogPropertyContextFlags	flags;	//!< Property context flags.
    /// @cond
    MakerDialog				*mDialog; //!< "Parent" MakerDialog.
    /// @endcond
};

/**
 * A hash table that use property key as key, a and MakerDialogPropertyContext
 * as value.
 *
 * It is essentially a GHashTable, so g_hash_table functions can be use with
 * it.
 */
typedef GHashTable MakerDialogPropertyTable;

/*=== Start Function Definition  ===*/
/**
 * New a MakerDialogPropertySpec.
 *
 * New a MakerDialogPropertySpec.
 * Note that the key is not duplicated during the construction,
 * nor it will be freed by maker_dialog_property_spec_free().
 *
 * @param key String that identify the property.
 * @param valueType Data type of the property value.
 * @return A newly allocated MakerDialogPropertyContext.
 * @see maker_dialog_property_spec_new_full()
 */
MakerDialogPropertySpec *maker_dialog_property_spec_new(const gchar *key, GType valueType);


/**
 * New a MakerDialogPropertySpec with all information.
 *
 * New a MakerDialogPropertySpec.
 * Note that the key is not duplicated during the construction,
 * nor it will be freed by maker_dialog_property_spec_free().
 *
 * @param key			String that identify the property.
 * @param valueType		Data type of the property value.
 * @param propertyFlags		Flags for a configuration property.
 * @param defaultValue		Default value represent in string.
 * @param validValues 		Valid values represent in strings.
 * @param parseOption   	Option for parsing \a defaultValue and \a validValues.
 * @param toStringFormat	printf()-like format string used in to_string functions(). \c NULL for using default  (intuitive) format.
 * @param min 			Minimum value of a number. Irrelevant to other data type.
 * @param max			Maximum value of a number. Irrelevant to other data type.
 * @param step			Increment added or subtracted by spinning the widget.
 * @param decimalDigits		Number of digits after decimal digits.
 * @param pageName		Page that this property belongs to. It will appear as a tab label in GUI. Can be NULL.
 * @param label			Label of this property.
 * @param translationContext	Translation message context as for dgettext().
 * @param tooltip		Tooltip to be shown when mouse hover over the property.
 * @param userData		For storing custom data structure.
 * @return A newly allocated MakerDialogPropertyContext.
 * @see maker_dialog_property_spec_new()
 */
MakerDialogPropertySpec *maker_dialog_property_spec_new_full(const gchar *key, GType valueType,
	const gchar *defaultValue, const gchar **validValues,
	const gchar *parseOption, const char *toStringFormat,
	gdouble min, gdouble max, gdouble step, gint decimalDigits,
	MakerDialogPropertyFlags propertyFlags,
	const gchar *pageName, const gchar *label, const gchar *translationContext,
	const gchar *tooltip, gpointer userData);

/**
 * Free a MakerDialogPropertySpec.
 *
 * Free a MakerDialogPropertySpec.
 * Note that this function only free itself, but not strings associated with it.
 *
 * @param spec Property specification.
 */
void maker_dialog_property_spec_free(MakerDialogPropertySpec *spec);

/**
 * New a MakerDialogPropertyContext.
 *
 * New a MakerDialogPropertyContext, according to the initValue.
 *
 *
 * @param spec Property specification.
 * @param obj A referencing object for set callback function. Can be NULL.
 * @return A newly allocated MakerDialogPropertyContext.
 * @see maker_dialog_property_context_new_full()
 */
MakerDialogPropertyContext *maker_dialog_property_context_new(MakerDialogPropertySpec *spec, gpointer obj);

/**
 * New a MakerDialogPropertyContext with callback functions.
 *
 * New a MakerDialogPropertyContext, according to the initValue
 * and set and validate callback functions.
 *
 * @param spec Property specification.
 * @param userData	For storing custom data structure.
 * @param validateFunc Callback function call for value validation.
 * @param applyFunc Callback function for applying value.
 * @return A newly allocated MakerDialogPropertyContext.
 * @see maker_dialog_property_context_new()
 */
MakerDialogPropertyContext *maker_dialog_property_context_new_full(MakerDialogPropertySpec *spec,
	gpointer userData,
	MakerDialogValidateCallbackFunc	validateFunc,
	MakerDialogApplyCallbackFunc applyFunc);

/**
 * Free a MakerDialogPropertyContext.
 *
 * Free a MakerDialog property context.
 * Note that the value under the property context will be unset by g_value_unset().
 *
 * @param ctx A MakerDialog property context.
 */
void maker_dialog_property_context_free(MakerDialogPropertyContext *ctx);

/**
 * Whether the property value is default value.
 *
 * Whether the property value is default value.
 * @param ctx A MakerDialog property context.
 * @return TRUE if the property value is default value; FALSE otherwise.
 * @see maker_dialog_set_value()
 * @see maker_dialog_property_get_default_string()
 * @see maker_dialog_property_get_default()
 * @see maker_dialog_property_set_default()
 * @see maker_dialog_property_set_value_fast()
 */
gboolean maker_dialog_property_is_default(MakerDialogPropertyContext *ctx);

/**
 * Get the "true" default value of a property.
 *
 * This function does not merely return \a defaultValue in \a spec.
 * It also checks whether it is in \c validValues, and flag ::MAKER_DIALOG_PROPERTY_FLAG_FIXED_SET.
 *
 * Specifically, this function returns:
 * # \a defaultValue, if:
 *   # \a defaultValue is in \a validValues  or ::MAKER_DIALOG_PROPERTY_FLAG_FIXED_SET is not set.
 *   # or \a validValues does not exist.
 * # First value in \a validValues, if:
 *   # \a defaultValue is not in \a validValues and ::MAKER_DIALOG_PROPERTY_FLAG_FIXED_SET is set.
 *   # \a defaultValue does not exist.
 * # \c NULL if none of above matches.
 * @param spec 	A MakerDailog property spec.
 * @retval defaultValue if it is valid.
 * @retval validValues[0] if \a defaltValue is not valid, but validValues exists.
 * @retval NULL if neither defaultValue is valid, nor validValues exists.
 * @see maker_dialog_set_value()
 * @see maker_dialog_property_is_default()
 * @see maker_dialog_property_get_default()
 * @see maker_dialog_property_set_default()
 * @see maker_dialog_property_set_value_fast()
 */
const gchar *maker_dialog_property_get_default_string(MakerDialogPropertySpec *spec);

/**
 * Return the "true" default value as a GValue.
 *
 * This function is similar to maker_dialog_property_get_default_string(),
 * except this function returns a GValue.
 *
 * Free the returned GValue after used.
 *
 * @param spec A MakerDailog property spec.
 * @return A newly allocated GValue which stores the default value.
 * @see maker_dialog_set_value()
 * @see maker_dialog_property_is_default()
 * @see maker_dialog_property_get_default_string()
 * @see maker_dialog_property_set_default()
 * @see maker_dialog_property_set_value_fast()
 */
GValue *maker_dialog_property_get_default(MakerDialogPropertySpec *spec);

/**
 * Set a property to default value.
 *
 * Set a property to default value.
 *
 * @param ctx A MakerDailog property context.
 * @see maker_dialog_set_value()
 * @see maker_dialog_property_is_default()
 * @see maker_dialog_property_get_default_string()
 * @see maker_dialog_property_get_default()
 * @see maker_dialog_property_set_value_fast()
 */
void maker_dialog_property_set_default(MakerDialogPropertyContext *ctx);

/**
 * Set value to property without validation and UI widget update.
 *
 * This function merely copies \a value to property context and updates property context flags.
 * Parameter \a valueIndexCtl control the behaviors that how \a valueIndex should be updated:
 *  - if valueIndexCtl ==-3 or  \a validValues does not exists, the \a valueIndex will not be updated.
 *  - if valueIndexCtl ==-2, then \a valueIndex is updated by finding the value in \a validValues.
 *  - if valueIndexCtl >=-1, then \a valueIndex is set as the \a valueIndexCtl.
 *
 * @param ctx 		A MakerDialog property context.
 * @param valueIndexCtl Number which indicates how should \a valueIndex be updated.
 * @param value 	Value to be check.
 * @see maker_dialog_set_value()
 * @see maker_dialog_property_is_default()
 * @see maker_dialog_property_get_default_string()
 * @see maker_dialog_property_get_default()
 * @see maker_dialog_property_set_default()
 */
void maker_dialog_property_set_value_fast(MakerDialogPropertyContext *ctx, GValue *value, gint valueIndexCtl);

/**
 * New a maker dialog property table.
 *
 * @return A newly allocated MakerDialogPropertyTable.
 */
MakerDialogPropertyTable* maker_dialog_property_table_new();

/**
 * Insert the property context to the table.
 *
 * Key of property context is from ctx-&gt;spec-&gt;key.
 * This key is served as hash search key as well.
 *
 * @param hTable A MakerDialogPropertyTable.
 * @param ctx A MakerDialog property context.
 */
void maker_dialog_property_table_insert(MakerDialogPropertyTable *hTable, const MakerDialogPropertyContext *ctx);

/**
 * Lookup the property context by key.
 *
 * Note that returned property context should not be freed at the point,
 * because the hash table is still referencing it.

 * @param hTable A MakerDialogPropertyTable.
 * @param key Key of property context.
 * @return Corresponding property context; or %NULL if no such property context.
 */
MakerDialogPropertyContext *maker_dialog_property_table_lookup(MakerDialogPropertyTable *hTable, const gchar *key);

/**
 * Lookup the property value by key.
 *
 * Note that returned property vlue should not be freed at the point,
 * because the hash table and property context are still referencing it.
 *
 * @param hTable A MakerDialogPropertyTable.
 * @param key Key of property context.
 * @return Corresponding property context; or %NULL if no such property context.
 */
GValue *maker_dialog_property_table_lookup_value(MakerDialogPropertyTable *hTable, const gchar *key);


/**
 * Destroys all keys and values in the GHashTable and decrements its reference count by 1.
 *
 * @param hTable A MakerDialogPropertyTable.
 */
void maker_dialog_property_table_destroy (MakerDialogPropertyTable *hTable);

/**
 * Call callback for each property.
 *
 * Calls the given function for each property.
 * The function is passed the key and value of each pair, and the given user_data parameter.
 * The hash table may not be modified while iterating over it (you can't add/remove items).
 *
 * @param mDialog 	A MakerDialog.
 * @param func 		The callback function to be called for each key/value pair.
 * @param userData 	User data to pass to the callback function.
 */
void maker_dialog_foreach_property(MakerDialog* mDialog, GHFunc func, gpointer userData);

/**
 * Prototype of callback function for foreach property function.
 *
 * The callback function should implement the actions for each property.
 * It will be called by maker_dialog_page_foreach_property() and maker_dialog_pages_foreach_property().
 *
 * @param mDialog 	A MakerDialog.
 * @param ctx  		The property context.
 * @param userData 	User data to be passed into the callback.
 * @see maker_dialog_pages_foreach_property().
 */
typedef void (* MakerDialogPropertyCallbackFunc)(MakerDialog *mDialog, MakerDialogPropertyContext *ctx, gpointer userData);

/**
 * Call callback for each property in a page.
 *
 * Calls the given function for each property in a page.
 *
 * @param mDialog 	A MakerDialog.
 * @param pageName 	The page to be working on.
 * @param func 		The callback function to be called.
 * @param userData 	User data to pass to the callback function.
 */
void maker_dialog_page_foreach_property(MakerDialog* mDialog, const gchar *pageName, MakerDialogPropertyCallbackFunc  func, gpointer userData);

/**
 * Call callback for each property in certain pages.
 *
 * Calls the given function for each property in certain pages.
 *
 * @param mDialog 	A MakerDialog.
 * @param pageNames 	Page names to be included in the execution. \c NULL for all keys, regardless the pages.
 * @param func 		The callback function to be called.
 * @param userData 	User data to pass to the callback function.
 */
void maker_dialog_pages_foreach_property(MakerDialog* mDialog, const gchar **pageNames, MakerDialogPropertyCallbackFunc  func, gpointer userData);

/*=== End Function Definition  ===*/


#endif /* MAKER_DIALOG_PROPERTY_H_ */

