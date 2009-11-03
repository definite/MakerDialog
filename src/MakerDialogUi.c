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
#include "MakerDialog.h"


MakerDialogUi *maker_dialog_ui_init(MakerDialog *mDialog, MakerDialogToolkitHandler *toolkitHandler){
    g_assert(mDialog);
    g_assert(toolkitHandler);
    MakerDialogUi *dlgUi=g_new(MakerDialogUi,1);
    dlgUi->toolkitHandler=toolkitHandler;
    dlgUi->dlgObj=NULL;
    dlgUi->mDialog=mDialog;
    mDialog->dlgUi=dlgUi;
    return dlgUi;
}

gboolean maker_dialog_ui_construct(MakerDialog *mDialog, gpointer parentWindow, gboolean modal){
    g_assert(mDialog->dlgUi->toolkitHandler->dialog_construct);
    mDialog->dlgUi->dlgObj=mDialog->dlgUi->toolkitHandler->dialog_construct(mDialog->dlgUi, parentWindow, modal);
    if (mDialog->dlgUi->dlgObj)
	return TRUE;
    return FALSE;
}

void maker_dialog_ui_destroy(MakerDialog *mDialog){
    g_assert(mDialog->dlgUi->dlgObj);
    g_assert(mDialog->dlgUi->toolkitHandler->dialog_destroy);
    mDialog->dlgUi->toolkitHandler->dialog_destroy(mDialog->dlgUi);
    //    g_free(mDialog->dlgUi->dlgObj);
}

gint maker_dialog_ui_run(MakerDialog *mDialog){
    g_assert(mDialog->dlgUi->toolkitHandler->dialog_run);
    return mDialog->dlgUi->toolkitHandler->dialog_run(mDialog->dlgUi);
}

void maker_dialog_ui_show(MakerDialog *mDialog){
    g_assert(mDialog->dlgUi->toolkitHandler->dialog_show);
    mDialog->dlgUi->toolkitHandler->dialog_show(mDialog->dlgUi);
}

void maker_dialog_ui_hide(MakerDialog *mDialog){
    g_assert(mDialog->dlgUi->toolkitHandler->dialog_hide);
    mDialog->dlgUi->toolkitHandler->dialog_hide(mDialog->dlgUi);
}

gboolean maker_dialog_ui_update(MakerDialog *mDialog, MakerDialogPropertyContext *ctx){
    g_assert(mDialog->dlgUi->toolkitHandler->widget_get_value);

    GValue *value=mDialog->dlgUi->toolkitHandler->widget_get_value(mDialog->dlgUi,ctx->spec->key);
    gboolean ret=TRUE;
    if (ctx->validateFunc && (!ctx->validateFunc(ctx->spec, value))){
	/* Value is invalid. */
	ret=FALSE;
    }else{
	maker_dialog_property_set_value_fast(ctx, value, -2);
	ctx->flags |= MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNSAVED;
    }
    g_value_unset(value);
    g_free(value);
    return ret;
}

