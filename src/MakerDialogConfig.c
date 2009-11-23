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

static MakerDialogConfigFile *maker_dialog_config_file_new(const gchar *path){
    MakerDialogConfigFile *configFile=g_new(MakerDialogConfigFile,1);
    configFile->path=g_strdup(path);
    configFile->filePointer=NULL;
    return configFile;
}

static void maker_dialog_config_file_free(MakerDialogConfigFile *configFile){
    g_free((gchar *)configFile->path);
    if (configFile->filePointer)
        g_free(configFile->filePointer);
    g_free(configFile);
}

MakerDialogConfigSet *maker_dialog_config_set_new(MakerDialogConfigFlag flags){
    return maker_dialog_config_set_new_full(NULL,
	    flags, NULL, NULL,
	    NULL, -1, NULL);
}

MakerDialogConfigSet *maker_dialog_config_set_new_full(const gchar **pageNames,
	MakerDialogConfigFlag flags, const gchar *filePattern, const gchar **searchDirs,
	    const gchar *defaultFilename, gint maxFileCount, gpointer userData){
    MakerDialogConfigSet *configSet=g_new(MakerDialogConfigSet,1);
    configSet->pageNames=pageNames;
    configSet->flags=flags;
    configSet->filePattern=filePattern;
    configSet->searchDirs=searchDirs;
    configSet->defaultFilename=defaultFilename;
    configSet->maxFileCount=maxFileCount;
    configSet->writeIndex=-1;
    configSet->currentIndex=-1;
    configSet->fileArray=g_ptr_array_new();
    configSet->userData=userData;
    return configSet;
}

static void maker_dialog_config_set_free_config_file(gpointer data, gpointer userData){
    maker_dialog_config_file_free((MakerDialogConfigFile *) data);
}

void maker_dialog_config_set_free(MakerDialogConfigSet *configSet){
    g_ptr_array_foreach(configSet->fileArray,maker_dialog_config_set_free_config_file, NULL);
    g_ptr_array_free(configSet->fileArray,TRUE);
    g_free(configSet);
}

MakerDialogConfig *maker_dialog_config_new(
	MakerDialog *mDialog, gboolean fileBased, MakerDialogConfigInterface *configInterface){
    MakerDialogConfig *config=g_new(MakerDialogConfig,1);
    config->fileBased=fileBased;
    config->mDialog=mDialog;
    config->pageSetTable=g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);
    config->setArray=g_ptr_array_new();
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

static GError *maker_dialog_config_set_init(MakerDialogConfigSet *configSet){
    GPatternSpec* pSpec=g_pattern_spec_new (configSet->filePattern);
    GDir *currDir=NULL;
    GError *error=NULL;
    gchar *currName=NULL;
    gchar *currPath=NULL;
    gint counter=0;
    gint i;
    const gchar *currentDirs[]={".", NULL};
    const gchar **searchDirs=(configSet->searchDirs)? configSet->searchDirs: currentDirs;
    for(i=0;searchDirs[i]!=NULL;i++){
	MAKER_DIALOG_DEBUG_MSG(5, "[I5] config_set_init(), searchDir=%s", searchDirs[i]);
	currDir=g_dir_open(searchDirs[i], 0, &error);
	if (error){
	    maker_dialog_config_error_print(error);
	    g_error_free(error);
	    error=NULL;
	    continue;
	}
	if (!currDir)
	    continue;
	while((currName=(gchar *) g_dir_read_name(currDir))!=NULL){
	    if (g_pattern_match(pSpec,strlen(currName),currName,NULL)){
		currPath=g_build_filename(searchDirs[i], currName, NULL);
		MAKER_DIALOG_DEBUG_MSG(6, "[I6] config_set_init(), currPath=%s",  currPath);
		if (g_access(currPath, R_OK)==0){
		    if (g_access(currPath, W_OK)==0){
			if (!(configSet->flags & MAKER_DIALOG_CONFIG_FLAG_NO_OVERRIDE)){
			    configSet->writeIndex=counter;
			}else if (configSet->writeIndex < 0){
			    configSet->writeIndex=counter;
			}
		    }
		    MAKER_DIALOG_DEBUG_MSG(6, "[I6] config_set_init(), writeIndex=%d", configSet->writeIndex );
		    MakerDialogConfigFile *configFile=maker_dialog_config_file_new(currPath);
		    g_ptr_array_add(configSet->fileArray, configFile);
		    counter++;
		    g_free(currPath);
		    if (configSet->maxFileCount>=0 && configSet->maxFileCount<=counter )
			break;
		}
	    }
	}
	g_dir_close(currDir);
	if (configSet->maxFileCount>=0 && configSet->maxFileCount<=counter )
	    break;
    }
    g_pattern_spec_free(pSpec);

    /* Whether to add writable file */
    if (configSet->writeIndex<0
	    && !(configSet->flags & MAKER_DIALOG_CONFIG_FLAG_READONLY)
	    && (configSet->maxFileCount<0 || configSet->maxFileCount>counter)){
	for(i=0;searchDirs[i]!=NULL;i++){
	    if (g_access(searchDirs[i],F_OK)!=0){
		/* Try building dir */
		if (g_mkdir_with_parents(searchDirs[i], 0755)!=0){
		    continue;
		}
	    }else if (g_access(searchDirs[i], W_OK)!=0){
		continue;
	    }
	    /* Ready to write */
	    MakerDialogConfigFile *configFile=maker_dialog_config_file_new(
		    g_build_filename(searchDirs[i], configSet->defaultFilename, NULL));
	    g_ptr_array_add(configSet->fileArray, configFile);
	    configSet->writeIndex=counter;
	    counter++;
	}
    }
    if (counter==0){
	return maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_CANT_READ,"config_set_init()");
    }
    if (configSet->writeIndex<0 && !(configSet->flags & MAKER_DIALOG_CONFIG_FLAG_READONLY)){
	return maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_CANT_WRITE,"config_set_init()");
    }
    return error;
}

void maker_dialog_config_add_config_set(MakerDialogConfig *config, MakerDialogConfigSet *configSet,GError **error){
    g_assert(configSet);
    GError *cfgErr=NULL;
    if (config->fileBased){
	cfgErr=maker_dialog_config_set_init(configSet);
	maker_dialog_config_error_handle(cfgErr, error);
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

gboolean maker_dialog_config_set_foreach_page(MakerDialogConfigSet *configSet,
	MakerDialogConfigSetEachPageFunc func,  gpointer userData, GError **error){
    gboolean clean=TRUE;
    GError *cfgErr=NULL;
    const gchar *page=NULL;
    gboolean ret;
    if (configSet->pageNames){
	gsize i;
	for(i=0;(page=configSet->pageNames[i])!=NULL;i++){
	    cfgErr=NULL;
	    ret=func(configSet,page, userData, &cfgErr);
	    if (!ret){
		clean=FALSE;
		maker_dialog_config_error_handle(cfgErr, error);
		if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		    return FALSE;
		}
	    }
	}
    }else{
	GNode *pageNode=NULL;
	for(pageNode=g_node_first_child(configSet->mDialog->pageRoot);pageNode!=NULL; pageNode=g_node_next_sibling(pageNode)){
	    page=(gchar *) pageNode->data;
	    cfgErr=NULL;
	    ret=func(configSet,page, userData, &cfgErr);
	    if (!ret){
		clean=FALSE;
		maker_dialog_config_error_handle(cfgErr, error);
		if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		    return clean;
		}
	    }
	}
    }
    return clean;
}

/*=== Start config file/set foreach callback and functions ===*/
typedef gboolean (* MakerDialogConfigSetEachFileFunc)(
	MakerDialogConfigFile *configFile, MakerDialogConfigSet *configSet, gpointer userData, GError **error);

static gboolean maker_dialog_config_set_foreach_file(MakerDialogConfigSet *configSet,
	MakerDialogConfigSetEachFileFunc func, gint untilIndex, gpointer userData, GError **error){
    MAKER_DIALOG_DEBUG_MSG(5,"[I5] config_set_foreach_file( , , %d, , )",untilIndex);
    GError *cfgErr=NULL;

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
	    maker_dialog_config_error_handle(cfgErr, error);
	    if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR)
		return clean;
	}
    }
    configSet->currentIndex=-1;
    return clean;
}

typedef gboolean (* MakerDialogConfigForeachSetCallbackFunc)(
	MakerDialogConfigSet *configFile, gint untillIndex, gpointer userData, GError **error);

static gboolean maker_dialog_config_foreach_set(MakerDialogConfig *config,
	MakerDialogConfigForeachSetCallbackFunc func, gint untilIndex, gpointer userData, GError **error){
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] config_foreach_set( , , %d, , )",untilIndex);
    if (config->setArray->len<=0)
	return MAKER_DIALOG_CONFIG_ERROR_NO_CONFIG_SET;
    GError *cfgErr=NULL;
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
	    maker_dialog_config_error_handle(cfgErr, error);
	}
    }
    return clean;
}
/*=== End config file/set foreach callback and functions ===*/

/*=== Start open/close functions ===*/
static gboolean maker_dialog_config_file_open
(MakerDialogConfigFile *configFile, MakerDialogConfigSet *configSet, gpointer userData, GError **error){
    MAKER_DIALOG_DEBUG_MSG(6,"[I6] config_file_open(%s, , , )",configFile->path);
    if (!(configSet->flags & MAKER_DIALOG_CONFIG_FLAG_NOT_FILE_BASE)){
	if (g_access(configFile->path, F_OK)!=0){
	    return (configSet->mDialog->config->configInterface->config_create(configSet, configFile, error))
		? TRUE: FALSE;
	}
    }
    return (configSet->mDialog->config->configInterface->config_open(configSet, configFile, error)) ? TRUE: FALSE;
}

static gboolean maker_dialog_config_set_open(MakerDialogConfigSet *configSet, gint untilIndex, gpointer userData, GError **error){
    /* It is possible that configSet does not have any files. */
    return maker_dialog_config_set_foreach_file(configSet, maker_dialog_config_file_open, untilIndex, userData, error);
}

gboolean maker_dialog_config_open_all(MakerDialogConfig *config, GError **error){
    MAKER_DIALOG_DEBUG_MSG(2,"[I2] config_open_all()");
    return maker_dialog_config_foreach_set(config, maker_dialog_config_set_open, -1, NULL, error);
}

static gboolean maker_dialog_config_file_close(
	MakerDialogConfigFile *configFile, MakerDialogConfigSet *configSet, gpointer userData, GError **error){
    return configSet->mDialog->config->configInterface->config_close(configSet, configFile, error);
}

static gboolean maker_dialog_config_set_close(
	MakerDialogConfigSet *configSet, gint untilIndex, gpointer userData, GError **error){
    return maker_dialog_config_set_foreach_file(configSet, maker_dialog_config_file_close, untilIndex, userData,error);
}

gboolean maker_dialog_config_close_all(MakerDialogConfig *config, GError **error){
    return maker_dialog_config_foreach_set(config, maker_dialog_config_set_close, -1, NULL, error);
}
/*=== End open/close functions ===*/

/*
 * Pre-Load settings in configuration set files.
 */
static gboolean maker_dialog_config_file_preload(
	MakerDialogConfigFile *configFile, MakerDialogConfigSet *configSet, gpointer userData, GError **error){
    return configSet->mDialog->config->configInterface
	->config_preload(configSet, configFile, (gchar *) userData, error);
}

static gboolean maker_dialog_config_set_preload(
	MakerDialogConfigSet *configSet, gint untilIndex, gpointer userData, GError **error){
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
	MakerDialogConfigSet *configSet, gint untilIndex, gpointer userData, GError **error){
    MAKER_DIALOG_DEBUG_MSG(3,"[I3] config_set_load(%s, %d, , )",
	    (configSet->pageNames)? configSet->pageNames[0]  : "NULL",untilIndex);
    gboolean ret=maker_dialog_config_set_preload(configSet, untilIndex, userData, error);
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] config_set_load(%s, %d, , ) preload done.",
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

gboolean maker_dialog_config_load_all(MakerDialogConfig *config, GError **error){
    MAKER_DIALOG_DEBUG_MSG(2,"[I2] config_load_all()");
    return maker_dialog_config_foreach_set(config, maker_dialog_config_set_load, -1, NULL, error);
}

gboolean maker_dialog_config_load_page(MakerDialogConfig *config, const gchar *pageName, GError **error){
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
 * @return MAKER_DIALOG_CONFIG_OK if success; non-zero ::GError *error code otherwise.
 */
static gboolean maker_dialog_config_set_save(MakerDialogConfigSet *configSet, gint untilIndex, gpointer userData, GError **error){
    MAKER_DIALOG_DEBUG_MSG(2,"[I2] config_set_save(%s, %d, )",(configSet->pageNames)? configSet->pageNames[0]  : "NULL",untilIndex);
    gchar *pageName=(gchar *) userData;
    g_assert(configSet->writeIndex<configSet->fileArray->len);
    gboolean clean=TRUE;
    GError *cfgErr=NULL;

    if (configSet->writeIndex<0 ){
	clean=FALSE;
	if (error)
	    *error=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_CANT_WRITE,"config_set_save()");
	return clean;
    }
    gboolean ret=maker_dialog_config_set_preload(configSet, configSet->writeIndex, (gpointer) pageName, &cfgErr);
    if (!ret){
	clean=FALSE;
	maker_dialog_config_error_handle(cfgErr, error);
	if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
	    return FALSE;
	}
    }

    MakerDialogConfigFile *configFile=g_ptr_array_index(configSet->fileArray, configSet->writeIndex);
    configSet->mDialog->config->configInterface->config_save(configSet, configFile, pageName, &cfgErr);
    if (cfgErr)
	clean=FALSE;
    maker_dialog_config_buffer_free(configSet->configBuf);
    maker_dialog_config_error_handle(cfgErr, error);
    return clean;
}

gboolean maker_dialog_config_save_all(MakerDialogConfig *config, GError **error){
    return maker_dialog_config_foreach_set(config, maker_dialog_config_set_save, -1, NULL, error);
}

gboolean maker_dialog_config_save_page(MakerDialogConfig *config, const gchar *pageName, GError **error){
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


MakerDialogConfigBuffer *maker_dialog_config_buffer_new(){
    MakerDialogConfigBuffer *configBuf=g_new(MakerDialogConfigBuffer, 1);
    configBuf->keyValueTable=g_hash_table_new_full(g_str_hash, g_str_equal, g_free, maker_dialog_value_free);
    return configBuf;
}

void maker_dialog_config_buffer_insert(MakerDialogConfigBuffer *configBuf, const gchar *key, MkdgValue *value){
    g_hash_table_insert(configBuf->keyValueTable, g_strdup(key), value);
}

void maker_dialog_config_buffer_free(MakerDialogConfigBuffer *configBuf){
    g_hash_table_destroy(configBuf->keyValueTable);
    g_free(configBuf);
}

GError *maker_dialog_config_error_new(MakerDialogConfigErrorCode code, const gchar *formatStr, ...){
    va_list ap;
    gchar *errMsg=(formatStr) ? g_strdup_vprintf(formatStr, ap): NULL;
    gchar *errMsg_final=g_strdup_printf("%s%s%s",
	    configErrorString[code], (errMsg)? ": " : "", (errMsg)? errMsg : "");
    GError *error=g_error_new_literal(MAKER_DIALOG_CONFIG_ERROR, code, errMsg_final);
    g_free(errMsg);
    g_free(errMsg_final);
    return error;
}

void maker_dialog_config_error_print(GError *error){
    if (MAKER_DIALOG_DEBUG_RUN(0)){
	g_warning("[WW] domain:%s [%d] %s", g_quark_to_string(error->domain), error->code, error->message);
    }
}

gboolean maker_dialog_config_error_handle(GError *errIn, GError **errOut){
    if (errIn){
	if (errOut){
	    if (*errOut){
		maker_dialog_config_error_print(*errOut);
		g_error_free(*errOut);
	    }
	    *errOut=errIn;
	}else{
	    maker_dialog_config_error_print(errIn);
	    g_error_free(errIn);
	    errIn=NULL;
	}
	return TRUE;
    }
    return FALSE;
}

