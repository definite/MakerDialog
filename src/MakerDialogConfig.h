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
 * @file MakerDialogConfigHandler.h
 * Configuration file handler for MakerDialog.
 *
 * MakerDialog configuration file handler provide common interfaces for
 * manipulating configuration files, while encapsulates the detail of
 * configuration back-end, such as ini, gconf, kcfg.
 */

/**
 * DIRECTORY_SEPARATOR is the separator for splits the directories in paths.
 *
 * If WIN32 is defined, DIRECTORY_SEPARATOR is '\\',
 * otherwise '/' is used as DIRECTORY_SEPARATOR.
 */
#ifdef WIN32
#define DIRECTORY_SEPARATOR '\\'
#else
#define DIRECTORY_SEPARATOR '/'
#endif

/**
 * The page name for no page.
 *
 * If key does not have a page name associate with it, then
 * this name will be assigned to it,
 * because some of the configuration back-ends (such as glib KV file)
 * expect a page name associate with it.
 */
#define MAKER_DIALOG_CONFIG_NO_PAGE "_NO_PAGE_"

/**
 * Definition of configuration flags for turning the behavior of configuration handler and configure file.
 */
typedef enum{
    /**
     * Hide the NULL, empty string, and 0 in configure file.
     * This flag may be ignored if the configuration back-end does not support
     * it.
     */
    MAKER_DIALOG_CONFIG_FLAG_HIDE_EMPTY=	0x1,

    /**
     * Hide the default values in configure file.
     * This flag may be ignored if the configuration back-end does not support
     * it.
     */
    MAKER_DIALOG_CONFIG_FLAG_HIDE_DEFAULT=	0x2,

    /**
     * Open the config file as read-only.
     * This flag may be automatically set, if the file permission is
     * read-only.
     */
    MAKER_DIALOG_CONFIG_FLAG_READONLY=		0x4,

    /**
     * Latter read value will not override the former.
     *
     * By default, if different values are associate with same key in
     * difference locations in file, the latter values will replace the
     * former.
     *
     * This flag stops this behavior.
     */
    MAKER_DIALOG_CONFIG_FLAG_NO_OVERRIDE=	0x8,
} MakerDialogConfigFlag;

typedef enum{
    MAKER_DIALOG_CONFIG_OK= 	0,	//!< Ok, No error.
    MAKER_DIALOG_CONFIG_ERROR_CANT_READ,	//!< File cannot be read.
    MAKER_DIALOG_CONFIG_ERROR_CANT_WRITE, 	//!< File cannot written.
    MAKER_DIALOG_CONFIG_ERROR_NOT_FOUND, 	//!< File not found.
    MAKER_DIALOG_CONFIG_ERROR_INVALID_FORMAT, 	//!< File format is invalid.
    MAKER_DIALOG_CONFIG_ERROR_INVALID_PAGE, 	//!< Page is invalid (no such page).
    MAKER_DIALOG_CONFIG_ERROR_INVALID_KEY, 	//!< Key is invalid (no such key).
    MAKER_DIALOG_CONFIG_ERROR_INVALID_VALUE, 	//!< Value is invalid.
    MAKER_DIALOG_CONFIG_ERROR_NO_CONFIG_SET, 	//!< No configuration set is added.
    MAKER_DIALOG_CONFIG_ERROR_OTHER, 		//!< Other error.
} MakerDialogConfigError;

/**
 * Data structure that holds configuration flags.
 */
typedef guint MakerDialogConfigFlags;

/**
 * Data structure of a configure file set.
 *
 * A configuration set stores keys and values of configuration options under given pages.
 * The page names can be omitted this set includes all the configuration.
 *
 * A configuration set manages one or more configuration files.
 * This implementation fits for applications that have system-wide configuration as default,
 * yet users are able to override the default with their own configuration
 * file.
 *
 * Note that normally you don't have to directly use the members here.
 * The values of members will be filled when constructing MakerDialog config.
 *
 * These members are listed here for convenience for configuration handler developers.
 */
typedef struct{
    gchar **pageNames;			//!< Page names that this configuration set stores.
    MakerDialogConfigFlags flags; 	//!< Configuration flags.
    gchar *filePattern;			//!< Glob-formated file pattern. Or just the filename.
    gchar **searchDirs;			//!< Directories to be searched.
    gchar *defaultFilename;		//!< Default filename to use if filePattern cannot match any files in search directories.
    gint maxFileCount			//!< Maximum number of configuration files. -1 for find all matched.
    gint writeIndex;			//!< Index of first writable file. -1 if all read-only. This is not affected by the flag MAKER_DIALOG_CONFIG_FLAG_READONLY.
    GPtrArray *filenameArray;		//!< Pointer to filenames.
    GPtrArray *fileArray;		//!< Pointer to actual file handler, i.e. not the filename.
} MakerDialogConfigSet;

/**
 * Configuration file handler for MakerDialog.
 *
 * Note that normally you don't have to directly use the fields here.
 * The fields will be filled by calling corresponding configuration handler.
 *
 * These fields are listed here for convenience for
 * developer of configuration handlers.
 */
typedef struct{
    gchar listDelimiter; //!< Character that separate the list values.
    MakerDialogConfigFlags flags; //!< Configuration flags.

    /**
     * Callback function to initialize configuration backend.
     * Called by maker_dialog_config_init().
     */
    gboolean (* config_init)(MakerDialog *dlg, gchar listDelimiter, MakerDialogConfigFlags flags);

    /**
     * Callback function to open the configuration file.
     * Called by maker_dialog_config_open_file().
     */
    gboolean (* config_open_file)(MakerDialog *dlg, const gchar *configFile,
	    const gchar **searchPath, GError **error);

    /**
     * Callback function to open the configuration file.
     * Called by maker_dialog_config_open_file().
     */
    gboolean (* config_close)(MakerDialog *dlg);

} MakerDialogConfigFileHandler;

/**
 * Configuration of MakerDialog.
 *
 * Note that normally you don't have to directly use the members here.
 * The values of members will be filled when constructing MakerDialog config.
 *
 * These members are listed here for convenience for
 * developer of config handlers.
 */
typedef struct{
    MakerDialog *mDialog;			//!< Referring MakerDialog.
    MakerDialogConfigHandler configHandler;	//!< A configuration handler which connects to configuration back-end.
    /// @cond
    GHashTable *pageConfigSetTable;	//!< Hash table whose key is a page name, value is a MakerDialogConfigSet.
    GPtrArray  *configSetArray;		//!< Pointer array that hold the config sets.
    /// @endcond

} MakerDialogConfig;

/**
 * New a configuration set.
 *
 * This function new a configuration set for loading and saving configuration files.
 *
 * This function also finds files which match the constrain of configuration set by following steps:
 * - For each search directory, find all files that matched the file pattern.
 * - If none of these files are writable,  this functions will create a
 *   file with default name to the first writable search directory, unless
 *   MAKER_DIALOG_CONFIG_FLAG_READONLY is set.
 * - During previous steps, if maxFileCount >0, then this function stops
 *   searching files.
 *
 * Error code is returned via #errorCode.
 * MAKER_DIALOG_CONFIG_OK is returned if succeed. Otherwise non-zero error
 * code is returned. Pass NULL to ignore the error.
 *
 * @param pageNames 		Pages that this configure set contains; NULL if all pages goes to same file.
 * @param flags 		Configuration flags.
 * @param filePattern 		Glob-formated file pattern. Or just the filename.
 * @param searchDir		Directory to be searched.
 * @param defaultFilename	Default filename to use if filePattern cannot match any files in search directories.
 * @param maxiFileCount		Maximum number of configuration files. -1 for find all matched.
 * @param errorCode		Returned error code. NULL to ignore error. Pass NULL to ignore error.
 * @return A newly allocated MakerDialog configuration set; NULL if failed.
 * @see maker_dialog_config_set_close(), maker_dialog_config_add_config_set().
 * @see ::MakerDialogConfigError.
 */
MakerDialogConfigSet *maker_dialog_config_set_new(const gchar **pageNames,
	MakerDialogConfigFlag flags, const gchar *filePattern, const gchar **searchDirs,
	const gchar *defaultFilename, gint maxFileCount, MakerDialogConfigError *errorCode);

/**
 * Free a configuration set.
 *
 * This function free the allocated memory of configuration set.
 * @param dlgCfgSet A MakerDialog configuration set.
 */
void maker_dialog_config_set_free(MakerDialogConfigSet *dlgCfgSet);

/**
 * New a MakerDialogConfig.
 *
 * This function initializes an UI front-end using the given toolkit handler
 * for the MakerDialog.
 * This function constructs a configuration back-end using the given
 * configuration handler.
 *
 * During construction, the new MakerDialogConfig is associated to the #mDialog.
 * Thus, maker_dialog_destroy() can free the associated MakerDialogUi as well.
 *
 * This function is meant for configuration handler developers.
 * For KV file, gconf, kcfg users, it is more convenient to call
 * maker_dialog_config_use_kv(),
 * maker_dialog_ui_use_gconf() or maker_dialog_ui_use_kcfg() than using this
 * function directly.
 *
 * @param mDialog A MakerDialog.
 * @param configHandler A MakerDialog configuration handler.
 * @return A newly allocated MakerDialogConfig.
 */
MakerDialogConfig *maker_dialog_config_new(MakerDialog *mDialog, MakerDialogConfigHandler *configHandler);

/**
 * Free a MakerDialogConfig.
 *
 * This function frees a MakerDialogConfig instances.
 * Also close the configuration files.
 *
 * @param mDialog A MakerDialog.
 * @return A newly allocated MakerDialogConfig.
 */
void *maker_dialog_config_free(MakerDialogConfig *dlgCfg);

/**
 * Add a configuration set to MakerDialog.
 *
 * This function add a configuration set to MakerDialog,
 * so that applications can access the configuration set.
 *
 * @param mDialog A MakerDialog.
 * @param dlgCfgSet The configuration set to be added.
 * @see ::MakerDialogConfig.
 */
void maker_dialog_config_add_config_set(MakerDialog *mDialog, MakerDialogConfigSet *dlgCfgSet);

/**
 * Open all configuration set and associated files.
 *
 * This function opens all configuration set and associated files.
 * It will open files if they do not yet exist.
 *
 * Note that all configuration set should be added before this function.
 * @param mDialog A MakerDialog.
 * @return MAKER_DIALOG_CONFIG_OK if success; non-zero ::MakerDialogConfigError code otherwise.
 */
MakerDialogConfigError  maker_dialog_config_open_all(MakerDialog *mDialog);

/**
 * Close all configuration set and associated files.
 *
 * This function closes all configuration set and associated files.
 * @param mDialog A MakerDialog.
 */
void maker_dialog_config_close_all(MakerDialog *mDialog);

/**
 * Load all configuration options of MakerDialog from file.
 *
 * This function loads all configuration options of MakerDialog instance  from file.
 *
 * Note that at least one configure set need to be added before this function.
 * @param mDialog A MakerDialog.
 * @return MAKER_DIALOG_CONFIG_OK if success; non-zero ::MakerDialogConfigError code otherwise.
 */
MakerDialogConfigError maker_dialog_config_load_all(MakerDialog *mDialog);

/**
 * Save all configuration options of MakerDialog to file.
 *
 * This function saves all configuration options of MakerDialog instance  to file.
 *
 * Note that at least one configure set need to be added before this function.
 * @param mDialog A MakerDialog.
 * @return MAKER_DIALOG_CONFIG_OK if success; non-zero ::MakerDialogConfigError code otherwise.
 */
MakerDialogConfigError maker_dialog_config_save_all(MakerDialog *mDialog);

/**
 * Load a page of configuration options of MakerDialog from file.
 *
 * This function loads a page of configuration options of MakerDialog instance  from file.
 *
 * Note that at least one configure set need to be added before this function.
 * @param mDialog A MakerDialog.
 * @param pageName Page name to be loaded.
 *
 * @return MAKER_DIALOG_CONFIG_OK if success; non-zero ::MakerDialogConfigError code otherwise.
 */
MakerDialogConfigError maker_dialog_config_load_page(MakerDialog *mDialog, const gchar *pageName);

/**
 * Save a page of configuration options of MakerDialog to file.
 *
 * This function saves a pages of configuration options of MakerDialog instance  to file.
 * Note that at least one configure set need to be added before this function.
 * @param mDialog A MakerDialog.
 * @param pageName Page name to be loaded.
 *
 * @return MAKER_DIALOG_CONFIG_OK if success; non-zero ::MakerDialogConfigError code otherwise.
 */
MakerDialogConfigError maker_dialog_config_save_page(MakerDialog *mDialog, const gchar *pageName);

/**
 * Concatenate path strings.
 *
 * This function concatenate to path strings,
 * a \c DIRECTORY_SEPARATOR will be inserted between path string if needed.
 * Note \c NULL must be put at the tail as end of strings.
 *
 * @param path1 The first path string.
 * @param ... The rest path string. \c NULL must be put in the end.
 * @return A newly allocated string that stores combined pathes.
 */
gchar *maker_dialog_path_concat(gchar *path1, ....);

/**
 * Return the canonicalized absolute pathname.
 *
 * It works exactly the same with realpath(3), except this function can handle the path with ~,
 * where realpath cannot.
 *
 * @param path The path to be resolved.
 * @param resolved_path Buffer for holding the resolved_path.
 * @return resolved path, NULL is the resolution is not sucessful.
 */
gchar *maker_dialog_truepath(const gchar *path, gchar *resolved_path);



