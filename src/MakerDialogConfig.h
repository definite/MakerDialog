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
 * Configuration module for MakerDialog.
 *
 * This module defines configuration relative data structure, functions,
 * and an interface for bridging between MakerDialog and configuration
 * back-end such as GKeyFile, GConf, KCfg.
 *
 * In MakerDialog, a property is a configuration option which associate with a
 * value. According to property specification, MakerDialog loads and saves
 * property values to configuration files.
 */
#ifndef MAKER_DIALOG_CONFIG_H_
#define MAKER_DIALOG_CONFIG_H_

/**
 * Definition of configuration flags for turning the behavior of configuration interface and configure file.
 */
typedef enum{
    /**
     * The configuration back-end is not file based.
     *
     * This flag is automatically set and intended for internal use.
     */
    MAKER_DIALOG_CONFIG_FLAG_NOT_FILE_BASE=	0x1,

    /**
     * Open the config file as read-only.
     * This flag may be automatically set, if the file permission is
     * read-only.
     */
    MAKER_DIALOG_CONFIG_FLAG_READONLY=		0x2,

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
    MAKER_DIALOG_CONFIG_FLAG_NO_OVERRIDE=	0x4,

    /**
     * Do not apply the loaded value.
     *
     * By default, after the new value is loaded,
     * the applyFunc() defined in property context will be called accordingly.
     *
     * This flag stops this behavior.
     */
    MAKER_DIALOG_CONFIG_FLAG_NO_APPLY=		0x8,

    /**
     * Stop on error.
     *
     * When encounter errors, the default behavior is trying to continue the loading.
     *
     * Set this flags to stop on error.
     */
    MAKER_DIALOG_CONFIG_FLAG_STOP_ON_ERROR=	0x10,

    /**
     * Hide the default values in configure file.
     * This flag may be ignored if the configuration back-end does not support
     * it.
     */
    MAKER_DIALOG_CONFIG_FLAG_HIDE_DEFAULT=	0x20,

    /**
     * Hide duplicated values in configure file.
     * This flag may be ignored if the configuration back-end does not support
     * it.
     */
    MAKER_DIALOG_CONFIG_FLAG_HIDE_DUPLICATE=	0x40,

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
 * These members are listed here for convenience for configuration interface developers.
 */
typedef struct{
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
    MakerDialogConfigBuffer	*configBuf;		//!< Configure buffer.
    /// @endcond
} MakerDialogConfigSet;

/**
 * Configuration file interface for MakerDialog.
 *
 * Note that normally you don't have to directly use the fields here.
 * The fields will be filled by calling corresponding configuration interface.
 *
 * These fields are listed here for convenience for
 * developer of configuration interfaces.
 */
typedef struct{

    /**
     * Callback function to create a new configuration set.
     *
     * Called by maker_dialog_config_open_all().
     * @param configSet		A configuration set.
     * @param configFile	The configuration file to be manipulated;
     *  or \c NULL if the configuration back-end is not file based.
     * @param error		Error return location, or \c NULL.
     * @return File object if success, \c NULL if error occurs.
     */
    gpointer (* config_create)(MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, GError **error);

    /**
     * Callback function to open a configuration set.
     * Called by maker_dialog_config_open_all().
     * @param configSet		A configuration set.
     * @param configFile	The configuration file to be manipulated;
     *  or \c NULL if the configuration back-end is not file based.
     * @param error		Error return location, or \c NULL.
     * @return File object if success, \c NULL if error occurs.
     */
    gpointer (* config_open)(MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, GError **error);

    /**
     * Callback function to close a configuration set.
     * Called by maker_dialog_config_close_all().
     * @param configSet		A configuration set.
     * @param configFile	The configuration file to be manipulated;
     *  or \c NULL if the configuration back-end is not file based.
     * @param error		Error return location, or \c NULL.
     * @return File object if success, \c NULL if error occurs.
     */
    gboolean (* config_close)(MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, GError **error);

    /**
     * Callback function to pre-load property from configuration set.
     *
     * Pre-load function loads configuration files in to a configuration
     * buffer, namely ::MakerDialogConfigBuffer.
     *
     * The configuration back-end does not need to implement the load function,
     * instead, preload should be implemented.
     * This is because preload function benefits not only load function, but also save function,
     * as it enables the set differentiate operations for non-duplicate save.
     *
     * The full loading operation will be completed by maker_dialog_config_load_all()
     * or maker_dialog_config_load_page().
     *
     * Called by maker_dialog_config_load_all(), maker_dialog_config_load_page(),
     * maker_dialog_config_save_all(),  maker_dialog_config_save_page().
     * @param configSet		A configuration set.
     * @param configFile	The configuration file to be manipulated;
     *  or \c NULL if the configuration back-end is not file based.
     * @param error		Error return location, or \c NULL.
     * @return File object if success, \c NULL if error occurs.
     */
    gboolean (* config_preload)(MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, const gchar *pageName, GError **error);

    /**
     * Callback function to load setting from a configuration set.
     *
     * Called by maker_dialog_config_save_all(),  maker_dialog_config_save_page().
     * @param configSet		A configuration set.
     * @param configFile	The configuration file to be manipulated;
     *  or \c NULL if the configuration back-end is not file based.
     * @param error		Error return location, or \c NULL.
     * @return File object if success, \c NULL if error occurs.
     */
    gboolean (* config_save)(MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, const gchar *pageName, GError **error);

    /**
     * Callback function to list all pages in a configuration set.
     *
     * Called by maker
     * @param configSet		A configuration set.
     * @param configFile	The configuration file to be manipulated;
     *  or \c NULL if the configuration back-end is not file based.
     * @param error		Error return location, or \c NULL.
     * @return File object if success, \c NULL if error occurs.
     */
    gchar * (* config_get_pages)(MakerDialogConfigSet *configSet, MakerDialogConfigFile *configFile, const gchar *pageName, GError **error);


} MakerDialogConfigInterface;

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
    MakerDialog *mDialog;			//!< Referring MakerDialog.
    MakerDialogConfigInterface *configInterface;	//!< A configuration interface which connects to configuration back-end.
    /// @cond
    GHashTable *pageSetTable;		//!< Hash table whose key is a page name, value is a MakerDialogConfigSet.
    GPtrArray  *setArray;		//!< Pointer array that hold the config sets.
    gboolean fileBased;			//!< The config back-end is file based.
    /// @endcond

} MakerDialogConfig;


/**
 * New a configuration set.
 *
 * This function new a configuration set for loading and saving configuration files.
 *
 * @param flags 		Configuration flags.
 *
 * @return A newly allocated MakerDialog configuration set; NULL if failed.
 * @see maker_dialog_config_set_close(), maker_dialog_config_add_config_set().
 * @see ::MakerDialogConfigErrorCode.
 */
MakerDialogConfigSet *maker_dialog_config_set_new(MakerDialogConfigFlag flags);

/**
 * New a configuration set with all available arguments.
 *
 * This function new a configuration set for loading and saving configuration files.
 *
 * @param pageNames 		Pages that this configure set contains; NULL if all pages goes to same file.
 * @param flags 		Configuration flags.
 * @param filePattern 		Glob-formated file pattern. Or just the filename.
 * @param searchDirs		Directory to be searched.
 * @param defaultFilename	Default filename to use if filePattern cannot match any files in search directories.
 * @param maxFileCount		Maximum number of configuration files. -1 for find all matched.
 * @param userData		Custom user data. Can be \c NULL.
 *
 * @return A newly allocated MakerDialog configuration set; NULL if failed.
 * @see maker_dialog_config_set_close(), maker_dialog_config_add_config_set().
 * @see ::MakerDialogConfigErrorCode.
 */
MakerDialogConfigSet *maker_dialog_config_set_new_full(const gchar **pageNames,
	MakerDialogConfigFlag flags, const gchar *filePattern, const gchar **searchDirs,
	const gchar *defaultFilename, gint maxFileCount, gpointer userData);

/**
 * Free a configuration set.
 *
 * This function free the allocated memory of configuration set.
 * @param configSet A MakerDialog configuration set.
 */
void maker_dialog_config_set_free(MakerDialogConfigSet *configSet);

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
 * @param fileBased	The config back-end is file based.
 * @param configInterface A MakerDialog configuration interface.
 * @return A newly allocated MakerDialogConfig.
 */
MakerDialogConfig *maker_dialog_config_new(
	MakerDialog *mDialog, gboolean fileBased, MakerDialogConfigInterface *configInterface);

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
void maker_dialog_config_add_config_set(MakerDialogConfig *config, MakerDialogConfigSet *configSet, GError **error);

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
gboolean maker_dialog_config_open_all(MakerDialogConfig *config, GError **error);

/**
 * Close all configuration set and associated files.
 *
 * This function closes all configuration set and associated files.
 * @param config	A MakerDialog config instance.
 * @param error		Error return location, or \c NULL.
 * @return TRUE if success; FALSE otherwise.
 */
gboolean maker_dialog_config_close_all(MakerDialogConfig *config, GError **error);

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
gboolean maker_dialog_config_load_all(MakerDialogConfig *config, GError **error);

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
gboolean maker_dialog_config_save_all(MakerDialogConfig *config, GError **error);

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
gboolean maker_dialog_config_load_page(MakerDialogConfig *config, const gchar *pageName, GError **error);

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
gboolean maker_dialog_config_save_page(MakerDialogConfig *config, const gchar *pageName, GError **error);

/**
 * Prototype of callback function for configuration set for-each page
 * function.
 *
 * This callback prototype is for maker_dialog_config_set_foreach_page().
 * It is called for each page in \a configSet.
 * @param configSet 	A MakerDialog configure set.
 * @param page 		Page in \a configSet.
 * @param userData	Custom data to be passed to this callback.
 * @param error		Error return location, or \c NULL.
 * @return TRUE to continue iteration; FALSE to stop.
 */
typedef gboolean (* MakerDialogConfigSetEachPageFunc)(MakerDialogConfigSet *configSet, const gchar *page, gpointer userData, GError **error);

/**
 * Run a callback for each page in a configure set.
 *
 * This function calls the callback function for each page in a configure set.
 * It stops and returns \c FALSE if the callback function returns FALSE;
 * otherwise it runs through all the pages in configure set.
 *
 * @param configSet 	A MakerDialog configure set.
 * @param func		Callback function.
 * @param userData	Custom data to be passed to \a func.
 * @param error		Error return location, or \c NULL.
 * @return TURE if all callback returns TRUE; FALSE if one or more callback
 * return FALSE.
 */
gboolean maker_dialog_config_set_foreach_page(MakerDialogConfigSet *configSet,
	MakerDialogConfigSetEachPageFunc func,  gpointer userData, GError **error);


/**
 * New a MakerDialog config buffer.
 *
 * This function news a MakerDialog config buffer.
 * MakerDialog config buffer is a buffer of property key and value pairs.
 * The buffer is filled in preload stage,
 * and freed after load or save stage.
 *
 * This function is meant to be used internally,
 * so most people do not need to call it.
 * @return A newly allocated MakerDialogConfigBuffer instance.
 */
MakerDialogConfigBuffer *maker_dialog_config_buffer_new();

/**
 * Insert a value to a MakerDialog config buffer.
 *
 * Insert a value to a MakerDialog config buffer.
 * @param configBuf 	A MakerDialog config buffer.
 * @param key		Property key to be inserted. This key will be duplicated in MakerDialog config buffer.
 * @param value		Value for the property.
 */
void maker_dialog_config_buffer_insert(MakerDialogConfigBuffer *configBuf, const gchar *key, MkdgValue *value);

/**
 * Free a MakerDialog config buffer.
 *
 * Free a MakerDialog config buffer.
 *
 * This function is meant to be used internally,
 * so most people do not need to call it.
 * @param configBuf A MakerDialog config buffer.
 */
void maker_dialog_config_buffer_free(MakerDialogConfigBuffer *configBuf);

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
 * @param formatStr	printf() format string.
 * @param ...		Argument for formatStr.
 * @return A newly allocated MakerDialog config error instance.
 */
MakerDialogError *maker_dialog_config_error_new(MakerDialogConfigErrorCode code, const gchar *formatStr, ...);

#endif /* MAKER_DIALOG_CONFIG_H_ */

