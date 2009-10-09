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
 * Enumeration of MakerDialog flags.
 *
 * Flags for a configuration property. These flags, along with the property
 * type, determine how the UI is represented.
 *
 */
typedef enum {
    MAKER_DIALOG_PROPERTY_FLAG_CAN_FREE    =0x1, //!< The property spec can be freed.
    MAKER_DIALOG_PROPERTY_FLAG_INVISIBLE   =0x2, //!< The property should not be appeared in UI.
    MAKER_DIALOG_PROPERTY_FLAG_INSENSITIVE =0x4, //!< The property should be insensitive. e.g. Gray-out in UI.
    MAKER_DIALOG_PROPERTY_FLAG_INEDITABLE =0x8, //!< The property does not accept custom input, but only choose among predefined values.
    MAKER_DIALOG_PROPERTY_FLAG_HAS_TRANSLATION =0x10, //!< The values of a property is associated.
    MAKER_DIALOG_PROPERTY_FLAG_TRANSLATION_WITH_CONTEXT =0x20, //!< The translation is with context. This flags should be used with ::MAKER_DIALOG_PROPERTY_FLAG_HAS_TRANSLATION.
    MAKER_DIALOG_PROPERTY_FLAG_HAS_DEFAULT_VALUE =0x40,  //!< This property has default value. This flags is set automatically.
    MAKER_DIALOG_PROPERTY_FLAG_HAS_VALID_VALUES =0x40,  //!< This property has valid values. This flags is set automatically.
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
    const gchar *key;		//!< String that identify the property.
    GType valueType;		//!< Data type of the property value.
    MakerDialogPropertyFlags 	propertyFlags; //!< Flags for a configuration property.
    const gchar *defaultValue;	//!< Default value represent in string.
    const gchar **validValues;	//!< Valid values represent in strings.
    gdouble min;		//!< Minimum value of a number. Irrelevant to other data type.
    gdouble max;		//!< Maximum value of a number. Irrelevant to other data type.
    gdouble step;		//!< Increment added or subtracted by spinning the widget.
    gint   decimalDigits;	//!< Number of digits after decimal digits.

    const gchar *pageName;		//!< Page that this property belongs to. It will appear as a tab label in GUI. Can be NULL.
    const gchar *label;		//!< Label of this property.
    const gchar *translationContext;	//!< Translation message context as for dgettext().
    const gchar *tooltip;	//!< Tooltip to be shown when mouse hover over the property.

    gpointer extraData;		//!< For storing custom data structure.
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
 * A MakerDialogPropertyContext is a property context which associates property specification,
 * a value, and a referencing object.
 *
 * When a value is changed though UI, validateFunc() will be invoked to validate whether the values is acceptable.
 * If it passes, then the value will be set and the referencing object can be
 * tuned by calling the setFunc().
 */
struct _MakerDialogPropertyContext{
    MakerDialogPropertySpec *spec;		//!< Corresponding property spec.
    GValue value;				//!< Current value of the property.
    gpointer obj;				//!< An referencing object.
    MakerDialogValidateCallbackFunc validateFunc;	//!< Function to be called for value validation.
    MakerDialogApplyCallbackFunc applyFunc;		//!< Function to be called for applying value.

    /// @cond
    gboolean hasValue;			//!< Whether the value is set.
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
 * @param key		String that identify the property.
 * @param valueType	Data type of the property value.
 * @param propertyFlags	Flags for a configuration property.
 * @param defaultValue	Default value represent in string.
 * @param validValues 	Valid values represent in strings.
 * @param min 		Minimum value of a number. Irrelevant to other data type.
 * @param max		Maximum value of a number. Irrelevant to other data type.
 * @param step		Increment added or subtracted by spinning the widget.
 * @param decimalDigits	Number of digits after decimal digits.
 * @param pageName	Page that this property belongs to. It will appear as a tab label in GUI. Can be NULL.
 * @param label		Label of this property.
 * @param translationContext	Translation message context as for dgettext().
 * @param tooltip	Tooltip to be shown when mouse hover over the property.
 * @param extraData	For storing custom data structure.
 * @return A newly allocated MakerDialogPropertyContext.
 * @see maker_dialog_property_spec_new()
 */
MakerDialogPropertySpec *maker_dialog_property_spec_new_full(const gchar *key, GType valueType,
	const gchar *defaultValue, const gchar **validValues,
	gdouble min, gdouble max, gdouble step, gint decimalDigits,
	MakerDialogPropertyFlags propertyFlags,
	const gchar *pageName, const gchar *label, const gchar *translationContext,
	const gchar *tooltip, gpointer extraData);

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
 * Note that initValue stores the current setting, usually in from configure file;
 * while default value is used when initValue does not exist.
 *
 * @param spec Property specification.
 * @param initValue Initial value for the property.
 * @param obj A referencing object for set callback function. Can be NULL.
 * @return A newly allocated MakerDialogPropertyContext.
 * @see maker_dialog_property_context_new_full()
 */
MakerDialogPropertyContext *maker_dialog_property_context_new(MakerDialogPropertySpec *spec,
       GValue *initValue, gpointer obj);

/**
 * New a MakerDialogPropertyContext with callback functions.
 *
 * New a MakerDialogPropertyContext, according to the initValue
 * and set and validate callback functions.
 *
 * Note that initValue stores the current setting, usually in from configure file;
 * while default value is used when initValue does not exist.
 *
 * @param spec Property specification.
 * @param initValue Initial value for the property.
 * @param obj A referencing object for set callback function. Can be NULL.
 * @param validateFunc Callback function call for value validation.
 * @param applyFunc Callback function for applying value.
 * @return A newly allocated MakerDialogPropertyContext.
 * @see maker_dialog_property_context_new()
 */
MakerDialogPropertyContext *maker_dialog_property_context_new_full(MakerDialogPropertySpec *spec,
	GValue *initValue, gpointer obj,
	MakerDialogValidateCallbackFunc	validateFunc,
	MakerDialogApplyCallbackFunc applyFunc);

/**
 * Free a MakerDialogPropertyContext.
 *
 * Free a MakerDialog property context.
 * Note that the value under the property context will be unset by g_value_unset().
 *
 * @param ctx Property context.
 */
void maker_dialog_property_context_free(MakerDialogPropertyContext *ctx);

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
 * @param ctx Property context.
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

/*=== End Function Definition  ===*/


#endif /* MAKER_DIALOG_PROPERTY_H_ */

