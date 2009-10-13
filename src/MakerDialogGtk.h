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
 * @file MakerDialogGtk.h
 * Gtk binding for MakerDialog.
 * This module provides convenient function to build a Gtk dialog.
 *
 * In MakerDialog, a property is a configuration option which associate with a
 * value. According to property specification, MakerDialog generates
 * corresponding UI component to manipulate the property value.
 */
#ifndef MAKER_DIALOG_GTK_H_
#define MAKER_DIALOG_GTK_H_
#include <glib.h>
#include <glib-object.h>
#include "gtk/maker-dialog-gtk.h"

/**
 * Use Gtk as toolkit.
 * @param dlg A MakeDialog instance.
 * @param argc Pointer to number of arguments from command-line.
 * @param argv Pointer to arguments from command-line.
 * @return TRUE for succeed; FALSE otherwise.
 *
 * Set the toolkit handler as Gtk.
 */
gboolean maker_dialog_set_toolkit_handler_gtk(MakerDialog *dlg, gint *argc, gchar ***argv);

#endif /* MAKER_DIALOG_GTK_H_ */

