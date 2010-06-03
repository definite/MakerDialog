/*
 * Copyright © 2009  Red Hat, Inc. All rights reserved.
 * Copyright © 2009  Ding-Yi Chen <dchen at redhat.com>
 *
 *  This file is part of Mkdg.
 *
 *  Mkdg is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Mkdg is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Mkdg.  If not, see <http://www.gnu.org/licenses/>.
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
    {"Aqua",		0x00FFFF},
    {"Aquamarine",	0x7FFFD4},
    {"Azure",		0xF0FFFF},
    {"Black",		0x000000},
    {"Blue",		0x0000FF},
    {"Brown",		0xA52A2A},
    {"Chocolate",	0x7B3F00},
    {"Crimson",		0xDC143C},
    {"Coral",		0xFF7F50},
    {"Cyan",		0x00FFFF},
    {"Fuchsia",		0xFF00FF},
    {"Gold",		0xFFD700},
    {"Gray",		0x808080},
    {"Green",		0x008000},
    {"Ivory",		0xFFFFF0},
    {"Lime",		0x00FF00},
    {"Magenta",		0xFF00FF},
    {"Maroon",		0x800000},
    {"Navy",		0x000080},
    {"Olive",		0x808000},
    {"Orange",		0xFFA500},
    {"Orchid",		0xDA70D6},
    {"Pink",		0xFFC0CB},
    {"Purple",		0x800080},
    {"Red",		0xFF0000},
    {"Sliver",		0xC0C0C0},
    {"Teal",		0x008080},
    {"Violet",		0xEE82EE},
    {"Wheat",		0xF5DE83},
    {"White",		0xFFFFFF},
    {"Yellow",		0xFFFF00},
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

/*=== Start pointer type ===*/
static void md_pointer_extract(MkdgValue *mValue, gpointer ptr){
    gpointer *ptr2=(gpointer *) ptr;
    *ptr2=mkdg_value_get_pointer(mValue);
}

static void md_pointer_set(MkdgValue *mValue, gpointer setValue){
    mkdg_value_set_pointer(mValue,setValue);
}

static gint md_pointer_compare(MkdgValue *mValue1, MkdgValue *mValue2, const gchar *compareOption){
    if (mValue2->mType!=MKDG_TYPE_POINTER){
	return -3;
    }
    gpointer val1=mkdg_value_get_pointer(mValue1);
    gpointer val2=mkdg_value_get_pointer(mValue2);
    if (val1==val2){
	return 0;
    }
    return -1;
}
/*=== End pointer type ===*/
/*=== Start boolean type ===*/
static void md_boolean_extract(MkdgValue *mValue, gpointer ptr){
    gboolean *ptr2=(gboolean *) ptr;
    *ptr2=mkdg_value_get_boolean(mValue);
}

static void md_boolean_set(MkdgValue *mValue, gpointer setValue){
    mkdg_value_set_boolean(mValue, (setValue) ? *(gboolean *) setValue: FALSE);
}

static MkdgValue *md_boolean_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption){
    mValue->data[0].v_boolean=mkdg_atob(str);
    return mValue;
}

static gchar *md_boolean_to_string(MkdgValue *mValue, const gchar *toStringFormat){
    return g_strdup(mValue->data[0].v_boolean ? "TRUE" : "FALSE");
}

static gint md_boolean_compare(MkdgValue *mValue1, MkdgValue *mValue2, const gchar *compareOption){
    if (mValue2->mType!=MKDG_TYPE_BOOLEAN){
	return -3;
    }
    gboolean val1=mValue1->data[0].v_boolean;
    gboolean val2=mValue2->data[0].v_boolean;
    if (val1==val2){
	return 0;
    }
    if (val1==TRUE){
	return 1;
    }
    return -1;
}

/*=== End boolean type ===*/
/*=== Start number type ===*/
static gint md_number_compare(MkdgValue *mValue1, MkdgValue *mValue2, const gchar *compareOption){
    if (!mkdg_type_is_number(mValue2->mType)){
	return -3;
    }
    gdouble val1=mkdg_value_to_double(mValue1);
    gdouble val2=mkdg_value_to_double(mValue2);
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

static void md_int_extract(MkdgValue *mValue, gpointer ptr){
    gint *ptr2=(gint *) ptr;
    *ptr2=mkdg_value_get_int(mValue);
}

static void md_int_set(MkdgValue *mValue, gpointer setValue){
    mValue->data[0].v_int=(setValue) ? *(gint *) setValue: 0;
}

static MkdgValue *md_int_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    if (G_UNLIKELY(mkdg_string_is_empty(str))){
	mkdg_value_set_int(mValue,0);
    }else{
	gint base=determine_base(str, parseOption, &startPtr);
	gint val=(gint) strtol(startPtr, NULL, base);
	mkdg_value_set_int(mValue,val);
    }
    return mValue;
}

static gchar *md_int_to_string(MkdgValue *mValue, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%d";
    g_string_printf(strBuf, toStringFormat ,mkdg_value_get_int(mValue));
    return g_string_free(strBuf, FALSE);
}

static void md_uint_extract(MkdgValue *mValue, gpointer ptr){
    guint *ptr2=(guint *) ptr;
    *ptr2=mkdg_value_get_uint(mValue);
}

static void md_uint_set(MkdgValue *mValue, gpointer setValue){
    mValue->data[0].v_uint=(setValue) ? *(guint *) setValue: 0;
}

static MkdgValue *md_uint_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    if (G_UNLIKELY(mkdg_string_is_empty(str))){
	mkdg_value_set_uint(mValue,0);
    }else{
	gint base=determine_base(str, parseOption, &startPtr);
	guint val=(guint) strtol(startPtr, NULL, base);
	mkdg_value_set_uint(mValue,val);
    }
    return mValue;
}

static gchar *md_uint_to_string(MkdgValue *mValue, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%u";
    g_string_printf(strBuf, toStringFormat ,mkdg_value_get_uint(mValue));
    return g_string_free(strBuf, FALSE);
}

static void md_int32_extract(MkdgValue *mValue, gpointer ptr){
    gint32 *ptr2=(gint32 *) ptr;
    *ptr2=mkdg_value_get_int32(mValue);
}

static void md_int32_set(MkdgValue *mValue, gpointer setValue){
    mValue->data[0].v_int32=(setValue) ? *(gint32 *) setValue: 0;
}

static MkdgValue *md_int32_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    if (G_UNLIKELY(mkdg_string_is_empty(str))){
	mkdg_value_set_int32(mValue,0);
    }else{
	gint base=determine_base(str, parseOption, &startPtr);
	gint32 val=(gint32) strtol(startPtr, NULL, base);
	mkdg_value_set_int32(mValue,val);
    }
    return mValue;
}

static gchar *md_int32_to_string(MkdgValue *mValue, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%d";
    g_string_printf(strBuf, toStringFormat ,mkdg_value_get_int32(mValue));
    return g_string_free(strBuf, FALSE);
}

static void md_uint32_extract(MkdgValue *mValue, gpointer ptr){
    guint32 *ptr2=(guint32 *) ptr;
    *ptr2=mkdg_value_get_uint32(mValue);
}

static void md_uint32_set(MkdgValue *mValue, gpointer setValue){
    mValue->data[0].v_uint32=(setValue) ? *(guint32 *) setValue: 0;
}

static MkdgValue *md_uint32_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    if (G_UNLIKELY(mkdg_string_is_empty(str))){
	mkdg_value_set_uint32(mValue,0);
    }else{
	gint base=determine_base(str, parseOption, &startPtr);
	guint32 val=(guint32) strtol(startPtr, NULL, base);
	mkdg_value_set_uint32(mValue,val);
    }
    return mValue;
}

static gchar *md_uint32_to_string(MkdgValue *mValue, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%u";
    g_string_printf(strBuf, toStringFormat ,mkdg_value_get_uint32(mValue));
    return g_string_free(strBuf, FALSE);
}

static void md_int64_extract(MkdgValue *mValue, gpointer ptr){
    gint64 *ptr2=(gint64 *) ptr;
    *ptr2=mkdg_value_get_int64(mValue);
}

static void md_int64_set(MkdgValue *mValue, gpointer setValue){
    mValue->data[0].v_int64=(setValue) ? *(gint64 *) setValue: 0;
}

static MkdgValue *md_int64_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    if (G_UNLIKELY(mkdg_string_is_empty(str))){
	mkdg_value_set_int64(mValue,0);
    }else{
	gint base=determine_base(str, parseOption, &startPtr);
	gint64 val=(gint64) strtol(startPtr, NULL, base);
	mkdg_value_set_int64(mValue,val);
    }
    return mValue;
}

static gchar *md_int64_to_string(MkdgValue *mValue, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%d";
    g_string_printf(strBuf, toStringFormat ,mkdg_value_get_int64(mValue));
    return g_string_free(strBuf, FALSE);
}

static void md_uint64_extract(MkdgValue *mValue, gpointer ptr){
    guint64 *ptr2=(guint64 *) ptr;
    *ptr2=mkdg_value_get_uint64(mValue);
}

static void md_uint64_set(MkdgValue *mValue, gpointer setValue){
    mValue->data[0].v_uint64=(setValue) ? *(guint64 *) setValue: 0;
}

static MkdgValue *md_uint64_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    if (G_UNLIKELY(mkdg_string_is_empty(str))){
	mkdg_value_set_uint64(mValue,0);
    }else{
	gint base=determine_base(str, parseOption, &startPtr);
	guint64 val=(guint64) strtol(startPtr, NULL, base);
	mkdg_value_set_uint64(mValue,val);
    }
    return mValue;
}

static gchar *md_uint64_to_string(MkdgValue *mValue, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%u";
    g_string_printf(strBuf, toStringFormat ,mkdg_value_get_uint64(mValue));
    return g_string_free(strBuf, FALSE);
}

static void md_long_extract(MkdgValue *mValue, gpointer ptr){
    glong *ptr2=(glong *) ptr;
    *ptr2=mkdg_value_get_long(mValue);
}

static void md_long_set(MkdgValue *mValue, gpointer setValue){
    mValue->data[0].v_long=(setValue) ? *(glong *) setValue: 0;
}

static MkdgValue *md_long_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    if (G_UNLIKELY(mkdg_string_is_empty(str))){
	mkdg_value_set_long(mValue,0l);
    }else{
	gint base=determine_base(str,  parseOption, &startPtr);
	glong val= strtol(startPtr, NULL, base);
	mkdg_value_set_long(mValue,val);
    }
    return mValue;
}

static gchar *md_long_to_string(MkdgValue *mValue, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%ld";
    g_string_printf(strBuf, toStringFormat ,mkdg_value_get_long(mValue));
    return g_string_free(strBuf, FALSE);
}

static void md_ulong_extract(MkdgValue *mValue, gpointer ptr){
    gulong *ptr2=(gulong *) ptr;
    *ptr2=mkdg_value_get_ulong(mValue);
}

static void md_ulong_set(MkdgValue *mValue, gpointer setValue){
    mValue->data[0].v_ulong=(setValue) ? *(gulong *) setValue: 0;
}

static MkdgValue *md_ulong_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption){
    gchar *startPtr=NULL;
    if (G_UNLIKELY(mkdg_string_is_empty(str))){
	mkdg_value_set_ulong(mValue,0);
    }else{
	gint base=determine_base(str, parseOption, &startPtr);
	gulong val= (gulong) strtoll(startPtr, NULL, base);
	mkdg_value_set_ulong(mValue,val);
    }
    return mValue;
}

static gchar *md_ulong_to_string(MkdgValue *mValue, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%lu";
    g_string_printf(strBuf, toStringFormat ,mkdg_value_get_ulong(mValue));
    return g_string_free(strBuf, FALSE);
}

static void md_float_extract(MkdgValue *mValue, gpointer ptr){
    gfloat *ptr2=(gfloat *) ptr;
    *ptr2=mkdg_value_get_float(mValue);
}

static void md_float_set(MkdgValue *mValue, gpointer setValue){
    mValue->data[0].v_float=(setValue) ? *(gfloat *) setValue: 0.0f;
}

static MkdgValue *md_float_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption){
    gfloat val= (G_UNLIKELY(mkdg_string_is_empty(str))) ? 0.0f: (gfloat) strtod(str, NULL);
    mkdg_value_set_float(mValue,val);
    return mValue;
}

static gchar *md_float_to_string(MkdgValue *mValue, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%g";
    g_string_printf(strBuf, toStringFormat ,mkdg_value_get_float(mValue));
    return g_string_free(strBuf, FALSE);
}

static void md_double_extract(MkdgValue *mValue, gpointer ptr){
    gdouble *ptr2=(gdouble *) ptr;
    *ptr2=mkdg_value_get_double(mValue);
}

static void md_double_set(MkdgValue *mValue, gpointer setValue){
    mValue->data[0].v_double=(setValue) ? *(gdouble *) setValue: 0.0;
}

static MkdgValue *md_double_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption){
    gdouble val= (G_UNLIKELY(mkdg_string_is_empty(str))) ? 0.0 : (gdouble) strtod(str, NULL);
    mkdg_value_set_double(mValue,val);
    return mValue;
}

static gchar *md_double_to_string(MkdgValue *mValue, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%g";
    g_string_printf(strBuf, toStringFormat ,mkdg_value_get_double(mValue));
    return g_string_free(strBuf, FALSE);
}
/*=== End number type ===*/
/*=== Start string type ===*/
static void md_string_extract(MkdgValue *mValue, gpointer ptr){
    gchar **ptr2=(gchar **) ptr;
    *ptr2=g_strdup(mkdg_value_get_string(mValue));
}

static void md_string_set(MkdgValue *mValue, gpointer setValue){
    if (mValue->flags & MKDG_VALUE_FLAG_NEED_FREE ){
	mValue->data[0].v_string= g_strdup( (setValue) ? (gchar *) setValue : "");
    }else{
	mValue->data[0].v_string= (setValue) ? setValue : "";
    }
}

static MkdgValue *md_string_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption){
    gchar *strValue= g_strdup((G_UNLIKELY(mkdg_string_is_empty(str))) ? "" : str);
    mkdg_value_set_string(mValue, strValue);
    return mValue;
}

static gchar *md_string_to_string(MkdgValue *mValue, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    if (!toStringFormat)
	toStringFormat="%s";
    g_string_printf(strBuf, toStringFormat ,mkdg_value_get_string(mValue));
    return g_string_free(strBuf, FALSE);
}

#define STRING_COMPARE_CASE_INSENSITIVE_FLAG	0x1
#define STRING_COMPARE_CASE_INSENSITIVE_OPTSTR  "icase"
static gint md_string_compare(MkdgValue *mValue1, MkdgValue *mValue2, const gchar *compareOption){
    if (mValue2->mType!=MKDG_TYPE_STRING){
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
    gchar *str1=mkdg_value_get_string(mValue1);
    gchar *str2=mkdg_value_get_string(mValue2);
    MKDG_DEBUG_MSG(7, "[I7] md_string_compare() str1=%s str2=%s",str1, str2);
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

static void md_string_free(MkdgValue *mValue){
    if (mValue->flags  & MKDG_VALUE_FLAG_NEED_FREE){
	g_free(mValue->data[0].v_string);
    }
}
/*=== End string type ===*/
/*=== Start string list type ===*/
static gchar *emptyStrList[]={"", NULL};

static void md_string_list_extract(MkdgValue *mValue, gpointer ptr){
    gchar ***ptr2=(gchar ***) ptr;
    *ptr2=g_strdupv(mkdg_value_get_string_list(mValue));
}

static void md_string_list_set(MkdgValue *mValue, gpointer setValue){
    if (mValue->flags & MKDG_VALUE_FLAG_NEED_FREE ){
	mValue->data[0].v_string_list= g_strdupv( (setValue) ? (gchar **) setValue : emptyStrList);
    }else{
	mValue->data[0].v_string_list= (setValue) ? (gchar **) setValue : emptyStrList;
    }
}

static MkdgValue *md_string_list_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption){
    if (!str){
	mkdg_value_set_string_list(mValue, NULL);
	return mValue;
    }
    const gchar *delimiters=(mkdg_string_is_empty(parseOption))? ";": parseOption;
    gchar **sList=mkdg_string_split_set(str,delimiters, '\\', TRUE, -1);
    mkdg_value_set_string_list(mValue, sList);
    gint i;
    for(i=0;sList[i]!=NULL;i++);
    // Count number of string length.
    mValue->data[1].v_int=i;
    return mValue;
}

static gchar *md_string_list_to_string(MkdgValue *mValue, const gchar *toStringFormat){
    return mkdg_string_list_combine(mkdg_value_get_string_list(mValue), ";",'\\', TRUE);
}

static gint md_string_list_compare(MkdgValue *mValue1, MkdgValue *mValue2, const gchar *compareOption){
    if (mValue2->mType!=MKDG_TYPE_STRING_LIST){
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
    gchar **strList1=mkdg_value_get_string_list(mValue1);
    gchar **strList2=mkdg_value_get_string_list(mValue2);
    if (strList1==NULL){
	if (strList2==NULL)
	    return 0;
	return -1;
    }
    if (strList2==NULL){
	return 1;
    }

    gint i=0;
    if (mValue1->data[1].v_int < mValue2->data[1].v_int){
	return -1;
    }else if (mValue1->data[1].v_int > mValue2->data[1].v_int){
	return 1;
    }else{
	for(i=0;strList2[i]!=NULL;i++){
	    if (strList1[i]==NULL)
		return -1;
	    if (flags & STRING_COMPARE_CASE_INSENSITIVE_FLAG){
		ret=g_ascii_strcasecmp(strList1[i], strList2[i]);
	    }else{
		ret=strcmp(strList1[i], strList2[i]);
	    }
	    if (ret>0)
		return 1;
	    else if (ret<0)
		return -1;
	}
    }
    if (strList1[i]!=NULL)
	return 1;
    return 0;
}

static void md_string_list_free(MkdgValue *mValue){
    if (mValue->flags  & MKDG_VALUE_FLAG_NEED_FREE){
	g_strfreev(mValue->data[0].v_string_list);
    }
}
/*=== End string list type ===*/
/*=== Start color type ===*/
static void md_color_extract(MkdgValue *mValue, gpointer ptr){
    MkdgColor *ptr2=(MkdgColor *) ptr;
    *ptr2=mkdg_value_get_color(mValue);
}

static void md_color_set(MkdgValue *mValue, gpointer setValue){
    mkdg_value_set_color(mValue, (setValue) ? *(guint32 *) setValue: 0);
}

static MkdgValue *md_color_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption){
    if (G_UNLIKELY(mkdg_string_is_empty(str))){
	/* Default is black */
	mkdg_value_set_color(mValue, 0);
    }else{
	if (str[0]=='#'){
	    mkdg_value_set_color(mValue, (MkdgColor) strtol(str+sizeof(gchar), NULL, 16));
	}else{
	    gint index=find_color_index_by_name(str);
	    if (index>=0){
		mkdg_value_set_color(mValue, mkdgColorList[index].value);
	    }else{
		/* Default is black */
		mkdg_value_set_color(mValue, 0);
	    }
	}
    }
    return mValue;
}

static gchar *md_color_to_string(MkdgValue *mValue, const gchar *toStringFormat){
    GString *strBuf=g_string_new(NULL);
    gchar buf[12];
    if (!toStringFormat)
	toStringFormat="%s";
    MkdgColor color=mkdg_value_get_color(mValue);
    gint index=find_color_index_by_value(color);
    MKDG_DEBUG_MSG(5,"[I5] md_color_to_string(): color=%X index=%d",color,index);
    if (index>=0){
	g_string_printf(strBuf, toStringFormat, mkdgColorList[index].name);
    }else{
	g_snprintf(buf,12,"#%X", color);
	g_string_printf(strBuf, toStringFormat, buf);
    }
    return g_string_free(strBuf, FALSE);
}

static gint md_color_compare(MkdgValue *mValue1, MkdgValue *mValue2, const gchar *compareOption){
    if (G_VALUE_TYPE(mValue2)!=MKDG_TYPE_COLOR){
	return -3;
    }
    MkdgColor v1=mkdg_value_get_color(mValue1);
    MkdgColor v2=mkdg_value_get_color(mValue2);
    return (v1==v2) ? 0 : (v1>v2)? 1: -1;
}

const MkdgTypeInterfaceMkdgType mkdgTypeInterfaces[]={
    { MKDG_TYPE_POINTER,	"POINTER",
	{md_pointer_extract,		md_pointer_set,
	    NULL, 	NULL,
	    md_pointer_compare,	NULL}},
    { MKDG_TYPE_BOOLEAN,	"BOOLEAN",
	{md_boolean_extract,		md_boolean_set,
	    md_boolean_from_string, 	md_boolean_to_string,
	    md_boolean_compare,	NULL}},
    { MKDG_TYPE_INT,		"INT",
	{md_int_extract,		md_int_set,
	    md_int_from_string, 	md_int_to_string,
	    md_number_compare,	NULL}},
    { MKDG_TYPE_UINT,		"UINT",
	{md_uint_extract,		md_uint_set,
	    md_uint_from_string, 	md_uint_to_string,
	    md_number_compare,	NULL}},
    { MKDG_TYPE_INT32,		"INT32",
	{md_int32_extract,		md_int32_set,
	    md_int32_from_string, 	md_int32_to_string,
	    md_number_compare,	NULL}},
    { MKDG_TYPE_UINT32,		"UINT32",
	{md_uint32_extract,		md_uint32_set,
	    md_uint32_from_string, 	md_uint32_to_string,
	    md_number_compare,	NULL}},
    { MKDG_TYPE_INT64,		"INT64",
	{md_int64_extract,		md_int64_set,
	    md_int64_from_string, 	md_int64_to_string,
	    md_number_compare,	NULL}},
    { MKDG_TYPE_UINT64,		"UINT64",
	{md_uint64_extract,		md_uint64_set,
	    md_uint64_from_string, 	md_uint64_to_string,
	    md_number_compare,	NULL}},
    { MKDG_TYPE_LONG,		"LONG",
	{md_long_extract,		md_long_set,
	    md_long_from_string, 	md_long_to_string,
	    md_number_compare,	NULL}},
    { MKDG_TYPE_ULONG,		"ULONG",
	{md_ulong_extract,		md_ulong_set,
	    md_ulong_from_string, 	md_ulong_to_string,
	    md_number_compare,	NULL}},
    { MKDG_TYPE_FLOAT,		"FLOAT",
	{md_float_extract,		md_float_set,
	    md_float_from_string, 	md_float_to_string,
	    md_number_compare,	NULL}},
    { MKDG_TYPE_DOUBLE,		"DOUBLE",
	{md_double_extract,		md_double_set,
	    md_double_from_string, 	md_double_to_string,
	    md_number_compare,	NULL}},
    { MKDG_TYPE_STRING,		"STRING",
	{md_string_extract,		md_string_set,
	    md_string_from_string, 	md_string_to_string,
	    md_string_compare,		md_string_free}},
    { MKDG_TYPE_STRING_LIST,	"STRING_LIST",
	{md_string_list_extract,		md_string_list_set,
	    md_string_list_from_string, 	md_string_list_to_string,
	    md_string_list_compare,	md_string_list_free}},
    { MKDG_TYPE_COLOR,		"COLOR",
	{md_color_extract,		md_color_set,
	    md_color_from_string, 	md_color_to_string,
	    md_color_compare,	NULL}},
    { MKDG_TYPE_NONE,		"NONE",
	{NULL,			NULL,			NULL,
	    NULL,			NULL}},
};

static const MkdgTypeInterface *mkdg_find_type_interface(MkdgType mType){
    if (mType<0 || mType>MKDG_TYPE_NONE){
	return NULL;
    }
    return &mkdgTypeInterfaces[mType].typeInterface;
}

/*=== End Type Interface functions ===*/
MkdgType mkdg_type_parse(const gchar *str){
    MkdgType mType;
    for(mType=0; mType<=MKDG_TYPE_NONE; mType++){
	if (g_ascii_strcasecmp(str,mkdgTypeInterfaces[mType].name)==0)
	    return mType;
    }
    return MKDG_TYPE_INVALID;
}

const gchar *mkdg_type_to_string(MkdgType mType){
    if (mType<0 || mType>MKDG_TYPE_NONE){
	return NULL;
    }
    return mkdgTypeInterfaces[mType].name;
}

static void mkdg_value_set_private(MkdgValue *mValue, gpointer setValue, const MkdgTypeInterface *typeInterface){
    if (mkdg_type_is_pointer(mValue->mType)){
	if (mValue->data[0].v_pointer && (mValue->flags & MKDG_VALUE_FLAG_NEED_FREE)){
	    /* Free old data */
	    typeInterface->free(mValue);
	}
    }
    typeInterface->set(mValue, setValue);
}

MkdgValue *mkdg_value_new(MkdgType mType, gpointer setValue){
    const MkdgTypeInterface *typeInterface=mkdg_find_type_interface(mType);
    if (!typeInterface)
	return NULL;
    MkdgValue *mValue=g_new0(MkdgValue, 1);
    mValue->flags=0;
    mValue->mType=mType;
    if (mkdg_type_is_pointer(mValue->mType)){
	mValue->data[0].v_pointer=NULL;
	mValue->flags |= MKDG_VALUE_FLAG_NEED_FREE;
    }
    mkdg_value_set_private(mValue, setValue, typeInterface);
    return mValue;
}

MkdgValue *mkdg_value_new_static(MkdgType mType, gpointer setValue){
    const MkdgTypeInterface *typeInterface=mkdg_find_type_interface(mType);
    if (!typeInterface)
	return NULL;
    MkdgValue *mValue=g_new0(MkdgValue, 1);
    mValue->flags=0;
    mValue->mType=mType;
    if (mkdg_type_is_pointer(mValue->mType)){
	mValue->data[0].v_pointer=NULL;
    }
    mkdg_value_set_private(mValue, setValue, typeInterface);
    return mValue;
}

gboolean mkdg_value_copy(MkdgValue *srcValue, MkdgValue *destValue){
    if (srcValue->mType!=destValue->mType){
	return FALSE;
    }
    if (mkdg_type_is_pointer(srcValue->mType)){
	mkdg_value_set(destValue, srcValue->data[0].v_pointer);
    }else{
	destValue->data[0]=srcValue->data[0];
	destValue->data[1]=srcValue->data[1];
    }
    return TRUE;
}

void mkdg_value_extract(MkdgValue *mValue, gpointer ptr){
    const MkdgTypeInterface *typeInterface=mkdg_find_type_interface(mValue->mType);
    typeInterface->extract(mValue, ptr);
}

void mkdg_value_set(MkdgValue *mValue, gpointer setValue){
    const MkdgTypeInterface *typeInterface=mkdg_find_type_interface(mValue->mType);
    mkdg_value_set_private(mValue, setValue, typeInterface);
}

void mkdg_value_free(gpointer mValue){
    MkdgValue *mV=(MkdgValue *) mValue;
    const MkdgTypeInterface *typeInterface=mkdg_find_type_interface(mV->mType);
    if (mV->flags  & MKDG_VALUE_FLAG_NEED_FREE){
	typeInterface->free(mV);
    }
    g_free(mV);
}

gboolean mkdg_type_is_pointer(MkdgType mType){
    switch (mType){
	case MKDG_TYPE_POINTER:
	case MKDG_TYPE_STRING:
	case MKDG_TYPE_STRING_LIST:
	    return TRUE;
	default:
	    break;
    }
    return FALSE;
}

gboolean mkdg_type_is_number(MkdgType mType){
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

void mkdg_value_from_double(MkdgValue *value, gdouble number){
    switch (value->mType){
	case MKDG_TYPE_INT:
	    mkdg_value_set_int(value, (gint) number);
	    break;
	case MKDG_TYPE_UINT:
	    mkdg_value_set_uint(value, (guint) number);
	    break;
	case MKDG_TYPE_INT32:
	    mkdg_value_set_int32(value, (gint32) number);
	    break;
	case MKDG_TYPE_UINT32:
	    mkdg_value_set_uint32(value, (guint32) number);
	    break;
	case MKDG_TYPE_INT64:
	    mkdg_value_set_int64(value, (gint64) number);
	    break;
	case MKDG_TYPE_UINT64:
	    mkdg_value_set_uint64(value, (guint64) number);
	    break;
	case MKDG_TYPE_LONG:
	    mkdg_value_set_long(value, (glong) number);
	    break;
	case MKDG_TYPE_ULONG:
	    mkdg_value_set_ulong(value, (gulong) number);
	    break;
	case MKDG_TYPE_FLOAT:
	    mkdg_value_set_float(value, (gfloat) number);
	    break;
	case MKDG_TYPE_DOUBLE:
	    mkdg_value_set_double(value, (gdouble) number);
	    break;
	case MKDG_TYPE_COLOR:
	    mkdg_value_set_color(value, (guint32) number);
	    break;
	default:
	    break;
    }
}

gdouble mkdg_value_to_double(MkdgValue *value){
    switch (value->mType){
	case MKDG_TYPE_INT:
	    return (gdouble) mkdg_value_get_int(value);
	case MKDG_TYPE_UINT:
	    return (gdouble) mkdg_value_get_uint(value);
	case MKDG_TYPE_INT32:
	    return (gdouble) mkdg_value_get_int32(value);
	case MKDG_TYPE_UINT32:
	    return (gdouble) mkdg_value_get_uint32(value);
	case MKDG_TYPE_INT64:
	    return (gdouble) mkdg_value_get_int64(value);
	case MKDG_TYPE_UINT64:
	    return (gdouble) mkdg_value_get_uint64(value);
	case MKDG_TYPE_LONG:
	    return (gdouble) mkdg_value_get_long(value);
	case MKDG_TYPE_ULONG:
	    return (gdouble) mkdg_value_get_ulong(value);
	case MKDG_TYPE_FLOAT:
	    return (gdouble) mkdg_value_get_float(value);
	case MKDG_TYPE_DOUBLE:
	    return (gdouble) mkdg_value_get_double(value);
	case MKDG_TYPE_COLOR:
	    return (gdouble) mkdg_value_get_double(value);
	default:
	    break;
    }
    return  0.0;
}

MkdgValue *mkdg_value_from_string(MkdgValue *mValue, const gchar *str, const gchar *parseOption){
    const MkdgTypeInterface *typeInterface=mkdg_find_type_interface(mValue->mType);
    if (!typeInterface)
	return NULL;
    if (mkdg_type_is_pointer(mValue->mType)){
	if (mValue->data[0].v_pointer && (mValue->flags  & MKDG_VALUE_FLAG_NEED_FREE)){
	    /* Free old data */
	    typeInterface->free(mValue);
	}
    }
    typeInterface->from_string(mValue, str, parseOption);
    return mValue;
}

gchar *mkdg_value_to_string(MkdgValue *mValue, const gchar *toStringFormat){
    const MkdgTypeInterface *typeInterface=mkdg_find_type_interface(mValue->mType);
    if (!typeInterface)
	return NULL;
    return typeInterface->to_string(mValue, toStringFormat);
}

gchar *mkdg_string_convert(const gchar *str, MkdgType mType, const gchar *parseOption, const gchar *toStringFormat){
    MkdgValue *mValue=mkdg_value_new(mType,NULL);
    mkdg_value_from_string(mValue, str, parseOption);
    gchar *result=mkdg_value_to_string(mValue, toStringFormat);
    mkdg_value_free(mValue);
    return result;
}

gchar *mkdg_string_normalized(const gchar *str, MkdgType mType){
    return mkdg_string_convert(str, mType, NULL, NULL);
}

gint mkdg_value_compare(MkdgValue *mValue1, MkdgValue *mValue2, const gchar *compareOption){
    const MkdgTypeInterface *typeInterface=mkdg_find_type_interface(mValue2->mType);
    if (!typeInterface)
	return -2;
    typeInterface=mkdg_find_type_interface(mValue1->mType);
    if (!typeInterface)
	return -2;
    return typeInterface->compare(mValue1,mValue2, compareOption);
}


