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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <glib/gi18n.h>
#include <locale.h>
#include "Mkdg.h"

MkdgPropertySpec *maker_dialog_property_spec_new(const gchar *key, MkdgType valueType){
    return maker_dialog_property_spec_new_full(key, valueType,
	    NULL, NULL,
	    NULL, NULL, NULL,
	    0.0, 0.0, 0.0, 0,
	    MAKER_DIALOG_PROPERTY_FLAG_CAN_FREE,  NULL, NULL, NULL, NULL,
	    NULL, NULL, NULL, NULL);
}

MkdgPropertySpec *maker_dialog_property_spec_new_full(const gchar *key,
	MkdgType valueType,
	const gchar *defaultValue, gchar **validValues,
	const gchar *parseOption, const char *toStringFormat, const gchar *compareOption,
	gdouble min, gdouble max, gdouble step, gint decimalDigits,
	MkdgPropertyFlags propertyFlags,
	const gchar *pageName, const gchar *groupName, const gchar *label, const gchar *translationContext,
	const gchar *tooltip, gchar **imagePaths, MkdgControlRule *rules, gpointer userData){
    MkdgPropertySpec *spec=g_new(MkdgPropertySpec, 1);
    if (spec){
	spec->key=key;
	spec->valueType=valueType;
	spec->defaultValue=defaultValue;
	spec->validValues=validValues;
	spec->parseOption=parseOption;
	spec->toStringFormat=toStringFormat;
	spec->compareOption=compareOption;

	spec->min=min;
	spec->max=max;
	spec->step=step;
	spec->decimalDigits=decimalDigits;

	spec->flags=propertyFlags | MAKER_DIALOG_PROPERTY_FLAG_CAN_FREE;
	spec->pageName=pageName;
	spec->groupName=groupName;
	spec->label=label;
	spec->translationContext=translationContext;
	spec->tooltip=tooltip;
	spec->imagePaths=imagePaths;
	spec->rules=rules;

	spec->userData=userData;
    }
    return spec;
}

static void maker_dialog_control_rule_set
(MkdgControlRule *rule, MkdgRelation relation, const gchar *testValue, const gchar *key, MkdgWidgetControl match, MkdgWidgetControl notMatch){
    rule->relation=relation;
    rule->testValue=g_strdup(testValue);
    rule->key=g_strdup(key);
    rule->match=match;
    rule->notMatch=notMatch;
}

void maker_dialog_control_rule_free(MkdgControlRule *rule){
    g_free((gchar *) rule->testValue);
    g_free((gchar *) rule->key);
}

static void maker_dialog_control_rules_free(MkdgControlRule *rules){
    if (!rules)
	return;
    while(rules->key!=NULL){
	MkdgControlRule *rulesTmp=rules;
	maker_dialog_control_rule_free(rulesTmp);
	rules++;
    }
    g_free(rules);
}

void maker_dialog_property_spec_free(MkdgPropertySpec *spec){
    if (spec->flags & MAKER_DIALOG_PROPERTY_FLAG_CAN_FREE){
	g_free((gchar *)spec->key);
	g_free((gchar *)spec->defaultValue);
	g_strfreev((gchar **)spec->validValues);
	g_free((gchar *) spec->parseOption);
	g_free((gchar *) spec->toStringFormat);
	g_free((gchar *) spec->pageName);
	g_free((gchar *) spec->groupName);
	g_free((gchar *) spec->label);
	g_free((gchar *) spec->translationContext);
	g_free((gchar *) spec->tooltip);
	g_free(spec->imagePaths);
	maker_dialog_control_rules_free(spec->rules);
	g_free(spec->userData);
    }
    g_free(spec);
}

MkdgPropertyContext *maker_dialog_property_context_new(
	MkdgPropertySpec *spec,gpointer userData){
    return maker_dialog_property_context_new_full(spec, userData, NULL, NULL);
}

MkdgPropertyContext *maker_dialog_property_context_new_full(
	MkdgPropertySpec *spec,	gpointer userData,
	MkdgValidateCallbackFunc validateFunc,
	MkdgApplyCallbackFunc applyFunc){
    MkdgPropertyContext *ctx=g_new(MkdgPropertyContext,1);
    if (ctx){
	ctx->flags=0;
	ctx->spec=spec;
	ctx->userData=userData;
	ctx->valueIndex=-1;
	ctx->value=maker_dialog_value_new(ctx->spec->valueType, NULL);
	ctx->validateFunc=validateFunc;
	ctx->applyFunc=applyFunc;
	ctx->mDialog=NULL;
    }
    return ctx;
}

void maker_dialog_property_context_free(MkdgPropertyContext *ctx){
    maker_dialog_value_free(ctx->value);
    if (ctx->spec->flags & MAKER_DIALOG_PROPERTY_FLAG_CAN_FREE){
	maker_dialog_property_spec_free(ctx->spec);
    }
    g_free(ctx);
}

static void  _maker_dialog_property_context_free_wrap(gpointer obj){
    maker_dialog_property_context_free((MkdgPropertyContext *) obj);
}

const gchar *maker_dialog_property_get_default_string(MkdgPropertySpec *spec){
    if (spec->defaultValue){
	if (spec->validValues){
	    if (spec->flags & MAKER_DIALOG_PROPERTY_FLAG_FIXED_SET){
		gint index=-1;
		/* Make sure default value is in valid values */
		index=maker_dialog_find_string(spec->defaultValue, spec->validValues,-1);
		if (index<0){
		    /* Force to set on 1-st validValue */
		    index=0;
		}
		return spec->validValues[index];
	    }else{
		/* Allow to edit, so we can use default value
		 * anyway
		 */
		return spec->defaultValue;
	    }
	}else{
	    return spec->defaultValue;
	}
    }else{
	/* No default value */
	if (spec->validValues){
	    return spec->validValues[0];
	}
    }
    return NULL;
}

gboolean maker_dialog_property_is_default(MkdgPropertyContext *ctx){
    MkdgValue *mValue=maker_dialog_property_get_default(ctx->spec);
    gboolean result=(maker_dialog_value_compare(ctx->value, mValue, NULL)==0)? TRUE: FALSE;
    maker_dialog_value_free(mValue);
    return result;
}

MkdgValue *maker_dialog_property_get_default(MkdgPropertySpec *spec){
    MkdgValue *mValue=maker_dialog_value_new(spec->valueType, NULL);
    return maker_dialog_value_from_string(mValue, maker_dialog_property_get_default_string(spec), spec->parseOption);
}

gboolean maker_dialog_property_set_default(MkdgPropertyContext *ctx){
    MkdgValue *mValue=maker_dialog_property_get_default(ctx->spec);
    gboolean result=FALSE;
    if (mValue){
	maker_dialog_property_set_value_fast(ctx, mValue, -2);
	result=TRUE;
    }
    maker_dialog_value_free(mValue);
    return result;
}

static void maker_dialog_property_context_update_index(MkdgPropertyContext *ctx){
    gint i;
    MkdgValue *mValue=maker_dialog_value_new(ctx->spec->valueType, NULL);
    for (i=0;ctx->spec->validValues[i]!=NULL;i++){
	maker_dialog_value_from_string(mValue, ctx->spec->validValues[i], ctx->spec->parseOption);
	if (maker_dialog_value_compare(ctx->value, mValue, NULL)==0){
	    maker_dialog_value_free(mValue);
	    ctx->valueIndex=i;
	    return;
	}
    }
    ctx->valueIndex=-1;
}

void maker_dialog_property_set_value_fast(MkdgPropertyContext *ctx, MkdgValue *value, gint valueIndexCtl){
    maker_dialog_value_copy(value,ctx->value);
    ctx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_HAS_VALUE | MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED;
    if (ctx->spec->validValues && valueIndexCtl!=-3){
	if (valueIndexCtl==-2){
	    maker_dialog_property_context_update_index(ctx);
	}else{
	    ctx->valueIndex=valueIndexCtl;
	}
    }
}

gboolean maker_dialog_property_from_string(MkdgPropertyContext *ctx, const gchar *str){
    MkdgValue *mValue=maker_dialog_value_new(ctx->spec->valueType, NULL);
    if ( maker_dialog_value_from_string(mValue, str, ctx->spec->parseOption)){
	maker_dialog_property_set_value_fast(ctx, mValue, -2);
	maker_dialog_value_free(mValue);
	return TRUE;
    }
    maker_dialog_value_free(mValue);
    return FALSE;
}

gchar *maker_dialog_property_to_string(MkdgPropertyContext *ctx){
    gchar *str=maker_dialog_value_to_string(ctx->value, ctx->spec->toStringFormat);
    return str;
}

static gboolean maker_dialog_eval_control_rule(MkdgPropertyContext *ctx, MkdgControlRule *rule){
    MkdgValue *mTestValue=maker_dialog_value_new(ctx->spec->valueType, NULL);
    maker_dialog_value_from_string(mTestValue, rule->testValue, ctx->spec->parseOption);

    gint ret=0;
    gboolean result=FALSE;
    ret=maker_dialog_value_compare(ctx->value, mTestValue, NULL);
    switch(rule->relation){
	case MAKER_DIALOG_RELATION_EQUAL:
	    result=(ret==0)? TRUE: FALSE;
	    break;
	case MAKER_DIALOG_RELATION_NOT_EQUAL:
	    result=(ret!=0)? TRUE: FALSE;
	    break;
	case MAKER_DIALOG_RELATION_LESS:
	    result=(ret==-1)? TRUE: FALSE;
	    break;
	case MAKER_DIALOG_RELATION_LESS_OR_EQUAL:
	    result=(ret==-1 || ret==0)? TRUE: FALSE;
	    break;
	case MAKER_DIALOG_RELATION_GREATER:
	    result=(ret==1)? TRUE: FALSE;
	    break;
	case MAKER_DIALOG_RELATION_GREATER_OR_EQUAL:
	    result=(ret==1 || ret==0)? TRUE: FALSE;
	    break;
	default:
	    break;
    }
    if (mTestValue)
	maker_dialog_value_free(mTestValue);
    return result;
}

void maker_dialog_property_foreach_control_rule(MkdgPropertyContext *ctx, MkdgPropertyEachControlRule func, gpointer userData){
    if (ctx->spec->rules){
	MkdgControlRule *rule=ctx->spec->rules;
	while (rule->key!=NULL){
	    gboolean ret=maker_dialog_eval_control_rule(ctx, rule);
	    MkdgPropertyContext *refCtx=maker_dialog_get_property_context(ctx->mDialog, rule->key);
	    MkdgWidgetControl control=MAKER_DIALOG_WIDGET_CONTROL_NOTHING;
	    if (ret && rule->match){
		func(refCtx, rule->match, userData);
		control=rule->match;
	    }else if (rule->notMatch) {
		func(refCtx, rule->notMatch, userData);
		control=rule->notMatch;
	    }
	    if (control & MAKER_DIALOG_WIDGET_CONTROL_SHOW){
		refCtx->flags &= ~MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_HIDDEN;
	    }else if (control & MAKER_DIALOG_WIDGET_CONTROL_HIDE){
		refCtx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_HIDDEN;
	    }
	    if (control & MAKER_DIALOG_WIDGET_CONTROL_SENSITIVE){
		refCtx->flags &= ~MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_INSENSITIVE;
	    }else if (control & MAKER_DIALOG_WIDGET_CONTROL_HIDE){
		refCtx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_INSENSITIVE;
	    }
	    rule++;
	}
    }
}

MkdgPropertyTable* maker_dialog_property_table_new(){
    return g_hash_table_new_full(g_str_hash,g_str_equal,NULL, _maker_dialog_property_context_free_wrap);
}

void maker_dialog_property_table_insert(MkdgPropertyTable *hTable, const MkdgPropertyContext *ctx){
    g_hash_table_insert(hTable, (gpointer) ctx->spec->key, (gpointer) ctx);
}

MkdgPropertyContext *maker_dialog_property_table_lookup(MkdgPropertyTable *hTable, const gchar *key){
    return (MkdgPropertyContext *) g_hash_table_lookup((gpointer) hTable,(gpointer) key);
}

MkdgValue *maker_dialog_property_table_lookup_value(MkdgPropertyTable *hTable, const gchar *key){
    MkdgPropertyContext *ctx=maker_dialog_property_table_lookup(hTable, key);
    return  ctx->value;
}

void maker_dialog_property_table_destroy (MkdgPropertyTable *hTable){
    g_hash_table_destroy(hTable);
}

void maker_dialog_foreach_property(Mkdg* mDialog, GHFunc func, gpointer userData){
    g_hash_table_foreach(mDialog->propertyTable, func, userData);
}

/*=== Start enumeration and flags ===*/
static MkdgIdPair mkdgRelationData[]={
    {"==",	MAKER_DIALOG_RELATION_EQUAL},
    {"!=",	MAKER_DIALOG_RELATION_NOT_EQUAL},
    {"<",	MAKER_DIALOG_RELATION_LESS},
    {"<=",	MAKER_DIALOG_RELATION_LESS_OR_EQUAL},
    {">",	MAKER_DIALOG_RELATION_GREATER},
    {">=",	MAKER_DIALOG_RELATION_GREATER_OR_EQUAL},
    {"EQ",	MAKER_DIALOG_RELATION_EQUAL},
    {"NE",	MAKER_DIALOG_RELATION_NOT_EQUAL},
    {"LT",	MAKER_DIALOG_RELATION_LESS},
    {"LE",	MAKER_DIALOG_RELATION_LESS_OR_EQUAL},
    {"GT",	MAKER_DIALOG_RELATION_GREATER},
    {"GE",	MAKER_DIALOG_RELATION_GREATER_OR_EQUAL},
    {NULL,	MAKER_DIALOG_RELATION_INVALID},
};

MkdgRelation maker_dialog_relation_parse(const gchar *str){
    return maker_dialog_id_parse(mkdgRelationData, str, FALSE);
}

static MkdgIdPair mkdgSpecFlagData[]={
    {"FIXED_SET",		MAKER_DIALOG_PROPERTY_FLAG_FIXED_SET},
    {"PREFER_RADIO_BUTTONS",	MAKER_DIALOG_PROPERTY_FLAG_PREFER_RADIO_BUTTONS},
    {NULL,			0},
};

MkdgPropertyFlags maker_dialog_property_flags_parse(const gchar *str){
    return maker_dialog_flag_parse(mkdgSpecFlagData, str, FALSE);
}

static MkdgIdPair mkdgWidgetControlData[]={
    {"SHOW",		MAKER_DIALOG_WIDGET_CONTROL_SHOW},
    {"HIDE",		MAKER_DIALOG_WIDGET_CONTROL_HIDE},
    {"SENSITIVE",	MAKER_DIALOG_WIDGET_CONTROL_SENSITIVE},
    {"INSENSITIVE",	MAKER_DIALOG_WIDGET_CONTROL_INSENSITIVE},
    {NULL,		MAKER_DIALOG_WIDGET_CONTROL_NOTHING},
};

MkdgWidgetControl maker_dialog_widget_control_parse(const gchar *str){
    return maker_dialog_flag_parse(mkdgWidgetControlData, str, FALSE);
}

MkdgControlRule  *maker_dialog_control_rules_parse(const gchar *str){
    gchar **ctrlList=maker_dialog_string_split_set(str, ";", '\\', FALSE, -1);
    GArray *ctrlArray=g_array_new(FALSE, FALSE, sizeof(MkdgControlRule));
    MkdgControlRule *rule=NULL;
    gint i;
    for(i=0;ctrlList[i]!=NULL;i++){
	gchar **strList=maker_dialog_string_split_set(ctrlList[i], ",", '\\', FALSE, 5);
	/* StrList[0] is relation */
	MkdgRelation relation=maker_dialog_relation_parse(strList[0]);
	if (relation<=0){
	    goto END_WIDGET_CONTROL_RULE;
	}
	g_array_set_size(ctrlArray, ctrlArray->len+1);
	rule=&g_array_index(ctrlArray, MkdgControlRule, ctrlArray->len-1);
	MkdgWidgetControl match=maker_dialog_widget_control_parse(strList[3]);
	MkdgWidgetControl notMatch=maker_dialog_widget_control_parse(strList[4]);
	maker_dialog_control_rule_set(rule, relation,strList[1],strList[2],match,notMatch);
END_WIDGET_CONTROL_RULE:
	g_strfreev(strList);
    }
    g_strfreev(ctrlList);
    g_array_set_size(ctrlArray, ctrlArray->len+1);
    rule=&g_array_index(ctrlArray, MkdgControlRule, ctrlArray->len-1);
    maker_dialog_control_rule_set(rule, MAKER_DIALOG_RELATION_NIL, NULL, NULL, 0, 0);
    MkdgControlRule *rules=(MkdgControlRule *) g_array_free(ctrlArray, FALSE);
    return rules;
}

/*=== End enumeration and flags ===*/

