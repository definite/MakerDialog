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
#include "MakerDialog.h"

static GError *convert_error_code(GError *error, const gchar *filename, const gchar *prefix){
    if (!error)
	return NULL;
    if (error->domain==MAKER_DIALOG_ERROR)
	return error;

    GError *cfgErr=NULL;
    if (error->domain==G_FILE_ERROR){
	switch(error->code){
	    case G_FILE_ERROR_ACCES:
		cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_PERMISSION_DENY, prefix);
		break;
	    default:
		cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_OTHER, prefix);
		break;
	}
    }else if (error->domain==G_KEY_FILE_ERROR){
	switch(error->code){
	    case G_KEY_FILE_ERROR_UNKNOWN_ENCODING:
	    case G_KEY_FILE_ERROR_PARSE:
		cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_INVALID_FORMAT, prefix);
		break;
	    case G_KEY_FILE_ERROR_NOT_FOUND:
		cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_NOT_FOUND, prefix);
		break;
	    default:
		cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_OTHER, prefix);
		break;
	}
    }else{
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_OTHER, prefix);
    }
    g_warning("MakerDialogConfigKeyFile: %s%s %s: domain=%s code=%d:%s",
	    (filename)? "On file: " : "",
	    (filename)? filename : "",
	    prefix, g_quark_to_string(error->domain), error->code, error->message);
    g_error_free(error);
    return cfgErr;
}

/*=== Start Config interface callbacks ===*/
static gboolean maker_dialog_config_set_key_file_init(MakerDialogConfigSet *configSet, MakerDialogError **error){
    return maker_dialog_config_set_prepare_files(configSet, error);
}

static void maker_dialog_config_set_key_file_finalize(MakerDialogConfigSet *configSet){
}

static gchar **maker_dialog_config_file_key_file_get_pages(MakerDialogConfigFile *configFile, MakerDialogError **error){
    return g_key_file_get_groups (configFile->fileObj, NULL);
}

static gboolean maker_dialog_config_file_key_file_can_access(MakerDialogConfigFile *configFile, guint permission, MakerDialogError **error){
    g_assert(configFile->path);
    guint perm=permission;
    if (perm & W_OK){
	if (!maker_dialog_file_isWritable(configFile->path))
	    return FALSE;
	perm &= ~W_OK;
    }
    if (g_access(configFile->path,(int) perm)){
	return FALSE;
    }
    return TRUE;
}

static gboolean maker_dialog_config_file_key_file_create(MakerDialogConfigFile *configFile, MakerDialogError **error){
    MakerDialogError *cfgErr=NULL;
    if (g_access(configFile->path, F_OK)==0){
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_ALREADY_EXIST, "File %s already exist!", configFile->path);
    }else if (maker_dialog_file_isWritable(configFile->path)){
	GKeyFile *keyFile=g_key_file_new();
	configFile->fileObj= (gpointer) keyFile;
	return TRUE;
    }else{
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_CANT_WRITE, "File %s already exist.", configFile->path);
    }
    maker_dialog_error_handle(cfgErr, error);
    return FALSE;
}

static gboolean maker_dialog_config_file_key_file_open(MakerDialogConfigFile *configFile, MakerDialogError **error){
    MakerDialogError * cfgErr=NULL, *cfgErr_prep=NULL;
    if (g_access(configFile->path,R_OK)==0){
	GKeyFile *keyFile=g_key_file_new();
	configFile->fileObj= (gpointer) keyFile;
	g_key_file_load_from_file(keyFile, configFile->path, G_KEY_FILE_NONE, &cfgErr_prep);
	if (cfgErr_prep){
	    if (cfgErr->code==G_KEY_FILE_ERROR_PARSE){
		/* Possibly empty file */
		g_warning("Error parse on file %s, but it can also mean an empty file.", configFile->path);
		maker_dialog_error_handle(cfgErr_prep,error);
	    }else{
		cfgErr=convert_error_code(cfgErr_prep, configFile->path,
			"config_file_key_file_open(): g_key_file_load_from_file");
		maker_dialog_error_handle(cfgErr,error);
		if (keyFile){
		    g_key_file_free(keyFile);
		}
		return FALSE;
	    }
	}else{
	    /* No error */
	    configFile->flags |= MAKER_DIALOG_CONFIG_FILE_FLAG_HAS_CONTENT;
	}
	return TRUE;
    }
    cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_CANT_READ, "File %s cannot be read!", configFile->path);
    maker_dialog_error_handle(cfgErr,error);
    return FALSE;
}

static void key_file_reload(MakerDialogConfigFile *configFile, MakerDialogError **error){
    if (configFile->fileObj){
	g_key_file_free((GKeyFile *) configFile->fileObj);
    }
    configFile->flags &= ~MAKER_DIALOG_CONFIG_FILE_FLAG_HAS_CONTENT;
    if (maker_dialog_config_file_key_file_open(configFile, error))
	return;
    /* For read-only, new install system */
    GKeyFile *keyFile=g_key_file_new();
    configFile->fileObj= (gpointer) keyFile;
}

static gboolean maker_dialog_config_file_key_file_close(MakerDialogConfigFile *configFile, MakerDialogError **error){
    if (configFile->fileObj){
	g_key_file_free((GKeyFile *) configFile->fileObj);
	configFile->fileObj=NULL;
	return TRUE;
    }
    return FALSE;
}

static gboolean key_file_preload_property(MakerDialogConfigFile *configFile, MakerDialogConfigBuffer *configBuf, MakerDialogPropertyContext *ctx, MakerDialogError **error){
    if (maker_dialog_config_buffer_lookup(configBuf, ctx->spec->key) && (configFile->configSet->config->flags & MAKER_DIALOG_CONFIG_FLAG_NO_OVERRIDE)){
	/* It is expected behavior if NO_OVERRIDE is set. */
	return TRUE;
    }
    gchar *value=g_key_file_get_value((GKeyFile *)configFile->fileObj, ctx->spec->pageName, ctx->spec->key, error);
    if (*error){
	return FALSE;
    }
    MkdgValue *mValue=maker_dialog_value_new(ctx->spec->valueType, NULL);
    maker_dialog_value_from_string(mValue, value, ctx->spec->parseOption);
    maker_dialog_config_buffer_insert(configBuf, ctx->spec->key, mValue);
    return TRUE;
}

static gboolean key_file_preload_page(MakerDialogConfigFile *configFile, MakerDialogConfigBuffer *configBuf, const gchar * page, MakerDialogError **error){
    MakerDialogError *cfgErr=NULL;
    gchar **keys=g_key_file_get_keys((GKeyFile *) configFile->fileObj , page, NULL, &cfgErr);
    if (cfgErr){
	maker_dialog_error_handle(cfgErr, error);
	return FALSE;
    }
    gint i;
    for (i=0;keys[i]!=NULL;i++){
	MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(configFile->configSet->config->mDialog, keys[i]);
	if (!key_file_preload_property(configFile, configBuf, ctx, &cfgErr)){
	    maker_dialog_error_handle(cfgErr, error);
	    if (configFile->configSet->config->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		g_strfreev(keys);
		return FALSE;
	    }
	}
    }
    g_strfreev(keys);
    return TRUE;
}

static gboolean maker_dialog_config_file_key_file_preload(MakerDialogConfigFile *configFile, MakerDialogConfigBuffer *configBuf, MakerDialogError **error){
    MakerDialogError *cfgErr=NULL;
    key_file_reload(configFile, NULL);
    MAKER_DIALOG_DEBUG_MSG(3, "[I3] config_file_key_file_preload( %s, )", configFile->path);
    g_assert(configFile->fileObj);
    gchar **pages=maker_dialog_config_file_key_file_get_pages(configFile, &cfgErr);
    if (cfgErr!=NULL){
	maker_dialog_error_handle(cfgErr, error);
	return FALSE;
    }
    gint i;

    for(i=0;pages[i]!=NULL;i++){
	key_file_preload_page(configFile, configBuf, pages[i], &cfgErr);
	if (cfgErr!=NULL){
	    maker_dialog_error_handle(cfgErr, error);
	    if (configFile->configSet->config->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		g_strfreev(pages);
		return FALSE;
	    }
	}
    }
    g_strfreev(pages);
    return TRUE;
}

/*
 * This function just save the setting, without checking the duplication, and
 * so on.
 */
static gboolean key_file_save_property(MakerDialogConfigFile *configFile, MakerDialogConfigBuffer *configBuf,
	MakerDialogPropertyContext *ctx, MakerDialogError **error){
    MAKER_DIALOG_DEBUG_MSG(5, "[I5] key_file_save_property(%d, , , )",configFile->path);
    MakerDialogError * cfgErr=NULL;
    /* Check whether the line need to be saved */
    gboolean needSave=TRUE;
    MkdgValue *bufValue=maker_dialog_config_buffer_lookup(configBuf, ctx->spec->key);
    if (bufValue){
	if ((configFile->configSet->flags & MAKER_DIALOG_CONFIG_FLAG_HIDE_DUPLICATE)
		&& maker_dialog_value_compare(bufValue, ctx->value, ctx->spec->compareOption)==0){
	    MAKER_DIALOG_DEBUG_MSG(4, "[I4] key_file_save_property() duplicated, no need to save.");
	    needSave=FALSE;
	}
    }else{
	if ((configFile->configSet->flags & MAKER_DIALOG_CONFIG_FLAG_HIDE_DEFAULT) && maker_dialog_property_is_default(ctx)){
	    MAKER_DIALOG_DEBUG_MSG(4, "[I4] key_file_save_property() is default value, no need to save.");
	    needSave=FALSE;
	}
    }
    MAKER_DIALOG_DEBUG_MSG(4, "[I4] key_file_save_property(%s, , %s, ) page=%s needSave=%s",
	    configFile->path, ctx->spec->key, (ctx->spec->pageName)? ctx->spec->pageName: "-", (needSave)? "TRUE": "FALSE");

    if (needSave){
	gint ret=0;
	if (ctx->spec->valueType==MKDG_TYPE_BOOLEAN){
	    /* GKeyFile only accept "true" and  "false" */
	    ret=fprintf((FILE *) configFile->userData,"%s=%s\n",ctx->spec->key, (maker_dialog_value_get_boolean(ctx->value))? "true" : "false");
	}else{
	    ret=fprintf((FILE *) configFile->userData,"%s=%s\n",ctx->spec->key, maker_dialog_property_to_string(ctx));
	}
	if (ret<0){
	    cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_CANT_WRITE, "key_file_save_property() failed on key %s",ctx->spec->key);
	    maker_dialog_error_handle(cfgErr, error);
	    return FALSE;
	}
	ctx->flags&=~MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNSAVED;
    }
    return TRUE;
}

static gboolean key_file_save_page(MakerDialogConfigFile *configFile, MakerDialogConfigBuffer *configBuf,
	const gchar *pageName, MakerDialogError **error){
    MakerDialogError * cfgErr=NULL;
    MakerDialogNodeIter iter=maker_dialog_page_property_iter_init(configFile->configSet->config->mDialog, pageName);
    if (fprintf((FILE *) configFile->userData,"[%s]\n",pageName)<0){
	/* Write error */
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_CANT_WRITE, "key_file_save_page() failed on page %s",pageName);
	maker_dialog_error_handle(cfgErr, error);
	return FALSE;
    }

    MakerDialogPropertyContext *ctx=NULL;
    while(maker_dialog_page_property_iter_has_next(iter)){
	ctx=maker_dialog_page_property_iter_next(&iter);
	g_assert(ctx);
	if (!key_file_save_property(configFile, configBuf, ctx, &cfgErr)){
	    maker_dialog_error_handle(cfgErr,error);
	    if (configFile->configSet->config->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		return FALSE;
	    }
	}
    }
    return TRUE;
}

static gboolean maker_dialog_config_file_key_file_save(MakerDialogConfigFile *configFile, MakerDialogConfigBuffer *configBuf, MakerDialogError **error){
    MakerDialogError * cfgErr=NULL;
    MAKER_DIALOG_DEBUG_MSG(4, "[I4] config_file_key_file_save(%s, , )", configFile->path);
    if (!maker_dialog_config_file_key_file_can_access(configFile, W_OK, error)){
	MAKER_DIALOG_DEBUG_MSG(4, "[I4] config_file_key_file_save(%s, , ) cannot write", configFile->path);
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_CANT_WRITE, "config_file_key_file_save()");
	maker_dialog_error_handle(cfgErr, error);
	return FALSE;
    }

    gchar *parentDir=g_path_get_dirname(configFile->path);
    g_assert(g_mkdir_with_parents(parentDir, 0x755)==0);
    configFile->userData=(gpointer) g_fopen(configFile->path, "w");
    MakerDialogNodeIter iter=maker_dialog_page_iter_init(configFile->configSet->config->mDialog);
    while(maker_dialog_page_iter_has_next(iter)){
	GNode *pageNode=maker_dialog_page_iter_next(&iter);
	g_assert(pageNode);
	if (!key_file_save_page(configFile, configBuf, (gchar *) pageNode->data, &cfgErr)){
	    maker_dialog_error_handle(cfgErr, error);
	    if (configFile->configSet->config->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		return FALSE;
	    }
	}
    }
    fclose((FILE *) configFile->userData);
    return TRUE;
}
/*=== End Config interface callbacks ===*/

MakerDialogConfigFileInterface MAKER_DIALOG_CONFIG_FILE_INTERFACE_KEY_FILE={
    maker_dialog_config_set_key_file_init,
    maker_dialog_config_set_key_file_finalize,
    maker_dialog_config_file_key_file_can_access,
    maker_dialog_config_file_key_file_create,
    maker_dialog_config_file_key_file_open,
    maker_dialog_config_file_key_file_close,
    maker_dialog_config_file_key_file_preload,
    maker_dialog_config_file_key_file_save,
    maker_dialog_config_file_key_file_get_pages
};

MakerDialogConfig *maker_dialog_config_use_key_file(MakerDialog *mDialog){
    MakerDialogConfig *config=maker_dialog_config_new_full(mDialog, 0, &MAKER_DIALOG_CONFIG_FILE_INTERFACE_KEY_FILE);
    return config;
}


gboolean maker_dialog_module_init(MakerDialog *mDialog){
    maker_dialog_config_new_full(mDialog, 0, &MAKER_DIALOG_CONFIG_FILE_INTERFACE_KEY_FILE);
    return TRUE;

}
