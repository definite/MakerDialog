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

gboolean int_verify_func(gint actual, gint expect, const gchar *prompt, const gchar *inStr){
    const gchar *input=(inStr) ? inStr :"";
    verboseMsg_print(VERBOSE_MSG_INFO3,"[Info] %s: on input %s\n\tcomparing: actual=%d, expect=%d\n",prompt, input, actual, expect);
    if (actual<expect){
	verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s\n",prompt, input);
	verboseMsg_print(VERBOSE_MSG_ERROR,"    Actual %ld is lesser than expect %ld", actual, expect);
	return FALSE;
    }else if (actual>expect){
	verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s\n",prompt, input);
	verboseMsg_print(VERBOSE_MSG_ERROR,"    Actual %ld is greater than expect %ld", actual, expect);
	return FALSE;
    }
    verboseMsg_print(VERBOSE_MSG_INFO2,"[Ok]: expect and actual are equal to %ld\n", expect);
    return TRUE;
}

gboolean long_int_verify_func(glong actual, glong expect, const gchar *prompt, const gchar *inStr){
    const gchar *input=(inStr) ? inStr :"";
    verboseMsg_print(VERBOSE_MSG_INFO3,"[Info] %s: on input %s\n\tcomparing: actual=%ld, expect=%ld\n",prompt, input, actual, expect);
    if (actual<expect){
	verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s\n",prompt, input);
	verboseMsg_print(VERBOSE_MSG_ERROR,"    Actual %ld is lesser than expect %ld\n", actual, expect);
	return FALSE;
    }else if (actual>expect){
	verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s\n",prompt, input);
	verboseMsg_print(VERBOSE_MSG_ERROR,"    Actual %ld is greater than expect %ld\n", actual, expect);
	return FALSE;
    }
    verboseMsg_print(VERBOSE_MSG_INFO2,"[Ok]: expect and actual are equal to %ld\n", expect);
    return TRUE;
}

gboolean string_verify_func(const gchar *actual, const gchar *expect, const gchar *prompt, const gchar *inStr){
    const gchar *input=(inStr) ? inStr :"";
    if (expect==NULL && actual==NULL){
	verboseMsg_print(VERBOSE_MSG_INFO2,"[Ok]: both are NULL\n");
	return TRUE;
    }
    if (expect==NULL){
	verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s\n",prompt, input);
	verboseMsg_print(VERBOSE_MSG_ERROR,"    Expect:[NULL]\tActual:%s\n",actual);
	return FALSE;
    }
    if (actual==NULL){
	verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s\n",prompt, input);
	verboseMsg_print(VERBOSE_MSG_ERROR,"    Expect:%s\tActual:[NULL]\n",expect);
	return FALSE;
    }

    gint ret=strcmp(expect,actual);
    if (ret){
	verboseMsg_print(VERBOSE_MSG_ERROR,"[Error]: %s: on input %s\n",prompt,input);
	verboseMsg_print(VERBOSE_MSG_ERROR,"    Expect:%s\tActual:%s\n",expect,actual);
    }else{
	verboseMsg_print(VERBOSE_MSG_INFO2,"[Ok]: on input %s: expect:(%s) and actual:(%s) matched: \n", input, expect,actual);
    }
    return TRUE;
}

void test_output_rec_g_free(OutputRec actOutRec){
    g_free(actOutRec);
}

int get_testId(int argc, char** argv, TestSubject *testCollection, const gchar *verboseEnv){
    int i;
    int argIndex=1;
    if (argc<2){
	printf("Usage: %s [-V num] <test num> \n",argv[0]);
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

