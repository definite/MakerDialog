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


MkdgConfigSet *mkdg_config_set_new(){
    MkdgConfigSet *configSet=mkdg_config_set_new_full(NULL,
	    NULL, NULL,
	    NULL, -1,
	    0, NULL, NULL);
    configSet->status=0;
    return configSet;
}

MkdgConfigSet *mkdg_config_set_new_full(const gchar **pageNames,
	const gchar *filePattern, const gchar **searchDirs,
	const gchar *defaultFilename, gint maxFileCount,
	MkdgConfigFlags flags, MkdgConfigFileInterface *configInterface,
	gpointer userData){
    MkdgConfigSet *configSet=g_new(MkdgConfigSet,1);
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
    configSet->status= MKDG_CONFIG_SET_STATUS_HAS_LOCAL_SETTING;
    return configSet;
}

static void mkdg_config_set_free_config_file(gpointer data, gpointer userData){
    mkdg_config_file_free((MkdgConfigFile *) data);
}

void mkdg_config_set_free(MkdgConfigSet *configSet){
    g_ptr_array_foreach(configSet->fileArray,mkdg_config_set_free_config_file, NULL);
    g_ptr_array_free(configSet->fileArray,TRUE);
    configSet->configInterface->config_set_finalize(configSet);
    g_free(configSet);
}

gboolean mkdg_config_set_prepare_files(MkdgConfigSet *configSet, MkdgError **error){
    g_assert(configSet->filePattern);
    GPatternSpec* pSpec=g_pattern_spec_new (configSet->filePattern);
    GDir *currDir=NULL;
    MkdgError *cfgErr=NULL;
    gchar *currName=NULL;
    gchar *currPath=NULL;
    gint counter=0;
    gint i;
    const gchar *currentDirs[]={".", NULL};
    const gchar **searchDirs=(configSet->searchDirs)? configSet->searchDirs: currentDirs;
    for(i=0;searchDirs[i]!=NULL;i++){
	MKDG_DEBUG_MSG(5, "[I5] config_set_prepare_files(), searchDir=%s", searchDirs[i]);
	currDir=g_dir_open(searchDirs[i], 0, &cfgErr);
	if (mkdg_error_handle(cfgErr, error)){
	    continue;
	}
	if (!currDir)
	    continue;
	while((currName=(gchar *) g_dir_read_name(currDir))!=NULL){
	    if (g_pattern_match(pSpec,strlen(currName),currName,NULL)){
		currPath=g_build_filename(searchDirs[i], currName, NULL);
		MKDG_DEBUG_MSG(6, "[I6] config_set_prepare_files(), currPath=%s",  currPath);
		if (g_access(currPath, R_OK)==0){
		    if (!(configSet->flags & MKDG_CONFIG_FLAG_READONLY)){
			if (g_access(currPath, W_OK)==0){
			    if (!(configSet->flags & MKDG_CONFIG_FLAG_NO_OVERRIDE)){
				configSet->writeIndex=counter;
			    }else if (configSet->writeIndex < 0){
				configSet->writeIndex=counter;
			    }
			}
		    }
		    MKDG_DEBUG_MSG(6, "[I6] config_set_prepare_files(), writeIndex=%d", configSet->writeIndex );
		    MkdgConfigFile *configFile=mkdg_config_file_new(currPath, configSet);
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
	    && !(configSet->flags & MKDG_CONFIG_FLAG_READONLY)
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
	    MkdgConfigFile *configFile=mkdg_config_file_new(
		    g_build_filename(searchDirs[i], configSet->defaultFilename, NULL), configSet);
	    g_ptr_array_add(configSet->fileArray, configFile);
	    configSet->writeIndex=counter;
	    counter++;
	}
    }

    if (counter==0){
	cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_CANT_READ,"config_set_prepare_files()");
	mkdg_error_handle(cfgErr, error);
	return FALSE;
    }
    if (configSet->writeIndex<0 && !(configSet->flags & MKDG_CONFIG_FLAG_READONLY)){
	cfgErr=mkdg_error_new(MKDG_ERROR_CONFIG_CANT_WRITE,"config_set_prepare_files()");
	mkdg_error_handle(cfgErr, error);
	return FALSE;
    }
    return TRUE;
}

gboolean mkdg_config_set_foreach_page(MkdgConfigSet *configSet,
	MkdgConfigSetEachPageFunc func,  gpointer userData, MkdgError **error){
    gboolean clean=TRUE;
    MkdgError *cfgErr=NULL;
    const gchar *page=NULL;
    gboolean ret;
    if (configSet->pageNames){
	gsize i;
	for(i=0;(page=configSet->pageNames[i])!=NULL;i++){
	    cfgErr=NULL;
	    ret=func(configSet,page, userData, &cfgErr);
	    if (!ret){
		clean=FALSE;
		mkdg_error_handle(cfgErr, error);
		if (configSet->flags & MKDG_CONFIG_FLAG_STOP_ON_ERROR){
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
		mkdg_error_handle(cfgErr, error);
		if (configSet->flags & MKDG_CONFIG_FLAG_STOP_ON_ERROR){
		    return clean;
		}
	    }
	}
    }
    return clean;
}


