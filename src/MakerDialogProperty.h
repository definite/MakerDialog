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
} MAKER_DIALOG_PROPERTY_FLAG;

/**
 * Data structure for storing property flags.
 */
typedef guint MakerDialogPropertyFlags;

typedef struct _MakerDialogPropertySpec MakerDialogPropertySpec;

typedef struct _MakerDialogPropertyContext MakerDialogPropertyContext;

/**
 * Prototype of validate callback function, which is invoked when a property value
 * is changed via UI.
 *
 * @param ctx A context of a MakerDialog context.
 * @param value Value to be set of the property.
 * @return TRUE for pass; FALSE for fail.
 * @see MakerDialogSetCallbackFunc(), ::MakerDialogPropertySpec .
 */
typedef gboolean (* MakerDialogCheckCallbackFunc)(MakerDialogPropertySpec *spec, GValue *value);

/**
 * Prototype of set callback function, which is invoked when a property value
 * is changed via UI.
 *
 * @param ctx A context of a MakerDialog context.
 * @param value Value to be set of the property.
 *
 * @see MakerDialogCheckCallbackFunc(), ::MakerDialogPropertySpec .
 */
typedef void (* MakerDialogSetCallbackFunc)(MakerDialogPropertyContext *ctx, GValue *value);

/**
 * A MakerDialogPropertySpec determine how UI components be generated.
 *
 * As the name suggests, this data structure stores the specification of a
 * configuration property.
 */
struct _MakerDialogPropertySpec{
    const gchar *key;		//!< String that identify the property.
    GType valueType;		//!< Data type of the property value.
    MakerDialogPropertyFlags 	propertyFlags; //!< Flags for a configuration property.
    gchar *defaultValue;	//!< Default value represent in string. Can be NULL.
    const gchar **validValues;	//!< Valid values of this dialog. Can be NULL.
    gdouble min;		//!< Minimum value of a number. Irrelevant to other data type.
    gdouble max;		//!< Maximum value of a number. Irrelevant to other data type.
    gdouble step;		//!< Increment added or subtracted by spinning the widget.
    gint   decimalDigits;	//!< Number of digits after decimal digits.

    gchar *pageName;		//!< Page that this property belongs to. It will appear as a tab label in GUI. Can be NULL.
    gchar *label;		//!< Label of this property.
    gchar *translationContext;	//!< Translation message context as for dgettext().
    const gchar *tooltip;	//!< Tooltip to be shown when mouse hover over the property.

    gpointer extraData;		//!< For storing custom data structure.
};

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
    MakerDialogCheckCallbackFunc validateFunc;	//!< Function to be called for validating the value.
    MakerDialogSetCallbackFunc setFunc;		//!< Function to be called when property value is set in UI.
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
 */
MakerDialogPropertySpec *maker_dialog_property_spec_new(const gchar *key, GType valueType);

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
 * @param spec Property specification.
 * @param obj A referencing object for set callback function. Can be NULL.
 * @return A newly allocated MakerDialogPropertyContext.
 */
MakerDialogPropertyContext *maker_dialog_property_context_new(MakerDialogPropertySpec *spec, gpointer obj);

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

