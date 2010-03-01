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
 * @file MakerDialogConfig.h
 * Configuration back-end interface for MakerDialog.
 *
 * This configuration interface bridges between MakerDialog and configuration back-end
 * such as GKeyFile, GConf, and KCfg.
 *
 * Through the configuration interface,
 * MakerDialog loads and saves
 * property values from and to configuration files.
 */
#ifndef MAKER_DIALOG_CONFIG_H_
#define MAKER_DIALOG_CONFIG_H_
#include "MakerDialogConfigDef.h"

/**
 * Configuration of MakerDialog.
 *
 * Note that normally you don't have to directly use the members here.
 * The values of members will be filled when constructing MakerDialog config.
 *
 * These members are listed here for convenience for
 * developer of config interfaces.
 */
typedef struct{
    MakerDialog				*mDialog;		//!< Referring MakerDialog.
    MakerDialogConfigFlags	 	flags; 			//!< Global configuration flags. It maybe override by set specifc flags.
    MakerDialogConfigFileInterface	*configInterface;	//!< Global configure file interface.  It maybe override by set specifc interfaces.
    /// @cond
    GHashTable *pageSetTable;		//!< Hash table whose key is a page name, value is a MakerDialogConfigSet.
    GPtrArray  *setArray;		//!< Pointer array that hold the config sets.
    /// @endcond
} MakerDialogConfig;

/**
 * New a MakerDialogConfig.
 *
 * This function allocates an empty MakerDialogConfig instances.
 * It is equivalent of  maker_dialog_config_new_full(mDialog, 0, NULL);
 *
 * @param mDialog 	A MakerDialog instance.
 * @param flags 	Configuration flags.
 * @param configInterface A MakerDialog configuration interface.
 * @return A newly allocated MakerDialogConfig.
 * @see maker_dialog_config_new_full().
 * @since Modified in 0.3.
 */
 MakerDialogConfig *maker_dialog_config_new(MakerDialog *mDialog);

/**
 * New a MakerDialogConfig.
 *
 * This function constructs a configuration back-end using the given
 * configuration interface.
 *
 * During construction, the new MakerDialogConfig is associated to the \a mDialog.
 * Thus, maker_dialog_destroy() can free the associated MakerDialogConfig as well.
 *
 * This function is meant for configuration interface developers.
 * For GKeyFile, gconf, kcfg users, it is more convenient to call
 * maker_dialog_config_use_key_file(),
 * maker_dialog_ui_use_gconf() or maker_dialog_ui_use_kcfg() than using this
 * function directly.
 *
 * @param mDialog 	A MakerDialog instance.
 * @param flags 	Configuration flags.
 * @param configInterface A MakerDialog configuration interface.
 * @return A newly allocated MakerDialogConfig.
 * @see maker_dialog_config_new().
 * @since 0.3
 */
MakerDialogConfig *maker_dialog_config_new_full(
	MakerDialog *mDialog,
	MakerDialogConfigFlags flags, MakerDialogConfigFileInterface *configInterface);

/**
 * Free a MakerDialogConfig.
 *
 * This function frees a MakerDialogConfig instances.
 * Also close the configuration files.
 *
 * @param config	A MakerDialog config instance.
 */
void maker_dialog_config_free(MakerDialogConfig *config);

/**
 * Add a configuration set to MakerDialog.
 *
 * This function add a configuration set to MakerDialog,
 * so that applications can access the configuration set.
 *
 * This function also finds files which match the constrain of configuration set by following steps:
 * - For each search directory, find all files that matched the file pattern.
 * - If none of these files are writable,  this functions will create a
 *   file with default name to the first writable search directory, unless
 *   MAKER_DIALOG_CONFIG_FLAG_READONLY is set.
 * - During previous steps, if maxFileCount >0, then this function stops
 *   searching files.
 *
 * Error code is returned via \a error. Pass NULL to ignore error.
 * @param config 	A MakerDialog config.
 * @param configSet 	The configuration set to be added.
 * @param error		Error return location, or \c NULL.
 * @see ::MakerDialogConfig.
 */
void maker_dialog_config_add_config_set(MakerDialogConfig *config, MakerDialogConfigSet *configSet, MakerDialogError **error);

/**
 * Open all configuration set and associated files.
 *
 * This function opens all configuration set and associated files.
 * It will open files if they do not yet exist.
 *
 * Note that all configuration set should be added before this function.
 * @param config	A MakerDialog config instance.
 * @param error		Error return location, or \c NULL.
 * @return TRUE if success; FALSE otherwise.
 */
gboolean maker_dialog_config_open_all(MakerDialogConfig *config, MakerDialogError **error);

/**
 * Close all configuration set and associated files.
 *
 * This function closes all configuration set and associated files.
 * @param config	A MakerDialog config instance.
 * @param error		Error return location, or \c NULL.
 * @return TRUE if success; FALSE otherwise.
 */
gboolean maker_dialog_config_close_all(MakerDialogConfig *config, MakerDialogError **error);

/**
 * Load all configuration options of MakerDialog from file.
 *
 * This function loads all configuration options of MakerDialog instance  from file.
 *
 * Note that at least one configure set need to be added before this function.
 * @param config	A MakerDialog config instance.
 * @param error		Error return location, or \c NULL.
 * @return TRUE if success; FALSE otherwise.
 */
gboolean maker_dialog_config_load_all(MakerDialogConfig *config, MakerDialogError **error);

/**
 * Save all configuration options of MakerDialog to file.
 *
 * This function saves all configuration options of MakerDialog instance  to file.
 *
 * Note that at least one configure set need to be added before this function.
 * @param config	A MakerDialog config instance.
 * @param error		Error return location, or \c NULL.
 * @return TRUE if success; FALSE otherwise.
 */
gboolean maker_dialog_config_save_all(MakerDialogConfig *config, MakerDialogError **error);

/**
 * Load a page of configuration options of MakerDialog from file.
 *
 * This function loads a page of configuration options of MakerDialog instance  from file.
 *
 * Note that at least one configure set need to be added before this function.
 * @param config	A MakerDialog config instance.
 * @param pageName 	Page name to be loaded.
 * @param error		Error return location, or \c NULL.
 * @return TRUE if success; FALSE otherwise.
 *
 * @return MAKER_DIALOG_CONFIG_OK if success; non-zero ::MakerDialogConfigErrorCode code otherwise.
 */
gboolean maker_dialog_config_load_page(MakerDialogConfig *config, const gchar *pageName, MakerDialogError **error);

/**
 * Save a page of configuration options of MakerDialog to file.
 *
 * This function saves a pages of configuration options of MakerDialog instance  to file.
 * Note that at least one configure set need to be added before this function.
 * @param config	A MakerDialog config instance.
 * @param pageName 	Page name to be loaded.
 * @param error		Error return location, or \c NULL.
 * @return TRUE if success; FALSE otherwise.
 */
gboolean maker_dialog_config_save_page(MakerDialogConfig *config, const gchar *pageName, MakerDialogError **error);

/**
 * Parse the configuration flags from a string.
 *
 * Parse the configuration flags from a string.
 * @param str		String to be parsed.
 * @return Parsed configuration flag; or \c 0 if none matched.
 */
MakerDialogConfigFlags maker_dialog_config_flags_parse(const gchar *str);

/**
 * GQuark for MakerDialog configuration domain.
 *
 * GQuark for MakerDialog configuration domain.
 * @returns GQuark for MakerDialog configuration domain.
 */
GQuark maker_dialog_config_error_quark (void);

#endif /* MAKER_DIALOG_CONFIG_H_ */

