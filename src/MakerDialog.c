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
    mDialog->toolkitHandler=NULL;
    mDialog->buttonSpecCount=buttonSpecCount;
    mDialog->buttonSpecs=buttonSpecs;
    mDialog->propertyTable=maker_dialog_property_table_new();
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
    mDialog->destroyHookList=g_hook_list_init(mDialog->destroyHookLit,sizeof(GHook));
    return mDialog;
}

void maker_dialog_add_property(MakerDialog *mDialog, MakerDialogPropertyContext *ctx){
    maker_dialog_property_table_insert(mDialog->propertyTable, ctx);
    gboolean bValue=FALSE;
    guint uintValue=0;
    gint intValue=0;
    gdouble doubleValue=0.0;

    switch(ctx->spec->valueType){
	case G_TYPE_BOOLEAN:
	    if (!ctx->hasValue && ctx->spec->defaultValue){
		bValue=maker_dialog_atob(ctx->spec->defaultValue);
	    }
	    g_value_set_boolean(&ctx->value,bValue);
	    break;
	case G_TYPE_UINT:
	    if (!ctx->hasValue && ctx->spec->defaultValue){
		uintValue=atoi(ctx->spec->defaultValue);
	    }
	    g_value_set_uint(&ctx->value,uintValue);
	    break;
	case G_TYPE_INT:
	    if (!ctx->hasValue && ctx->spec->defaultValue){
		intValue=atoi(ctx->spec->defaultValue);
	    }
	    g_value_set_int(&ctx->value,intValue);
	    break;
	case G_TYPE_DOUBLE:
	    if (!ctx->hasValue && ctx->spec->defaultValue){
		doubleValue=atof(ctx->spec->defaultValue);
	    }
	    g_value_set_int(&ctx->value,doubleValue);
	    break;
	case G_TYPE_STRING:
	    if (ctx->spec->validValues){
		gint index=-1;
		/* Make sure init value and default value is in valid values */
		if (ctx->hasValue){
		    index=maker_dialog_find_string(g_value_get_string(&ctx->value),ctx->spec->validValues,-1);
		}
		if (index<0 && ctx->spec->defaultValue){
		    index=maker_dialog_find_string(ctx->spec->defaultValue,ctx->spec->validValues,-1);
		}

		if (index<0 && (ctx->spec->propertyFlags & MAKER_DIALOG_PROPERTY_FLAG_INEDITABLE)){
		    index=0;
		}

		if (index<0){
		    /* Allow to edit, so we can use init and default value
		     * anyway
		     */
		    if (!ctx->hasValue && ctx->spec->defaultValue){
			g_value_set_string(&ctx->value,ctx->spec->defaultValue);
		    }
		}else{
		    g_value_set_string(&ctx->value,ctx->spec->validValues[index]);
		}
	    }else{
		if (!ctx->hasValue && ctx->spec->defaultValue){
		    g_value_set_string(&ctx->value,ctx->spec->defaultValue);
		}
	    }
	    break;
	default:
	    break;
    }
}

void maker_dialog_destroy(MakerDialog *mDialog){
//    maker_dialog_ui_destroy(mDialog);
    maker_dialog_property_table_destroy(mDialog->propertyTable);
    g_free(mDialog->title);
    g_free(mDialog);
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

gint maker_dialog_find_string(const gchar *str, const gchar **strlist, gint max_find){
    gint index=-1,i;
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


