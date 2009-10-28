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
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include "MakerDialog.h"
#define MAKER_DLALOG_VERBOSE_ENV "MAKER_DIALOG_VERBOSE"
static gint verboseLevel=0;

void MAKER_DIALOG_DEBUG_MSG(gint level, const gchar *format, ...){
    va_list ap;
    if (level<verboseLevel){
	va_start(ap, format);
	g_logv("MakerDialog",G_LOG_LEVEL_DEBUG, format, ap);
	va_end(ap);
    }
}

MakerDialog *maker_dialog_init(const gchar *title,
	guint buttonSpecCount, MakerDialogButtonSpec *buttonSpecs){
    MakerDialog *mDialog=g_new(MakerDialog,1);
    mDialog->title=g_strdup(title);
    mDialog->buttonSpecCount=buttonSpecCount;
    mDialog->buttonSpecs=buttonSpecs;
    mDialog->propertyTable=maker_dialog_property_table_new();
    mDialog->pageRoot=g_node_new(NULL);
    mDialog->maxSizeInPixel.width=-1;
    mDialog->maxSizeInPixel.height=-1;
    mDialog->maxSizeInChar.width=-1;
    mDialog->maxSizeInChar.height=-1;
    mDialog->labelAlignment.x=0.0f;
    mDialog->labelAlignment.y=0.5f;
    mDialog->componentAlignment.x=0.0f;
    mDialog->componentAlignment.y=0.5f;
    if (getenv(MAKER_DLALOG_VERBOSE_ENV)){
	verboseLevel=atoi(getenv(MAKER_DLALOG_VERBOSE_ENV));
    }
    mDialog->dlgUi=NULL;
    mDialog->dlgCfg=NULL;
    return mDialog;
}

static GNode *maker_dialog_prepare_page_node(MakerDialog *mDialog, const gchar *pageName){
    const gchar *pageName_tmp=(pageName)? pageName : MAKER_DIALOG_CONFIG_NO_PAGE;
    GNode *result=maker_dialog_find_page_node(mDialog, (gpointer) pageName_tmp);
    if (result){
	result=g_node_new((gpointer) pageName_tmp);
	g_node_append(mDialog->pageRoot,result);
    }
    return result;
}

void maker_dialog_add_property(MakerDialog *mDialog, MakerDialogPropertyContext *ctx){
    maker_dialog_property_table_insert(mDialog->propertyTable, ctx);
    GNode *propPageNode=maker_dialog_prepare_page_node(mDialog, ctx->spec->pageName);
    GNode *propKeyNode=g_node_new((gpointer) ctx->spec->key);
    g_node_append(propPageNode,propKeyNode);
    const gchar *initString=maker_dialog_property_get_default_string(ctx->spec);
    if (initString){
	maker_dialog_string_set_g_value(ctx->valueType, initString, ctx->value, TRUE);
    }
    ctx->mDialog=mDialog;
}

void maker_dialog_destroy(MakerDialog *mDialog){
    if (mDialog->dlgUi){
	maker_dialog_ui_destroy(mDialog);
    }
    if (mDialog->dlgCfg){
        maker_dialog_config_free(mDialog->dlgCfg);
    }

    maker_dialog_property_table_destroy(mDialog->propertyTable);
    g_node_destroy(mDialog->pageRoot);
    g_free(mDialog->title);
    g_free(mDialog);
}

GValue *maker_dialog_get_value(MakerDialog *mDialog, const gchar *key){
    return maker_dialog_property_table_lookup_value(mDialog->propertyTable, key);
}

MakerDialogPropertyContext *maker_dialog_get_property_context(MakerDialog *mDialog, const gchar *key){
    return maker_dialog_property_table_lookup(mDialog->propertyTable, key);
}

gboolean maker_dialog_apply_value(MakerDialog *mDialog, const gchar *key){
    MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(mDialog, key);

    gboolean ret=TRUE;
    if (ctx->validateFunc && (!ctx->validateFunc(ctx->spec, &ctx->value))){
	/* Value is invalid. */
	ret=FALSE;
    }
    if (ret && ctx->applyFunc){
	ctx->applyFunc(ctx,&ctx->value);
    }else{
	ret=FALSE;
    }

    return ret;
}

gboolean maker_dialog_set_value(MakerDialog *mDialog, const gchar *key, GValue *value){
    MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(mDialog, key);

    gboolean ret=TRUE;
    if (ctx->validateFunc && (!ctx->validateFunc(ctx->spec, &ctx->value))){
	/* Value is invalid. */
	ret=FALSE;
    }
    if (ret && mDialog->dlgUi->toolkitHandler->widget_set_value){
	mDialog->dlgUi->toolkitHandler->widget_set_value(mDialog->dlgUi, ctx->spec->key, value);
	g_value_copy(value,&ctx->value);
    }else{
	ret=FALSE;
    }
    return ret;
}

GNode *maker_dialog_find_page_node(MakerDialog *mDialog, const gchar *pageName){
    const gchar *pageName_tmp=(pageName)? pageName : MAKER_DIALOG_CONFIG_NO_PAGE;
    return g_node_find(mDialog->pageRoot, G_POST_ORDER, G_TRAVERSE_NON_LEAVES, (gpointer) pageName_tmp);
}

gboolean maker_dialog_atob(const gchar *string){
    if (!string)
	return FALSE;
    if (strlen(string)<=0)
	return FALSE;
    if (string[0]=='F' || string[0]=='f' || string[0]=='N' ||  string[0]=='n')
	return FALSE;
    char *endPtr=NULL;
    long int longValue=strtol(string, &endPtr, 10);

    if (longValue==0 && *endPtr=='\0'){
	// 0
	return FALSE;
    }
    return TRUE;
}

GValue *maker_dialog_string_set_g_value(GValue *value, GType valueType, const gchar *str, gboolean needInit){
    if (!str)
	return NULL;
    gboolean bValue=FALSE;
    guint uintValue=0;
    gint intValue=0;
    gdouble doubleValue=0.0;
    if (needInit){
	g_value_init(value, valueType);
    }
    switch(valueType){
	case G_TYPE_BOOLEAN:
	    bValue=maker_dialog_atob(str);
	    g_value_set_boolean(value,bValue);
	    break;
	case G_TYPE_UINT:
	    uintValue=atoi(str);
	    g_value_set_uint(value,uintValue);
	    break;
	case G_TYPE_INT:
	    intValue=atoi(str);
	    g_value_set_int(value,intValue);
	    break;
	case G_TYPE_DOUBLE:
	    doubleValue=atof(str);
	    g_value_set_int(value,doubleValue);
	    break;
	case G_TYPE_STRING:
	    g_value_set_string(value,str);
	    break;
	default:
	    break;
    }
    return value;
}

gint maker_dialog_value_compare(GValue *value1, GValue *value2){
    if (G_VALUE_TYPE(value1)!=G_VALUE_TYPE(value2))
	return -2;
    gboolean bValue1,bValue2;
    gint intValue1,intValue2;
    guint uintValue1,uintValue2;
    gdouble dValue1,dValue2;
    gchar *strValue1, *strValue2;
    switch(G_VALUE_TYPE(value1)){
	case G_TYPE_BOOLEAN:
	    bValue1=g_value_get_boolean (value1);
	    bValue2=g_value_get_boolean (value2);
	    if (bValue1==bValue2){
		return 0;
	    }
	    if (bValue1==TRUE){
		return -1;
	    }
	    return 1;
	case G_TYPE_INT:
	    intValue1=g_value_get_int (value1);
	    intValue2=g_value_get_int (value2);
	    if (intValue1==intValue2){
		return 0;
	    }
	    if (intValue1>intValue2){
		return -1;
	    }
	    return 1;
	case G_TYPE_UINT:
	    uintValue1=g_value_get_uint (value1);
	    uintValue2=g_value_get_uint (value2);
	    if (uintValue1==uintValue2){
		return 0;
	    }
	    if (uintValue1>uintValue2){
		return -1;
	    }
	    return 1;
	case G_TYPE_DOUBLE:
	    dValue1=g_value_get_double (value1);
	    dValue2=g_value_get_double (value2);
	    if (uintValue1==uintValue2){
		return 0;
	    }
	    if (uintValue1>uintValue2){
		return -1;
	    }
	    return 1;
	case G_TYPE_STRING:
	    strValue1=g_value_get_string (value1);
	    strValue2=g_value_get_string (value2);
	    return strcmp(strValue1,strValue2);
	default:
	    break;
    }
    /* Not supported */
    return -3;
}

gint maker_dialog_find_string(const gchar *str, const gchar **strlist, gint max_find){
    gint index=-1,i;
    if (!str){
	return -2;
    }
    if (!strlist){
	return -3;
    }
    for(i=0; strlist[i]!=NULL; i++){
	if (max_find>=0 && i>=max_find){
	    break;
	}
	if (strcmp(str,strlist[i])==0){
	    index=i;
	    break;
	}
    }
    return index;
}


