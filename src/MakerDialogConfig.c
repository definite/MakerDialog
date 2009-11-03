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

static GError *maker_dialog_config_set_new_private(MakerDialogConfigSet *dlgCfgSet){
    GPatternSpec* pSpec=g_pattern_spec_new (dlgCfgSet->filePattern);
    GDir *currDir=NULL;
    GError *error=NULL;
    gchar *currName=NULL;
    gchar *currPath=NULL;
    gint counter=0;
    gint i;
    for(i=0;dlgCfgSet->searchDirs[i]!=NULL;i++){
	MAKER_DIALOG_DEBUG_MSG(5, "[I5] config_set_new_private(), searchDir=%s", dlgCfgSet->searchDirs[i]);
	currDir=g_dir_open(dlgCfgSet->searchDirs[i], 0, &error);
	if (error){
	    maker_dialog_config_error_print(error);
	    g_error_free(error);
	    continue;
	}
	if (!currDir)
	    continue;
	while((currName=(gchar *) g_dir_read_name(currDir))!=NULL){
	    if (g_pattern_match(pSpec,strlen(currName),currName,NULL)){
		currPath=g_build_filename(dlgCfgSet->searchDirs[i], currName, NULL);
		MAKER_DIALOG_DEBUG_MSG(6, "[I6] config_set_new_private(), currPath=%s",  currPath);
		if (g_access(currPath, R_OK)==0){
		    if (g_access(currPath, W_OK)==0){
			if (!(dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_NO_OVERRIDE)){
			    dlgCfgSet->writeIndex=counter;
			}else if (dlgCfgSet->writeIndex < 0){
			    dlgCfgSet->writeIndex=counter;
			}
		    }
		    MAKER_DIALOG_DEBUG_MSG(6, "[I6] config_set_new_private(), writeIndex=%d", dlgCfgSet->writeIndex );
		    MakerDialogConfigFile *configFile=maker_dialog_config_file_new(currPath);
		    g_ptr_array_add(dlgCfgSet->fileArray, configFile);
		    counter++;
		    g_free(currPath);
		    if (dlgCfgSet->maxFileCount>=0 && dlgCfgSet->maxFileCount<=counter )
			break;
		}
	    }
	}
	g_dir_close(currDir);
	if (dlgCfgSet->maxFileCount>=0 && dlgCfgSet->maxFileCount<=counter )
	    break;
    }
    g_pattern_spec_free(pSpec);

    /* Whether to add writable file */
    if (dlgCfgSet->writeIndex<0
	   && !(dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_READONLY)
   	   && (dlgCfgSet->maxFileCount<0 || dlgCfgSet->maxFileCount>counter)){
	for(i=0;dlgCfgSet->searchDirs[i]!=NULL;i++){
	    if (g_access(dlgCfgSet->searchDirs[i],F_OK)!=0){
		/* Try building dir */
		if (g_mkdir_with_parents(dlgCfgSet->searchDirs[i], 0755)!=0){
		    continue;
		}
	    }else if (g_access(dlgCfgSet->searchDirs[i], W_OK)!=0){
		continue;
	    }
	    /* Ready to write */
	    MakerDialogConfigFile *configFile=maker_dialog_config_file_new(
		    g_build_filename(dlgCfgSet->searchDirs[i], dlgCfgSet->defaultFilename, NULL));
	    g_ptr_array_add(dlgCfgSet->fileArray, configFile);
	    dlgCfgSet->writeIndex=counter;
	    counter++;
	}
    }
    if (counter==0){
	return maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_CANT_READ,"config_set_new_private()");
    }
    if (dlgCfgSet->writeIndex<0 && !(dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_READONLY)){
	return maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_CANT_WRITE,"config_set_new_private()");
    }
    return error;
}

MakerDialogConfigSet *maker_dialog_config_set_new(const gchar **pageNames,
	MakerDialogConfigFlag flags, const gchar *filePattern, const gchar **searchDirs,
	    const gchar *defaultFilename, gint maxFileCount, gpointer userData, GError **error){
    MakerDialogConfigSet *dlgCfgSet=g_new(MakerDialogConfigSet,1);
    dlgCfgSet->modified=FALSE;
    dlgCfgSet->pageNames=pageNames;
    dlgCfgSet->flags=flags;
    dlgCfgSet->filePattern=filePattern;
    dlgCfgSet->searchDirs=searchDirs;
    dlgCfgSet->defaultFilename=defaultFilename;
    dlgCfgSet->maxFileCount=maxFileCount;
    dlgCfgSet->writeIndex=-1;
    dlgCfgSet->currentIndex=-1;
    dlgCfgSet->fileArray=g_ptr_array_new();
    dlgCfgSet->userData=userData;
    GError *_error=maker_dialog_config_set_new_private(dlgCfgSet);
    if (_error){
	if (error){
	    *error=_error;
	}else{
	    maker_dialog_config_error_print(_error);
	    g_error_free(_error);
	}
    }
    return dlgCfgSet;
}

static void maker_dialog_config_set_free_config_file(gpointer data, gpointer userData){
    maker_dialog_config_file_free((MakerDialogConfigFile *) data);
}

void maker_dialog_config_set_free(MakerDialogConfigSet *dlgCfgSet){
    g_ptr_array_foreach(dlgCfgSet->fileArray,maker_dialog_config_set_free_config_file, NULL);
    g_ptr_array_free(dlgCfgSet->fileArray,TRUE);
    g_free(dlgCfgSet);
}

MakerDialogConfig *maker_dialog_config_new(MakerDialog *mDialog, MakerDialogConfigHandler *configHandler){
    MakerDialogConfig *dlgCfg=g_new(MakerDialogConfig,1);
    dlgCfg->mDialog=mDialog;
    dlgCfg->pageConfigSetTable=g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);
    dlgCfg->configSetArray=g_ptr_array_new();
    dlgCfg->configHandler=configHandler;
    mDialog->dlgCfg=dlgCfg;
    return dlgCfg;
}

static void maker_dialog_config_free_set(gpointer data, gpointer userData){
    maker_dialog_config_set_free((MakerDialogConfigSet *) data);
}

void maker_dialog_config_free(MakerDialogConfig *dlgCfg){
    maker_dialog_config_close_all(dlgCfg->mDialog, NULL);
    g_hash_table_destroy(dlgCfg->pageConfigSetTable);
    g_ptr_array_foreach(dlgCfg->configSetArray, maker_dialog_config_free_set, NULL);
    g_ptr_array_free(dlgCfg->configSetArray, TRUE);
    g_free(dlgCfg);
}

void maker_dialog_config_add_config_set(MakerDialog *mDialog, MakerDialogConfigSet *dlgCfgSet){
    g_assert(dlgCfgSet);
    g_ptr_array_add(mDialog->dlgCfg->configSetArray,dlgCfgSet);
    if (dlgCfgSet->pageNames){
	gint i;
	for (i=0; dlgCfgSet->pageNames[i]!=NULL; i++){
	    g_hash_table_insert(mDialog->dlgCfg->pageConfigSetTable, (gpointer) dlgCfgSet->pageNames[i], dlgCfgSet);
	}
    }
    dlgCfgSet->mDialog=mDialog;
}

//static GError *error error_tmp=MAKER_DIALOG_CONFIG_OK;
typedef gboolean (* maker_dialog_config_set_foreach_file_callback)(MakerDialogConfigFile *configFile, MakerDialogConfigSet *dlgCfgSet, gpointer userData, GError **error);

static gboolean maker_dialog_config_set_foreach_file(MakerDialogConfigSet *dlgCfgSet, maker_dialog_config_set_foreach_file_callback func, gint untilIndex, gpointer userData, GError **error){
    MAKER_DIALOG_DEBUG_MSG(5,"[I5] config_foreach_file( , , %d, , )",untilIndex);
    GError *lastError=NULL;

    gboolean ret,clean=TRUE;
    gsize i;
    for(i=0;i<dlgCfgSet->fileArray->len;i++){
	if (untilIndex>=0 && i>=untilIndex){
	    break;
	}
	dlgCfgSet->currentIndex=i;
	ret=func(g_ptr_array_index(dlgCfgSet->fileArray,i), dlgCfgSet, userData, error);
	if (!ret){
	    clean=FALSE;
	    if (dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		return ret;
	    }else if (lastError){
		if (lastError){
		    maker_dialog_config_error_print(lastError);
		    g_error_free(lastError);
		}
	    }
	    lastError=(error) ? *error: NULL;
	}
    }
    dlgCfgSet->currentIndex=-1;
    return clean;
}

typedef gboolean (* MakerDialogConfigForeachSetCallbackFunc)(MakerDialogConfigSet *configFile, gint untillIndex, gpointer userData, GError **error);

static gboolean maker_dialog_config_foreach_set(MakerDialogConfig *dlgCfg, MakerDialogConfigForeachSetCallbackFunc func, gint untilIndex, gpointer userData, GError **error){
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] config_foreach_set( , , %d, , )",untilIndex);
    if (dlgCfg->configSetArray->len<=0)
	return MAKER_DIALOG_CONFIG_ERROR_NO_CONFIG_SET;
    GError *lastError=NULL;
    gboolean ret,clean=TRUE;
    gsize i;
    for(i=0;i<dlgCfg->configSetArray->len;i++){
	if (untilIndex>=0 && i>=untilIndex){
	    break;
	}
	MakerDialogConfigSet *dlgCfgSet=g_ptr_array_index(dlgCfg->configSetArray,i);
	ret=func(dlgCfgSet, untilIndex, userData, error);
	if (!ret){
	    clean=FALSE;
	    if (dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		return ret;
	    }else if (lastError){
		if (lastError){
		    maker_dialog_config_error_print(lastError);
		    g_error_free(lastError);
		}
	    }
	    lastError=(error) ? *error: NULL;
	}
    }
    return clean;
}

static gboolean maker_dialog_config_file_open(MakerDialogConfigFile *configFile, MakerDialogConfigSet *dlgCfgSet, gpointer userData, GError **error){
    if (g_access(configFile->path, F_OK)!=0){
	return (dlgCfgSet->mDialog->dlgCfg->configHandler->config_create(dlgCfgSet, configFile, error)) ? TRUE: FALSE;
    }
    return (dlgCfgSet->mDialog->dlgCfg->configHandler->config_open(dlgCfgSet, configFile, error)) ? TRUE: FALSE;
}

static gboolean maker_dialog_config_set_open(MakerDialogConfigSet *dlgCfgSet, gint untilIndex, gpointer userData, GError **error){
    return maker_dialog_config_set_foreach_file(dlgCfgSet, maker_dialog_config_file_open, untilIndex, userData, error);
}

gboolean maker_dialog_config_open_all(MakerDialog *mDialog, GError **error){
    MAKER_DIALOG_DEBUG_MSG(2,"[I2] config_open_all()");
    return maker_dialog_config_foreach_set(mDialog->dlgCfg, maker_dialog_config_set_open, -1, NULL, error);
}

static gboolean maker_dialog_config_file_close(MakerDialogConfigFile *configFile, MakerDialogConfigSet *dlgCfgSet, gpointer userData, GError **error){
    return dlgCfgSet->mDialog->dlgCfg->configHandler->config_close(dlgCfgSet, configFile, error);
}

static gboolean maker_dialog_config_set_close(MakerDialogConfigSet *dlgCfgSet, gint untilIndex, gpointer userData, GError **error){
    return maker_dialog_config_set_foreach_file(dlgCfgSet, maker_dialog_config_file_close, untilIndex, userData,error);
}

gboolean maker_dialog_config_close_all(MakerDialog *mDialog, GError **error){
    return maker_dialog_config_foreach_set(mDialog->dlgCfg, maker_dialog_config_set_close, -1, NULL, error);
}

/*
 * Pre-Load settings in configuration set files.
 */
static gboolean maker_dialog_config_file_preload(MakerDialogConfigFile *configFile, MakerDialogConfigSet *dlgCfgSet, gpointer userData, GError **error){
    return dlgCfgSet->mDialog->dlgCfg->configHandler->config_preload(dlgCfgSet, configFile, (gchar *) userData, error);
}

static gboolean maker_dialog_config_set_preload(MakerDialogConfigSet *dlgCfgSet, gint untilIndex, gpointer userData, GError **error){
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] config_set_preload(%s, %d,  , )",(dlgCfgSet->pageNames)? dlgCfgSet->pageNames[0]  : "NULL",untilIndex);
    dlgCfgSet->dlgCfgBuf=maker_dialog_config_buffer_new();
    return maker_dialog_config_set_foreach_file(dlgCfgSet, maker_dialog_config_file_preload, untilIndex, userData, error);
}

static void maker_dialog_config_file_load_buffer(gpointer hashKey, gpointer value, gpointer userData){
    gchar *key= (gchar *) hashKey;
    GValue *gValue=(GValue *) value;
    MakerDialogConfigSet *dlgCfgSet=(MakerDialogConfigSet *) userData;
    MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(dlgCfgSet->mDialog, key);
    maker_dialog_set_value(dlgCfgSet->mDialog, key, gValue);
    if (!(dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_NO_APPLY)){
	ctx->applyFunc(ctx, &ctx->value);
	ctx->flags &= ~MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED;
    }
}

static gboolean maker_dialog_config_set_load(MakerDialogConfigSet *dlgCfgSet, gint untilIndex, gpointer userData, GError **error){
    MAKER_DIALOG_DEBUG_MSG(3,"[I3] config_set_load(%s, %d, , )",(dlgCfgSet->pageNames)? dlgCfgSet->pageNames[0]  : "NULL",untilIndex);
    gboolean ret=maker_dialog_config_set_preload(dlgCfgSet, untilIndex, userData, error);
    if (!ret && dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
	return ret;
    }
    g_hash_table_foreach(dlgCfgSet->dlgCfgBuf->keyValueTable,maker_dialog_config_file_load_buffer, dlgCfgSet);
    maker_dialog_config_buffer_free(dlgCfgSet->dlgCfgBuf);
    return ret;
}

gboolean maker_dialog_config_load_all(MakerDialog *mDialog, GError **error){
    MAKER_DIALOG_DEBUG_MSG(2,"[I2] config_load_all()");
    return maker_dialog_config_foreach_set(mDialog->dlgCfg, maker_dialog_config_set_load, -1, NULL, error);
}

gboolean maker_dialog_config_load_page(MakerDialog *mDialog, const gchar *pageName, GError **error){
    if (mDialog->dlgCfg->configSetArray->len<=0){
	if (error)
	    *error=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_NO_CONFIG_SET, "config_load_page()");
	return FALSE;
    }
    MakerDialogConfigSet *dlgCfgSet=NULL;
    if (g_hash_table_size(mDialog->dlgCfg->pageConfigSetTable)){
	dlgCfgSet=(MakerDialogConfigSet *)g_hash_table_lookup(mDialog->dlgCfg->pageConfigSetTable, pageName);
    }else{
	dlgCfgSet=g_ptr_array_index(mDialog->dlgCfg->configSetArray,0);
    }
    return maker_dialog_config_set_load(dlgCfgSet, -1, (gpointer) pageName, error);
}

/*
 * Save settings in configuration set files.
 *
 * This functions save setting from MakerDialog instance to configuration set files,
 * set the values to corresponding keys.
 *
 * @param dlgCfgSet A MakerDialog configuration set.
 * @return MAKER_DIALOG_CONFIG_OK if success; non-zero ::GError *error code otherwise.
 */
static gboolean maker_dialog_config_set_save(MakerDialogConfigSet *dlgCfgSet, gint untilIndex, gpointer userData, GError **error){
    MAKER_DIALOG_DEBUG_MSG(2,"[I2] config_set_save(%s, %d, )",(dlgCfgSet->pageNames)? dlgCfgSet->pageNames[0]  : "NULL",untilIndex);
    gchar *pageName=(gchar *) userData;
    g_assert(dlgCfgSet->writeIndex<dlgCfgSet->fileArray->len);
    gboolean clean=TRUE;

    if (dlgCfgSet->writeIndex<0 ){
	clean=FALSE;
	if (error)
	    *error=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_CANT_WRITE,"config_set_save()");
	return clean;
    }
    gboolean ret=maker_dialog_config_set_preload(dlgCfgSet, dlgCfgSet->writeIndex, (gpointer) pageName, error);
    if (!ret){
	clean=FALSE;
	if (dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
	    return FALSE;
	}
    }

    MakerDialogConfigFile *configFile=g_ptr_array_index(dlgCfgSet->fileArray, dlgCfgSet->writeIndex);
    GError *saveError=NULL;
    dlgCfgSet->mDialog->dlgCfg->configHandler->config_save(dlgCfgSet, configFile, pageName, &saveError);
    maker_dialog_config_buffer_free(dlgCfgSet->dlgCfgBuf);
    if (saveError){
	if (error){
	   if (*error){
	       maker_dialog_config_error_print(*error);
	       g_error_free(*error);
	   }
	   *error=saveError;
	}
    }
    return clean;
}

gboolean maker_dialog_config_save_all(MakerDialog *mDialog, GError **error){
    return maker_dialog_config_foreach_set(mDialog->dlgCfg, maker_dialog_config_set_save, -1, NULL, error);
}

gboolean maker_dialog_config_save_page(MakerDialog *mDialog, const gchar *pageName, GError **error){
    if (mDialog->dlgCfg->configSetArray->len<=0)
	if (error){
	    *error=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_NO_CONFIG_SET, "config_save_page()");
	}
	return FALSE;
    gpointer dlgCfgSet=NULL;
    if (g_hash_table_size(mDialog->dlgCfg->pageConfigSetTable)){
	dlgCfgSet=g_hash_table_lookup(mDialog->dlgCfg->pageConfigSetTable, pageName);
    }else{
	dlgCfgSet=g_ptr_array_index(mDialog->dlgCfg->configSetArray,0);
    }
    return maker_dialog_config_set_save(dlgCfgSet, -1, (gpointer) pageName, error);
}

MakerDialogConfigBuffer *maker_dialog_config_buffer_new(){
    MakerDialogConfigBuffer *dlgCfgBuf=g_new(MakerDialogConfigBuffer, 1);
    dlgCfgBuf->keyValueTable=g_hash_table_new_full(g_str_hash, g_str_equal, g_free, maker_dialog_g_value_free);
    return dlgCfgBuf;
}

void maker_dialog_config_buffer_free(MakerDialogConfigBuffer *dlgCfgBuf){
    g_hash_table_destroy(dlgCfgBuf->keyValueTable);
    g_free(dlgCfgBuf);
}

GError *maker_dialog_config_error_new(MakerDialogConfigErrorCode code, const gchar *prefix){
    if (prefix)
	return g_error_new(MAKER_DIALOG_CONFIG_ERROR, code, "%s [%d] %s",prefix, code,configErrorString[code]);
    return g_error_new(MAKER_DIALOG_CONFIG_ERROR, code, "[%d] %s", code,configErrorString[code]);
}

void maker_dialog_config_error_print(GError *error){
    g_warning("[WW] domain:%s [%d] %s", g_quark_to_string(error->domain), error->code, error->message);
}



/*=== Start General file functions ===*/
gboolean maker_dialog_file_isWritable(const gchar *filename){
    gchar parentDirBuf[PATH_MAX];
    gchar *parentDir;
    gboolean result=TRUE;

    if (g_access(filename,W_OK)!=0){
	if (g_access(filename,F_OK)==0){
	    // Read only.
	    return FALSE;
	}
	// Can't write the file , test whether the parent director can write
	g_strlcpy(parentDirBuf,filename,PATH_MAX);
	parentDir=dirname(parentDirBuf);
	if (g_access(parentDir,W_OK)!=0){
	    result=FALSE;
	}
    }
    return result;
}

gchar* maker_dialog_truepath(const gchar *path, gchar *resolved_path){
    gchar workingPath[PATH_MAX];
    gchar fullPath[PATH_MAX];
    gchar *result=NULL;
    g_strlcpy(workingPath,path,PATH_MAX);

    if ( workingPath[0] != '~' ){
	result = realpath(workingPath, resolved_path);
    }else{
	gchar *firstSlash, *suffix, *homeDirStr;
	struct passwd *pw;

	// initialize variables
	firstSlash = suffix = homeDirStr = NULL;

	firstSlash = strchr(workingPath, DIRECTORY_SEPARATOR);
	if (firstSlash == NULL)
	    suffix = "";
	else
	{
	    *firstSlash = 0;    // so userName is null terminated
	    suffix = firstSlash + 1;
	}

	if (workingPath[1] == '\0')
	    pw = getpwuid( getuid() );
	else
	    pw = getpwnam( &workingPath[1] );

	if (pw != NULL)
	    homeDirStr = pw->pw_dir;

	if (homeDirStr != NULL){
	    gint ret=g_sprintf(fullPath, "%s%c%s", homeDirStr, DIRECTORY_SEPARATOR, suffix);
	    if (ret>0){
		result = realpath(fullPath, resolved_path);
	    }

	}
    }
    return result;
}

