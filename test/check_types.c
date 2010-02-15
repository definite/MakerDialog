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

/*=== Start of comp test ===*/
typedef struct {
    MkdgType mType;
    const gchar *value1Str;
    const gchar *value2Str;
    const gchar *parseOpt;
    const gchar *cmpOpt;
} MkdgComp_InputRec;

typedef struct{
    OutputRec out;
    MkdgComp_InputRec in;
} MkdgComp_TestRec;

MkdgComp_TestRec COMPARISON_DATASET[]={
    {{-1}, {MKDG_TYPE_STRING, "default", "hsu", NULL, NULL}},
    {{0}, {MKDG_TYPE_STRING, "dvorak", "dvorak", NULL, NULL}},
    {{-1}, {MKDG_TYPE_STRING, "dvorak", "dvorak_hsu", NULL, NULL}},
    {{-1}, {MKDG_TYPE_INT, "32", "103", NULL, NULL}},
    {{0}, {MKDG_TYPE_COLOR, "white", "#FFFFFF", NULL, NULL}},
    {{0}, {MKDG_TYPE_COLOR, "red", "#FF0000", NULL, NULL}},
    {{-1}, {MKDG_TYPE_COLOR, "blue", "red", NULL, NULL}},
    {{0},  {MKDG_TYPE_STRING_LIST, "test1;test2","test1;test2", NULL, NULL}},
    {{-1},  {MKDG_TYPE_STRING_LIST, "test1","test1;test2", NULL, NULL}},
    {{1},  {MKDG_TYPE_STRING_LIST, "SADF;ASDF","TTT", NULL, NULL}},
    {{-1}, {MKDG_TYPE_INVALID, NULL, NULL, NULL, NULL}},
};

OutputRec compTest_run_func(InputRec inputRec, Param param){
    MkdgComp_InputRec *inRec=(MkdgComp_InputRec *) inputRec;
    MkdgValue *mValue1=maker_dialog_value_new(inRec->mType, NULL);
    maker_dialog_value_from_string(mValue1, inRec->value1Str, inRec->parseOpt);
    MkdgValue *mValue2=maker_dialog_value_new(inRec->mType, NULL);
    maker_dialog_value_from_string(mValue2, inRec->value2Str, inRec->parseOpt);
    output_rec_set_int(result,maker_dialog_value_compare(mValue1, mValue2, inRec->cmpOpt));
    return result;
}

gchar *compTest_to_string(InputRec inputRec, Param param){
    MkdgComp_InputRec *inRec=(MkdgComp_InputRec *) inputRec;
    GString *strBuf=g_string_new(NULL);
    g_string_printf(strBuf,"mType=%d,value1=%s\tvalue2=%s", inRec->mType, inRec->value1Str, inRec->value2Str);
    return g_string_free(strBuf, FALSE);
}

gboolean compTest_foreach(TestSubject *testSubject){
    gboolean clean=TRUE;
    MkdgComp_TestRec *rec=(MkdgComp_TestRec *) testSubject->dataSet;
    for(;rec->in.mType!=MKDG_TYPE_INVALID;rec++){
	MkdgComp_InputRec *inRec=&(rec->in);
	OutputRec expOutRec=rec->out;
	OutputRec actOutRec=testSubject->run(inRec, testSubject->param);
	gchar *inStr=compTest_to_string(inRec, testSubject->param);
	if (!testSubject->verify(actOutRec, expOutRec, testSubject->prompt, inStr)){
	    clean=FALSE;
	}
	g_free(inStr);
	if (!clean)
	    return FALSE;
    }
    printf("All sub-test completed.\n");
    return TRUE;
}
/*=== End of comp test ===*/

/*=== Start of from string test ===*/
typedef struct {
    MkdgType mType;
    const gchar *valueStr;
    const gchar *parseOpt;
} MkdgFromStr_InputRec;

typedef struct{
    OutputRec out;
    MkdgFromStr_InputRec in;
} MkdgFromStr_TestRec;

char *STRING_LIST_DATA[7][3]={
    {"ASDF", "SADF",NULL},
    {"ASDF", "",NULL},
    {"", "SADF",NULL},
    {"ASDF;", "SADF",NULL},
    {"ASDF", ";SADF",NULL},
    {"ASDF", "S;ADF",NULL},
    {NULL, NULL, NULL}
};

MkdgFromStr_TestRec FROM_STR_DATASET[]={
    {{.v_int=2},	{MKDG_TYPE_INT,		"2", NULL}},
    {{.v_int=0},	{MKDG_TYPE_INT,		"0", NULL}},
    {{.v_double=1.0}, 	{MKDG_TYPE_DOUBLE, 	"1.0", NULL}},
    {{.v_double=2.0}, 	{MKDG_TYPE_DOUBLE, 	"2.0", NULL}},
    {{.v_double=0.0}, 	{MKDG_TYPE_DOUBLE, 	"0.0", NULL}},
    {{.v_double=-1.0}, 	{MKDG_TYPE_DOUBLE, 	"-1.0", NULL}},
    {{.v_string_list=STRING_LIST_DATA[0]}, 	{MKDG_TYPE_STRING_LIST, "ASDF;SADF", NULL}},
    {{.v_string_list=STRING_LIST_DATA[1]}, 	{MKDG_TYPE_STRING_LIST, "ASDF;", NULL}},
    {{.v_string_list=STRING_LIST_DATA[2]}, 	{MKDG_TYPE_STRING_LIST, ";SADF", NULL}},
    {{.v_string_list=STRING_LIST_DATA[3]}, 	{MKDG_TYPE_STRING_LIST, "ASDF\\;;SADF", NULL}},
    {{.v_string_list=STRING_LIST_DATA[4]}, 	{MKDG_TYPE_STRING_LIST, "ASDF;\\;SADF", NULL}},
    {{.v_string_list=STRING_LIST_DATA[5]}, 	{MKDG_TYPE_STRING_LIST, "ASDF;S\\;ADF", NULL}},
    {{0}, 	{MKDG_TYPE_INVALID, 	"0", NULL}},
};

OutputRec fromStrTest_run_func(InputRec inputRec, Param param){
    MkdgFromStr_InputRec *inRec=(MkdgFromStr_InputRec *) inputRec;
    MkdgValue *mValue=maker_dialog_value_new(inRec->mType, NULL);
    maker_dialog_value_from_string(mValue, inRec->valueStr, inRec->parseOpt);
    OutputRec result=mValue->data[0];
    return result;
}

gchar *fromStrTest_to_string(InputRec inputRec, Param param){
    MkdgFromStr_InputRec *inRec=(MkdgFromStr_InputRec *) inputRec;
    GString *strBuf=g_string_new(NULL);
    switch(inRec->mType){
	case MKDG_TYPE_INT:
	    g_string_printf(strBuf,"mType=INT, value=%s", inRec->valueStr);
	    break;
	case MKDG_TYPE_LONG:
	    g_string_printf(strBuf,"mType=LONG, value=%s", inRec->valueStr);
	    break;
	case MKDG_TYPE_DOUBLE:
	    g_string_printf(strBuf,"mType=DOUBLE, value=%s", inRec->valueStr);
	    break;
	default:
	    break;
    }
    return g_string_free(strBuf, FALSE);
}

gboolean fromStrTest_foreach(TestSubject *testSubject){
    gboolean clean=TRUE;
    MkdgFromStr_TestRec *rec=(MkdgFromStr_TestRec *) testSubject->dataSet;
    for(;rec->in.mType!=MKDG_TYPE_INVALID;rec++){
	MkdgFromStr_InputRec *inRec=&(rec->in);
	OutputRec expOutRec=rec->out;
	OutputRec actOutRec=testSubject->run(inRec, testSubject->param);
	gchar *inStr=fromStrTest_to_string(inRec, testSubject->param);
	switch(rec->in.mType){
	    case MKDG_TYPE_INT:
		clean=int_verify_func(actOutRec, expOutRec, testSubject->prompt, inStr);
		break;
	    case MKDG_TYPE_LONG:
		clean=long_verify_func(actOutRec, expOutRec, testSubject->prompt, inStr);
		break;
	    case MKDG_TYPE_DOUBLE:
		clean=double_verify_func(actOutRec, expOutRec, testSubject->prompt, inStr);
		break;
	    default:
		break;
	}
	g_free(inStr);
	if (!clean)
	    return FALSE;
    }
    printf("All sub-test completed.\n");
    return TRUE;
}
/*=== End of from string test ===*/

TestSubject TEST_COLLECTION[]={
    {"Comparison functions",
	COMPARISON_DATASET,
	{0},
	compTest_foreach, compTest_run_func, int_verify_func},
    {"From string",
	FROM_STR_DATASET,
	{0},
	fromStrTest_foreach, fromStrTest_run_func, int_verify_func},
    {NULL,NULL, {0}, NULL, NULL, NULL},
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

