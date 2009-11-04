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

static GError *convert_error_code(GError *error, const gchar *prefix){
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
    g_warning("MakerDialogConfigKeyFile: %s: domain=%s code=%d:%s", prefix, g_quark_to_string(error->domain), error->code, error->message);
    g_error_free(error);
    return cfgErr;
}


/*=== Start Config handler callbacks ===*/
static gpointer maker_dialog_config_key_file_create(MakerDialogConfigSet *dlgCfgSet, MakerDialogConfigFile *configFile, GError **error){
    return configFile;
}

static gpointer  maker_dialog_config_key_file_open(MakerDialogConfigSet *dlgCfgSet, MakerDialogConfigFile *configFile, GError **error){
    return configFile;
}

static gboolean maker_dialog_config_key_file_close(MakerDialogConfigSet *dlgCfgSet, MakerDialogConfigFile *configFile, GError **error){
    if (dlgCfgSet->userData){
	g_key_file_free((GKeyFile *)dlgCfgSet->userData);
	dlgCfgSet->userData=NULL;
    }
    return TRUE;
}

static gboolean maker_dialog_config_key_file_preload_to_buffer(MakerDialogConfigSet *dlgCfgSet, GKeyFile *keyFile, const gchar *pageName, GError **error){
    MAKER_DIALOG_DEBUG_MSG(4, "[I4] config_key_file_preload_to_buffer( , , %s, )", (pageName)? pageName: "");
    GError *cfgErr=NULL, *cfgErr_prep=NULL, *cfgErr_last=NULL;
    gchar **keys=g_key_file_get_keys(keyFile, pageName, NULL, &cfgErr_prep);
    if ((cfgErr=convert_error_code(cfgErr_prep, "config_key_file_preload_to_buffer()"))){
	goto FILE_LOAD_TO_BUFFER_END;
    }
    gsize i;
    for(i=0; keys[i]!=NULL; i++){
	MAKER_DIALOG_DEBUG_MSG(4, "[I4] config_key_file_preload_to_buffer( , , %s, ) loading key=%s", (pageName)? pageName: "", keys[i]);
	MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(dlgCfgSet->mDialog,keys[i]);
	if (!ctx){
	    if (dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		cfgErr=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_INVALID_KEY, "config_key_file_preload_to_buffer()");
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
	gdouble dValue;
	gchar *strValue;
	switch(ctx->spec->valueType){
	    case MKDG_TYPE_BOOLEAN:
		bValue=g_key_file_get_boolean(keyFile, pageName,keys[i], &cfgErr_prep);
		if (!cfgErr_prep){
		    g_value_init(value, ctx->spec->valueType);
		    g_value_set_boolean(value, bValue);
		    ctx->flags &= ~(MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNSAVED);
		    ctx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED | MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_HAS_VALUE;
		}
		break;
	    case MKDG_TYPE_INT:
	    case MKDG_TYPE_UINT:
	    case MKDG_TYPE_LONG:
	    case MKDG_TYPE_ULONG:
	    case MKDG_TYPE_FLOAT:
	    case MKDG_TYPE_DOUBLE:
		dValue=g_key_file_get_double(keyFile, pageName, keys[i],  &cfgErr_prep);
		if (!cfgErr_prep){
		    g_value_init(value, ctx->spec->valueType);
		    maker_dialog_value_set_number(value,dValue);
		    ctx->flags &= ~(MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNSAVED);
		    ctx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED | MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_HAS_VALUE;
		}
		break;
	    case MKDG_TYPE_STRING:
		strValue=g_key_file_get_string(keyFile, pageName, keys[i],  &cfgErr_prep);
		if (!cfgErr_prep){
		    g_value_init(value, ctx->spec->valueType);
		    g_value_set_string(value,  strValue);
		    ctx->flags &= ~(MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNSAVED);
		    ctx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED | MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_HAS_VALUE;
		}
		break;
	    default:
		cfgErr_prep=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_INVALID_FORMAT, "config_key_file_preload_to_buffer()");
		break;
	}
	cfgErr=convert_error_code(cfgErr_prep, "config_key_file_preload_to_buffer()");
	if (cfgErr){
	    if (dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
		goto FILE_LOAD_TO_BUFFER_END;
	    }
	    if (cfgErr_last){
		maker_dialog_config_error_print(cfgErr_last);
		g_error_free(cfgErr_last);
	    }
	    cfgErr_last=cfgErr;
	}else{
	    gchar *str=maker_dialog_value_to_string(value, ctx->spec->toStringFormat);
	    MAKER_DIALOG_DEBUG_MSG(2, "[I2] Load key %s with value %s", keys[i], str );
	    g_free(str);
	    g_hash_table_insert(dlgCfgSet->dlgCfgBuf->keyValueTable, g_strdup(keys[i]), value);
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

static gboolean maker_dialog_config_key_file_preload(MakerDialogConfigSet *dlgCfgSet, MakerDialogConfigFile *configFile, const gchar *pageName, GError **error){
    if (dlgCfgSet->currentIndex==0){
	if (dlgCfgSet->userData){
	    g_key_file_free((GKeyFile *)dlgCfgSet->userData);
	}
	dlgCfgSet->userData=(gpointer) g_key_file_new();
    }

    GError * cfgErr=NULL, *cfgErr_prep=NULL;
    MAKER_DIALOG_DEBUG_MSG(3, "[I3] config_key_file_preload( , %s, %s, )",configFile->path, (pageName)? pageName: "");
    if (g_access(configFile->path,R_OK)==0){
	g_assert(dlgCfgSet->userData);
	GKeyFile *keyFile=(GKeyFile *) dlgCfgSet->userData;

	g_key_file_load_from_file(keyFile, configFile->path, G_KEY_FILE_NONE, &cfgErr_prep);
	if (cfgErr_prep && cfgErr_prep->code==G_KEY_FILE_ERROR_PARSE){
	    /* Possibly empty file */
	    maker_dialog_config_error_print(cfgErr_prep);
	    g_free(cfgErr_prep);
	    goto FILE_LOAD_END;
	}
	if ((cfgErr=convert_error_code(cfgErr_prep, "config_key_file_preload():g_key_file_load_from_file"))){
	    goto FILE_LOAD_END;
	}
	if (pageName){
	    maker_dialog_config_key_file_preload_to_buffer(dlgCfgSet, keyFile, pageName, &cfgErr);
	}else{
	    /* All pages in the configuration set */
	    gsize i;
	    gchar **groups=g_key_file_get_groups(keyFile, NULL);
	    for(i=0; groups[i]!=NULL; i++){
		maker_dialog_config_key_file_preload_to_buffer(dlgCfgSet, keyFile, groups[i], &cfgErr);
		if (cfgErr && dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR ){
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
    const gchar *pageName;
    MakerDialogConfigSet *dlgCfgSet;
    FILE *outF;
    GError **error;
};

static void maker_dialog_keyfile_save_private(MakerDialog *mDialog, MakerDialogPropertyContext *ctx, gpointer userData){
    struct SaveFileBind *sBind=(struct SaveFileBind *) userData;
    /* Check whether the line need to be saved */
    gboolean needSave=TRUE;
    GValue *bufValue=(GValue *)g_hash_table_lookup(sBind->dlgCfgSet->dlgCfgBuf->keyValueTable, ctx->spec->key);
    if (bufValue){
	if ((sBind->dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_HIDE_DUPLICATE) && maker_dialog_value_compare(bufValue,&ctx->value, NULL)==0){
	    MAKER_DIALOG_DEBUG_MSG(4, "[I4] config_key_file_save_private() duplicated, no need to save.");
	    needSave=FALSE;
	}
    }else{
	if ((sBind->dlgCfgSet->flags & MAKER_DIALOG_CONFIG_FLAG_HIDE_DEFAULT) && maker_dialog_property_is_default(ctx)){
	    MAKER_DIALOG_DEBUG_MSG(4, "[I4] config_key_file_save_private() is default value, no need to save.");
	    needSave=FALSE;
	}
    }
    MAKER_DIALOG_DEBUG_MSG(4, "[I4] config_key_file_save_private(-,%s,%s) needSave=%s",ctx->spec->key, (sBind->pageName)? sBind->pageName: "-", (needSave)? "TRUE": "FALSE");

    if (needSave){
	if (sBind->counter==0){
	    fprintf(sBind->outF,"[%s]\n",sBind->pageName);
	}
	if (ctx->spec->valueType==MKDG_TYPE_BOOLEAN){
	    /* GKeyFile only accept "true" and  "false" */
	    fprintf(sBind->outF,"%s=%s\n",ctx->spec->key, (g_value_get_boolean(&ctx->value))? "true" : "false");
	}else{
	    fprintf(sBind->outF,"%s=%s\n",ctx->spec->key, maker_dialog_value_to_string(&ctx->value, ctx->spec->toStringFormat));
	}
	ctx->flags&=~MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNSAVED;
	sBind->counter++;
    }
}

static gboolean maker_dialog_config_key_file_save(MakerDialogConfigSet *dlgCfgSet, MakerDialogConfigFile *configFile, const gchar *pageName, GError **error){
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
    const gchar *page=NULL;
    struct SaveFileBind sBind;
    sBind.counter=0;
    sBind.dlgCfgSet=dlgCfgSet;
    sBind.outF=outF;
    sBind.error=error;

    if (dlgCfgSet->pageNames){
	gsize i;
	for(i=0;(page=dlgCfgSet->pageNames[i])!=NULL;i++){
	    if (sBind.counter>0)
		fprintf(outF,"\n");
	    sBind.pageName=page;
	    maker_dialog_page_foreach_property(dlgCfgSet->mDialog, page,  maker_dialog_keyfile_save_private, (gpointer) &sBind);
	}
    }else{
	GNode *pageNode=NULL;
	for(pageNode=g_node_first_child(dlgCfgSet->mDialog->pageRoot);pageNode!=NULL; pageNode=g_node_next_sibling(pageNode)){
	    page=(gchar *) pageNode->data;
	    if (sBind.counter>0)
		fprintf(outF,"\n");
	    sBind.pageName=page;
	    maker_dialog_page_foreach_property(dlgCfgSet->mDialog, page, maker_dialog_keyfile_save_private, (gpointer) &sBind);
	}
    }
    fclose(outF);
    return TRUE;
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

