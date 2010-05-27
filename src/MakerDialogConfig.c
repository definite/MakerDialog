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
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <glib.h>
#include <glib/gstdio.h>
#include "Mkdg.h"
#include "MkdgTypes.h"

MkdgConfig *maker_dialog_config_new(Mkdg *mDialog){
    return maker_dialog_config_new_full(mDialog, 0, NULL);
}

MkdgConfig *maker_dialog_config_new_full(
	Mkdg *mDialog,
	MkdgConfigFlags flags, MkdgConfigFileInterface *configInterface){
    MkdgConfig *config=g_new(MkdgConfig,1);
    config->mDialog=mDialog;
    config->pageSetTable=g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);
    config->setArray=g_ptr_array_sized_new(5);
    config->configInterface=configInterface;
    mDialog->config=config;
    return config;
}

static void maker_dialog_config_free_set(gpointer data, gpointer userData){
    maker_dialog_config_set_free((MkdgConfigSet *) data);
}

void maker_dialog_config_free(MkdgConfig *config){
    maker_dialog_config_close_all(config, NULL);
    g_hash_table_destroy(config->pageSetTable);
    g_ptr_array_foreach(config->setArray, maker_dialog_config_free_set, NULL);
    g_ptr_array_free(config->setArray, TRUE);
    g_free(config);
}

void maker_dialog_config_add_config_set(MkdgConfig *config, MkdgConfigSet *configSet, MkdgError **error){
    g_assert(configSet);
    MkdgError *cfgErr=NULL;
    if (configSet->configInterface==NULL || !(configSet->status & MAKER_DIALOG_CONFIG_SET_STATUS_HAS_LOCAL_SETTING)){
	configSet->configInterface=config->configInterface;
	configSet->status &=  ~MAKER_DIALOG_CONFIG_SET_STATUS_HAS_LOCAL_SETTING;
    }
    configSet->configInterface->config_set_init(configSet, &cfgErr);
    maker_dialog_error_handle(cfgErr, error);
    g_ptr_array_add(config->setArray,configSet);
    if (configSet->pageNames){
	gint i;
	for (i=0; configSet->pageNames[i]!=NULL; i++){
	    g_hash_table_insert(config->pageSetTable, (gpointer) configSet->pageNames[i], configSet);
	}
    }
    configSet->config=config;
}

/*=== Start config file/set foreach callback and functions ===*/
typedef gboolean (* MkdgConfigSetEachFileFunc)(
	MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf,
	gpointer userData, MkdgError **error);

static gboolean maker_dialog_config_set_foreach_file(MkdgConfigSet *configSet,  MkdgConfigBuffer *configBuf,
	MkdgConfigSetEachFileFunc func, gint untilIndex, gpointer userData, MkdgError **error){
    MkdgError *cfgErr=NULL;
    MAKER_DIALOG_DEBUG_MSG(5,"[I5] config_set_foreach_file( , , , %d, , )",untilIndex);
    gsize i;
    for(i=0;i<configSet->fileArray->len;i++){
	if (untilIndex>=0 && i>=untilIndex){
	    break;
	}
	func(g_ptr_array_index(configSet->fileArray,i), configBuf, userData, &cfgErr);
	if (cfgErr){
	    maker_dialog_error_handle(cfgErr, error);
	    if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR)
		return FALSE;
	}
    }
    return TRUE;
}

typedef gboolean (* MkdgConfigSetForeachCallbackFunc)(
	MkdgConfigSet *configSet, gint untillIndex, gpointer userData, MkdgError **error);

static gboolean maker_dialog_foreach_config_set(MkdgConfig *config,
	MkdgConfigSetForeachCallbackFunc func, gint untilIndex, gpointer userData, MkdgError **error){
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] foreach_config_set( , , %d, , )",untilIndex);
    MkdgError *cfgErr=NULL;
    if (config->setArray->len<=0){
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_NO_CONFIG_SET, "foreach_config_set()");
	maker_dialog_error_handle(cfgErr, error);
	return FALSE;
    }
    gboolean ret;
    gsize i;
    for(i=0;i<config->setArray->len;i++){
	if (untilIndex>=0 && i>=untilIndex){
	    break;
	}
	MkdgConfigSet *configSet=g_ptr_array_index(config->setArray,i);
	ret=func(configSet, untilIndex, userData, &cfgErr);
	if (cfgErr!=NULL){
	    maker_dialog_error_handle(cfgErr, error);
	    if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		return FALSE;
	    }
	    cfgErr=NULL;
	}
    }
    return TRUE;
}
/*=== End config file/set foreach callback and functions ===*/

/*=== Start open/close functions ===*/
static gboolean maker_dialog_config_file_open(
	MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf, gpointer userData, MkdgError **error){
    MAKER_DIALOG_DEBUG_MSG(5,"[I5] config_file_open(%s, , , )",configFile->path);
    if (!configFile->configSet->configInterface->config_file_can_access(configFile, R_OK, error)){
	return FALSE;
    }
    return (configFile->configSet->configInterface->config_file_open(configFile, error)) ? TRUE: FALSE;
}

static gboolean maker_dialog_config_set_open(MkdgConfigSet *configSet, gint untilIndex, gpointer userData, MkdgError **error){
    /* It is possible that configSet does not have any files. */
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] config_set_open(%s, %d,  , )",
	    (configSet->pageNames)? configSet->pageNames[0]  : "NULL",untilIndex);
    return maker_dialog_config_set_foreach_file(configSet, NULL, maker_dialog_config_file_open, untilIndex, userData, error);
}

gboolean maker_dialog_config_open_all(MkdgConfig *config, MkdgError **error){
    return maker_dialog_foreach_config_set(config, maker_dialog_config_set_open, -1, NULL, error);
}

static gboolean maker_dialog_config_file_close(
	MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf, gpointer userData, MkdgError **error){
    return configFile->configSet->configInterface->config_file_close(configFile, error);
}

static gboolean maker_dialog_config_set_close(
	MkdgConfigSet *configSet, gint untilIndex, gpointer userData, MkdgError **error){
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] config_set_close(%s, %d,  , )",
	    (configSet->pageNames)? configSet->pageNames[0]  : "NULL",untilIndex);
    return maker_dialog_config_set_foreach_file(configSet, NULL, maker_dialog_config_file_close, untilIndex, userData,error);
}

gboolean maker_dialog_config_close_all(MkdgConfig *config, MkdgError **error){
    return maker_dialog_foreach_config_set(config, maker_dialog_config_set_close, -1, NULL, error);
}
/*=== End open/close functions ===*/

/*
 * Pre-Load settings in configuration set files.
 */
static gboolean maker_dialog_config_file_preload(
	MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf, gpointer userData, MkdgError **error){
    MAKER_DIALOG_DEBUG_MSG(5,"[I5] config_file_preload(%s, , , )", configFile->path);
    return configFile->configSet->configInterface->config_file_preload(configFile, configBuf, error);
}

static gboolean maker_dialog_config_set_preload(
	MkdgConfigSet *configSet, MkdgConfigBuffer *configBuf, gint untilIndex, gpointer userData, MkdgError **error){
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] config_set_preload(%s, , %d,  , )",
	    (configSet->pageNames)? configSet->pageNames[0]  : "NULL",untilIndex);
    return maker_dialog_config_set_foreach_file(
	    configSet, configBuf, maker_dialog_config_file_preload, untilIndex, userData, error);
}

static void maker_dialog_config_load_buffer(gpointer hashKey, gpointer value, gpointer userData){
    gchar *key= (gchar *) hashKey;
    MAKER_DIALOG_DEBUG_MSG(5,"[I5] config_load_buffer (%s, , )", key);
    MkdgValue *mValue=(MkdgValue *) value;
    MkdgConfigSet *configSet=(MkdgConfigSet *) userData;
    MkdgPropertyContext *ctx=maker_dialog_get_property_context(configSet->config->mDialog, key);
    maker_dialog_set_value(configSet->config->mDialog, key, mValue);
    if (!(configSet->flags & MAKER_DIALOG_CONFIG_FLAG_NO_APPLY)){
	maker_dialog_apply_value(configSet->config->mDialog,ctx->spec->key);
    }
}

static gboolean maker_dialog_config_set_load(
	MkdgConfigSet *configSet, gint untilIndex, gpointer userData, MkdgError **error){
    MkdgConfigBuffer *configBuf=maker_dialog_config_buffer_new();
    gboolean ret=maker_dialog_config_set_preload(configSet, configBuf, untilIndex, userData, error);
    MAKER_DIALOG_DEBUG_MSG(5,"[I5] config_set_load(%s, %d, , ) preload done.",
	    (configSet->pageNames)? configSet->pageNames[0]  : "NULL",untilIndex);
    if (!ret && configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
	return ret;
    }
    g_hash_table_foreach(configBuf->keyValueTable,maker_dialog_config_load_buffer, configSet);
    maker_dialog_config_buffer_free(configBuf);
    MAKER_DIALOG_DEBUG_MSG(5,"[I5]  config_set_load() load done.");
    return ret;
}

gboolean maker_dialog_config_load_all(MkdgConfig *config, MkdgError **error){
    MkdgError *cfgErr=NULL;
    if (config->setArray->len<=0){
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_NO_CONFIG_SET, "config_load_page()");
	maker_dialog_error_handle(cfgErr, error);
	return FALSE;
    }
    return maker_dialog_foreach_config_set(config, maker_dialog_config_set_load, -1, NULL, error);
}

gboolean maker_dialog_config_load_page(MkdgConfig *config, const gchar *pageName, MkdgError **error){
    MkdgError *cfgErr=NULL;
    if (config->setArray->len<=0){
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_NO_CONFIG_SET, "config_load_page()");
	maker_dialog_error_handle(cfgErr, error);
	return FALSE;
    }
    MkdgConfigSet *configSet=NULL;
    if (g_hash_table_size(config->pageSetTable)){
	configSet=(MkdgConfigSet *) g_hash_table_lookup(config->pageSetTable, pageName);
    }else{
	configSet=(MkdgConfigSet *) g_ptr_array_index(config->setArray,0);
    }
    return maker_dialog_config_set_load(configSet, -1, (gpointer) pageName, error);
}

/*
 * Save settings in configuration set files.
 *
 * This functions save setting from Mkdg instance to configuration set files,
 * set the values to corresponding keys.
 *
 * @param configSet A Mkdg configuration set.
 * @return MAKER_DIALOG_CONFIG_OK if success; non-zero ::MkdgError *error code otherwise.
 */
static gboolean maker_dialog_config_set_save(MkdgConfigSet *configSet, gint untilIndex, gpointer userData, MkdgError **error){
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] config_set_save(%s, %d, , ) writeIndex=%d",(configSet->pageNames)? configSet->pageNames[0]  : "NULL",untilIndex, configSet->writeIndex);
    g_assert(configSet->writeIndex<configSet->fileArray->len);
    MkdgError *cfgErr=NULL;
    gboolean result=TRUE;

    if (configSet->writeIndex<0 ){
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_NO_CONFIG_SET, "config_set_save()");
	maker_dialog_error_handle(cfgErr, error);
	return FALSE;
    }

    MkdgConfigBuffer *configBuf=maker_dialog_config_buffer_new();
    gboolean ret=maker_dialog_config_set_preload(configSet, configBuf, configSet->writeIndex, userData, &cfgErr);
    if (cfgErr){
	maker_dialog_error_handle(cfgErr, error);
	if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
	    goto CONFIG_SET_SAVE_END;
	}
    }

    MkdgConfigFile *configFile=g_ptr_array_index(configSet->fileArray, configSet->writeIndex);
    ret=configSet->configInterface->config_file_save(configFile, configBuf, &cfgErr);
    if (cfgErr){
	maker_dialog_error_handle(cfgErr, error);
    }
CONFIG_SET_SAVE_END:
    maker_dialog_config_buffer_free(configBuf);
    return result;
}

gboolean maker_dialog_config_save_all(MkdgConfig *config, MkdgError **error){
    return maker_dialog_foreach_config_set(config, maker_dialog_config_set_save, -1, NULL, error);
}

gboolean maker_dialog_config_save_page(MkdgConfig *config, const gchar *pageName, MkdgError **error){
    MkdgError *cfgErr=NULL;
    if (config->setArray->len<=0){
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_NO_CONFIG_SET, "config_save_page()");
	maker_dialog_error_handle(cfgErr, error);
	return FALSE;
    }
    MkdgConfigSet *configSet=NULL;
    if (g_hash_table_size(config->pageSetTable)){
	configSet=(MkdgConfigSet *) g_hash_table_lookup(config->pageSetTable, pageName);
    }else{
	configSet=(MkdgConfigSet *) g_ptr_array_index(config->setArray,0);
    }
    return maker_dialog_config_set_save(configSet, -1, (gpointer) pageName, error);
}

static gboolean merge_str_lists(GPtrArray *ptrArray, gchar **strList){
    gint j;
    if (strList==NULL)
	return FALSE;
    for(j=0;strList[j]!=NULL;j++){
	gint index=maker_dialog_find_string(strList[j], (gchar **) ptrArray->pdata, -1);
	if (index<0){
	    g_ptr_array_add(ptrArray, g_strdup(strList[j]));
	}
    }
    g_strfreev(strList);
    return TRUE;
}

static gboolean maker_dialog_config_file_get_pages(
	MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf, gpointer userData, MkdgError **error){
    GPtrArray *ptrArray=(GPtrArray *) userData;
    gchar **pages=configFile->configSet->configInterface->config_file_get_pages(configFile,  error);
    return merge_str_lists(ptrArray, pages);
}

static gboolean maker_dialog_config_set_get_pages(MkdgConfigSet *configSet, gint untilIndex, gpointer userData, MkdgError **error){
    return maker_dialog_config_set_foreach_file(
	    configSet, NULL, maker_dialog_config_file_get_pages, untilIndex, userData, error);
}

gchar **maker_dialog_config_get_pages(MkdgConfig *config, MkdgError **error){
    MkdgError *cfgErr=NULL;
    if (config->setArray->len<=0){
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_NO_CONFIG_SET, "config_get_pages()");
	maker_dialog_error_handle(cfgErr, error);
	return FALSE;
    }
    GPtrArray *ptrArray=g_ptr_array_new();
    maker_dialog_foreach_config_set(config, maker_dialog_config_set_get_pages, -1, ptrArray, error);
    return (gchar **) g_ptr_array_free(ptrArray, FALSE);
}

typedef struct {
    GPtrArray *ptrArray;
    const gchar *str;
}  MkdgConfigPtrArrayStrBind;

static gboolean maker_dialog_config_file_get_keys(
	MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf, gpointer userData, MkdgError **error){
    MkdgConfigPtrArrayStrBind *mBind=(MkdgConfigPtrArrayStrBind *) userData;
    gchar **keys=configFile->configSet->configInterface->config_file_get_keys(configFile, mBind->str, error);
    return merge_str_lists(mBind->ptrArray, keys);
}

static gboolean maker_dialog_config_set_get_keys(MkdgConfigSet *configSet, gint untilIndex, gpointer userData, MkdgError **error){
    return maker_dialog_config_set_foreach_file(
	    configSet, NULL, maker_dialog_config_file_get_keys, untilIndex, userData, error);
}

gchar **maker_dialog_config_get_keys(MkdgConfig *config, const gchar *pageName, MkdgError **error){
    MkdgError *cfgErr=NULL;
    if (config->setArray->len<=0){
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_NO_CONFIG_SET, "config_get_keys()");
	maker_dialog_error_handle(cfgErr, error);
	return FALSE;
    }
    MkdgConfigPtrArrayStrBind mBind;
    mBind.ptrArray=g_ptr_array_new();
    mBind.str=pageName;
    maker_dialog_foreach_config_set(config, maker_dialog_config_set_get_keys, -1, &mBind, error);
    return (gchar **) g_ptr_array_free(mBind.ptrArray, FALSE);
}

MkdgValue *maker_dialog_config_get_value(MkdgConfig *config, const gchar *pageName, const gchar *key,
	MkdgType valueType, const gchar *parseOption, MkdgError **error){
    MkdgConfigSet *configSet=NULL;
    if (g_hash_table_size(config->pageSetTable)){
	configSet=(MkdgConfigSet *) g_hash_table_lookup(config->pageSetTable, pageName);
    }else{
	configSet=(MkdgConfigSet *) g_ptr_array_index(config->setArray,0);
    }

    gint i;
    MkdgValue *mValue=NULL, *mValueOld=NULL;
    MkdgError *cfgErr=NULL;
    for(i=0; i< configSet->fileArray->len;i++){
	MkdgConfigFile *configFile=(MkdgConfigFile *) g_ptr_array_index(configSet->fileArray,i);
	mValue=configSet->configInterface->config_file_get_value(configFile, pageName, key, valueType, parseOption, &cfgErr);
	if (mValue!=NULL){
	    if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_NO_OVERRIDE){
		return mValue;
	    }
	    if (mValueOld!=NULL){
		maker_dialog_value_free(mValueOld);
	    }
	    mValueOld=mValue;
	}
    }
    return mValue;
}

static MkdgIdPair mkdgConfigFlagData[]={
    {"READONLY",		MAKER_DIALOG_CONFIG_FLAG_READONLY},
    {"NO_OVERRIDE",		MAKER_DIALOG_CONFIG_FLAG_NO_OVERRIDE},
    {"NO_APPLY",		MAKER_DIALOG_CONFIG_FLAG_NO_APPLY},
    {"STOP_ON_ERROR",		MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR},
    {"HIDE_DEFAULT",		MAKER_DIALOG_CONFIG_FLAG_HIDE_DEFAULT},
    {"HIDE_DUPLICATE",		MAKER_DIALOG_CONFIG_FLAG_HIDE_DUPLICATE},
    {NULL,			0},
};

MkdgConfigFlags maker_dialog_config_flags_parse(const gchar *str){
    return maker_dialog_flag_parse(mkdgConfigFlagData, str, FALSE);
}

