/*
 * Copyright © 2009  Red Hat, Inc. All rights reserved.
 * Copyright © 2009  Ding-Yi Chen <dchen at redhat.com>
 *
 *  This file is part of Mkdg.
 *
 *  Mkdg is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Mkdg is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Mkdg.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <gconf/gconf-value.h>
#include <gconf/gconf-changeset.h>
#include "MakerDialog.h"
#include "MakerDialogConfigGConf.h"

/*=== Start Error handling ===*/
#define GCONF_ERROR_FORMAT	"MkdgConfigGConf: In %s, %s Error: %s"

static MkdgError *mkdg_config_gconf_error_new(MkdgErrorCode code, const gchar *location, const gchar *cause, const gchar *errorMsg){
    return mkdg_error_new(code, GCONF_ERROR_FORMAT, location, cause,errorMsg);
}

static MkdgError *convert_error_code(GError *error, const gchar *location){
    if (error==NULL)
	return NULL;
    if (error->domain==MKDG_ERROR)
	return error;

    MkdgError *cfgErr=NULL;
    switch(error->code){
	case GCONF_ERROR_SUCCESS:
	    g_error_free(error);
	    return NULL;
	case GCONF_ERROR_IN_SHUTDOWN:
	    cfgErr=mkdg_config_gconf_error_new(MKDG_ERROR_CONFIG_NOT_READY, location, "Database is shutting down", error->message);
	    break;
	case GCONF_ERROR_NO_SERVER:
	    cfgErr=mkdg_config_gconf_error_new(MKDG_ERROR_CONFIG_NOT_READY, location, "No server", error->message);
	    break;
	case GCONF_ERROR_NO_PERMISSION:
	    cfgErr=mkdg_config_gconf_error_new(MKDG_ERROR_CONFIG_PERMISSION_DENY, location, "No permission", error->message);
	    break;
	case GCONF_ERROR_BAD_ADDRESS:
	    cfgErr=mkdg_config_gconf_error_new(MKDG_ERROR_CONFIG_PERMISSION_DENY, location, "Bad address", error->message);
	    break;
	case GCONF_ERROR_PARSE_ERROR:
	    cfgErr=mkdg_config_gconf_error_new(MKDG_ERROR_CONFIG_INVALID_FORMAT, location,  "Parse error", error->message);
	    break;
	case GCONF_ERROR_CORRUPT:
	    cfgErr=mkdg_config_gconf_error_new(MKDG_ERROR_CONFIG_INVALID_FORMAT, location,  "Corrupt", error->message);
	    break;
	case GCONF_ERROR_TYPE_MISMATCH:
	    cfgErr=mkdg_config_gconf_error_new(MKDG_ERROR_CONFIG_INVALID_FORMAT, location, "Type mismatch", error->message);
	    break;
	case GCONF_ERROR_BAD_KEY:
	    cfgErr=mkdg_config_gconf_error_new(MKDG_ERROR_CONFIG_INVALID_KEY, location, "Bad key", error->message);
	    break;
	case GCONF_ERROR_IS_DIR:
	    cfgErr=mkdg_config_gconf_error_new(MKDG_ERROR_CONFIG_INVALID_KEY, location, "Is dir", error->message);
	    break;
	case GCONF_ERROR_IS_KEY:
	    cfgErr=mkdg_config_gconf_error_new(MKDG_ERROR_CONFIG_INVALID_PAGE, location, "Is Key", error->message);
	    break;
	case GCONF_ERROR_OVERRIDDEN:
	    cfgErr=mkdg_config_gconf_error_new(MKDG_ERROR_CONFIG_CANT_WRITE, location, "Overridden", error->message);
	    break;
	case GCONF_ERROR_LOCK_FAILED:
	    cfgErr=mkdg_config_gconf_error_new(MKDG_ERROR_CONFIG_CANT_WRITE, location, "Lock failed", error->message);
	    break;
	case GCONF_ERROR_NO_WRITABLE_DATABASE:
	    cfgErr=mkdg_config_gconf_error_new(MKDG_ERROR_CONFIG_CANT_WRITE, location, "No writable database", error->message);
	    break;
	case GCONF_ERROR_OAF_ERROR:
	    cfgErr=mkdg_config_gconf_error_new(MKDG_ERROR_CONFIG_OTHER, location, "liboaf error", error->message);
	    break;
	case GCONF_ERROR_LOCAL_ENGINE:
	    cfgErr=mkdg_config_gconf_error_new(MKDG_ERROR_CONFIG_OTHER, location, "Trying to use remote method on local engine", error->message);
	    break;
	case GCONF_ERROR_FAILED :
	    cfgErr=mkdg_config_gconf_error_new(MKDG_ERROR_CONFIG_OTHER, location, "Failed", error->message);
	    break;
	default:
	    cfgErr=mkdg_config_gconf_error_new(MKDG_ERROR_CONFIG_OTHER, location, "Other error", error->message);
	    break;
    }
    g_error_free(error);
    return cfgErr;
}
/*=== End Error handling ===*/

/*=== Start Config interface callbacks ===*/
static gboolean mkdg_config_set_gconf_init(MkdgConfigSet *configSet, MkdgError **error){
    g_assert(configSet);
    configSet->userData=gconf_engine_get_default();
    if (configSet->userData==NULL)
	return FALSE;
    MkdgConfigFile *configFile=mkdg_config_file_new(configSet->filePattern,configSet);
    g_ptr_array_add(configSet->fileArray,configFile);
    configSet->writeIndex=0;
    return TRUE;
}

static void mkdg_config_set_gconf_finalize(MkdgConfigSet *configSet){
    gconf_engine_unref((GConfEngine *) configSet->userData);
}

static void gconf_entry_free_wrapper(gpointer data){
    gconf_entry_free((GConfEntry *) data);
}

static void gconf_gslist_free(GSList *sList, GFreeFunc freeFunc){
    GSList *sListItem=sList;
    for(;sListItem!=NULL; sListItem=g_slist_next(sListItem)){
	freeFunc(sListItem->data);
    }
    g_slist_free(sList);
}

static gchar **mkdg_config_file_gconf_get_pages(MkdgConfigFile *configFile, MkdgError **error){
    MkdgError *cfgErr=NULL;
    g_assert(configFile->configSet->userData);
    GConfEngine *engine=(GConfEngine *)configFile->configSet->userData;
    GSList *sListHead=gconf_engine_all_dirs(engine,configFile->path,&cfgErr);
    GSList *sList=sListHead;
    gchar *pagePath;
    gchar *page;
    GPtrArray *ptrArray=g_ptr_array_new();
    if (cfgErr==NULL){
	for(;sList!=NULL; sList=g_slist_next(sList)){
	    pagePath=(gchar *)sList->data;
	    MKDG_DEBUG_MSG(5, "[I5] config_gconf_get_pages(%s, ) path=%s", configFile->path, pagePath);
	    page=g_path_get_basename (pagePath);
	    g_ptr_array_add(ptrArray, page);
	}
	/* Free sList */
	gconf_gslist_free(sListHead, g_free);
    }else{
	mkdg_error_handle(cfgErr,error);
    }
    g_ptr_array_add(ptrArray, NULL);
    return (gchar **) g_ptr_array_free(ptrArray, FALSE);
}

static gchar **mkdg_config_file_gconf_get_keys(MkdgConfigFile *configFile, const gchar *pageName, MkdgError **error){
    MkdgError *cfgErr=NULL;
    g_assert(configFile->configSet->userData);
    GConfEngine *engine=(GConfEngine *)configFile->configSet->userData;
    gchar *path=g_build_filename(configFile->path, pageName, NULL);
    GSList *sListHead=gconf_engine_all_dirs(engine,path,&cfgErr);
    GSList *sList=sListHead;
    GConfEntry *cfgEntry;
    gchar *key=NULL;
    GPtrArray *ptrArray=g_ptr_array_new();
    if (cfgErr==NULL){
	for(;sList!=NULL; sList=g_slist_next(sList)){
	    cfgEntry=(GConfEntry *)sList->data;
	    key=g_strdup(cfgEntry->key);
	    MKDG_DEBUG_MSG(5, "[I5] config_gconf_get_keys(%s, ) path=%s key=%s", configFile->path, path, key );
	    g_ptr_array_add(ptrArray,key);
	}
	/* Free sList */
	gconf_gslist_free(sListHead, gconf_entry_free_wrapper);
    }else{
	mkdg_error_handle(cfgErr,error);
    }
    g_ptr_array_add(ptrArray, NULL);
    return (gchar **) g_ptr_array_free(ptrArray, FALSE);
}

static MkdgValue *mkdg_config_gconf_engine_get_value(GConfEngine *engine, const gchar *path,
	MkdgType valueType, const gchar *parseOption, MkdgError **error){
    MkdgError *cfgErr_prep=NULL;
    MkdgValue *mValue=mkdg_value_new(valueType, NULL);
    gchar *strValue=NULL;
    switch(valueType){
	case MKDG_TYPE_BOOLEAN:
	    mkdg_value_set_boolean(mValue, gconf_engine_get_bool(engine, path, &cfgErr_prep));
	    break;
	case MKDG_TYPE_INT:
	    mkdg_value_set_int(mValue, gconf_engine_get_int(engine, path, &cfgErr_prep));
	    break;
	case MKDG_TYPE_INT32:
	    mkdg_value_set_int32(mValue, gconf_engine_get_int(engine, path, &cfgErr_prep));
	    break;
	case MKDG_TYPE_INT64:
	    mkdg_value_set_int64(mValue, gconf_engine_get_int(engine, path, &cfgErr_prep));
	    break;
	case MKDG_TYPE_UINT:
	    mkdg_value_set_uint(mValue, (guint) gconf_engine_get_int(engine, path, &cfgErr_prep));
	    break;
	case MKDG_TYPE_UINT32:
	    mkdg_value_set_uint32(mValue, (guint32) gconf_engine_get_int(engine, path, &cfgErr_prep));
	    break;
	case MKDG_TYPE_UINT64:
	    mkdg_value_set_uint64(mValue, (guint64) gconf_engine_get_int(engine, path, &cfgErr_prep));
	    break;
	case MKDG_TYPE_LONG:
	    mkdg_value_set_long(mValue,  gconf_engine_get_int(engine, path, &cfgErr_prep));
	    break;
	case MKDG_TYPE_ULONG:
	    mkdg_value_set_ulong(mValue, (gulong) gconf_engine_get_int(engine, path, &cfgErr_prep));
	    break;
	case MKDG_TYPE_FLOAT:
	    mkdg_value_set_float(mValue, (gfloat) gconf_engine_get_float(engine, path, &cfgErr_prep));
	    break;
	case MKDG_TYPE_DOUBLE:
	    mkdg_value_set_double(mValue, gconf_engine_get_float(engine, path, &cfgErr_prep));
	    break;
	case MKDG_TYPE_STRING:
	case MKDG_TYPE_STRING_LIST:
	case MKDG_TYPE_COLOR:
	    strValue=(gchar *) gconf_engine_get_string(engine, path, &cfgErr_prep);
	    if (strValue!=NULL){
		MKDG_DEBUG_MSG(6, "[I6] config_gconf_engine_get_value() strValue=%s", strValue);
		mkdg_value_from_string(mValue, strValue, parseOption);
		g_free(strValue);
	    }
	    break;
	default:
	    break;
    }
    if (cfgErr_prep!=NULL){
	MkdgError *cfgErr=convert_error_code(cfgErr_prep, "config_gconf_engine_get_value()" );
	mkdg_error_handle(cfgErr, error);
	return NULL;
    }
    return mValue;
}

static MkdgValue *mkdg_config_file_gconf_get_value(MkdgConfigFile *configFile, const gchar *pageName, const gchar *key,
	MkdgType valueType, const gchar *parseOption, MkdgError **error){
    GConfEngine *engine=(GConfEngine *)configFile->configSet->userData;
    gchar *path=g_build_filename(configFile->path, pageName, key, NULL);
    MkdgValue *mValue=mkdg_config_gconf_engine_get_value(engine, path, valueType, parseOption, error);
    g_free(path);
    return mValue;
}

static guint mkdg_config_file_gconf_get_access(MkdgConfigFile *configFile, MkdgError **error){
    GConfEngine *engine=(GConfEngine *)configFile->configSet->userData;
    guint accessFlags=0;
    if (gconf_engine_dir_exists (engine, configFile->path, error)){
	accessFlags|=F_OK | R_OK;
	/* TODO: Test write */
	/* Assume it is writable */
	if (!(configFile->flags &MKDG_CONFIG_FILE_FLAG_CANT_WRITE))
	    accessFlags|=W_OK;
    }
    return accessFlags;
}

static gboolean mkdg_config_file_gconf_can_access(MkdgConfigFile *configFile, guint permission, MkdgError **error){
    if (!configFile->configSet->userData)
	return FALSE;
    guint flags=mkdg_config_file_gconf_get_access(configFile, error);
    MKDG_DEBUG_MSG(5, "**** [I5] gconf_can_access()", flags);
    return mkdg_has_all_flags(permission, flags);
}

static gboolean mkdg_config_file_gconf_create(MkdgConfigFile *configFile, MkdgError **error){
    g_assert(configFile->configSet->userData);
    MKDG_DEBUG_MSG(5, "[I5] config_gconf_create(%s, )", configFile->path);
    GConfEngine *engine=(GConfEngine *)configFile->configSet->userData;
    MkdgError *cfgErr=NULL;
    if (gconf_engine_dir_exists (engine, configFile->path, &cfgErr)){
	cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_ALREADY_EXIST, "Path %s already exists!", configFile->path);
    }else{
	cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_OTHER, "No create mode with GConf back-end!");
    }
    mkdg_error_handle(cfgErr,error);
    return FALSE;
}

static gboolean mkdg_config_file_gconf_open(MkdgConfigFile *configFile, MkdgError **error){
    g_assert(configFile->configSet->userData);
    MKDG_DEBUG_MSG(5, "[I5] config_gconf_open(%s, )", configFile->path);
    GConfEngine *engine=(GConfEngine *)configFile->configSet->userData;
    MkdgError *cfgErr=NULL;
    if (!gconf_engine_dir_exists (engine, configFile->path, &cfgErr)){
	cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_NO_FILE, "Path %s does not exist!", configFile->path);
	mkdg_error_handle(cfgErr,error);
	return FALSE;
    }
    gconf_engine_ref(engine);
    configFile->fileObj=(gpointer) engine;
    return TRUE;
}

static gboolean mkdg_config_file_gconf_close(MkdgConfigFile *configFile, MkdgError **error){
    configFile->fileObj=NULL;
    gconf_engine_unref((GConfEngine *)configFile->configSet->userData);
    return TRUE;
}

static gboolean gconf_preload_property(MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf, GSList *entryList, MkdgError **error){
    MkdgError * cfgErr=NULL;
    GConfEntry *cfgEntry=(GConfEntry *) entryList->data;
    gchar *key=g_path_get_basename(cfgEntry->key);
    MkdgPropertyContext *ctx=mkdg_get_property_context(configFile->configSet->config->mDialog,key);
    if (ctx==NULL){
	cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_INVALID_KEY, "config_gconf_preload_property() key=%s, cfgEntry->key=%s", key, cfgEntry->key);
	mkdg_error_handle(cfgErr, error);
	g_free(key);
	if (configFile->configSet->flags & MKDG_CONFIG_FLAG_STOP_ON_ERROR ){
	    return FALSE;
	}
	return TRUE;
    }
    MKDG_DEBUG_MSG(3, "[I3] gconf_preload_property(%s,  , %s, )", configFile->path, ctx->spec->key);
    gchar *strValue=NULL;

    MkdgValue *mValue=mkdg_value_new(ctx->spec->valueType, NULL);
    switch(ctx->spec->valueType){
	case MKDG_TYPE_BOOLEAN:
	    mkdg_value_set_boolean(mValue, gconf_value_get_bool(cfgEntry->value));
	    break;
	case MKDG_TYPE_INT:
	    mkdg_value_set_int(mValue, gconf_value_get_int(cfgEntry->value));
	    break;
	case MKDG_TYPE_INT32:
	    mkdg_value_set_int32(mValue, gconf_value_get_int(cfgEntry->value));
	    break;
	case MKDG_TYPE_INT64:
	    mkdg_value_set_int64(mValue, gconf_value_get_int(cfgEntry->value));
	    break;
	case MKDG_TYPE_UINT:
	    mkdg_value_set_uint(mValue, (guint) gconf_value_get_int(cfgEntry->value));
	    break;
	case MKDG_TYPE_UINT32:
	    mkdg_value_set_uint32(mValue, (guint32) gconf_value_get_int(cfgEntry->value));
	    break;
	case MKDG_TYPE_UINT64:
	    mkdg_value_set_uint64(mValue, (guint64) gconf_value_get_int(cfgEntry->value));
	    break;
	case MKDG_TYPE_LONG:
	    mkdg_value_set_long(mValue,  gconf_value_get_int(cfgEntry->value));
	    break;
	case MKDG_TYPE_ULONG:
	    mkdg_value_set_ulong(mValue, (gulong) gconf_value_get_int(cfgEntry->value));
	    break;
	case MKDG_TYPE_FLOAT:
	    mkdg_value_set_float(mValue, (gfloat) gconf_value_get_float(cfgEntry->value));
	    break;
	case MKDG_TYPE_DOUBLE:
	    mkdg_value_set_double(mValue, gconf_value_get_float(cfgEntry->value));
	    break;
	case MKDG_TYPE_STRING:
	    strValue=(gchar *) gconf_value_get_string(cfgEntry->value);
	    MKDG_DEBUG_MSG(6, "[I6] gconf_preload_property() strValue=%s", strValue);
	    mkdg_value_from_string(mValue, strValue, NULL);
	    break;
	case MKDG_TYPE_STRING_LIST:
	    mkdg_value_from_string(mValue, gconf_value_get_string(cfgEntry->value),NULL);
	    break;
	case MKDG_TYPE_COLOR:
	    mkdg_value_from_string(mValue, gconf_value_get_string(cfgEntry->value), NULL);
	    break;
	default:
	    break;
    }
    MKDG_DEBUG_MSG(3,
	    configFile->path, mkdg_value_to_string(mValue, ctx->spec->toStringFormat), ctx->spec->key );
    mkdg_config_buffer_insert(configBuf, ctx->spec->key, mValue);
    return TRUE;
}

static gboolean gconf_preload_page(MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf, const gchar *pagePath, MkdgError **error){
    MkdgError * cfgErr=NULL;
    GSList *entryListHead=gconf_engine_all_entries((GConfEngine *)configFile->configSet->userData, pagePath, &cfgErr);
    if (cfgErr!=NULL){
	mkdg_error_handle(cfgErr, error);
	if (configFile->configSet->config->flags & MKDG_CONFIG_FLAG_STOP_ON_ERROR){
	    return FALSE;
	}
    }
    GSList *entryList=entryListHead;
    for(;entryList!=NULL;entryList=g_slist_next(entryList)){
	gconf_preload_property(configFile, configBuf, entryList, &cfgErr);
	if (cfgErr!=NULL){
	    mkdg_error_handle(cfgErr, error);
	    if (configFile->configSet->flags & MKDG_CONFIG_FLAG_STOP_ON_ERROR ){
		gconf_gslist_free(entryListHead, gconf_entry_free_wrapper);
		return FALSE;
	    }
	}
    }
    gconf_gslist_free(entryListHead, gconf_entry_free_wrapper);
    return TRUE;
}

static gboolean mkdg_config_file_gconf_preload(MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf, MkdgError **error){
    g_assert(configFile->configSet->userData);
    MKDG_DEBUG_MSG(5, "[I5] config_gconf_preload(%s, , )", configFile->path);
    GConfEngine *engine=(GConfEngine *)configFile->configSet->userData;
    MkdgError * cfgErr=NULL, *cfgErr_prep=NULL;
    GSList *dirListHead=gconf_engine_all_dirs(engine,configFile->path,&cfgErr_prep);
    if (cfgErr_prep!=NULL){
	cfgErr=convert_error_code(cfgErr_prep, "config_file_gconf_preload()");
	mkdg_error_handle(cfgErr,error);
        return FALSE;
    }
    GSList *dirList=dirListHead;
    gchar *pagePath;
    g_assert(!cfgErr);
    for(;dirList!=NULL; dirList=g_slist_next(dirList)){
	pagePath=(gchar *)dirList->data;
	gconf_preload_page(configFile, configBuf, pagePath, &cfgErr);
	if (cfgErr){
	    mkdg_error_handle(cfgErr, error);
	    if (configFile->configSet->flags & MKDG_CONFIG_FLAG_STOP_ON_ERROR ){
		gconf_gslist_free(dirListHead, g_free);
		return FALSE;
	    }
	}
    }
    gconf_gslist_free(dirListHead, g_free);
    return TRUE;
}

/*
 * This function just save the setting, without checking the duplication, and
 * so on.
 */
static gboolean gconf_save_property(MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf,
	MkdgPropertyContext *ctx, const gchar *pageName, GConfChangeSet *changeSet, MkdgError **error){
    MkdgError * cfgErr=NULL;
    /* Check whether the line need to be saved */
    MkdgValue *bufValue=mkdg_config_buffer_lookup(configBuf, ctx->spec->key);
    gboolean needChange=TRUE;
    if (bufValue!=NULL){
	if (mkdg_value_compare(bufValue, ctx->value, NULL)==0){
	    /* Same value already in gconf */
	    MKDG_DEBUG_MSG(4, "[I4] gconf_save_property() same value already in gconf, no need to change.");
	    needChange=FALSE;
	}
    }
    MKDG_DEBUG_MSG(4, "[I4] gconf_save_property(%s, , %s, ) page=%s needChange=%s",
	    configFile->path, ctx->spec->key, (ctx->spec->pageName)? ctx->spec->pageName: "-", (needChange)? "TRUE": "FALSE");

    if (needChange){
	gchar *keyPath=g_build_filename(configFile->path,pageName,ctx->spec->key, NULL);

	switch (ctx->spec->valueType){
	    case MKDG_TYPE_BOOLEAN:
		gconf_change_set_set_bool(changeSet, keyPath, mkdg_value_get_boolean(ctx->value));
		break;
	    case MKDG_TYPE_INT:
		gconf_change_set_set_int(changeSet, keyPath, mkdg_value_get_int(ctx->value));
		break;
	    case MKDG_TYPE_INT32:
		gconf_change_set_set_int(changeSet, keyPath, mkdg_value_get_int32(ctx->value));
		break;
	    case MKDG_TYPE_INT64:
		gconf_change_set_set_int(changeSet, keyPath, mkdg_value_get_int64(ctx->value));
		break;
	    case MKDG_TYPE_UINT:
		gconf_change_set_set_int(changeSet, keyPath, mkdg_value_get_uint(ctx->value));
		break;
	    case MKDG_TYPE_UINT32:
		gconf_change_set_set_int(changeSet, keyPath, mkdg_value_get_uint32(ctx->value));
		break;
	    case MKDG_TYPE_UINT64:
		gconf_change_set_set_int(changeSet, keyPath, mkdg_value_get_uint64(ctx->value));
		break;
	    case MKDG_TYPE_LONG:
		gconf_change_set_set_int(changeSet, keyPath, mkdg_value_get_long(ctx->value));
		break;
	    case MKDG_TYPE_ULONG:
		gconf_change_set_set_int(changeSet, keyPath, mkdg_value_get_long(ctx->value));
		break;
	    case MKDG_TYPE_FLOAT:
		gconf_change_set_set_float(changeSet, keyPath, mkdg_value_get_float(ctx->value));
		break;
	    case MKDG_TYPE_DOUBLE:
		gconf_change_set_set_float(changeSet, keyPath, mkdg_value_get_double(ctx->value));
		break;
	    case MKDG_TYPE_STRING:
		gconf_change_set_set_string(changeSet, keyPath, mkdg_value_get_string(ctx->value));
		break;
	    case MKDG_TYPE_STRING_LIST:
		gconf_change_set_set_string(changeSet, keyPath, mkdg_value_to_string(ctx->value, NULL));
		break;
	    case MKDG_TYPE_COLOR:
		gconf_change_set_set_string(changeSet, keyPath, mkdg_value_to_string(ctx->value, NULL));
		break;
//            case MKDG_TYPE_FILE:
//            case MKDG_TYPE_KEY:
            case MKDG_TYPE_NONE:
                break;
            default:
		cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_CANT_WRITE, "gconf_save_property()");
		break;
	}
	g_free(keyPath);
    }
    if (cfgErr!=NULL){
	mkdg_error_handle(cfgErr,error);
	return FALSE;
    }
    return TRUE;
}

static gboolean gconf_save_page(MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf,
	const gchar *pageName, GConfChangeSet *changeSet, MkdgError **error){
    MkdgError * cfgErr=NULL;
    MkdgNodeIter iter=mkdg_page_property_iter_init(configFile->configSet->config->mDialog, pageName);

    MkdgPropertyContext *ctx=NULL;
    while(mkdg_page_property_iter_has_next(iter)){
	ctx=mkdg_page_property_iter_next(&iter);
	g_assert(ctx);
	if (!gconf_save_property(configFile, configBuf, ctx, pageName, changeSet, &cfgErr)){
	    mkdg_error_handle(cfgErr,error);
	    if (configFile->configSet->config->flags & MKDG_CONFIG_FLAG_STOP_ON_ERROR){
		return FALSE;
	    }
	}
    }
    return TRUE;
}

static gboolean mkdg_config_file_gconf_save(MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf, MkdgError **error){
    MkdgError * cfgErr=NULL;
    GConfChangeSet *changeSet=gconf_change_set_new();
//    if (!mkdg_config_file_gconf_can_access(configFile, W_OK, error)){
//	cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_CANT_WRITE, "gconf_save()");
//	return FALSE;
//    }
    MkdgNodeIter iter=mkdg_page_iter_init(configFile->configSet->config->mDialog);
    while(mkdg_page_iter_has_next(iter)){
	GNode *pageNode=mkdg_page_iter_next(&iter);
	g_assert(pageNode);
	if (!gconf_save_page(configFile, configBuf, (gchar *) pageNode->data, changeSet, &cfgErr)){
	    mkdg_error_handle(cfgErr, error);
	    if (configFile->configSet->config->flags & MKDG_CONFIG_FLAG_STOP_ON_ERROR){
		return FALSE;
	    }
	}
    }
    gboolean ret=gconf_engine_commit_change_set (
	    (GConfEngine *) configFile->configSet->userData, changeSet, TRUE, &cfgErr);
    if (!ret){
	mkdg_error_handle(cfgErr, error);
	gconf_change_set_clear(changeSet);
    }
    gconf_change_set_unref(changeSet);
    return ret;
}
/*=== End Config interface callbacks ===*/

MkdgConfigFileInterface MKDG_CONFIG_FILE_INTERFACE_GCONF={
    mkdg_config_set_gconf_init,
    mkdg_config_set_gconf_finalize,
    mkdg_config_file_gconf_can_access,
    mkdg_config_file_gconf_create,
    mkdg_config_file_gconf_open,
    mkdg_config_file_gconf_close,
    mkdg_config_file_gconf_preload,
    mkdg_config_file_gconf_save,
    mkdg_config_file_gconf_get_pages,
    mkdg_config_file_gconf_get_keys,
    mkdg_config_file_gconf_get_value,
};

MkdgConfig *mkdg_config_use_gconf(Mkdg *mDialog){
    MkdgConfig *config=mkdg_config_new_full(mDialog, 0, &MKDG_CONFIG_FILE_INTERFACE_GCONF);
    return config;
}

/*=== Start Schema file generation ===*/
typedef enum{
    XML_TAG_TYPE_NO_TAG,
    XML_TAG_TYPE_EMPTY,
    XML_TAG_TYPE_SHORT,
    XML_TAG_TYPE_LONG,
    XML_TAG_TYPE_BEGIN_ONLY,
    XML_TAG_TYPE_END_ONLY,
} XmlTagsType;

typedef struct{
    const gchar *schemasHome;
    const gchar *owner;
    gchar **localeArray;
    gint indentSpace;
    FILE *outF;
} SchemasFileData;

static void append_indent_space(GString *strBuf, gint indentLevel, gint indentSpace){
    int i,indentLen=indentLevel*indentSpace;
    for(i=0;i<indentLen;i++){
	g_string_append_c(strBuf,' ');
    }
}

static GString *xml_tags_to_string(const gchar *tagName, XmlTagsType type,
	const gchar *attribute, const gchar *value,gint indentLevel, gint indentSpace){
    GString *strBuf=g_string_new(NULL);
    append_indent_space(strBuf,indentLevel,indentSpace);

    if(type!=XML_TAG_TYPE_NO_TAG){
	g_string_append_printf(strBuf,"<%s%s%s%s%s>",
		(type==XML_TAG_TYPE_END_ONLY) ? "/": "",
		(!mkdg_string_is_empty(tagName))? tagName : "",
		(!mkdg_string_is_empty(attribute)) ? " ":"",  (!mkdg_string_is_empty(attribute))? attribute : "",
		(type==XML_TAG_TYPE_EMPTY) ? "/": ""
	);
    }
    if (type==XML_TAG_TYPE_EMPTY)
	return strBuf;
    if (type==XML_TAG_TYPE_BEGIN_ONLY)
	return strBuf;
    if (type==XML_TAG_TYPE_END_ONLY)
	return strBuf;

    if (type==XML_TAG_TYPE_LONG){
	g_string_append_c(strBuf,'\n');
    }

    if (value){
	if (type==XML_TAG_TYPE_LONG || type==XML_TAG_TYPE_NO_TAG){
	    append_indent_space(strBuf,indentLevel+1, indentSpace);
	    int i, valueLen=strlen(value);
	    for(i=0;i<valueLen;i++){
		g_string_append_c(strBuf,value[i]);
		if (value[i]=='\n'){
		    append_indent_space(strBuf,indentLevel+1, indentSpace);
		}
	    }
	    g_string_append_c(strBuf,'\n');
	    if (type==XML_TAG_TYPE_LONG){
		append_indent_space(strBuf,indentLevel, indentSpace);
	    }
	}else{
	    g_string_append(strBuf,value);
	}
    }

    if (type==XML_TAG_TYPE_LONG || type==XML_TAG_TYPE_SHORT){
	g_string_append_printf(strBuf,"</%s>",tagName);
    }
    return strBuf;
}

static void xml_tags_write(SchemasFileData *sData, const gchar *tagName, XmlTagsType type,
	const gchar *attribute, const gchar *value){
    static int indentLevel=0;
    if (type==XML_TAG_TYPE_END_ONLY)
	indentLevel--;

    GString *strBuf=xml_tags_to_string(tagName, type, attribute, value, indentLevel, sData->indentSpace);
    MKDG_DEBUG_MSG(5,"[I5] config_gconf_xml_tags_write:%s",strBuf->str);
    fprintf(sData->outF,"%s\n",strBuf->str);

    if (type==XML_TAG_TYPE_BEGIN_ONLY)
	indentLevel++;
    g_string_free(strBuf,TRUE);
}

static void ctx_write_locale(MkdgPropertyContext *ctx, SchemasFileData *sData, const gchar *localeStr){
    gchar buf[50];
    g_snprintf(buf,50,"name=\"%s\"",localeStr);
    setlocale(LC_MESSAGES,localeStr);
    xml_tags_write(sData,"locale",XML_TAG_TYPE_BEGIN_ONLY,buf,NULL);
    xml_tags_write(sData,"short",XML_TAG_TYPE_SHORT,NULL, _(ctx->spec->label));
    xml_tags_write(sData,"long",XML_TAG_TYPE_LONG,NULL, _(ctx->spec->tooltip));
    xml_tags_write(sData,"locale",XML_TAG_TYPE_END_ONLY,NULL,NULL);
}

static void xml_each_page_each_property_func(Mkdg *mDialog, MkdgPropertyContext *ctx, gpointer userData){
    MKDG_DEBUG_MSG(5,"[I5] xml_each_page_each_property_func()");
    //    MKDG_DEBUG_MSG(5,"[I5] xml_each_page_each_property_func(%s, %s, )",
//            mDialog->title, ctx->spec->key);
    SchemasFileData *sData=(SchemasFileData *) userData;
    xml_tags_write(sData,"schema",XML_TAG_TYPE_BEGIN_ONLY,NULL,NULL);
    GString *strBuf=g_string_new(NULL);
    g_string_printf(strBuf, "/schemas%s/%s/%s",
	    sData->schemasHome,
	    (mkdg_string_is_empty(ctx->spec->pageName)) ? MKDG_PAGE_UNNAMED : ctx->spec->pageName,
	    ctx->spec->key);
    xml_tags_write(sData,"key",XML_TAG_TYPE_SHORT,NULL,strBuf->str);
    xml_tags_write(sData,"applyto",XML_TAG_TYPE_SHORT,NULL,strBuf->str + strlen("/schemas"));
    xml_tags_write(sData,"owner",XML_TAG_TYPE_SHORT,NULL,sData->owner);
    switch(ctx->spec->valueType){
	case MKDG_TYPE_BOOLEAN:
	    xml_tags_write(sData,"type",XML_TAG_TYPE_SHORT,NULL,"bool");
	    break;
	case MKDG_TYPE_INT:
	case MKDG_TYPE_UINT:
	case MKDG_TYPE_LONG:
	case MKDG_TYPE_ULONG:
	    xml_tags_write(sData,"type",XML_TAG_TYPE_SHORT,NULL,"int");
	    break;
	case MKDG_TYPE_FLOAT:
	case MKDG_TYPE_DOUBLE:
	    xml_tags_write(sData,"type",XML_TAG_TYPE_SHORT,NULL,"float");
	    break;
	case MKDG_TYPE_STRING:
	case MKDG_TYPE_COLOR:
	    xml_tags_write(sData,"type",XML_TAG_TYPE_SHORT,NULL,"string");
	    break;
	default:
	    break;
    }
    if (ctx->spec->defaultValue){
	xml_tags_write(sData,"default",XML_TAG_TYPE_SHORT,NULL,ctx->spec->defaultValue);
    }
    gboolean hasCLocale=FALSE;
    if (sData->localeArray){
	int i;
	for(i=0;sData->localeArray[i]!=NULL;i++){
	    if (strcmp(sData->localeArray[i],"C")==0){
		hasCLocale=TRUE;
	    }
	    ctx_write_locale(ctx,sData,sData->localeArray[i]);
	}
    }
    if (!hasCLocale)
	ctx_write_locale(ctx,sData,"C");
    setlocale(LC_ALL,NULL);
    xml_tags_write(sData,"schema",XML_TAG_TYPE_END_ONLY,NULL,NULL);
    g_string_free(strBuf,TRUE);
}
#undef STRING_BUFFER_SIZE_DEFAULT

gboolean mkdg_config_gconf_write_schemas_file
(Mkdg *mDialog, const gchar *filename, gint indentSpace, const gchar *schemasHome, const gchar *owner, const gchar *locales, GError **error){
    g_assert(filename);
    g_assert(schemasHome);
    g_assert(owner);
    MKDG_DEBUG_MSG(4,"[I4] config_gconf_write_schemas_file( , %s, %d, %s, %s, %s, )",
	    filename, indentSpace, schemasHome, owner, (locales)? locales: "");

    FILE *outF=fopen(filename,"w");
    GError *cfgErr=NULL;
    if (outF==NULL){
	cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_CANT_WRITE, "config_gconf_write_schemas_file");
	mkdg_error_handle(cfgErr, error);
	return FALSE;
    }
    SchemasFileData sData;
    sData.schemasHome=schemasHome;
    sData.owner=owner;
    if (locales){
	sData.localeArray=g_strsplit_set(locales, ";", -1);
    }else{
	sData.localeArray=NULL;
    }
    sData.outF=outF;
    xml_tags_write(&sData,"gconfschemafile",XML_TAG_TYPE_BEGIN_ONLY,NULL,NULL);
    xml_tags_write(&sData,"schemalist",XML_TAG_TYPE_BEGIN_ONLY,NULL,NULL);
    mkdg_foreach_page_foreach_property(mDialog, NULL, NULL, xml_each_page_each_property_func, &sData);
    xml_tags_write(&sData,"schemalist",XML_TAG_TYPE_END_ONLY,NULL,NULL);
    xml_tags_write(&sData,"gconfschemafile",XML_TAG_TYPE_END_ONLY,NULL,NULL);
    if (fclose(outF)){
	cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_CANT_WRITE, "config_gconf_write_schemas_file fclose()");
	mkdg_error_handle(cfgErr, error);
	return FALSE;
    }
    if (sData.localeArray){
	g_strfreev(sData.localeArray);
    }
    return TRUE;
}


