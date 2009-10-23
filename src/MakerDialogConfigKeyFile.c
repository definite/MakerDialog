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
typedef struct KeyFileHandle{
    const char *filename;
    GKeyFile *keyFile;
};

static MakerDialogConfigError error_tmp;

static MakerDialogConfigError set_error_code(GError *error, const gchar *prefix){
    MakerDialogConfigError error_code=MAKER_DIALOG_CONFIG_OK;
    if (error){
	if (error->domain==G_FILE_ERROR){
	    switch(error->code){
		case G_FILE_ERROR_ACCES:
		    error_code=MAKER_DIALOG_CONFIG_ERROR_PERMISSION_DENY;
		    break;
		default:
		    error_code=MAKER_DIALOG_CONFIG_ERROR_OTHER;
	    }
	}else if (error->domain==G_KEY_FILE_ERROR){
	    switch(error->code){
		case G_KEY_FILE_ERROR_UNKNOWN_ENCODING:
		case G_KEY_FILE_ERROR_PARSE:
		    error_code=MAKER_DIALOG_CONFIG_ERROR_INVALID_FORMAT;
		    break;
		case G_KEY_FILE_ERROR_NOT_FOUND:
		    error_code=MAKER_DIALOG_CONFIG_ERROR_NOT_FOUND;
		    break;
		default:
		    error_code=MAKER_DIALOG_CONFIG_ERROR_OTHER;
	    }
	}else{
	    error_code=MAKER_DIALOG_CONFIG_ERROR_OTHER;
	}
	g_warning("MakerDialogConfigKeyFile: %s: %s", prefix, error->message);
	g_error_free(error);
    }
    return error_code;
}


/*=== Start Config handler callbacks ===*/
static void key_file_handle_free(KeyFileHandle *keyFileHandle){
    g_key_file_free(keyFileHandle->keyFile);
    g_free(keyFileHandle);
}

static KeyFileHandler *key_file_handle_new(const gchar *filename){
    KeyFileHandle *keyFileHandle=g_new(KeyFileHandler,1);
    keyFileHandle->filename=filename;
    keyFileHandle->keyFile=g_key_file_new();
    error_tmp=MAKER_DIALOG_CONFIG_OK;
    return keyFileHandle;
}

static MakerDialogConfigError maker_dialog_config_key_file_create(MakerDialogConfigSet *dlgCfgSet, gpointer *configFile, const gchar *filename){
    KeyFileHandle *keyFileHandle=key_file_handle_new(filename);
    configFile=(gpointer) &keyFileHandle;
    return error_tmp;
}

static MakerDialogConfigError maker_dialog_config_key_file_open(MakerDialogConfigSet *dlgCfgSet, gpointer *configFile, const gchar *filename){
    KeyFileHandle *keyFileHandle=key_file_handle_new(filename);
    configFile=(gpointer) &keyFileHandle;
    return error_tmp;
}

static MakerDialogConfigError maker_dialog_config_key_file_close(MakerDialogConfigSet *dlgCfgSet, gpointer configFile){
    KeyFileHandle *keyFileHandle=(KeyFileHandle *) configFile;
    key_file_handle_free(keyFileHandle);
    return MAKER_DIALOG_CONFIG_OK;
}

static MakerDialogConfigError maker_dialog_config_key_file_load_page(MakerDialogConfigSet *dlgCfgSet, KeyFileHandle *keyFileHandle, const gchar *pageName){
    gchar **keys=g_key_file_get_keys(keyFileHandle->keyFile, pageName, NULL, &error);
    if (!(error_tmp=set_error_code(error, "maker_dialog_config_key_file_load():g_key_file_get_keys"))){
	goto FILE_LOAD_PAGE_END;
    }
    gsize i;
    for(i=0; keys[i]!=NULL; i++){
	MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(dlgCfgSet->mDialog,keys[i]);
	if (!ctx){
	    if (dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		error_tmp=MAKER_DIALOG_CONFIG_ERROR_INVALID_KEY;
		goto FILE_LOAD_PAGE_END;
	    }else{
		continue;
	    }
	}
	if ((ctx->flags & MAKER_DIALOG_PROPERTY_CONTEXT_HAS_VALUE)
		&& (dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_NO_OVERRIDE)){
	    /* Has value and NO_OVERRIDE is set */
	    continue;
	}
	GValue *value=g_new0(GValue, 1);
	switch(ctx->spec->valueType){
	    case G_TYPE_BOOLEAN:
		gboolean bValue=g_key_file_get_boolean(keyFileHandle->keyFile, pageName,keys[i], &error);
		if (!error){
		    g_value_init(value, ctx->spec->valueType);
		    g_value_set_boolean(value, bValue);
		    ctx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED.
		}
		break;
	    case G_TYPE_INT:
		gint intValue=g_key_file_get_integer(keyFileHandle->keyFile, pageName,keys[i], &error);
		if (!error){
		    g_value_init(value, ctx->spec->valueType);
		    g_value_set_int(value, intValue);
		    ctx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED.
		}
		break;
	    case G_TYPE_UINT:
		gint uintValue=g_key_file_get_integer(keyFileHandle->keyFile, pageName, keys[i], &error);
		if (!error){
		    g_value_init(value, ctx->spec->valueType);
		    g_value_set_uint(value, (guint) uintValue);
		    ctx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED.
		}
		break;
	    case G_TYPE_DOUBLE:
		gint dValue=g_key_file_get_double(keyFileHandle->keyFile, pageName, keys[i], &error);
		if (!error){
		    g_value_init(value, ctx->spec->valueType);
		    g_value_set_double(value, (guint) dValue);
		    ctx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED.
		}
		break;
	    case G_TYPE_STRING:
		gint strValue=g_key_file_get_string(keyFileHandle->keyFile, pageName, keys[i], &error);
		if (!error){
		    g_value_init(value, ctx->spec->valueType);
		    g_value_set_string(value, (guint) strValue);
		    ctx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED.
		}
		break;
	    default:
		break;
	}
	error_tmp=set_error_code(error, "maker_dialog_config_key_file_load():g_key_file_get");
	if (dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR && error_tmp){
	    goto FILE_LOAD_PAGE_END;
	}
	g_hash_table_insert(dlgCfgSet->dlgCfgBuf->keyValueTable, g_strdup(keys[i]), &value);
    }
FILE_LOAD_PAGE_END:
    g_strfreev(keys);
    return error_tmp;
}

static MakerDialogConfigError maker_dialog_config_key_file_load(MakerDialogConfigSet *dlgCfgSet, gpointer configFile){
    GError *error=NULL;
    KeyFileHandle *keyFileHandle=(KeyFileHandle *) configFile;
    if (g_access(keyFileHandle->filename,R_OK)==0){
	g_key_file_load_from_file(keyFileHandle->keyFile, keyFileHandle->filename, G_KEY_FILE_NONE, 0, &error);
	if (!(error_tmp=set_error_code(error, "maker_dialog_config_key_file_load():g_key_file_load_from_file"))){
	    key_file_handle_free(keyFileHandle);
	    return error_tmp;
	}
	if (dlgCfgSet->workingPageName){
	    error_tmp=maker_dialog_config_key_file_load_page(dlgCfgSet, keyFileHandle, dlgCfgSet->workingPageName);
	}else{
	    /* All pages in the configuration set */
	    gsize i;
	    gchar **groups=g_key_file_get_groups(keyFileHandle->keyFile, NULL);
	    for(i=0; groups[i]!=NULL; i++){
		error_tmp=maker_dialog_config_key_file_load_page(dlgCfgSet, keyFileHandle, groups[i]);
		if (dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR && error_tmp){
		    g_strfreev(groups);
		    goto FILE_LOAD_END;
		}
	    }
	    g_strfreev(groups);
	}
    }
FILE_LOAD_END:
    key_file_handle_free(keyFileHandle);
    return error_tmp;
}

static MakerDialogConfigError maker_dialog_config_key_file_save(MakerDialogConfigSet *dlgCfgSet, gpointer configFile){
    GError *error=NULL;
    KeyFileHandle *keyFileHandle=(KeyFileHandle *) configFile;
    if (maker_dialog_file_isWritable(keyFileHandle->filename)){

	g_key_file_load_from_file(keyFileHandle->keyFile, keyFileHandle->filename, G_KEY_FILE_NONE, 0, &error);
	if (!(error_tmp=set_error_code(error, "maker_dialog_config_key_file_load():g_key_file_load_from_file"))){
	    key_file_handle_free(keyFileHandle);
	    return error_tmp;
	}
	if (dlgCfgSet->workingPageName){
	    error_tmp=maker_dialog_config_key_file_load_page(dlgCfgSet, keyFileHandle, dlgCfgSet->workingPageName);
	}else{
	    /* All pages in the configuration set */
	    gsize i;
	    gchar **groups=g_key_file_get_groups(keyFileHandle->keyFile, NULL);
	    for(i=0; groups[i]!=NULL; i++){
		error_tmp=maker_dialog_config_key_file_load_page(dlgCfgSet, keyFileHandle, groups[i]);
		if (dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR && error_tmp){
		    g_strfreev(groups);
		    goto FILE_LOAD_END;
		}
	    }
	    g_strfreev(groups);
	}
    }
FILE_LOAD_END:
    key_file_handle_free(keyFileHandle);
    return error_tmp;
}

/*=== End Config handler callbacks ===*/

const MakerDialogConfigHandler MakerDialogConfig={
    maker_dialog_config_key_file_create,
    maker_dialog_config_key_file_open,
    maker_dialog_config_key_file_close,
}
