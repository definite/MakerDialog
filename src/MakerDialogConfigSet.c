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
#include <glib.h>
#include <glib/gstdio.h>
#include <string.h>
#include <unistd.h>
#include "MakerDialog.h"


MakerDialogConfigSet *maker_dialog_config_set_new(){
    MakerDialogConfigSet *configSet=maker_dialog_config_set_new_full(NULL,
	    NULL, NULL,
	    NULL, -1,
	    0, NULL, NULL);
    configSet->status=0;
    return configSet;
}

MakerDialogConfigSet *maker_dialog_config_set_new_full(const gchar **pageNames,
	const gchar *filePattern, const gchar **searchDirs,
	const gchar *defaultFilename, gint maxFileCount,
	MakerDialogConfigFlags flags, MakerDialogConfigFileInterface *configInterface,
	gpointer userData){
    MakerDialogConfigSet *configSet=g_new(MakerDialogConfigSet,1);
    configSet->pageNames=pageNames;
    configSet->flags=flags;
    configSet->filePattern=filePattern;
    configSet->searchDirs=searchDirs;
    configSet->defaultFilename=defaultFilename;
    configSet->maxFileCount=maxFileCount;
    configSet->writeIndex=-1;
    configSet->fileArray=g_ptr_array_new();
    configSet->configInterface=configInterface;
    configSet->userData=userData;
    configSet->status= MAKER_DIALOG_CONFIG_SET_STATUS_HAS_LOCAL_SETTING;
    return configSet;
}

static void maker_dialog_config_set_free_config_file(gpointer data, gpointer userData){
    maker_dialog_config_file_free((MakerDialogConfigFile *) data);
}

void maker_dialog_config_set_free(MakerDialogConfigSet *configSet){
    g_ptr_array_foreach(configSet->fileArray,maker_dialog_config_set_free_config_file, NULL);
    g_ptr_array_free(configSet->fileArray,TRUE);
    configSet->configInterface->config_set_finalize(configSet);
    g_free(configSet);
}

gboolean maker_dialog_config_set_prepare_files(MakerDialogConfigSet *configSet, MakerDialogError **error){
    g_assert(configSet->filePattern);
    GPatternSpec* pSpec=g_pattern_spec_new (configSet->filePattern);
    GDir *currDir=NULL;
    MakerDialogError *cfgErr=NULL;
    gchar *currName=NULL;
    gchar *currPath=NULL;
    gint counter=0;
    gint i;
    const gchar *currentDirs[]={".", NULL};
    const gchar **searchDirs=(configSet->searchDirs)? configSet->searchDirs: currentDirs;
    for(i=0;searchDirs[i]!=NULL;i++){
	MAKER_DIALOG_DEBUG_MSG(5, "[I5] config_set_prepare_files(), searchDir=%s", searchDirs[i]);
	currDir=g_dir_open(searchDirs[i], 0, &cfgErr);
	if (maker_dialog_error_handle(cfgErr, error)){
	    continue;
	}
	if (!currDir)
	    continue;
	while((currName=(gchar *) g_dir_read_name(currDir))!=NULL){
	    if (g_pattern_match(pSpec,strlen(currName),currName,NULL)){
		currPath=g_build_filename(searchDirs[i], currName, NULL);
		MAKER_DIALOG_DEBUG_MSG(6, "[I6] config_set_prepare_files(), currPath=%s",  currPath);
		if (g_access(currPath, R_OK)==0){
		    if (!(configSet->flags & MAKER_DIALOG_CONFIG_FLAG_READONLY)){
			if (g_access(currPath, W_OK)==0){
			    if (!(configSet->flags & MAKER_DIALOG_CONFIG_FLAG_NO_OVERRIDE)){
				configSet->writeIndex=counter;
			    }else if (configSet->writeIndex < 0){
				configSet->writeIndex=counter;
			    }
			}
		    }
		    MAKER_DIALOG_DEBUG_MSG(6, "[I6] config_set_prepare_files(), writeIndex=%d", configSet->writeIndex );
		    MakerDialogConfigFile *configFile=maker_dialog_config_file_new(currPath, configSet);
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
		    /* Cannot build parent directories. */
		    continue;
		}
	    }else if (g_access(searchDirs[i], W_OK)!=0){
		continue;
	    }
	    /* Ready to write */
	    MakerDialogConfigFile *configFile=maker_dialog_config_file_new(
		    g_build_filename(searchDirs[i], configSet->defaultFilename, NULL), configSet);
	    g_ptr_array_add(configSet->fileArray, configFile);
	    configSet->writeIndex=counter;
	    counter++;
	}
    }

    if (counter==0){
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_CANT_READ,"config_set_prepare_files()");
	maker_dialog_error_handle(cfgErr, error);
	return FALSE;
    }
    if (configSet->writeIndex<0 && !(configSet->flags & MAKER_DIALOG_CONFIG_FLAG_READONLY)){
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_CONFIG_CANT_WRITE,"config_set_prepare_files()");
	maker_dialog_error_handle(cfgErr, error);
	return FALSE;
    }
    return TRUE;
}

gboolean maker_dialog_config_set_foreach_page(MakerDialogConfigSet *configSet,
	MakerDialogConfigSetEachPageFunc func,  gpointer userData, MakerDialogError **error){
    gboolean clean=TRUE;
    MakerDialogError *cfgErr=NULL;
    const gchar *page=NULL;
    gboolean ret;
    if (configSet->pageNames){
	gsize i;
	for(i=0;(page=configSet->pageNames[i])!=NULL;i++){
	    cfgErr=NULL;
	    ret=func(configSet,page, userData, &cfgErr);
	    if (!ret){
		clean=FALSE;
		maker_dialog_error_handle(cfgErr, error);
		if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		    return FALSE;
		}
	    }
	}
    }else{
	GNode *pageNode=NULL;
	for(pageNode=g_node_first_child(configSet->config->mDialog->pageRoot);pageNode!=NULL; pageNode=g_node_next_sibling(pageNode)){
	    page=(gchar *) pageNode->data;
	    cfgErr=NULL;
	    ret=func(configSet,page, userData, &cfgErr);
	    if (!ret){
		clean=FALSE;
		maker_dialog_error_handle(cfgErr, error);
		if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		    return clean;
		}
	    }
	}
    }
    return clean;
}


