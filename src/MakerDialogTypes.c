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
#include "MakerDialogTypes.h"
#include "MakerDialogUtil.h"
#include <stdlib.h>
#include <string.h>
#include <glib/gprintf.h>

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


void maker_dialog_value_free(gpointer value){
    GValue *gValue=(GValue  *) value;
    g_value_unset(gValue);
    g_free(gValue);
}

gboolean maker_dialog_value_is_number(GValue *value){
    switch (G_VALUE_TYPE(value)){
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

gdouble maker_dialog_value_get_double(GValue *value){
    switch (G_VALUE_TYPE(value)){
	case MKDG_TYPE_INT:
	    return (gdouble) g_value_get_int(value);
	case MKDG_TYPE_UINT:
	    return (gdouble) g_value_get_uint(value);
	case MKDG_TYPE_LONG:
	    return (gdouble) g_value_get_long(value);
	case MKDG_TYPE_ULONG:
	    return (gdouble) g_value_get_ulong(value);
	case MKDG_TYPE_FLOAT:
	    return (gdouble) g_value_get_float(value);
	case MKDG_TYPE_DOUBLE:
	    return (gdouble) g_value_get_double(value);
	default:
	    break;
    }
    return  0.0;
}

void maker_dialog_value_set_number(GValue *value, gdouble number){
    switch (G_VALUE_TYPE(value)){
	case MKDG_TYPE_INT:
	    g_value_set_int(value, (gint) number);
	    break;
	case MKDG_TYPE_UINT:
	    g_value_set_uint(value, (guint) number);
	    break;
	case MKDG_TYPE_LONG:
	    g_value_set_long(value, (glong) number);
	    break;
	case MKDG_TYPE_ULONG:
	    g_value_set_ulong(value, (gulong) number);
	    break;
	case MKDG_TYPE_FLOAT:
	    g_value_set_float(value, (gfloat) number);
	    break;
	case MKDG_TYPE_DOUBLE:
	    g_value_set_double(value, (gdouble) number);
	    break;
	default:
	    break;
    }
}

/*=== Start Type Handler functions ===*/
typedef struct{
    GType type;
    GValue *(* from_string) (GValue *value, const gchar *str, const gchar *parseOption);
    gchar *(* to_string) (GValue *value, const gchar *toStringFormat);
    gint (* compare) (GValue *value1, GValue *value2, MakerDialogCompareFunc func);
} TypeHandler;

static GValue *md_boolean_from_string(GValue *value, const gchar *str, const gchar *parseOption){
    gboolean val=maker_dialog_atob(str);
    g_value_set_boolean(value,val);
    return value;
}

static gchar *md_boolean_to_string(GValue *value, const gchar *toStringFormat){
    return g_strdup((g_value_get_boolean(value))? "TRUE" : "FALSE");
}

static gint md_boolean_compare(GValue *value1, GValue *value2, MakerDialogCompareFunc func){
    if (G_VALUE_TYPE(value2)!=MKDG_TYPE_BOOLEAN){
	return -3;
    }
    gboolean val1=g_value_get_boolean (value1);
    gboolean val2=g_value_get_boolean (value2);
    if (func){
	return func(&val1, &val2);
    }
    if (val1==val2){
	return 0;
    }
    if (val1==TRUE){
	return 1;
    }
    return -1;
}

static gint md_number_compare(GValue *value1, GValue *value2, MakerDialogCompareFunc func){
    if (!maker_dialog_value_is_number(value2)){
	return -3;
    }
    gdouble val1=maker_dialog_value_get_double(value1);
    gdouble val2=maker_dialog_value_get_double(value2);
    if (func){
	return func(&val1, &val2);
    }
    if (val1==val2){
	return 0;
    }
    if (val1>val2){
	return 1;
    }
    return -1;
}

static gint determine_base(const gchar *str, const gchar *parseOption, gchar **startPtr){
    gint base=10;
    *startPtr=(gchar *) str;
    if (parseOption){
	base=atoi (parseOption);
    }else{
	if (strlen(str)>2 && str[0]=='0' && str[1]=='x'){
	    base=16;
	    *startPtr= (gchar *) str+2;
	}else if (strlen(str)>2 && str[0]=='0' && str[1]!='.'){
	    base=8;
	    *startPtr= (gchar *) str+1;
	}
    }
    return base;
}

static GValue *md_int_from_string(GValue *value, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    gint base=determine_base(str, parseOption, &startPtr);
    gint val=(gint) strtol(startPtr, NULL, base);
    g_value_set_int(value,val);
    return value;
}

static gchar *md_int_to_string(GValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%d";
    g_string_printf(strBuf, toStringFormat ,g_value_get_int(value));
    return g_string_free(strBuf, FALSE);
}

static GValue *md_uint_from_string(GValue *value, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    gint base=determine_base(str, parseOption, &startPtr);
    guint val=(guint) strtol(startPtr, NULL, base);
    g_value_set_uint(value,val);
    return value;
}

static gchar *md_uint_to_string(GValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%u";
    g_string_printf(strBuf, toStringFormat ,g_value_get_uint(value));
    return g_string_free(strBuf, FALSE);
}


static GValue *md_long_from_string(GValue *value, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    gint base=determine_base(str,  parseOption, &startPtr);
    glong val= strtol(startPtr, NULL, base);
    g_value_set_long(value,val);
    return value;
}

static gchar *md_long_to_string(GValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%ld";
    g_string_printf(strBuf, toStringFormat ,g_value_get_long(value));
    return g_string_free(strBuf, FALSE);
}

static GValue *md_ulong_from_string(GValue *value, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    gint base=determine_base(str, parseOption, &startPtr);
    gulong val= (gulong) strtoll(startPtr, NULL, base);
    g_value_set_ulong(value,val);
    return value;
}

static gchar *md_ulong_to_string(GValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%lu";
    g_string_printf(strBuf, toStringFormat ,g_value_get_ulong(value));
    return g_string_free(strBuf, FALSE);
}

static GValue *md_float_from_string(GValue *value, const gchar *str, const gchar *parseOption){
    gfloat val= (gfloat) strtod(str, NULL);
    g_value_set_float(value,val);
    return value;
}

static gchar *md_float_to_string(GValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%g";
    g_string_printf(strBuf, toStringFormat ,g_value_get_float(value));
    return g_string_free(strBuf, FALSE);
}

static GValue *md_double_from_string(GValue *value, const gchar *str, const gchar *parseOption){
    gdouble val= strtod(str, NULL);
    g_value_set_double(value,val);
    return value;
}

static gchar *md_double_to_string(GValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%g";
    g_string_printf(strBuf, toStringFormat ,g_value_get_double(value));
    return g_string_free(strBuf, FALSE);
}

static GValue *md_string_from_string(GValue *value, const gchar *str, const gchar *parseOption){
    g_value_set_string(value, str);
    return value;
}

static gchar *md_string_to_string(GValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%s";
    g_string_printf(strBuf, toStringFormat ,g_value_get_string(value));
    return g_string_free(strBuf, FALSE);
}

static gint md_string_compare(GValue *value1, GValue *value2, MakerDialogCompareFunc func){
    if (G_VALUE_TYPE(value2)!=MKDG_TYPE_STRING){
	return -3;
    }
    if (func){
	return func((gchar *)g_value_get_string(value1), (gchar *) g_value_get_string(value2));
    }
    return strcmp((gchar *) g_value_get_string(value1), (gchar *) g_value_get_string(value2));
}

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
    return value;
}

static gchar *md_color_to_string(GValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    gchar buf[12];
    if (!toStringFormat)
	toStringFormat="%s";

    guint color=g_value_get_uint(value);
    gint index=find_color_index_by_value(color);
    if (index>=0){
	g_string_printf(strBuf, toStringFormat, mkdgColorList[index].name);
    }else{
	g_snprintf(buf,12,"#%X", color);
	g_string_printf(strBuf, toStringFormat, buf);
    }
    return g_string_free(strBuf, FALSE);
}

const TypeHandler typeHandlers[]={
    { MKDG_TYPE_BOOLEAN, 	md_boolean_from_string,	md_boolean_to_string,	md_boolean_compare},
    { MKDG_TYPE_INT,		md_int_from_string,	md_int_to_string,	md_number_compare},
    { MKDG_TYPE_UINT,		md_uint_from_string,	md_uint_to_string, 	md_number_compare},
    { MKDG_TYPE_LONG,		md_long_from_string,	md_long_to_string,	md_number_compare},
    { MKDG_TYPE_ULONG,		md_ulong_from_string,	md_ulong_to_string,	md_number_compare},
    { MKDG_TYPE_FLOAT,		md_float_from_string,	md_float_to_string, 	md_number_compare},
    { MKDG_TYPE_DOUBLE,		md_double_from_string,	md_double_to_string, 	md_number_compare},
    { MKDG_TYPE_STRING,		md_string_from_string,	md_string_to_string, 	md_string_compare},
    { MKDG_TYPE_INVALID,	NULL,			NULL,			NULL},

};

static const TypeHandler *maker_dialog_find_type_handler(GType type){
    gsize i;
    for(i=0;typeHandlers[i].type!=MKDG_TYPE_INVALID;i++){
	if (typeHandlers[i].type==type){
	    return &typeHandlers[i];
	}
    }
    return NULL;
}
/*=== End Type Handler functions ===*/

GValue *maker_dialog_value_from_string(GValue *value, const gchar *str, const gchar *parseOption){
    if (!str)
	return NULL;
    const TypeHandler *typeHandler=maker_dialog_find_type_handler(G_VALUE_TYPE(value));
    if (!typeHandler)
	return NULL;
    typeHandler->from_string(value, str, parseOption);
    return value;
}

gchar *maker_dialog_value_to_string(GValue *value, const gchar *toStringFormat){
    const TypeHandler *typeHandler=maker_dialog_find_type_handler(G_VALUE_TYPE(value));
    if (!typeHandler)
	return NULL;
    return typeHandler->to_string(value, toStringFormat);
}

gchar *maker_dialog_string_normalized(const gchar *str, GType type){
    GValue value={0};
    g_value_init(&value, type);
    maker_dialog_value_from_string(&value, str, NULL);
    gchar *result=maker_dialog_value_to_string(&value, NULL);
    g_value_unset(&value);
    return result;
}


gint maker_dialog_value_compare(GValue *value1, GValue *value2, MakerDialogCompareFunc compFunc){
    const TypeHandler *typeHandler=maker_dialog_find_type_handler(G_VALUE_TYPE(value2));
    if (!typeHandler)
	return -2;
    typeHandler=maker_dialog_find_type_handler(G_VALUE_TYPE(value1));
    if (!typeHandler)
	return -2;
    return typeHandler->compare(value1,value2, compFunc);
}


