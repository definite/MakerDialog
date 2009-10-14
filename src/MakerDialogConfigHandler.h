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
     * Create the configure file if the file does not exist.
     * This flag is ignored if MAKER_DIALOG_CONFIG_FLAG_READONLY is set.
     */
    MAKER_DIALOG_CONFIG_FLAG_CREATE_IF_ABSENT=		0x8,

    /**
     * Return only the first matching result.
     *
     * If set, maker_dialog_config_open_file() and
     * maker_dialog_config_open_file_set() will only return the first matching file.
     *
     * For example, this flag can be set for if the application only need to
     * use system-wide setting, but not user customization.
     */
    MAKER_DIALOG_CONFIG_FLAG_RETURN_FIRST_ONLY=	0x10,

    /**
     * Latter read value will not replace the former.
     *
     * By default, if different values are associate with same key in
     * difference locations in file, the latter values will replace the
     * former.
     *
     * This flag stops this behavior.
     */
    MAKER_DIALOG_CONFIG_FLAG_NO_REPLACE=	0x20,
} MakerDialogConfigFlag;

typedef enum{
    MAKER_DIALOG_CONFIG_ERROR_NIL= 	0,	//!< No error.
    MAKER_DIALOG_CONFIG_ERROR_CANT_READ,	//!< File cannot be read.
    MAKER_DIALOG_CONFIG_ERROR_CANT_WRITE, 	//!< File cannot written.
    MAKER_DIALOG_CONFIG_ERROR_NOT_FOUND, 	//!< File not found.
    MAKER_DIALOG_CONFIG_ERROR_INVALID_FORMAT, 	//!< File format is invalid.
    MAKER_DIALOG_CONFIG_ERROR_INVALID_KEY, 	//!< Key is invalid (no such key).
    MAKER_DIALOG_CONFIG_ERROR_INVALID_VALUE, 	//!< Value is invalid.
} MakerDialogConfigError;


/**
 * Data structure that holds configuration flags.
 */
typedef guint MakerDialogConfigFlags;

/**
 * Data structure of a configure file.
 *
 * Note that normally you don't have to directly use the members here.
 * The values of members will be filled when constructing MakerDialog config.
 *
 * These members are listed here for convenience for
 * developer of config  handlers.
 */
typedef struct{
    MakerDialogConfigFlags flags; 	//!< Configuration flags.
    gchar *filePattern;			//!< Glob-formated file pattern. Or just the filename.
    gchar **searchDirs;			//!< Directories to be searched.
    GPtrArray *fileArray;		//!< Array of file handles, not the file name!.
    gint writeIndex;			//!< Index of first writable file. -1 if all read-only. This is not affected by the flag MAKER_DIALOG_CONFIG_FLAG_READONLY.
} MakerDialogConfigFile;

/**
 * Configuration of MakerDialog.
 *
 * Note that normally you don't have to directly use the members here.
 * The values of members will be filled when constructing MakerDialog config.
 *
 * These members are listed here for convenience for
 * developer of config  handlers.
 */
typedef struct{
    MakerDialog *dlg;			//!< Pointer to
    GHashTable *pageConfigTable;	//!< Hash table whose key is a page name, value is a MakerDialogConfigFile.
} MakerDialogConfig;

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
    gpointer confObj; //!< Object of the configuration handler, usually the file type.
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
