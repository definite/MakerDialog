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
 * @file MakerDialogUtil.h
 * Utility functions for MakerDialog.
 *
 * This file lists the utility functions for general purpose.
 */
#ifndef MAKER_DIALOG_UTIL_H_
#define MAKER_DIALOG_UTIL_H_
#include <glib.h>
#include <glib-object.h>

/**
 * Environment variable for MakerDialog debug.
 */
#define MAKER_DLALOG_VERBOSE_ENV "MAKER_DIALOG_VERBOSE"

/**
 * Print debug message.
 *
 * @param level Level of the debug message, the higher the level, the more verbose it should be.
 * @param format Printf-like format string.
 * @param ... Parameter of format.
 */
void MAKER_DIALOG_DEBUG_MSG(gint level, const gchar *format, ...);

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
 * Return the index of a string in a string list.
 * @param str String to be found.
 * @param strlist List of string. Must be ended with NULL.
 * @param max_find Max strings to find. -1 for find until NULL.
 * @return Index of the string if str is in strlist before max_find; or -1 if on such string.
 */
gint maker_dialog_find_string(const gchar *str, const gchar **strlist, gint max_find);

/**
 * Whether a set of flags contains all the specified flags.
 *
 * If \a flagSet  contains ALL the flags specified in \a specFlags,
 * then return \c TRUE, otherwise \c FALSE.
 * @param flagSet A set of flags to be checked.
 * @param specFlags Must-have flags.
 * @return TRUE if \a flagSet  contains ALL the flags specified in \a specFlags; FALSE otherwise.
 */
gboolean maker_dialog_has_all_flags(guint flagSet, guint specFlags);

/**
 * Whether a GValue holds number.
 *
 * Whether a GValue holds number.
 * @param value		A GValue.
 * @return TRUE if GValue holds number; FALSE otherwise.
 * @see maker_dialog_g_value_get_double().
 */
gboolean maker_dialog_g_value_is_number(GValue *value);

/**
 * Get a double from a numeric GValue.
 *
 * This function gets a double from a numeric GValue.
 * It returns \c 0.0 if the GValue is non-numeric.
 * Use maker_dialog_g_value_is_number() to determine whether the returned \c 0.0
 * is because of non-numeric GValue.
 *
 * @param value		A GValue.
 * @return TRUE if GValue holds number; FALSE otherwise.
 * @see maker_dialog_g_value_is_number().
 * @see maker_dialog_g_value_set_double().
 */
gdouble maker_dialog_g_value_get_double(GValue *value);

/**
 * Set a double to a numeric GValue.
 *
 * This function sets a double to a numeric GValue.
 *
 * @param value		A GValue.
 * @param number	Number to set.
 * @see maker_dialog_g_value_get_double()
 */
void maker_dialog_g_value_set_number(GValue *value, gdouble number);

/**
 * Free a GValue.
 *
 * This function unset the \a value and free the space \a value occupies.
 * @param value		A GValue.
 */
void maker_dialog_g_value_free(gpointer value);

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
 * Set the content of GValue from a given string.
 *
 * This function sets the content of GValue from a given string.
 * Parameter \a parseOption provides additional control. Set it \c NULL for using default.
 *
 * Available Option:
 *  - Set base for \c G_TYPE_INT, \c G_TYPE_UINT, \c G_TYPE_LONG, \c G_TYPE_ULONG values:
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
GValue *maker_dialog_g_value_from_string(GValue *value, const gchar *str, const gchar *parseOption);

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
gchar *maker_dialog_g_value_to_string(GValue *value, const gchar *toStringFormat);

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
 *  - G_TYPE_BOOLEAN: TRUE is greater than FALSE.
 *  - G_TYPE_INT: Natural order.
 *  - G_TYPE_UINT: Natural order.
 *  - G_TYPE_DOUBLE: Natural order.
 *  - G_TYPE_STRING: As in strcmp.
 *
 * Numeric values can be compared each other, but G_TYPE_BOOLEAN does not count as
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
gint maker_dialog_g_value_compare(GValue *value1, GValue *value2, MakerDialogCompareFunc compFunc);

#endif /* MAKER_DIALOG_UTIL_H_ */

