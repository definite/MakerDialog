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

#ifndef P

static MakerDialogConfigError maker_dialog_config_set_new_private(MakerDialogConfigSet dlgCfgSet){
    GPatternSpec* pSpec=g_pattern_spec_new (dlgCfgSet->filePattern);
    GError *error=NULL;
    GDir *currDir=NULL;
    gchar *currName=NULL;
    gint counter=0;
    gint i;
    for(i=0;dlgCfgSet->searchDirs[i]!=NULL;i++){
	currDir=g_dir_open(dlgCfgSet->searchDirs[i], 0, **error);
	if (!currDir)
	    continue;
	while((currName=g_dir_read_name(currDir))!=NULL){
	    if (g_pattern_match(pSpec,currName)){
		if (g_access(currName, R_OK)==0){
		    if (g_access(currName, W_OK)==0 && writeIndex<0){
			writeIndex=counter;
		    }
		    counter++;
		}
	    }
	}
	g_dir_close(currDir);
    }
    return MAKER_DIALOG_CONFIG_OK;

}

MakerDialogConfigSet *maker_dialog_config_set_open(const gchar **pageNames,
	MakerDialogConfigFlag flags, const gchar *filePattern, const char **searchDirs,
	const gchar *defaultFilename, gint maxFileCount, MakerDialogConfigError *errorCode){
    MakerDialogConfigSet *dlgCfgSet=g_new(MakerDialogConfigSet,1);
    dlgCfgSet->pageNames=pageNames;
    dlgCfgSet->flags=flages;
    dlgCfgSet->filePattern=filePattern;
    dlgCfgSet->searchDirs=serachDirs;
    dlgCfgSet->defaultFilename=defaultFilename;
    dlgCfgSet->maxFileCount=maxFileCount;
    dlgCfgSet->writeIndex=-1;
    dlgCfgSet->filenameChunk=g_string_chunk_new(200);
    dlgCfgSet->filenameArray=g_ptr_array_new();
    MakerDialogConfigError _errorCode=MakerDialogConfigError maker_dialog_config_set_open_private(dlgCfgSet);
    if (errorCode){
	*errorCode=_errorCode;
    }
    return dlgCfgSet;
}

