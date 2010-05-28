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

MkdgUi *mkdg_ui_init(Mkdg *mDialog, MkdgToolkitInterface *toolkitInterface){
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
gboolean mkdg_ui_construct(MkdgUi *dlgUi, gpointer parentWindow, gboolean modal){
    g_assert(dlgUi->toolkitInterface->dialog_construct);
    dlgUi->dlgObj=dlgUi->toolkitInterface->dialog_construct(dlgUi, parentWindow, modal);
    if (dlgUi->dlgObj)
	return TRUE;
    return FALSE;
}

void mkdg_ui_destroy(MkdgUi *dlgUi){
    g_assert(dlgUi->dlgObj);
    g_assert(dlgUi->toolkitInterface->dialog_destroy);
    dlgUi->toolkitInterface->dialog_destroy(dlgUi);
}

gint mkdg_ui_run(MkdgUi *dlgUi){
    g_assert(dlgUi->toolkitInterface->dialog_run);
    return dlgUi->toolkitInterface->dialog_run(dlgUi);
}

void mkdg_ui_show(MkdgUi *dlgUi){
    g_assert(dlgUi->toolkitInterface->dialog_show);
    dlgUi->toolkitInterface->dialog_show(dlgUi);
}

void mkdg_ui_hide(MkdgUi *dlgUi){
    g_assert(dlgUi->toolkitInterface->dialog_hide);
    dlgUi->toolkitInterface->dialog_hide(dlgUi);
}

static void mkdg_ui_each_control_rule(MkdgPropertyContext *ctx, MkdgWidgetControl control, gpointer userData){
    MkdgUi *dlgUi=(MkdgUi *) userData;
    dlgUi->toolkitInterface->widget_control(dlgUi, ctx->spec->key, control);
}

gboolean mkdg_ui_update(MkdgUi *dlgUi, MkdgPropertyContext *ctx){
    g_assert(dlgUi->toolkitInterface->widget_get_value);

    MKDG_DEBUG_MSG(2,"[I2] mkdg_ui_update( , %s)", ctx->spec->key);
    MkdgValue *value=dlgUi->toolkitInterface->widget_get_value(dlgUi,ctx->spec->key);
    gboolean ret=TRUE;
    if (ctx->validateFunc && (!ctx->validateFunc(ctx->spec, value))){
	/* Value is invalid. */
	ret=FALSE;
    }else{
	mkdg_property_set_value_fast(ctx, value, -2);
	ctx->flags |= MKDG_PROPERTY_CONTEXT_FLAG_UNSAVED;
	mkdg_property_foreach_control_rule(ctx, mkdg_ui_each_control_rule, (gpointer) dlgUi );
    }
    mkdg_value_free(value);
    return ret;
}

gpointer mkdg_ui_get_widget(MkdgUi *dlgUi, const gchar *key){
    if (dlgUi->toolkitInterface->get_widget){
	return dlgUi->toolkitInterface->get_widget(dlgUi, key);
    }
    return NULL;
}

static MkdgIdPair mkdgResponseIdData[]={
    {"REJECT",			MKDG_RESPONSE_REJECT},
    {"ACCEPT",			MKDG_RESPONSE_ACCEPT},
    {"DELETE_EVENT",		MKDG_RESPONSE_DELETE_EVENT},
    {"OK",			MKDG_RESPONSE_OK},
    {"CANCEL",			MKDG_RESPONSE_CANCEL},
    {"CLOSE",			MKDG_RESPONSE_CLOSE},
    {"YES",			MKDG_RESPONSE_YES},
    {"NO",			MKDG_RESPONSE_NO},
    {"APPLY",			MKDG_RESPONSE_APPLY},
    {"FILE",			MKDG_RESPONSE_FILE},
    {"NEW",			MKDG_RESPONSE_NEW},
    {"OPEN",			MKDG_RESPONSE_OPEN},
    {"SAVE",			MKDG_RESPONSE_SAVE},
    {"SAVE_AS",			MKDG_RESPONSE_SAVE_AS},
    {"PRINT",			MKDG_RESPONSE_PRINT},
    {"QUIT",			MKDG_RESPONSE_QUIT},
    {"EDIT",			MKDG_RESPONSE_EDIT},
    {"UNDO",			MKDG_RESPONSE_UNDO},
    {"REDO",			MKDG_RESPONSE_REDO},
    {"REFRESH",			MKDG_RESPONSE_REFRESH},
    {"CUT",			MKDG_RESPONSE_CUT},
    {"COPY",			MKDG_RESPONSE_COPY},
    {"PASTE",			MKDG_RESPONSE_PASTE},
    {"SELECT_ALL",		MKDG_RESPONSE_SELECT_ALL},
    {"FIND",			MKDG_RESPONSE_FIND},
    {"FIND_AND_REPLACE",	MKDG_RESPONSE_FIND_AND_REPLACE},
    {"HELP",			MKDG_RESPONSE_HELP},
    {"GOTO_TOP",		MKDG_RESPONSE_GOTO_TOP},
    {"GOTO_BOTTOM",		MKDG_RESPONSE_GOTO_BOTTOM},
    {"GOTO_FIRST",		MKDG_RESPONSE_GOTO_FIRST},
    {"GOTO_LAST",		MKDG_RESPONSE_GOTO_LAST},
    {"GO_UP",			MKDG_RESPONSE_GO_UP},
    {"GO_DOWN",			MKDG_RESPONSE_GO_DOWN},
    {"GO_BACK",			MKDG_RESPONSE_GO_BACK},
    {"GO_FORWARD",		MKDG_RESPONSE_GO_FORWARD},
    {"NIL",			MKDG_RESPONSE_NIL},
    {NULL,			MKDG_RESPONSE_INVALID},
};

MkdgResponse mkdg_parse_button_response_id(const gchar *idStr){
    gint id;
    if ((id=atoi(idStr))>0){
	return id;
    }
    return mkdg_id_parse(mkdgResponseIdData, idStr, FALSE);
}



