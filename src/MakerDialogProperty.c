#include <stdlib.h>
#include <strings.h>
#include <glib/gi18n.h>
#include <locale.h>
#include "MakerDialog.h"


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

void maker_dialog_property_spec_free(MakerDialogPropertySpec *spec){
    g_free(spec);
}

MakerDialogPropertyContext *maker_dialog_property_context_new(MakerDialogPropertySpec *spec, gpointer obj){
    MakerDialogPropertyContext *ctx=g_new(MakerDialogPropertyContext,1);
    if (ctx){
	ctx->spec=spec;
	ctx->obj=obj;
	memset(&ctx->value, 0, sizeof(GValue));
	g_value_init(&ctx->value,spec->valueType);
	ctx->validateFunc=NULL;
	ctx->setFunc=NULL;
    }
    return ctx;
}

void maker_dialog_property_context_free(MakerDialogPropertyContext *ctx, gboolean freeSpec){
    g_value_unset (&(ctx->value));
    if (freeSpec){
	maker_dialog_property_spec_free(ctx->spec);
    }
    g_free(ctx);
}

void maker_dialog_property_context_free_without_spec(MakerDialogPropertyContext *ctx){
    maker_dialog_property_context_free(ctx, FALSE);
}

void maker_dialog_property_context_free_with_spec(MakerDialogPropertyContext *ctx){
    maker_dialog_property_context_free(ctx, TRUE);
}


MakerDialogPropertyTable* maker_dialog_property_table_new(gboolean freeSpec){
    return g_hash_table_new_full(g_str_hash,g_str_equal,NULL,
	    (freeSpec)? maker_dialog_property_context_free_with_spec: maker_dialog_property_context_free_without_spec);
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

void maker_dialog_property_table_destroy (MakerDialogPropertyTable *hTable){
    g_hash_table_destroy(hTable);
}
