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
#include <stdlib.h>
#include <string.h>
#include <glib/gprintf.h>
#include "MakerDialogTypes.h"
#include "MakerDialogUtil.h"

GType mkdgTypeList[]={
    G_TYPE_BOOLEAN,
    G_TYPE_INT,
    G_TYPE_UINT,
    G_TYPE_LONG,
    G_TYPE_ULONG,
    G_TYPE_FLOAT,
    G_TYPE_DOUBLE,
    G_TYPE_STRING,
    G_TYPE_UINT, 	/* MKDG_TYPE_COLOR has not initialized*/
    G_TYPE_NONE,
};

GType mkdg_color_get_type (void)
{
    static GType type = 0;

    if (G_UNLIKELY(type == 0)) {
	static const GTypeInfo info = {
	    0,
	    (GBaseInitFunc) NULL,
	    (GBaseFinalizeFunc) NULL,
	    (GClassInitFunc) NULL,
	    (GClassFinalizeFunc) NULL,
	    NULL /* class_data */,
	    sizeof (MkdgColor),
	    0 /* n_preallocs */,
	    NULL,
	    NULL
	};

	type = g_type_register_static (G_TYPE_UINT, "MkdgColor", &info, (GTypeFlags)0);
	mkdgTypeList[MKDG_TYPE_COLOR]=type;
    }
    return type;
}

typedef struct _{
    const gchar *name;
    guint32	value;
} MkdgColorInfo;

MkdgColorInfo mkdgColorList[]={
    {"aqua",		0x00FFFF},
    {"black",		0x000000},
    {"blue",		0x0000FF},
    {"fuchsia",		0xFF00FF},
    {"grey",		0x808080},
    {"green",		0x008000},
    {"lime",		0x00FF00},
    {"maroon",		0x800000},
    {"navy",		0x000080},
    {"olive",		0x808000},
    {"purple",		0x800080},
    {"red",		0xFF0000},
    {"sliver",		0xC0C0C0},
    {"teal",		0x008080},
    {"white",		0xFFFFFF},
    {"yellow",		0xFFFF00},
    {NULL,		0}
};

static gint find_color_index_by_name(const gchar *name){
    gint i;
    for(i=0;mkdgColorList[i].name!=NULL;i++){
	if (g_ascii_strncasecmp(mkdgColorList[i].name,name,30)==0){
	    return i;
	}
    }
    return -1;
}

static gint find_color_index_by_value(guint32 value){
    gint i;
    for(i=0;mkdgColorList[i].name!=NULL;i++){
	if (mkdgColorList[i].value==value){
	    return i;
	}
    }
    return -1;
}

GType maker_dialog_type_to_g_type(MkdgType mType){
    if (mkdgTypeList[mType] != G_TYPE_INVALID){
	return mkdgTypeList[mType];
    }
    switch (mType){
	case MKDG_TYPE_COLOR:
	    return MKDG_G_TYPE_COLOR;
	default:
	    break;
    }
    return G_TYPE_INVALID;
}

MkdgType maker_dialog_type_from_g_type(GType gType){
    if (gType==G_TYPE_INVALID)
	return MKDG_TYPE_INVALID;
    MkdgType mType;
    for(mType=0; mkdgTypeList[mType]!=G_TYPE_NONE;mType++){
	if (mkdgTypeList[mType]==gType)
	    return mType;
	if (mType==MKDG_G_TYPE_COLOR)
	    return MKDG_TYPE_COLOR;
    }
    return MKDG_TYPE_NONE;
}

gboolean maker_dialog_type_is_number(MkdgType mType){
    switch (mType){
	case MKDG_TYPE_INT:
	case MKDG_TYPE_UINT:
	case MKDG_TYPE_LONG:
	case MKDG_TYPE_ULONG:
	case MKDG_TYPE_FLOAT:
	case MKDG_TYPE_DOUBLE:
	    return TRUE;
	default:
	    break;
    }
    return FALSE;
}

/*=== Start Type Interface functions ===*/
typedef struct{
    MkdgType type;
    MkdgTypeInterface typeInterface;
} MkdgTypeInterfaceMkdgType;

static gint md_color_compare(GValue *value1, GValue *value2, MakerDialogCompareFunc func){
    if (G_VALUE_TYPE(value2)!=MKDG_TYPE_COLOR){
	return -3;
    }
    guint32 v1=g_value_get_uint(value1);
    guint32 v2=g_value_get_uint(value2);
    if (func){
	return func(&v1, &v2);
    }
    return (v1==v2) ? 0 : (v1>v2)? 1: -1;
}

static GValue *md_color_from_string(GValue *value, const gchar *str, const gchar *parseOption){
    if (G_UNLIKELY(maker_dialog_string_is_empty(str))){
	/* Default is black */
	g_value_set_uint(value, 0);
    }else{
	if (str[0]=='#'){
	    g_value_set_uint(value, (guint) strtol(str+sizeof(gchar), NULL, 16));
	}else{
	    gint index=find_color_index_by_name(str);
	    if (index>=0){
		g_value_set_uint(value, mkdgColorList[index].value);
	    }else{
		/* Default is black */
		g_value_set_uint(value, 0);
	    }
	}
    }
    return value;
}

static gchar *md_color_to_string(GValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    gchar buf[12];
    if (!toStringFormat)
	toStringFormat="%s";
    guint color=g_value_get_uint(value);
    gint index=find_color_index_by_value(color);
    MAKER_DIALOG_DEBUG_MSG(5,"[I5] md_color_to_string(): color=%X index=%d",color,index);
    if (index>=0){
	g_string_printf(strBuf, toStringFormat, mkdgColorList[index].name);
    }else{
	g_snprintf(buf,12,"#%X", color);
	g_string_printf(strBuf, toStringFormat, buf);
    }
    return g_string_free(strBuf, FALSE);
}

const MkdgTypeInterfaceMkdgType mkdgTypeInterfaces[]={
    { MKDG_TYPE_COLOR,		{md_color_from_string,	md_color_to_string, 	md_color_compare}},
    { MKDG_TYPE_INVALID,	{NULL,			NULL,			NULL}},
};

extern const MkdgTypeInterface *maker_dialog_find_gtype_interface(GType type);

const MkdgTypeInterface *maker_dialog_find_type_interface(MkdgType mType){
    gsize i;
    for(i=0;mkdgTypeInterfaces[i].type!=MKDG_TYPE_INVALID;i++){
	if (mkdgTypeInterfaces[i].type==mType){
	    return &mkdgTypeInterfaces[i].typeInterface;
	}
    }
    return maker_dialog_find_gtype_interface(maker_dialog_type_to_g_type(mType));
}

/*=== End Type Interface functions ===*/

MkdgValue *maker_dialog_value_new(MkdgType mType, GValue *gValue){
    MkdgValue *mValue=g_new(MkdgValue, 1);
    mValue->mType=mType;
    mValue->value=g_new0(GValue, 1);
    g_value_init(mValue->value, maker_dialog_type_to_g_type(mType));
    if (gValue){
	g_value_copy(gValue,mValue->value);
    }
    return mValue;
}

void maker_dialog_value_free(gpointer mValue){
    MkdgValue *mV=(MkdgValue *) mValue;
    maker_dialog_g_value_free(mV->value);
    g_free(mV);
}

MkdgValue *maker_dialog_value_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption){
    const MkdgTypeInterface *typeInterface=maker_dialog_find_type_interface(mValue->mType);
    if (!typeInterface)
	return NULL;
    typeInterface->from_string(mValue->value, str, parseOption);
    return mValue;
}

gchar *maker_dialog_value_to_string(MkdgValue *mValue, const gchar *toStringFormat){
    const MkdgTypeInterface *typeInterface=maker_dialog_find_type_interface(mValue->mType);
    if (!typeInterface)
	return NULL;
    return typeInterface->to_string(mValue->value, toStringFormat);
}

gchar *maker_dialog_string_normalized(const gchar *str, MkdgType mType){
    MkdgValue *mValue=maker_dialog_value_new(mType,NULL);
    maker_dialog_value_from_string(mValue, str, NULL);
    gchar *result=maker_dialog_value_to_string(mValue, NULL);
    maker_dialog_value_free(mValue);
    return result;
}

gint maker_dialog_value_compare(MkdgValue *mValue1, MkdgValue *mValue2, MakerDialogCompareFunc compFunc){
    const MkdgTypeInterface *typeInterface=maker_dialog_find_type_interface(mValue2->mType);
    if (!typeInterface)
	return -2;
    typeInterface=maker_dialog_find_type_interface(mValue1->mType);
    if (!typeInterface)
	return -2;
    return typeInterface->compare(mValue1->value,mValue2->value, compFunc);
}


