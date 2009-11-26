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

