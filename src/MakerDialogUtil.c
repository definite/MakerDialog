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
#include <libgen.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <glib.h>
#include <glib/gstdio.h>
#include "MakerDialogUtil.h"

#ifndef G_LOG_DOMAIN
#define G_LOG_DOMAIN "MakerDialog"
#endif

gint makerDialogVerboseLevel=0;

gboolean MAKER_DIALOG_DEBUG_RUN(gint level){
    return (level<=makerDialogVerboseLevel)? TRUE : FALSE;
}

void MAKER_DIALOG_DEBUG_MSG(gint level, const gchar *format, ...){
    va_list ap;
    if (level<=makerDialogVerboseLevel){
	va_start(ap, format);
	g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, format, ap);
	va_end(ap);
    }
}

/*=== Start Type Interface functions ===*/


gboolean maker_dialog_atob(const gchar *string){
    if (maker_dialog_string_is_empty(string))
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

gboolean maker_dialog_string_is_empty(const gchar *str){
    if (!str)
	return TRUE;
    if (str[0]=='\0')
	return TRUE;
    return FALSE;
}

void maker_dialog_g_value_free(gpointer value){
    GValue *gValue=(GValue  *) value;
    g_value_unset(gValue);
    g_free(gValue);
}

gboolean maker_dialog_g_type_is_number(GType type){
    switch (type){
	case G_TYPE_INT:
	case G_TYPE_UINT:
	case G_TYPE_LONG:
	case G_TYPE_ULONG:
	case G_TYPE_FLOAT:
	case G_TYPE_DOUBLE:
	    return TRUE;
	default:
	    break;
    }
    return FALSE;
}

gdouble maker_dialog_g_value_to_double(GValue *value){
    switch (G_VALUE_TYPE(value)){
	case G_TYPE_INT:
	    return (gdouble) g_value_get_int(value);
	case G_TYPE_UINT:
	    return (gdouble) g_value_get_uint(value);
	case G_TYPE_LONG:
	    return (gdouble) g_value_get_long(value);
	case G_TYPE_ULONG:
	    return (gdouble) g_value_get_ulong(value);
	case G_TYPE_FLOAT:
	    return (gdouble) g_value_get_float(value);
	case G_TYPE_DOUBLE:
	    return (gdouble) g_value_get_double(value);
	default:
	    break;
    }
    return  0.0;
}

void maker_dialog_g_value_from_double(GValue *value, gdouble number){
    switch (G_VALUE_TYPE(value)){
    	case G_TYPE_INT:
	    g_value_set_int(value, (gint) number);
	    break;
	case G_TYPE_UINT:
	    g_value_set_uint(value, (guint) number);
	    break;
	case G_TYPE_LONG:
	    g_value_set_long(value, (glong) number);
	    break;
	case G_TYPE_ULONG:
	    g_value_set_ulong(value, (gulong) number);
	    break;
	case G_TYPE_FLOAT:
	    g_value_set_float(value, (gfloat) number);
	    break;
	case G_TYPE_DOUBLE:
	    g_value_set_double(value, (gdouble) number);
	    break;
	default:
	    break;
    }
}


gboolean maker_dialog_has_all_flags(guint flagSet, guint specFlags){
    return ((~(flagSet & specFlags)) & specFlags)? FALSE : TRUE;
}

/*=== Start General file functions ===*/
gboolean maker_dialog_file_isWritable(const gchar *filename){
    gchar parentDirBuf[PATH_MAX];
    gchar *parentDir;
    gboolean result=TRUE;

    if (g_access(filename,W_OK)!=0){
	if (g_access(filename,F_OK)==0){
	    // Read only.
	    return FALSE;
	}
	// Can't write the file , test whether the parent director can write
	g_strlcpy(parentDirBuf,filename,PATH_MAX);
	parentDir=dirname(parentDirBuf);
	if (g_access(parentDir,W_OK)!=0){
	    result=FALSE;
	}
    }
    return result;
}

gchar* maker_dialog_truepath(const gchar *path, gchar *resolved_path){
    gchar workingPath[PATH_MAX];
    gchar fullPath[PATH_MAX];
    gchar *result=NULL;
    g_strlcpy(workingPath,path,PATH_MAX);

    if ( workingPath[0] != '~' ){
	result = realpath(workingPath, resolved_path);
    }else{
	gchar *firstSlash, *suffix, *homeDirStr;
	struct passwd *pw;

	// initialize variables
	firstSlash = suffix = homeDirStr = NULL;

	firstSlash = strchr(workingPath, DIRECTORY_SEPARATOR);
	if (firstSlash == NULL)
	    suffix = "";
	else
	{
	    *firstSlash = 0;    // so userName is null terminated
	    suffix = firstSlash + 1;
	}

	if (workingPath[1] == '\0')
	    pw = getpwuid( getuid() );
	else
	    pw = getpwnam( &workingPath[1] );

	if (pw != NULL)
	    homeDirStr = pw->pw_dir;

	if (homeDirStr != NULL){
	    gint ret=g_sprintf(fullPath, "%s%c%s", homeDirStr, DIRECTORY_SEPARATOR, suffix);
	    if (ret>0){
		result = realpath(fullPath, resolved_path);
	    }

	}
    }
    return result;
}

