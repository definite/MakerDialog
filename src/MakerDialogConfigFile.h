/*
 * Copyright © 2010  Red Hat, Inc. All rights reserved.
 * Copyright © 2010  Ding-Yi Chen <dchen at redhat.com>
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
 * @file MkdgConfigFile.h
 * Configuration File for Mkdg.
 *
 * A configuration file stores property values.
 *
 * The configuration back-end interface functions are defined here.
 *
 * @since 0.3; was included in MkdgConfig.h prior 0.2.
 */
#ifndef MKDG_CONFIG_FILE_H_
#define MKDG_CONFIG_FILE_H_
#include "MkdgConfigDef.h"

/**
 * Configuration file.
 *
 * Configuration file stores the path and the pointer to the file handle.
 */
struct _MkdgConfigFile{
    const gchar 		*path;			//!< Path to configuration file.
    MkdgConfigFileFlags	flags;			//!< Flags for configuration file.
    MkdgConfigSet	*configSet;		//!< Configuration set which manages this file.
    gpointer			fileObj;		//!< Configuration file instance.
    gpointer			userData;		//!< Extra user data that can be used to hold interface specific data.
};

/**
 * New a MakerDialog configuration file.
 *
 * New a MakerDialog configuration file.
 * @param path		//!< Path of the file.
 * @param configSet	//!< Associated configuration set.
 * @return A newly allocated MakerDialog configuration file.
 */
MkdgConfigFile *mkdg_config_file_new(const gchar *path, MkdgConfigSet *configSet);

/**
 * New a MakerDialog configuration file.
 *
 * New a MakerDialog configuration file.
 * @param path		//!< Path of the file.
 * @param flags		//!< Flags for configuration file.
 * @param configSet	//!< Associated configuration set.
 * @return A newly allocated MakerDialog configuration file.
 */
MkdgConfigFile *mkdg_config_file_new_full(const gchar *path, MkdgConfigFileFlags flags,MkdgConfigSet *configSet);


/**
 * Free a MakerDialog configuration file.
 *
 * Free a MakerDialog configuration file.
 * @param configFile	//!< The MakerDialog configuration file to be freed.
 */
void mkdg_config_file_free(MkdgConfigFile *configFile);

/**
 * MakerDialog configuration callback functions to be implemented.
 *
 * These callback functions connect to configuration back-end
 * to perform corresponding tasks.
 * These callback functions are called automatically, so no need
 * to call them directly.
 * @since 0.3
 */
struct _MkdgConfigFileInterface{
    /**
     * Initialize configuration set.
     *
     * This function initializes configuration set by
     * filling \a fileArray with available configuration files and
     * pointing \a writeIndex to proper file, that is:
     * - last writable file if if \c MKDG_CONFIG_NO_OVERRIDE is not set;
     * - or pointing to first writable file if \c MKDG_CONFIG_NO_OVERRIDE is set.
     * - -1 if no writable files or \c MKDG_CONFIG_READONLY is set.
     *
     * For non-file-based configuration back-end,
     * insert an empty configure file to \a fileArray,
     * and point \a writeIndex is set to:
     * - 0, if \c MKDG_CONFIG_READONLY is not set;
     * - or -1, if \c MKDG_CONFIG_READONLY is not set.
     *
     * @param configSet		A MakerDialog configuration set.
     * @param error		Error return location, or \c NULL.
     * @return \c TRUE if user is granted with the \a permission;\c FALSE otherwise.
     */
    gboolean (* config_set_init)(MkdgConfigSet *configSet, MkdgError **error);

    /**
     * Finalize configuration set.
     *
     * This function finalizes configuration set.
     * All file associated/opened by this configuration set should be close.
     * Associated instances should be freed as well.
     * @param configSet		A MakerDialog configuration set.
     */
    void (* config_set_finalize)(MkdgConfigSet *configSet);

    /**
     * Callback function that checks user's permission of a configuration file.
     *
     * This function connects to function that checks whether the current user
     * can access the file with given permission.
     *
     * Permission consist one or more following flags:
     * - F_OK: File exists.
     * - R_OK: File is readable.
     * - W_OK: File is writable.
     * - X_OK: File is executable.
     *
     * This function returns \c TRUE if user is granted with the \a permission;
     * \c FALSE otherwise.
     *
     * @param configFile	The configuration file to checked.
     * @param permission	The permission mask.
     * @param error		Error return location, or \c NULL.
     * @return \c TRUE if user is granted with the \a permission;\c FALSE otherwise.
     */
    gboolean (* config_file_can_access)(MkdgConfigFile *configFile, guint permission, MkdgError **error);

    /**
     * Callback function to create a new configuration file.
     *
     * This function connects to create function of the configuration back-end.
     * That is, it creates a file (or record) which does not yet exist.
     * \c FALSE is returned when the file already existed,
     * or failed to create the file.
     *
     * Called by mkdg_config_set_open().
     * @param configFile	The configuration file to be manipulated.
     * @param error		Error return location, or \c NULL.
     * @return \c TRUE if succeed; \c FALSE if the file already exists or failed to create.
     */
    gboolean (* config_file_create)(MkdgConfigFile *configFile, MkdgError **error);

    /**
     * Callback function to open a configuration file.
     *
     * This function connects to open function of the configuration back-end.
     * That is, it opens a file (or record) if it is readable.
     * \c FALSE is returned when the file is not readable,
     * or failed to open the file.
     *
     *
     * Called by mkdg_config_set_open().
     * @param configFile	The configuration file to be manipulated.
     * @param error		Error return location, or \c NULL.
     * @return \c TRUE if succeed; \c FALSE if the file does not exist or failed to open.
     */
    gboolean (* config_file_open)(MkdgConfigFile *configFile, MkdgError **error);

    /**
     * Callback function to close a configuration file.
     *
     * This function connects to close function of the configuration back-end.
     * Called by mkdg_config_set_close().
     * @param configFile	The configuration file to be manipulated.
     * @param error		Error return location, or \c NULL.
     * @return \c TRUE if succeed; \c FALSE otherwise.
     */
    gboolean (* config_file_close)(MkdgConfigFile *configFile, MkdgError **error);

    /**
     * Callback function to preload all property from a configuration file.
     *
     * This function connects to function that load all property from a
     * configuration file to a configuration buffer.
     * Called by mkdg_config_set_preload().
     *
     * @param configFile	The configuration file to be manipulated.
     * @param configBuf		The configuration buffer for storing the loaded property.
     * @param error		Error return location, or \c NULL.
     * @return \c TRUE if succeed; \c FALSE otherwise.
     * @see config_file_preload_page(), config_file_preload_property()
     */
    gboolean (* config_file_preload)(MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf, MkdgError **error);

    /**
     * Callback function to save all property from a configuration file.
     *
     * This function connects to function that save all property from a
     * configuration file.
     * Called by mkdg_config_set_preload().
     *
     * @param configFile	The configuration file to be manipulated.
     * @param error		Error return location, or \c NULL.
     * @return \c TRUE if succeed; \c FALSE otherwise.
     * @see config_file_preload_page(), config_file_preload_property()
     */
    gboolean (* config_file_save)(MkdgConfigFile *configFile, MkdgConfigBuffer *configBuf, MkdgError **error);

    /**
     * Callback function to get all pages in a configuration file.
     *
     * This function connects to function which retrieve all pages
     * from a configuration file.
     * Called by mkdg_config_get_pages().
     *
     * @param configFile	The configuration file to be manipulated.
     * @param error		Error return location, or \c NULL.
     * @return A newly allocated list of strings which contains results;  or \c NULL if no pages are found.
     * @see mkdg_config_get_pages(), config_file_get_keys(), mkdg_config_get_keys().
     */
    gchar ** (* config_file_get_pages)(MkdgConfigFile *configFile, MkdgError **error);

    /**
     * Callback function to get all keys of a page in a configuration file.
     *
     * This function connects to function which retrieve all keys in a page
     * from a configuration file.
     * Called by mkdg_config_get_keys().
     *
     * @param configFile	The configuration file to be manipulated.
     * @param pageName		Name of the page.
     * @param error		Error return location, or \c NULL.
     * @return A newly allocated list of strings which contains results;  or \c NULL if no keys are found.
     * @see config_get_pages(), mkdg_config_get_pages(), mkdg_config_get_keys().
     */
    gchar ** (* config_file_get_keys)(MkdgConfigFile *configFile, const gchar *pageName, MkdgError **error);

    /**
     * Callback function to get the value of associated key.
     *
     * This function connects to function which retrieve the value of the
     * given key from a configuration file.
     * Called by mkdg_config_get_value().
     *
     * @param configFile	The configuration file to be manipulated.
     * @param pageName		Name of the page.
     * @param key		Key.
     * @param valueType		Type of the value.
     * @param parseOption	Parse option. Use \c NULL for default handling.
     * @param error		Error return location, or \c NULL.
     * @return A newly allocated list of strings which contains results;  or \c NULL if no keys are found.
     * @see config_get_pages(), mkdg_config_get_pages(), mkdg_config_get_keys().
     */
    MkdgValue * (* config_file_get_value)(MkdgConfigFile *configFile, const gchar *pageName, const gchar *key,
	    MkdgType valueType, const gchar *parseOption, MkdgError **error);

};

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
 * @return A newly allocated MkdgConfigBuffer instance.
 */
MkdgConfigBuffer *mkdg_config_buffer_new();

/**
 * Insert a value to a MakerDialog config buffer.
 *
 * Insert a value to a MakerDialog config buffer.
 * @param configBuf 	A MakerDialog config buffer.
 * @param key		Property key to be inserted. This key will be duplicated in MakerDialog config buffer.
 * @param value		Value for the property.
 */
void mkdg_config_buffer_insert(MkdgConfigBuffer *configBuf, const gchar *key, MkdgValue *value);

/**
 * Lookup the associated value of a key in MakerDialog config buffer.
 *
 * Lookup the associated value of a key in MakerDialog config buffer.
 * @param configBuf 	A MakerDialog config buffer.
 * @param key		Property key to be inserted. This key will be duplicated in MakerDialog config buffer.
 * @return The associated value of a key, or \c NULL the no such key in the config buffer.
 */
MkdgValue *mkdg_config_buffer_lookup(MkdgConfigBuffer *configBuf, const gchar *key);

/**
 * Free a MakerDialog config buffer.
 *
 * Free a MakerDialog config buffer.
 *
 * This function is meant to be used internally,
 * so most people do not need to call it.
 * @param configBuf A MakerDialog config buffer.
 */
void mkdg_config_buffer_free(MkdgConfigBuffer *configBuf);

#endif /* MKDG_CONFIG_FILE_H_ */

