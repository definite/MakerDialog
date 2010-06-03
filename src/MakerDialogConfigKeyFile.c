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

static MkdgError *convert_error_code(GError *error, const gchar *filename, const gchar *prefix){
    if (!error)
	return NULL;
    if (error->domain==MKDG_ERROR)
	return error;

    GError *cfgErr=NULL;
    if (error->domain==G_FILE_ERROR){
	switch(error->code){
	    case G_FILE_ERROR_ACCES:
		cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_PERMISSION_DENY, prefix);
		break;
	    default:
		cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_OTHER, prefix);
		break;
	}
    }else if (error->domain==G_KEY_FILE_ERROR){
	switch(error->code){
	    case G_KEY_FILE_ERROR_UNKNOWN_ENCODING:
	    case G_KEY_FILE_ERROR_PARSE:
		cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_INVALID_FORMAT, prefix);
		break;
	    case G_KEY_FILE_ERROR_NOT_FOUND:
		cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_NOT_FOUND, prefix);
		break;
	    default:
		cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_OTHER, prefix);
		break;
	}
    }else{
	cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_OTHER, prefix);
    }
    g_warning("MkdgConfigKeyFile: %s%s %s: domain=%s code=%d:%s",
	    (filename)? "On file: " : "",
	    (filename)? filename : "",
	    prefix, g_quark_to_string(error->domain), error->code, error->message);
    g_error_free(error);
    return cfgErr;
}

/*=== Start Config interface callbacks ===*/
static gboolean mkdg_config_set_key_file_init(MkdgConfigSet *configSet, MkdgError **error){
    return mkdg_config_set_prepare_files(configSet, error);
}

static void mkdg_config_set_key_file_finalize(MkdgConfigSet *configSet){
}

static gchar **mkdg_config_file_key_file_get_pages(MkdgConfigFile *configFile, MkdgError **error){
    return g_key_file_get_groups (configFile->fileObj, NULL);
}

static gchar **mkdg_config_file_key_file_get_keys(MkdgConfigFile *configFile, const gchar *pageName, MkdgError **error){
    return g_key_file_get_keys (configFile->fileObj, pageName, NULL, error);
}

static MkdgValue *mkdg_config_file_key_file_get_value(MkdgConfigFile *configFile, const gchar *pageName, const gchar *key,
       	MkdgType valueType, const gchar *parseOption, MkdgError **error){
    MkdgError *cfgErr_prep=NULL;
    gchar *str=g_key_file_get_string(configFile->fileObj, pageName, key, &cfgErr_prep);
    if (cfgErr_prep!=NULL){
	MkdgError *cfgErr=convert_error_code(cfgErr_prep, configFile->path, "config_file_key_file_get_value()");
	mkdg_error_handle(cfgErr,error);
	return NULL;
    }
    MkdgValue *mValue=mkdg_value_new(valueType,NULL);
    MkdgValue *ret=mkdg_value_from_string(mValue, str, parseOption);
    g_free(str);
    return ret;
}

static gboolean mkdg_config_file_key_file_can_access(MkdgConfigFile *configFile, guint permission, MkdgError **error){
    g_assert(configFile->path);
    guint perm=permission;
    if (perm & W_OK){
	if (!mkdg_file_isWritable(configFile->path))
	    return FALSE;
	perm &= ~W_OK;
    }
    if (g_access(configFile->path,(int) perm)){
	return FALSE;
    }
    return TRUE;
}

static gboolean mkdg_config_file_key_file_create(MkdgConfigFile *configFile, MkdgError **error){
    MkdgError *cfgErr=NULL;
    if (g_access(configFile->path, F_OK)==0){
	cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_ALREADY_EXIST, "File %s already exist!", configFile->path);
    }else if (mkdg_file_isWritable(configFile->path)){
	GKeyFile *keyFile=g_key_file_new();
	configFile->fileObj= (gpointer) keyFile;
	return TRUE;
    }else{
	cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_CANT_WRITE, "File %s already exist.", configFile->path);
    }
    mkdg_error_handle(cfgErr, error);
    return FALSE;
}

static gboolean mkdg_config_file_key_file_open(MkdgConfigFile *configFile, MkdgError **error){
    MkdgError * cfgErr=NULL, *cfgErr_prep=NULL;
    if (g_access(configFile->path,R_OK)==0){
	GKeyFile *keyFile=g_key_file_new();
	configFile->fileObj= (gpointer) keyFile;
	g_key_file_load_from_file(keyFile, configFile->path, G_KEY_FILE_NONE, &cfgErr_prep);
	if (cfgErr_prep){
	    if (cfgErr->code==G_KEY_FILE_ERROR_PARSE){
		/* Possibly empty file */
		g_warning("Error parse on file %s, but it can also mean an empty file.", configFile->path);
		mkdg_error_handle(cfgErr_prep,error);
	    }else{
		cfgErr=convert_error_code(cfgErr_prep, configFile->path,
			"config_file_key_file_open(): g_key_file_load_from_file");
		mkdg_error_handle(cfgErr,error);
		if (keyFile){
		    g_key_file_free(keyFile);
		}
		return FALSE;
	    }
	}else{
	    /* No error */
	    configFile->flags |= MKDG_CONFIG_FILE_FLAG_HAS_CONTENT;
	}
	return TRUE;
    }
    cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_CANT_READ, "File %s cannot be read!", configFile->path);
    mkdg_error_handle(cfgErr,error);
    return FALSE;
}

static void key_file_reload(MkdgConfigFile *configFile, MkdgError **error){
    if (configFile->fileObj){
	g_key_file_free((GKeyFile *) configFile->fileObj);
    }
    configFile->flags &= ~MKDG_CONFIG_FILE_FLAG_HAS_CONTENT;
    if (mkdg_config_file_key_file_open(configFile, error))
	return;
    /* For read-only, new install system */
    GKeyFile *keyFile=g_key_file_new();
    configFile->fileObj= (gpointer) keyFile;
}

static gboolean mkdg_config_file_key_file_close(MkdgConfigFile *configFile, MkdgError **error){
    if (configFile->fileObj){
	g_key_file_free((GKeyFile *) configFile->fileObj);
	configFile->fileObj=NULL;
	return TRUE;
    }
    return FALSE;
}

static gboolean key_file_preload_property(MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf, MkdgPropertyContext *ctx, MkdgError **error){
    if (mkdg_config_buffer_lookup(configBuf, ctx->spec->key) && (configFile->configSet->config->flags & MKDG_CONFIG_FLAG_NO_OVERRIDE)){
	/* It is expected behavior if NO_OVERRIDE is set. */
	return TRUE;
    }
    MkdgError *cfgErr=NULL;
    MkdgValue *mValue= mkdg_config_file_key_file_get_value(configFile, ctx->spec->pageName, ctx->spec->key,
	    ctx->spec->valueType, ctx->spec->parseOption, &cfgErr);
    if (mValue==NULL){
	mkdg_error_handle(cfgErr,error);
	return FALSE;
    }
    mkdg_config_buffer_insert(configBuf, ctx->spec->key, mValue);
    return TRUE;
}

static gboolean key_file_preload_page(MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf, const gchar * page, MkdgError **error){
    MkdgError *cfgErr=NULL;
    gchar **keys=g_key_file_get_keys((GKeyFile *) configFile->fileObj , page, NULL, &cfgErr);
    if (cfgErr){
	mkdg_error_handle(cfgErr, error);
	return FALSE;
    }
    gint i;
    for (i=0;keys[i]!=NULL;i++){
	MkdgPropertyContext *ctx=mkdg_get_property_context(configFile->configSet->config->mDialog, keys[i]);
	if (!key_file_preload_property(configFile, configBuf, ctx, &cfgErr)){
	    mkdg_error_handle(cfgErr, error);
	    if (configFile->configSet->config->flags & MKDG_CONFIG_FLAG_STOP_ON_ERROR){
		g_strfreev(keys);
		return FALSE;
	    }
	}
    }
    g_strfreev(keys);
    return TRUE;
}

static gboolean mkdg_config_file_key_file_preload(MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf, MkdgError **error){
    MkdgError *cfgErr=NULL;
    key_file_reload(configFile, NULL);
    MKDG_DEBUG_MSG(3, "[I3] config_file_key_file_preload( %s, )", configFile->path);
    g_assert(configFile->fileObj);
    gchar **pages=mkdg_config_file_key_file_get_pages(configFile, &cfgErr);
    if (cfgErr!=NULL){
	mkdg_error_handle(cfgErr, error);
	return FALSE;
    }
    gint i;

    for(i=0;pages[i]!=NULL;i++){
	key_file_preload_page(configFile, configBuf, pages[i], &cfgErr);
	if (cfgErr!=NULL){
	    mkdg_error_handle(cfgErr, error);
	    if (configFile->configSet->config->flags & MKDG_CONFIG_FLAG_STOP_ON_ERROR){
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
static gboolean key_file_save_property(MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf,
	MkdgPropertyContext *ctx, MkdgError **error){
    MKDG_DEBUG_MSG(5, "[I5] key_file_save_property(%d, , , )",configFile->path);
    MkdgError * cfgErr=NULL;
    /* Check whether the line need to be saved */
    gboolean needSave=TRUE;
    MkdgValue *bufValue=mkdg_config_buffer_lookup(configBuf, ctx->spec->key);
    if (bufValue){
	if ((configFile->configSet->flags & MKDG_CONFIG_FLAG_HIDE_DUPLICATE)
		&& mkdg_value_compare(bufValue, ctx->value, ctx->spec->compareOption)==0){
	    MKDG_DEBUG_MSG(4, "[I4] key_file_save_property() duplicated, no need to save.");
	    needSave=FALSE;
	}
    }else{
	if ((configFile->configSet->flags & MKDG_CONFIG_FLAG_HIDE_DEFAULT) && mkdg_property_is_default(ctx)){
	    MKDG_DEBUG_MSG(4, "[I4] key_file_save_property() is default value, no need to save.");
	    needSave=FALSE;
	}
    }
    MKDG_DEBUG_MSG(4, "[I4] key_file_save_property(%s, , %s, ) page=%s needSave=%s",
	    configFile->path, ctx->spec->key, (ctx->spec->pageName)? ctx->spec->pageName: "-", (needSave)? "TRUE": "FALSE");

    if (needSave){
	gint ret=0;
	if (ctx->spec->valueType==MKDG_TYPE_BOOLEAN){
	    /* GKeyFile only accept "true" and  "false" */
	    ret=fprintf((FILE *) configFile->userData,"%s=%s\n",ctx->spec->key, (mkdg_value_get_boolean(ctx->value))? "true" : "false");
	}else{
	    ret=fprintf((FILE *) configFile->userData,"%s=%s\n",ctx->spec->key, mkdg_property_to_string(ctx));
	}
	if (ret<0){
	    cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_CANT_WRITE, "key_file_save_property() failed on key %s",ctx->spec->key);
	    mkdg_error_handle(cfgErr, error);
	    return FALSE;
	}
	ctx->flags&=~MKDG_PROPERTY_CONTEXT_FLAG_UNSAVED;
    }
    return TRUE;
}

static gboolean key_file_save_page(MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf,
	const gchar *pageName, MkdgError **error){
    MkdgError * cfgErr=NULL;
    MkdgNodeIter iter=mkdg_page_property_iter_init(configFile->configSet->config->mDialog, pageName);
    if (fprintf((FILE *) configFile->userData,"[%s]\n",pageName)<0){
	/* Write error */
	cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_CANT_WRITE, "key_file_save_page() failed on page %s",pageName);
	mkdg_error_handle(cfgErr, error);
	return FALSE;
    }

    MkdgPropertyContext *ctx=NULL;
    while(mkdg_page_property_iter_has_next(iter)){
	ctx=mkdg_page_property_iter_next(&iter);
	g_assert(ctx);
	if (!key_file_save_property(configFile, configBuf, ctx, &cfgErr)){
	    mkdg_error_handle(cfgErr,error);
	    if (configFile->configSet->config->flags & MKDG_CONFIG_FLAG_STOP_ON_ERROR){
		return FALSE;
	    }
	}
    }
    return TRUE;
}

static gboolean mkdg_config_file_key_file_save(MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf, MkdgError **error){
    MkdgError * cfgErr=NULL;
    MKDG_DEBUG_MSG(4, "[I4] config_file_key_file_save(%s, , )", configFile->path);
    if (!mkdg_config_file_key_file_can_access(configFile, W_OK, error)){
	MKDG_DEBUG_MSG(4, "[I4] config_file_key_file_save(%s, , ) cannot write", configFile->path);
	cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_CANT_WRITE, "config_file_key_file_save()");
	mkdg_error_handle(cfgErr, error);
	return FALSE;
    }

    gchar *parentDir=g_path_get_dirname(configFile->path);
    g_assert(g_mkdir_with_parents(parentDir, 0x755)==0);
    configFile->userData=(gpointer) g_fopen(configFile->path, "w");
    MkdgNodeIter iter=mkdg_page_iter_init(configFile->configSet->config->mDialog);
    while(mkdg_page_iter_has_next(iter)){
	GNode *pageNode=mkdg_page_iter_next(&iter);
	g_assert(pageNode);
	if (!key_file_save_page(configFile, configBuf, (gchar *) pageNode->data, &cfgErr)){
	    mkdg_error_handle(cfgErr, error);
	    if (configFile->configSet->config->flags & MKDG_CONFIG_FLAG_STOP_ON_ERROR){
		return FALSE;
	    }
	}
    }
    fclose((FILE *) configFile->userData);
    return TRUE;
}
/*=== End Config interface callbacks ===*/

MkdgConfigFileInterface MKDG_CONFIG_FILE_INTERFACE_KEY_FILE={
    mkdg_config_set_key_file_init,
    mkdg_config_set_key_file_finalize,
    mkdg_config_file_key_file_can_access,
    mkdg_config_file_key_file_create,
    mkdg_config_file_key_file_open,
    mkdg_config_file_key_file_close,
    mkdg_config_file_key_file_preload,
    mkdg_config_file_key_file_save,
    mkdg_config_file_key_file_get_pages,
    mkdg_config_file_key_file_get_keys,
    mkdg_config_file_key_file_get_value,
};

MkdgConfig *mkdg_config_use_key_file(Mkdg *mDialog){
    MkdgConfig *config=mkdg_config_new_full(mDialog, 0, &MKDG_CONFIG_FILE_INTERFACE_KEY_FILE);
    return config;
}


gboolean mkdg_module_init(Mkdg *mDialog){
    mkdg_config_new_full(mDialog, 0, &MKDG_CONFIG_FILE_INTERFACE_KEY_FILE);
    return TRUE;

}
