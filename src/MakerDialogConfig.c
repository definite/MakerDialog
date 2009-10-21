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
#include "MakerDialog.h"

static MakerDialogConfigError maker_dialog_config_set_new_private(MakerDialogConfigSet dlgCfgSet){
    GPatternSpec* pSpec=g_pattern_spec_new (dlgCfgSet->filePattern);
    GError *error=NULL;
    GDir *currDir=NULL;
    gchar *currName=NULL;
    gint counter=0;
    gint i;
    gint dirWriteIndex=-1;
    for(i=0;dlgCfgSet->searchDirs[i]!=NULL;i++){
	currDir=g_dir_open(dlgCfgSet->searchDirs[i], 0, **error);
	if (error){
	    g_warning(error->message);
	    g_error_free(error);
	    continue;
	}
	if (!currDir)
	    continue;
	while((currName=g_dir_read_name(currDir))!=NULL){

	    if (g_pattern_match(pSpec,currName)){
		if (g_access(currName, R_OK)==0){
		    if (g_access(currName, W_OK)==0 && writeIndex<0){
			dlgCfgSet->writeIndex=counter;
		    }
		    gchar *path= maker_dialog_path_concat(currDir, currName, NULL);
		    g_ptr_array_add(dlgCfgSet->filenameArray, path);
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
	    gchar *path= maker_dialog_path_concat(dlgCfgSet->searchDirs[i], dlgCfgSet->defaultFilename, NULL);
	    g_ptr_array_add(dlgCfgSet->filenameArray, path);
	    dlgCfgSet->writeIndex=counter;
	    counter++;
	}
    }
    if (counter==0){
	return MAKER_DIALOG_CONFIG_ERROR_CANT_READ;
    }
    if (writeIndex<0 && !(dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_READONLY)){
	return MAKER_DIALOG_CONFIG_ERROR_CANT_WRITE;
    }
    return MAKER_DIALOG_CONFIG_OK;
}

MakerDialogConfigSet *maker_dialog_config_set_new(const gchar **pageNames,
	MakerDialogConfigFlag flags, const gchar *filePattern, const gchar **searchDirs,
	const gchar *defaultFilename, gint maxFileCount, MakerDialogConfigError *errorCode){
    MakerDialogConfigSet *dlgCfgSet=g_new(MakerDialogConfigSet,1);
    dlgCfgSet->pageNames=pageNames;
    dlgCfgSet->flags=flags;
    dlgCfgSet->filePattern=filePattern;
    dlgCfgSet->searchDirs=serachDirs;
    dlgCfgSet->defaultFilename=defaultFilename;
    dlgCfgSet->maxFileCount=maxFileCount;
    dlgCfgSet->writeIndex=-1;
    dlgCfgSet->filenameArray=g_ptr_array_new();
    dlgCfgSet->fileArray=g_ptr_array_new();
    MakerDialogConfigError _errorCode=MakerDialogConfigError maker_dialog_config_set_open_private(dlgCfgSet);
    if (errorCode){
	*errorCode=_errorCode;
    }
    return dlgCfgSet;
}

static void maker_dialog_config_set_free_string(gpointer data, gpointer userData){
    g_free(data);
}

void maker_dialog_config_set_free(MakerDialogConfigSet *dlgCfgSet){
    g_ptr_array_foreach(dlgCfgSet->filenameArray,maker_dialog_config_set_free_string, NULL);
    g_ptr_array_free(dlgCfgSet->filenameArray);
    g_ptr_array_foreach(dlgCfgSet->fileArray,maker_dialog_config_set_free_string, NULL);
    g_ptr_array_free(dlgCfgSet->fileArray);
    g_free(dlgCfgSet);
}

/*
 * Load settings in configuration set files.
 *
 * This functions load setting from configuration set files to
 * the given MakerDialog instances.
 *
 * Note that maker_dialog_set
 * set the values to corresponding keys.
 *
 *
 * @param dlgCfgSet A MakerDialog configuration set.
 * @param mDialog  The MakerDialog instances to be set.
 * @return MAKER_DIALOG_CONFIG_OK if success; non-zero ::MakerDialogConfigError code otherwise.
 */
MakerDialogConfigError maker_dialog_config_set_load(MakerDialogConfigSet *dlgCfgSet, MakerDialog *mDialog){
}

/**
 * Save settings in configuration set files.
 *
 * This functions save setting from MakerDialog instance
 * to  configuration set files.
 * set the values to corresponding keys.
 *
 * @param dlgCfgSet A MakerDialog configuration set.
 * @param mDialog  The MakerDialog instances to be set.
 * @return MAKER_DIALOG_CONFIG_OK if success; non-zero ::MakerDialogConfigError code otherwise.
 */
MakerDialogConfigError maker_dialog_config_set_save(MakerDialogConfigSet *dlgCfgSet, MakerDialog *mDialog){
}

/*
 * Load a setting page in configuration set files.
 *
 * This functions load setting from a page of configuration set files to
 * the given MakerDialog instances.
 *
 * Note that maker_dialog_set
 * set the values to corresponding keys.
 *
 *
 * @param dlgCfgSet A MakerDialog configuration set.
 * @param mDialog  The MakerDialog instances to be set.
 * @param pageName Page to be load.
 * @return MAKER_DIALOG_CONFIG_OK if success; non-zero ::MakerDialogConfigError code otherwise.
 * @see maker_dialog_config_load_page(), maker_dialog_config_save_page(), maker_dialog_config_set_save_page().
 */
MakerDialogConfigError maker_dialog_config_set_load_page(MakerDialogConfigSet *dlgCfgSet, MakerDialog *mDialog, const gchar *pageName){
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
MakerDialogConfigError maker_dialog_config_set_save_page(MakerDialogConfigSet *dlgCfgSet, MakerDialog *mDialog, const gchar *pageName){
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

void *maker_dialog_config_free(MakerDialogConfig *dlgCfg){
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
	    g_hash_table_insert(dlgCfgSet->pageNames[i], dlgCfgSet);
	}
    }
}

static MakerDialogConfigError error_tmp=MAKER_DIALOG_CONFIG_OK;

static void maker_dialog_config_file_open(gpointer data, gpointer userData){
    gchar *filename=(gchar *) data;
    MakerDialogConfigSet *dlgCfgSet=(MakerDialogConfigSet *) userData;
    gpointer configFile;

    if (g_access(filename, F_OK)!=0){
	error_tmp=dlgCfgSet->configHandler->config_create(dlgCfgSet, &configFile, filename);
    }else{
	error_tmp=dlgCfgSet->configHandler->config_open(dlgCfgSet, &configFile, filename);
    }
    g_ptr_array_add(dlgCfgSet->fileArray, configFile);
}

static void maker_dialog_config_set_open(gpointer data, gpointer userData){
    MakerDialogConfigSet *dlgCfgSet=(MakerDialogConfigSet *) data;
    g_ptr_array_foreach(dlgCfgSet->filenameArray, maker_dialog_config_file_open , dlgCfgSet);
}

MakerDialogConfigError maker_dialog_config_open_all(MakerDialog *mDialog){
    if (mDialog->dlgCfg->configSetArray->len<=0)
	return MAKER_DIALOG_CONFIG_ERROR_NO_CONFIG_SET;
    g_ptr_array_foreach(mDialog->dlgCfg->configSetArray, maker_dialog_config_set_open, NULL);
    return error_tmp;
}

static void maker_dialog_config_file_close(gpointer fileHandle, gpointer userData){
    MakerDialogConfigSet *dlgCfgSet=(MakerDialogConfigSet *) userData;
    error_tmp=dlgCfgSet->configHandler->config_close(dlgCfgSet, fileHandle);
}

static void maker_dialog_config_set_close(gpointer data, gpointer userData){
    MakerDialogConfigSet *dlgCfgSet=(MakerDialogConfigSet *) data;
    g_ptr_array_foreach(dlgCfgSet->fileArray, maker_dialog_config_file_close , dlgCfgSet);
    g_ptr_array_set_size(dlgCfgSet->fileArray, 0);
}

MakerDialogConfigError maker_dialog_config_close_all(MakerDialog *mDialog){
    if (mDialog->dlgCfg->configSetArray->len<=0)
	return MAKER_DIALOG_CONFIG_ERROR_NO_CONFIG_SET;
    g_ptr_array_foreach(mDialog->dlgCfg->configSetArray, maker_dialog_config_set_close, NULL);
    return error_tmp;
}

static void maker_dialog_config_file_load(gpointer fileHandle, gpointer userData){
    MakerDialogConfigSet *dlgCfgSet=(MakerDialogConfigSet *) userData;
    error_tmp=dlgCfgSet->configHandler->config_load(dlgCfgSet, fileHandle);
}

static void maker_dialog_config_set_load(gpointer data, gpointer userData){
    MakerDialogConfigSet *dlgCfgSet=(MakerDialogConfigSet *) data;
    g_ptr_array_foreach(dlgCfgSet->fileArray, maker_dialog_config_file_load, dlgCfgSet);
}

MakerDialogConfigError maker_dialog_config_load_all(MakerDialog *mDialog){
    if (mDialog->dlgCfg->configSetArray->len<=0)
	return MAKER_DIALOG_CONFIG_ERROR_NO_CONFIG_SET;
    g_ptr_array_foreach(mDialog->dlgCfg->configSetArray, maker_dialog_config_set_load, NULL);
    return error_tmp;
}

MakerDialogConfigError maker_dialog_config_load_page(MakerDialog *mDialog, const gchar *pageName){
    if (mDialog->dlgCfg->configSetArray->len<=0)
	return MAKER_DIALOG_CONFIG_ERROR_NO_CONFIG_SET;
    gpointer dlgCfgSet=NULL;
    if (g_hash_table_size(mDialog->dlgCfg->pageConfigSetTable)){
	dlgCfgSet=g_hash_table_lookup(mDialog->dlgCfg->pageConfigSetTable, (gpointer) pageName);
    }else{
	dlgCfgSet=g_ptr_array_index(mDialog->dlgCfg->configSetArray,0);
    }
    maker_dialog_config_set_load(dlgCfgSet, NULL);
}

static void maker_dialog_config_file_save(gpointer fileHandle, gpointer userData){
    MakerDialogConfigSet *dlgCfgSet=(MakerDialogConfigSet *) userData;
    error_tmp=dlgCfgSet->configHandler->config_save(dlgCfgSet, fileHandle);
}

static void maker_dialog_config_set_save(gpointer data, gpointer userData){
    MakerDialogConfigSet *dlgCfgSet=(MakerDialogConfigSet *) data;
    g_ptr_array_foreach(dlgCfgSet->fileArray, maker_dialog_config_file_save , dlgCfgSet);
}

MakerDialogConfigError maker_dialog_config_save_all(MakerDialog *mDialog){
    if (mDialog->dlgCfg->configSetArray->len<=0)
	return MAKER_DIALOG_CONFIG_ERROR_NO_CONFIG_SET;
    g_ptr_array_foreach(mDialog->dlgCfg->configSetArray, maker_dialog_config_set_save, NULL);
    return error_tmp;
}


/*=== Start General file functions ===*/
gboolean
maker_dialog_file_isWritable(const gchar *filename){
    gchar parentDirBuf[PATH_MAX];
    gchar *parentDir;
    gboolean result=TRUE;

    if (access(filename,W_OK)!=0){
	// Can't write the file , test whether the parent director can write
	g_strlcpy(parentDirBuf,filename,PATH_MAX);
	parentDir=dirname(parentDirBuf);
	if (access(filename,F_OK)==0 || access(parentDir,W_OK)!=0){
	    result=FALSE;
	}
    }
    return result;
}


gchar *maker_dialog_path_concat(const gchar *path1, ...){
    int ret=0;
    va_list ap;
    va_start(ap, path1);
    gchar *str=NULL;
    GString *strBuf=g_string_new(NULL);
    int len=0;
    while((str=var_args(ap, (gchar*)))!=NULL){
	len=strlen(str);
	if (len>0){
	    if (dest[len-1]=='\\' || dest[len-1]=='/'){
		/* hasDirSeparator already */
	    }else{
		g_string_append_c(strBuf,DIRECTORY_SEPARATOR);
	    }
	    g_string_append(strBuf,str);
	}
    }
    return g_string_append(strBuf, FALSE);
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

