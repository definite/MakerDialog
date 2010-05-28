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
 *  along with Mkdg.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include "MakerDialog.h"
extern gint makerDialogVerboseLevel;

Mkdg *mkdg_new(){
    Mkdg *mDialog=g_new(Mkdg,1);
    mDialog->title=NULL;
    mDialog->buttonSpecs=NULL;
    mDialog->propertyTable=mkdg_property_table_new();
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
    mDialog->argc=0;
    mDialog->argv=NULL;
    return mDialog;
}

Mkdg *mkdg_init(const gchar *title, MkdgButtonSpec *buttonSpecs){
    Mkdg *mDialog=mkdg_new();
    mDialog->title=g_strdup(title);
    mDialog->buttonSpecs=buttonSpecs;
    return mDialog;
}

void mkdg_set_args(Mkdg *mDialog, gint argc, gchar **argv){
    mDialog->argc=argc;
    mDialog->argv=argv;
}

static GNode *mkdg_prepare_page_node(Mkdg *mDialog, const gchar *pageName){
    const gchar *pageName_tmp=(pageName)? pageName : MKDG_PAGE_UNNAMED;
    GNode *result=mkdg_find_page_node(mDialog, (gpointer) pageName_tmp);
    if (!result){
	result=g_node_new((gpointer) pageName_tmp);
	g_node_append(mDialog->pageRoot,result);
    }
    return result;
}

static GNode *mkdg_prepare_group_node(Mkdg *mDialog, const gchar *pageName, const gchar *groupName){
    const gchar *groupName_tmp=(groupName)? groupName : MKDG_GROUP_UNNAMED;
    GNode *pageNode=mkdg_prepare_page_node(mDialog, pageName);
    GNode *result=mkdg_find_group_node(mDialog, pageName, (gpointer) groupName_tmp);
    if (!result){
	result=g_node_new((gpointer) groupName_tmp);
	g_node_append(pageNode,result);
    }
    return result;
}

void mkdg_add_property(Mkdg *mDialog, MkdgPropertyContext *ctx){
    MKDG_DEBUG_MSG(2, "[I2] add_property( , %s)",ctx->spec->key);
    mkdg_property_table_insert(mDialog->propertyTable, ctx);
    GNode *propGroupNode=mkdg_prepare_group_node(mDialog, ctx->spec->pageName, ctx->spec->groupName);
    GNode *propKeyNode=g_node_new((gpointer) ctx);
    g_node_append(propGroupNode,propKeyNode);
//    mkdg_property_get_default(ctx->spec);
    ctx->mDialog=mDialog;
}

void mkdg_destroy(Mkdg *mDialog){
    MKDG_DEBUG_MSG(3, "[I3] destroy()");
    if (mDialog->ui){
	mkdg_ui_destroy(mDialog->ui);
    }
    if (mDialog->config){
        mkdg_config_free(mDialog->config);
    }

    g_node_destroy(mDialog->pageRoot);
    mkdg_property_table_destroy(mDialog->propertyTable);
    g_free(mDialog->title);
    if (mDialog->flags & MKDG_FLAG_FREE_ALL){
	/* Free button specs */
	gint i;
	for(i=0;mDialog->buttonSpecs[i].responseId!=MKDG_RESPONSE_NIL;i++){
	    g_free((gchar *) mDialog->buttonSpecs[i].buttonText);
	}
	g_free(mDialog->buttonSpecs);
    }
    g_free(mDialog);
}

MkdgValue *mkdg_get_value(Mkdg *mDialog, const gchar *key){
    MkdgPropertyContext *ctx=mkdg_get_property_context(mDialog, key);
    if (ctx->flags & MKDG_PROPERTY_CONTEXT_FLAG_HAS_VALUE)
	return NULL;
    return ctx->value;
}

MkdgPropertyContext *mkdg_get_property_context(Mkdg *mDialog, const gchar *key){
    return mkdg_property_table_lookup(mDialog->propertyTable, key);
}

gboolean mkdg_apply_value(Mkdg *mDialog, const gchar *key){
    MKDG_DEBUG_MSG(2,"[I2] apply_value( , %s)",key);
    MkdgPropertyContext *ctx=mkdg_get_property_context(mDialog, key);

    gboolean ret=TRUE;
    if (ctx->validateFunc && (!ctx->validateFunc(ctx->spec, ctx->value))){
	/* Value is invalid. */
	ret=FALSE;
    }
    if (ret && ctx->applyFunc){
	ctx->applyFunc(ctx,ctx->value);
	ctx->flags &= ~MKDG_PROPERTY_CONTEXT_FLAG_UNAPPLIED;
    }else{
	ret=FALSE;
    }

    return ret;
}

gboolean mkdg_set_value(Mkdg *mDialog, const gchar *key, MkdgValue *value){
    MKDG_DEBUG_MSG(2,"[I2] set_value( , %s, )", key);
    MkdgPropertyContext *ctx=mkdg_get_property_context(mDialog, key);
    if (!value){
	return mkdg_property_set_default(ctx);
    }
    if (MKDG_DEBUG_RUN(3)){
	gchar *str=mkdg_value_to_string(value, ctx->spec->toStringFormat);
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
	mkdg_property_set_value_fast(ctx, value, -2);
    }
    return ret;
}


