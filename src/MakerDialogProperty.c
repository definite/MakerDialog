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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <glib/gi18n.h>
#include <locale.h>
#include "MakerDialog.h"

MakerDialogPropertySpec *maker_dialog_property_spec_new(const gchar *key, MkdgType valueType){
    return maker_dialog_property_spec_new_full(key, valueType,
	    NULL, NULL,
	    NULL, NULL, NULL,
	    0.0, 0.0, 0.0, 0,
	    MAKER_DIALOG_PROPERTY_FLAG_CAN_FREE,  NULL, NULL, NULL, NULL,
	    NULL, NULL, NULL, NULL);
}

MakerDialogPropertySpec *maker_dialog_property_spec_new_full(const gchar *key,
	MkdgType valueType,
	const gchar *defaultValue, gchar **validValues,
	const gchar *parseOption, const char *toStringFormat, const gchar *compareOption,
	gdouble min, gdouble max, gdouble step, gint decimalDigits,
	MakerDialogPropertyFlags propertyFlags,
	const gchar *pageName, const gchar *groupName, const gchar *label, const gchar *translationContext,
	const gchar *tooltip, gchar **imagePaths, MakerDialogControlRule *rules, gpointer userData){
    MakerDialogPropertySpec *spec=g_new(MakerDialogPropertySpec, 1);
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

void maker_dialog_control_rule_free(MakerDialogControlRule *rule){
    g_free((gchar *) rule->testValue);
    g_free((gchar *) rule->key);
}

static void maker_dialog_control_rules_free(MakerDialogControlRule *rules){
    if (!rules)
	return;
    while(rules->key!=NULL){
	MakerDialogControlRule *rulesTmp=rules;
	maker_dialog_control_rule_free(rulesTmp);
	rules++;
    }
    g_free(rules);
}

void maker_dialog_property_spec_free(MakerDialogPropertySpec *spec){
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

MakerDialogPropertyContext *maker_dialog_property_context_new(
	MakerDialogPropertySpec *spec,gpointer userData){
    return maker_dialog_property_context_new_full(spec, userData, NULL, NULL);
}

MakerDialogPropertyContext *maker_dialog_property_context_new_full(
	MakerDialogPropertySpec *spec,	gpointer userData,
	MakerDialogValidateCallbackFunc validateFunc,
	MakerDialogApplyCallbackFunc applyFunc){
    MakerDialogPropertyContext *ctx=g_new(MakerDialogPropertyContext,1);
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

void maker_dialog_property_context_free(MakerDialogPropertyContext *ctx){
    maker_dialog_value_free(ctx->value);
    if (ctx->spec->flags & MAKER_DIALOG_PROPERTY_FLAG_CAN_FREE){
	maker_dialog_property_spec_free(ctx->spec);
    }
    g_free(ctx);
}

static void  _maker_dialog_property_context_free_wrap(gpointer obj){
    maker_dialog_property_context_free((MakerDialogPropertyContext *) obj);
}

const gchar *maker_dialog_property_get_default_string(MakerDialogPropertySpec *spec){
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

gboolean maker_dialog_property_is_default(MakerDialogPropertyContext *ctx){
    MkdgValue *mValue=maker_dialog_property_get_default(ctx->spec);
    gboolean result=(maker_dialog_value_compare(ctx->value, mValue, NULL)==0)? TRUE: FALSE;
    maker_dialog_value_free(mValue);
    return result;
}

MkdgValue *maker_dialog_property_get_default(MakerDialogPropertySpec *spec){
    MkdgValue *mValue=maker_dialog_value_new(spec->valueType, NULL);
    return maker_dialog_value_from_string(mValue, maker_dialog_property_get_default_string(spec), spec->parseOption);
}

gboolean maker_dialog_property_set_default(MakerDialogPropertyContext *ctx){
    MkdgValue *mValue=maker_dialog_property_get_default(ctx->spec);
    gboolean result=FALSE;
    if (mValue){
	maker_dialog_property_set_value_fast(ctx, mValue, -2);
	result=TRUE;
    }
    maker_dialog_value_free(mValue);
    return result;
}

static void maker_dialog_property_context_update_index(MakerDialogPropertyContext *ctx){
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

void maker_dialog_property_set_value_fast(MakerDialogPropertyContext *ctx, MkdgValue *value, gint valueIndexCtl){
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

gboolean maker_dialog_property_from_string(MakerDialogPropertyContext *ctx, const gchar *str){
    MkdgValue *mValue=maker_dialog_value_new(ctx->spec->valueType, NULL);
    if ( maker_dialog_value_from_string(mValue, str, ctx->spec->parseOption)){
	maker_dialog_property_set_value_fast(ctx, mValue, -2);
	maker_dialog_value_free(mValue);
	return TRUE;
    }
    maker_dialog_value_free(mValue);
    return FALSE;
}

gchar *maker_dialog_property_to_string(MakerDialogPropertyContext *ctx){
    gchar *str=maker_dialog_value_to_string(ctx->value, ctx->spec->toStringFormat);
    return str;
}

static gboolean maker_dialog_eval_control_rule(MakerDialogPropertyContext *ctx, MakerDialogControlRule *rule){
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

void maker_dialog_property_foreach_control_rule(MakerDialogPropertyContext *ctx, MakerDialogPropertyEachControlRule func, gpointer userData){
    if (ctx->spec->rules){
	MakerDialogControlRule *rule=ctx->spec->rules;
	while (rule->key!=NULL){
	    gboolean ret=maker_dialog_eval_control_rule(ctx, rule);
	    MakerDialogPropertyContext *refCtx=maker_dialog_get_property_context(ctx->mDialog, rule->key);
	    MakerDialogWidgetControl control=MAKER_DIALOG_WIDGET_CONTROL_NOTHING;
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

MakerDialogPropertyTable* maker_dialog_property_table_new(){
    return g_hash_table_new_full(g_str_hash,g_str_equal,NULL, _maker_dialog_property_context_free_wrap);
}

void maker_dialog_property_table_insert(MakerDialogPropertyTable *hTable, const MakerDialogPropertyContext *ctx){
    g_hash_table_insert(hTable, (gpointer) ctx->spec->key, (gpointer) ctx);
}

MakerDialogPropertyContext *maker_dialog_property_table_lookup(MakerDialogPropertyTable *hTable, const gchar *key){
    return (MakerDialogPropertyContext *) g_hash_table_lookup((gpointer) hTable,(gpointer) key);
}

MkdgValue *maker_dialog_property_table_lookup_value(MakerDialogPropertyTable *hTable, const gchar *key){
    MakerDialogPropertyContext *ctx=maker_dialog_property_table_lookup(hTable, key);
    return  ctx->value;
}

void maker_dialog_property_table_destroy (MakerDialogPropertyTable *hTable){
    g_hash_table_destroy(hTable);
}

void maker_dialog_foreach_property(MakerDialog* mDialog, GHFunc func, gpointer userData){
    g_hash_table_foreach(mDialog->propertyTable, func, userData);
}

/*=== Start enumeration and flags ===*/
static MakerDialogIdPair mkdgRelationData[]={
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

MakerDialogRelation maker_dialog_relation_parse(const gchar *str){
    return maker_dialog_id_parse(mkdgRelationData, str, FALSE);
}

static MakerDialogIdPair mkdgSpecFlagData[]={
    {"FIXED_SET",		MAKER_DIALOG_PROPERTY_FLAG_FIXED_SET},
    {"PREFER_RADIO_BUTTONS",	MAKER_DIALOG_PROPERTY_FLAG_PREFER_RADIO_BUTTONS},
    {NULL,			0},
};

MakerDialogPropertyFlags maker_dialog_property_flags_parse(const gchar *str){
    return maker_dialog_flag_parse(mkdgSpecFlagData, str, FALSE);
}


/*=== End enumeration and flags ===*/

