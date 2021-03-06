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
 * @file MakerDialogProperty.h
 * Property Specification and Manipulation.
 *
 * In Mkdg, a property is a configuration option which associate with a
 * value. According to property specification, Mkdg generates
 * corresponding UI component to manipulate the property value.
 */
#ifndef MKDG_PROPERTY_H_
#define MKDG_PROPERTY_H_
#include <glib.h>
#include <glib-object.h>


/**
 * An end-of-property-spec-list definition.
 *
 * This defines an empty property spec, which can be put in property spec arrays
 * as a terminator for following code:
 * @code
 * MkdgPropertySpec specList[]={
 *     {....},
 *     {....}.
 *     ......,
 *    MKDG_PROPERTY_SPEC_ENDER
 * };
 *
 * for(i=0; specList[i]->validType!=MKDG_TYPE_INVALID;i++){
 * ...
 * }
 * @endcode
 */
#define MKDG_PROPERTY_SPEC_ENDER {\
    NULL, MKDG_TYPE_INVALID, 0,\
    NULL, NULL, NULL, NULL, NULL,\
    0.0, 0.0, 0.0, 0,\
    NULL, NULL, NULL, NULL, NULL,\
    NULL, NULL, NULL}\

/**
 * Enumeration of Mkdg flags.
 *
 * Flags for a configuration property. These flags, along with the property
 * type, determine how the UI is represented.
 */
typedef enum {
    /**
     * The property spec can be freed.
     *
     * This flag is automatically set by mkdg_property_spec_new() and mkdg_property_spec_new_full().
     * Property specs read from file are freeable, for example.
     */
    MKDG_PROPERTY_FLAG_CAN_FREE    		=0x100,
    MKDG_PROPERTY_FLAG_FIXED_SET 		=0x200, //!< The property choose only among predefined valid values.
    MKDG_PROPERTY_FLAG_PREFER_RADIO_BUTTONS 	=0x400, //!< Use radio buttons if possible. Need to set ::MKDG_PROPERTY_FLAG_FIXED_SET as well.
} MKDG_PROPERTY_FLAG;

/**
 * Data structure for storing property flags.
 */
typedef MkdgFlags MkdgPropertyFlags;

/**
 * The relational operators used in Mkdg.
 *
 * The relation is used to test property value and test value.
 */
typedef enum {
    MKDG_RELATION_INVALID=-1,		//!< Invalid relation.
    MKDG_RELATION_NIL=0,		//!< No relation, it is used as a terminator.
    MKDG_RELATION_EQUAL,		//!< If property value and test value are equal.
    MKDG_RELATION_NOT_EQUAL,		//!< If property value and test value are not equal.
    MKDG_RELATION_LESS,			//!< If property value is less than test value.
    MKDG_RELATION_LESS_OR_EQUAL,	//!< If property value is less than or equal to test value.
    MKDG_RELATION_GREATER,		//!< If property value is greater than test value.
    MKDG_RELATION_GREATER_OR_EQUAL,	//!< If property value is greater than or equal totest value.
} MKDG_RELATION;

/**
 * Data structure for storing relations.
 */
typedef gint MkdgRelation;

/**
 * Flags of widget control.
 */
typedef enum {
    MKDG_WIDGET_CONTROL_NOTHING		=0x0,	//!< Do nothing.
    MKDG_WIDGET_CONTROL_SHOW		=0x1,	//!< Show a widget.
    MKDG_WIDGET_CONTROL_HIDE		=0x2,	//!< Hide a widget. Ignore if \c MKDG_WIDGET_CONTROL_SHOW is set.
    MKDG_WIDGET_CONTROL_SENSITIVE	=0x4,	//!< Make a widget sensitive (able to get input).
    MKDG_WIDGET_CONTROL_INSENSITIVE	=0x8,	//!< Make a widget insensitive (not able to get input).
} MKDG_WIDGET_CONTROL;

/**
 * Data structure for storing widget control.
 */
typedef guint MkdgWidgetControl;

/**
 * Control rule that associated with the property.
 *
 */
typedef struct{
    MkdgRelation relation;		//!< Relational operator.
    const gchar *testValue;			//!< Test value for property value to be compared with.
    const gchar *key;				//!< The key of property to be affect with.
    MkdgWidgetControl match;		//!< Control to be perform if property value matches this rule. Put 0 to do nothing,
    MkdgWidgetControl notMatch;		//!< Control to be perform if property value does not match this rule. Put 0 to do nothing,
} MkdgControlRule;

/**
 * A MkdgPropertySpec determine how UI components be generated.
 *
 * As the name suggests, this data structure stores the specification of a
 * configuration property.
 */
typedef struct _MkdgPropertySpec{
    const gchar *key;			//!< String that identify the property.
    MkdgType valueType;			//!< Data type of the property value.
    MkdgPropertyFlags flags; 	//!< Flags for a configuration property.
    const gchar *defaultValue;		//!< Default value represent in string. Can be \c NULL.
    gchar **validValues;		//!< Valid values represent in strings. Can be \c NULL.
    /**
     * Option for parsing \a defaultValue and \a validValues.
     * For example "8" can be passed as base for integer property. \c NULL for using default (intuitive) parse.
     */
    const gchar *parseOption;
    const gchar *toStringFormat;	//!< printf()-like format string used in to_string functions(). \c NULL for using default  (intuitive) format.
    const gchar *compareOption;	 	//!< Option for comparing values. \c NULL for using the natural order.

    gdouble min;			//!< Minimum value of a number. Irrelevant to other data type.
    gdouble max;			//!< Maximum value of a number. Or max characters of an input entry.
    gdouble step;			//!< Increment added or subtracted by spinning the widget.
    gint   decimalDigits;		//!< Number of digits after decimal digits.

    const gchar *pageName;		//!< Page that this property belongs to. It will appear as a tab label in GUI. Can be \c NULL.
    const gchar *groupName;		//!< Group that this property belongs to. It will appear as a label in GUI. Can be \c NULL.
    const gchar *label;			//!< Label of this property.
    const gchar *translationContext;	//!< Translation message context as for dgettext(). Can be \c NULL.
    const gchar *tooltip;		//!< Tooltip to be shown when mouse hover over the property. Can be \c NULL.
    gchar **imagePaths;		//!< Path to associated images file, the last should be NULL-terminated. Can be \c NULL.

    MkdgControlRule *rules;	//!< Rules that involved with other

    gpointer userData;			//!< For storing custom data structure. Can be \c NULL.
} MkdgPropertySpec;

/**
 * A MkdgPropertyContext is a property context which associates property specification,
 * a value, user data, verification and apply functions.
 */
typedef struct _MkdgPropertyContext MkdgPropertyContext;

/**
 * Prototype of callback function for validating value.
 *
 * This callback function will be called when value validation is required.
 * TRUE should be returned is value is valid, FALSE otherwise.
 *
 * @param ctx A context of a Mkdg context.
 * @param value Value to be set of the property.
 * @return TRUE for pass; FALSE for fail.
 * @see MkdgApplyCallbackFunc(), ::MkdgPropertySpec .
 */
typedef gboolean (* MkdgValidateCallbackFunc)(MkdgPropertySpec *spec, MkdgValue *value);

/**
 * Prototype of callback function for applying value.
 *
 * This callback function will be called when applying the current value of
 * property to the system.
 *
 * @param ctx A context of a Mkdg context.
 * @param value Value to be set of the property.
 *
 * @see MkdgValidateCallbackFunc(), ::MkdgPropertySpec .
 */
typedef void (* MkdgApplyCallbackFunc)(MkdgPropertyContext *ctx, MkdgValue *value);

/**
 * Property context flags.
 *
 * These are enumeration of property context flags.
 * Note these flags are automatically set by Mkdg. Do not manually modify them.
 */
typedef enum{
    MKDG_PROPERTY_CONTEXT_FLAG_HAS_VALUE	=0x1, //!< Whether the value is set.
    MKDG_PROPERTY_CONTEXT_FLAG_UNSAVED		=0x2, //!< The value is modified but unsaved.
    MKDG_PROPERTY_CONTEXT_FLAG_UNAPPLIED	=0x4, //!< The value has not been applied. i.e. value has not passed to property context applyFunc().
    MKDG_PROPERTY_CONTEXT_FLAG_HIDDEN		=0x8, //!< The UI widget of the property is hided.
    MKDG_PROPERTY_CONTEXT_FLAG_INSENSITIVE	=0x10, //!< The UI widget of the property is insensitive.
} MkdgPropertyContextFlag;

/**
 * Data structure that holds property context flags.
 */
typedef guint MkdgPropertyContextFlags;

/**
 * A MkdgPropertyContext is a property context which associates property specification,
 * a value, and a referencing object.
 *
 * When a value is changed though UI, validateFunc() will be invoked to validate whether the values is acceptable.
 * If it passes, then the value will be set and the referencing object can be
 * tuned by calling the setFunc().
 */
struct _MkdgPropertyContext{
    MkdgPropertySpec 	*spec;		//!< Corresponding property spec.
    MkdgValue 			*value;		//!< Current value of the property.
    gint			valueIndex;	//!< Index of the value in validValues. -1 if value is not in validValues, or validValues does not exist.
    gpointer 			userData;	//!< For storing user data
    MkdgValidateCallbackFunc 	validateFunc;	//!< Function to be called for value validation.
    MkdgApplyCallbackFunc 	applyFunc;	//!< Function to be called for applying value.
    MkdgPropertyContextFlags	flags;	//!< Property context flags.
    /// @cond
    Mkdg				*mDialog; //!< "Parent" Mkdg.
    /// @endcond
};

/**
 * A hash table that use property key as key, a and MkdgPropertyContext
 * as value.
 *
 * It is essentially a GHashTable, so g_hash_table functions can be use with
 * it.
 */
typedef GHashTable MkdgPropertyTable;

/*=== Start Function Definition  ===*/
/**
 * New a MkdgPropertySpec.
 *
 * New a MkdgPropertySpec.
 * Note that the key is not duplicated during the construction,
 * nor it will be freed by mkdg_property_spec_free().
 *
 * @param key String that identify the property.
 * @param valueType Data type of the property value.
 * @return A newly allocated MkdgPropertyContext.
 * @see mkdg_property_spec_new_full()
 */
MkdgPropertySpec *mkdg_property_spec_new(const gchar *key, MkdgType valueType);


/**
 * New a MkdgPropertySpec with all information.
 *
 * New a MkdgPropertySpec.
 * Note that the key is not duplicated during the construction,
 * nor it will be freed by mkdg_property_spec_free().
 *
 * @param key			String that identify the property.
 * @param valueType		Data type of the property value.
 * @param propertyFlags		Flags for a configuration property.
 * @param defaultValue		Default value represent in string. Can be \c NULL.
 * @param validValues 		Valid values represent in strings. Can be \c NULL.
 * @param parseOption   	Option for parsing \a defaultValue and \a validValues. Can be \c NULL.
 * @param toStringFormat	printf()-like format string used in to_string functions(). \c NULL for using default  (intuitive) format.
 * @param compareOption		Option for comparing values. \c NULL for using the natural order.
 * @param min 			Minimum value of a number. Irrelevant to other data type.
 * @param max			Maximum value of a number. Irrelevant to other data type.
 * @param step			Increment added or subtracted by spinning the widget.
 * @param decimalDigits		Number of digits after decimal digits.
 * @param pageName		Page that this property belongs to. It will appear as a tab label in GUI. Can be \c NULL.
 * @param groupName		Group that this property belongs to. It will appear as a label in GUI. Can be \c NULL.
 * @param label			Label of this property.
 * @param translationContext	Translation message context as for dgettext(). Can be \c NULL.
 * @param tooltip		Tooltip to be shown when mouse hover over the property.  Can be \c NULL.
 * @param imagePaths		Associated image files.  Can be \c NULL.
 * @param rules			Control rules.  Can be \c NULL.
 * @param userData		For storing custom data structure. Can be \c NULL.
 * @return A newly allocated MkdgPropertyContext.
 * @see mkdg_property_spec_new()
 */
MkdgPropertySpec *mkdg_property_spec_new_full(const gchar *key,
	MkdgType valueType,
	const gchar *defaultValue, gchar **validValues,
	const gchar *parseOption, const char *toStringFormat, const gchar *compareOption,
	gdouble min, gdouble max, gdouble step, gint decimalDigits,
	MkdgPropertyFlags propertyFlags,
	const gchar *pageName, const gchar *groupName, const gchar *label, const gchar *translationContext,
	const gchar *tooltip, gchar **imagePaths, MkdgControlRule *rules, gpointer userData);

/**
 * Free a MkdgPropertySpec.
 *
 * Free a MkdgPropertySpec.
 * Note that this function only free itself, but not strings associated with it.
 *
 * @param spec Property specification.
 */
void mkdg_property_spec_free(MkdgPropertySpec *spec);

/**
 * New a MkdgPropertyContext.
 *
 * New a MkdgPropertyContext, according to the initValue.
 *
 *
 * @param spec Property specification.
 * @param userData	For storing custom data structure.
 * @return A newly allocated MkdgPropertyContext.
 * @see mkdg_property_context_new_full()
 */
MkdgPropertyContext *mkdg_property_context_new(MkdgPropertySpec *spec, gpointer userData);

/**
 * New a MkdgPropertyContext with callback functions.
 *
 * New a MkdgPropertyContext, according to the initValue
 * and set and validate callback functions.
 *
 * @param spec Property specification.
 * @param userData	For storing custom data structure.
 * @param validateFunc Callback function call for value validation.
 * @param applyFunc Callback function for applying value.
 * @return A newly allocated MkdgPropertyContext.
 * @see mkdg_property_context_new()
 */
MkdgPropertyContext *mkdg_property_context_new_full(MkdgPropertySpec *spec,
	gpointer userData,
	MkdgValidateCallbackFunc	validateFunc,
	MkdgApplyCallbackFunc applyFunc);

/**
 * Free a MkdgPropertyContext.
 *
 * Free a Mkdg property context.
 * Note that the value under the property context will be unset by g_value_unset().
 *
 * @param ctx A Mkdg property context.
 */
void mkdg_property_context_free(MkdgPropertyContext *ctx);

/**
 * Whether the property value is default value.
 *
 * Whether the property value is default value.
 * @param ctx A Mkdg property context.
 * @return TRUE if the property value is default value; FALSE otherwise.
 * @see mkdg_set_value()
 * @see mkdg_property_get_default_string()
 * @see mkdg_property_get_default()
 * @see mkdg_property_set_default()
 * @see mkdg_property_set_value_fast()
 */
gboolean mkdg_property_is_default(MkdgPropertyContext *ctx);

/**
 * Get the "true" default value of a property.
 *
 * This function does not merely return \a defaultValue in \a spec.
 * It also checks whether it is in \c validValues, and flag ::MKDG_PROPERTY_FLAG_FIXED_SET.
 *
 * Specifically, this function returns:
 * # \a defaultValue, if:
 *   # \a defaultValue is in \a validValues  or ::MKDG_PROPERTY_FLAG_FIXED_SET is not set.
 *   # or \a validValues does not exist.
 * # First value in \a validValues, if:
 *   # \a defaultValue is not in \a validValues and ::MKDG_PROPERTY_FLAG_FIXED_SET is set.
 *   # \a defaultValue does not exist.
 * # \c NULL if none of above matches.
 * @param spec 	A MakerDailog property spec.
 * @retval defaultValue if it is valid.
 * @retval validValues[0] if \a defaltValue is not valid, but validValues exists.
 * @retval NULL if neither defaultValue is valid, nor validValues exists.
 * @see mkdg_set_value()
 * @see mkdg_property_is_default()
 * @see mkdg_property_get_default()
 * @see mkdg_property_set_default()
 * @see mkdg_property_set_value_fast()
 */
const gchar *mkdg_property_get_default_string(MkdgPropertySpec *spec);

/**
 * Return the "true" default value.
 *
 * This function is similar to mkdg_property_get_default_string(),
 * except this function returns a MkdgValue.
 *
 * Free the returned result after used.
 *
 * @param spec A MakerDailog property spec.
 * @return A newly allocated MkdgValue which stores the default value.
 * @see mkdg_set_value()
 * @see mkdg_property_is_default()
 * @see mkdg_property_get_default_string()
 * @see mkdg_property_set_default()
 * @see mkdg_property_set_value_fast()
 * @see mkdg_property_from_string()
 * @see mkdg_property_to_string()
 */
MkdgValue *mkdg_property_get_default(MkdgPropertySpec *spec);

/**
 * Set a property to default value.
 *
 * Set a property to default value.
 *
 * @param ctx A MakerDailog property context.
 * @return TRUE if succeed; FALSE otherwise.
 * @see mkdg_set_value()
 * @see mkdg_property_is_default()
 * @see mkdg_property_get_default_string()
 * @see mkdg_property_get_default()
 * @see mkdg_property_set_value_fast()
 * @see mkdg_property_from_string()
 * @see mkdg_property_to_string()
 */
gboolean mkdg_property_set_default(MkdgPropertyContext *ctx);

/**
 * Set value to property without validation and UI widget update.
 *
 * This function merely copies \a value to property context and updates property context flags.
 * Parameter \a valueIndexCtl control the behaviors that how \a valueIndex should be updated:
 *  - if valueIndexCtl ==-3 or  \a validValues does not exists, the \a valueIndex will not be updated.
 *  - if valueIndexCtl ==-2, then \a valueIndex is updated by finding the value in \a validValues.
 *  - if valueIndexCtl >=-1, then \a valueIndex is set as the \a valueIndexCtl.
 *
 * @param ctx 		A Mkdg property context.
 * @param valueIndexCtl Number which indicates how should \a valueIndex be updated.
 * @param value 	Value to be check.
 * @see mkdg_set_value()
 * @see mkdg_property_is_default()
 * @see mkdg_property_get_default_string()
 * @see mkdg_property_get_default()
 * @see mkdg_property_set_default()
 * @see mkdg_property_from_string()
 * @see mkdg_property_to_string()
 */
void mkdg_property_set_value_fast(MkdgPropertyContext *ctx, MkdgValue *value, gint valueIndexCtl);

/**
 * Set a property value from a string.
 *
 * Set a property value from a string.
 * @param ctx 		A Mkdg property context.
 * @param str		The string to be parsed.
 * @return TRUE if succeed; FALSE if the string cannot be parsed.
 * @see mkdg_set_value()
 * @see mkdg_property_is_default()
 * @see mkdg_property_get_default_string()
 * @see mkdg_property_get_default()
 * @see mkdg_property_set_default()
 * @see mkdg_property_set_value_fast()
 * @see mkdg_property_to_string()
 */
gboolean mkdg_property_from_string(MkdgPropertyContext *ctx, const gchar *str);

/**
 * Output a property value to a string.
 *
 * Output a property value to a string.
 * @param ctx 		A Mkdg property context.
 * @return A newly allocated string which shows the value of the property.
 * @see mkdg_set_value()
 * @see mkdg_property_is_default()
 * @see mkdg_property_get_default_string()
 * @see mkdg_property_get_default()
 * @see mkdg_property_set_default()
 * @see mkdg_property_set_value_fast()
 * @see mkdg_property_from_string()
 */
gchar *mkdg_property_to_string(MkdgPropertyContext *ctx);

/**
 * Prototype of callback function for each control rule in a property.
 *
 * This function will be called by mkdg_property_foreach_control_rule().
 * @param ctx 		A Mkdg property context.
 * @param control	Flags of widget control.
 * @param userData	Custom user data.
 */
typedef void (* MkdgPropertyEachControlRule)(MkdgPropertyContext *ctx, MkdgWidgetControl control, gpointer userData);

/**
 * Call callback for each widget control rules in a property.
 *
 * Call callback for each widget control rules in a property.
 * @param ctx 		A Mkdg property context.
 * @param func		The callback function.
 * @param userData	Custom user data to be passed to callback function.
 */
void mkdg_property_foreach_control_rule(MkdgPropertyContext *ctx, MkdgPropertyEachControlRule func, gpointer userData);

/**
 * New a maker dialog property table.
 *
 * @return A newly allocated MkdgPropertyTable.
 */
MkdgPropertyTable* mkdg_property_table_new();

/**
 * Insert the property context to the table.
 *
 * Key of property context is from ctx-&gt;spec-&gt;key.
 * This key is served as hash search key as well.
 *
 * @param hTable A MkdgPropertyTable.
 * @param ctx A Mkdg property context.
 */
void mkdg_property_table_insert(MkdgPropertyTable *hTable, const MkdgPropertyContext *ctx);

/**
 * Lookup the property context by key.
 *
 * Note that returned property context should not be freed at the point,
 * because the hash table is still referencing it.

 * @param hTable A MkdgPropertyTable.
 * @param key Key of property context.
 * @return Corresponding property context; or %NULL if no such property context.
 */
MkdgPropertyContext *mkdg_property_table_lookup(MkdgPropertyTable *hTable, const gchar *key);

/**
 * Lookup the property value by key.
 *
 * Note that returned property vlue should not be freed at the point,
 * because the hash table and property context are still referencing it.
 *
 * @param hTable A MkdgPropertyTable.
 * @param key Key of property context.
 * @return Corresponding property context; or %NULL if no such property context.
 */
MkdgValue *mkdg_property_table_lookup_value(MkdgPropertyTable *hTable, const gchar *key);

/**
 * Destroys all keys and values in the GHashTable and decrements its reference count by 1.
 *
 * @param hTable A MkdgPropertyTable.
 */
void mkdg_property_table_destroy (MkdgPropertyTable *hTable);

/**
 * Call callback for each property.
 *
 * Calls the given function for each property.
 * The function is passed the key and value of each pair, and the given user_data parameter.
 * The hash table may not be modified while iterating over it (you can't add/remove items).
 *
 * This function does not guarantee the order of keys and pages.
 * Use mkdg_foreach_page_foreach_property() for ordered pages and keys.
 *
 * @param mDialog 	A Mkdg.
 * @param func 		The callback function to be called for each key/value pair.
 * @param userData 	User data to pass to the callback function.
 */
void mkdg_foreach_property(Mkdg* mDialog, GHFunc func, gpointer userData);

/**
 * Parse a Mkdg relation from string.
 *
 * Parse a Mkdg relation from string.
 * @param str		String to be parsed.
 * @return Corresponding MkdgRelation; or \c MKDG_RELATION_INVALID if none matched.
 */
MkdgRelation mkdg_relation_parse(const gchar *str);

/**
 * Parse a Mkdg property flags from string.
 *
 * This function parse a Mkdg property flags from string,
 * which use '|' to delimit among flags.
 *
 * @param str		String to be parsed. '|' is used to separate flags.
 * @return Corresponding MkdgPropertyFlags; or \c 0 if none matched.
 */
MkdgPropertyFlags mkdg_property_flags_parse(const gchar *str);

/**
 * Parse a widget control from a string.
 *
 * Parse a widget control from a string.
 * @param str		The string to be parsed.
 * @return Corresponding widget control; or \c MKDG_WIDGET_CONTROL_NOTHING if none matched.
 */
MkdgWidgetControl mkdg_widget_control_parse(const gchar *str);

/**
 * Parse control rules from a string.
 *
 * Parse control rules from a string.
 * @param str		The string to be parsed.
 * @return A newly allocated array of MkdgControlRule, ending with the rule that
 * has the \c MKDG_RELATION_NIL.
 */
MkdgControlRule  *mkdg_control_rules_parse(const gchar *str);
/*=== End Function Definition  ===*/

#endif /* MKDG_PROPERTY_H_ */

