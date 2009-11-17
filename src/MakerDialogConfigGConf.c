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
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <gconf/gconf-value.h>
#include <gconf/gconf-changeset.h>
#include "MakerDialog.h"
#include "MakerDialogConfigGConf.h"

/*=== Start Config interface callbacks ===*/
static gpointer maker_dialog_config_gconf_create
(MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, GError **error){
    if (!configSet->userData){
	configSet->userData=gconf_client_get_default();
    }
    configFile->filePointer=configSet->userData;
    return configSet->userData;
}

static gpointer  maker_dialog_config_gconf_open
(MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, GError **error){
    MAKER_DIALOG_DEBUG_MSG(5, "[I5] config_gconf_open( , %s, )", configFile->path);
    if (!configSet->userData){
	GConfClient *client=gconf_client_get_default();
	configSet->userData=client;
	gconf_client_preload(client, configFile->path,GCONF_CLIENT_PRELOAD_RECURSIVE,error);
    }
    configFile->filePointer=configSet->userData;
    return configSet->userData;
}

static gboolean maker_dialog_config_gconf_close
(MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, GError **error){
    MAKER_DIALOG_DEBUG_MSG(5, "[I5] config_gconf_close( , %s, )", configFile->path);
    configFile->filePointer=NULL;
    if (configSet->userData){
	gconf_client_clear_cache((GConfClient *) configSet->userData);
    }
    return TRUE;
}

static void maker_dialog_slist_free_individual(gpointer data, gpointer userData){
    g_free(data);
}

static gboolean maker_dialog_config_gconf_preload_to_buffer
(MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, const gchar *pageName, GError **error){
    GString *strBuf=g_string_new(configFile->path);
    g_string_append_printf(strBuf,"/%s/",pageName);
    GConfClient *client=(GConfClient *) configSet->userData;
    GSList *sListHead=gconf_client_all_entries(client,strBuf->str, error);
    GSList *sList;
    GError *cfgErr=NULL, *cfgErr_last=NULL;
    gboolean clean=TRUE;
    for(sList=sListHead;sList!=NULL; sList=g_slist_next(sList)){
	gchar *key=(gchar *)sList->data;
	GConfValue *cValue=gconf_client_get(client, key, &cfgErr);
	MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(configSet->mDialog,key);
	if (!ctx){
	    clean=FALSE;
	    if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR ){
		cfgErr=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_INVALID_KEY, "config_gconf_preload_to_buffer()");
		goto GCONF_LOAD_BUF_END;
	    }else{
		if (cfgErr_last){
		    maker_dialog_config_error_print(cfgErr_last);
		    g_error_free(cfgErr_last);
		}
		cfgErr_last=cfgErr;
		continue;
	    }
	}
	MkdgValue *mValue=maker_dialog_value_new(ctx->spec->valueType, NULL);
	switch(ctx->spec->valueType){
	    case MKDG_TYPE_BOOLEAN:
		maker_dialog_value_set_boolean(mValue, gconf_value_get_bool(cValue));
	        break;
	    case MKDG_TYPE_INT:
		maker_dialog_value_set_int(mValue, gconf_value_get_int(cValue));
		break;
	    case MKDG_TYPE_UINT:
		maker_dialog_value_set_uint(mValue, (guint) gconf_value_get_int(cValue));
		break;
	    case MKDG_TYPE_LONG:
		maker_dialog_value_set_long(mValue,  gconf_value_get_int(cValue));
		break;
	    case MKDG_TYPE_ULONG:
		maker_dialog_value_set_ulong(mValue, (gulong) gconf_value_get_int(cValue));
		break;
	    case MKDG_TYPE_FLOAT:
		maker_dialog_value_set_float(mValue, (gfloat) gconf_value_get_float(cValue));
	    case MKDG_TYPE_DOUBLE:
		maker_dialog_value_set_double(mValue, gconf_value_get_float(cValue));
		break;
	    case MKDG_TYPE_STRING:
	    case MKDG_TYPE_COLOR:
		maker_dialog_value_from_string(mValue, gconf_value_get_string(cValue), NULL);
		break;
	    default:
		break;
	}
	g_hash_table_insert(configSet->configBuf->keyValueTable, (gchar *) ctx->spec->key, mValue);
    }
GCONF_LOAD_BUF_END:
    if (cfgErr){
	if (error){
	    *error=cfgErr;
	}else{
	    maker_dialog_config_error_print(cfgErr);
	    g_error_free(cfgErr);
	}
	return FALSE;
    }
    g_slist_foreach(sListHead,maker_dialog_slist_free_individual,NULL);
    g_slist_free(sListHead);
    return clean;
}

static gboolean maker_dialog_config_gconf_preload
(MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, const gchar *pageName, GError **error){
    MAKER_DIALOG_DEBUG_MSG(5, "[I5] config_gconf_preload( , %s, %s, )", configFile->path, (pageName)? pageName : "");
    gboolean clean=TRUE,ret;
    GError *cfgErr=NULL, *cfgErr_last=NULL;
    if (pageName){
	clean=maker_dialog_config_gconf_preload_to_buffer(configSet, configFile, pageName, &cfgErr);
    }else{
	/* All pages in the configuration set */

	GConfClient *client=(GConfClient *) configSet->userData;
	GSList *sListHead=gconf_client_all_dirs(client,configFile->path,&cfgErr);
	GSList *sList=sListHead;
	if (!cfgErr){
	    gchar *path;
	    for(;sList!=NULL; sList=g_slist_next(sList)){
		path=(gchar *)sList->data;
		gchar *pageNameStr=g_path_get_basename(path);
		ret=maker_dialog_config_gconf_preload_to_buffer(configSet, configFile, pageNameStr, &cfgErr);
		g_free(pageNameStr);
		if (!ret){
		    clean=FALSE;
		    if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR ){
			break;
		    }else if (cfgErr_last){
			maker_dialog_config_error_print(cfgErr_last);
			g_error_free(cfgErr_last);
		    }
		    cfgErr_last=cfgErr;
		}
	    }
	}
	/* Free sList */
	g_slist_foreach(sListHead,maker_dialog_slist_free_individual,NULL);
	g_slist_free(sListHead);
    }
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

static void maker_dialog_config_gconf_save_property_private(MakerDialog *mDialog, MakerDialogPropertyContext *ctx, gpointer userData){
    if (ctx->flags & MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNSAVED){
	GConfChangeSet *cChangeSet=(GConfChangeSet *) userData;
	GConfValue *confValue=NULL;
	switch(ctx->spec->valueType){
	    case MKDG_TYPE_BOOLEAN:
		confValue=gconf_value_new(GCONF_VALUE_BOOL);
		gconf_value_set_bool(confValue, g_value_get_boolean(ctx->value->data));
		break;
	    case MKDG_TYPE_INT:
	    case MKDG_TYPE_UINT:
	    case MKDG_TYPE_LONG:
	    case MKDG_TYPE_ULONG:
		confValue=gconf_value_new(GCONF_VALUE_INT);
		gconf_value_set_int(confValue, (gint) maker_dialog_g_value_get_double(ctx->value->data));
		break;
	    case MKDG_TYPE_FLOAT:
		confValue=gconf_value_new(GCONF_VALUE_FLOAT);
		gconf_value_set_float(confValue, maker_dialog_g_value_get_double(ctx->value->data));
		break;
	    case MKDG_TYPE_STRING:
		confValue=gconf_value_new(GCONF_VALUE_STRING);
		gconf_value_set_string(confValue, maker_dialog_property_to_string(ctx));
		break;
	    case MKDG_TYPE_COLOR:
		confValue=gconf_value_new(GCONF_VALUE_STRING);
		gconf_value_set_string(confValue, maker_dialog_property_to_string(ctx));
		break;
	    default:
		break;
	}
	if (confValue){
	    gconf_change_set_set_nocopy(cChangeSet, ctx->spec->key, confValue);
	}
    }

}

static gboolean maker_dialog_config_gconf_save_private(MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile,
	GConfChangeSet *cChangeSet, const gchar *pageName, GError **error){
    MAKER_DIALOG_DEBUG_MSG(4, "[I4] config_gconf_save_private(-,%s,%s)",configFile->path, (pageName)? pageName: "-");
    maker_dialog_page_foreach_property(configSet->mDialog, pageName, NULL, NULL,
	    maker_dialog_config_gconf_save_property_private, (gpointer) cChangeSet);
    return TRUE;
}

static gboolean maker_dialog_config_gconf_save(MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, const gchar *pageName, GError **error){
    MAKER_DIALOG_DEBUG_MSG(3, "[I3] config_gconf_save(-,%s,%s)",configFile->path, (pageName)? pageName: "-");
    GConfClient *client=(GConfClient *) configSet->userData;
    GError *cfgErr=NULL, *cfgErr_last=NULL;
    gboolean clean=TRUE,ret;
    GConfChangeSet *cChangeSet=gconf_change_set_new();
    if (pageName){
	clean=maker_dialog_config_gconf_save_private(configSet, configFile, cChangeSet, pageName, &cfgErr);
    }else{
	/* All pages in the configuration set */
	GConfClient *client=(GConfClient *) configSet->userData;
	GSList *sListHead=gconf_client_all_dirs(client,configFile->path,&cfgErr);
	GSList *sList=sListHead;
	if (!cfgErr){
	    gchar *path;
	    for(;sList!=NULL; sList=g_slist_next(sList)){
		path=(gchar *)sList->data;
		gchar *pageNameStr=g_path_get_basename(path);
		ret=maker_dialog_config_gconf_save_private(configSet, configFile, cChangeSet, pageNameStr, &cfgErr);
		g_free(pageNameStr);
		if (!ret){
		    clean=FALSE;
		    if (configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR ){
			break;
		    }else if (cfgErr_last){
			maker_dialog_config_error_print(cfgErr_last);
			g_error_free(cfgErr_last);
		    }
		    cfgErr_last=cfgErr;
		}
	    }
	}
    }
    if (cfgErr && configSet->flags & MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR){
	if (error){
	    *error=cfgErr;
	}else{
	    maker_dialog_config_error_print(cfgErr);
	    g_error_free(cfgErr);
	}
	return FALSE;
    }

    GError *cfgErr2=NULL;
    ret=gconf_client_commit_change_set(client, cChangeSet, TRUE, &cfgErr2);
    if (cfgErr2 || cfgErr){
	if (cfgErr2 && cfgErr){
	    cfgErr_last=cfgErr2;
	    maker_dialog_config_error_print(cfgErr);
	    g_error_free(cfgErr);
	}else if (cfgErr2){
	    cfgErr_last=cfgErr2;
	}else{
	    cfgErr_last=cfgErr;
	}
	if (error){
	    *error=cfgErr_last;
	}else{
	    maker_dialog_config_error_print(cfgErr_last);
	    g_error_free(cfgErr_last);
	}
	return FALSE;
    }
    return TRUE;
}

/*=== End Config interface callbacks ===*/

MakerDialogConfigInterface makerDialogConfigGConfInterface={
    maker_dialog_config_gconf_create,
    maker_dialog_config_gconf_open,
    maker_dialog_config_gconf_close,
    maker_dialog_config_gconf_preload,
    maker_dialog_config_gconf_save,
    NULL
};

MakerDialogConfig *maker_dialog_config_use_gconf(MakerDialog *mDialog){
    MakerDialogConfig *config=maker_dialog_config_new(mDialog, FALSE, &makerDialogConfigGConfInterface);
    return config;
}

/*=== Start Schema file generation ===*/
typedef enum{
    XML_TAG_TYPE_NO_TAG,
    XML_TAG_TYPE_EMPTY,
    XML_TAG_TYPE_SHORT,
    XML_TAG_TYPE_LONG,
    XML_TAG_TYPE_BEGIN_ONLY,
    XML_TAG_TYPE_END_ONLY,
} XmlTagsType;

typedef struct{
    const gchar *schemasHome;
    const gchar *owner;
    const gchar **localeArray;
    gint indentSpace;
    FILE *outF;
} SchemasFileData;

static void append_indent_space(GString *strBuf, gint indentLevel, gint indentSpace){
    int i,indentLen=indentLevel*indentSpace;
    for(i=0;i<indentLen;i++){
	g_string_append_c(strBuf,' ');
    }
}

static GString *xml_tags_to_string(const gchar *tagName, XmlTagsType type,
	const gchar *attribute, const gchar *value,gint indentLevel, gint indentSpace){
    GString *strBuf=g_string_new(NULL);
    append_indent_space(strBuf,indentLevel,indentSpace);

    if(type!=XML_TAG_TYPE_NO_TAG){
	g_string_append_printf(strBuf,"<%s%s%s%s%s>",
		(type==XML_TAG_TYPE_END_ONLY) ? "/": "",
		(!maker_dialog_string_is_empty(tagName))? tagName : "",
		(!maker_dialog_string_is_empty(attribute)) ? " ":"",  (!maker_dialog_string_is_empty(attribute))? attribute : "",
		(type==XML_TAG_TYPE_EMPTY) ? "/": ""
	);
    }
    if (type==XML_TAG_TYPE_EMPTY)
	return strBuf;
    if (type==XML_TAG_TYPE_BEGIN_ONLY)
	return strBuf;
    if (type==XML_TAG_TYPE_END_ONLY)
	return strBuf;

    if (type==XML_TAG_TYPE_LONG){
	g_string_append_c(strBuf,'\n');
    }

    if (value){
	if (type==XML_TAG_TYPE_LONG || type==XML_TAG_TYPE_NO_TAG){
	    append_indent_space(strBuf,indentLevel+1, indentSpace);
	    int i, valueLen=strlen(value);
	    for(i=0;i<valueLen;i++){
		g_string_append_c(strBuf,value[i]);
		if (value[i]=='\n'){
		    append_indent_space(strBuf,indentLevel+1, indentSpace);
		}
	    }
	    g_string_append_c(strBuf,'\n');
	    if (type==XML_TAG_TYPE_LONG){
		append_indent_space(strBuf,indentLevel, indentSpace);
	    }
	}else{
	    g_string_append(strBuf,value);
	}
    }

    if (type==XML_TAG_TYPE_LONG || type==XML_TAG_TYPE_SHORT){
	g_string_append_printf(strBuf,"</%s>",tagName);
    }
    return strBuf;
}

static void xml_tags_write(SchemasFileData *sData, const gchar *tagName, XmlTagsType type,
	const gchar *attribute, const gchar *value){
    static int indentLevel=0;
    if (type==XML_TAG_TYPE_END_ONLY)
	indentLevel--;

    GString *strBuf=xml_tags_to_string(tagName, type, attribute, value, indentLevel, sData->indentSpace);
    MAKER_DIALOG_DEBUG_MSG(5,"[I5] config_gconf_xml_tags_write:%s",strBuf->str);
    fprintf(sData->outF,"%s\n",strBuf->str);

    if (type==XML_TAG_TYPE_BEGIN_ONLY)
	indentLevel++;
    g_string_free(strBuf,TRUE);
}

static void ctx_write_locale(MakerDialogPropertyContext *ctx, SchemasFileData *sData, const gchar *localeStr){
    gchar buf[50];
    g_snprintf(buf,50,"name=\"%s\"",localeStr);
    setlocale(LC_MESSAGES,localeStr);
    xml_tags_write(sData,"locale",XML_TAG_TYPE_BEGIN_ONLY,buf,NULL);
    xml_tags_write(sData,"short",XML_TAG_TYPE_SHORT,NULL, _(ctx->spec->label));
    xml_tags_write(sData,"long",XML_TAG_TYPE_LONG,NULL, _(ctx->spec->tooltip));
    xml_tags_write(sData,"locale",XML_TAG_TYPE_END_ONLY,NULL,NULL);
}

#define STRING_BUFFER_SIZE_DEFAULT 2000
static void xml_each_page_each_property_func(MakerDialog *mDialog, MakerDialogPropertyContext *ctx, gpointer userData){
    SchemasFileData *sData=(SchemasFileData *) userData;
    xml_tags_write(sData,"schema",XML_TAG_TYPE_BEGIN_ONLY,NULL,NULL);
    gchar buf[STRING_BUFFER_SIZE_DEFAULT];
    g_snprintf(buf,STRING_BUFFER_SIZE_DEFAULT,"/schemas%s/%s",sData->schemasHome,ctx->spec->key);
    xml_tags_write(sData,"key",XML_TAG_TYPE_SHORT,NULL,buf);
    xml_tags_write(sData,"applyto",XML_TAG_TYPE_SHORT,NULL,buf+strlen("/schemas"));
    xml_tags_write(sData,"owner",XML_TAG_TYPE_SHORT,NULL,sData->owner);
    switch(ctx->spec->valueType){
	case MKDG_TYPE_BOOLEAN:
	    xml_tags_write(sData->outF,"type",XML_TAG_TYPE_SHORT,NULL,"bool");
	    break;
	case MKDG_TYPE_INT:
	case MKDG_TYPE_UINT:
	case MKDG_TYPE_LONG:
	case MKDG_TYPE_ULONG:
	    xml_tags_write(sData->outF,"type",XML_TAG_TYPE_SHORT,NULL,"int");
	    break;
	case MKDG_TYPE_FLOAT:
	case MKDG_TYPE_DOUBLE:
	    xml_tags_write(sData->outF,"type",XML_TAG_TYPE_SHORT,NULL,"float");
	    break;
	case MKDG_TYPE_STRING:
	case MKDG_TYPE_COLOR:
	    xml_tags_write(sData->outF,"type",XML_TAG_TYPE_SHORT,NULL,"string");
	    break;
	default:
	    break;
    }
    if (ctx->spec->defaultValue){
	xml_tags_write(sData->outF,"default",XML_TAG_TYPE_SHORT,NULL,ctx->spec->defaultValue);
    }
    gboolean hasCLocale=FALSE;
    if (sData->localeArray){
	int i;
	for(i=0;sData->localeArray[i]!=NULL;i++){
	    if (strcmp(sData->localeArray[i],"C")==0){
		hasCLocale=TRUE;
	    }
	    ctx_write_locale(ctx,sData,sData->localeArray[i]);
	}
    }
    if (!hasCLocale)
	ctx_write_locale(ctx,sData,"C");
    setlocale(LC_ALL,NULL);
    xml_tags_write(sData,"schema",XML_TAG_TYPE_END_ONLY,NULL,NULL);
}

gboolean maker_dialog_config_gconf_write_schemas_file
(MakerDialogConfig *config, const gchar *filename, gint indentSpace, const gchar *owner, const gchar *locales, GError **error){
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] config_gconf_write_schemas_file( , %s, %s, %s, %s, )",
	    filename, owner, schemasHome, (locales)? locales: "-");

    FILE *outF=fopen(filename,"w");
    GError *cfgErr=NULL;
    if (outF==NULL){
	cfgErr=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_CANT_WRITE, "config_gconf_write_schemas_file");
	maker_dialog_config_error_handling(cfgErr, error);
	return FALSE;
    }
    SchemasFileData sData;
    MakerDialogConfigSet *configSet=g_ptr_array_index(config->setArray,0);
    MakerDialogConfigFile *configFile=g_ptr_array_index(configSet->fileArray,0);
    sData.schemasHome=configFile->path;
    sData.owner=owner;
    if (locales){
	sData.localeArray=g_strsplit_set(locales);
    }else{
	sData.localeArray=NULL;
    }
    sData.outF=outF;
    xml_tags_write(&sData,"gconfschemafile",XML_TAG_TYPE_BEGIN_ONLY,NULL,NULL);
    xml_tags_write(&sData,"schemalist",XML_TAG_TYPE_BEGIN_ONLY,NULL,NULL);
    maker_dialog_foreach_page_foreach_property(configSet->mDialog, NULL, NULL, xml_each_page_each_property_func, &sData);
    g_ptr_array_foreach(self->_priv->propList,ctx_write_callback,&sData);
    xml_tags_write(&sData,"schemalist",XML_TAG_TYPE_END_ONLY,NULL,NULL);
    xml_tags_write(&sData,"gconfschemafile",XML_TAG_TYPE_END_ONLY,NULL,NULL);
    if (fclose(outF)){
	cfgErr=maker_dialog_config_error_new(MAKER_DIALOG_CONFIG_ERROR_CANT_WRITE, "config_gconf_write_schemas_file fclose()");
	maker_dialog_config_error_handling(cfgErr, error);
	return FALSE;
    }
    return TRUE;
}


