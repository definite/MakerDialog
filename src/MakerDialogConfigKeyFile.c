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
#include <stdio.h>
#include <unistd.h>
#include <glib/gstdio.h>
#include "MakerDialog.h"

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
static MakerDialogConfigError maker_dialog_config_key_file_create(MakerDialogConfigSet *dlgCfgSet, MakerDialogConfigFile *configFile){
    return MAKER_DIALOG_CONFIG_OK;
}

static MakerDialogConfigError maker_dialog_config_key_file_open(MakerDialogConfigSet *dlgCfgSet, MakerDialogConfigFile *configFile){
    return MAKER_DIALOG_CONFIG_OK;
}

static MakerDialogConfigError maker_dialog_config_key_file_close(MakerDialogConfigSet *dlgCfgSet, MakerDialogConfigFile *configFile){
    if (dlgCfgSet->userData){
	g_key_file_free((GKeyFile *)dlgCfgSet->userData);
	dlgCfgSet->userData=NULL;
    }
    return MAKER_DIALOG_CONFIG_OK;
}

static MakerDialogConfigError maker_dialog_config_key_file_preload_to_buffer(MakerDialogConfigSet *dlgCfgSet, GKeyFile *keyFile, const gchar *pageName){
    GError *error=NULL;
    gchar **keys=g_key_file_get_keys(keyFile, pageName, NULL, &error);
    MakerDialogConfigError cfgErr=MAKER_DIALOG_CONFIG_OK;
    if (!(cfgErr=set_error_code(error, "maker_dialog_config_key_file_load():g_key_file_get_keys"))){
	goto FILE_LOAD_TO_BUFFER_END;
    }
    gsize i;
    for(i=0; keys[i]!=NULL; i++){
	MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(dlgCfgSet->mDialog,keys[i]);
	if (!ctx){
	    if (dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		cfgErr=MAKER_DIALOG_CONFIG_ERROR_INVALID_KEY;
		goto FILE_LOAD_TO_BUFFER_END;
	    }else{
		continue;
	    }
	}
	if ((ctx->flags & MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_HAS_VALUE)
		&& (dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_NO_OVERRIDE)){
	    /* Has value and NO_OVERRIDE is set */
	    continue;
	}
	GValue *value=g_new0(GValue, 1);
	gboolean bValue;
	gint intValue;
	guint uintValue;
	gdouble dValue;
	gchar *strValue;
	switch(ctx->spec->valueType){
	    case G_TYPE_BOOLEAN:
		bValue=g_key_file_get_boolean(keyFile, pageName,keys[i], &error);
		if (!error){
		    g_value_init(value, ctx->spec->valueType);
		    g_value_set_boolean(value, bValue);
		    ctx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED;
		}
		break;
	    case G_TYPE_INT:
		intValue=g_key_file_get_integer(keyFile, pageName,keys[i], &error);
		if (!error){
		    g_value_init(value, ctx->spec->valueType);
		    g_value_set_int(value, intValue);
		    ctx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED;
		}
		break;
	    case G_TYPE_UINT:
		uintValue=g_key_file_get_integer(keyFile, pageName, keys[i], &error);
		if (!error){
		    g_value_init(value, ctx->spec->valueType);
		    g_value_set_uint(value, (guint) uintValue);
		    ctx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED;
		}
		break;
	    case G_TYPE_DOUBLE:
		dValue=g_key_file_get_double(keyFile, pageName, keys[i], &error);
		if (!error){
		    g_value_init(value, ctx->spec->valueType);
		    g_value_set_double(value,  dValue);
		    ctx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED;
		}
		break;
	    case G_TYPE_STRING:
		strValue=g_key_file_get_string(keyFile, pageName, keys[i], &error);
		if (!error){
		    g_value_init(value, ctx->spec->valueType);
		    g_value_set_string(value,  strValue);
		    ctx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED;
		}
		break;
	    default:
		break;
	}
	cfgErr=set_error_code(error, "maker_dialog_config_key_file_load():g_key_file_get");
	if (cfgErr && dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
	    goto FILE_LOAD_TO_BUFFER_END;
	}
	g_hash_table_insert(dlgCfgSet->dlgCfgBuf->keyValueTable, g_strdup(keys[i]), &value);
    }
FILE_LOAD_TO_BUFFER_END:
    g_strfreev(keys);
    return cfgErr;
}

static MakerDialogConfigError maker_dialog_config_key_file_preload(MakerDialogConfigSet *dlgCfgSet, MakerDialogConfigFile *configFile, const gchar *pageName){
    GError *error=NULL;
    if (dlgCfgSet->currentIndex==0){
	if (dlgCfgSet->userData){
	    g_key_file_free((GKeyFile *)dlgCfgSet->userData);
	}
	dlgCfgSet->userData=(gpointer) g_key_file_new();
    }

    MakerDialogConfigError cfgErr=MAKER_DIALOG_CONFIG_OK;
    if (g_access(configFile->path,R_OK)==0){
	g_assert(dlgCfgSet->userData);
	GKeyFile *keyFile=(GKeyFile *) dlgCfgSet->userData;

	g_key_file_load_from_file(keyFile, configFile->path, G_KEY_FILE_NONE, &error);
	if (!(cfgErr=set_error_code(error, "maker_dialog_config_key_file_preload():preload from file"))){
	    goto FILE_LOAD_END;
	}
	if (pageName){
	    cfgErr=maker_dialog_config_key_file_preload_to_buffer(dlgCfgSet, keyFile, pageName);
	}else{
	    /* All pages in the configuration set */
	    gsize i;
	    gchar **groups=g_key_file_get_groups(keyFile, NULL);
	    for(i=0; groups[i]!=NULL; i++){
		cfgErr= maker_dialog_config_key_file_preload_to_buffer(dlgCfgSet, keyFile, groups[i]);
		if (dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR && error_tmp){
		    g_strfreev(groups);
		    goto FILE_LOAD_END;
		}
	    }
	    g_strfreev(groups);
	}
    }
FILE_LOAD_END:
    return cfgErr;
}

struct SaveFileBind{
    gint counter;
    const gchar *pageName;
    MakerDialogConfigSet *dlgCfgSet;
    FILE *outF;
};

static void maker_dialog_save_keyfile(MakerDialog *mDialog, MakerDialogPropertyContext *ctx, gpointer userData){
    struct SaveFileBind *sBind=(struct SaveFileBind *) userData;
    /* Check whether the line need to be saved */
    gboolean needSave=TRUE;
    GValue *bufValue=(GValue *)g_hash_table_lookup(sBind->dlgCfgSet->dlgCfgBuf->keyValueTable, ctx->spec->key);
    if (bufValue){
	if ((sBind->dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_HIDE_DUPLICATE) && maker_dialog_g_value_compare(bufValue,&ctx->value, NULL)==0){
	    needSave=FALSE;
	}
    }else{
	if ((sBind->dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_HIDE_DEFAULT) && maker_dialog_property_value_is_default(ctx)){
	    needSave=FALSE;
	}
    }

    if (needSave){
	if (sBind->counter==0){
	    fprintf(sBind->outF,"[%s]\n",sBind->pageName);
	}
	fprintf(sBind->outF,"%s=%s\n",sBind->pageName, maker_dialog_g_value_to_string(&ctx->value, NULL));
	sBind->counter++;
    }
}

static MakerDialogConfigError maker_dialog_config_key_file_save(MakerDialogConfigSet *dlgCfgSet, MakerDialogConfigFile *configFile, const gchar *pageName){
    if (!maker_dialog_file_isWritable(configFile->path)){
	return MAKER_DIALOG_CONFIG_ERROR_CANT_WRITE;
    }
    FILE *outF=fopen(configFile->path,"w+");
    if (!outF){
	return MAKER_DIALOG_CONFIG_ERROR_CANT_WRITE;
    }
    const gchar *page=NULL;
    struct SaveFileBind sBind;
    sBind.counter=0;
    sBind.dlgCfgSet=dlgCfgSet;
    sBind.outF=outF;

    if (dlgCfgSet->pageNames){
	gsize i;
	for(i=0;(page=dlgCfgSet->pageNames[i])!=NULL;i++){
	    if (sBind.counter>0)
		fprintf(outF,"\n");
	    maker_dialog_page_foreach_property(dlgCfgSet->mDialog, page, maker_dialog_save_keyfile, &sBind);
	}
    }else{
	GNode *pageNode=NULL;
	for(pageNode=g_node_first_child(dlgCfgSet->mDialog->pageRoot);pageNode!=NULL; pageNode=g_node_next_sibling(pageNode)){
	    page=(gchar *) pageNode->data;
	    if (sBind.counter>0)
		fprintf(outF,"\n");
	    maker_dialog_page_foreach_property(dlgCfgSet->mDialog, page, maker_dialog_save_keyfile, &sBind);
	}
    }
    fclose(outF);
    return MAKER_DIALOG_CONFIG_OK;
}

/*=== End Config handler callbacks ===*/

MakerDialogConfigHandler makerDialogConfigKeyFileHandler={
    maker_dialog_config_key_file_create,
    maker_dialog_config_key_file_open,
    maker_dialog_config_key_file_close,
    maker_dialog_config_key_file_preload,
    maker_dialog_config_key_file_save
};

MakerDialogConfig *maker_dialog_config_use_key_file(MakerDialog *mDialog){
    MakerDialogConfig *makerDialogConfigKeyFile=maker_dialog_config_new(mDialog, &makerDialogConfigKeyFileHandler);
    return makerDialogConfigKeyFile;
}

