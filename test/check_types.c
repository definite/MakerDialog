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

typedef struct {
    MkdgType mType;
    const gchar *value1Str;
    const gchar *value2Str;
    const gchar *parseOpt;
    const gchar *cmpOpt;
} MkdgType_InputRec;

typedef gint MkdgType_OutputRec;

typedef struct{
    MkdgType_OutputRec out;
    MkdgType_InputRec in;
} MkdgType_TestRec;


#define COMPARISON_DATASET_SIZE 2

MkdgType_TestRec COMPARISON_DATASET[COMPARISON_DATASET_SIZE]={
    {-1, {MKDG_TYPE_STRING, "default", "hsu", NULL, NULL}},
    {0, {MKDG_TYPE_STRING, "dvorak", "dvorak", NULL, NULL}},
};

/*==========================================================
 * Start of testing functions.
 */


gpointer compTest_run_func(InputRec inputRec, Param param){
    MkdgType_InputRec *inRec=(MkdgType_InputRec *) inputRec;
    MkdgValue *mValue1=maker_dialog_value_new(inRec->mType, NULL);
    maker_dialog_value_from_string(mValue1, inRec->value1Str, inRec->parseOpt);
    MkdgValue *mValue2=maker_dialog_value_new(inRec->mType, NULL);
    maker_dialog_value_from_string(mValue2, inRec->value2Str, inRec->parseOpt);
    gint ret=maker_dialog_value_compare(mValue1, mValue2, inRec->cmpOpt);
    MkdgType_OutputRec *retPtr=g_new(MkdgType_OutputRec, 1);
    *retPtr=ret;
    return retPtr;
}

gboolean compTest_int_verify_func(OutputRec actual, OutputRec expect, const gchar *prompt, const gchar *inStr){
    MkdgType_OutputRec *actOutRec=(MkdgType_OutputRec *) actual;
    MkdgType_OutputRec *expOutRec=(MkdgType_OutputRec *) expect;
    return int_verify_func(*actOutRec,*expOutRec, prompt, inStr);
}

gchar *compTest_to_string(InputRec inputRec, Param param){
    MkdgType_InputRec *inRec=(MkdgType_InputRec *) inputRec;
    GString *strBuf=g_string_new(NULL);
    g_string_printf(strBuf,"mType=%d,value1=%s\tvalue2=%s", inRec->mType, inRec->value1Str, inRec->value2Str);
    return g_string_free(strBuf, FALSE);
}

gboolean compTest_foreach(TestSubject *testSubject){
    gboolean clean=TRUE;
    guint i;
    MkdgType_TestRec *recs=(MkdgType_TestRec *) testSubject->dataSet;
    for(i=0;i< testSubject->dataSetSize ;i++){
	MkdgType_InputRec *inRec=&(recs[i].in);
	MkdgType_OutputRec *outRec=&(recs[i].out);
	OutputRec *actOutRec=testSubject->run(inRec, testSubject->param);
	gchar *inStr=(testSubject->in_rec_to_string) ? testSubject->in_rec_to_string(inRec, testSubject->param) : NULL;
	if (!testSubject->verify(actOutRec, outRec, testSubject->prompt, inStr)){
	    clean=FALSE;
	}
	g_free(inStr);
	g_free(actOutRec);
	if (!clean)
	    return FALSE;
    }
    printf("All sub-test completed.\n");
    return TRUE;
}
/*=== End of test callback functions. */

TestSubject TEST_COLLECTION[]={
    {"Comparison functions",
	COMPARISON_DATASET,
	COMPARISON_DATASET_SIZE,
	NULL,
	compTest_foreach, compTest_run_func, compTest_int_verify_func, compTest_to_string},
    {NULL,NULL, 0, NULL, NULL, NULL, NULL, NULL},
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

