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
#include "MakerDialogUtil.h"

gint makerDialogVerboseLevel=0;

void MAKER_DIALOG_DEBUG_MSG(gint level, const gchar *format, ...){
    va_list ap;
    if (level<makerDialogVerboseLevel){
	va_start(ap, format);
	g_logv("MakerDialog",G_LOG_LEVEL_DEBUG, format, ap);
	va_end(ap);
    }
}

static gboolean string_is_empty(const gchar *str){
    if (!str)
    	return TRUE;
    if (str[0]=='\0')
	return TRUE;
    return FALSE;
}

gboolean maker_dialog_atob(const gchar *string){
    if (string_is_empty(string))
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

gboolean maker_dialog_g_value_is_number(GValue *value){
    if (G_VALUE_TYPE(value)==G_TYPE_INT)
	return TRUE;
    if (G_VALUE_TYPE(value)==G_TYPE_UINT)
	return TRUE;
    if (G_VALUE_TYPE(value)==G_TYPE_LONG)
	return TRUE;
    if (G_VALUE_TYPE(value)==G_TYPE_ULONG)
	return TRUE;
    if (G_VALUE_TYPE(value)==G_TYPE_FLOAT)
	return TRUE;
    if (G_VALUE_TYPE(value)==G_TYPE_DOUBLE)
	return TRUE;
    return FALSE;
}

gdouble maker_dialog_g_value_get_double(GValue *value){
    if (G_VALUE_TYPE(value)==G_TYPE_INT)
	return (gdouble) g_value_get_int(value);
    if (G_VALUE_TYPE(value)==G_TYPE_UINT)
	return (gdouble) g_value_get_uint(value);
    if (G_VALUE_TYPE(value)==G_TYPE_LONG)
	return (gdouble) g_value_get_long(value);
    if (G_VALUE_TYPE(value)==G_TYPE_ULONG)
	return (gdouble) g_value_get_ulong(value);
    if (G_VALUE_TYPE(value)==G_TYPE_FLOAT)
	return (gdouble) g_value_get_float(value);
    if (G_VALUE_TYPE(value)==G_TYPE_DOUBLE)
	return (gdouble) g_value_get_double(value);
    return  0.0;
}

/*=== Start Type Handler functions ===*/
typedef struct{
    GType type;
    GValue *(* from_string) (GValue *value, const gchar *str, gpointer option);
    gchar *(* to_string) (GValue *value, const gchar *formatStr);
    gint (* compare) (GValue *value1, GValue *value2, MakerDialogCompareFunc func);
} TypeHandler;

static GValue *md_boolean_from_string(GValue *value, const gchar *str, gpointer option){
    gboolean val=maker_dialog_atob(str);
    g_value_set_boolean(value,val);
    return value;
}

static gchar *md_boolean_to_string(GValue *value, const gchar *formatStr){
    return g_strdup((g_value_get_boolean(value))? "TRUE" : "FALSE");
}

static gint md_boolean_compare(GValue *value1, GValue *value2, MakerDialogCompareFunc func){
    if (G_VALUE_TYPE(value2)!=G_TYPE_BOOLEAN){
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
    if (!maker_dialog_g_value_is_number(value2)){
	return -3;
    }
    gdouble val1=maker_dialog_g_value_get_double(value1);
    gdouble val2=maker_dialog_g_value_get_double(value2);
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

static gint determine_base(const gchar *str, gpointer option, gchar **startPtr){
    gint base=10;
    gint *base_ptr= (gint *) option;
    *startPtr=(gchar *) str;
    if (base_ptr){
	base=*base_ptr;
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

static GValue *md_int_from_string(GValue *value, const gchar *str, gpointer option){
    gchar *startPtr=NULL;
    gint base=determine_base(str, option, &startPtr);
    gint val=(gint) strtol(startPtr, NULL, base);
    g_value_set_int(value,val);
    return value;
}

static gchar *md_int_to_string(GValue *value, const gchar *formatStr){
    GString *strBuf=g_string_new(NULL);
    if (!formatStr)
	formatStr="%d";
    g_string_printf(strBuf, formatStr ,g_value_get_int(value));
    return g_string_free(strBuf, FALSE);
}

static GValue *md_uint_from_string(GValue *value, const gchar *str, gpointer option){
    gchar *startPtr=NULL;
    gint base=determine_base(str, option, &startPtr);
    guint val=(guint) strtol(startPtr, NULL, base);
    g_value_set_uint(value,val);
    return value;
}

static gchar *md_uint_to_string(GValue *value, const gchar *formatStr){
    GString *strBuf=g_string_new(NULL);
    if (!formatStr)
	formatStr="%u";
    g_string_printf(strBuf, formatStr ,g_value_get_uint(value));
    return g_string_free(strBuf, FALSE);
}


static GValue *md_long_from_string(GValue *value, const gchar *str, gpointer option){
    gchar *startPtr=NULL;
    gint base=determine_base(str, option, &startPtr);
    glong val= strtol(startPtr, NULL, base);
    g_value_set_long(value,val);
    return value;
}

static gchar *md_long_to_string(GValue *value, const gchar *formatStr){
    GString *strBuf=g_string_new(NULL);
    if (!formatStr)
	formatStr="%ld";
    g_string_printf(strBuf, formatStr ,g_value_get_long(value));
    return g_string_free(strBuf, FALSE);
}

static GValue *md_ulong_from_string(GValue *value, const gchar *str, gpointer option){
    gchar *startPtr=NULL;
    gint base=determine_base(str, option, &startPtr);
    gulong val= (gulong) strtoll(startPtr, NULL, base);
    g_value_set_ulong(value,val);
    return value;
}

static gchar *md_ulong_to_string(GValue *value, const gchar *formatStr){
    GString *strBuf=g_string_new(NULL);
    if (!formatStr)
	formatStr="%lu";
    g_string_printf(strBuf, formatStr ,g_value_get_ulong(value));
    return g_string_free(strBuf, FALSE);
}

static GValue *md_float_from_string(GValue *value, const gchar *str, gpointer option){
    gfloat val= (gfloat) strtod(str, NULL);
    g_value_set_float(value,val);
    return value;
}

static gchar *md_float_to_string(GValue *value, const gchar *formatStr){
    GString *strBuf=g_string_new(NULL);
    if (!formatStr)
	formatStr="%g";
    g_string_printf(strBuf, formatStr ,g_value_get_float(value));
    return g_string_free(strBuf, FALSE);
}

static GValue *md_double_from_string(GValue *value, const gchar *str, gpointer option){
    gdouble val= strtod(str, NULL);
    g_value_set_double(value,val);
    return value;
}

static gchar *md_double_to_string(GValue *value, const gchar *formatStr){
    GString *strBuf=g_string_new(NULL);
    if (!formatStr)
	formatStr="%g";
    g_string_printf(strBuf, formatStr ,g_value_get_double(value));
    return g_string_free(strBuf, FALSE);
}

static GValue *md_string_from_string(GValue *value, const gchar *str, gpointer option){
    g_value_set_string(value, str);
    return value;
}

static gchar *md_string_to_string(GValue *value, const gchar *formatStr){
    GString *strBuf=g_string_new(NULL);
    if (!formatStr)
	formatStr="%s";
    g_string_printf(strBuf, formatStr ,g_value_get_string(value));
    return g_string_free(strBuf, FALSE);
}

static gint md_string_compare(GValue *value1, GValue *value2, MakerDialogCompareFunc func){
    if (G_VALUE_TYPE(value2)!=G_TYPE_STRING){
	return -3;
    }
    if (func){
	return func((gchar *)g_value_get_string(value1), (gchar *) g_value_get_string(value2));
    }
    return strcmp((gchar *) g_value_get_string(value1), (gchar *) g_value_get_string(value2));
}

const TypeHandler typeHandlers[]={
    { G_TYPE_BOOLEAN, 	md_boolean_from_string,	md_boolean_to_string,	md_boolean_compare},
    { G_TYPE_INT,	md_int_from_string,	md_int_to_string,	md_number_compare},
    { G_TYPE_UINT,	md_uint_from_string,	md_uint_to_string, 	md_number_compare},
    { G_TYPE_LONG,	md_long_from_string,	md_long_to_string,	md_number_compare},
    { G_TYPE_ULONG,	md_ulong_from_string,	md_ulong_to_string,	md_number_compare},
    { G_TYPE_FLOAT,	md_float_from_string,	md_float_to_string, 	md_number_compare},
    { G_TYPE_DOUBLE,	md_double_from_string,	md_double_to_string, 	md_number_compare},
    { G_TYPE_STRING,	md_string_from_string,	md_string_to_string, 	md_string_compare},
    { G_TYPE_INVALID,	NULL,			NULL,			NULL},

};

static const TypeHandler *maker_dialog_find_type_handler(GType type){
    gsize i;
    for(i=0;typeHandlers[i].type!=G_TYPE_INVALID;i++){
	if (typeHandlers[i].type==type){
	    return &typeHandlers[i];
	}
    }
    return NULL;
}
/*=== End Type Handler functions ===*/

GValue *maker_dialog_g_value_from_string(GValue *value, const gchar *str, gpointer option){
    if (!str)
	return NULL;
    const TypeHandler *typeHandler=maker_dialog_find_type_handler(G_VALUE_TYPE(value));
    if (!typeHandler)
	return NULL;
    typeHandler->from_string(value, str, option);
    return value;
}

gchar *maker_dialog_g_value_to_string(GValue *value, const gchar *formatStr){
    const TypeHandler *typeHandler=maker_dialog_find_type_handler(G_VALUE_TYPE(value));
    if (!typeHandler)
	return NULL;
    return typeHandler->to_string(value, formatStr);
}

gint maker_dialog_g_value_compare(GValue *value1, GValue *value2, gpointer option){
    const TypeHandler *typeHandler=maker_dialog_find_type_handler(G_VALUE_TYPE(value2));
    if (!typeHandler)
	return -2;
    typeHandler=maker_dialog_find_type_handler(G_VALUE_TYPE(value1));
    if (!typeHandler)
	return -2;
    return typeHandler->compare(value1,value2, option);
}



