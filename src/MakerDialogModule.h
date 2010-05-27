/*
 * Copyright © 2010  Red Hat, Inc. All rights reserved.
 * Copyright © 2010  Ding-Yi Chen <dchen at redhat.com>
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
/**
 * @file MkdgModule.h
 * Module handling.
 *
 * Module handling in Mkdg.
 *
 * @since 0.3
 */
#ifndef MAKER_DIALOG_MODULE_H_
#define MAKER_DIALOG_MODULE_H_
#include "Mkdg.h"

typedef gboolean (* MkdgModuleInitFunc)(Mkdg *mDialog);

/**
 * Mkdg supported modules.
 *
 * Thie enumeration list current Mkdg supported modules.
 * Note that GKeyFile support is built-in, so it does not deem a module.
 * @since 0.3
 */
typedef enum{
    MAKER_DIALOG_MODULE_INVALID=-1,	//!< Invalid module.
    MAKER_DIALOG_MODULE_GCONF2,		//!< GConf2 module for configuration interface.
    MAKER_DIALOG_MODULE_GKEYFILE,	//!< GKeyFile module for configuration interface.
    MAKER_DIALOG_MODULE_GTK2,		//!< Gtk2 module for UI interface.
} MAKER_DIALOG_MODULE;

/**
 * Parse module name from a string.
 *
 * This function parses module name from a string.
 * @param moduleName The module name to be parsed.
 * @return Corresponding module; or \c MAKER_DIALOG_MODULE_INVALID if none
 * matched.
 * @since 0.3
 */
MAKER_DIALOG_MODULE maker_dialog_module_parse(const gchar *moduleName);

/**
 * Load a Mkdg module.
 *
 * Load a Mkdg module.
 * @param mDialog	A Mkdg instance.
 * @param module 	The module to be loaded.
 * @param error		Error returned here.
 * @return \c TRUE if the module is loaded successfully, \c FALSE otherwise.
 * @since 0.3
 */
gboolean maker_dialog_module_load(Mkdg *mDialog, MAKER_DIALOG_MODULE module, MkdgError **error);

#endif /* MAKER_DIALOG_MODULE_H_ */
