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
 * @file MkdgConfig.h
 * Configuration back-end interface for Mkdg.
 *
 * This configuration interface bridges between MakerDialog and configuration back-end
 * such as GKeyFile, GConf, and KCfg.
 *
 * Through the configuration interface,
 * MakerDialog loads and saves
 * property values from and to configuration files.
 */
#ifndef MKDG_CONFIG_H_
#define MKDG_CONFIG_H_
#include "MakerDialogConfigDef.h"

/**
 * Configuration of Mkdg.
 *
 * Note that normally you don't have to directly use the members here.
 * The values of members will be filled when constructing MakerDialog config.
 *
 * These members are listed here for convenience for
 * developer of config interfaces.
 */
typedef struct{
    Mkdg				*mDialog;		//!< Referring Mkdg.
    MkdgConfigFlags	 	flags; 			//!< Global configuration flags. It maybe override by set specifc flags.
    MkdgConfigFileInterface	*configInterface;	//!< Global configure file interface.  It maybe override by set specifc interfaces.
    /// @cond
    GHashTable *pageSetTable;		//!< Hash table whose key is a page name, value is a MkdgConfigSet.
    GPtrArray  *setArray;		//!< Pointer array that hold the config sets.
    /// @endcond
} MkdgConfig;

/**
 * New a MkdgConfig.
 *
 * This function allocates an empty MkdgConfig instances.
 * It is equivalent of  mkdg_config_new_full(mDialog, 0, NULL);
 *
 * @param mDialog 	A MakerDialog instance.
 * @param flags 	Configuration flags.
 * @param configInterface A MakerDialog configuration interface.
 * @return A newly allocated MkdgConfig.
 * @see mkdg_config_new_full().
 * @since Modified in 0.3.
 */
 MkdgConfig *mkdg_config_new(Mkdg *mDialog);

/**
 * New a MkdgConfig.
 *
 * This function constructs a configuration back-end using the given
 * configuration interface.
 *
 * During construction, the new MkdgConfig is associated to the \a mDialog.
 * Thus, mkdg_destroy() can free the associated MkdgConfig as well.
 *
 * This function is meant for configuration interface developers.
 * For GKeyFile, gconf, kcfg users, it is more convenient to call
 * mkdg_config_use_key_file(),
 * mkdg_ui_use_gconf() or mkdg_ui_use_kcfg() than using this
 * function directly.
 *
 * @param mDialog 	A MakerDialog instance.
 * @param flags 	Configuration flags.
 * @param configInterface A MakerDialog configuration interface.
 * @return A newly allocated MkdgConfig.
 * @see mkdg_config_new().
 * @since 0.3
 */
MkdgConfig *mkdg_config_new_full(
	Mkdg *mDialog,
	MkdgConfigFlags flags, MkdgConfigFileInterface *configInterface);

/**
 * Free a MkdgConfig.
 *
 * This function frees a MkdgConfig instances.
 * Also close the configuration files.
 *
 * @param config	A MakerDialog config instance.
 */
void mkdg_config_free(MkdgConfig *config);

/**
 * Add a configuration set to Mkdg.
 *
 * This function add a configuration set to Mkdg,
 * so that applications can access the configuration set.
 *
 * This function also finds files which match the constrain of configuration set by following steps:
 * - For each search directory, find all files that matched the file pattern.
 * - If none of these files are writable,  this functions will create a
 *   file with default name to the first writable search directory, unless
 *   MKDG_CONFIG_FLAG_READONLY is set.
 * - During previous steps, if maxFileCount >0, then this function stops
 *   searching files.
 *
 * Error code is returned via \a error. Pass NULL to ignore error.
 * @param config 	A MakerDialog config.
 * @param configSet 	The configuration set to be added.
 * @param error		Error return location, or \c NULL.
 * @see ::MkdgConfig.
 */
void mkdg_config_add_config_set(MkdgConfig *config, MkdgConfigSet *configSet, MkdgError **error);

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
gboolean mkdg_config_open_all(MkdgConfig *config, MkdgError **error);

/**
 * Close all configuration set and associated files.
 *
 * This function closes all configuration set and associated files.
 * @param config	A MakerDialog config instance.
 * @param error		Error return location, or \c NULL.
 * @return TRUE if success; FALSE otherwise.
 */
gboolean mkdg_config_close_all(MkdgConfig *config, MkdgError **error);

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
gboolean mkdg_config_load_all(MkdgConfig *config, MkdgError **error);

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
gboolean mkdg_config_save_all(MkdgConfig *config, MkdgError **error);

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
 * @return MKDG_CONFIG_OK if success; non-zero ::MkdgConfigErrorCode code otherwise.
 */
gboolean mkdg_config_load_page(MkdgConfig *config, const gchar *pageName, MkdgError **error);

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
gboolean mkdg_config_save_page(MkdgConfig *config, const gchar *pageName, MkdgError **error);

/**
 * Return all pages in configuration back-end.
 *
 * This function returns all pages which are stored in configuration back-end.
 *
 * @param config	The configuration back-end.
 * @param error		Error return location, or \c NULL.
 * @return A newly allocated list of strings which contains results;  or \c NULL if no pages are found.
 */
gchar **mkdg_config_get_pages(MkdgConfig *config, MkdgError **error);


/**
 * Return all keys in a page of the configuration back-end.
 *
 * This function returns all keys in configuration back-end of a given page.
 *
 * @param config	The configuration back-end.
 * @param pageName	Name of the page.
 * @param error		Error return location, or \c NULL.
 * @return A newly allocated list of strings which contains results;  or \c NULL if no keys are found.
 */
gchar **mkdg_config_get_keys(MkdgConfig *config, const gchar *pageName, MkdgError **error);

/**
 * Return the value of a key in configuration back-end.
 *
 * This function traverses all configuration files and returns the value of
 * the key.
 *
 * If \c MKDG_CONFIG_FLAG_NO_OVERRIDE is set, the first corresponding value is returned,
 * otherwise the last value is returned.
 *
 * Note that if MKDG_CONFIG_FLAG_NO_OVERRIDE
 *
 * @param config	The configuration back-end.
 * @param pageName	Name of the page.
 * @param error		Error return location, or \c NULL.
 * @return A newly allocated MkdgValue that store the value;  or \c NULL if the key is not in the configuration back-end.
 */
MkdgValue *mkdg_config_get_value(MkdgConfig *config, const gchar *pageName, const gchar *key,
	MkdgType valueType, const gchar *parseOption, MkdgError **error);

/**
 * Parse the configuration flags from a string.
 *
 * Parse the configuration flags from a string.
 * @param str		String to be parsed.
 * @return Parsed configuration flag; or \c 0 if none matched.
 */
MkdgConfigFlags mkdg_config_flags_parse(const gchar *str);

/**
 * GQuark for MakerDialog configuration domain.
 *
 * GQuark for MakerDialog configuration domain.
 * @returns GQuark for MakerDialog configuration domain.
 */
GQuark mkdg_config_error_quark (void);

#endif /* MKDG_CONFIG_H_ */

