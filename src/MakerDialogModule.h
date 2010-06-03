/*
 * Copyright © 2010  Red Hat, Inc. All rights reserved.
 * Copyright © 2010  Ding-Yi Chen <dchen at redhat.com>
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
 * @file MkdgModule.h
 * Module handling.
 *
 * Module handling in MakerDialog.
 *
 * @since 0.3
 */
#ifndef MKDG_MODULE_H_
#define MKDG_MODULE_H_
#include "MakerDialog.h"

typedef gboolean (* MkdgModuleInitFunc)(Mkdg *mDialog);

/**
 * MakerDialog supported modules.
 *
 * Thie enumeration list current MakerDialog supported modules.
 * Note that GKeyFile support is built-in, so it does not deem a module.
 * @since 0.3
 */
typedef enum{
    MKDG_MODULE_INVALID=-1,	//!< Invalid module.
    MKDG_MODULE_GCONF2,		//!< GConf2 module for configuration interface.
    MKDG_MODULE_GKEYFILE,	//!< GKeyFile module for configuration interface.
    MKDG_MODULE_GTK2,		//!< Gtk2 module for UI interface.
} MKDG_MODULE;

/**
 * Parse module name from a string.
 *
 * This function parses module name from a string.
 * @param moduleName The module name to be parsed.
 * @return Corresponding module; or \c MKDG_MODULE_INVALID if none
 * matched.
 * @since 0.3
 */
MKDG_MODULE mkdg_module_parse(const gchar *moduleName);

/**
 * Load a MakerDialog module.
 *
 * Load a MakerDialog module.
 * @param mDialog	A MakerDialog instance.
 * @param module 	The module to be loaded.
 * @param error		Error returned here.
 * @return \c TRUE if the module is loaded successfully, \c FALSE otherwise.
 * @since 0.3
 */
gboolean mkdg_module_load(Mkdg *mDialog, MKDG_MODULE module, MkdgError **error);

#endif /* MKDG_MODULE_H_ */
