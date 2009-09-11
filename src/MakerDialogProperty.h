/*
 * Copyright © 2009  Red Hat, Inc. All rights reserved.
 * Copyright © 2009  Ding-Yi Chen <dchen at redhat.com>
 *
 * This file is part of the ibus-chewing Project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef MAKER_DIALOG_H_
#define MAKER_DIALOG_H_
#include <glib.h>
#include <glib-object.h>

/**
 * SECTION: MakerDialogProperty:
 * @short_description: Property handling.
 * @stability: Stable
 *
 * In MakerDialog, a property is a configuration option which associate with a
 * value. According to property specification, MakerDialog generates
 * corresponding UI component to manipulate the property value.
 */

/**
 * MAKER_DIALOG_PROPERTY_FLAG:
 * @MAKER_DIALOG_PROPERTY_FLAG_AUTO_FREE: The property should not be auto
 * free.
 * @MAKER_DIALOG_PROPERTY_FLAG_INVISIBLE: The property should not be appeared in UI.
 * @MAKER_DIALOG_PROPERTY_FLAG_INSENSITIVE: The property should be insensitive. e.g. Gray-out in UI.
 * @MAKER_DIALOG_PROPERTY_FLAG_INEDITABLE: The property does not accept custom input,
 *   but only choose among predefined values.
 * @MAKER_DIALOG_PROPERTY_FLAG_HAS_TRANSLATION: The values of a property is associated.
 * @MAKER_DIALOG_PROPERTY_FLAG_TRANSLIATION_WITH_CONTEXT: The translation is with context. This flags
 * should be used with @HAS_TRANSLATION.
 *
 * Flags for a configuration property. These flags, along with the property
 * type, determine how the UI is represented.
 */
typedef enum {
    MAKER_DIALOG_PROPERTY_FLAG_AUTO_FREE   =0x1,
    MAKER_DIALOG_PROPERTY_FLAG_INVISIBLE   =0x2,
    MAKER_DIALOG_PROPERTY_FLAG_INSENSITIVE =0x4,
    MAKER_DIALOG_PROPERTY_FLAG_INEDITABLE =0x8,
    MAKER_DIALOG_PROPERTY_FLAG_HAS_TRANSLATION =0x10,
    MAKER_DIALOG_PROPERTY_FLAG_TRANSLATION_WITH_CONTEXT =0x20,
} MAKER_DIALOG_PROPERTY_FLAG;

/**
 * MakerDialogPropertyFlags:
 *
 * Data structure for storing property flags.
 */
typedef guint MakerDialogPropertyFlags;

typedef struct _MakerDialogPropertySpec MakerDialogPropertySpec;

typedef struct _MakerDialogPropertyContext MakerDialogPropertyContext;

/**
 * MakerDialogCheckCallbackFunc:
 * @ctx: A context of a MakerDialog context.
 * @value: Value to be set of the property.
 * @returns: TRUE for pass; FALSE for fail.
 *
 * Prototype of check callback function, which is invoked when a property value
 * is changed via UI.
 *
 * @see_also: MakerDialogSetCallbackFunc(), #MakerDialogPropertySpec .
 */
typedef void (* MakerDialogCheckCallbackFunc)(MakerDialogPropertySpec *spec, GValue *value);

/**
 * MakerDialogSetCallbackFunc:
 * @ctx: A context of a MakerDialog context.
 * @value: Value to be set of the property.
 *
 * Prototype of set callback function, which is invoked when a property value
 * is changed via UI.
 * @see_also: MakerDialogCheckCallbackFunc(), #MakerDialogPropertySpec .
 */
typedef void (* MakerDialogSetCallbackFunc)(MakerDialogPropertyContext *ctx, GValue *value);

/**
 * MakerDialogPropertySpec:
 * @key: String that identify the property.
 * @valueType: Data type of the property value.
 * @propertyFlags: Flags for a configuration property.
 * @defaultValue: Default value represent in string. Can be NULL.
 * @validValues: Valid values of this dialog. Can be NULL.
 * @min: Minimum value of a number. Irrelevant to other data type.
 * @max: Maximum value of a number. Irrelevant to other data type.
 * @pageName: Page that this property belongs to. It will appear as a tab label in GUI. Can be NULL.
 * @label: Label of this property.
 * @translationContext: Translation message context as for dgettext().
 * @tooltip: Tooltip to be shown when mouse hover over the property.
 * @setFunc: Function to be called when property value is set in UI.
 * @extraData: For storing custom data structure.
 *
 * A MakerDialogPropertySpec determine how UI components be generated.
 * As the name suggests, this data structure stores the specification of a
 * configuration property.
 *
 */
struct _MakerDialogPropertySpec{
    const gchar *key;
    GType valueType;
    MakerDialogPropertyFlags propertyFlags;
    gchar *defaultValue;
    const gchar **validValues;
    gdouble min;
    gdouble max;


    gchar *pageName;
    gchar *label;
    gchar *translationContext;
    const gchar *tooltip;

    MakerDialogCheckCallbackFunc checkFunc;
    MakerDialogSetCallbackFunc setFunc;

    gpointer extraData;
};

/**
 * MakerDialogPropertyContext:
 * @spec: Corresponding property spec.
 * @value: Current value of the property.
 * @userData: User data to be used in callback.
 *
 * A MakerDialogPropertyContext is a property context which associates property specification and value.
 *
 * When a value is changed though UI, checkFunc() will be invoked to check whether the values is acceptable.
 * If it is, then
 * correctness of the value.
 * The flag @valueCanFree is
 */
struct _MakerDialogPropertyContext{
    MakerDialogPropertySpec *spec;
    GValue value;
    gpointer userData;  //<! User data to be used in callback.
};

/**
 * MakerDialogPropertyTable:
 *
 * A hash table that use property key as key, a and MakerDialogPropertyContext
 * as value.
 *
 * It is essentially a GHashTable, so g_hash_table functions can be use with
 * it.
 */
typedef GHashTable MakerDialogPropertyTable;

/*=== Start Function Definition  ===*/
void MAKER_DIALOG_DEBUG_MSG(gint level, const char *format, ...);

/**
 * maker_dialog_atob:
 * @string: A string.
 * @returns: Boolean value represented by @string.
 *
 * String to boolean.
 * It returns FALSE if:
 *    1. @string is NULL or have 0 length.
 *    2. @string starts with 'F', 'f', 'N' or 'n'.
 *    3. @string can be converted to a numeric 0.
 *
 * Everything else is TRUE.
 */
gboolean maker_dialog_atob(const gchar *string);

/**
 * maker_dialog_property_spec_new:
 * @key: String that identify the property.
 * @valueType: Data type of the property value.
 * @returns: A newly allocated MakerDialogPropertyContext.
 *
 * New a MakerDialogPropertySpec.
 * Note that @key is not duplicated during the construction,
 * nor it will be freed by maker_dialog_property_spec_free().
 */
MakerDialogPropertySpec *maker_dialog_property_spec_new(const gchar *key, GType valueType);

/**
 * maker_dialog_property_spec_free:
 * @spec: Property specification.
 * @freeDeep: TRUE for free the strings under @spec as well (except key); FALSE
 * for free the @spec itself only.
 *
 * Free a MakerDialogPropertySpec.
 */
void maker_dialog_property_spec_free(MakerDialogPropertySpec *spec,gboolean freeDeep);

/**
 * maker_dialog_property_context_new:
 * @spec: Property specification.
 * @userData: User data to be passed to set and check callback functions. Can
 * be %NULL.
 * @returns: A newly allocated MakerDialogPropertyContext.
 *
 * New a MakerDialogPropertyContext.
 */
MakerDialogPropertyContext *maker_dialog_property_context_new(MakerDialogPropertySpec *spec, gpointer userData);

/**
 * maker_dialog_property_context_free:
 * @ctx: Property context.
 * @freeDeep: TRUE for free spec (which also freeDeep); FALSE
 * for free the @ctx itself only.
 *
 * Free a MakerDialogPropertyContext. The @value under the @ctx will be
 * unset by g_value_unset().
 */
void maker_dialog_property_context_free(MakerDialogPropertyContext *ctx, gboolean freeDeep);

/**
 * maker_dialog_property_table_new:
 * @returns: A newly allocated MakerDialogPropertyTable.
 *
 * New a maker dialog property table.
 */
MakerDialogPropertyTable* maker_dialog_property_table_new();

/**
 * maker_dialog_property_table_insert:
 * @hTable: A MakerDialogPropertyTable.
 * @ctx: Property context.
 *
 * Insert the property context to the table.
 * Key of property context is from ctx-&gt;spec-&gt;key.
 * This key is served as hash search key as well.
 */
void maker_dialog_property_table_insert(MakerDialogPropertyTable *hTable, const MakerDialogPropertyContext *ctx);

/**
 * maker_dialog_property_table_lookup:
 * @hTable: A MakerDialogPropertyTable.
 * @key: Key of property context.
 * @returns: Corresponding property context; or %NULL if no such property
 * context.
 *
 * Lookup the property context by key.
 * Note that returned property context should not be freed at the point,
 * because the hash table is still referencing it.
 */
MakerDialogPropertyContext *maker_dialog_property_table_lookup(MakerDialogPropertyTable *hTable, const gchar *key);

/**
 * maker_dialog_property_table_lookup:
 * @hTable: A MakerDialogPropertyTable.
 * @key: Key of property context.
 * @returns: Corresponding property context; or %NULL if no such property
 * context.
 *
 * Lookup the property value by key.
 * Note that returned property vlue should not be freed at the point,
 * because the hash table and property context are still referencing it.
 */
GValue *maker_dialog_property_table_lookup_value(MakerDialogPropertyTable *hTable, const gchar *key);

/*=== End Function Definition  ===*/


#endif /* MAKER_DIALOG_H_ */

