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
 * @file MakerDialogType.h
 * Types used in MakerDialog.
 *
 * MakerDialogType (abbreivate as MKDGType) are derived from GType.
 * In fact most of them are essentially equivalent to their GType
 * counterparts, however,
 * there are some extra types like MKDG_TYPE_COLOR,
 * which is useful for dialogs but not included in GType.
 */
#ifndef MAKER_DIALOG_TYPES_H_
#define MAKER_DIALOG_TYPES_H_
#include <glib.h>
#include <glib-object.h>

/**
 * Type of MakerDialog color.
 */
typedef guint32 MkdgColor;


/**
 * Return the GType of MakerDialog color.
 *
 * Return the GType of MakerDialog color.
 * @return GType of MakerDialog color.
 */
GType mkdg_color_get_type();

/**
 * MakerDialog "invalid" type.
 *
 * Equivalent to G_TYPE_INVALID.
 * Usually use as terminator.
 */
#define MKDG_TYPE_INVALID	G_TYPE_INVALID

/**
 * MakerDialog none type.
 *
 * Equivalent to G_TYPE_NONE.
 */
#define MKDG_TYPE_NONE		G_TYPE_NONE

/**
 * MakerDialog boolean type.
 *
 * Equivalent to G_TYPE_BOOLEAN.
 */
#define MKDG_TYPE_BOOLEAN	G_TYPE_BOOLEAN

/**
 * MakerDialog integer type.
 *
 * Equivalent to G_TYPE_INT.
 */
#define MKDG_TYPE_INT		G_TYPE_INT

/**
 * MakerDialog unsigned integer type.
 *
 * Equivalent to G_TYPE_UINT.
 */
#define MKDG_TYPE_UINT		G_TYPE_UINT

/**
 * MakerDialog long integer type.
 *
 * Equivalent to G_TYPE_LONG.
 */
#define MKDG_TYPE_LONG		G_TYPE_LONG

/**
 * MakerDialog unsigned long integer type.
 *
 * Equivalent to G_TYPE_ULONG.
 */
#define MKDG_TYPE_ULONG		G_TYPE_ULONG

/**
 * MakerDialog floating point number type.
 *
 * Equivalent to G_TYPE_FLOAT.
 */
#define MKDG_TYPE_FLOAT		G_TYPE_FLOAT

/**
 * MakerDialog double floating point number type.
 *
 * Equivalent to G_TYPE_DOUBLE.
 */
#define MKDG_TYPE_DOUBLE	G_TYPE_DOUBLE

/**
 * MakerDialog string type.
 *
 * Equivalent to G_TYPE_STRING.
 */
#define MKDG_TYPE_STRING	G_TYPE_STRING

/**
 * MakerDialog string type.
 *
 * It is based on G_INT_32.
 */
#define MKDG_TYPE_COLOR		mkdg_color_get_type()

/**
 * Free a GValue.
 *
 * This function unset the \a value and free the space \a value occupies.
 * @param value		A GValue.
 */
void maker_dialog_value_free(gpointer value);

/**
 * Whether a GValue holds number.
 *
 * Whether a GValue holds number.
 * @param value		A GValue.
 * @return TRUE if GValue holds number; FALSE otherwise.
 * @see maker_dialog_value_get_double().
 */
gboolean maker_dialog_value_is_number(GValue *value);

/**
 * Get a double from a numeric GValue.
 *
 * This function gets a double from a numeric GValue.
 * It returns \c 0.0 if the GValue is non-numeric.
 * Use maker_dialog_value_is_number() to determine whether the returned \c 0.0
 * is because of non-numeric GValue.
 *
 * @param value		A GValue.
 * @return TRUE if GValue holds number; FALSE otherwise.
 * @see maker_dialog_value_is_number().
 * @see maker_dialog_value_set_double().
 */
gdouble maker_dialog_value_get_double(GValue *value);

/**
 * Set a double to a numeric GValue.
 *
 * This function sets a double to a numeric GValue.
 *
 * @param value		A GValue.
 * @param number	Number to set.
 * @see maker_dialog_value_get_double()
 */
void maker_dialog_value_set_number(GValue *value, gdouble number);


/**
 * Set the content of GValue from a given string.
 *
 * This function sets the content of GValue from a given string.
 * Parameter \a parseOption provides additional control. Set it \c NULL for using default.
 *
 * Available Option:
 *  - Set base for \c MKDG_TYPE_INT, \c MKDG_TYPE_UINT, \c MKDG_TYPE_LONG, \c MKDG_TYPE_ULONG values:
 *    - Type is (gint *). Default is 10 (decimal).
 *  - Others ignore \a option.
 *
 * Note that \a value needs to be initialized.
 *
 * @param value 	A GValue.
 * @param str 		The string to be converted from.
 * @param parseOption	Additional control. Can be \c NULL.
 * @return The argument \a value.
 */
GValue *maker_dialog_value_from_string(GValue *value, const gchar *str, const gchar *parseOption);

/**
 * Output value of a GValue to string.
 *
 * This function outputs value of a GValue to string.
 * Parameter \a toStringFormat provides additional output control.
 * Its format is identical to the format string of printf().
 * Pass \c NULL for using default format for that type.
 *
 *
 * @param value 		A GValue.
 * @param toStringFormat	Custom printf()-like format string. Pass \c NULL for using default format for that type.
 * @return The argument \a value.
 */
gchar *maker_dialog_value_to_string(GValue *value, const gchar *toStringFormat);

/**
 * Return a normalized string-representation.
 *
 * A "normalized" string-representation can be feed into from_string() function
 * without extra argument. For example, "0x10" can be normalized to "16".
 *
 * After normalization, it is easy to compare value with string functions such as strcmp().
 * @param str	String to be normalized.
 * @param type	Type that \a str to be parsed as.
 * @return A newly allocated string which stores normalized result.
 */
gchar *maker_dialog_string_normalized(const gchar *str, GType type);

/**
 * Prototype of comparing function.
 *
 * @param value1 	The first value.
 * @param value2 	The second value.
 * @retval -1 if \a value1 \< \a value2.
 * @retval 0 if \a value1 = \a value2.
 * @retval 1 if \a value1 \> \a value2.
 */
typedef gint (* MakerDialogCompareFunc)(gpointer value1, gpointer value2);

/**
 * Compare value of two GValues.
 *
 * This function compares value of two GValues,
 * and returns:
 *  - -1 if \a value1 \< \a value2.
 *  - 0 if \a value1 = \a value2.
 *  - 1 if \a value1 \> \a value2.
 *
 * Currently, this function supports following types:
 *  - MKDG_TYPE_BOOLEAN: TRUE is greater than FALSE.
 *  - MKDG_TYPE_INT: Natural order.
 *  - MKDG_TYPE_UINT: Natural order.
 *  - MKDG_TYPE_DOUBLE: Natural order.
 *  - MKDG_TYPE_STRING: As in strcmp.
 *
 * Numeric values can be compared each other, but MKDG_TYPE_BOOLEAN does not count as
 * number. Nor can number compare with string.
 *
 * -3 is returned if the values cannot be compared.
 * -2 is returned if the either of value types is not supported.
 * Beware, this function uses values \<= -2 to returns error.
 *
 * @param value1 	The first value.
 * @param value2 	The second value.
 * @param compFunc	Comparison function. Can be \c NULL.
 * @retval -3  if the values cannot be compared.
 * @retval -2 if the type is not supported.
 * @retval -1 if \a value1 \< \a value2.
 * @retval 0 if \a value1 = \a value2.
 * @retval 1 if \a value1 \> \a value2.
 */
gint maker_dialog_value_compare(GValue *value1, GValue *value2, MakerDialogCompareFunc compFunc);
#endif /* MAKER_DIALOG_TYPES_H_ */
