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

static GError *convert_error_code(GError *error, const gchar *filename, const gchar *prefix){
    if (!error)
	return NULL;
    if (error->domain==MAKER_DIALOG_CONFIG_ERROR)
	return error;

    GError *cfgErr=NULL;
    if (error->domain==G_FILE_ERROR){
	switch(error->code){
	    case G_FILE_ERROR_ACCES:
		cfgErr=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_PERMISSION_DENY, prefix);
		break;
	    default:
		cfgErr=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_OTHER, prefix);
		break;
	}
    }else if (error->domain==G_KEY_FILE_ERROR){
	switch(error->code){
	    case G_KEY_FILE_ERROR_UNKNOWN_ENCODING:
	    case G_KEY_FILE_ERROR_PARSE:
		cfgErr=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_INVALID_FORMAT, prefix);
		break;
	    case G_KEY_FILE_ERROR_NOT_FOUND:
		cfgErr=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_NOT_FOUND, prefix);
		break;
	    default:
		cfgErr=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_OTHER, prefix);
		break;
	}
    }else{
	cfgErr=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_OTHER, prefix);
    }
    g_warning("MakerDialogConfigKeyFile: %s%s %s: domain=%s code=%d:%s",
	    (filename)? "On file: " : "",
	    (filename)? filename : "",
	    prefix, g_quark_to_string(error->domain), error->code, error->message);
    g_error_free(error);
    return cfgErr;
}

/*=== Start Config interface callbacks ===*/
static gpointer maker_dialog_config_key_file_create(MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, GError **error){
    if (!configSet->userData){
	configSet->userData=(gpointer) g_key_file_new();
    }
    return configSet->userData;
}

static gpointer  maker_dialog_config_key_file_open(MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, GError **error){
    if (!configSet->userData){
	configSet->userData=(gpointer) g_key_file_new();
    }
    return configSet->userData;
}

static gboolean maker_dialog_config_key_file_close(MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, GError **error){
    if (configSet->userData){
	g_key_file_free((GKeyFile *)configSet->userData);
	configSet->userData=NULL;
    }
    return TRUE;
}

static gboolean maker_dialog_config_key_file_preload_to_buffer(
	MakerDialogConfigSet *configSet, GKeyFile *keyFile, const gchar *pageName, GError **error){
    MAKER_DIALOG_DEBUG_MSG(4, "[I4] config_key_file_preload_to_buffer( , , %s, )", (pageName)? pageName: "");
    GError *cfgErr=NULL, *cfgErr_prep=NULL, *cfgErr_last=NULL;
    gchar **keys=g_key_file_get_keys(keyFile, pageName, NULL, &cfgErr_prep);
    if ((cfgErr=convert_error_code(cfgErr_prep, NULL, "config_key_file_preload_to_buffer()"))){
	goto FILE_LOAD_TO_BUFFER_END;
    }
    gsize i;
    for(i=0; keys[i]!=NULL; i++){
	MAKER_DIALOG_DEBUG_MSG(4, "[I4] config_key_file_preload_to_buffer( , , %s, ) loading key=%s",
		(pageName)? pageName: "", keys[i]);
	MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(configSet->mDialog,keys[i]);
	if (!ctx){
	    if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		cfgErr=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_INVALID_KEY,
		       	"config_key_file_preload_to_buffer()");
		goto FILE_LOAD_TO_BUFFER_END;
	    }else{
		continue;
	    }
	}
	if ((ctx->flags & MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_HAS_VALUE)
		&& (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_NO_OVERRIDE)){
	    /* Has value and NO_OVERRIDE is set */
	    continue;
	}
	MkdgValue *mValue=maker_dialog_value_new(ctx->spec->valueType, NULL);
	gchar *strValue=g_key_file_get_string(keyFile, pageName,keys[i], &cfgErr_prep);
	if (maker_dialog_value_from_string(mValue,strValue,NULL)){
	    if (!cfgErr_prep){
		ctx->flags &= ~(MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNSAVED);
		ctx->flags |=
		    MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED | MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_HAS_VALUE;
	    }
	}else{
	    /* Unsupported type */
	    cfgErr_prep=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_INVALID_FORMAT,
		    "config_key_file_preload_to_buffer()");
	}

	cfgErr=convert_error_code(cfgErr_prep, NULL, "config_key_file_preload_to_buffer()");
	if (cfgErr){
	    if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		goto FILE_LOAD_TO_BUFFER_END;
	    }
	    if (cfgErr_last){
		maker_dialog_config_error_print(cfgErr_last);
		g_error_free(cfgErr_last);
	    }
	    cfgErr_last=cfgErr;
	}else{
	    if (MAKER_DIALOG_DEBUG_RUN(2)){
		gchar *str=maker_dialog_value_to_string(mValue, ctx->spec->toStringFormat);
		MAKER_DIALOG_DEBUG_MSG(2, "[I2] Load key %s with value %s", keys[i], str );
		g_free(str);
	    }
	    maker_dialog_config_buffer_insert(configSet->configBuf, keys[i], mValue);
	}
    }
FILE_LOAD_TO_BUFFER_END:
    g_strfreev(keys);
    if (cfgErr){
	if (error){
	    *error=cfgErr;
	}else{
	    maker_dialog_config_error_print(cfgErr);
	    g_error_free(cfgErr);
	}
	return FALSE;
    }
    return TRUE;
}

static gboolean maker_dialog_config_key_file_preload(MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, const gchar *pageName, GError **error){
    if (configSet->currentIndex==0){
	if (configSet->userData){
	    g_key_file_free((GKeyFile *)configSet->userData);
	}
	configSet->userData=(gpointer) g_key_file_new();
    }

    GError * cfgErr=NULL, *cfgErr_prep=NULL;
    MAKER_DIALOG_DEBUG_MSG(3, "[I3] config_key_file_preload( , %s, %s, )",configFile->path, (pageName)? pageName: "");
    if (g_access(configFile->path,R_OK)==0){
	g_assert(configSet->userData);
	GKeyFile *keyFile=(GKeyFile *) configSet->userData;

	g_key_file_load_from_file(keyFile, configFile->path, G_KEY_FILE_NONE, &cfgErr_prep);
	if (cfgErr_prep && cfgErr_prep->code==G_KEY_FILE_ERROR_PARSE){
	    /* Possibly empty file */
	    g_warning("On file %s", configFile->path);
	    maker_dialog_config_error_print(cfgErr_prep);
	    g_error_free(cfgErr_prep);
	    goto FILE_LOAD_END;
	}
	if ((cfgErr=convert_error_code(cfgErr_prep, configFile->path,
			"config_key_file_preload(): g_key_file_load_from_file"))){
	    goto FILE_LOAD_END;
	}
	if (pageName){
	    maker_dialog_config_key_file_preload_to_buffer(configSet, keyFile, pageName, &cfgErr);
	}else{
	    /* All pages in the configuration set */
	    gsize i;
	    gchar **groups=g_key_file_get_groups(keyFile, NULL);
	    for(i=0; groups[i]!=NULL; i++){
		maker_dialog_config_key_file_preload_to_buffer(configSet, keyFile, groups[i], &cfgErr);
		if (cfgErr && configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR ){
		    g_strfreev(groups);
		    goto FILE_LOAD_END;
		}
	    }
	    g_strfreev(groups);
	}
    }
FILE_LOAD_END:
    if (cfgErr){
	if (error){
	    *error=cfgErr;
	}else{
	    maker_dialog_config_error_print(cfgErr);
	    g_error_free(cfgErr);
	}
	return FALSE;
    }
    return TRUE;
}

struct SaveFileBind{
    gint counter;
    MakerDialogConfigSet *configSet;
    FILE *outF;
    GError **error;
};

static void maker_dialog_keyfile_save_private(MakerDialog *mDialog, MakerDialogPropertyContext *ctx, gpointer userData){
    MAKER_DIALOG_DEBUG_MSG(5, "[I5] conf_keyfile_save_private( , %s, )", ctx->spec->key);
    struct SaveFileBind *sBind=(struct SaveFileBind *) userData;
    /* Check whether the line need to be saved */
    gboolean needSave=TRUE;
    MkdgValue *bufValue=(MkdgValue *)g_hash_table_lookup(sBind->configSet->configBuf->keyValueTable, ctx->spec->key);
    if (bufValue){
	if ((sBind->configSet->flags & MAKER_DIALOG_CONFIG_FLAG_HIDE_DUPLICATE)
		&& maker_dialog_value_compare(bufValue, ctx->value, ctx->spec->compareOption)==0){
	    MAKER_DIALOG_DEBUG_MSG(4, "[I4] config_key_file_save_private() duplicated, no need to save.");
	    needSave=FALSE;
	}
    }else{
	if ((sBind->configSet->flags & MAKER_DIALOG_CONFIG_FLAG_HIDE_DEFAULT) && maker_dialog_property_is_default(ctx)){
	    MAKER_DIALOG_DEBUG_MSG(4, "[I4] config_key_file_save_private() is default value, no need to save.");
	    needSave=FALSE;
	}
    }
    MAKER_DIALOG_DEBUG_MSG(4, "[I4] config_key_file_save_private( , %s, ) page=%s needSave=%s",
	    ctx->spec->key, (ctx->spec->pageName)? ctx->spec->pageName: "-", (needSave)? "TRUE": "FALSE");

    if (needSave){
	if (sBind->counter==0){
	    fprintf(sBind->outF,"[%s]\n",ctx->spec->pageName);
	}
	if (ctx->spec->valueType==MKDG_TYPE_BOOLEAN){
	    /* GKeyFile only accept "true" and  "false" */
	    fprintf(sBind->outF,"%s=%s\n",ctx->spec->key, (maker_dialog_value_get_boolean(ctx->value))? "true" : "false");
	}else{
	    fprintf(sBind->outF,"%s=%s\n",ctx->spec->key, maker_dialog_property_to_string(ctx));
	}
	ctx->flags&=~MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNSAVED;
	sBind->counter++;
    }
}

static gboolean maker_dialog_config_key_file_config_set_each_page(MakerDialogConfigSet *configSet, const gchar *pageName, gpointer userData, GError **error){
    MAKER_DIALOG_DEBUG_MSG(5, "[I5] config_key_file_config_set_each_page( , %s, , )", (pageName)? pageName: "-");
    maker_dialog_page_foreach_property(configSet->mDialog, pageName, NULL, NULL, maker_dialog_keyfile_save_private, userData);
    return TRUE;
}

static gboolean maker_dialog_config_key_file_save(MakerDialogConfigSet *configSet,
	MakerDialogConfigFile *configFile, const gchar *pageName, GError **error){
    MAKER_DIALOG_DEBUG_MSG(3, "[I3] config_key_file_save(-,%s,%s)",configFile->path, (pageName)? pageName: "-");
    if (!maker_dialog_file_isWritable(configFile->path)){
	if (error){
	    *error=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_CANT_WRITE, "config_key_file_save()");
	}
	return FALSE;
    }
    FILE *outF=fopen(configFile->path,"w+");
    if (!outF){
	if (error){
	    *error=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_CANT_WRITE, "config_key_file_save()");
	}
	return FALSE;
    }
    struct SaveFileBind sBind;
    sBind.counter=0;
    sBind.configSet=configSet;
    sBind.outF=outF;
    sBind.error=error;
    gboolean ret=maker_dialog_config_set_foreach_page(configSet, maker_dialog_config_key_file_config_set_each_page, (gpointer) &sBind, error);
    fclose(outF);
    return ret;
}

//static gchar **maker_dialog_config_key_file_get_pages(MakerDialogConfigSet *configSet,
//        MakerDialogConfigFile *configFile, const gchar *pageName, GError **error){
//    MAKER_DIALOG_DEBUG_MSG(3, "[I3] config_key_file_get_pages(-,%s,%s)",configFile->path, (pageName)? pageName: "-");
//    if (configSet->userData){
//        if (error){
//            *error=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_NO_CONFIG_SET, "config_key_file_get_pages()");
//        }
//        return NULL;
//    }
//    return g_key_file_get_groups((GKeyFile *) configSet->userData, NULL);
//}
/*=== End Config interface callbacks ===*/

MakerDialogConfigInterface makerDialogConfigKeyFileInterface={
    maker_dialog_config_key_file_create,
    maker_dialog_config_key_file_open,
    maker_dialog_config_key_file_close,
    maker_dialog_config_key_file_preload,
    maker_dialog_config_key_file_save,
    NULL
};

MakerDialogConfig *maker_dialog_config_use_key_file(MakerDialog *mDialog){
    MakerDialogConfig *config=maker_dialog_config_new(mDialog, TRUE, &makerDialogConfigKeyFileInterface);
    return config;
}

