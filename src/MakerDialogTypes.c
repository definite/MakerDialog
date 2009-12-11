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

/*=== Start Type Interface functions ===*/
typedef struct{
    MkdgType type;
    const gchar *name;
    MkdgTypeInterface typeInterface;
} MkdgTypeInterfaceMkdgType;

static void md_boolean_set(MkdgValue *value, gpointer setValue){
    value->data[0].v_boolean=(setValue) ? *(gboolean *) setValue: FALSE;
}

static MkdgValue *md_boolean_from_string(MkdgValue *value, const gchar *str, const gchar *parseOption){
    value->data[0].v_boolean=maker_dialog_atob(str);
    return value;
}

static gchar *md_boolean_to_string(MkdgValue *value, const gchar *toStringFormat){
    return g_strdup(value->data[0].v_boolean ? "TRUE" : "FALSE");
}

static gint md_boolean_compare(MkdgValue *value1, MkdgValue *value2, const gchar *compareOption){
    if (G_VALUE_TYPE(value2)!=G_TYPE_BOOLEAN){
	return -3;
    }
    gboolean val1=value1->data[0].v_boolean;
    gboolean val2=value2->data[0].v_boolean;
    if (val1==val2){
	return 0;
    }
    if (val1==TRUE){
	return 1;
    }
    return -1;
}


static gint md_number_compare(MkdgValue *value1, MkdgValue *value2, const gchar *compareOption){
    if (!maker_dialog_type_is_number(value2->mType)){
	return -3;
    }
    gdouble val1=maker_dialog_value_to_double(value1);
    gdouble val2=maker_dialog_value_to_double(value2);
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

static void md_int_set(MkdgValue *value, gpointer setValue){
    value->data[0].v_int=(setValue) ? *(gint *) setValue: 0;
}

static MkdgValue *md_int_from_string(MkdgValue *value, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    if (G_UNLIKELY(maker_dialog_string_is_empty(str))){
	maker_dialog_value_set_int(value,0);
    }else{
	gint base=determine_base(str, parseOption, &startPtr);
	gint val=(gint) strtol(startPtr, NULL, base);
	maker_dialog_value_set_int(value,val);
    }
    return value;
}

static gchar *md_int_to_string(MkdgValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%d";
    g_string_printf(strBuf, toStringFormat ,maker_dialog_value_get_int(value));
    return g_string_free(strBuf, FALSE);
}

static void md_uint_set(MkdgValue *value, gpointer setValue){
    value->data[0].v_uint=(setValue) ? *(guint *) setValue: 0;
}

static MkdgValue *md_uint_from_string(MkdgValue *value, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    if (G_UNLIKELY(maker_dialog_string_is_empty(str))){
	maker_dialog_value_set_uint(value,0);
    }else{
	gint base=determine_base(str, parseOption, &startPtr);
	guint val=(guint) strtol(startPtr, NULL, base);
	maker_dialog_value_set_uint(value,val);
    }
    return value;
}

static gchar *md_uint_to_string(MkdgValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%u";
    g_string_printf(strBuf, toStringFormat ,maker_dialog_value_get_uint(value));
    return g_string_free(strBuf, FALSE);
}

static void md_int32_set(MkdgValue *value, gpointer setValue){
    value->data[0].v_int32=(setValue) ? *(gint32 *) setValue: 0;
}

static MkdgValue *md_int32_from_string(MkdgValue *value, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    if (G_UNLIKELY(maker_dialog_string_is_empty(str))){
	maker_dialog_value_set_int32(value,0);
    }else{
	gint base=determine_base(str, parseOption, &startPtr);
	gint32 val=(gint32) strtol(startPtr, NULL, base);
	maker_dialog_value_set_int32(value,val);
    }
    return value;
}

static gchar *md_int32_to_string(MkdgValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%d";
    g_string_printf(strBuf, toStringFormat ,maker_dialog_value_get_int32(value));
    return g_string_free(strBuf, FALSE);
}

static void md_uint32_set(MkdgValue *value, gpointer setValue){
    value->data[0].v_uint32=(setValue) ? *(guint32 *) setValue: 0;
}

static MkdgValue *md_uint32_from_string(MkdgValue *value, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    if (G_UNLIKELY(maker_dialog_string_is_empty(str))){
	maker_dialog_value_set_uint32(value,0);
    }else{
	gint base=determine_base(str, parseOption, &startPtr);
	guint32 val=(guint32) strtol(startPtr, NULL, base);
	maker_dialog_value_set_uint32(value,val);
    }
    return value;
}

static gchar *md_uint32_to_string(MkdgValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%u";
    g_string_printf(strBuf, toStringFormat ,maker_dialog_value_get_uint32(value));
    return g_string_free(strBuf, FALSE);
}

static void md_int64_set(MkdgValue *value, gpointer setValue){
    value->data[0].v_int64=(setValue) ? *(gint64 *) setValue: 0;
}

static MkdgValue *md_int64_from_string(MkdgValue *value, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    if (G_UNLIKELY(maker_dialog_string_is_empty(str))){
	maker_dialog_value_set_int64(value,0);
    }else{
	gint base=determine_base(str, parseOption, &startPtr);
	gint64 val=(gint64) strtol(startPtr, NULL, base);
	maker_dialog_value_set_int64(value,val);
    }
    return value;
}

static gchar *md_int64_to_string(MkdgValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%d";
    g_string_printf(strBuf, toStringFormat ,maker_dialog_value_get_int64(value));
    return g_string_free(strBuf, FALSE);
}

static void md_uint64_set(MkdgValue *value, gpointer setValue){
    value->data[0].v_uint64=(setValue) ? *(guint64 *) setValue: 0;
}

static MkdgValue *md_uint64_from_string(MkdgValue *value, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    if (G_UNLIKELY(maker_dialog_string_is_empty(str))){
	maker_dialog_value_set_uint64(value,0);
    }else{
	gint base=determine_base(str, parseOption, &startPtr);
	guint64 val=(guint64) strtol(startPtr, NULL, base);
	maker_dialog_value_set_uint64(value,val);
    }
    return value;
}

static gchar *md_uint64_to_string(MkdgValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%u";
    g_string_printf(strBuf, toStringFormat ,maker_dialog_value_get_uint64(value));
    return g_string_free(strBuf, FALSE);
}

static void md_long_set(MkdgValue *value, gpointer setValue){
    value->data[0].v_long=(setValue) ? *(glong *) setValue: 0;
}

static MkdgValue *md_long_from_string(MkdgValue *value, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    if (G_UNLIKELY(maker_dialog_string_is_empty(str))){
	maker_dialog_value_set_long(value,0l);
    }else{
	gint base=determine_base(str,  parseOption, &startPtr);
	glong val= strtol(startPtr, NULL, base);
	maker_dialog_value_set_long(value,val);
    }
    return value;
}

static gchar *md_long_to_string(MkdgValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%ld";
    g_string_printf(strBuf, toStringFormat ,maker_dialog_value_get_long(value));
    return g_string_free(strBuf, FALSE);
}

static void md_ulong_set(MkdgValue *value, gpointer setValue){
    value->data[0].v_ulong=(setValue) ? *(gulong *) setValue: 0;
}

static MkdgValue *md_ulong_from_string(MkdgValue *value, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    if (G_UNLIKELY(maker_dialog_string_is_empty(str))){
	maker_dialog_value_set_ulong(value,0);
    }else{
	gint base=determine_base(str, parseOption, &startPtr);
	gulong val= (gulong) strtoll(startPtr, NULL, base);
	maker_dialog_value_set_ulong(value,val);
    }
    return value;
}

static gchar *md_ulong_to_string(MkdgValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%lu";
    g_string_printf(strBuf, toStringFormat ,maker_dialog_value_get_ulong(value));
    return g_string_free(strBuf, FALSE);
}

static void md_float_set(MkdgValue *value, gpointer setValue){
    value->data[0].v_float=(setValue) ? *(gfloat *) setValue: 0.0f;
}

static MkdgValue *md_float_from_string(MkdgValue *value, const gchar *str, const gchar *parseOption){
    gfloat val= (G_UNLIKELY(maker_dialog_string_is_empty(str))) ? 0.0f: (gfloat) strtod(str, NULL);
    maker_dialog_value_set_float(value,val);
    return value;
}

static gchar *md_float_to_string(MkdgValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%g";
    g_string_printf(strBuf, toStringFormat ,maker_dialog_value_get_float(value));
    return g_string_free(strBuf, FALSE);
}

static void md_double_set(MkdgValue *value, gpointer setValue){
    value->data[0].v_double=(setValue) ? *(gdouble *) setValue: 0.0;
}

static MkdgValue *md_double_from_string(MkdgValue *value, const gchar *str, const gchar *parseOption){
    gdouble val= (G_UNLIKELY(maker_dialog_string_is_empty(str))) ? 0.0 : (gdouble) strtod(str, NULL);
    maker_dialog_value_set_double(value,val);
    return value;
}

static gchar *md_double_to_string(MkdgValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%g";
    g_string_printf(strBuf, toStringFormat ,maker_dialog_value_get_double(value));
    return g_string_free(strBuf, FALSE);
}

static void md_string_set(MkdgValue *value, gpointer setValue){
    if (value->flags & MKDG_VALUE_FLAG_NEED_FREE ){
	value->data[0].v_pointer= g_strdup( (setValue) ? (gchar *) setValue : "");
    }else{
	value->data[0].v_pointer= (setValue) ? setValue : "";
    }
}

static MkdgValue *md_string_from_string(MkdgValue *value, const gchar *str, const gchar *parseOption){
    const gchar *strValue= g_strdup((G_UNLIKELY(maker_dialog_string_is_empty(str))) ? "" : str);
    maker_dialog_value_set_string(value, strValue);
    return value;
}

static gchar *md_string_to_string(MkdgValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%s";
    g_string_printf(strBuf, toStringFormat ,maker_dialog_value_get_string(value));
    return g_string_free(strBuf, FALSE);
}

#define STRING_COMPARE_CASE_INSENSITIVE_FLAG	0x1
#define STRING_COMPARE_CASE_INSENSITIVE_OPTSTR  "icase"
static gint md_string_compare(MkdgValue *value1, MkdgValue *value2, const gchar *compareOption){
    if (value2->mType!=MKDG_TYPE_STRING){
	return -3;
    }
    gint ret;
    guint flags=0;
    if (compareOption){
	gchar **options=g_strsplit_set(compareOption,";",-1);
	gint i;
	for(i=0; options[i]!=NULL; i++){
	    if (strcmp(options[i], STRING_COMPARE_CASE_INSENSITIVE_OPTSTR)==0){
		flags |= STRING_COMPARE_CASE_INSENSITIVE_FLAG;
	    }
	}
	g_strfreev(options);
    }
    gchar *str1=maker_dialog_value_get_string(value1);
    gchar *str2=maker_dialog_value_get_string(value2);
    MAKER_DIALOG_DEBUG_MSG(7, "[I7] md_string_compare() str1=%s str2=%s",str1, str2);
    if (flags & STRING_COMPARE_CASE_INSENSITIVE_FLAG){
	ret=g_ascii_strcasecmp(str1, str2);
    }else{
	ret=strcmp(str1, str2);
    }
    if (ret>0)
	ret=1;
    else if (ret<0)
	ret=-1;
    return ret;
}

static void md_string_free(MkdgValue *value){
    if (value->flags  & MKDG_VALUE_FLAG_NEED_FREE){
	g_free(value->data[0].v_pointer);
    }
}

static void md_color_set(MkdgValue *value, gpointer setValue){
    maker_dialog_value_set_color(value, (setValue) ? *(guint32 *) setValue: 0);
}

static MkdgValue *md_color_from_string(MkdgValue *value, const gchar *str, const gchar *parseOption){
    if (G_UNLIKELY(maker_dialog_string_is_empty(str))){
	/* Default is black */
	maker_dialog_value_set_color(value, 0);
    }else{
	if (str[0]=='#'){
	    maker_dialog_value_set_color(value, (MkdgColor) strtol(str+sizeof(gchar), NULL, 16));
	}else{
	    gint index=find_color_index_by_name(str);
	    if (index>=0){
		maker_dialog_value_set_color(value, mkdgColorList[index].value);
	    }else{
		/* Default is black */
		maker_dialog_value_set_color(value, 0);
	    }
	}
    }
    return value;
}

static gchar *md_color_to_string(MkdgValue *value, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    gchar buf[12];
    if (!toStringFormat)
	toStringFormat="%s";
    MkdgColor color=maker_dialog_value_get_color(value);
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

static gint md_color_compare(MkdgValue *value1, MkdgValue *value2, const gchar *compareOption){
    if (G_VALUE_TYPE(value2)!=MKDG_TYPE_COLOR){
	return -3;
    }
    MkdgColor v1=maker_dialog_value_get_color(value1);
    MkdgColor v2=maker_dialog_value_get_color(value2);
    return (v1==v2) ? 0 : (v1>v2)? 1: -1;
}


const MkdgTypeInterfaceMkdgType mkdgTypeInterfaces[]={
    { MKDG_TYPE_BOOLEAN,	"BOOLEAN",	{md_boolean_set,	md_boolean_from_string, md_boolean_to_string,	md_boolean_compare,	NULL}},
    { MKDG_TYPE_INT,		"INT",		{md_int_set,		md_int_from_string,	md_int_to_string,	md_number_compare,	NULL}},
    { MKDG_TYPE_UINT,		"UINT",		{md_uint_set,		md_uint_from_string,	md_uint_to_string, 	md_number_compare,	NULL}},
    { MKDG_TYPE_INT32,		"INT32",	{md_int32_set,		md_int32_from_string,	md_int32_to_string,	md_number_compare,	NULL}},
    { MKDG_TYPE_UINT32,		"UINT32",	{md_uint32_set,		md_uint32_from_string,	md_uint32_to_string, 	md_number_compare,	NULL}},
    { MKDG_TYPE_INT64,		"INT64",	{md_int64_set,		md_int64_from_string,	md_int64_to_string,	md_number_compare,	NULL}},
    { MKDG_TYPE_UINT64,		"UINT64",	{md_uint64_set,		md_uint64_from_string,	md_uint64_to_string, 	md_number_compare,	NULL}},
    { MKDG_TYPE_LONG,		"LONG",		{md_long_set,		md_long_from_string,	md_long_to_string,	md_number_compare,	NULL}},
    { MKDG_TYPE_ULONG,		"ULONG",	{md_ulong_set,		md_ulong_from_string,	md_ulong_to_string,	md_number_compare,	NULL}},
    { MKDG_TYPE_FLOAT,		"FLOAT",	{md_float_set,		md_float_from_string,	md_float_to_string, 	md_number_compare,	NULL}},
    { MKDG_TYPE_DOUBLE,		"DOUBLE",	{md_double_set,		md_double_from_string,	md_double_to_string, 	md_number_compare,	NULL}},
    { MKDG_TYPE_STRING,		"STRING",	{md_string_set,		md_string_from_string,	md_string_to_string, 	md_string_compare,	md_string_free}},
    { MKDG_TYPE_COLOR,		"COLOR",	{md_color_set,		md_color_from_string,	md_color_to_string, 	md_color_compare,	NULL}},
    { MKDG_TYPE_NONE,		"NONE",		{NULL,			NULL,			NULL,			NULL,			NULL}},
    { MKDG_TYPE_INVALID,	"INVALID",	{NULL,			NULL,			NULL,			NULL,			NULL}},
};

static const MkdgTypeInterface *maker_dialog_find_type_interface(MkdgType mType){
    if (mType<0 || mType>MKDG_TYPE_NONE){
	return NULL;
    }
    return &mkdgTypeInterfaces[mType].typeInterface;
}

/*=== End Type Interface functions ===*/
MkdgType maker_dialog_type_parse(const gchar *str){
    MkdgType mType;
    for(mType=0; mType<=MKDG_TYPE_NONE; mType++){
	if (g_ascii_strcasecmp(str,mkdgTypeInterfaces[mType].name)==0)
	    return mType;
    }
    return MKDG_TYPE_INVALID;
}

const gchar *maker_dialog_type_to_string(MkdgType mType){
    if (mType<0 || mType>MKDG_TYPE_NONE){
	return NULL;
    }
    return mkdgTypeInterfaces[mType].name;
}

static void maker_dialog_value_set_private(MkdgValue *mValue, gpointer setValue, const MkdgTypeInterface *typeInterface){
    if (maker_dialog_type_is_pointer(mValue->mType)){
	if (mValue->data[0].v_pointer && (mValue->flags & MKDG_VALUE_FLAG_NEED_FREE)){
	    /* Free old data */
	    typeInterface->free(mValue);
	}
    }
    typeInterface->set(mValue, setValue);
}

MkdgValue *maker_dialog_value_new(MkdgType mType, gpointer setValue){
    const MkdgTypeInterface *typeInterface=maker_dialog_find_type_interface(mType);
    if (!typeInterface)
	return NULL;
    MkdgValue *mValue=g_new0(MkdgValue, 1);
    mValue->flags=0;
    mValue->mType=mType;
    if (maker_dialog_type_is_pointer(mValue->mType)){
	mValue->data[0].v_pointer=NULL;
	mValue->flags |= MKDG_VALUE_FLAG_NEED_FREE;
    }
    maker_dialog_value_set_private(mValue, setValue, typeInterface);
    return mValue;
}

MkdgValue *maker_dialog_value_new_static(MkdgType mType, gpointer setValue){
    const MkdgTypeInterface *typeInterface=maker_dialog_find_type_interface(mType);
    if (!typeInterface)
	return NULL;
    MkdgValue *mValue=g_new0(MkdgValue, 1);
    mValue->flags=0;
    mValue->mType=mType;
    if (maker_dialog_type_is_pointer(mValue->mType)){
	mValue->data[0].v_pointer=NULL;
    }
    maker_dialog_value_set_private(mValue, setValue, typeInterface);
    return mValue;
}

gboolean maker_dialog_value_copy(MkdgValue *srcValue, MkdgValue *destValue){
    if (srcValue->mType!=destValue->mType){
	return FALSE;
    }
    if (maker_dialog_type_is_pointer(srcValue->mType)){
	maker_dialog_value_set(destValue, srcValue->data[0].v_pointer);
    }else{
	destValue->data[0]=srcValue->data[0];
	destValue->data[1]=srcValue->data[1];
    }
    return TRUE;
}

void maker_dialog_value_set(MkdgValue *mValue, gpointer setValue){
    const MkdgTypeInterface *typeInterface=maker_dialog_find_type_interface(mValue->mType);
    maker_dialog_value_set_private(mValue, setValue, typeInterface);
}

void maker_dialog_value_free(gpointer mValue){
    MkdgValue *mV=(MkdgValue *) mValue;
    const MkdgTypeInterface *typeInterface=maker_dialog_find_type_interface(mV->mType);
    if (mV->flags  & MKDG_VALUE_FLAG_NEED_FREE){
	typeInterface->free(mV);
    }
    g_free(mV);
}

gboolean maker_dialog_type_is_pointer(MkdgType mType){
    switch (mType){
	case MKDG_TYPE_STRING:
	    return TRUE;
	default:
	    break;
    }
    return FALSE;
}

gboolean maker_dialog_type_is_number(MkdgType mType){
    switch (mType){
	case MKDG_TYPE_INT:
	case MKDG_TYPE_UINT:
	case MKDG_TYPE_INT32:
	case MKDG_TYPE_UINT32:
	case MKDG_TYPE_INT64:
	case MKDG_TYPE_UINT64:
	case MKDG_TYPE_LONG:
	case MKDG_TYPE_ULONG:
	case MKDG_TYPE_FLOAT:
	case MKDG_TYPE_DOUBLE:
	case MKDG_TYPE_COLOR:
	    return TRUE;
	default:
	    break;
    }
    return FALSE;
}

void maker_dialog_value_from_double(MkdgValue *value, gdouble number){
    switch (value->mType){
	case MKDG_TYPE_INT:
	    maker_dialog_value_set_int(value, (gint) number);
	    break;
	case MKDG_TYPE_UINT:
	    maker_dialog_value_set_uint(value, (guint) number);
	    break;
	case MKDG_TYPE_INT32:
	    maker_dialog_value_set_int32(value, (gint32) number);
	    break;
	case MKDG_TYPE_UINT32:
	    maker_dialog_value_set_uint32(value, (guint32) number);
	    break;
	case MKDG_TYPE_INT64:
	    maker_dialog_value_set_int64(value, (gint64) number);
	    break;
	case MKDG_TYPE_UINT64:
	    maker_dialog_value_set_uint64(value, (guint64) number);
	    break;
	case MKDG_TYPE_LONG:
	    maker_dialog_value_set_long(value, (glong) number);
	    break;
	case MKDG_TYPE_ULONG:
	    maker_dialog_value_set_ulong(value, (gulong) number);
	    break;
	case MKDG_TYPE_FLOAT:
	    maker_dialog_value_set_float(value, (gfloat) number);
	    break;
	case MKDG_TYPE_DOUBLE:
	    maker_dialog_value_set_double(value, (gdouble) number);
	    break;
	case MKDG_TYPE_COLOR:
	    maker_dialog_value_set_color(value, (guint32) number);
	    break;
	default:
	    break;
    }
}

gdouble maker_dialog_value_to_double(MkdgValue *value){
    switch (value->mType){
	case MKDG_TYPE_INT:
	    return (gdouble) maker_dialog_value_get_int(value);
	case MKDG_TYPE_UINT:
	    return (gdouble) maker_dialog_value_get_uint(value);
	case MKDG_TYPE_INT32:
	    return (gdouble) maker_dialog_value_get_int32(value);
	case MKDG_TYPE_UINT32:
	    return (gdouble) maker_dialog_value_get_uint32(value);
	case MKDG_TYPE_INT64:
	    return (gdouble) maker_dialog_value_get_int64(value);
	case MKDG_TYPE_UINT64:
	    return (gdouble) maker_dialog_value_get_uint64(value);
	case MKDG_TYPE_LONG:
	    return (gdouble) maker_dialog_value_get_long(value);
	case MKDG_TYPE_ULONG:
	    return (gdouble) maker_dialog_value_get_ulong(value);
	case MKDG_TYPE_FLOAT:
	    return (gdouble) maker_dialog_value_get_float(value);
	case MKDG_TYPE_DOUBLE:
	    return (gdouble) maker_dialog_value_get_double(value);
	case MKDG_TYPE_COLOR:
	    return (gdouble) maker_dialog_value_get_double(value);
	default:
	    break;
    }
    return  0.0;
}

MkdgValue *maker_dialog_value_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption){
    const MkdgTypeInterface *typeInterface=maker_dialog_find_type_interface(mValue->mType);
    if (!typeInterface)
	return NULL;
    if (maker_dialog_type_is_pointer(mValue->mType)){
	if (mValue->data[0].v_pointer && (mValue->flags  & MKDG_VALUE_FLAG_NEED_FREE)){
	    /* Free old data */
	    typeInterface->free(mValue);
	}
    }
    typeInterface->from_string(mValue, str, parseOption);
    return mValue;
}

gchar *maker_dialog_value_to_string(MkdgValue *mValue, const gchar *toStringFormat){
    const MkdgTypeInterface *typeInterface=maker_dialog_find_type_interface(mValue->mType);
    if (!typeInterface)
	return NULL;
    return typeInterface->to_string(mValue, toStringFormat);
}

gchar *maker_dialog_string_convert(const gchar *str, MkdgType mType, const gchar *parseOption, const gchar *toStringFormat){
    MkdgValue *mValue=maker_dialog_value_new(mType,NULL);
    maker_dialog_value_from_string(mValue, str, parseOption);
    gchar *result=maker_dialog_value_to_string(mValue, toStringFormat);
    maker_dialog_value_free(mValue);
    return result;
}

gchar *maker_dialog_string_normalized(const gchar *str, MkdgType mType){
    return maker_dialog_string_convert(str, mType, NULL, NULL);
}

gint maker_dialog_value_compare(MkdgValue *mValue1, MkdgValue *mValue2, const gchar *compareOption){
    const MkdgTypeInterface *typeInterface=maker_dialog_find_type_interface(mValue2->mType);
    if (!typeInterface)
	return -2;
    typeInterface=maker_dialog_find_type_interface(mValue1->mType);
    if (!typeInterface)
	return -2;
    return typeInterface->compare(mValue1,mValue2, compareOption);
}

/*=== Start Type get/ set functions ===*/
//gboolean maker_dialog_value_get_boolean(MkdgValue *mValue){
//    return mValue->data[0].v_boolean;
//}

//void maker_dialog_value_set_boolean(MkdgValue *mValue, gboolean setValue){
//    mValue->data[0].v_boolean = setValue;
//}



