/*
 * Copyright © 2010  Red Hat, Inc. All rights reserved.
 * Copyright © 2010  Ding-Yi Chen <dchen at redhat.com>
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

MakerDialogConfigFile *maker_dialog_config_file_new(const gchar *path, MakerDialogConfigSet *configSet){
    MakerDialogConfigFile *configFile=g_new(MakerDialogConfigFile,1);
    configFile->path=g_strdup(path);
    configFile->fileObj=NULL;
    configFile->configSet=configSet;
    configFile->flags=0;
    configFile->userData=NULL;
    return configFile;
}

void maker_dialog_config_file_free(MakerDialogConfigFile *configFile){
    g_free((gchar *)configFile->path);
    if (configFile->fileObj)
	g_free(configFile->fileObj);
    g_free(configFile);
}

MakerDialogConfigBuffer *maker_dialog_config_buffer_new(){
    MakerDialogConfigBuffer *configBuf=g_new(MakerDialogConfigBuffer, 1);
    configBuf->keyValueTable=g_hash_table_new_full(g_str_hash, g_str_equal, g_free, maker_dialog_value_free);
    return configBuf;
}

void maker_dialog_config_buffer_insert(MakerDialogConfigBuffer *configBuf, const gchar *key, MkdgValue *value){
    g_hash_table_insert(configBuf->keyValueTable, g_strdup(key), value);
}

MkdgValue *maker_dialog_config_buffer_lookup(MakerDialogConfigBuffer *configBuf, const gchar *key){
    return (MkdgValue *) g_hash_table_lookup(configBuf->keyValueTable, key);
}

void maker_dialog_config_buffer_free(MakerDialogConfigBuffer *configBuf){
    g_hash_table_destroy(configBuf->keyValueTable);
    g_free(configBuf);
}

