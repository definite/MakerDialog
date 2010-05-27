/*
 * Copyright © 2009  Red Hat, Inc. All rights reserved.
 * Copyright © 2009  Ding-Yi Chen <dchen at redhat.com>
 *
 *  This file is part of Mkdg.
 *
 *  Mkdg is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Mkdg is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Mkdg.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include "Mkdg.h"

MkdgUi *maker_dialog_ui_init(Mkdg *mDialog, MkdgToolkitInterface *toolkitInterface){
    g_assert(mDialog);
    g_assert(toolkitInterface);
    MkdgUi *ui=g_new(MkdgUi,1);
    ui->toolkitInterface=toolkitInterface;
    ui->dlgObj=NULL;
    ui->mDialog=mDialog;
    mDialog->ui=ui;
    return ui;
}

#include <stdio.h>
gboolean maker_dialog_ui_construct(MkdgUi *dlgUi, gpointer parentWindow, gboolean modal){
    g_assert(dlgUi->toolkitInterface->dialog_construct);
    dlgUi->dlgObj=dlgUi->toolkitInterface->dialog_construct(dlgUi, parentWindow, modal);
    if (dlgUi->dlgObj)
	return TRUE;
    return FALSE;
}

void maker_dialog_ui_destroy(MkdgUi *dlgUi){
    g_assert(dlgUi->dlgObj);
    g_assert(dlgUi->toolkitInterface->dialog_destroy);
    dlgUi->toolkitInterface->dialog_destroy(dlgUi);
}

gint maker_dialog_ui_run(MkdgUi *dlgUi){
    g_assert(dlgUi->toolkitInterface->dialog_run);
    return dlgUi->toolkitInterface->dialog_run(dlgUi);
}

void maker_dialog_ui_show(MkdgUi *dlgUi){
    g_assert(dlgUi->toolkitInterface->dialog_show);
    dlgUi->toolkitInterface->dialog_show(dlgUi);
}

void maker_dialog_ui_hide(MkdgUi *dlgUi){
    g_assert(dlgUi->toolkitInterface->dialog_hide);
    dlgUi->toolkitInterface->dialog_hide(dlgUi);
}

static void maker_dialog_ui_each_control_rule(MkdgPropertyContext *ctx, MkdgWidgetControl control, gpointer userData){
    MkdgUi *dlgUi=(MkdgUi *) userData;
    dlgUi->toolkitInterface->widget_control(dlgUi, ctx->spec->key, control);
}

gboolean maker_dialog_ui_update(MkdgUi *dlgUi, MkdgPropertyContext *ctx){
    g_assert(dlgUi->toolkitInterface->widget_get_value);

    MAKER_DIALOG_DEBUG_MSG(2,"[I2] maker_dialog_ui_update( , %s)", ctx->spec->key);
    MkdgValue *value=dlgUi->toolkitInterface->widget_get_value(dlgUi,ctx->spec->key);
    gboolean ret=TRUE;
    if (ctx->validateFunc && (!ctx->validateFunc(ctx->spec, value))){
	/* Value is invalid. */
	ret=FALSE;
    }else{
	maker_dialog_property_set_value_fast(ctx, value, -2);
	ctx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNSAVED;
	maker_dialog_property_foreach_control_rule(ctx, maker_dialog_ui_each_control_rule, (gpointer) dlgUi );
    }
    maker_dialog_value_free(value);
    return ret;
}

gpointer maker_dialog_ui_get_widget(MkdgUi *dlgUi, const gchar *key){
    if (dlgUi->toolkitInterface->get_widget){
	return dlgUi->toolkitInterface->get_widget(dlgUi, key);
    }
    return NULL;
}

static MkdgIdPair mkdgResponseIdData[]={
    {"REJECT",			MAKER_DIALOG_RESPONSE_REJECT},
    {"ACCEPT",			MAKER_DIALOG_RESPONSE_ACCEPT},
    {"DELETE_EVENT",		MAKER_DIALOG_RESPONSE_DELETE_EVENT},
    {"OK",			MAKER_DIALOG_RESPONSE_OK},
    {"CANCEL",			MAKER_DIALOG_RESPONSE_CANCEL},
    {"CLOSE",			MAKER_DIALOG_RESPONSE_CLOSE},
    {"YES",			MAKER_DIALOG_RESPONSE_YES},
    {"NO",			MAKER_DIALOG_RESPONSE_NO},
    {"APPLY",			MAKER_DIALOG_RESPONSE_APPLY},
    {"FILE",			MAKER_DIALOG_RESPONSE_FILE},
    {"NEW",			MAKER_DIALOG_RESPONSE_NEW},
    {"OPEN",			MAKER_DIALOG_RESPONSE_OPEN},
    {"SAVE",			MAKER_DIALOG_RESPONSE_SAVE},
    {"SAVE_AS",			MAKER_DIALOG_RESPONSE_SAVE_AS},
    {"PRINT",			MAKER_DIALOG_RESPONSE_PRINT},
    {"QUIT",			MAKER_DIALOG_RESPONSE_QUIT},
    {"EDIT",			MAKER_DIALOG_RESPONSE_EDIT},
    {"UNDO",			MAKER_DIALOG_RESPONSE_UNDO},
    {"REDO",			MAKER_DIALOG_RESPONSE_REDO},
    {"REFRESH",			MAKER_DIALOG_RESPONSE_REFRESH},
    {"CUT",			MAKER_DIALOG_RESPONSE_CUT},
    {"COPY",			MAKER_DIALOG_RESPONSE_COPY},
    {"PASTE",			MAKER_DIALOG_RESPONSE_PASTE},
    {"SELECT_ALL",		MAKER_DIALOG_RESPONSE_SELECT_ALL},
    {"FIND",			MAKER_DIALOG_RESPONSE_FIND},
    {"FIND_AND_REPLACE",	MAKER_DIALOG_RESPONSE_FIND_AND_REPLACE},
    {"HELP",			MAKER_DIALOG_RESPONSE_HELP},
    {"GOTO_TOP",		MAKER_DIALOG_RESPONSE_GOTO_TOP},
    {"GOTO_BOTTOM",		MAKER_DIALOG_RESPONSE_GOTO_BOTTOM},
    {"GOTO_FIRST",		MAKER_DIALOG_RESPONSE_GOTO_FIRST},
    {"GOTO_LAST",		MAKER_DIALOG_RESPONSE_GOTO_LAST},
    {"GO_UP",			MAKER_DIALOG_RESPONSE_GO_UP},
    {"GO_DOWN",			MAKER_DIALOG_RESPONSE_GO_DOWN},
    {"GO_BACK",			MAKER_DIALOG_RESPONSE_GO_BACK},
    {"GO_FORWARD",		MAKER_DIALOG_RESPONSE_GO_FORWARD},
    {"NIL",			MAKER_DIALOG_RESPONSE_NIL},
    {NULL,			MAKER_DIALOG_RESPONSE_INVALID},
};

MkdgResponse maker_dialog_parse_button_response_id(const gchar *idStr){
    gint id;
    if ((id=atoi(idStr))>0){
	return id;
    }
    return maker_dialog_id_parse(mkdgResponseIdData, idStr, FALSE);
}



