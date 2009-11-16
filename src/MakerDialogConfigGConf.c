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
#include <stdio.h>
#include <unistd.h>
#include <glib/gstdio.h>
#include <
#include "MakerDialog.h"
#include "MakerDialogConfigGConf.h"

/*=== Start Config interface callbacks ===*/
static gpointer maker_dialog_config_gconf_create(
	MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, GError **error){
    if (!configSet->userData){
	configSet->userData=gconf_client_get_default();
    }
    configFile->filePointer=configSet->userData;
    return configSet->userData;
}

static gpointer  maker_dialog_config_gconf_open(
	MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, GError **error){
    if (!configSet->userData){
	GConfClient *client=gconf_client_get_default();
	configSet->userData=client;
	gconf_client_preload(client, configFile->path,GCONF_CLIENT_PRELOAD_RECURSIVE,error);
    }
    configFile->filePointer=configSet->userData;
    return configSet->userData;
}

static gboolean maker_dialog_config_gconf_close(
	MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, GError **error){
    configFile->filePointer=NULL;
    if (configSet->userData){
	gconf_client_clear_cache((GConfClient *) configSet->userData);
    }
    return TRUE;
}

static void maker_dialog_slist_free_individual(gpointer data, gpointer userData){
    g_free(data);
}

static gboolean maker_dialog_config_gconf_preload_to_buffer(
	MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, const gchar *pageName, GError **error){
    GString *strBuf=g_string_new(configFile->path);
    g_string_append_printf(strBuf,"/%s/",pageName);
    guint pathLen=strBuf->len;
    GConfClient *client=(GConfClient *) configSet->userData;
    GSList *sListHead=gconf_client_all_entries(client,strBuf->str, error);
    GSList *sList;
    GError *cfgErr=NULL; *cfgErr_last=NULL;
    gboolean clean=TRUE;
    for(sList=sListHead;sList!=NULL; sList=g_slist_next(sList)){
	gchar *key=(gchar *)sList->data;
	GConfValue *cValue=gconf_client_get(client, key, &cfgErr);
	MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(configSet->mDialog,key);
	if (!ctx){
	    clean=FALSE;
	    if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR ){
		cfgErr=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_INVALID_KEY, "config_gconf_preload_to_buffer()");
		goto GCONF_LOAD_BUF_END;
	    }else{
		if (cfgErr_last){
		    maker_dialog_config_error_print(cfgErr_last);
		    g_error_free(cfgErr_last);
		}
		cfgErr_last=cfgErr;
		continue;
	    }
	}
	MkdgValue *mValue=maker_dialog_value_new(ctx->spec->valueType, NULL);
	switch(ctx->spec->valueType){
	    case MKDG_TYPE_BOOLEAN:
		g_value_set_boolean(mValue->value, gconf_value_get_boolean(cValue));
	        break;
	    case MKDG_TYPE_INT:
	    case MKDG_TYPE_UINT:
	    case MKDG_TYPE_LONG:
	    case MKDG_TYPE_ULONG:
		maker_dialog_g_value_set_number(mValue->value, (gdouble) gconf_value_get_int(cValue));
		break;
	    case MKDG_TYPE_FLOAT:
	    case MKDG_TYPE_DOUBLE:
		maker_dialog_g_value_set_number(mValue->value, gconf_value_get_float(cValue));
		break;
	    case MKDG_TYPE_STRING:
	    case MKDG_TYPE_COLOR:
		maker_dialog_value_from_string(mValue, gconf_value_get_string(cValue), NULL);
		break;
	}
	g_hash_table_insert(configSet->configBuf, ctx->spec->key, mValue);
    }
GCONF_LOAD_BUF_END:
    if (cfgErr){
	if (error){
	    *error=cfgErr;
	}else{
	    maker_dialog_config_error_print(cfgErr);
	    g_error_free(cfgErr);
	}
	return FALSE;
    }
    g_slist_foreach(sListhead,maker_dialog_slist_free_individual,NULL);
    g_slist_free(sListHead);
    return result;
}

static gboolean maker_dialog_config_gconf_preload(
	MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, const gchar *pageName, GError **error){
    gboolean clean=TRUE,ret;
    GError *cfgErr=NULL, *cfgErr_last=NULL;
    if (pageName){
	clean=maker_dialog_config_gconf_preload_to_buffer(configSet, configFile, pageName, &cfgErr);
    }else{
	/* All pages in the configuration set */
	GConfClient *client=(GConfClient *) configSet->userData;
	GSList *sListHead=gconf_client_all_dirs(client,strBuf->str,&cfgErr);
	GSList *sList=sListHead;
	if (!cfgErr){
	    gchar *path;
	    for(;sList!=NULL; sList=g_slist_next(sList)){
		path=(gchar *)sList->data;
		pageNameStr=g_path_get_basename(path);
		ret=maker_dialog_config_gconf_preload_to_buffer(configSet, configFile, pageNameStr, &cfgErr);
		g_free(pageNameStr);
		if (!ret){
		    clean=FALSE;
		    if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR ){
			break;
		    }else if (cfgErr_last){
			maker_dialog_config_error_print(cfgErr_last);
			g_error_free(cfgErr_last);
		    }
		    cfgErr_last=cfgErr;
		}
	    }
	}
	/* Free sList */
	g_slist_foreach(sListhead,maker_dialog_slist_free_individual,NULL);
	g_slist_free(sListHead);
FILE_LOAD_END:
    if (cfgErr){
	if (error){
	    *error=cfgErr;
	}else{
	    maker_dialog_config_error_print(cfgErr);
	    g_error_free(cfgErr);
	}
	return FALSE;
    }
    return TRUE;
}

static gboolean maker_dialog_config_gconf_preload_to_buffer(
	MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, const gchar *pageName, GError **error){

static

static gboolean maker_dialog_config_gconf_save(MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, const gchar *pageName, GError **error){
    MAKER_DIALOG_DEBUG_MSG(3, "[I3] config_gconf_save(-,%s,%s)",configFile->path, (pageName)? pageName: "-");
    GConfClient *client=(GConfClient *) configSet->userData;
    GConfigChangeSet *cChangeSet=gconf_change_set_new();
    if (pageName){
	ret=maker_dialog_config_gconf_save_private(configSet, configFile, pageNameStr, &cfgErr);
    }else{
	/* All pages in the configuration set */
	GConfClient *client=(GConfClient *) configSet->userData;
	GSList *sListHead=gconf_client_all_dirs(client,strBuf->str,&cfgErr);
	GSList *sList=sListHead;
	if (!cfgErr){
	    gchar *path;
	    for(;sList!=NULL; sList=g_slist_next(sList)){
		path=(gchar *)sList->data;
		pageNameStr=g_path_get_basename(path);
		ret=maker_dialog_config_gconf_save_private(configSet, configFile, pageNameStr, &cfgErr);
		g_free(pageNameStr);
		if (!ret){
		    clean=FALSE;
		    if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR ){
			break;
		    }else if (cfgErr_last){
			maker_dialog_config_error_print(cfgErr_last);
			g_error_free(cfgErr_last);
		    }
		    cfgErr_last=cfgErr;
		}
	    }
	}
    if (configSet->pageNames){
	gsize i;
	for(i=0;(page=configSet->pageNames[i])!=NULL;i++){
	    maker_dialog_page_foreach_property(configSet->mDialog, page,  maker_dialog_gconf_save_private, (gpointer) cChangeSet);
	}
    }else{
	GNode *pageNode=NULL;
	for(pageNode=g_node_first_child(configSet->mDialog->pageRoot);pageNode!=NULL; pageNode=g_node_next_sibling(pageNode)){
	    page=(gchar *) pageNode->data;
	    maker_dialog_page_foreach_property(configSet->mDialog, page,  maker_dialog_gconf_save_private, (gpointer) cChangeSet);
	}
    }
    return gconf_client_commit_change_set(client, cChangeSet, TRUE, error);
}

/*=== End Config interface callbacks ===*/

MakerDialogConfigInterface makerDialogConfigGConfInterface={
    maker_dialog_config_gconf_create,
    maker_dialog_config_gconf_open,
    maker_dialog_config_gconf_close,
    maker_dialog_config_gconf_preload,
    maker_dialog_config_gconf_save
};

MakerDialogConfig *maker_dialog_config_use_gconf(MakerDialog *mDialog){
    MakerDialogConfig *config=maker_dialog_config_new(mDialog, FALSE, &makerDialogConfigGConfInterface);
    return config;
}

