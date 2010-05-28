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

MkdgPropertySpec *mkdg_property_spec_new(const gchar *key, MkdgType valueType){
    return mkdg_property_spec_new_full(key, valueType,
	    NULL, NULL,
	    NULL, NULL, NULL,
	    0.0, 0.0, 0.0, 0,
	    MKDG_PROPERTY_FLAG_CAN_FREE,  NULL, NULL, NULL, NULL,
	    NULL, NULL, NULL, NULL);
}

MkdgPropertySpec *mkdg_property_spec_new_full(const gchar *key,
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

	spec->flags=propertyFlags | MKDG_PROPERTY_FLAG_CAN_FREE;
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

static void mkdg_control_rule_set
(MkdgControlRule *rule, MkdgRelation relation, const gchar *testValue, const gchar *key, MkdgWidgetControl match, MkdgWidgetControl notMatch){
    rule->relation=relation;
    rule->testValue=g_strdup(testValue);
    rule->key=g_strdup(key);
    rule->match=match;
    rule->notMatch=notMatch;
}

void mkdg_control_rule_free(MkdgControlRule *rule){
    g_free((gchar *) rule->testValue);
    g_free((gchar *) rule->key);
}

static void mkdg_control_rules_free(MkdgControlRule *rules){
    if (!rules)
	return;
    while(rules->key!=NULL){
	MkdgControlRule *rulesTmp=rules;
	mkdg_control_rule_free(rulesTmp);
	rules++;
    }
    g_free(rules);
}

void mkdg_property_spec_free(MkdgPropertySpec *spec){
    if (spec->flags & MKDG_PROPERTY_FLAG_CAN_FREE){
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
	mkdg_control_rules_free(spec->rules);
	g_free(spec->userData);
    }
    g_free(spec);
}

MkdgPropertyContext *mkdg_property_context_new(
	MkdgPropertySpec *spec,gpointer userData){
    return mkdg_property_context_new_full(spec, userData, NULL, NULL);
}

MkdgPropertyContext *mkdg_property_context_new_full(
	MkdgPropertySpec *spec,	gpointer userData,
	MkdgValidateCallbackFunc validateFunc,
	MkdgApplyCallbackFunc applyFunc){
    MkdgPropertyContext *ctx=g_new(MkdgPropertyContext,1);
    if (ctx){
	ctx->flags=0;
	ctx->spec=spec;
	ctx->userData=userData;
	ctx->valueIndex=-1;
	ctx->value=mkdg_value_new(ctx->spec->valueType, NULL);
	ctx->validateFunc=validateFunc;
	ctx->applyFunc=applyFunc;
	ctx->mDialog=NULL;
    }
    return ctx;
}

void mkdg_property_context_free(MkdgPropertyContext *ctx){
    mkdg_value_free(ctx->value);
    if (ctx->spec->flags & MKDG_PROPERTY_FLAG_CAN_FREE){
	mkdg_property_spec_free(ctx->spec);
    }
    g_free(ctx);
}

static void  _mkdg_property_context_free_wrap(gpointer obj){
    mkdg_property_context_free((MkdgPropertyContext *) obj);
}

const gchar *mkdg_property_get_default_string(MkdgPropertySpec *spec){
    if (spec->defaultValue){
	if (spec->validValues){
	    if (spec->flags & MKDG_PROPERTY_FLAG_FIXED_SET){
		gint index=-1;
		/* Make sure default value is in valid values */
		index=mkdg_find_string(spec->defaultValue, spec->validValues,-1);
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

gboolean mkdg_property_is_default(MkdgPropertyContext *ctx){
    MkdgValue *mValue=mkdg_property_get_default(ctx->spec);
    gboolean result=(mkdg_value_compare(ctx->value, mValue, NULL)==0)? TRUE: FALSE;
    mkdg_value_free(mValue);
    return result;
}

MkdgValue *mkdg_property_get_default(MkdgPropertySpec *spec){
    MkdgValue *mValue=mkdg_value_new(spec->valueType, NULL);
    return mkdg_value_from_string(mValue, mkdg_property_get_default_string(spec), spec->parseOption);
}

gboolean mkdg_property_set_default(MkdgPropertyContext *ctx){
    MkdgValue *mValue=mkdg_property_get_default(ctx->spec);
    gboolean result=FALSE;
    if (mValue){
	mkdg_property_set_value_fast(ctx, mValue, -2);
	result=TRUE;
    }
    mkdg_value_free(mValue);
    return result;
}

static void mkdg_property_context_update_index(MkdgPropertyContext *ctx){
    gint i;
    MkdgValue *mValue=mkdg_value_new(ctx->spec->valueType, NULL);
    for (i=0;ctx->spec->validValues[i]!=NULL;i++){
	mkdg_value_from_string(mValue, ctx->spec->validValues[i], ctx->spec->parseOption);
	if (mkdg_value_compare(ctx->value, mValue, NULL)==0){
	    mkdg_value_free(mValue);
	    ctx->valueIndex=i;
	    return;
	}
    }
    ctx->valueIndex=-1;
}

void mkdg_property_set_value_fast(MkdgPropertyContext *ctx, MkdgValue *value, gint valueIndexCtl){
    mkdg_value_copy(value,ctx->value);
    ctx->flags |= MKDG_PROPERTY_CONTEXT_FLAG_HAS_VALUE | MKDG_PROPERTY_CONTEXT_FLAG_UNAPPLIED;
    if (ctx->spec->validValues && valueIndexCtl!=-3){
	if (valueIndexCtl==-2){
	    mkdg_property_context_update_index(ctx);
	}else{
	    ctx->valueIndex=valueIndexCtl;
	}
    }
}

gboolean mkdg_property_from_string(MkdgPropertyContext *ctx, const gchar *str){
    MkdgValue *mValue=mkdg_value_new(ctx->spec->valueType, NULL);
    if ( mkdg_value_from_string(mValue, str, ctx->spec->parseOption)){
	mkdg_property_set_value_fast(ctx, mValue, -2);
	mkdg_value_free(mValue);
	return TRUE;
    }
    mkdg_value_free(mValue);
    return FALSE;
}

gchar *mkdg_property_to_string(MkdgPropertyContext *ctx){
    gchar *str=mkdg_value_to_string(ctx->value, ctx->spec->toStringFormat);
    return str;
}

static gboolean mkdg_eval_control_rule(MkdgPropertyContext *ctx, MkdgControlRule *rule){
    MkdgValue *mTestValue=mkdg_value_new(ctx->spec->valueType, NULL);
    mkdg_value_from_string(mTestValue, rule->testValue, ctx->spec->parseOption);

    gint ret=0;
    gboolean result=FALSE;
    ret=mkdg_value_compare(ctx->value, mTestValue, NULL);
    switch(rule->relation){
	case MKDG_RELATION_EQUAL:
	    result=(ret==0)? TRUE: FALSE;
	    break;
	case MKDG_RELATION_NOT_EQUAL:
	    result=(ret!=0)? TRUE: FALSE;
	    break;
	case MKDG_RELATION_LESS:
	    result=(ret==-1)? TRUE: FALSE;
	    break;
	case MKDG_RELATION_LESS_OR_EQUAL:
	    result=(ret==-1 || ret==0)? TRUE: FALSE;
	    break;
	case MKDG_RELATION_GREATER:
	    result=(ret==1)? TRUE: FALSE;
	    break;
	case MKDG_RELATION_GREATER_OR_EQUAL:
	    result=(ret==1 || ret==0)? TRUE: FALSE;
	    break;
	default:
	    break;
    }
    if (mTestValue)
	mkdg_value_free(mTestValue);
    return result;
}

void mkdg_property_foreach_control_rule(MkdgPropertyContext *ctx, MkdgPropertyEachControlRule func, gpointer userData){
    if (ctx->spec->rules){
	MkdgControlRule *rule=ctx->spec->rules;
	while (rule->key!=NULL){
	    gboolean ret=mkdg_eval_control_rule(ctx, rule);
	    MkdgPropertyContext *refCtx=mkdg_get_property_context(ctx->mDialog, rule->key);
	    MkdgWidgetControl control=MKDG_WIDGET_CONTROL_NOTHING;
	    if (ret && rule->match){
		func(refCtx, rule->match, userData);
		control=rule->match;
	    }else if (rule->notMatch) {
		func(refCtx, rule->notMatch, userData);
		control=rule->notMatch;
	    }
	    if (control & MKDG_WIDGET_CONTROL_SHOW){
		refCtx->flags &= ~MKDG_PROPERTY_CONTEXT_FLAG_HIDDEN;
	    }else if (control & MKDG_WIDGET_CONTROL_HIDE){
		refCtx->flags |= MKDG_PROPERTY_CONTEXT_FLAG_HIDDEN;
	    }
	    if (control & MKDG_WIDGET_CONTROL_SENSITIVE){
		refCtx->flags &= ~MKDG_PROPERTY_CONTEXT_FLAG_INSENSITIVE;
	    }else if (control & MKDG_WIDGET_CONTROL_HIDE){
		refCtx->flags |= MKDG_PROPERTY_CONTEXT_FLAG_INSENSITIVE;
	    }
	    rule++;
	}
    }
}

MkdgPropertyTable* mkdg_property_table_new(){
    return g_hash_table_new_full(g_str_hash,g_str_equal,NULL, _mkdg_property_context_free_wrap);
}

void mkdg_property_table_insert(MkdgPropertyTable *hTable, const MkdgPropertyContext *ctx){
    g_hash_table_insert(hTable, (gpointer) ctx->spec->key, (gpointer) ctx);
}

MkdgPropertyContext *mkdg_property_table_lookup(MkdgPropertyTable *hTable, const gchar *key){
    return (MkdgPropertyContext *) g_hash_table_lookup((gpointer) hTable,(gpointer) key);
}

MkdgValue *mkdg_property_table_lookup_value(MkdgPropertyTable *hTable, const gchar *key){
    MkdgPropertyContext *ctx=mkdg_property_table_lookup(hTable, key);
    return  ctx->value;
}

void mkdg_property_table_destroy (MkdgPropertyTable *hTable){
    g_hash_table_destroy(hTable);
}

void mkdg_foreach_property(Mkdg* mDialog, GHFunc func, gpointer userData){
    g_hash_table_foreach(mDialog->propertyTable, func, userData);
}

/*=== Start enumeration and flags ===*/
static MkdgIdPair mkdgRelationData[]={
    {"==",	MKDG_RELATION_EQUAL},
    {"!=",	MKDG_RELATION_NOT_EQUAL},
    {"<",	MKDG_RELATION_LESS},
    {"<=",	MKDG_RELATION_LESS_OR_EQUAL},
    {">",	MKDG_RELATION_GREATER},
    {">=",	MKDG_RELATION_GREATER_OR_EQUAL},
    {"EQ",	MKDG_RELATION_EQUAL},
    {"NE",	MKDG_RELATION_NOT_EQUAL},
    {"LT",	MKDG_RELATION_LESS},
    {"LE",	MKDG_RELATION_LESS_OR_EQUAL},
    {"GT",	MKDG_RELATION_GREATER},
    {"GE",	MKDG_RELATION_GREATER_OR_EQUAL},
    {NULL,	MKDG_RELATION_INVALID},
};

MkdgRelation mkdg_relation_parse(const gchar *str){
    return mkdg_id_parse(mkdgRelationData, str, FALSE);
}

static MkdgIdPair mkdgSpecFlagData[]={
    {"FIXED_SET",		MKDG_PROPERTY_FLAG_FIXED_SET},
    {"PREFER_RADIO_BUTTONS",	MKDG_PROPERTY_FLAG_PREFER_RADIO_BUTTONS},
    {NULL,			0},
};

MkdgPropertyFlags mkdg_property_flags_parse(const gchar *str){
    return mkdg_flag_parse(mkdgSpecFlagData, str, FALSE);
}

static MkdgIdPair mkdgWidgetControlData[]={
    {"SHOW",		MKDG_WIDGET_CONTROL_SHOW},
    {"HIDE",		MKDG_WIDGET_CONTROL_HIDE},
    {"SENSITIVE",	MKDG_WIDGET_CONTROL_SENSITIVE},
    {"INSENSITIVE",	MKDG_WIDGET_CONTROL_INSENSITIVE},
    {NULL,		MKDG_WIDGET_CONTROL_NOTHING},
};

MkdgWidgetControl mkdg_widget_control_parse(const gchar *str){
    return mkdg_flag_parse(mkdgWidgetControlData, str, FALSE);
}

MkdgControlRule  *mkdg_control_rules_parse(const gchar *str){
    gchar **ctrlList=mkdg_string_split_set(str, ";", '\\', FALSE, -1);
    GArray *ctrlArray=g_array_new(FALSE, FALSE, sizeof(MkdgControlRule));
    MkdgControlRule *rule=NULL;
    gint i;
    for(i=0;ctrlList[i]!=NULL;i++){
	gchar **strList=mkdg_string_split_set(ctrlList[i], ",", '\\', FALSE, 5);
	/* StrList[0] is relation */
	MkdgRelation relation=mkdg_relation_parse(strList[0]);
	if (relation<=0){
	    goto END_WIDGET_CONTROL_RULE;
	}
	g_array_set_size(ctrlArray, ctrlArray->len+1);
	rule=&g_array_index(ctrlArray, MkdgControlRule, ctrlArray->len-1);
	MkdgWidgetControl match=mkdg_widget_control_parse(strList[3]);
	MkdgWidgetControl notMatch=mkdg_widget_control_parse(strList[4]);
	mkdg_control_rule_set(rule, relation,strList[1],strList[2],match,notMatch);
END_WIDGET_CONTROL_RULE:
	g_strfreev(strList);
    }
    g_strfreev(ctrlList);
    g_array_set_size(ctrlArray, ctrlArray->len+1);
    rule=&g_array_index(ctrlArray, MkdgControlRule, ctrlArray->len-1);
    mkdg_control_rule_set(rule, MKDG_RELATION_NIL, NULL, NULL, 0, 0);
    MkdgControlRule *rules=(MkdgControlRule *) g_array_free(ctrlArray, FALSE);
    return rules;
}

/*=== End enumeration and flags ===*/

