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

void maker_dialog_error_print(MakerDialogError *error){
    if (MAKER_DIALOG_DEBUG_RUN(0)){
	g_warning("[WW] domain:%s [%d] %s", g_quark_to_string(error->domain), error->code, error->message);
    }
}

void maker_dialog_error_print_with_prefix(const gchar *prefix,MakerDialogError *error){
    if (MAKER_DIALOG_DEBUG_RUN(0)){
	g_warning("[WW] %s%sdomain:%s [%d] %s",
		(prefix)? prefix : "",
		(prefix)? " " : "",
		g_quark_to_string(error->domain), error->code, error->message);
    }
}

gboolean maker_dialog_error_handle(MakerDialogError *errIn, MakerDialogError **errOut){
    if (errIn){
	if (errOut){
	    if (*errOut){
		maker_dialog_error_print(*errOut);
		g_error_free(*errOut);
	    }
	    *errOut=errIn;
	}else{
	    maker_dialog_error_print(errIn);
	    g_error_free(errIn);
	    errIn=NULL;
	}
	return TRUE;
    }
    return FALSE;
}

MakerDialogIdDataPair *maker_dialog_id_parse(MakerDialogIdDataPair *pairedData, const gchar *str, gboolean caseSensitive){
    gint i=0,ret;
    for(i=0; pairedData[i].strId!=NULL;i++){
	if (caseSensitive){
	    ret=strcmp(str, pairedData[i].strId);
	}else{
	    ret=g_ascii_strcasecmp(str, pairedData[i].strId);
	}
	if (ret==0)
	    return &pairedData[i];
    }
    return &pairedData[i];
}

guint32 maker_dialog_flag_parse(MakerDialogIdDataPair *pairedData, const gchar *str, gboolean caseSensitive){
    gchar **flagList=maker_dialog_string_split_set(str, " \t|;", '\\', FALSE, -1);
    guint32 flags=0;
    gint i;
    for(i=0; flagList[i]!=NULL;i++){
	MakerDialogIdDataPair *dataPair=maker_dialog_id_parse(pairedData, flagList[i], caseSensitive);
	flags|=dataPair->data.v_uint32;
    }
    return flags;
}

gint maker_dialog_find_string(const gchar *str, gchar **strlist, gint max_find){
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

static gint maker_dialog_string_char_at(const gchar *str, gchar ch){
    gint i;
    for(i=0; str[i]!='\0'; i++){
	if (str[i]==ch)
	    return i;
    }
    return -1;
}

gchar **maker_dialog_string_split_set
(const gchar *str, const gchar *delimiters, gchar escapeChar, gboolean emptyToken, gint maxTokens){
    g_return_val_if_fail (str != NULL, NULL);
    g_return_val_if_fail (delimiters != NULL, NULL);
    GPtrArray *ptrArray=g_ptr_array_new();
    gint tokenCount=0;
    gchar *chPtr=(gchar *)str;
    GString *strBuf=g_string_new("");
    while(*chPtr!='\0'){
	if (*chPtr==escapeChar){
	    chPtr++;
	}else if (maker_dialog_string_char_at(delimiters, *chPtr)>=0){
	    if (maxTokens<0 || tokenCount<maxTokens-1){
		/* Can have more tokens */
		if (emptyToken || strBuf->len){
		    /* A valid token */
		    g_ptr_array_add(ptrArray,g_string_free(strBuf,FALSE));
		    tokenCount++;
		    strBuf=g_string_new("");
		}
		chPtr++;
		continue;
	    }
	}
	gunichar wch=g_utf8_get_char(chPtr);
	if (wch){
	    g_string_append_unichar(strBuf,wch);
	    chPtr=g_utf8_next_char(chPtr);
	}else{
	    g_warning("maker_dialog_string_split_set(): Invalid format. str=%s",str);
	}
    }

    /* Pick up the rest of the string */
    g_assert(strBuf);
    if (emptyToken || strBuf->len){
	/* A valid token */
	g_ptr_array_add(ptrArray,g_string_free(strBuf,FALSE));
	tokenCount++;
    }else{
	g_string_free(strBuf,TRUE);
    }
    g_ptr_array_add(ptrArray, NULL);
    return (gchar **) g_ptr_array_free(ptrArray, FALSE);
}

gchar *maker_dialog_string_list_combine
(const gchar **strList, const gchar *delimiters, gchar escapeChar, gboolean emptyToken){
    g_return_val_if_fail (strList != NULL, NULL);
    g_return_val_if_fail (delimiters != NULL, NULL);
    GString *strBuf=g_string_new(NULL);
    gint i;
    gchar *chPtr;
    gboolean isPrevEmpty=TRUE;
    for(i=0;strList[i]!=NULL; i++){
	if (!isPrevEmpty){
	    g_string_append_c(strBuf, delimiters[0]);
	}
	chPtr=(gchar *) strList[i];
	if (*chPtr!='\0'){
	    isPrevEmpty=FALSE;
	    while(*chPtr!='\0'){
		if (maker_dialog_string_char_at(delimiters, *chPtr)>=0){
		    g_string_append_c(strBuf,escapeChar);
		}else if (*chPtr==escapeChar){
		    g_string_append_c(strBuf,escapeChar);
		}
		gunichar wch=g_utf8_get_char(chPtr);
		g_string_append_unichar(strBuf,wch);
		chPtr=g_utf8_next_char(chPtr);
	    }
	}else{
	    /* Empty string */
	    isPrevEmpty=TRUE;
	}
    }
    return g_string_free(strBuf,FALSE);
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

