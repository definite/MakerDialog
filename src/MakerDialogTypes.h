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
 * @file MakerDialogTypes.h
 * Types used in MakerDialog.
 *
 * MakerDialog type (abbreivate as MKDGType) are inspired from GType.
 * In fact most of them are equivalent to their GType
 * counterparts, however,
 * there are some extra types like MKDG_TYPE_COLOR,
 * which is useful for dialogs but not included in GType.
 */
#ifndef MAKER_DIALOG_TYPES_H_
#define MAKER_DIALOG_TYPES_H_
#include <glib.h>
#include <glib-object.h>
#include "MakerDialogUtil.h"

/**
 * Type of MakerDialog color.
 */
typedef guint32 MkdgColor;

/**
 * Enumeration of supported MakerDialog types.
 *
 * These are constants, which can be used in initializer elements.
 *
 */
typedef enum {
    MKDG_TYPE_INVALID=-1,	//!< "Invalid" type. Usually use as terminator.
    MKDG_TYPE_BOOLEAN,		//!< Boolean type.
    MKDG_TYPE_INT,		//!< Integer type.
    MKDG_TYPE_UINT,		//!< Unsigned integer type.
    MKDG_TYPE_INT32,		//!< 32-bit integer type.
    MKDG_TYPE_UINT32,		//!< 32-bit unsigned integer type.
    MKDG_TYPE_INT64,		//!< 64-bit integer type.
    MKDG_TYPE_UINT64,		//!< 64-bit unsigned integer type.
    MKDG_TYPE_LONG,		//!< Long integer type.
    MKDG_TYPE_ULONG,		//!< Unsigned long integer type.
    MKDG_TYPE_FLOAT,		//!< Floating point number type.
    MKDG_TYPE_DOUBLE,		//!< Double precision floating point number type.
    MKDG_TYPE_STRING,		//!< String type.
    MKDG_TYPE_COLOR,		//!< Color type.
    MKDG_TYPE_NONE,		//!< "None" type.
} MkdgType;

/**
 * Enumeration of flags for MakerDialog value.
 */
typedef enum{
    MKDG_VALUE_FLAG_NEED_FREE		=0x1,	//!< Need to free the data.
} MKDG_VALUE_FLAG;

/**
 * Flags for MakerDialog value.
 */
typedef guint32 MkdgValueFlags;

/**
 * Generic value and type holder for Maker Dialog.
 *
 * This data structure mimics GValue.
 */
typedef struct{
    MkdgType 		mType;		//!< MakerDialog type.
    MkdgValueFlags	flags;		//!< Flags MakerDialog value.
    MkdgValueHolder     data[2];	//!< Value holder.
} MkdgValue;

/**
 * Type interface.
 *
 * Type interface are callback function that handles following property value operations:
 * - set(): Set the value.
 * - from_string() : parse value from string.
 * - to_string(): output value as a string.
 * - compare(): compare 2 values. See maker_dialog_value_compare() for details of return values.
 */
typedef struct{
    /**
     * Set value.
     *
     * Set value to a MakerDialog value.
     * Note for pointer-type type such as #MKDG_TYPE_STRING,
     * the \a setValue must be deep-copied to \a mValue,
     * unless \a needFree in mValue is \c FALSE.
     * @param value 		MkdgValue that stores the result.
     * @param setValue		Value to be set. \c NULL for default value for the type.
     */
    void (* set)(MkdgValue *mValue, gpointer setValue);

    /**
     * Parse from string callback function.
     *
     * Parse from string callback function.
     * @param value 		MkdgValue that stores the result.
     * @param str   		String to be parse from. \c NULL or "" to assign type default,
     * such as 0 for numeric values.
     * @param parseOption 	Option for parsing.
     * @return \a value if succeed; \c NULL if failed.
     */
    MkdgValue *(* from_string) (MkdgValue *value, const gchar *str, const gchar *parseOption);

    /**
     * Output value as string callback function.
     *
     * Output value as string callback function.
     * @param value 		A MkdgValue.
     * @param toStringFormat 	printf()-like format string.
     * @return A newly allocated string which shows the value; \c NULL if failed.
     */
    gchar *(* to_string) (MkdgValue *value, const gchar *toStringFormat);

    /**
     * Compare value of two MkdgValues.
     *
     * This function compares value of two MkdgValues.
     *
     * @param value1 	The first value.
     * @param value2 	The second value.
     * @param compareOption Comparison option. Can be \c NULL.
     * @retval -3  if the values cannot be compared.
     * @retval -2 if the type is not supported.
     * @retval -1 if \a value1 \< \a value2.
     * @retval 0 if \a value1 = \a value2.
     * @retval 1 if \a value1 \> \a value2.
     * @see maker_dialog_value_compare()
     */
    gint (* compare) (MkdgValue *value1, MkdgValue *value2, const gchar *compareOption);

    /**
     * Free the MkdgValues.
     *
     * Free the MkdgValues.
     * @param value 		A MkdgValue.
     */
    void (* free) (MkdgValue *mValue);

} MkdgTypeInterface;

/**
 * Parse a MakerDialog type from a string.
 *
 * This function parses MakerDialog types defined in #MkdgType, without the "MKDG_TYPE_" prefix,
 * such as "INT", "BOOLEAN", "COLOR".
 *
 * All others string will return \c MKDG_TYPE_INVALID.
 *
 * @param str		String to be parsed.
 * @return Corresponding MakerDialog type.
 */
MkdgType maker_dialog_type_parse(const gchar *str);

/**
 * Output a MakerDialog type as a string.
 *
 * Output a MakerDialog type as a string.
 * @param mType		A MakerDialog type.
 * @return A static string that represents the MakerDialog type.
 */
const gchar *maker_dialog_type_to_string(MkdgType mType);

/**
 * New a MakerDialog value.
 *
 * New a MakerDialog value.
 * @param mType		MakerDialog type.
 * @param setValue	Value to be set. \c NULL for default value of each type.
 * @return A newly allocated MakerDialog value instance.
 */
MkdgValue *maker_dialog_value_new(MkdgType mType, gpointer setValue);

/**
 * New a MakerDialog value from a static content.
 *
 * New a MakerDialog value from a static content, such as static string.
 * This function does not set MKDG_VALUE_FLAG_NEED_FREE, regardless the value type.
 *
 * @param mType		MakerDialog type.
 * @param setValue	Value to be set. \c NULL for default value of each type.
 * @return A newly allocated MakerDialog value instance.
 */
MkdgValue *maker_dialog_value_new_static(MkdgType mType, gpointer setValue);

/**
 * Copy a MakerDialog value to another.
 *
 * Copy a MakerDialog value to another.
 * This function returns \c FALSE if types of these values are not identical.
 *
 * @param srcValue	A MakerDialog value to be copied from.
 * @param destValue	A MakerDialog value to be copied to.
 * @return \c TRUE if succeed; \c FALSE if types of these values are not identical.
 */
gboolean maker_dialog_value_copy(MkdgValue *srcValue, MkdgValue *destValue);

/**
 * Set a MakerDialog value.
 *
 * Set a MakerDialog value.
 * @param mValue	A MakerDialog value.
 * @param setValue	Value to be set. \c NULL for default value of each type.
 */
void maker_dialog_value_set(MkdgValue *mValue, gpointer setValue);

/**
 * Free a MakerDialog value.
 *
 * Free a MakerDialog value.
 * @param mValue	The MakerDialog value to be freed.
 */
void maker_dialog_value_free(gpointer mValue);

/**
 * Whether a MakerDialog type is a pointer type.
 *
 * Whether a MakerDialog type is a pointer type such as #MKDG_TYPE_STRING.
 * @param mType		A MakerDialog type.
 * @return TRUE if the type is a numeric type; FALSE otherwise.
 * @see maker_dialog_value_get_double().
 */
gboolean maker_dialog_type_is_pointer(MkdgType mType);

/**
 * Whether a MakerDialog type is a numeric type.
 *
 * Whether a MakerDialog type is a number type.
 * Note that #MKDG_TYPE_COLOR is deemed as a number type,
 * since it stores number.
 * @param mType		A MakerDialog type.
 * @return TRUE if the type is a numbe holds number; FALSE otherwise.
 * @see maker_dialog_value_to_double().
 */
gboolean maker_dialog_type_is_number(MkdgType mType);

/**
 * Set the content of MakerDialog value from a double.
 *
 * This function set the content of MakerDialog value from a double.
 * This function do nothing for non numerical data type.
 *
 * @param value		A MakerDialog value.
 * @param number	Number to set.
 * @see maker_dialog_value_to_double()
 */
void maker_dialog_value_from_double(MkdgValue *value, gdouble number);

/**
 * Output a MakerDialog value as a double.
 *
 * This function gets a double from a numeric MakerDialog Value.
 * It returns \c 0.0 if the value is non-numeric.
 * If that is undesirable,
 * use maker_dialog_type_is_number() to check before using this function.
 *
 * @param value	A MakerDialog value.
 * @return A double that represent the value, or 0.0 for non-numeric type.
 * @see maker_dialog_type_is_number().
 * @see maker_dialog_value_from_double()
 */
gdouble maker_dialog_value_to_double(MkdgValue *value);

/**
 * Set the content of MakerDialog value from a given string.
 *
 * This function sets the content of MakerDialog value from a given string, regarding the
 * type of that value. For, if type of that value is int, then the \a str is interpreted as
 * integer. Thus, \a value should be initialize to a supported type.
 * Parameter \a parseOption provides additional control. Set it \c NULL for using default.
 *
 * This function returns the value if the setting is successful;
 * or \c NULL if type of value is not supported.
 *
 * Available Option:
 *  - Set base for \c MKDG_TYPE_INT, \c MKDG_TYPE_UINT, \c MKDG_TYPE_LONG, \c MKDG_TYPE_ULONG values:
 *    - Default is "10" (decimal).
 *  - Others ignore \a parseOption.
 *
 *
 * @param mValue	A MakerDialog value.
 * @param str 		The string to be converted from.
 * @param parseOption	Additional parse control. Can be \c NULL.
 * @return The argument \a mValue if setting is successful; or \c NULL if type is not supported.
 * @see maker_dialog_value_to_string()
 */
MkdgValue *maker_dialog_value_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption);

/**
 * Output a MakerDialog value to a string.
 *
 * This function outputs a MakerDialog value to astring.
 * Parameter \a toStringFormat provides additional output control.
 * Its format is identical to the format string of printf().
 * Pass \c NULL for using default format for that type.
 *
 *
 * @param mValue		A MakerDialog value.
 * @param toStringFormat	Custom printf()-like format string. Pass \c NULL for using default format for that type.
 * @return The string representation of value; or \c NULL if the type is not supported.
 * @see maker_dialog_value_from_string()
 */
gchar *maker_dialog_value_to_string(MkdgValue *mValue, const gchar *toStringFormat);

/**
 * Convert a string to a given format.
 *
 * Convert a string to a given format.
 * @param str			String to be converted.
 * @param mType			Type that \a str to be parsed as.
 * @param parseOption		Additional parse control. Can be \c NULL.
 * @param toStringFormat	Custom printf()-like format string. Pass \c NULL for using default format for that type.
 * @return A newly allocated converted string as result.
 */
gchar *maker_dialog_string_convert(const gchar *str, MkdgType mType, const gchar *parseOption, const gchar *toStringFormat);

/**
 * Return a normalized string-representation.
 *
 * A "normalized" string-representation can be feed into from_string() function
 * without extra argument. For example, "0x10" can be normalized to "16".
 *
 * After normalization, it is easy to compare value with string functions such as strcmp().
 * @param str	String to be normalized.
 * @param mType	Type that \a str to be parsed as.
 * @return A newly allocated string which stores normalized result.
 */
gchar *maker_dialog_string_normalized(const gchar *str, MkdgType mType);

/**
 * Compare value of two MakerDialog values.
 *
 * This function compares value of two MakerDialog values,
 * and returns:
 *  - -1 if \a mValue1 \< \a mValue2.
 *  - 0 if \a mValue1 = \a mValue2.
 *  - 1 if \a mValue1 \> \a mValue2.
 *
 * Currently, this function supports following types:
 *  - MKDG_TYPE_BOOLEAN: TRUE is greater than FALSE.
 *  - Numeric types (e.g. MKDG_TYPE_INT, MKDG_TYPE_FLOAT): Natural order.
 *  - MKDG_TYPE_STRING: As in strcmp.
 *  - MKDG_TYPE_COLOR: Converted to unsigned integers with RRGGBB format, then do integer comparison.
 *
 * Numeric values can be compared each other, but MKDG_TYPE_BOOLEAN does not count as
 * number. Nor can number compare with string.
 *
 * -3 is returned if the values cannot be compared.
 * -2 is returned if the either of value types is not supported.
 * Beware, this function uses values \<= -2 to returns error.
 *
 * @param mValue1 	The first value.
 * @param mValue2 	The second value.
 * @param compareOption	Comparison option. \c NULL for using the natural order.
 * @retval -3  if the values cannot be compared.
 * @retval -2 if the type is not supported.
 * @retval -1 if \a mValue1 \< \a mValue2.
 * @retval 0 if \a mValue1 = \a mValue2.
 * @retval 1 if \a mValue1 \> \a mValue2.
 */
gint maker_dialog_value_compare(MkdgValue *mValue1, MkdgValue *mValue2, const gchar *compareOption);

/**
 * Compare value of two MakerDialog values.
 *
 * This function compares value of two MakerDialog values,
 * and returns:
 *  - -1 if \a mValue1 \< \a mValue2.
 *  - 0 if \a mValue1 = \a mValue2.
 *  - 1 if \a mValue1 \> \a mValue2.
 *
 * Currently, this function supports following types:
 *  - MKDG_TYPE_BOOLEAN: TRUE is greater than FALSE.
 *  - MKDG_TYPE_INT: Natural order.
 *  - MKDG_TYPE_UINT: Natural order.
 *  - MKDG_TYPE_LONG: Natural order.
 *  - MKDG_TYPE_ULONG: Natural order.
 *  - MKDG_TYPE_FLOAT: Natural order.
 *  - MKDG_TYPE_DOUBLE: Natural order.
 *  - MKDG_TYPE_STRING: As in strcmp.
 *  - MKDG_TYPE_COLOR: Converted to unsigned integers with RRGGBB format, then do integer comparison.
 *
 * Numeric values can be compared each other, but MKDG_TYPE_BOOLEAN does not count as
 * number. Nor can number compare with string.
 *
 * -3 is returned if the values cannot be compared.
 * -2 is returned if the either of value types is not supported.
 * Beware, this function uses values \<= -2 to returns error.
 *
 * @param mValue1 	The first value.
 * @param mValue2 	The second value.
 * @param compFunc	Comparison function. Can be \c NULL.
 * @retval -3  if the values cannot be compared.
 * @retval -2 if the type is not supported.
 * @retval -1 if \a mValue1 \< \a mValue2.
 * @retval 0 if \a mValue1 = \a mValue2.
 * @retval 1 if \a mValue1 \> \a mValue2.
 * @see maker_dialog_value_compare()
 */
gint maker_dialog_value_compare_with_func(MkdgValue *mValue1, MkdgValue *mValue2, MakerDialogCompareFunc compFunc);

/**
 * Get a boolean value from a MakerDialog value.
 *
 * Get a boolean value from a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @return The boolean value.
 */
#define maker_dialog_value_get_boolean(mValue)		mValue->data[0].v_boolean

/**
 * Set a boolean value to a MakerDialog value.
 *
 * Set a boolean value to a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @param setValue The value to be set.
 */
#define maker_dialog_value_set_boolean(mValue, setValue)	mValue->data[0].v_boolean = setValue

/**
 * Get an integer value from a MakerDialog value.
 *
 * Get an integer value from a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @return The integer value.
 */
#define maker_dialog_value_get_int(mValue) 		mValue->data[0].v_int

/**
 * Set an integer value to a MakerDialog value.
 *
 * Set an integer value to a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @param setValue The value to be set.
 */
#define maker_dialog_value_set_int(mValue, setValue)	mValue->data[0].v_int = setValue

/**
 * Get an unsigned integer value from a MakerDialog value.
 *
 * Get an unsigned integer value from a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @return The integer value.
 */
#define maker_dialog_value_get_uint(mValue) 		mValue->data[0].v_uint

/**
 * Set an unsigned integer value to a MakerDialog value.
 *
 * Set an unsigned integer value to a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @param setValue The value to be set.
 */
#define maker_dialog_value_set_uint(mValue, setValue)	mValue->data[0].v_uint = setValue


/**
 * Get a 32-bit integer value from a MakerDialog value.
 *
 * Get a 32-bit integer value from a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @return The integer value.
 */
#define maker_dialog_value_get_int32(mValue) 		mValue->data[0].v_int32

/**
 * Set a 32-bit integer value to a MakerDialog value.
 *
 * Set a 32-bit integer value to a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @param setValue The value to be set.
 */
#define maker_dialog_value_set_int32(mValue, setValue)	mValue->data[0].v_int32 = setValue

/**
 * Get a 32-bit unsigned integer value from a MakerDialog value.
 *
 * Get a 32-bit unsigned integer value from a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @return The integer value.
 */
#define maker_dialog_value_get_uint32(mValue) 		mValue->data[0].v_uint32

/**
 * Set a 32-bit unsigned integer value to a MakerDialog value.
 *
 * Set a 32-bit unsigned integer value to a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @param setValue The value to be set.
 */
#define maker_dialog_value_set_uint32(mValue, setValue)	mValue->data[0].v_uint32 = setValue

/**
 * Get a 64-bit integer value from a MakerDialog value.
 *
 * Get a 64-bit integer value from a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @return The integer value.
 */
#define maker_dialog_value_get_int64(mValue) 		mValue->data[0].v_int64

/**
 * Set a 64-bit integer value to a MakerDialog value.
 *
 * Set a 64-bit integer value to a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @param setValue The value to be set.
 */
#define maker_dialog_value_set_int64(mValue, setValue)	mValue->data[0].v_int64 = setValue

/**
 * Get a 64-bit unsigned integer value from a MakerDialog value.
 *
 * Get a 64-bit unsigned integer value from a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @return The integer value.
 */
#define maker_dialog_value_get_uint64(mValue) 		mValue->data[0].v_uint64

/**
 * Set a 64-bit unsigned integer value to a MakerDialog value.
 *
 * Set a 64-bit unsigned integer value to a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @param setValue The value to be set.
 */
#define maker_dialog_value_set_uint64(mValue, setValue)	mValue->data[0].v_uint64 = setValue

/**
 * Get a long integer value from a MakerDialog value.
 *
 * Get a long integer value from a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @return The long integer value.
 */
#define maker_dialog_value_get_long(mValue) 		mValue->data[0].v_long

/**
 * Set a long integer value to a MakerDialog value.
 *
 * Set a long integer value to a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @param setValue The value to be set.
 */
#define maker_dialog_value_set_long(mValue, setValue)	mValue->data[0].v_long = setValue

/**
 * Get an unsigned long integer value from a MakerDialog value.
 *
 * Get an unsigned long integer value from a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @return The unsigned long integer value.
 */
#define maker_dialog_value_get_ulong(mValue) 		mValue->data[0].v_ulong

/**
 * Set an unsigned long integer value to a MakerDialog value.
 *
 * Set an unsigned long integer value to a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @param setValue The value to be set.
 */
#define maker_dialog_value_set_ulong(mValue, setValue)	mValue->data[0].v_ulong = setValue

/**
 * Get a floating-point value from a makerdialog value.
 *
 * Get a floating-point value from a makerdialog value.
 * @param mValue A MakerDailog value.
 * @return the floating-point value.
 */
#define maker_dialog_value_get_float(mValue) 		mValue->data[0].v_float

/**
 * Set a floating-point value to a MakerDialog value.
 *
 * Set a floating-point value to a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @param setValue The value to be set.
 */
#define maker_dialog_value_set_float(mValue, setValue)	mValue->data[0].v_float = setValue

/**
 * Get a double precision floating-point value from a makerdialog value.
 *
 * Get a double precision floating-point value from a makerdialog value.
 * @param mValue A MakerDailog value.
 * @return the double precision floating-point value.
 */
#define maker_dialog_value_get_double(mValue) 		mValue->data[0].v_double

/**
 * Set a double precision floating-point value to a MakerDialog value.
 *
 * Set a double precision floating-point value to a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @param setValue The value to be set.
 */
#define maker_dialog_value_set_double(mValue, setValue)	mValue->data[0].v_double = setValue

/**
 * Get a string value from a makerdialog value.
 *
 * Get a string value from a makerdialog value.
 * @param mValue A MakerDailog value.
 * @return the string value.
 */
#define maker_dialog_value_get_string(mValue) 		(gchar *) mValue->data[0].v_pointer

/**
 * Set a string value to a MakerDialog value.
 *
 * Set a string value to a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @param setValue The value to be set.
 */
#define maker_dialog_value_set_string(mValue, setValue)	mValue->data[0].v_pointer = (gpointer) setValue

/**
 * Get a color value from a makerdialog value.
 *
 * Get a color value from a makerdialog value.
 * @param mValue A MakerDailog value.
 * @return the color value.
 */
#define maker_dialog_value_get_color(mValue) 		mValue->data[0].v_uint32

/**
 * Set a color value to a MakerDialog value.
 *
 * Set a color value to a MakerDialog value.
 * @param mValue A MakerDailog value.
 * @param setValue The value to be set.
 */
#define maker_dialog_value_set_color(mValue, setValue)	mValue->data[0].v_uint32 = setValue

#endif /* MAKER_DIALOG_TYPES_H_ */
