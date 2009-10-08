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
#include <unistd.h>

#ifndef VERBOSE_LEVEL
#define VERBOSE_LEVEL 2
#endif
static gint verboseLevel=VERBOSE_LEVEL;

void MAKER_DIALOG_DEBUG_MSG(gint level, const gchar *format, ...){
    va_list ap;
    if (level<verboseLevel){
	va_start(ap, format);
	g_logv("MakerDialog",G_LOG_LEVEL_DEBUG, format, ap);
	va_end(ap);
    }
}

const MakerDialogToolkitHandler makerDialogHandlerNone={
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

MakerDialog *maker_dialog_init(const gchar *title, gboolean freeSpec,
	guint buttonSpecCount, MakerDialogButtonSpec *buttonSpecs){
    MakerDialog *dlg=g_new(MakerDialog,1);
    dlg->title=g_strdup(title);
    dlg->handler=&makerDialogHandlerNone;
    dlg->freeSpec=freeSpec;
    dlg->buttonSpecCount=buttonSpecCount;
    dlg->buttonSpecs=buttonSpecs;
    dlg->propertyTable=maker_dialog_property_table_new(freeSpec);
    dlg->maxSizeInPixel.width=-1;
    dlg->maxSizeInPixel.height=-1;
    dlg->maxSizeInChar.width=-1;
    dlg->maxSizeInChar.height=-1;
    dlg->labelAlignment.x=0.0f;
    dlg->labelAlignment.y=0.0f;
    dlg->componentAlignment.x=0.0f;
    dlg->componentAlignment.y=0.0f;
    return dlg;
}

void maker_dialog_add_property(MakerDialog *dlg, MakerDialogPropertyContext *ctx, const gchar *initValue){
    maker_dialog_property_table_insert(dlg->propertyTable, ctx);
    g_value_init(&ctx->value,ctx->spec->valueType);

    switch(ctx->spec->valueType){
	case G_TYPE_BOOLEAN:
	    gboolean bValue=FALSE;
	    if(initValue){
		bValue=atob(initValue);
	    }else if (ctx->spec->defaultValue){
		bValue=atob(ctx->spec->defaultValue);
	    }
	    g_value_set_boolean(&ctx->value,bValue);
	    break;
	case G_TYPE_UINT:
	    guint uintValue=0;
	    if(initValue){
		uintValue=atoi(initValue);
	    }else if (ctx->spec->defaultValue){
		uintValue=atoi(ctx->spec->defaultValue);
	    }
	    g_value_set_uint(&ctx->value,uintValue);
	    break;
	case G_TYPE_INT:
	    gint intValue=0;
	    if(initValue){
		intValue=atoi(initValue);
	    }else if (ctx->spec->defaultValue){
		intValue=atoi(ctx->spec->defaultValue);
	    }
	    g_value_set_int(&ctx->value,intValue);
	    break;
	case G_TYPE_DOUBLE:
	    gdouble doubleValue=0;
	    if(initValue){
		doubleValue=atof(initValue);
	    }else if (ctx->spec->defaultValue){
		doubleValue=atof(ctx->spec->defaultValue);
	    }
	    g_value_set_int(&ctx->value,doubleValue);
	    break;
	case G_TYPE_STRING:
	    if (ctx->spec->validValues){
		gint index=-1;
		/* Make sure init value and default value is in valid values */
		if (initValue){
		    index=maker_dialog_find_string(initValue,ctx->spec->validValues,-1);
		}
		if (index<0 && ctx->spec->defaultValue){
		    index=maker_dialog_find_string(ctx->spec->defaultValue,ctx->spec->validValues,-1);
		}

		if (index<0 && (ctx->spec->propertyFlags & MAKER_DIALOG_PROPERTY_FLAG_INEDITABLE)){
		    index=0
		}

		if (index<0){
		    /* Allow to edit, so we can use init and default value
		     * anyway
		     */
		    if (initValue){
			g_value_set_string(&ctx->value,initValue);
		    }else (ctx->spec->defaultValue){
			g_value_set_string(&ctx->value,ctx->spec->defaultValue);
		    }
		}else{
		    g_value_set_string(&ctx->value,ctx->spec->validValues[index]);
		}
	    }else{
		if (initValue){
		    g_value_set_string(&ctx->value,initValue);
		}else (ctx->spec->defaultValue){
		    g_value_set_string(&ctx->value,ctx->spec->defaultValue);
		}
	    }
	    break;
	default:
	    break;
    }
}

void maker_dialog_set_toolkit_handler(MakerDialog *dlg, MakerDialogToolkitHandler *handler){
    dlg->handler=handler;
}

void maker_dialog_construct(MakerDialog *dlg){
    g_assert(dlg->handler->dialog_construct);
    dlg->handler->dialog_obj=dlg->handler->dialog_construct(dlg);
}

gint maker_dialog_run(MakerDialog *dlg){
    g_assert(dlg->handler->dialog_run);
    return dlg->handler->dialog_run(dlg);
}

void maker_dialog_show(MakerDialog *dlg){
    g_assert(dlg->handler->dialog_show);
    dlg->handler->dialog_show(dlg);
}

void maker_dialog_hide(MakerDialog *dlg){
    g_assert(dlg->handler->dialog_hide);
    dlg->handler->dialog_hide(dlg);
}

void maker_dialog_destroy(MakerDialog *dlg){
    if (dlg->handler->dialog_obj){
	g_assert(dlg->handler->dialog_destroy);
	dlg->handler->dialog_destroy(dlg);
	if (dlg->handler->dialog_obj)
	    g_free(dialog_obj);
    }
    maker_dialog_property_table_destroy(dlg->propertyTable);
    g_free(dlg->title);
    g_free(dlg);
}

GValue *maker_dialog_get_value(MakerDialog *dlg, const gchar *key){
    return maker_dialog_property_table_lookup_value(dlg->propertyTable, key);
}

static void _propertyContext_call_set_value(MakerDialogPropertyContext *ctx, GValue *value){
    g_value_copy(value,ctx->valule);
    if (ctx->setFunc){
	ctx->setFunc(ctx,ctx->value);
    }
}

static gboolean  _propertyContext_apply_value(MakerDialog *dlg, const gchar *key, gboolean needValidate){
    MakerDialogPropertyContext *ctx=maker_dialog_property_table_lookup(dlg->propertyTable, key);
    if (needValidate && (!ctx->validateFunc)){
	/* No validation function */
	return FALSE;
    }

    g_assert(dlg->handler->component_get_value);
    GValue *value=dlg->handler->component_get_value(dlg,key);
    gboolean ret=TRUE;
    if (needValidate && (!ctx->validateFunc(ctx->spec, value))){
	/* Value is invalid. */
	ret=FALSE;
    }else{
	_propertyContext_call_set_value(ctx, value);
    }
    g_value_unset(value);
    g_free(value);

    return ret;
    g_value_copy(value,ctx->valule);

}

void maker_dialog_apply_value(MakerDialog *dlg, const gchar *key){
    _propertyContext_apply_value(dlg, key, FALSE);
}

gboolean maker_dialog_validate_and_apply_value(MakerDialog *dlg, const gchar *key){
    return _propertyContext_apply_value(dlg, key, TRUE);
}

gboolean maker_dialog_validate_and_apply_value(MakerDialog *dlg, const gchar *key){
    MakerDialogPropertyContext *ctx=maker_dialog_property_table_lookup(dlg->propertyTable, key);
    if (!ctx->validateFunc){
	/* No validation function */
	return FALSE;
    }
    g_assert(dlg->handler->component_get_value);
    GValue *value=dlg->handler->component_get_value(dlg,key);
    gboolean ret=FALSE;
    if (ctx->validateFunc(ctx->spec, value)){
	_propertyContext_apply_value(ctx, value);
	ret=TRUE;
    }else{
	/* Value is invalid. */
    }
    g_value_unset(value);
    g_free(value);

    return ret;
}

static _propertyContext_set_value(MakerDialogPropertyContext *ctx, GValue *value){
    g_assert(dlg->handler->component_set_value);
    dlg->handler->component_call_set_value(dlg, key, value);
    _propertyContext_apply_value(ctx, value);
}

void maker_dialog_set_value(MakerDialog *dlg, const gchar *key, GValue *value){
    MakerDialogPropertyContext *ctx=maker_dialog_property_table_lookup(dlg->propertyTable, key);
    _propertyContext_call_value(ctx, value);
}

gboolean maker_dialog_validate_and_set_value(MakerDialog *dlg, const gchar *key, GValue *value){
    MakerDialogPropertyContext *ctx=maker_dialog_property_table_lookup(dlg->propertyTable, key);
    if (!ctx->validateFunc){
	/* No validation function */
	return FALSE;
    }
    if (!ctx->validateFunc(ctx->spec, value)){
	/* Value is invalid. */
	return FALSE;
    }
    _propertyContext_set_value(ctx, value);
    return TRUE;
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


