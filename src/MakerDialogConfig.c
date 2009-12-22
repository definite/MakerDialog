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
#include "MakerDialog.h"

static const gchar *configErrorString[]={
    "permission deny",
    "cannot read",
    "cannot write",
    "not found",
    "no config set",
    "invalid format",
    "invalid page",
    "invalid key",
    "invalid value",
    "other error",
    NULL
};

GQuark maker_dialog_config_error_quark(){
    return g_quark_from_static_string("MakerDialogConfig");
}

MakerDialogConfig *maker_dialog_config_new(
	MakerDialog *mDialog, gboolean fileBased, MakerDialogConfigInterface *configInterface){
    MakerDialogConfig *config=g_new(MakerDialogConfig,1);
    config->fileBased=fileBased;
    config->mDialog=mDialog;
    config->pageSetTable=g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);
    config->setArray=g_ptr_array_sized_new(5);
    config->configInterface=configInterface;
    mDialog->config=config;
    return config;
}

static void maker_dialog_config_free_set(gpointer data, gpointer userData){
    maker_dialog_config_set_free((MakerDialogConfigSet *) data);
}

void maker_dialog_config_free(MakerDialogConfig *config){
    maker_dialog_config_close_all(config, NULL);
    g_hash_table_destroy(config->pageSetTable);
    g_ptr_array_foreach(config->setArray, maker_dialog_config_free_set, NULL);
    g_ptr_array_free(config->setArray, TRUE);
    g_free(config);
}

void maker_dialog_config_add_config_set(MakerDialogConfig *config, MakerDialogConfigSet *configSet,MakerDialogError **error){
    g_assert(configSet);
    MakerDialogError *cfgErr=NULL;
    if (config->fileBased){
	maker_dialog_config_set_prepare_files(configSet, &cfgErr);
	maker_dialog_error_handle(cfgErr, error);
    }else{
	configSet->flags |= MAKER_DIALOG_CONFIG_FLAG_NOT_FILE_BASE;
	if (configSet->filePattern){
	    MakerDialogConfigFile *configFile=maker_dialog_config_file_new(configSet->filePattern);
	    g_ptr_array_add(configSet->fileArray, configFile);
	    configSet->writeIndex=0;
	}
    }
    g_ptr_array_add(config->setArray,configSet);
    if (configSet->pageNames){
	gint i;
	for (i=0; configSet->pageNames[i]!=NULL; i++){
	    g_hash_table_insert(config->pageSetTable, (gpointer) configSet->pageNames[i], configSet);
	}
    }
    configSet->mDialog=config->mDialog;
}

/*=== Start config file/set foreach callback and functions ===*/
typedef gboolean (* MakerDialogConfigSetEachFileFunc)(
	MakerDialogConfigFile *configFile, MakerDialogConfigSet *configSet, gpointer userData, MakerDialogError **error);

static gboolean maker_dialog_config_set_foreach_file(MakerDialogConfigSet *configSet,
	MakerDialogConfigSetEachFileFunc func, gint untilIndex, gpointer userData, MakerDialogError **error){
    MakerDialogError *cfgErr=NULL;

    gboolean ret,clean=TRUE;
    gsize i;
    for(i=0;i<configSet->fileArray->len;i++){
	if (untilIndex>=0 && i>=untilIndex){
	    break;
	}
	configSet->currentIndex=i;
	cfgErr=NULL;
	ret=func(g_ptr_array_index(configSet->fileArray,i), configSet, userData, &cfgErr);
	if (!ret){
	    clean=FALSE;
	    maker_dialog_error_handle(cfgErr, error);
	    if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR)
		return clean;
	}
    }
    configSet->currentIndex=-1;
    return clean;
}

typedef gboolean (* MakerDialogConfigForeachSetCallbackFunc)(
	MakerDialogConfigSet *configFile, gint untillIndex, gpointer userData, MakerDialogError **error);

static gboolean maker_dialog_config_foreach_set(MakerDialogConfig *config,
	MakerDialogConfigForeachSetCallbackFunc func, gint untilIndex, gpointer userData, MakerDialogError **error){
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] config_foreach_set( , , %d, , )",untilIndex);
    if (config->setArray->len<=0)
	return MAKER_DIALOG_CONFIG_ERROR_NO_CONFIG_SET;
    MakerDialogError *cfgErr=NULL;
    gboolean ret,clean=TRUE;
    gsize i;
    for(i=0;i<config->setArray->len;i++){
	if (untilIndex>=0 && i>=untilIndex){
	    break;
	}
	MakerDialogConfigSet *configSet=g_ptr_array_index(config->setArray,i);
	cfgErr=NULL;
	ret=func(configSet, untilIndex, userData, &cfgErr);
	if (!ret){
	    clean=FALSE;
	    maker_dialog_error_handle(cfgErr, error);
	}
    }
    return clean;
}
/*=== End config file/set foreach callback and functions ===*/

/*=== Start open/close functions ===*/
static gboolean maker_dialog_config_file_open
(MakerDialogConfigFile *configFile, MakerDialogConfigSet *configSet, gpointer userData, MakerDialogError **error){
    MAKER_DIALOG_DEBUG_MSG(5,"[I5] config_file_open(%s, , , )",configFile->path);
    if (!(configSet->flags & MAKER_DIALOG_CONFIG_FLAG_NOT_FILE_BASE)){
	if (g_access(configFile->path, F_OK)!=0){
	    return (configSet->mDialog->config->configInterface->config_create(configSet, configFile, error))
		? TRUE: FALSE;
	}
    }
    return (configSet->mDialog->config->configInterface->config_open(configSet, configFile, error)) ? TRUE: FALSE;
}

static gboolean maker_dialog_config_set_open(MakerDialogConfigSet *configSet, gint untilIndex, gpointer userData, MakerDialogError **error){
    /* It is possible that configSet does not have any files. */
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] config_set_open(%s, %d,  , )",
	    (configSet->pageNames)? configSet->pageNames[0]  : "NULL",untilIndex);
    return maker_dialog_config_set_foreach_file(configSet, maker_dialog_config_file_open, untilIndex, userData, error);
}

gboolean maker_dialog_config_open_all(MakerDialogConfig *config, MakerDialogError **error){
    return maker_dialog_config_foreach_set(config, maker_dialog_config_set_open, -1, NULL, error);
}

static gboolean maker_dialog_config_file_close(
	MakerDialogConfigFile *configFile, MakerDialogConfigSet *configSet, gpointer userData, MakerDialogError **error){
    return configSet->mDialog->config->configInterface->config_close(configSet, configFile, error);
}

static gboolean maker_dialog_config_set_close(
	MakerDialogConfigSet *configSet, gint untilIndex, gpointer userData, MakerDialogError **error){
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] config_set_close(%s, %d,  , )",
	    (configSet->pageNames)? configSet->pageNames[0]  : "NULL",untilIndex);
    return maker_dialog_config_set_foreach_file(configSet, maker_dialog_config_file_close, untilIndex, userData,error);
}

gboolean maker_dialog_config_close_all(MakerDialogConfig *config, MakerDialogError **error){
    return maker_dialog_config_foreach_set(config, maker_dialog_config_set_close, -1, NULL, error);
}
/*=== End open/close functions ===*/

/*
 * Pre-Load settings in configuration set files.
 */
static gboolean maker_dialog_config_file_preload(
	MakerDialogConfigFile *configFile, MakerDialogConfigSet *configSet, gpointer userData, MakerDialogError **error){
    return configSet->mDialog->config->configInterface
	->config_preload(configSet, configFile, (gchar *) userData, error);
}

static gboolean maker_dialog_config_set_preload(
	MakerDialogConfigSet *configSet, gint untilIndex, gpointer userData, MakerDialogError **error){
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] config_set_preload(%s, %d,  , )",
	    (configSet->pageNames)? configSet->pageNames[0]  : "NULL",untilIndex);
    configSet->configBuf=maker_dialog_config_buffer_new();
    return maker_dialog_config_set_foreach_file(
	    configSet, maker_dialog_config_file_preload, untilIndex, userData, error);
}

static void maker_dialog_config_file_load_buffer(gpointer hashKey, gpointer value, gpointer userData){
    gchar *key= (gchar *) hashKey;
    MAKER_DIALOG_DEBUG_MSG(5,"[I5] config_file_load_buffer (%s, , )", key);
    MkdgValue *mValue=(MkdgValue *) value;
    MakerDialogConfigSet *configSet=(MakerDialogConfigSet *) userData;
    MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(configSet->mDialog, key);
    maker_dialog_set_value(configSet->mDialog, key, mValue);
    if (!(configSet->flags & MAKER_DIALOG_CONFIG_FLAG_NO_APPLY)){
	maker_dialog_apply_value(configSet->mDialog,ctx->spec->key);
    }
}

static gboolean maker_dialog_config_set_load(
	MakerDialogConfigSet *configSet, gint untilIndex, gpointer userData, MakerDialogError **error){
    gboolean ret=maker_dialog_config_set_preload(configSet, untilIndex, userData, error);
    MAKER_DIALOG_DEBUG_MSG(5,"[I5] config_set_load(%s, %d, , ) preload done.",
	    (configSet->pageNames)? configSet->pageNames[0]  : "NULL",untilIndex);
    if (!ret && configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
	return ret;
    }
    g_hash_table_foreach(configSet->configBuf->keyValueTable,maker_dialog_config_file_load_buffer, configSet);
    maker_dialog_config_buffer_free(configSet->configBuf);
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] config_set_load(%s, %d, , ) load done.",
	    (configSet->pageNames)? configSet->pageNames[0]  : "NULL",untilIndex);
    return ret;
}

gboolean maker_dialog_config_load_all(MakerDialogConfig *config, MakerDialogError **error){
    return maker_dialog_config_foreach_set(config, maker_dialog_config_set_load, -1, NULL, error);
}

gboolean maker_dialog_config_load_page(MakerDialogConfig *config, const gchar *pageName, MakerDialogError **error){
    if (config->setArray->len<=0){
	if (error)
	    *error=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_NO_CONFIG_SET, "config_load_page()");
	return FALSE;
    }
    MakerDialogConfigSet *configSet=NULL;
    if (g_hash_table_size(config->pageSetTable)){
	configSet=(MakerDialogConfigSet *)g_hash_table_lookup(config->pageSetTable, pageName);
    }else{
	configSet=g_ptr_array_index(config->setArray,0);
    }
    return maker_dialog_config_set_load(configSet, -1, (gpointer) pageName, error);
}

/*
 * Save settings in configuration set files.
 *
 * This functions save setting from MakerDialog instance to configuration set files,
 * set the values to corresponding keys.
 *
 * @param configSet A MakerDialog configuration set.
 * @return MAKER_DIALOG_CONFIG_OK if success; non-zero ::MakerDialogError *error code otherwise.
 */
static gboolean maker_dialog_config_set_save(MakerDialogConfigSet *configSet, gint untilIndex, gpointer userData, MakerDialogError **error){
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] config_set_save(%s, %d, )",(configSet->pageNames)? configSet->pageNames[0]  : "NULL",untilIndex);
    gchar *pageName=(gchar *) userData;
    g_assert(configSet->writeIndex<configSet->fileArray->len);
    gboolean clean=TRUE;
    MakerDialogError *cfgErr=NULL;

    if (configSet->writeIndex<0 ){
	clean=FALSE;
	if (error)
	    *error=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_CANT_WRITE,"config_set_save()");
	return clean;
    }
    gboolean ret=maker_dialog_config_set_preload(configSet, configSet->writeIndex, (gpointer) pageName, &cfgErr);
    if (!ret){
	clean=FALSE;
	maker_dialog_error_handle(cfgErr, error);
	if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
	    return FALSE;
	}
    }

    MakerDialogConfigFile *configFile=g_ptr_array_index(configSet->fileArray, configSet->writeIndex);
    configSet->mDialog->config->configInterface->config_save(configSet, configFile, pageName, &cfgErr);
    if (cfgErr)
	clean=FALSE;
    maker_dialog_config_buffer_free(configSet->configBuf);
    maker_dialog_error_handle(cfgErr, error);
    return clean;
}

gboolean maker_dialog_config_save_all(MakerDialogConfig *config, MakerDialogError **error){
    return maker_dialog_config_foreach_set(config, maker_dialog_config_set_save, -1, NULL, error);
}

gboolean maker_dialog_config_save_page(MakerDialogConfig *config, const gchar *pageName, MakerDialogError **error){
    if (config->setArray->len<=0)
	if (error){
	    *error=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_NO_CONFIG_SET, "config_save_page()");
	}
	return FALSE;
    gpointer configSet=NULL;
    if (g_hash_table_size(config->pageSetTable)){
	configSet=g_hash_table_lookup(config->pageSetTable, pageName);
    }else{
	configSet=g_ptr_array_index(config->setArray,0);
    }
    return maker_dialog_config_set_save(configSet, -1, (gpointer) pageName, error);
}

MakerDialogError *maker_dialog_config_error_new(MakerDialogConfigErrorCode code, const gchar *formatStr, ...){
    va_list ap;
    va_start(ap, formatStr);
    gchar *errMsg=(formatStr) ? g_strdup_vprintf(formatStr, ap): NULL;
    gchar *errMsg_final=g_strdup_printf("%s%s%s",
	    configErrorString[code], (errMsg)? ": " : "", (errMsg)? errMsg : "");
    MakerDialogError *error=g_error_new_literal(MAKER_DIALOG_CONFIG_ERROR, code, errMsg_final);
    g_free(errMsg);
    g_free(errMsg_final);
    return error;
}

