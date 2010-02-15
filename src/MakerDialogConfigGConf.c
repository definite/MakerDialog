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
#define GCONF_ERROR_FORMAT	"MakerDialogConfigGConf: In %s, %s Error: %s"

static MakerDialogError *maker_dialog_config_gconf_error_new(MakerDialogErrorCode code, const gchar *location, const gchar *cause, const gchar *errorMsg){
    return maker_dialog_error_new(code, GCONF_ERROR_FORMAT, location, cause,errorMsg);
}

static MakerDialogError *convert_error_code(GError *error, const gchar *location){
    if (error==NULL)
	return NULL;
    if (error->domain==MAKER_DIALOG_ERROR)
	return error;

    MakerDialogError *cfgErr=NULL;
    switch(error->code){
	case GCONF_ERROR_SUCCESS:
	    g_error_free(error);
	    return NULL;
	case GCONF_ERROR_IN_SHUTDOWN:
	    cfgErr=maker_dialog_config_gconf_error_new(MAKER_DIALOG_ERROR_CONFIG_NOT_READY, location, "Database is shutting down", error->message);
	    break;
	case GCONF_ERROR_NO_SERVER:
	    cfgErr=maker_dialog_config_gconf_error_new(MAKER_DIALOG_ERROR_CONFIG_NOT_READY, location, "No server", error->message);
	    break;
	case GCONF_ERROR_NO_PERMISSION:
	    cfgErr=maker_dialog_config_gconf_error_new(MAKER_DIALOG_ERROR_CONFIG_PERMISSION_DENY, location, "No permission", error->message);
	    break;
	case GCONF_ERROR_BAD_ADDRESS:
	    cfgErr=maker_dialog_config_gconf_error_new(MAKER_DIALOG_ERROR_CONFIG_PERMISSION_DENY, location, "Bad address", error->message);
	    break;
	case GCONF_ERROR_PARSE_ERROR:
	    cfgErr=maker_dialog_config_gconf_error_new(MAKER_DIALOG_ERROR_CONFIG_INVALID_FORMAT, location,  "Parse error", error->message);
	    break;
	case GCONF_ERROR_CORRUPT:
	    cfgErr=maker_dialog_config_gconf_error_new(MAKER_DIALOG_ERROR_CONFIG_INVALID_FORMAT, location,  "Corrupt", error->message);
	    break;
	case GCONF_ERROR_TYPE_MISMATCH:
	    cfgErr=maker_dialog_config_gconf_error_new(MAKER_DIALOG_ERROR_CONFIG_INVALID_FORMAT, location, "Type mismatch", error->message);
	    break;
	case GCONF_ERROR_BAD_KEY:
	    cfgErr=maker_dialog_config_gconf_error_new(MAKER_DIALOG_ERROR_CONFIG_INVALID_KEY, location, "Bad key", error->message);
	    break;
	case GCONF_ERROR_IS_DIR:
	    cfgErr=maker_dialog_config_gconf_error_new(MAKER_DIALOG_ERROR_CONFIG_INVALID_KEY, location, "Is dir", error->message);
	    break;
	case GCONF_ERROR_IS_KEY:
	    cfgErr=maker_dialog_config_gconf_error_new(MAKER_DIALOG_ERROR_CONFIG_INVALID_PAGE, location, "Is Key", error->message);
	    break;
	case GCONF_ERROR_OVERRIDDEN:
	    cfgErr=maker_dialog_config_gconf_error_new(MAKER_DIALOG_ERROR_CONFIG_CANT_WRITE, location, "Overridden", error->message);
	    break;
	case GCONF_ERROR_LOCK_FAILED:
	    cfgErr=maker_dialog_config_gconf_error_new(MAKER_DIALOG_ERROR_CONFIG_CANT_WRITE, location, "Lock failed", error->message);
	    break;
	case GCONF_ERROR_NO_WRITABLE_DATABASE:
	    cfgErr=maker_dialog_config_gconf_error_new(MAKER_DIALOG_ERROR_CONFIG_CANT_WRITE, location, "No writable database", error->message);
	    break;
	case GCONF_ERROR_OAF_ERROR:
	    cfgErr=maker_dialog_config_gconf_error_new(MAKER_DIALOG_ERROR_CONFIG_OTHER, location, "liboaf error", error->message);
	    break;
	case GCONF_ERROR_LOCAL_ENGINE:
	    cfgErr=maker_dialog_config_gconf_error_new(MAKER_DIALOG_ERROR_CONFIG_OTHER, location, "Trying to use remote method on local engine", error->message);
	    break;
	case GCONF_ERROR_FAILED :
	    cfgErr=maker_dialog_config_gconf_error_new(MAKER_DIALOG_ERROR_CONFIG_OTHER, location, "Failed", error->message);
	    break;
	default:
	    cfgErr=maker_dialog_config_gconf_error_new(MAKER_DIALOG_ERROR_CONFIG_OTHER, location, "Other error", error->message);
	    break;
    }
    g_error_free(error);
    return cfgErr;
}
/*=== End Error handling ===*/

/*=== Start Config interface callbacks ===*/
static gboolean maker_dialog_config_set_gconf_init(MakerDialogConfigSet *configSet, MakerDialogError **error){
    g_assert(configSet);
    configSet->userData=gconf_engine_get_default();
    if (configSet->userData==NULL)
	return FALSE;
    MakerDialogConfigFile *configFile=maker_dialog_config_file_new(configSet->filePattern,configSet);
    g_ptr_array_add(configSet->fileArray,configFile);
    configSet->writeIndex=0;
    return TRUE;
}

static void maker_dialog_config_set_gconf_finalize(MakerDialogConfigSet *configSet){
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

static gchar **config_gconf_get_page_list(MakerDialogConfigFile *configFile, MakerDialogError **error){
    MakerDialogError *cfgErr=NULL;
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
	    MAKER_DIALOG_DEBUG_MSG(5, "[I5] config_gconf_get_page_list(%s, ) path=%s", configFile->path, pagePath);
	    page=g_path_get_basename (pagePath);
	    g_ptr_array_add(ptrArray, page);
	}
	/* Free sList */
	gconf_gslist_free(sListHead, g_free);
    }else{
	maker_dialog_error_handle(cfgErr,error);
    }
    g_ptr_array_add(ptrArray, NULL);
    return (gchar **) g_ptr_array_free(ptrArray, FALSE);
}

static gchar **maker_dialog_config_file_gconf_get_pages(MakerDialogConfigFile *configFile, MakerDialogError **error){
    return config_gconf_get_page_list(configFile, error);
}

static guint maker_dialog_config_file_gconf_get_access(MakerDialogConfigFile *configFile, MakerDialogError **error){
    GConfEngine *engine=(GConfEngine *)configFile->configSet->userData;
    guint accessFlags=0;
    if (gconf_engine_dir_exists (engine, configFile->path, error)){
	accessFlags|=F_OK | R_OK;
	/* TODO: Test write */
	/* Assume it is writable */
	if (!(configFile->flags &MAKER_DIALOG_CONFIG_FILE_FLAG_CANT_WRITE))
	    accessFlags|=W_OK;
    }
    return accessFlags;
}

static gboolean maker_dialog_config_file_gconf_can_access(MakerDialogConfigFile *configFile, guint permission, MakerDialogError **error){
    if (!configFile->configSet->userData)
	return FALSE;
    guint flags=maker_dialog_config_file_gconf_get_access(configFile, error);
    MAKER_DIALOG_DEBUG_MSG(5, "**** [I5] gconf_can_access()", flags);
    return maker_dialog_has_all_flags(permission, flags);
}

static gboolean maker_dialog_config_file_gconf_create(MakerDialogConfigFile *configFile, MakerDialogError **error){
    g_assert(configFile->configSet->userData);
    MAKER_DIALOG_DEBUG_MSG(5, "[I5] config_gconf_create(%s, )", configFile->path);
    GConfEngine *engine=(GConfEngine *)configFile->configSet->userData;
    MakerDialogError *cfgErr=NULL;
    if (gconf_engine_dir_exists (engine, configFile->path, &cfgErr)){
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_ALREADY_EXIST, "Path %s already exists!", configFile->path);
    }else{
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_OTHER, "No create mode with GConf back-end!");
    }
    maker_dialog_error_handle(cfgErr,error);
    return FALSE;
}

static gboolean maker_dialog_config_file_gconf_open(MakerDialogConfigFile *configFile, MakerDialogError **error){
    g_assert(configFile->configSet->userData);
    MAKER_DIALOG_DEBUG_MSG(5, "[I5] config_gconf_open(%s, )", configFile->path);
    GConfEngine *engine=(GConfEngine *)configFile->configSet->userData;
    MakerDialogError *cfgErr=NULL;
    if (!gconf_engine_dir_exists (engine, configFile->path, &cfgErr)){
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_NO_FILE, "Path %s does not exist!", configFile->path);
	maker_dialog_error_handle(cfgErr,error);
	return FALSE;
    }
    gconf_engine_ref(engine);
    configFile->fileObj=(gpointer) engine;
    return TRUE;
}

static gboolean maker_dialog_config_file_gconf_close(MakerDialogConfigFile *configFile, MakerDialogError **error){
    configFile->fileObj=NULL;
    gconf_engine_unref((GConfEngine *)configFile->configSet->userData);
    return TRUE;
}

static gboolean gconf_preload_property(MakerDialogConfigFile *configFile, MakerDialogConfigBuffer *configBuf, GSList *entryList, MakerDialogError **error){
    MakerDialogError * cfgErr=NULL;
    GConfEntry *cfgEntry=(GConfEntry *) entryList->data;
    gchar *key=g_path_get_basename(cfgEntry->key);
    MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(configFile->configSet->config->mDialog,key);
    if (ctx==NULL){
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_INVALID_KEY, "config_gconf_preload_property() key=%s, cfgEntry->key=%s", key, cfgEntry->key);
	maker_dialog_error_handle(cfgErr, error);
	g_free(key);
	if (configFile->configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR ){
	    return FALSE;
	}
	return TRUE;
    }
    MAKER_DIALOG_DEBUG_MSG(3, "[I3] gconf_preload_property(%s,  , %s, )", configFile->path, ctx->spec->key);
    gchar *strValue=NULL;

    MkdgValue *mValue=maker_dialog_value_new(ctx->spec->valueType, NULL);
    switch(ctx->spec->valueType){
	case MKDG_TYPE_BOOLEAN:
	    maker_dialog_value_set_boolean(mValue, gconf_value_get_bool(cfgEntry->value));
	    break;
	case MKDG_TYPE_INT:
	    maker_dialog_value_set_int(mValue, gconf_value_get_int(cfgEntry->value));
	    break;
	case MKDG_TYPE_INT32:
	    maker_dialog_value_set_int32(mValue, gconf_value_get_int(cfgEntry->value));
	    break;
	case MKDG_TYPE_INT64:
	    maker_dialog_value_set_int64(mValue, gconf_value_get_int(cfgEntry->value));
	    break;
	case MKDG_TYPE_UINT:
	    maker_dialog_value_set_uint(mValue, (guint) gconf_value_get_int(cfgEntry->value));
	    break;
	case MKDG_TYPE_UINT32:
	    maker_dialog_value_set_uint32(mValue, (guint32) gconf_value_get_int(cfgEntry->value));
	    break;
	case MKDG_TYPE_UINT64:
	    maker_dialog_value_set_uint64(mValue, (guint64) gconf_value_get_int(cfgEntry->value));
	    break;
	case MKDG_TYPE_LONG:
	    maker_dialog_value_set_long(mValue,  gconf_value_get_int(cfgEntry->value));
	    break;
	case MKDG_TYPE_ULONG:
	    maker_dialog_value_set_ulong(mValue, (gulong) gconf_value_get_int(cfgEntry->value));
	    break;
	case MKDG_TYPE_FLOAT:
	    maker_dialog_value_set_float(mValue, (gfloat) gconf_value_get_float(cfgEntry->value));
	    break;
	case MKDG_TYPE_DOUBLE:
	    maker_dialog_value_set_double(mValue, gconf_value_get_float(cfgEntry->value));
	    break;
	case MKDG_TYPE_STRING:
	    strValue=(gchar *) gconf_value_get_string(cfgEntry->value);
	    MAKER_DIALOG_DEBUG_MSG(6, "[I6] gconf_preload_property() strValue=%s", strValue);
	    maker_dialog_value_from_string(mValue, strValue, NULL);
	    break;
	case MKDG_TYPE_STRING_LIST:
	    maker_dialog_value_from_string(mValue, gconf_value_get_string(cfgEntry->value),NULL);
	    break;
	case MKDG_TYPE_COLOR:
	    maker_dialog_value_from_string(mValue, gconf_value_get_string(cfgEntry->value), NULL);
	    break;
	default:
	    break;
    }
    MAKER_DIALOG_DEBUG_MSG(3,
	    configFile->path, maker_dialog_value_to_string(mValue, ctx->spec->toStringFormat), ctx->spec->key );
    maker_dialog_config_buffer_insert(configBuf, ctx->spec->key, mValue);
    return TRUE;
}

static gboolean gconf_preload_page(MakerDialogConfigFile *configFile, MakerDialogConfigBuffer *configBuf, const gchar *pagePath, MakerDialogError **error){
    MakerDialogError * cfgErr=NULL;
    GSList *entryListHead=gconf_engine_all_entries((GConfEngine *)configFile->configSet->userData, pagePath, &cfgErr);
    if (cfgErr!=NULL){
	maker_dialog_error_handle(cfgErr, error);
	if (configFile->configSet->config->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
	    return FALSE;
	}
    }
    GSList *entryList=entryListHead;
    for(;entryList!=NULL;entryList=g_slist_next(entryList)){
	gconf_preload_property(configFile, configBuf, entryList, &cfgErr);
	if (cfgErr!=NULL){
	    maker_dialog_error_handle(cfgErr, error);
	    if (configFile->configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR ){
		gconf_gslist_free(entryListHead, gconf_entry_free_wrapper);
		return FALSE;
	    }
	}
    }
    gconf_gslist_free(entryListHead, gconf_entry_free_wrapper);
    return TRUE;
}

static gboolean maker_dialog_config_file_gconf_preload(MakerDialogConfigFile *configFile, MakerDialogConfigBuffer *configBuf, MakerDialogError **error){
    g_assert(configFile->configSet->userData);
    MAKER_DIALOG_DEBUG_MSG(5, "[I5] config_gconf_preload(%s, , )", configFile->path);
    GConfEngine *engine=(GConfEngine *)configFile->configSet->userData;
    MakerDialogError * cfgErr=NULL, *cfgErr_prep=NULL;
    GSList *dirListHead=gconf_engine_all_dirs(engine,configFile->path,&cfgErr_prep);
    if (cfgErr_prep!=NULL){
	cfgErr=convert_error_code(cfgErr_prep, "config_file_gconf_preload()");
	maker_dialog_error_handle(cfgErr,error);
        return FALSE;
    }
    GSList *dirList=dirListHead;
    gchar *pagePath;
    g_assert(!cfgErr);
    for(;dirList!=NULL; dirList=g_slist_next(dirList)){
	pagePath=(gchar *)dirList->data;
	gconf_preload_page(configFile, configBuf, pagePath, &cfgErr);
	if (cfgErr){
	    maker_dialog_error_handle(cfgErr, error);
	    if (configFile->configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR ){
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
static gboolean gconf_save_property(MakerDialogConfigFile *configFile, MakerDialogConfigBuffer *configBuf,
	MakerDialogPropertyContext *ctx, const gchar *pageName, GConfChangeSet *changeSet, MakerDialogError **error){
    MakerDialogError * cfgErr=NULL;
    /* Check whether the line need to be saved */
    MkdgValue *bufValue=maker_dialog_config_buffer_lookup(configBuf, ctx->spec->key);
    gboolean needChange=TRUE;
    if (bufValue!=NULL){
	if (maker_dialog_value_compare(bufValue, ctx->value, NULL)==0){
	    /* Same value already in gconf */
	    MAKER_DIALOG_DEBUG_MSG(4, "[I4] gconf_save_property() same value already in gconf, no need to change.");
	    needChange=FALSE;
	}
    }
    MAKER_DIALOG_DEBUG_MSG(4, "[I4] gconf_save_property(%s, , %s, ) page=%s needChange=%s",
	    configFile->path, ctx->spec->key, (ctx->spec->pageName)? ctx->spec->pageName: "-", (needChange)? "TRUE": "FALSE");

    if (needChange){
	gchar *keyPath=g_build_filename(configFile->path,pageName,ctx->spec->key, NULL);

	switch (ctx->spec->valueType){
	    case MKDG_TYPE_BOOLEAN:
		gconf_change_set_set_bool(changeSet, keyPath, maker_dialog_value_get_boolean(ctx->value));
		break;
	    case MKDG_TYPE_INT:
		gconf_change_set_set_int(changeSet, keyPath, maker_dialog_value_get_int(ctx->value));
		break;
	    case MKDG_TYPE_INT32:
		gconf_change_set_set_int(changeSet, keyPath, maker_dialog_value_get_int32(ctx->value));
		break;
	    case MKDG_TYPE_INT64:
		gconf_change_set_set_int(changeSet, keyPath, maker_dialog_value_get_int64(ctx->value));
		break;
	    case MKDG_TYPE_UINT:
		gconf_change_set_set_int(changeSet, keyPath, maker_dialog_value_get_uint(ctx->value));
		break;
	    case MKDG_TYPE_UINT32:
		gconf_change_set_set_int(changeSet, keyPath, maker_dialog_value_get_uint32(ctx->value));
		break;
	    case MKDG_TYPE_UINT64:
		gconf_change_set_set_int(changeSet, keyPath, maker_dialog_value_get_uint64(ctx->value));
		break;
	    case MKDG_TYPE_LONG:
		gconf_change_set_set_int(changeSet, keyPath, maker_dialog_value_get_long(ctx->value));
		break;
	    case MKDG_TYPE_ULONG:
		gconf_change_set_set_int(changeSet, keyPath, maker_dialog_value_get_long(ctx->value));
		break;
	    case MKDG_TYPE_FLOAT:
		gconf_change_set_set_float(changeSet, keyPath, maker_dialog_value_get_float(ctx->value));
		break;
	    case MKDG_TYPE_DOUBLE:
		gconf_change_set_set_float(changeSet, keyPath, maker_dialog_value_get_double(ctx->value));
		break;
	    case MKDG_TYPE_STRING:
		gconf_change_set_set_string(changeSet, keyPath, maker_dialog_value_get_string(ctx->value));
		break;
	    case MKDG_TYPE_STRING_LIST:
		gconf_change_set_set_string(changeSet, keyPath, maker_dialog_value_to_string(ctx->value, NULL));
		break;
	    case MKDG_TYPE_COLOR:
		gconf_change_set_set_string(changeSet, keyPath, maker_dialog_value_to_string(ctx->value, NULL));
		break;
//            case MKDG_TYPE_FILE:
//            case MKDG_TYPE_KEY:
            case MKDG_TYPE_NONE:
                break;
            default:
		cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_CANT_WRITE, "gconf_save_property()");
		break;
	}
	g_free(keyPath);
    }
    if (cfgErr!=NULL){
	maker_dialog_error_handle(cfgErr,error);
	return FALSE;
    }
    return TRUE;
}

static gboolean gconf_save_page(MakerDialogConfigFile *configFile, MakerDialogConfigBuffer *configBuf,
	const gchar *pageName, GConfChangeSet *changeSet, MakerDialogError **error){
    MakerDialogError * cfgErr=NULL;
    MakerDialogNodeIter iter=maker_dialog_page_property_iter_init(configFile->configSet->config->mDialog, pageName);

    MakerDialogPropertyContext *ctx=NULL;
    while(maker_dialog_page_property_iter_has_next(iter)){
	ctx=maker_dialog_page_property_iter_next(&iter);
	g_assert(ctx);
	if (!gconf_save_property(configFile, configBuf, ctx, pageName, changeSet, &cfgErr)){
	    maker_dialog_error_handle(cfgErr,error);
	    if (configFile->configSet->config->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		return FALSE;
	    }
	}
    }
    return TRUE;
}

static gboolean maker_dialog_config_file_gconf_save(MakerDialogConfigFile *configFile, MakerDialogConfigBuffer *configBuf, MakerDialogError **error){
    MakerDialogError * cfgErr=NULL;
    GConfChangeSet *changeSet=gconf_change_set_new();
//    if (!maker_dialog_config_file_gconf_can_access(configFile, W_OK, error)){
//	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_CANT_WRITE, "gconf_save()");
//	return FALSE;
//    }
    MakerDialogNodeIter iter=maker_dialog_page_iter_init(configFile->configSet->config->mDialog);
    while(maker_dialog_page_iter_has_next(iter)){
	GNode *pageNode=maker_dialog_page_iter_next(&iter);
	g_assert(pageNode);
	if (!gconf_save_page(configFile, configBuf, (gchar *) pageNode->data, changeSet, &cfgErr)){
	    maker_dialog_error_handle(cfgErr, error);
	    if (configFile->configSet->config->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		return FALSE;
	    }
	}
    }
    gboolean ret=gconf_engine_commit_change_set (
	    (GConfEngine *) configFile->configSet->userData, changeSet, TRUE, &cfgErr);
    if (!ret){
	maker_dialog_error_handle(cfgErr, error);
	gconf_change_set_clear(changeSet);
    }
    gconf_change_set_unref(changeSet);
    return ret;
}
/*=== End Config interface callbacks ===*/

MakerDialogConfigFileInterface MAKER_DIALOG_CONFIG_FILE_INTERFACE_GCONF={
    maker_dialog_config_set_gconf_init,
    maker_dialog_config_set_gconf_finalize,
    maker_dialog_config_file_gconf_can_access,
    maker_dialog_config_file_gconf_create,
    maker_dialog_config_file_gconf_open,
    maker_dialog_config_file_gconf_close,
    maker_dialog_config_file_gconf_preload,
    maker_dialog_config_file_gconf_save,
    maker_dialog_config_file_gconf_get_pages
};

MakerDialogConfig *maker_dialog_config_use_gconf(MakerDialog *mDialog){
    MakerDialogConfig *config=maker_dialog_config_new_full(mDialog, 0, &MAKER_DIALOG_CONFIG_FILE_INTERFACE_GCONF);
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
		(!maker_dialog_string_is_empty(tagName))? tagName : "",
		(!maker_dialog_string_is_empty(attribute)) ? " ":"",  (!maker_dialog_string_is_empty(attribute))? attribute : "",
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
    MAKER_DIALOG_DEBUG_MSG(5,"[I5] config_gconf_xml_tags_write:%s",strBuf->str);
    fprintf(sData->outF,"%s\n",strBuf->str);

    if (type==XML_TAG_TYPE_BEGIN_ONLY)
	indentLevel++;
    g_string_free(strBuf,TRUE);
}

static void ctx_write_locale(MakerDialogPropertyContext *ctx, SchemasFileData *sData, const gchar *localeStr){
    gchar buf[50];
    g_snprintf(buf,50,"name=\"%s\"",localeStr);
    setlocale(LC_MESSAGES,localeStr);
    xml_tags_write(sData,"locale",XML_TAG_TYPE_BEGIN_ONLY,buf,NULL);
    xml_tags_write(sData,"short",XML_TAG_TYPE_SHORT,NULL, _(ctx->spec->label));
    xml_tags_write(sData,"long",XML_TAG_TYPE_LONG,NULL, _(ctx->spec->tooltip));
    xml_tags_write(sData,"locale",XML_TAG_TYPE_END_ONLY,NULL,NULL);
}

static void xml_each_page_each_property_func(MakerDialog *mDialog, MakerDialogPropertyContext *ctx, gpointer userData){
    MAKER_DIALOG_DEBUG_MSG(5,"[I5] xml_each_page_each_property_func()");
    //    MAKER_DIALOG_DEBUG_MSG(5,"[I5] xml_each_page_each_property_func(%s, %s, )",
//            mDialog->title, ctx->spec->key);
    SchemasFileData *sData=(SchemasFileData *) userData;
    xml_tags_write(sData,"schema",XML_TAG_TYPE_BEGIN_ONLY,NULL,NULL);
    GString *strBuf=g_string_new(NULL);
    g_string_printf(strBuf, "/schemas%s/%s/%s",
	    sData->schemasHome,
	    (maker_dialog_string_is_empty(ctx->spec->pageName)) ? MAKER_DIALOG_PAGE_UNNAMED : ctx->spec->pageName,
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

gboolean maker_dialog_config_gconf_write_schemas_file
(MakerDialog *mDialog, const gchar *filename, gint indentSpace, const gchar *schemasHome, const gchar *owner, const gchar *locales, GError **error){
    g_assert(filename);
    g_assert(schemasHome);
    g_assert(owner);
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] config_gconf_write_schemas_file( , %s, %d, %s, %s, %s, )",
	    filename, indentSpace, schemasHome, owner, (locales)? locales: "");

    FILE *outF=fopen(filename,"w");
    GError *cfgErr=NULL;
    if (outF==NULL){
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_CANT_WRITE, "config_gconf_write_schemas_file");
	maker_dialog_error_handle(cfgErr, error);
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
    maker_dialog_foreach_page_foreach_property(mDialog, NULL, NULL, xml_each_page_each_property_func, &sData);
    xml_tags_write(&sData,"schemalist",XML_TAG_TYPE_END_ONLY,NULL,NULL);
    xml_tags_write(&sData,"gconfschemafile",XML_TAG_TYPE_END_ONLY,NULL,NULL);
    if (fclose(outF)){
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_CANT_WRITE, "config_gconf_write_schemas_file fclose()");
	maker_dialog_error_handle(cfgErr, error);
	return FALSE;
    }
    if (sData.localeArray){
	g_strfreev(sData.localeArray);
    }
    return TRUE;
}


