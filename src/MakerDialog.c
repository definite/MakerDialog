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
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include "MakerDialog.h"
extern gint makerDialogVerboseLevel;

static MakerDialog *maker_dialog_new(){
    MakerDialog *mDialog=g_new(MakerDialog,1);
    mDialog->title=NULL;
    mDialog->buttonSpecs=NULL;
    mDialog->propertyTable=maker_dialog_property_table_new();
    mDialog->pageRoot=g_node_new(mDialog);
    mDialog->maxSizeInPixel.width=-1;
    mDialog->maxSizeInPixel.height=-1;
    mDialog->maxSizeInChar.width=-1;
    mDialog->maxSizeInChar.height=-1;
    mDialog->labelAlignment.x=0.0f;
    mDialog->labelAlignment.y=0.5f;
    mDialog->componentAlignment.x=0.0f;
    mDialog->componentAlignment.y=0.5f;
    if (getenv(MAKER_DLALOG_VERBOSE_ENV)){
	makerDialogVerboseLevel=atoi(getenv(MAKER_DLALOG_VERBOSE_ENV));
    }
    mDialog->ui=NULL;
    mDialog->config=NULL;
    mDialog->userData=NULL;
    return mDialog;
}

MakerDialog *maker_dialog_init(const gchar *title, MakerDialogButtonSpec *buttonSpecs){
    MakerDialog *mDialog=maker_dialog_new();
    mDialog->title=g_strdup(title);
    mDialog->buttonSpecs=buttonSpecs;
    return mDialog;
}

MakerDialog *maker_dialog_new_from_key_file(const gchar *filename, MakerDialogError **error){
    GKeyFile *keyFile=g_key_file_new();
    MakerDialogError *cfgErr=NULL;
    MakerDialog *mDialog=NULL;
    if (!g_key_file_load_from_file(keyFile, filename, G_KEY_FILE_NONE, &cfgErr)){
	maker_dialog_error_handle(cfgErr, error);
	goto FINAL_LOAD_FROM_KEYFILE;
    }
    mDialog=maker_dialog_new();
    maker_dialog_new_from_key_file_section_main(mDialog, keyFile, &cfgErr);
    maker_dialog_error_handle(cfgErr,error);
    maker_dialog_new_from_key_file_section_keys(mDialog, keyFile, &cfgErr);
    maker_dialog_error_handle(cfgErr,error);
    mDialog->flags|= MAKER_DIALOG_FLAG_FREE_ALL;
FINAL_LOAD_FROM_KEYFILE:
    g_key_file_free(keyFile);

    return mDialog;
}

static GNode *maker_dialog_prepare_page_node(MakerDialog *mDialog, const gchar *pageName){
    const gchar *pageName_tmp=(pageName)? pageName : MAKER_DIALOG_PAGE_UNNAMED;
    GNode *result=maker_dialog_find_page_node(mDialog, (gpointer) pageName_tmp);
    if (!result){
	result=g_node_new((gpointer) pageName_tmp);
	g_node_append(mDialog->pageRoot,result);
    }
    return result;
}

static GNode *maker_dialog_prepare_group_node(MakerDialog *mDialog, const gchar *pageName, const gchar *groupName){
    const gchar *groupName_tmp=(groupName)? groupName : MAKER_DIALOG_GROUP_UNNAMED;
    GNode *pageNode=maker_dialog_prepare_page_node(mDialog, pageName);
    GNode *result=maker_dialog_find_group_node(mDialog, pageName, (gpointer) groupName_tmp);
    if (!result){
	result=g_node_new((gpointer) groupName_tmp);
	g_node_append(pageNode,result);
    }
    return result;
}

void maker_dialog_add_property(MakerDialog *mDialog, MakerDialogPropertyContext *ctx){
    MAKER_DIALOG_DEBUG_MSG(2, "[I2] add_property( , %s)",ctx->spec->key);
    maker_dialog_property_table_insert(mDialog->propertyTable, ctx);
    GNode *propGroupNode=maker_dialog_prepare_group_node(mDialog, ctx->spec->pageName, ctx->spec->groupName);
    GNode *propKeyNode=g_node_new((gpointer) ctx);
    g_node_append(propGroupNode,propKeyNode);
//    maker_dialog_property_get_default(ctx->spec);
    ctx->mDialog=mDialog;
}

void maker_dialog_destroy(MakerDialog *mDialog){
    MAKER_DIALOG_DEBUG_MSG(3, "[I3] destroy()");
    if (mDialog->ui){
	maker_dialog_ui_destroy(mDialog->ui);
    }
    if (mDialog->config){
        maker_dialog_config_free(mDialog->config);
    }

    g_node_destroy(mDialog->pageRoot);
    maker_dialog_property_table_destroy(mDialog->propertyTable);
    g_free(mDialog->title);
    if (mDialog->flags & MAKER_DIALOG_FLAG_FREE_ALL){
	/* Free button specs */
	gint i;
	for(i=0;mDialog->buttonSpecs[i].responseId!=MAKER_DIALOG_RESPONSE_NIL;i++){
	    g_free((gchar *) mDialog->buttonSpecs[i].buttonText);
	}
	g_free(mDialog->buttonSpecs);
    }
    g_free(mDialog);
}

MkdgValue *maker_dialog_get_value(MakerDialog *mDialog, const gchar *key){
    MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(mDialog, key);
    if (ctx->flags & MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_HAS_VALUE)
	return NULL;
    return ctx->value;
}

MakerDialogPropertyContext *maker_dialog_get_property_context(MakerDialog *mDialog, const gchar *key){
    return maker_dialog_property_table_lookup(mDialog->propertyTable, key);
}

gboolean maker_dialog_apply_value(MakerDialog *mDialog, const gchar *key){
    MAKER_DIALOG_DEBUG_MSG(2,"[I2] apply_value( , %s)",key);
    MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(mDialog, key);

    gboolean ret=TRUE;
    if (ctx->validateFunc && (!ctx->validateFunc(ctx->spec, ctx->value))){
	/* Value is invalid. */
	ret=FALSE;
    }
    if (ret && ctx->applyFunc){
	ctx->applyFunc(ctx,ctx->value);
	ctx->flags &= ~MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED;
    }else{
	ret=FALSE;
    }

    return ret;
}

gboolean maker_dialog_set_value(MakerDialog *mDialog, const gchar *key, MkdgValue *value){
    MAKER_DIALOG_DEBUG_MSG(2,"[I2] set_value( , %s, )", key);
    MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(mDialog, key);
    if (!value){
	return maker_dialog_property_set_default(ctx);
    }
    if (MAKER_DIALOG_DEBUG_RUN(3)){
	gchar *str=maker_dialog_value_to_string(value, ctx->spec->toStringFormat);
	g_debug("[I3] set_value( , %s, ) value=%s",ctx->spec->key, str);
	g_free(str);
    }
    gboolean ret=TRUE;
    if (ctx->validateFunc && (!ctx->validateFunc(ctx->spec, value))){
	/* Value is invalid. */
	ret=FALSE;
    }
    if (ret){
	if  (mDialog->ui){
	    if (mDialog->ui->toolkitInterface->widget_set_value){
		mDialog->ui->toolkitInterface->widget_set_value(mDialog->ui, ctx->spec->key, value);
	    }else{
		ret=FALSE;
	    }
	}
	maker_dialog_property_set_value_fast(ctx, value, -2);
    }
    return ret;
}

