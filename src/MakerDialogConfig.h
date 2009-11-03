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
 * Configuration file handler for MakerDialog.
 *
 * MakerDialog configuration file handler provide common interfaces for
 * manipulating configuration files, while encapsulates the detail of
 * configuration back-end, such as ini, gconf, kcfg.
 */
#ifndef MAKER_DIALOG_CONFIG_H_
#define MAKER_DIALOG_CONFIG_H_
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
 * this name will be saved as page name.
 * because some of the configuration back-ends (such as glib KeyFile)
 * expect a page name associate with it.
 */
#define MAKER_DIALOG_CONFIG_NO_PAGE "_NO_PAGE_"

/**
 * Definition of configuration flags for turning the behavior of configuration handler and configure file.
 */
typedef enum{
    /**
     * Open the config file as read-only.
     * This flag may be automatically set, if the file permission is
     * read-only.
     */
    MAKER_DIALOG_CONFIG_FLAG_READONLY=		0x1,

    /**
     * Latter read value will not override the former.
     *
     * By default, if different values are associate with same key in
     * difference locations in file, the latter values will replace the
     * former.
     *
     * This flag stops this behavior.
     * Note that this flags changes the behavior of open and save functions.
     * Use this flags with care.
     */
    MAKER_DIALOG_CONFIG_FLAG_NO_OVERRIDE=	0x2,

    /**
     * Do not apply the loaded value.
     *
     * By default, after the new value is loaded,
     * the applyFunc() defined in property context will be called accordingly.
     *
     * This flag stops this behavior.
     */
    MAKER_DIALOG_CONFIG_FLAG_NO_APPLY=		0x4,

    /**
     * Stop on error.
     *
     * When encounter errors, the default behavior is trying to continue the loading.
     *
     * Set this flags to stop on error.
     */
    MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR=	0x8,

    /**
     * Hide the default values in configure file.
     * This flag may be ignored if the configuration back-end does not support
     * it.
     */
    MAKER_DIALOG_CONFIG_FLAG_HIDE_DEFAULT=	0x10,

    /**
     * Hide duplicated values in configure file.
     * This flag may be ignored if the configuration back-end does not support
     * it.
     */
    MAKER_DIALOG_CONFIG_FLAG_HIDE_DUPLICATE=	0x20,

} MakerDialogConfigFlag;

/**
 * Configuration error code enumeration.
 *
 * This enumeration lists all error codes for configuration file operation.
 */
typedef enum{
    MAKER_DIALOG_CONFIG_ERROR_PERMISSION_DENY,	//!< Permission denied; the file permissions do not allow the attempted operation.
    MAKER_DIALOG_CONFIG_ERROR_CANT_READ,	//!< File cannot be read.
    MAKER_DIALOG_CONFIG_ERROR_CANT_WRITE, 	//!< File cannot written.
    MAKER_DIALOG_CONFIG_ERROR_NOT_FOUND, 	//!< File not found.
    MAKER_DIALOG_CONFIG_ERROR_NO_CONFIG_SET, 	//!< No configuration set is added.
    MAKER_DIALOG_CONFIG_ERROR_INVALID_FORMAT, 	//!< File format is invalid.
    MAKER_DIALOG_CONFIG_ERROR_INVALID_PAGE, 	//!< Page is invalid (no such page).
    MAKER_DIALOG_CONFIG_ERROR_INVALID_KEY, 	//!< Key is invalid (no such key).
    MAKER_DIALOG_CONFIG_ERROR_INVALID_VALUE, 	//!< Value is invalid.
    MAKER_DIALOG_CONFIG_ERROR_OTHER, 		//!< Other error.
} MakerDialogConfigErrorCode;

/**
 * Error domain for configuration file processing.
 *
 * Errors in this domain will be from the ::MakerDialogConfigErrorCode enumeration.
 * See GError for information on error domains.
 */
#define MAKER_DIALOG_CONFIG_ERROR maker_dialog_config_error_quark()

/**
 * Data structure that holds configuration flags.
 */
typedef guint MakerDialogConfigFlags;

/**
 * Configuration buffer which stores the contents of pre-loaded configuration files.
 *
 * Configuration buffer stores the contents of pre-loaded configuration files.
 * This buffer can then be loaded to MakerDialog, or save only the modified
 * parts.
 */
typedef struct{
    GHashTable *keyValueTable;			//!< Hash table whose key is configuration key, value is associated value for that key.
} MakerDialogConfigBuffer;

/**
 * Configuration file.
 *
 * Configuration file stores the path and the pointer to the file handle.
 */
typedef struct{
    const gchar 	*path;			//!< Path to configuration file.
    gpointer		filePointer;		//!< Pointer to file.
} MakerDialogConfigFile;

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
 * The member writeIndex is pointing to last writable file if \c MAKER_DIALOG_CONFIG_NO_OVERRIDE is not set;
 * or pointing to first writable file if \c MAKER_DIALOG_CONFIG_NO_OVERRIDE is set;
 * or -1 if all file in configuration set is readonly.
 * Note this index is not affected by the flag MAKER_DIALOG_CONFIG_FLAG_READONLY.
 *
 * Note that normally you don't have to directly use the members here.
 * The values of members will be filled when constructing MakerDialog config.
 *
 * These members are listed here for convenience for configuration handler developers.
 */
typedef struct{
    gboolean			modified;		//!< Whether the configuration values are modified but unsaved.
    const gchar 		**pageNames;		//!< Page names that this configuration set stores.
    MakerDialogConfigFlags 	flags; 			//!< Configuration flags.
    const gchar 		*filePattern;		//!< Glob-formated file pattern. Or just the filename.
    const gchar 		**searchDirs;		//!< Directories to be searched.
    const gchar 		*defaultFilename;	//!< Default filename to use if filePattern cannot match any files in search directories.
    gint 			maxFileCount;		//!< Maximum number of configuration files. -1 for find all matched.
    gint 			writeIndex;		//!< Index of writable file. -1 if all read-only.
    GPtrArray 			*fileArray;		//!< Array of MakerDialogConfigFile.
    gint			currentIndex;		//!< Index for current working file. Non-negative values are real index value; while -1 is for not starting.
    gpointer			userData;		//!< Pass user data.
    /// @cond
    MakerDialog			*mDialog;		//!< Parent MakerDialog config.
    MakerDialogConfigBuffer	*dlgCfgBuf;		//!< Configure buffer.
    /// @endcond
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

    /**
     * Callback function to create a new configuration file.
     * Called by maker_dialog_config_open_all().
     */
    gpointer (* config_create)(MakerDialogConfigSet *dlgCfgSet, MakerDialogConfigFile *configFile, GError **error);

    /**
     * Callback function to open a configuration file.
     * Called by maker_dialog_config_open_all().
     */
    gpointer (* config_open)(MakerDialogConfigSet *dlgCfgSet, MakerDialogConfigFile *configFile, GError **error);

    /**
     * Callback function to close a configuration file.
     * Called by maker_dialog_config_close_all().
     */
    gboolean (* config_close)(MakerDialogConfigSet *dlgCfgSet, MakerDialogConfigFile *configFile, GError **error);

    /**
     * Callback function to pre-load configuration files.
     *
     * Pre-load function loads configuration files in to a configuration
     * buffer, namely ::MakerDialogConfigBuffer.
     *
     * The configuration back-end does not need to implement the load function,
     * instead, preload should be implemented.
     * This is because preload function benefits not only load function, but also save function,
     * as it enables the set differentiate operations for non-duplicate save.
     *
     * The full loading operation will be completed by maker_dialog_config_load_all() or  maker_dialog_config_load_page().
     *
     * Called by maker_dialog_config_load_all(),  maker_dialog_config_load_page(), maker_dialog_config_save_all(),  maker_dialog_config_save_page().
     */
    gboolean (* config_preload)(MakerDialogConfigSet *dlgCfgSet, MakerDialogConfigFile *configFile, const gchar *pageName, GError **error);

    /**
     * Callback function to load setting from a  configuration file.
     * Called by maker_dialog_config_save_all(),  maker_dialog_config_save_page().
     */
    gboolean (* config_save)(MakerDialogConfigSet *dlgCfgSet, MakerDialogConfigFile *configFile, const gchar *pageName, GError **error);

} MakerDialogConfigHandler;

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
    MakerDialogConfigHandler *configHandler;	//!< A configuration handler which connects to configuration back-end.
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
 * Error code is returned via \a error. Pass NULL to ignore error.
 *
 * @param pageNames 		Pages that this configure set contains; NULL if all pages goes to same file.
 * @param flags 		Configuration flags.
 * @param filePattern 		Glob-formated file pattern. Or just the filename.
 * @param searchDirs		Directory to be searched.
 * @param defaultFilename	Default filename to use if filePattern cannot match any files in search directories.
 * @param maxFileCount		Maximum number of configuration files. -1 for find all matched.
 * @param userData		Custom user data. Can be \c NULL.
 * @param error			Return location for a \c GError, or \c NULL.
 *
 * @return A newly allocated MakerDialog configuration set; NULL if failed.
 * @see maker_dialog_config_set_close(), maker_dialog_config_add_config_set().
 * @see ::MakerDialogConfigErrorCode.
 */
MakerDialogConfigSet *maker_dialog_config_set_new(const gchar **pageNames,
	MakerDialogConfigFlag flags, const gchar *filePattern, const gchar **searchDirs,
	const gchar *defaultFilename, gint maxFileCount, gpointer userData, GError **error);

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
 * This function constructs a configuration back-end using the given
 * configuration handler.
 *
 * During construction, the new MakerDialogConfig is associated to the \a mDialog.
 * Thus, maker_dialog_destroy() can free the associated MakerDialogConfig as well.
 *
 * This function is meant for configuration handler developers.
 * For GKeyFile, gconf, kcfg users, it is more convenient to call
 * maker_dialog_config_use_key_file(),
 * maker_dialog_ui_use_gconf() or maker_dialog_ui_use_kcfg() than using this
 * function directly.
 *
 * @param mDialog 	A MakerDialog.
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
 * @param dlgCfg 	A MakerDialog config.
 */
void maker_dialog_config_free(MakerDialogConfig *dlgCfg);

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
 * @param mDialog	A MakerDialog.
 * @param error		Return location for a \c GError, or \c NULL.
 * @return TRUE if success; FALSE otherwise.
 */
gboolean maker_dialog_config_open_all(MakerDialog *mDialog, GError **error);

/**
 * Close all configuration set and associated files.
 *
 * This function closes all configuration set and associated files.
 * @param mDialog	A MakerDialog.
 * @param error		Return location for a \c GError, or \c NULL.
 * @return TRUE if success; FALSE otherwise.
 */
gboolean maker_dialog_config_close_all(MakerDialog *mDialog, GError **error);

/**
 * Load all configuration options of MakerDialog from file.
 *
 * This function loads all configuration options of MakerDialog instance  from file.
 *
 * Note that at least one configure set need to be added before this function.
 * @param mDialog	A MakerDialog.
 * @param error		Return location for a \c GError, or \c NULL.
 * @return TRUE if success; FALSE otherwise.
 */
gboolean maker_dialog_config_load_all(MakerDialog *mDialog, GError **error);

/**
 * Save all configuration options of MakerDialog to file.
 *
 * This function saves all configuration options of MakerDialog instance  to file.
 *
 * Note that at least one configure set need to be added before this function.
 * @param mDialog	A MakerDialog.
 * @param error		Return location for a \c GError, or \c NULL.
 * @return TRUE if success; FALSE otherwise.
 */
gboolean maker_dialog_config_save_all(MakerDialog *mDialog, GError **error);

/**
 * Load a page of configuration options of MakerDialog from file.
 *
 * This function loads a page of configuration options of MakerDialog instance  from file.
 *
 * Note that at least one configure set need to be added before this function.
 * @param mDialog	A MakerDialog.
 * @param pageName 	Page name to be loaded.
 * @param error		Return location for a \c GError, or \c NULL.
 * @return TRUE if success; FALSE otherwise.
 *
 * @return MAKER_DIALOG_CONFIG_OK if success; non-zero ::MakerDialogConfigErrorCode code otherwise.
 */
gboolean maker_dialog_config_load_page(MakerDialog *mDialog, const gchar *pageName, GError **error);

/**
 * Save a page of configuration options of MakerDialog to file.
 *
 * This function saves a pages of configuration options of MakerDialog instance  to file.
 * Note that at least one configure set need to be added before this function.
 * @param mDialog 	A MakerDialog.
 * @param pageName 	Page name to be loaded.
 * @param error		Return location for a \c GError, or \c NULL.
 * @return TRUE if success; FALSE otherwise.
 */
gboolean maker_dialog_config_save_page(MakerDialog *mDialog, const gchar *pageName, GError **error);

/**
 * New a MakerDialog config buffer.
 *
 * @return A newly allocated MakerDialogConfigBuffer instance.
 */
MakerDialogConfigBuffer *maker_dialog_config_buffer_new();

/**
 * Free a MakerDialog config buffer.
 *
 * @param dlgCfgBuf A MakerDialog config buffer.
 */
void maker_dialog_config_buffer_free(MakerDialogConfigBuffer *dlgCfgBuf);

/**
 * GQuark for MakerDialog configuration domain.
 *
 * GQuark for MakerDialog configuration domain.
 * @returns GQuark for MakerDialog configuration domain.
 */
GQuark maker_dialog_config_error_quark (void);

/**
 * New a MakerDialog configuration error.
 *
 * New a MakerDialog configuration error.
 * @param code 		Error code.
 * @param prefix	Prefix of the error message. Can be \c NULL.
 * @return A newly allocated MakerDialog config error instance.
 */
GError *maker_dialog_config_error_new(MakerDialogConfigErrorCode code, const gchar *prefix);

/**
 * Print a MakerDialog configuration error message.
 *
 * Print a MakerDialog configuration error message in following format:
 * @code
 * [WW] domain:\<domain\> [\<code\>] \<message\>
 * @endcode
 *
 * @param error 	Error to be printed.
 */
void maker_dialog_config_error_print(GError *error);


/**
 * Whether the file is writable or can be created.
 *
 * This function returns TRUE when \a filename is writable,
 * or it does not but the parent directory is writable.
 * Returns FALSE otherwise.
 *
 * @param filename Filename to be tested.
 * @return TRUE for the file is writable or can be created; FALSE otherwise.
 */
gboolean maker_dialog_file_isWritable(const gchar *filename);

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


#endif /* MAKER_DIALOG_CONFIG_H_ */

