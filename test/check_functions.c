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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "check_functions.h"

static gint _verboseLevel=VERBOSE_MSG_WARNING;

gint verboseMsg_print(gint verboseLevel, const gchar *format, ...){
    int ret=0;
    va_list ap;
    if (_verboseLevel>=verboseLevel){
	va_start(ap, format);
	ret = vfprintf(stderr, format, ap);
	va_end(ap);
    }
    return ret;
}

gboolean int_verify_func(OutputRec actual, OutputRec expect, const gchar *prompt, const gchar *inStr){
    const gchar *input=(inStr) ? inStr :"";
    verboseMsg_print(VERBOSE_MSG_INFO3,"[Info] %s: on input %s\n\tcomparing: actual=%d, expected=%d\n",prompt, input, actual.v_int, expect.v_int);
    switch(num_compare_func(gint, actual.v_int, expect.v_int)){
	case -1:
	    verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s\n",prompt, input);
	    verboseMsg_print(VERBOSE_MSG_ERROR,"    Actual %d is lesser than expected %d\n", actual.v_int, expect.v_int);
	    return FALSE;
	case 1:
	    verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s\n",prompt, input);
	    verboseMsg_print(VERBOSE_MSG_ERROR,"    Actual %d is greater than expected %d\n", actual.v_int, expect.v_int);
	    return FALSE;
	default:
	    break;
    }
    verboseMsg_print(VERBOSE_MSG_INFO2,"[Ok]: expect and actual are equal to %d\n", expect.v_int);
    return TRUE;
}

gboolean long_verify_func(OutputRec actual, OutputRec expect, const gchar *prompt, const gchar *inStr){
    const gchar *input=(inStr) ? inStr :"";
    verboseMsg_print(VERBOSE_MSG_INFO3,"[Info] %s: on input %s\n\tcomparing: actual=%ld, expected=%ld\n",prompt, input, actual.v_long, expect.v_long);
    switch(num_compare_func(glong, actual.v_long, expect.v_long)){
	case -1:
	    verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s\n",prompt, input);
	    verboseMsg_print(VERBOSE_MSG_ERROR,"    Actual %ld is lesser than expected %ld\n", actual.v_long, expect.v_long);
	    return FALSE;
	case 1:
	    verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s\n",prompt, input);
	    verboseMsg_print(VERBOSE_MSG_ERROR,"    Actual %ld is greater than expected %ld\n", actual.v_long, expect.v_long);
	    return FALSE;
	default:
	    break;
    }
    verboseMsg_print(VERBOSE_MSG_INFO2,"[Ok]: expect and actual are equal to %ld\n", expect.v_long);
    return TRUE;
}

gboolean double_verify_func(OutputRec actual, OutputRec expect, const gchar *prompt, const gchar *inStr){
    const gchar *input=(inStr) ? inStr :"";
    verboseMsg_print(VERBOSE_MSG_INFO3,"[Info] %s: on input %s\n\tcomparing: actual=%g, expected=%g\n",prompt, input, actual.v_double, expect.v_double);
    switch(num_compare_func(gdouble, actual.v_double, expect.v_double)){
	case -1:
	    verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s\n",prompt, input);
	    verboseMsg_print(VERBOSE_MSG_ERROR,"    Actual %g is lesser than expected %g\n", actual.v_double, expect.v_double);
	    return FALSE;
	case 1:
	    verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s\n",prompt, input);
	    verboseMsg_print(VERBOSE_MSG_ERROR,"    Actual %g is greater than expected %g\n", actual.v_double, expect.v_double);
	    return FALSE;
	default:
	    break;
    }
    verboseMsg_print(VERBOSE_MSG_INFO2,"[Ok]: expect and actual are equal to %g\n", expect.v_double);
    return TRUE;
}

gboolean string_verify_func(OutputRec actual, OutputRec expect, const gchar *prompt, const gchar *inStr){
    const gchar *input=(inStr) ? inStr :"";
    const gchar *expectDat=expect.v_string;
    const gchar *actualDat=actual.v_string;

    if (expectDat==NULL && actualDat==NULL){
	verboseMsg_print(VERBOSE_MSG_INFO2,"[Ok]: both are NULL\n");
	return TRUE;
    }
    if (expectDat==NULL){
	verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s\n",prompt, input);
	verboseMsg_print(VERBOSE_MSG_ERROR,"    Expect:[NULL]\tActual:%s\n",actualDat);
	return FALSE;
    }
    if (actualDat==NULL){
	verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s\n",prompt, input);
	verboseMsg_print(VERBOSE_MSG_ERROR,"    Expect:%s\tActual:[NULL]\n",expectDat);
	return FALSE;
    }

    gint ret=strcmp(expectDat,actualDat);
    if (ret){
	verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s\n",prompt,input);
	verboseMsg_print(VERBOSE_MSG_ERROR,"    Expect:%s\tActual:%s\n",expectDat,actualDat);
    }else{
	verboseMsg_print(VERBOSE_MSG_INFO2,"[Ok]: on input %s: expect:(%s) and actual:(%s) matched: \n", input, expectDat,actualDat);
    }
    return TRUE;
}

gchar *string_list_print(gchar **stringList){
    GString *strBuf=g_string_new("{");
    gint i;
    for(i=0;stringList[i]!=NULL;i++){
	if (i>0){
	    g_string_append_c(strBuf,',');
	}
	g_string_append_printf(strBuf,"\n\t\"%s\"",stringList[i]);
    }
    g_string_append_printf(strBuf,"}\n");
    return g_string_free(strBuf, FALSE);
}

gboolean string_list_verify_func(OutputRec actual, OutputRec expect, const gchar *prompt, const gchar *inStr){
    const gchar *input=(inStr) ? inStr :"";
    gchar *actualStr=NULL, *expectStr=NULL;
    g_debug("string_list_verify_func");
    gchar **expectDat=(gchar **) expect.v_pointer;
    gchar **actualDat=(gchar **) actual.v_pointer;


    if (expectDat==NULL && actualDat==NULL){
	verboseMsg_print(VERBOSE_MSG_INFO2,"[Ok]: both are NULL\n");
	return TRUE;
    }
    if (expectDat==NULL){
	verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s\n",prompt, input);
	actualStr=string_list_print(actualDat);
	verboseMsg_print(VERBOSE_MSG_ERROR,"    Expect:[NULL]\tActual:%s\n",actualStr);
	g_free(actualStr);
	return FALSE;
    }
    if (actualDat==NULL){
	verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s\n",prompt, input);
	expectStr=string_list_print(expectDat);
	verboseMsg_print(VERBOSE_MSG_ERROR,"    Expect:%s\tActual:[NULL]\n",expectStr);
	g_free(expectStr);
	return FALSE;
    }

    gint i=0;
    for (i=0;(actualDat[i]!=NULL) && (expectDat[i]!=NULL) ;i++){
	g_debug("string_list_verify_func i=%d",i);
	if (strcmp(actualDat[i],expectDat[i])!=0){
	    verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s, string#%d:\n",prompt, input,i);
	    verboseMsg_print(VERBOSE_MSG_ERROR,"    Expect:%s\tActual:%s\n",expectDat[i],actualDat[i]);
	    return FALSE;
	}
    }
    g_debug("string_list_verify_func i=%d",i);
    if (expectDat[i]!=NULL){
	verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s, string#%d:\n",prompt,input,i);
	verboseMsg_print(VERBOSE_MSG_ERROR,"    Actual:%d strings, but expected additional string %s\n",
		i,expectDat[i]);
	return FALSE;
    }
    g_debug("string_list_verify_func i=%d 2",i);
    if (actualDat[i]!=NULL){
	verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s, string#%d:\n",prompt, input, i);
	verboseMsg_print(VERBOSE_MSG_ERROR,"    Only expect:%d strings, but unexpected actual string %s\n",
		i,actualDat[i]);
	return FALSE;
    }
    verboseMsg_print(VERBOSE_MSG_INFO2,"[Ok]: on input %s: expect: and actual: matched: \n", input);
    g_debug("string_list_verify_func i=%d -1",i);
    return TRUE;
}

int get_testId(int argc, char** argv, TestSubject *testCollection, const gchar *verboseEnv){
    int i;
    int argIndex=1;
    if (argc<2){
	printf("Usage: %s <test num> \n",argv[0]);
	return -1;
    }

    if (getenv(verboseEnv)){
	_verboseLevel=atoi(getenv(verboseEnv));
    }
    g_debug("verbose level:%d, env:%s",_verboseLevel, getenv(verboseEnv));

    int test_index=atoi(argv[argIndex]);

    /* Detect Invalid test number */
    if (test_index<0){
	printf("Invalid test number.\n");
	return -2;
    }
    for(i=0;i<=test_index;i++){
	if (testCollection[i].prompt==NULL){
	    printf("Invalid test number.\n");
	    return -2;
	}
    }
    return test_index;
}

gboolean perform_test_by_id(gint testId, TestSubject *testCollection){
    TestSubject *testSubject=&testCollection[testId];
    return testSubject->foreach(testSubject);
}

