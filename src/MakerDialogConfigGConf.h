/*
 * Copyright © 2009  Red Hat, Inc. All rights reserved.
 * Copyright © 2009  Ding-Yi Chen <dchen at redhat.com>
 *
 *  This file is part of Mkdg.
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
 *  along with Mkdg.  If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * @file MakerDialogConfigGConf.h
 * GConf configuration back-end.
 *
 * This module implements the UI interface to bridge between GConf back-end and Mkdg.
 */
#ifndef MKDG_CONFIG_G_CONF_H_
#define MKDG_CONFIG_G_CONF_H_
#include <glib.h>
#include <glib-object.h>
#include "MakerDialogConfig.h"

/**
 * The configure file interface for GConf.
 *
 * The configure file interface for GConf.
 */
extern MkdgConfigFileInterface MKDG_CONFIG_FILE_INTERFACE_GCONF;

/**
 * New a MkdgConfig which use glib GConf back-end.
 *
 * This function returns a newly allocated MkdgConfig
 * which use glib GConf back-end.
 *
 * During construction, the new MkdgConfig is associated to the \a mDialog.
 * Thus, mkdg_destroy() can free the associated MkdgConfig as well.
 *
 * @param mDialog 	A Mkdg.
 * @return A newly allocated MkdgConfig with glib GConf back-end.
 * @see mkdg_config_new().
 */
MkdgConfig *mkdg_config_use_gconf(Mkdg *mDialog);

/**
 * Output the property as a GConf schemes file.
 *
 * Output the property as a GConf schemes file.
 * @param mDialog		A MakerDialog instance.
 * @param filename		Output filename.
 * @param indentSpace		Number of spaces for indent.
 * @param owner			Owner of the schemas.
 * @param schemasHome		The "home directory" of GConf schemas.
 * @param locales		Supported locales, use ';' as delimiter.
 * @param error		Error return location, or \c NULL.
 * @return TRUE if succeed; FALSE otherwise.
 *
 * Output the parameters as GConf schemes file.
 */
gboolean mkdg_config_gconf_write_schemas_file
(Mkdg *mDialog, const gchar *filename, gint indentSpace, const gchar *schemasHome, const gchar *owner, const gchar *locales, GError **error);

#endif /* MKDG_CONFIG_G_CONF_H_ */

