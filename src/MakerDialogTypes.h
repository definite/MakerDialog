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
 * #MKDG_G_TYPE_COLOR on the other hand, is a GType but not constant,
 * thus cannot be used in initializer elements.
 */
typedef enum {
    MKDG_TYPE_INVALID=-1,	//!< "Invalid" type. Usually use as terminator.
    MKDG_TYPE_BOOLEAN,		//!< Boolean type.
    MKDG_TYPE_INT,		//!< Integer type.
    MKDG_TYPE_UINT,		//!< Unsigned integer type.
    MKDG_TYPE_LONG,		//!< Long integer type.
    MKDG_TYPE_ULONG,		//!< Unsigned long integer type.
    MKDG_TYPE_FLOAT,		//!< Floating point number type.
    MKDG_TYPE_DOUBLE,		//!< Double precision floating point number type.
    MKDG_TYPE_STRING,		//!< String type.
    MKDG_TYPE_COLOR,		//!< Color type.
    MKDG_TYPE_NONE,		//!< "None" type.
} MkdgType;

/**
 * Generic value and type holder for Maker Dialog.
 *
 * This data structure is a wrap of GValue.
 */
typedef struct{
    MkdgType mType;		//!< MakerDialog type.
    GValue *value;		//!< Value stores here.
} MkdgValue;


/**
 * Return the GType of MakerDialog color.
 *
 * Return the GType of MakerDialog color.
 * @return GType of MakerDialog color.
 */
GType mkdg_color_get_type();

/**
 * GType for MakerDialog color type.
 *
 * This type is based on guint32.
 *
 * Note that this type neither is a constant,
 * nor can it be used in initializer elements.
 */
#define MKDG_G_TYPE_COLOR		mkdg_color_get_type()

/**
 * Convert MakerDialog type to GType.
 *
 * Convert MakerDialog type to GType.
 * @param mType		MakerDialog type.
 * @return Corresponding GType.
 */
GType maker_dialog_type_to_g_type(MkdgType mType);

/**
 * Convert GType to MakerDialog type.
 *
 * Convert GType to MakerDialog type.
 * @param gType		GType.
 * @return Corresponding MakerDialog type.
 */
MkdgType maker_dialog_type_from_g_type(GType gType);

/**
 * Whether a MakerDialog type is  number.
 *
 * Whether a MakerDialog type is  number.
 * @param mType		A MakerDialog type.
 * @return TRUE if GValue holds number; FALSE otherwise.
 * @see maker_dialog_value_get_double().
 */
gboolean maker_dialog_type_is_number(MkdgType mType);

/**
 * New a MakerDialog value.
 *
 * New a MakerDialog value.
 * @param mType		MakerDialog type.
 * @param gValue	GValue to be copied from. Can be \c NULL.
 * @return A newly allocated MakerDialog value instance.
 */
MkdgValue *maker_dialog_value_new(MkdgType mType, GValue *gValue);

/**
 * Free a MakerDialog value.
 *
 * @param mValue	The MakerDialog value to be freed.
 */
void maker_dialog_value_free(gpointer mValue);


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
 * @param parseOption	Additional control. Can be \c NULL.
 * @return The argument \a mValue if setting is successful; or \c NULL if type is not supported.
 * @see maker_dialog_g_value_from_string()
 * @see maker_dialog_g_value_to_string()
 * @see maker_dialog_value_to_string()
 */
MkdgValue *maker_dialog_value_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption);

/**
 * Output value of a MakerDialog value as string.
 *
 * This function outputs value of a MakerDialog value to string.
 * Parameter \a toStringFormat provides additional output control.
 * Its format is identical to the format string of printf().
 * Pass \c NULL for using default format for that type.
 *
 *
 * @param mValue		A MakerDialog value.
 * @param toStringFormat	Custom printf()-like format string. Pass \c NULL for using default format for that type.
 * @return The string representation of value; or \c NULL if the type is not supported.
 * @see maker_dialog_g_value_from_string()
 * @see maker_dialog_g_value_to_string()
 * @see maker_dialog_value_from_string()
 */
gchar *maker_dialog_value_to_string(MkdgValue *mValue, const gchar *toStringFormat);

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
 * @see maker_dialog_g_value_compare()
 */
gint maker_dialog_value_compare(MkdgValue *mValue1, MkdgValue *mValue2, MakerDialogCompareFunc compFunc);
#endif /* MAKER_DIALOG_TYPES_H_ */
