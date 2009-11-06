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


MakerDialogUi *maker_dialog_ui_init(MakerDialog *mDialog, MakerDialogToolkitInterface *toolkitInterface){
    g_assert(mDialog);
    g_assert(toolkitInterface);
    MakerDialogUi *ui=g_new(MakerDialogUi,1);
    ui->toolkitInterface=toolkitInterface;
    ui->dlgObj=NULL;
    ui->mDialog=mDialog;
    mDialog->ui=ui;
    return ui;
}

gboolean maker_dialog_ui_construct(MakerDialog *mDialog, gpointer parentWindow, gboolean modal){
    g_assert(mDialog->ui->toolkitInterface->dialog_construct);
    mDialog->ui->dlgObj=mDialog->ui->toolkitInterface->dialog_construct(mDialog->ui, parentWindow, modal);
    if (mDialog->ui->dlgObj)
	return TRUE;
    return FALSE;
}

void maker_dialog_ui_destroy(MakerDialog *mDialog){
    g_assert(mDialog->ui->dlgObj);
    g_assert(mDialog->ui->toolkitInterface->dialog_destroy);
    mDialog->ui->toolkitInterface->dialog_destroy(mDialog->ui);
    //    g_free(mDialog->ui->dlgObj);
}

gint maker_dialog_ui_run(MakerDialog *mDialog){
    g_assert(mDialog->ui->toolkitInterface->dialog_run);
    return mDialog->ui->toolkitInterface->dialog_run(mDialog->ui);
}

void maker_dialog_ui_show(MakerDialog *mDialog){
    g_assert(mDialog->ui->toolkitInterface->dialog_show);
    mDialog->ui->toolkitInterface->dialog_show(mDialog->ui);
}

void maker_dialog_ui_hide(MakerDialog *mDialog){
    g_assert(mDialog->ui->toolkitInterface->dialog_hide);
    mDialog->ui->toolkitInterface->dialog_hide(mDialog->ui);
}

gboolean maker_dialog_ui_update(MakerDialog *mDialog, MakerDialogPropertyContext *ctx){
    g_assert(mDialog->ui->toolkitInterface->widget_get_value);

    GValue *value=mDialog->ui->toolkitInterface->widget_get_value(mDialog->ui,ctx->spec->key);
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

