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

static MakerDialogConfigFile *maker_dialog_config_file_new(const gchar *path){
    MakerDialogConfigFile *configFile=g_new(MakerDialogConfigFile,1);
    configFile->path=path;
    configFile->filePointer=NULL;
    return configFile;
}

static void maker_dialog_config_file_free(MakerDialogConfigFile *configFile){
    g_free((gchar *)configFile->path);
    if (configFile->filePointer)
        g_free(configFile->filePointer);
    g_free(configFile);
}

static MakerDialogConfigError maker_dialog_config_set_new_private(MakerDialogConfigSet *dlgCfgSet){
    GPatternSpec* pSpec=g_pattern_spec_new (dlgCfgSet->filePattern);
    GError *error=NULL;
    GDir *currDir=NULL;
    gchar *currName=NULL;
    gint counter=0;
    gint i;
    for(i=0;dlgCfgSet->searchDirs[i]!=NULL;i++){
	currDir=g_dir_open(dlgCfgSet->searchDirs[i], 0, &error);
	if (error){
	    g_warning(error->message);
	    g_error_free(error);
	    continue;
	}
	if (!currDir)
	    continue;
	while((currName=(gchar *) g_dir_read_name(currDir))!=NULL){
	    if (g_pattern_match(pSpec,strlen(currName),currName,NULL)){
		if (g_access(currName, R_OK)==0){
		    if (g_access(currName, W_OK)==0){
			if (!(dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_NO_OVERRIDE)){
			    dlgCfgSet->writeIndex=counter;
			}else if (dlgCfgSet->writeIndex < 0){
			    dlgCfgSet->writeIndex=counter;
			}
		    }
		    MakerDialogConfigFile *configFile=maker_dialog_config_file_new(
			    g_build_filename(dlgCfgSet->searchDirs[i], currName, NULL));
		    g_ptr_array_add(dlgCfgSet->fileArray, configFile);
		    counter++;
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
		    g_build_filename(dlgCfgSet->searchDirs[i], currName, NULL));
	    g_ptr_array_add(dlgCfgSet->fileArray, configFile);
	    dlgCfgSet->writeIndex=counter;
	    counter++;
	}
    }
    if (counter==0){
	return MAKER_DIALOG_CONFIG_ERROR_CANT_READ;
    }
    if (dlgCfgSet->writeIndex<0 && !(dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_READONLY)){
	return MAKER_DIALOG_CONFIG_ERROR_CANT_WRITE;
    }
    return MAKER_DIALOG_CONFIG_OK;
}

MakerDialogConfigSet *maker_dialog_config_set_new(const gchar **pageNames,
	MakerDialogConfigFlag flags, const gchar *filePattern, const gchar **searchDirs,
	const gchar *defaultFilename, gint maxFileCount, MakerDialogConfigError *errorCode){
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
    MakerDialogConfigError _errorCode=maker_dialog_config_set_new_private(dlgCfgSet);
    if (errorCode){
	*errorCode=_errorCode;
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
    maker_dialog_config_close_all(dlgCfg->mDialog);
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

//static MakerDialogConfigError error_tmp=MAKER_DIALOG_CONFIG_OK;
typedef MakerDialogConfigError (* maker_dialog_config_set_foreach_file_callback)(MakerDialogConfigFile *configFile, MakerDialogConfigSet *dlgCfgSet, gpointer userData);

static MakerDialogConfigError maker_dialog_config_set_foreach_file(MakerDialogConfigSet *dlgCfgSet, maker_dialog_config_set_foreach_file_callback func, gint untilIndex, gpointer userData){
    MakerDialogConfigError cfgErr=MAKER_DIALOG_CONFIG_OK;
    gsize i;
    for(i=0;i<dlgCfgSet->fileArray->len;i++){
	if (untilIndex>=0 && i>untilIndex){
	    break;
	}
	dlgCfgSet->currentIndex=i;
	MakerDialogConfigError cfgErr_tmp=func(g_ptr_array_index(dlgCfgSet->fileArray,i), dlgCfgSet, userData);
	if (cfgErr_tmp){
	    if (dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		return cfgErr_tmp;
	    }else{
		cfgErr=(cfgErr==MAKER_DIALOG_CONFIG_OK)? cfgErr_tmp: cfgErr;
	    }
	}
    }
    dlgCfgSet->currentIndex=-1;
    return cfgErr;
}

typedef MakerDialogConfigError (* MakerDialogConfigForeachSetCallbackFunc)(MakerDialogConfigSet *configFile, gint untillIndex, gpointer userData);

static MakerDialogConfigError maker_dialog_config_foreach_set(MakerDialogConfig *dlgCfg, MakerDialogConfigForeachSetCallbackFunc func, gint untilIndex, gpointer userData){
    if (dlgCfg->configSetArray->len<=0)
	return MAKER_DIALOG_CONFIG_ERROR_NO_CONFIG_SET;
    MakerDialogConfigError cfgErr=MAKER_DIALOG_CONFIG_OK;
    gsize i;
    for(i=0;i<dlgCfg->configSetArray->len;i++){
	if (untilIndex>=0 && i>untilIndex){
	    break;
	}
	MakerDialogConfigSet *dlgCfgSet=g_ptr_array_index(dlgCfg->configSetArray,i);
	MakerDialogConfigError cfgErr_tmp=func(dlgCfgSet, untilIndex, userData);
	if (cfgErr_tmp){
	    if (dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		return cfgErr_tmp;
	    }else{
		cfgErr=(cfgErr==MAKER_DIALOG_CONFIG_OK)? cfgErr_tmp: cfgErr;
	    }
	}
    }
    return cfgErr;
}

static MakerDialogConfigError maker_dialog_config_file_open(MakerDialogConfigFile *configFile, MakerDialogConfigSet *dlgCfgSet, gpointer userData){
    MakerDialogConfigError  cfgErr=MAKER_DIALOG_CONFIG_OK;
    if (g_access(configFile->path, F_OK)!=0){
	cfgErr=dlgCfgSet->mDialog->dlgCfg->configHandler->config_create(dlgCfgSet, configFile);
    }else{
	cfgErr=dlgCfgSet->mDialog->dlgCfg->configHandler->config_open(dlgCfgSet, configFile);
    }
    return cfgErr;
}

static MakerDialogConfigError maker_dialog_config_set_open(MakerDialogConfigSet *dlgCfgSet, gint untilIndex, gpointer userData){
    return maker_dialog_config_set_foreach_file(dlgCfgSet, maker_dialog_config_file_open, untilIndex, userData);
}

MakerDialogConfigError maker_dialog_config_open_all(MakerDialog *mDialog){
    return maker_dialog_config_foreach_set(mDialog->dlgCfg, maker_dialog_config_set_open, -1, NULL);
}

static MakerDialogConfigError maker_dialog_config_file_close(MakerDialogConfigFile *configFile, MakerDialogConfigSet *dlgCfgSet, gpointer userData){
    return dlgCfgSet->mDialog->dlgCfg->configHandler->config_close(dlgCfgSet, configFile);
}

static MakerDialogConfigError maker_dialog_config_set_close(MakerDialogConfigSet *dlgCfgSet, gint untilIndex, gpointer userData){
    return maker_dialog_config_set_foreach_file(dlgCfgSet, maker_dialog_config_file_close, untilIndex, userData);
}

MakerDialogConfigError maker_dialog_config_close_all(MakerDialog *mDialog){
    return maker_dialog_config_foreach_set(mDialog->dlgCfg, maker_dialog_config_set_close, -1, NULL);
}

/*
 * Pre-Load settings in configuration set files.
 */
static MakerDialogConfigError maker_dialog_config_file_preload(MakerDialogConfigFile *configFile, MakerDialogConfigSet *dlgCfgSet, gpointer userData){
    return dlgCfgSet->mDialog->dlgCfg->configHandler->config_preload(dlgCfgSet, configFile, (gchar *) userData);
}

static MakerDialogConfigError maker_dialog_config_set_preload(MakerDialogConfigSet *dlgCfgSet, gint untilIndex, gpointer userData){
    dlgCfgSet->dlgCfgBuf=maker_dialog_config_buffer_new();
    return maker_dialog_config_set_foreach_file(dlgCfgSet, maker_dialog_config_file_preload, untilIndex, userData);
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

static MakerDialogConfigError maker_dialog_config_set_load(MakerDialogConfigSet *dlgCfgSet, gint untilIndex, gpointer userData){
    MAKER_DIALOG_DEBUG_MSG(2,"[I2] maker_dialog_config_set_load(%s,%d,-)",(dlgCfgSet->pageNames)? dlgCfgSet->pageNames[0]  : "NULL",untilIndex);
    MakerDialogConfigError cfgErr=maker_dialog_config_set_preload(dlgCfgSet, untilIndex, userData);
    if (cfgErr && dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
	return cfgErr;
    }
    g_hash_table_foreach(dlgCfgSet->dlgCfgBuf->keyValueTable,maker_dialog_config_file_load_buffer, dlgCfgSet);
    maker_dialog_config_buffer_free(dlgCfgSet->dlgCfgBuf);
    return cfgErr;
}

MakerDialogConfigError maker_dialog_config_load_all(MakerDialog *mDialog){
    return maker_dialog_config_foreach_set(mDialog->dlgCfg, maker_dialog_config_set_load, -1, NULL);
}

MakerDialogConfigError maker_dialog_config_load_page(MakerDialog *mDialog, const gchar *pageName){
    if (mDialog->dlgCfg->configSetArray->len<=0)
	return MAKER_DIALOG_CONFIG_ERROR_NO_CONFIG_SET;
    MakerDialogConfigSet *dlgCfgSet=NULL;
    if (g_hash_table_size(mDialog->dlgCfg->pageConfigSetTable)){
	dlgCfgSet=(MakerDialogConfigSet *)g_hash_table_lookup(mDialog->dlgCfg->pageConfigSetTable, pageName);
    }else{
	dlgCfgSet=g_ptr_array_index(mDialog->dlgCfg->configSetArray,0);
    }
    return maker_dialog_config_set_load(dlgCfgSet, -1, (gpointer) pageName);
}

/*
 * Save settings in configuration set files.
 *
 * This functions save setting from MakerDialog instance to configuration set files,
 * set the values to corresponding keys.
 *
 * @param dlgCfgSet A MakerDialog configuration set.
 * @return MAKER_DIALOG_CONFIG_OK if success; non-zero ::MakerDialogConfigError code otherwise.
 */
static MakerDialogConfigError  maker_dialog_config_set_save(MakerDialogConfigSet *dlgCfgSet, gint untilIndex, gpointer userData){
    MAKER_DIALOG_DEBUG_MSG(2,"[I2] maker_dialog_config_set_save(%s,%d,-)",(dlgCfgSet->pageNames)? dlgCfgSet->pageNames[0]  : "NULL",untilIndex);
    gchar *pageName=(gchar *) userData;
    g_assert(dlgCfgSet->writeIndex<dlgCfgSet->fileArray->len);

    if (dlgCfgSet->writeIndex<0 && dlgCfgSet->writeIndex>=dlgCfgSet->fileArray->len){
	return MAKER_DIALOG_CONFIG_ERROR_CANT_WRITE;
    }
    MakerDialogConfigError cfgErr=maker_dialog_config_set_preload(dlgCfgSet, dlgCfgSet->writeIndex, (gpointer) pageName);
    if (dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
	return cfgErr;
    }

    MakerDialogConfigFile *configFile=g_ptr_array_index(dlgCfgSet->fileArray, dlgCfgSet->writeIndex);
    MakerDialogConfigError cfgErr_tmp=dlgCfgSet->mDialog->dlgCfg->configHandler->config_save(dlgCfgSet, configFile, pageName);
    cfgErr=(cfgErr==MAKER_DIALOG_CONFIG_OK)? cfgErr_tmp: cfgErr;
    maker_dialog_config_buffer_free(dlgCfgSet->dlgCfgBuf);
    return cfgErr;
}

MakerDialogConfigError maker_dialog_config_save_all(MakerDialog *mDialog){
    return maker_dialog_config_foreach_set(mDialog->dlgCfg, maker_dialog_config_set_preload, -1, NULL);
}

/*
 * Save a setting page in configuration set files.
 *
 * This functions save setting from a page of MakerDialog instance
 * to  configuration set files.
 * set the values to corresponding keys.
 *
 * @param dlgCfgSet A MakerDialog configuration set.
 * @param mDialog  The MakerDialog instances to be set.
 * @param pageName Page to be save.
 * @return MAKER_DIALOG_CONFIG_OK if success; non-zero ::MakerDialogConfigError code otherwise.
 * @see maker_dialog_config_load_page(), maker_dialog_config_set_load_page(), maker_dialog_config_save_page().
 */
MakerDialogConfigError maker_dialog_config_save_page(MakerDialog *mDialog, const gchar *pageName){
    if (mDialog->dlgCfg->configSetArray->len<=0)
	return MAKER_DIALOG_CONFIG_ERROR_NO_CONFIG_SET;
    gpointer dlgCfgSet=NULL;
    if (g_hash_table_size(mDialog->dlgCfg->pageConfigSetTable)){
	dlgCfgSet=g_hash_table_lookup(mDialog->dlgCfg->pageConfigSetTable, pageName);
    }else{
	dlgCfgSet=g_ptr_array_index(mDialog->dlgCfg->configSetArray,0);
    }
    return maker_dialog_config_set_save(dlgCfgSet, -1, (gpointer) pageName);
}

static void g_value_free(gpointer value){
    GValue *gValue=(GValue  *) value;
    g_value_unset(gValue);
    g_free(gValue);
}

MakerDialogConfigBuffer *maker_dialog_config_buffer_new(){
    MakerDialogConfigBuffer *dlgCfgBuf=g_new(MakerDialogConfigBuffer, 1);
    dlgCfgBuf->keyValueTable=g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_value_free);
    return dlgCfgBuf;
}

void maker_dialog_config_buffer_free(MakerDialogConfigBuffer *dlgCfgBuf){
    g_hash_table_destroy(dlgCfgBuf->keyValueTable);
    g_free(dlgCfgBuf);
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

    //     printf("*** path=%s \n",path);

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

