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
#include <glib.h>

#define STRING_BUFFER_SIZE_DEFAULT 2000
#define VERBOSE_MSG_NONE        -1 //!< No debug message output.
#define VERBOSE_MSG_CRITICAL    0  //!< Output critical messages.
#define VERBOSE_MSG_ERROR       1  //!< Output general errors and above.
#define VERBOSE_MSG_WARNING     2  //!< Output warning messages and above.
#define VERBOSE_MSG_INFO1       3  //!< Output the information message level 1 and above.
#define VERBOSE_MSG_INFO2       4  //!< Output the information message level 2 and above.
#define VERBOSE_MSG_INFO3       5  //!< Output the information message level 3 and above.
#define VERBOSE_MSG_INFO4       6  //!< Output the information message level 4 and above.
#define VERBOSE_MSG_INFO5       7  //!< Output the information message level 5 and above.
#define VERBOSE_MSG_INFO6       8  //!< Output the information message level 6 and above.

typedef gpointer InputRec;
typedef gpointer Param;
typedef gpointer OutputRec;

typedef struct _TestSubject TestSubject;

typedef gboolean (* ForeachFunc)(TestSubject *subject);
typedef OutputRec (* RunFunc)(InputRec inputRec, Param param);
typedef gboolean (* VerifyFunc)(OutputRec actOutRec, OutputRec expOutRec, const gchar *prompt, const gchar *inStr);
typedef void (* FreeFunc)(OutputRec actOutRec);
typedef gchar *(* InputRecToString)(InputRec inputRec, Param param);

struct _TestSubject{
    const gchar 	*prompt;
    gpointer 		dataSet;
    guint 		dataSetSize;
    Param    		param;
    ForeachFunc 	foreach;
    RunFunc 		run;
    VerifyFunc 		verify;
    InputRecToString    in_rec_to_string;
} ;


/**
 * Output Verbose message.
 *
 * This function prints the verbose to screen, and log files.
 * The message will be displayed on screen if \a verboseLevel is lesser or equal to current verbose level.
 *
 * The rest of this function works like fprintf.
 * @param verboseLevel
 * @param format
 * @param ...
 * @returns same as printf/fprintf
 */
gint verboseMsg_print(gint verboseLevel, const gchar *format, ...);

gboolean int_verify_func(gint actual, gint expect, const gchar *prompt, const gchar *inStr);
gboolean long_int_verify_func(glong actual, glong expect, const gchar *prompt, const gchar *inStr);
gboolean string_verify_func(const gchar *actual, const gchar *expect, const gchar *prompt, const gchar *inStr);

void test_output_rec_g_free(OutputRec actOutRec);

int get_testId(int argc, char** argv, TestSubject *testCollection, const gchar *verboseEnv);

gboolean perform_test_by_id(gint testId, TestSubject *testCollection);

