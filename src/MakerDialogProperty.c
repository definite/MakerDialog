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

MakerDialogPropertySpec *maker_dialog_property_spec_new(const gchar *key, GType valueType){
    return maker_dialog_property_spec_new_full(key, valueType,
	    NULL, NULL,
	    NULL, NULL,
	    0.0, 0.0, 0.0, 0,
	    MAKER_DIALOG_PROPERTY_FLAG_CAN_FREE,  NULL, NULL, NULL,
	    NULL, NULL);
}

MakerDialogPropertySpec *maker_dialog_property_spec_new_full(const gchar *key, GType valueType,
	const gchar *defaultValue, const gchar **validValues,
	const gchar *parseOption, const char *toStringFormat,
	gdouble min, gdouble max, gdouble step, gint decimalDigits,
	MakerDialogPropertyFlags propertyFlags,
	const gchar *pageName, const gchar *label, const gchar *translationContext,
	const gchar *tooltip, gpointer userData){
    MakerDialogPropertySpec *spec=g_new(MakerDialogPropertySpec,1);
    if (spec){
	spec->key=key;
	spec->valueType=valueType;
	spec->defaultValue=defaultValue;
	spec->validValues=validValues;
	spec->parseOption=parseOption;
	spec->toStringFormat=toStringFormat;

	spec->min=min;
	spec->max=max;
	spec->step=step;
	spec->decimalDigits=decimalDigits;

	spec->flags=propertyFlags | MAKER_DIALOG_PROPERTY_FLAG_CAN_FREE;
	spec->pageName=pageName;
	spec->label=label;
	spec->translationContext=translationContext;
	spec->tooltip=tooltip;

	spec->userData=userData;
    }
    return spec;
}

void maker_dialog_property_spec_free(MakerDialogPropertySpec *spec){
    g_free(spec);
}

MakerDialogPropertyContext *maker_dialog_property_context_new(
	MakerDialogPropertySpec *spec,gpointer obj){
    return maker_dialog_property_context_new_full(spec, obj, NULL, NULL);
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
	memset(&ctx->value, 0, sizeof(GValue));
	g_value_init(&ctx->value,spec->valueType);
	ctx->validateFunc=validateFunc;
	ctx->applyFunc=applyFunc;
	ctx->mDialog=NULL;
    }
    return ctx;
}

void maker_dialog_property_context_free(MakerDialogPropertyContext *ctx){
    g_value_unset (&(ctx->value));
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
		index=maker_dialog_find_string(spec->defaultValue,spec->validValues,-1);
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
	    if (spec->flags & MAKER_DIALOG_PROPERTY_FLAG_FIXED_SET){
		return NULL;
	    }
	    return spec->validValues[0];
	}
    }
    return NULL;
}

gboolean maker_dialog_property_is_default(MakerDialogPropertyContext *ctx){
    GValue defValue={0};
    g_value_init(&defValue,ctx->spec->valueType);
    maker_dialog_value_from_string(&defValue, maker_dialog_property_get_default_string(ctx->spec), ctx->spec->parseOption);
    gboolean result=(maker_dialog_value_compare(&ctx->value, &defValue, NULL)==0)? TRUE: FALSE;
    g_value_unset(&defValue);
    return result;
}

GValue *maker_dialog_property_get_default(MakerDialogPropertySpec *spec){
    GValue *value=g_new0(GValue, 1);
    g_value_init(value, spec->valueType);
    maker_dialog_value_from_string(value, maker_dialog_property_get_default_string(spec), spec->parseOption);
    return value;
}

void maker_dialog_property_set_default(MakerDialogPropertyContext *ctx){
    GValue *value=maker_dialog_property_get_default(ctx->spec);
    maker_dialog_property_set_value_fast(ctx, value, -1);
    g_value_unset(value);
    g_free(value);
}

static gint maker_dialog_find_value_in_strings(GValue *value, const gchar **strings, const gchar *parseOption){
    gint i;
    GValue val={0};
    g_value_init(&val, G_VALUE_TYPE(value));
    for (i=0;strings[i]!=NULL;i++){
	maker_dialog_value_from_string(&val, strings[i], parseOption);
	if (maker_dialog_value_compare(&val, value, NULL)==0){
	    g_value_unset(&val);
	    return i;
	}
	g_value_reset(&val);
    }
    g_value_unset(&val);
    return -1;
}

void maker_dialog_property_set_value_fast(MakerDialogPropertyContext *ctx, GValue *value, gint valueIndexCtl){
    g_value_copy(value,&ctx->value);
    ctx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_HAS_VALUE | MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED;
    if (ctx->spec->validValues && valueIndexCtl!=-3){
	if (valueIndexCtl==-2){
	    ctx->valueIndex=maker_dialog_find_value_in_strings(&ctx->value, ctx->spec->validValues, ctx->spec->parseOption);
	}else{
	    ctx->valueIndex=valueIndexCtl;
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

GValue *maker_dialog_property_table_lookup_value(MakerDialogPropertyTable *hTable, const gchar *key){
    MakerDialogPropertyContext *ctx=maker_dialog_property_table_lookup(hTable, key);
    return  &ctx->value;
}

void maker_dialog_property_table_destroy (MakerDialogPropertyTable *hTable){
    g_hash_table_destroy(hTable);
}

void maker_dialog_foreach_property(MakerDialog* mDialog, GHFunc func, gpointer userData){
    g_hash_table_foreach(mDialog->propertyTable, func, userData);
}

void maker_dialog_page_foreach_property(MakerDialog* mDialog, const gchar *pageName, MakerDialogPropertyCallbackFunc  func, gpointer userData){
    GNode *pageNode=maker_dialog_find_page_node(mDialog, pageName);
    g_assert(pageNode);
    GNode *keyNode=NULL;
    for(keyNode=g_node_first_child(pageNode);keyNode!=NULL; keyNode=g_node_next_sibling(keyNode)){
	MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(mDialog, (gchar *) keyNode->data);
	func(mDialog, ctx, userData);
    }
}

void maker_dialog_pages_foreach_property(MakerDialog* mDialog, const gchar **pageNames, MakerDialogPropertyCallbackFunc func, gpointer userData){
    if (pageNames){
	gsize i;
	for(i=0;pageNames[i]!=NULL;i++){
	    maker_dialog_page_foreach_property(mDialog, pageNames[i], func, userData);
	}
    }else{
	GNode *pageNode=NULL;
	for(pageNode=g_node_first_child(mDialog->pageRoot);pageNode!=NULL; pageNode=g_node_next_sibling(pageNode)){
	    maker_dialog_page_foreach_property(mDialog, (gchar *) pageNode->data, func, userData);
	}
    }
}


