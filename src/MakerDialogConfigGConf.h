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
 * @file MakerDialogConfigGConf.h
 * GConf configuration back-end.
 *
 * This module implements the UI interface to bridge between GConf back-end and MakerDialog.
 */
#ifndef MAKER_DIALOG_CONFIG_G_CONF_H_
#define MAKER_DIALOG_CONFIG_G_CONF_H_
#include <glib.h>
#include <glib-object.h>
#include "MakerDialogConfig.h"

/**
 * New a MakerDialogConfig using GConf back-end.
 *
 * This function constructs a configuration back-end using GConf GKeyFile
 * back-end.
 *
 * During construction, the new MakerDialogConfig is associated to the \a mDialog.
 * Thus, maker_dialog_destroy() can free the associated MakerDialogConfig as well.
 *
 * @param mDialog 	A MakerDialog.
 * @return A newly allocated MakerDialogConfig with glib GKeyFile back-end.
 * @see maker_dialog_config_new().
 */
MakerDialogConfig *maker_dialog_config_use_gconf(MakerDialog *mDialog);


#endif /* MAKER_DIALOG_CONFIG_G_CONF_H_ */

