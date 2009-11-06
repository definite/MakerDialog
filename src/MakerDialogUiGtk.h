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
/**
 * @file MakerDialogUiGtk.h
 * Gtk UI front-end for MakerDialog.
 *
 * This module implements the UI interface to bridge between Gtk and MakerDialog.
 */
#ifndef MAKER_DIALOG_UI_GTK_H_
#define MAKER_DIALOG_UI_GTK_H_
#include <glib.h>
#include <glib-object.h>
#include "gtk/maker-dialog-gtk.h"

/**
 * Use Gtk as UI front-end.
 *
 * It calls maker_dialog_ui_init() and gtk_init_check(),
 * so no need to initialize gtk again.
 *
 * @param mDialog A MakeDialog.
 * @param argc Pointer to number of arguments from command-line.
 * @param argv Pointer to arguments from command-line.
 * @return A newly allocated MakerDialogUi instance; NULL if failed.
 */
MakerDialogUi *maker_dialog_ui_use_gtk(MakerDialog *mDialog, gint *argc, gchar ***argv);

#endif /* MAKER_DIALOG_UI_GTK_H_ */

