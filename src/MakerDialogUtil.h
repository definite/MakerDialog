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
 * DIRECTORY_SEPARATOR is the separator for splits the directories in paths.
 *
 * If WIN32 is defined, DIRECTORY_SEPARATOR is '\\',
 * otherwise '/' is used as DIRECTORY_SEPARATOR.
 */
#ifdef WIN32
#define DIRECTORY_SEPARATOR '\\'
#else
#define DIRECTORY_SEPARATOR '/'
#endif

/**
 * Environment variable for MakerDialog debug.
 */
#define MAKER_DLALOG_VERBOSE_ENV "MAKER_DIALOG_VERBOSE"

/**
 * Prototype of comparing function.
 *
 * @param value1 	The first value in GValue.
 * @param value2 	The second value in GValue.
 * @retval -1 if \a value1 \< \a value2.
 * @retval 0 if \a value1 = \a value2.
 * @retval 1 if \a value1 \> \a value2.
 */
typedef gint (* MakerDialogCompareFunc)(gpointer value1, gpointer value2);


/**
 * Type interface.
 *
 * Type interface are callback function that handles following property value operations:
 * - from_string() : parse value from string.
 * - to_string(): output value as a string.
 * - compare(): compare 2 values. See maker_dialog_value_compare() for details of return values.
 */
typedef struct{
    /**
     * Parse from string callback function.
     *
     * @param value 		GValue that stores the result.
     * @param str   		String to be parse from. \c NULL or "" to assign type default,
     * such as 0 for numeric values.
     * @param parseOption 	Option for parsing.
     * @return \a value if succeed; \c NULL if failed.
     */
    GValue *(* from_string) (GValue *value, const gchar *str, const gchar *parseOption);

    /**
     * Output value as string callback function.
     *
     * @param value 		A GValue.
     * @param toStringFormat 	printf()-like format string.
     * @return A newly allocated string which shows the value; \c NULL if failed.
     */
    gchar *(* to_string) (GValue *value, const gchar *toStringFormat);

    /**
     * Compare value of two GValues.
     *
     * This function compares value of two GValues.
     *
     * @param value1 	The first value.
     * @param value2 	The second value.
     * @param compFunc	Comparison function. Can be \c NULL.
     * @retval -3  if the values cannot be compared.
     * @retval -2 if the type is not supported.
     * @retval -1 if \a value1 \< \a value2.
     * @retval 0 if \a value1 = \a value2.
     * @retval 1 if \a value1 \> \a value2.
     * @see maker_dialog_value_compare()
     */
    gint (* compare) (GValue *value1, GValue *value2, MakerDialogCompareFunc func);
} MkdgTypeInterface;

/**
 * Whether to run debugging logic.
 *
 * This function can be used to insert debug logic, such as:
 * @code
 * if (MAKER_DIALOG_DEBUG_RUN(2)){
 *  statement_to_be_run_when_debug_level >=2
 * }
 * @endcode
 *
 * @param level		Verbose level of the debug.
 * @return TRUE if \a level <= current verbose level; FALSE otherwise.
 */
gboolean MAKER_DIALOG_DEBUG_RUN(gint level);

/**
 * Print debug message.
 *
 * @param level 	Verbose level of the debug message, the higher the level, the more verbose it should be.
 * @param format 	Printf-like format string.
 * @param ... Parameter of format.
 */
void MAKER_DIALOG_DEBUG_MSG(gint level, const gchar *format, ...);

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
 * @param str 	A string.
 * @return Boolean value represented by the string.
 */
gboolean maker_dialog_atob(const gchar *str);

/**
 * Return the index of a string in a string list.
 *
 * Return the index of a string in a string list.
 * @param str 		String to be found.
 * @param strlist 	List of string. Must be ended with NULL.
 * @param max_find 	Max strings to find. -1 for find until NULL.
 * @return Index of the string if str is in strlist before max_find; or -1 if on such string.
 */
gint maker_dialog_find_string(const gchar *str, const gchar **strlist, gint max_find);

/**
 * Whether the string is empty.
 *
 * Whether the string is NULL, or has 0 length.
 * @param str 	The string to be tested.
 * @return Whether the string is empty.
 */
gboolean maker_dialog_string_is_empty(const gchar *str);

/**
 * Free a GValue.
 *
 * This function unset the \a value and free the space \a value occupies.
 * @param value		A GValue.
 */
void maker_dialog_g_value_free(gpointer value);

/**
 * Set the content of GValue from a given string.
 *
 * This function sets the content of GValue from a given string, regarding the
 * type of that value. For, if type of that GValue is int, then the \a str is interpreted as
 * integer. Thus, \a value should be initialize to a supported type.
 * Parameter \a parseOption provides additional control. Set it \c NULL for using default.
 *
 * This function returns the value if the setting is successful;
 * or \c NULL if type of value is not supported.
 *
 * Available Option:
 *  - Set base for \c G_TYPE_INT, \c G_TYPE_UINT, \c G_TYPE_LONG, \c G_TYPE_ULONG values:
 *    - Default is "10" (decimal).
 *  - Others ignore \a parseOption.
 *
 *
 * @param value		A GValue.
 * @param str 		The string to be converted from.
 * @param parseOption	Additional control. Can be \c NULL.
 * @return The argument \a value if setting is successful; or \c NULL if type is not supported.
 * @see maker_dialog_g_value_to_string()
 * @see maker_dialog_value_from_string()
 * @see maker_dialog_value_to_string()
 */
GValue *maker_dialog_g_value_from_string(GValue *value, const gchar *str, const gchar *parseOption);

/**
 * Output value of a GValue as string.
 *
 * This function outputs value of a GValue to string.
 * Parameter \a toStringFormat provides additional output control.
 * Its format is identical to the format string of printf().
 * Pass \c NULL for using default format for that type.
 *
 *
 * @param value			A GValue.
 * @param toStringFormat	Custom printf()-like format string. Pass \c NULL for using default format for that type.
 * @return The string representation of value; or \c NULL if the type is not supported.
 * @see maker_dialog_g_value_from_string()
 * @see maker_dialog_value_from_string()
 * @see maker_dialog_value_to_string()
 */
gchar *maker_dialog_g_value_to_string(GValue *value, const gchar *toStringFormat);


/**
 * Compare value of two GValues.
 *
 * This function compares value of two GValues.
 * It is similar to maker_dialog_value_compare(),
 * except this function only compares GValues
 * and does not care about the MakerDialog type.
 *
 * Thus, it is possible that color equals some unsigned integer.
 *
 * @param value1 	The first value.
 * @param value2 	The second value.
 * @param compFunc	Comparison function. Can be \c NULL.
 * @retval -3  if the values cannot be compared.
 * @retval -2 if the type is not supported.
 * @retval -1 if \a value1 \< \a value2.
 * @retval 0 if \a value1 = \a value2.
 * @retval 1 if \a value1 \> \a value2.
 * @see maker_dialog_value_compare()
 */
gint maker_dialog_g_value_compare(GValue *value1, GValue *value2, MakerDialogCompareFunc compFunc);

/**
 * Whether a GType is  number.
 *
 * Whether a GType is  number.
 * @param type		A GType.
 * @return TRUE if GValue holds number; FALSE otherwise.
 * @see maker_dialog_value_get_double().
 */
gboolean maker_dialog_g_type_is_number(GType type);

/**
 * Get a double from a numeric GValue.
 *
 * This function gets a double from a numeric GValue.
 * It returns \c 0.0 if the value is non-numeric.
 * Use maker_dialog_value_is_number() to determine whether the returned \c 0.0
 * is because of non-numeric GValue.
 *
 * @param value	A MakerDialog value.
 * @return TRUE if GValue holds number; FALSE otherwise.
 * @see maker_dialog_type_is_number().
 * @see maker_dialog_value_set_double().
 */
gdouble maker_dialog_g_value_get_double(GValue *value);

/**
 * Set a double to a numeric GValue.
 *
 * This function sets a double to a numeric GValue.
 *
 * @param value	A MakerDialog value.
 * @param number	Number to set.
 * @see maker_dialog_value_get_double()
 */
void maker_dialog_g_value_set_number(GValue *value, gdouble number);

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
 * Whether the file is writable or can be created.
 *
 * This function returns TRUE when \a filename is writable,
 * or it does not but the parent directory is writable.
 * Returns FALSE otherwise.
 *
 * @param filename Filename to be tested.
 * @return TRUE for the file is writable or can be created; FALSE otherwise.
 */
gboolean maker_dialog_file_isWritable(const gchar *filename);

/**
 * Return the canonicalized absolute pathname.
 *
 * It works exactly the same with realpath(3), except this function can handle the path with ~,
 * where realpath cannot.
 *
 * @param path The path to be resolved.
 * @param resolved_path Buffer for holding the resolved_path.
 * @return resolved path, NULL is the resolution is not sucessful.
 */
gchar *maker_dialog_truepath(const gchar *path, gchar *resolved_path);

#endif /* MAKER_DIALOG_UTIL_H_ */

