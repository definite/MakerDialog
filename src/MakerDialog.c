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

MakerDialog *maker_dialog_init(const gchar *title,
	guint buttonSpecCount, MakerDialogButtonSpec *buttonSpecs){
    MakerDialog *mDialog=g_new(MakerDialog,1);
    mDialog->title=g_strdup(title);
    mDialog->buttonSpecCount=buttonSpecCount;
    mDialog->buttonSpecs=buttonSpecs;
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
    mDialog->ipc=NULL;
    mDialog->userData=NULL;
    return mDialog;
}

static GNode *maker_dialog_prepare_page_node(MakerDialog *mDialog, const gchar *pageName){
    const gchar *pageName_tmp=(pageName)? pageName : MAKER_DIALOG_CONFIG_NO_PAGE;
    GNode *result=maker_dialog_find_page_node(mDialog, (gpointer) pageName_tmp);
    if (!result){
	result=g_node_new((gpointer) pageName_tmp);
	g_node_append(mDialog->pageRoot,result);
    }
    return result;
}

void maker_dialog_add_property(MakerDialog *mDialog, MakerDialogPropertyContext *ctx){
    MAKER_DIALOG_DEBUG_MSG(3, "[I3] add_property( , %s)",ctx->spec->key);
    maker_dialog_property_table_insert(mDialog->propertyTable, ctx);
    GNode *propPageNode=maker_dialog_prepare_page_node(mDialog, ctx->spec->pageName);
    GNode *propKeyNode=g_node_new((gpointer) ctx->spec->key);
    g_node_append(propPageNode,propKeyNode);
    maker_dialog_property_get_default(ctx->spec);
    ctx->mDialog=mDialog;
}

void maker_dialog_destroy(MakerDialog *mDialog){
    if (mDialog->ui){
	maker_dialog_ui_destroy(mDialog);
    }
    if (mDialog->config){
        maker_dialog_config_free(mDialog->config);
    }

    maker_dialog_property_table_destroy(mDialog->propertyTable);
    g_node_destroy(mDialog->pageRoot);
    g_free(mDialog->title);
    g_free(mDialog);
}

GValue *maker_dialog_get_value(MakerDialog *mDialog, const gchar *key){
    MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(mDialog, key);
    if (ctx->flags & MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_HAS_VALUE)
	return NULL;
    return &ctx->value;
}

MakerDialogPropertyContext *maker_dialog_get_property_context(MakerDialog *mDialog, const gchar *key){
    return maker_dialog_property_table_lookup(mDialog->propertyTable, key);
}

gboolean maker_dialog_apply_value(MakerDialog *mDialog, const gchar *key){
    MAKER_DIALOG_DEBUG_MSG(2,"[I2] apply_value( , %s)",key);
    MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(mDialog, key);

    gboolean ret=TRUE;
    if (ctx->validateFunc && (!ctx->validateFunc(ctx->spec, &ctx->value))){
	/* Value is invalid. */
	ret=FALSE;
    }
    if (ret && ctx->applyFunc){
	ctx->applyFunc(ctx,&ctx->value);
	ctx->flags &= ~MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED;
    }else{
	ret=FALSE;
    }

    return ret;
}

gboolean maker_dialog_set_value(MakerDialog *mDialog, const gchar *key, GValue *value){
    MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(mDialog, key);
    if (!value){
	return maker_dialog_property_set_default(ctx);
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

GNode *maker_dialog_find_page_node(MakerDialog *mDialog, const gchar *pageName){
    const gchar *pageName_tmp=(pageName)? pageName : MAKER_DIALOG_CONFIG_NO_PAGE;
    return g_node_find(mDialog->pageRoot, G_POST_ORDER, G_TRAVERSE_NON_LEAVES, (gpointer) pageName_tmp);
}


