/*
 * Copyright © 2009  Red Hat, Inc. All rights reserved.
 * Copyright © 2009  Ding-Yi Chen <dchen at redhat.com>
 *
 *  This file is part of Mkdg.
 *
 *  Mkdg is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Mkdg is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Mkdg.  If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * @file MakerDialogUtil.h
 * Utility functions for Mkdg.
 *
 * This file lists the utility functions for general purpose.
 */
#ifndef MKDG_UTIL_H_
#define MKDG_UTIL_H_
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
 * Generic value holder.
 *
 * Generic value holder holds value for various data type.
 * Primitive data type is
 * Note that you still need to allocate memory for string and pointer data types.
 */
typedef union {
    gboolean    v_boolean;		//!< For boolean value.
    gint        v_int;			//!< For integer value.
    guint       v_uint;			//!< For unsigned integer value.
    gint32      v_int32;		//!< For 32-bit integer value.
    guint32     v_uint32;		//!< For unsigned 32-bit integer value.
    gint64      v_int64;		//!< For 64-bit integer value.
    guint64     v_uint64;		//!< For 64-bit unsigned integer value.
    glong       v_long;			//!< For long integer value.
    gulong      v_ulong;		//!< For unsigned long integer value.
    gfloat      v_float;		//!< For floating-point number.
    gdouble     v_double;		//!< For double-precision floating-point number.
    gchar 	v_char;			//!< For character value.
    gunichar	v_unichar;		//!< For UCS4 wide character.
    gchar	*v_string;		//!< For strings.
    gchar	**v_string_list;	//!< For list of strings.
    gpointer    v_pointer;		//!< For generic pointers.
} MkdgValueHolder;

/**
 * Paired data, consist a numerical Id, a string Id and corresponding data.
 *
 * Paired data, consist a numerical Id, a string Id and corresponding data.
 * @since 0.3
 */
typedef struct{
    const gchar		*strId;	//!< String Id.
    gint		intId;	//!< Integer Id.
} MkdgIdPair;

/**
 * Environment variable for Mkdg debug.
 */
#define MAKER_DLALOG_VERBOSE_ENV "MKDG_VERBOSE"

/**
 * Flags data structure.
 *
 * 32-bit unsigned integer that holds flags.
 */
typedef guint32 MkdgFlags;

/**
 * Data structure for reporting error in Mkdg.
 *
 * It is essentially GError.
 */
typedef GError MkdgError;

/**
 * Prototype of comparing function.
 *
 * @param value1 	The first value in GValue.
 * @param value2 	The second value in GValue.
 * @retval -1 if \a value1 \< \a value2.
 * @retval 0 if \a value1 = \a value2.
 * @retval 1 if \a value1 \> \a value2.
 */
typedef gint (* MkdgCompareFunc)(gpointer value1, gpointer value2);


/**
 * Whether to run debugging logic.
 *
 * This function can be used to insert debug logic, such as:
 * @code
 * if (MKDG_DEBUG_RUN(2)){
 *  statement_to_be_run_when_debug_level >=2
 * }
 * @endcode
 *
 * @param level		Verbose level of the debug.
 * @return TRUE if \a level <= current verbose level; FALSE otherwise.
 */
gboolean MKDG_DEBUG_RUN(gint level);

/**
 * Print debug message.
 *
 * @param level 	Verbose level of the debug message, the higher the level, the more verbose it should be.
 * @param format 	Printf-like format string.
 * @param ... Parameter of format.
 */
void MKDG_DEBUG_MSG(gint level, const gchar *format, ...);

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
gboolean mkdg_atob(const gchar *str);

/**
 * Enumeration of error code enumeration.
 *
 * This enumeration lists all error codes for configuration file operation.
 */
typedef enum{
    MKDG_OK=0, 				//!< No error.
    MKDG_ERROR_CONFIG_ALREADY_EXIST=1, 	//!< File already exists.
    MKDG_ERROR_CONFIG_CANT_READ,	//!< File cannot be read.
    MKDG_ERROR_CONFIG_CANT_WRITE, 	//!< File cannot be written.
    MKDG_ERROR_CONFIG_INVALID_FORMAT, 	//!< File format is invalid.
    MKDG_ERROR_CONFIG_INVALID_KEY, 	//!< Key is invalid (no such key).
    MKDG_ERROR_CONFIG_INVALID_PAGE, 	//!< Page is invalid (no such page).
    MKDG_ERROR_CONFIG_INVALID_VALUE, 	//!< Value is invalid.
    MKDG_ERROR_CONFIG_NO_CONFIG_SET, 	//!< No configuration set is added.
    MKDG_ERROR_CONFIG_NO_FILE, 		//!< File does not exists.
    MKDG_ERROR_CONFIG_NOT_FOUND, 	//!< File not found.
    MKDG_ERROR_CONFIG_NOT_READY, 	//!< Configuration back-end is not ready.
    MKDG_ERROR_CONFIG_PERMISSION_DENY,	//!< Permission denied; the file permissions do not allow the attempted operation.
    MKDG_ERROR_CONFIG_OTHER, 		//!< Other error.
    MKDG_ERROR_LOADER_CANT_LOAD_LIB=50,	//!< Can't load library.
    MKDG_ERROR_LOADER_CANT_LOAD_SYM,	//!< Can't load such symbol.
    MKDG_ERROR_SPEC_CANT_READ=100,	//!< Spec file cannot be read.
    MKDG_ERROR_SPEC_INVALID_FORMAT, 	//!< Spec file format is invalid.
    MKDG_ERROR_SPEC_INVALID_KEY, 	//!< Spec file is invalid (no such key).
    MKDG_ERROR_SPEC_INVALID_PAGE, 	//!< Spec file is invalid (no such page).
    MKDG_ERROR_SPEC_INVALID_VALUE, 	//!< Spec attribute value is invalid.
} MkdgErrorCode;

/**
 * Error domain for Mkdg.
 *
 * Errors in this domain will be from the ::MkdgErrorCode enumeration.
 * See GError for information on error domains.
 */
#define MKDG_ERROR mkdg_error_quark()

/**
 * GQuark of Mkdg domain for error reporting.
 *
 * GQuark of Mkdg domain for error reporting.
 * @returns GQuark of Mkdg domain for error reporting.
 */
GQuark mkdg_error_quark (void);

/**
 * New a Mkdg error.
 *
 * New a Mkdg error.
 * @param code 		Error code.
 * @param formatStr	printf() format string.
 * @param ...		Argument for formatStr.
 * @return A newly allocated Mkdg error instance.
 * @since 0.3
 */
MkdgError *mkdg_error_new(MkdgErrorCode code, const gchar *formatStr, ...);

/**
 * Return corresponding error message, given error code.
 *
 * This function return corresponding error message, given error code.
 * The returned string is constant, no need to free it.
 *
 * @param code 		Error code.
 * @return A constant error message string, or \c NULL if \a code does not
 * match any error message.
 * @since 0.3
 */
const gchar *mkdg_get_error_message(MkdgErrorCode code);

/**
 * Print a Mkdg configuration error message.
 *
 * Print a Mkdg configuration error message in following format:
 * @code
 * [WW] domain:\<domain\> [\<code\>] \<error message\>, \<extra message\>
 * @endcode
 *
 * To suppress the error message, set environment variable \c MKDG_VERBOSE to a negative number.
 *
 * @param error 	Error to be printed.
 * @since 0.2
 */
void mkdg_error_print(MkdgError *error);

/**
 * Handle the error by keeping the latest error and print out the old error.
 *
 * This function provide a convenient function to handle error by keeping the
 * latest error if error output is not \c NULL,
 * while prints and purges the old error.
 *
 * That is, if \a errOut is \c NULL, the error in \a errIn will be printed and freed.
 *  If \a errOut is not \c NULL, the error in \a errIn will be kept as \a errOut,
 *  while the old error in \a errOut will be printed and freed.
 *
 *  Note that this function does nothing but returns FALSE if \a errIn is \c  NULL,
 *  and \a errIn will be clear to \c NULL before returning.
 *
 * @param errIn		Error input.
 * @param errOut	Error output stored here;
 * or \c NULL to print and purge error.
 * @return TRUE if error input is not NULL (has error); FALSE otherwise.
 * @since 0.2
 */
gboolean mkdg_error_handle(MkdgError *errIn, MkdgError **errOut);

/**
 * Parse a numerical Id from a string.
 *
 * This function parses a string and converts it to corresponding numerical Id.
 * Note the \a strId of the last Id pair must be \c NULL,
 * and its associated \a intId is served as default value, i.e.
 * returned if \a str does not match any \a strId in \a pairedData.
 *
 * @param pairedData	Paired data for parsing.
 * @param str		String to be parsed.
 * @param caseSensitive	Whether the matched is case sensitive.
 * @return Matched intId, or the last intId if none matched.
 * @since 0.3
 */
gint mkdg_id_parse(MkdgIdPair *pairedData, const gchar *str, gboolean caseSensitive);

/**
 * Return the associated string Id from a numerical id.
 *
 * This function returns the associated string Id from a numerical id.
 * Note the \a strId of the last Id pair must be \c NULL.
 *
 * @param pairedData	Paired data for parsing.
 * @param intId		Numerical Id.
 * @return Associated string Id; or NULL if none matches.
 * @since 0.3
 */
const gchar *mkdg_id_to_string(MkdgIdPair *pairedData, gint intId);

/**
 * Parse flags from a string.
 *
 * This function parses flags from a string, and returns flags that \a strId represented.
 *
 * If none match, 0 is returned.
 * @param pairedData	Paired data for parsing.
 * @param str		String to be parsed.
 * @param caseSensitive	Whether the matched is case sensitive.
 * @return Flags value; or 0 if none matched.
 * @since 0.2
 */
guint32 mkdg_flag_parse(MkdgIdPair *pairedData, const gchar *str, gboolean caseSensitive);

/**
 * Return the index of a string in a string list.
 *
 * Return the index of a string in a string list.
 * @param str 		String to be found.
 * @param strlist 	List of string. Must be ended with NULL.
 * @param max_find 	Max strings to find. -1 for find until NULL.
 * @return Index of the string if str is in strlist before max_find; or -1 if on such string.
 */
gint mkdg_find_string(const gchar *str, gchar **strlist, gint max_find);

/**
 * Whether the string is empty.
 *
 * Whether the string is NULL, or has 0 length.
 * @param str 	The string to be tested.
 * @return Whether the string is empty.
 */
gboolean mkdg_string_is_empty(const gchar *str);

/**
 * Splits string on unescaped delimiter characters.
 *
 * This function splits a string into tokens on unescaped delimiter characters,
 * A token is the (possibly size-0 if \a emptyToken is TRUE) longest string that does not contain
 * any of unescaped delimiter characters.
 * If \a maxTokens is reached, the remainder is appended to the last token.
 *
 * The characters right after the escape character are appended to token
 * unconditionally, so delimiters can be escaped this way.
 *
 * For example the result of mkdg_string_split_set("abc:;d\;ef:/ghi;", ":;", '\\', FALSE, -1)
 * is a NULL-terminated vector containing the three strings "abc", "d;ef", and "/ghi".
 * However if \a emptyToken is \c TRUE, then empty string "" will be generated
 * between two adjacent delimiters.
 * So the result of mkdg_string_split_set("abc:;d\;ef:/ghi;", ":;", '\\', TRUE, -1)
 * becomes five strings "abc", "", "d;ef", "/ghi", "".
 *
 * This function is aware of UTF-8 encoding,
 * but delimiters and escape character have to be ASCII characters.
 * String can be UTF-8 encoded, though.
 *
 * Use g_strfreev() to free the returned result.
 *
 * @param str 		String to be split.
 * @param delimiters	A set of characters to split the string.
 * @param escapeChar	The escape character.
 * @param emptyToken	Allow empty token.
 * @param maxTokens	The maximum number of tokens to split string into. If this is less than 1, the string is split completely.
 * @return A newly-allocated NULL-terminated array of strings; or NULL if either \a str or \a delimiters is \c NULL.
 * @see mkdg_string_combine_with_escape_char()
 * @since 0.2
 */
gchar **mkdg_string_split_set
(const gchar *str, const gchar *delimiters, gchar escapeChar, gboolean emptyToken, gint maxTokens);

/**
 * Combine a listed of strings into a single string with delimiters.
 *
 * This function combines between a listed of strings into a single string with delimiters.
 * Strings that containing delimiters and escape character will be escaped with
 * \a escapeChar.
 *
 * The first delimiter is inserted between each non-empty (length &gt;) string in \a strList.
 * However, if \a emptyToken is \c TRUE, the delimiters will be inserted as
 * around empty strings as well.
 *
 * @param strList	List of string to be combined.
 * @param delimiters	A set of characters to split the string.
 * @param escapeChar	The escape character.
 * @param emptyToken	Insert delimiters around empty string as well.
 * @return A newly-allocated string, containing result; or NULL if either \a strList or \a delimiters is \c NULL.
 * @since 0.2
 */
gchar *mkdg_string_list_combine
(gchar **strList, const gchar *delimiters, gchar escapeChar, gboolean emptyToken);

/**
 * Whether a set of flags contains all the specified flags.
 *
 * If \a flagSet  contains ALL the flags specified in \a specFlags,
 * then return \c TRUE, otherwise \c FALSE.
 * @param flagSet A set of flags to be checked.
 * @param specFlags Must-have flags.
 * @return TRUE if \a flagSet  contains ALL the flags specified in \a specFlags; FALSE otherwise.
 */
gboolean mkdg_has_all_flags(guint flagSet, guint specFlags);

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
gboolean mkdg_file_isWritable(const gchar *filename);

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
gchar *mkdg_truepath(const gchar *path, gchar *resolved_path);

#endif /* MKDG_UTIL_H_ */

