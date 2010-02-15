/*
 * Copyright © 2008  Red Hat, Inc. All rights reserved.
 * Copyright © 2008  Ding-Yi Chen <dchen at redhat dot com>
 *
 * This file is part of the MakerDialog Project.
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

typedef struct{
    gchar 		escapeChar;
    gchar 		*delimiters;
    gboolean		emptyToken;
    gint 		maxTokens;
    const gchar 	*str;
    gchar 		**strList;
} String_TestRec;

gchar *STRLIST_01a[]={
    "abc","d;ef","/ghi", NULL,
};

gchar *STRLIST_01b[]={
    "abc","", "d;ef","/ghi", "", NULL,
};

gchar *STRLIST_02a[]={
    "abc",";d;ef:/ghi;", NULL,
};

gchar *STRLIST_02b[]={
    "abc",";d;ef:/ghi;", NULL,
};

gchar *STRLIST_03a[]={
    "test", "ab", "這是\\測試成功-", "\\+", "good", NULL
};

gchar *STRLIST_03b[]={
    "", "test", "ab", "", "這是\\測試成功-", "\\+", "good", NULL
};

gchar *STRLIST_04a[]={
    "test", "ab-:這是\\測試成功--\\+:good", NULL
};

gchar *STRLIST_04b[]={
    "", "test:ab-:這是\\測試成功--\\+:good", NULL
};

/*=== Start of String testing ===*/

String_TestRec STRING_TEST_DATASET[]={
    {'\\',":;", FALSE, -1, "abc:;d\\;ef:/ghi;", STRLIST_01a},
    {'\\',":;", TRUE,  -1, "abc:;d\\;ef:/ghi;", STRLIST_01b},
    {'\\',":;", FALSE, 2, "abc:;d\\;ef:/ghi;", STRLIST_02a},
    {'\\',":;", TRUE, 2, "abc:;d\\;ef:/ghi;", STRLIST_02b},
    {'+',"-:",  FALSE, -1, "-test:ab-:這是\\測試+成功+--+\\++:good", STRLIST_03a},
    {'+',"-:",  TRUE, -1, "-test:ab-:這是\\測試+成功+--+\\++:good", STRLIST_03b},
    {'+',"-:",  FALSE, 2, "-test:ab-:這是\\測試+成功+--+\\++:good", STRLIST_04a},
    {'+',"-:",  TRUE, 2, "-test:ab-:這是\\測試+成功+--+\\++:good", STRLIST_04b},
    {'\0', NULL,  FALSE, -1, NULL, NULL},
};

OutputRec stringTest_run_func(InputRec inputRec, Param param){
    String_TestRec *recs=(String_TestRec *) inputRec;
    output_rec_set_pointer(result,maker_dialog_string_split_set(recs->str,recs->delimiters,recs->escapeChar, recs->emptyToken, recs->maxTokens));
    return result;
}

gboolean stringTest_foreach(TestSubject *testSubject){
    gboolean clean=TRUE;
    String_TestRec *rec=(String_TestRec *) testSubject->dataSet;
    for(;rec->str!=NULL; rec++){
	OutputRec actOutRec=testSubject->run(rec, testSubject->param);
	output_rec_set_pointer(expOutRec,rec->strList);
	gchar *inStr=g_strdup_printf("params(\"%s\", \"%s\", '%c', %s, %d)",
		rec->str,rec->delimiters,rec->escapeChar,(rec->emptyToken)? "TRUE": "FALSE", rec->maxTokens);

	if (!testSubject->verify(actOutRec, expOutRec , testSubject->prompt, inStr)){
	    clean=FALSE;
	}
	g_strfreev((gchar **) actOutRec.v_pointer);
	if (!clean)
	    return FALSE;
    }
    printf("All sub-test completed.\n");
    return TRUE;
}
/*=== End of String testing ===*/
/*=== Start of StringList testing ===*/

String_TestRec STRINGLIST_TEST_DATASET[]={
    {'\\',":;", FALSE, -1, "abc:d\\;ef:/ghi", STRLIST_01a},
    {'\\',":;", TRUE,  -1, "abc::d\\;ef:/ghi:", STRLIST_01b},
    {'\\',":;", FALSE, 2, "abc:\\;d\\;ef\\:/ghi\\;", STRLIST_02a},
    {'\\',":;", TRUE, 2, "abc:\\;d\\;ef\\:/ghi\\;", STRLIST_02b},
    {'+',"-:",  FALSE, -1, "test-ab-這是\\測試成功+--\\++-good", STRLIST_03a},
    {'+',"-:",  TRUE, -1, "-test-ab-這是\\測試成功+--\\++-good", STRLIST_03b},
    {'+',"-:",  FALSE, 2, "test-ab+-+:這是\\測試成功+-+-\\+++:good", STRLIST_04a},
    {'+',"-:",  TRUE, 2, "-test+:ab+-+:這是\\測試成功+-+-\\+++:good", STRLIST_04b},
    {'\0', NULL,  FALSE, -1, NULL, NULL},
};

OutputRec stringListTest_run_func(InputRec inputRec, Param param){
    String_TestRec *recs=(String_TestRec *) inputRec;
    output_rec_set_pointer(result,maker_dialog_string_list_combine(recs->strList,recs->delimiters,recs->escapeChar, recs->emptyToken));
    return result;
}

gboolean stringListTest_foreach(TestSubject *testSubject){
    gboolean clean=TRUE;
    String_TestRec *rec=(String_TestRec *) testSubject->dataSet;
    for(;rec->strList!=NULL; rec++){
	OutputRec actOutRec=testSubject->run(rec, testSubject->param);
	output_rec_set_string(expOutRec,(gchar *) rec->str);
	gchar *inStr=g_strdup_printf("params(\"%s\", \"%s\", '%c', %s)",
		rec->str,rec->delimiters,rec->escapeChar,(rec->emptyToken)? "TRUE": "FALSE");
	if (!testSubject->verify(actOutRec, expOutRec , testSubject->prompt, inStr)){
	    clean=FALSE;
	}
	g_free((gchar *) actOutRec.v_string);
	if (!clean)
	    return FALSE;
    }
    printf("All sub-test completed.\n");
    return TRUE;
}
/*=== End of StringList testing ===*/


TestSubject TEST_COLLECTION[]={
    {"maker_dialog_string_split_set()",
	STRING_TEST_DATASET,
	{0},
	stringTest_foreach, stringTest_run_func, string_list_verify_func},
    {"maker_dialog_string_list_combine()",
	STRINGLIST_TEST_DATASET,
	{0},
	stringListTest_foreach, stringListTest_run_func, string_verify_func},
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

