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
 *  along with Mkdg.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "MakerDialog.h"

MkdgConfigFile *mkdg_config_file_new(const gchar *path, MkdgConfigSet *configSet){
    MkdgConfigFile *configFile=g_new(MkdgConfigFile,1);
    configFile->path=g_strdup(path);
    configFile->fileObj=NULL;
    configFile->configSet=configSet;
    configFile->flags=0;
    configFile->userData=NULL;
    return configFile;
}

void mkdg_config_file_free(MkdgConfigFile *configFile){
    g_free((gchar *)configFile->path);
    if (configFile->fileObj)
	g_free(configFile->fileObj);
    g_free(configFile);
}

MkdgConfigBuffer *mkdg_config_buffer_new(){
    MkdgConfigBuffer *configBuf=g_new(MkdgConfigBuffer, 1);
    configBuf->keyValueTable=g_hash_table_new_full(g_str_hash, g_str_equal, g_free, mkdg_value_free);
    return configBuf;
}

void mkdg_config_buffer_insert(MkdgConfigBuffer *configBuf, const gchar *key, MkdgValue *value){
    g_hash_table_insert(configBuf->keyValueTable, g_strdup(key), value);
}

MkdgValue *mkdg_config_buffer_lookup(MkdgConfigBuffer *configBuf, const gchar *key){
    return (MkdgValue *) g_hash_table_lookup(configBuf->keyValueTable, key);
}

void mkdg_config_buffer_free(MkdgConfigBuffer *configBuf){
    g_hash_table_destroy(configBuf->keyValueTable);
    g_free(configBuf);
}

