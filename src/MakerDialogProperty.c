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
	    0.0, 0.0, 0.0, 0,
	    MAKER_DIALOG_PROPERTY_FLAG_CAN_FREE,  NULL, NULL, NULL,
	    NULL, NULL);
}

MakerDialogPropertySpec *maker_dialog_property_spec_new_full(const gchar *key, GType valueType,
	const gchar *defaultValue, const gchar **validValues,
	gdouble min, gdouble max, gdouble step, gint decimalDigits,
	MakerDialogPropertyFlags propertyFlags,
	const gchar *pageName, const gchar *label, const gchar *translationContext,
	const gchar *tooltip, gpointer extraData){
    MakerDialogPropertySpec *spec=g_new(MakerDialogPropertySpec,1);
    if (spec){
	spec->key=key;
	spec->valueType=valueType;
	spec->defaultValue=defaultValue;
	spec->validValues=validValues;
	spec->min=min;
	spec->max=max;
	spec->step=step;
	spec->decimalDigits=decimalDigits;

	spec->flags=propertyFlags | MAKER_DIALOG_PROPERTY_FLAG_CAN_FREE;
	spec->pageName=pageName;
	spec->label=label;
	spec->translationContext=translationContext;
	spec->tooltip=tooltip;

	spec->extraData=extraData;
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
	MakerDialogPropertySpec *spec,	gpointer obj,
	MakerDialogValidateCallbackFunc validateFunc,
	MakerDialogApplyCallbackFunc applyFunc){
    MakerDialogPropertyContext *ctx=g_new(MakerDialogPropertyContext,1);
    if (ctx){
	ctx->flags=0;
	ctx->spec=spec;
	ctx->obj=obj;
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

