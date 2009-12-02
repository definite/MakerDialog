/*
 * Copyright © 2008  Red Hat, Inc. All rights reserved.
 * Copyright © 2008  Ding-Yi Chen <dchen at redhat dot com>
 *
 * This file is part of the libUnihan Project.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "MakerDialog.h"
#include "check_functions.h"

#define RESET_COUNTER 1

typedef struct{
    gchar 		escapeChar;
    const gchar 	*delimiters;
    gint 		maxTokens;
    const gchar 	*str;
    const gchar 	**strList;
} String_TestRec;

const gchar *STRLIST_01[]={
    "abc","d;ef","/ghi", NULL,
};

const gchar *STRLIST_02[]={
    "abc","d;ef","/ghi", NULL,
};

const gchar *STRLIST_03[]={
    "test", "ab", "這是\\測試成功-", "+\\", "++", "good", NULL
};


#define STRING_TEST_DATASET_SIZE 3

String_TestRec STRING_TEST_DATASET[STRING_TEST_DATASET_SIZE]={
    {'\\',":;", -1, "abc:;d\\;ef:/ghi;", STRLIST_01},
    {'\\',":;",  2, "abc:;d\\;ef:/ghi;", STRLIST_02},
    {'+',"-:",  -1, "-test:ab-:這是\\測試+成功+--+\\++:good", STRLIST_03},
};

/*==========================================================
 * Start of testing functions.
 */

gpointer stringTest_run_func(InputRec inputRec, Param param){
    String_TestRec *recs=(String_TestRec *) inputRec;
    gchar **result=maker_dialog_string_split_set(recs->str,recs->delimiters,recs->escapeChar, recs->maxTokens);
    return (gpointer) result;
}

gboolean stringTest_verify_func(OutputRec actual, OutputRec expect, const gchar *prompt, const gchar *inStr){
    gchar **actOutRec=(gchar **) actual;
    gchar **expOutRec=(gchar **) expect;
    return string_list_verify_func(actOutRec, expOutRec, prompt, inStr);
}

gboolean stringTest_foreach(TestSubject *testSubject){
    gboolean clean=TRUE;
    guint i;
    String_TestRec *recs=(String_TestRec *) testSubject->dataSet;
    for(i=0;i< testSubject->dataSetSize ;i++){
	g_debug("1i=%d",i);
	OutputRec *actOutRec=testSubject->run(&recs[i], testSubject->param);
	gchar *inStr=g_strdup_printf("String %s",recs->str);
	if (!testSubject->verify(actOutRec, (gpointer) recs->strList, testSubject->prompt, inStr)){
	    clean=FALSE;
	}
	g_free(actOutRec);
	if (!clean)
	    return FALSE;
    }
    printf("All sub-test completed.\n");
    return TRUE;
}
/*=== End of test callback functions. */

TestSubject TEST_COLLECTION[]={
    {"maker_dialog_string_split_set()",
	STRING_TEST_DATASET,
	STRING_TEST_DATASET_SIZE,
	NULL,
	stringTest_foreach, stringTest_run_func, stringTest_verify_func},
    {NULL,NULL, 0, NULL, NULL, NULL, NULL },
};

int main(int argc, char** argv){
    int testId=get_testId(argc,argv,TEST_COLLECTION, "MAKER_DIALOG_VERBOSE");
    if (testId<0){
	return testId;
    }
    if (perform_test_by_id(testId,TEST_COLLECTION))
	return 0;
    return 1;
}

