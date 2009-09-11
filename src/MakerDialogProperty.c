#include <stdlib.h>
#include <strings.h>
#include <glib/gi18n.h>
#include <locale.h>
#include "MakerDialogProperty.h"

#ifndef VERBOSE_LEVEL
#define VERBOSE_LEVEL 2
#endif

static gint verboseLevel=VERBOSE_LEVEL;

void MAKER_DIALOG_DEBUG_MSG(gint level, const char *format, ...){
    va_list ap;
    if (level<verboseLevel){
	va_start(ap, format);
	g_logv("MakerDialog",G_LOG_LEVEL_DEBUG, format, ap);
	va_end(ap);
    }
}

MakerDialogPropertySpec *maker_dialog_property_spec_new(const gchar *key, GType valueType){
    MakerDialogPropertySpec *spec=g_new(MakerDialogPropertySpec,1);
    if (spec){
	spec->key=key;
	spec->valueType=valueType;
	spec->defaultValue=NULL;
	spec->validValues=NULL;
	spec->min=0.0;
	spec->max=0.0;

	spec->propertyFlags=0;
	spec->pageName=NULL;
	spec->label=NULL;
	spec->translationContext=NULL;
	spec->tooltip=NULL;

	spec->checkFunc=NULL;
	spec->setFunc=NULL;

	spec->extraData=NULL;
    }
    return spec;
}

void maker_dialog_property_spec_free(MakerDialogPropertySpec *spec, gboolean freeDeep){
    if (freeDeep){
	if (spec->defaultValue){
	    g_free(spec->defaultValue);
	}
	if (spec->validValues){
	    g_free(spec->validValues);
	}
	if (spec->pageName){
	    g_free(spec->pageName);
	}
	if (spec->label){
	    g_free(spec->label);
	}
	if (spec->translationContext){
	    g_free(spec->translationContext);
	}
	if (spec->extraData){
	    g_free(spec->extraData);
	}
    }
    g_free(spec);
}

MakerDialogPropertyContext *maker_dialog_property_context_new(MakerDialogPropertySpec *spec, gpointer userData){
    MakerDialogPropertyContext *ctx=g_new(MakerDialogPropertyContext,1);
    if (ctx){
	ctx->spec=spec;
	ctx->userData=userData;
	memset(&ctx->value, 0, sizeof(GValue));
	g_value_init(&ctx->value,spec->valueType);
    }
    return ctx;
}


void maker_dialog_property_context_free(MakerDialogPropertyContext *ctx, gboolean freeDeep){
    g_value_unset (&(ctx->value));
    if (freeDeep){
	if (ctx->userData){
	    g_free(ctx->userData);
	}
	maker_dialog_property_spec_free(ctx->spec,freeDeep);
    }
    g_free(ctx);
}

MakerDialogPropertyTable* maker_dialog_property_table_new(){
    return g_hash_table_new(g_str_hash,g_str_equal);
}

void maker_dialog_property_table_insert(MakerDialogPropertyTable *hTable, const MakerDialogPropertyContext *ctx){
    g_hash_table_insert(hTable, (gpointer) ctx->spec->key, (gpointer) ctx);
}

MakerDialogPropertyContext *maker_dialog_property_table_lookup(MakerDialogPropertyTable *hTable, const gchar *key){
    return (MakerDialogPropertyContext *) g_hash_table_lookup((gpointer) hTable,(gpointer) key);
}

GValue *maker_dialog_property_table_lookup_value(MakerDialogPropertyTable *hTable, const gchar *key){
    MakerDialogPropertyContext *ctx=maker_dialog_property_table_lookup(hTable, key);
    return  &ctx->value;
}

