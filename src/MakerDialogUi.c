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
#include "MakerDialog.h"

static void maker_dialog_ui_destroy_hook(gpointer dlgUi){
    maker_dialog_ui_destroy((MakerDialogUi *) dlgUi);
}

static void maker_dialog_ui_add_destroy_hook(MakerDialogUi *dlgUi){
    GHook *hook=g_hook_alloc(dlgUi->mDialog->destroyHookList);
    hook->data=dlgUi;
    hook->func=maker_dialog_ui_destroy_hook;
    g_hook_append (dlgUi->mDialog->destroyHookList, hook);

}

MakerDialogUi *maker_dialog_ui_init(MakerDialog *mDialog, MakerDialogToolkitHandler *toolkitHandler){
    g_assert(mDialog);
    g_assert(toolkitHandler);
    MakerDialogUi *dlgUi=g_new(MakerDialogUi,1);
    dlgUi->toolkitHandler=toolkitHandler;
    dlgUi->dialogObj=NULL;
    dlgUi->mDialog=mDialog;
    maker_dialog_ui_add_destroy_hook(dlgUi);
    return dlgUi;
}

gboolean maker_dialog_ui_construct(MakerDialogUi *dlgUi, gpointer parentWindow, gboolean modal){
    g_assert(dlgUi->toolkitHandler->dialog_construct);
    dlgUi->dialogObj=dlgUi->toolkitHandler->dialog_construct(dlgUi, parentWindow, modal);
    if (dlgUi->dialogObj)
	return TRUE;
    return FALSE;
}

gint maker_dialog_ui_run(MakerDialogUi *dlgUi){
    g_assert(dlgUi->toolkitHandler->dialog_run);
    return dlgUi->toolkitHandler->dialog_run(dlgUi);
}

void maker_dialog_ui_show(MakerDialogUi *dlgUi){
    g_assert(dlgUi->toolkitHandler->dialog_show);
    dlgUi->toolkitHandler->dialog_show(dlgUi);
}

void maker_dialog_ui_hide(MakerDialogUi *dlgUi){
    g_assert(dlgUi->toolkitHandler->dialog_hide);
    dlgUi->toolkitHandler->dialog_hide(dlgUi);
}

void maker_dialog_ui_destroy(MakerDialogUi *dlgUi){
    g_assert(dlgUi->dialogObj);
    g_assert(dlgUi->toolkitHandler->dialog_destroy);
    dlgUi->toolkitHandler->dialog_destroy(dlgUi);
    g_free(dlgUi->dialogObj);
}

GValue *maker_dialog_ui_get_value(MakerDialogUi *dlgUi, const gchar *key){
    return maker_dialog_property_table_lookup_value(dlgUi->mDialog->propertyTable, key);
}

gboolean maker_dialog_ui_apply_value(MakerDialogUi *dlgUi, const gchar *key){
    MakerDialogPropertyContext *ctx=maker_dialog_property_table_lookup(dlgUi->mDialog->propertyTable, key);

    gboolean ret=TRUE;
    if (ctx->validateFunc && (!ctx->validateFunc(ctx->spec, &ctx->value))){
	/* Value is invalid. */
	ret=FALSE;
    }
    if (ret && ctx->applyFunc){
	ctx->applyFunc(ctx,&ctx->value);
    }else{
	ret=FALSE;
    }

    return ret;
}

gboolean maker_dialog_ui_set_value(MakerDialogUi *dlgUi, const gchar *key, GValue *value){
    MakerDialogPropertyContext *ctx=maker_dialog_property_table_lookup(dlgUi->mDialog->propertyTable, key);

    gboolean ret=TRUE;
    if (ctx->validateFunc && (!ctx->validateFunc(ctx->spec, &ctx->value))){
	/* Value is invalid. */
	ret=FALSE;
    }
    if (ret && dlgUi->toolkitHandler->component_set_value){
	dlgUi->toolkitHandler->component_set_value(dlgUi, ctx->spec->key, value);
	g_value_copy(value,&ctx->value);
    }else{
	ret=FALSE;
    }
    return ret;
}

gboolean maker_dialog_ui_update_value(MakerDialogUi *dlgUi, const gchar *key){
    g_assert(dlgUi->toolkitHandler->component_get_value);
    MakerDialogPropertyContext *ctx=maker_dialog_property_table_lookup(dlgUi->mDialog->propertyTable, key);

    GValue *value=dlgUi->toolkitHandler->component_get_value(dlgUi,key);
    gboolean ret=TRUE;
    if (ctx->validateFunc && (!ctx->validateFunc(ctx->spec, value))){
	/* Value is invalid. */
	ret=FALSE;
    }else{
	g_value_copy(value,&ctx->value);
    }
    g_value_unset(value);
    g_free(value);

    return ret;
}

