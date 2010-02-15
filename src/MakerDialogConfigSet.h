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
 * @file MakerDialogConfigSet.h
 * Configuration Set for MakerDialog.
 *
 * A configuration set is a mandatory unit of MakerDialog configuration mechanism,
 * it manages pages of properties and configuration files
 * for storing the values of those properties.
 *
 * Same properties can reside in different configuration files in MakerDialog.
 * So applications can have system-wide configuration as default,
 * yet users are able to override the default with their own configuration file.
 *
 * Configuration sets are mandatory for using configuration back-end.
 * They can either be defined in source codes or MakerDialog spec files.
 * Latter is recommend, as it is not only easier to read,
 * but also consistent should API changed.
 *
 * @since 0.3; was included in MakerDialogConfig.h prior 0.2.
 */
#ifndef MAKER_DIALOG_CONFIG_SET_H_
#define MAKER_DIALOG_CONFIG_SET_H_
#include "MakerDialogConfigDef.h"

/**
 * @cond
 */
typedef enum{
    MAKER_DIALOG_CONFIG_SET_STATUS_HAS_LOCAL_SETTING=	0x1, //!< Whether the configure set is prepared.
    MAKER_DIALOG_CONFIG_SET_STATUS_IS_PREPARED=		0x2, //!< Whether the configure set is prepared.
}MAKER_DIALOG_CONFIG_SET_STATUS;
/**
 * @endcond
 */

/**
 * Data structure of a configuration set.
 *
 * This structure defines a MakerDialog configuration set.
 * Members includes the pageNames manages by this configuration set;
 * patterns and search paths of configuration files;
 * default filename if no configuration files can be found;
 * maximum number of configuration file to be found;
 * and flags to fine tune the configuration set.
 *
 * If \a flags are not explicitly defined,
 * it copies paraent MakerDialogConfig's flags for its own setting.
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
struct _MakerDialogConfigSet{
    const gchar 			**pageNames;		//!< Page names that this configuration set stores.
    const gchar 			*filePattern;		//!< Glob-formated file pattern. Or just the filename.
    const gchar 			**searchDirs;		//!< Directories to be searched.
    const gchar 			*defaultFilename;	//!< Default filename to use if filePattern cannot match any files in search directories.
    gint 				maxFileCount;		//!< Maximum number of configuration files. -1 for find all matched.
    gint 				writeIndex;		//!< Index of writable file. -1 if all read-only.
    GPtrArray 				*fileArray;		//!< Array of MakerDialogConfigFile.
    MakerDialogConfigFlags	 	flags; 			//!< Set specific configuration flags.
    MakerDialogConfigFileInterface	*configInterface;	//!< Set specific configure file interface.
    gpointer				userData;		//!< Pass user data.
    /// @cond
    guint32				status;			//!< Current set status.
    gboolean				hasLocalSetting;	//!< Has local setting.
    MakerDialogConfig			*config;		//!< Parent MakerDialog config.
 //   MakerDialogConfigBuffer		*configBuf;		//!< Configure buffer.
    /// @endcond
};

/**
 * New a configuration set.
 *
 * This function new a configuration set for loading and saving configuration files.
 *
 * @return A newly allocated MakerDialog configuration set; NULL if failed.
 * @see maker_dialog_config_set_close(), maker_dialog_config_add_config_set().
 * @see ::MakerDialogConfigErrorCode.
 */
MakerDialogConfigSet *maker_dialog_config_set_new();

/**
 * New a configuration set with all available arguments.
 *
 * This function new a configuration set for loading and saving configuration files.
 *
 * @param pageNames 		Pages that this configure set contains; NULL if all pages goes to same file.
 * @param filePattern 		Glob-formated file pattern. Or just the filename.
 * @param searchDirs		Directory to be searched.
 * @param defaultFilename	Default filename to use if filePattern cannot match any files in search directories.
 * @param maxFileCount		Maximum number of configuration files. -1 for find all matched.
 * @param flags 		Configuration flags.
 * @param configInterface	Interface for configuration file back-end.
 * @param userData		Custom user data. Can be \c NULL.
 *
 * @return A newly allocated MakerDialog configuration set; NULL if failed.
 * @see maker_dialog_config_set_close(), maker_dialog_config_add_config_set().
 * @see ::MakerDialogConfigErrorCode.
 */
MakerDialogConfigSet *maker_dialog_config_set_new_full(const gchar **pageNames,
	const gchar *filePattern, const gchar **searchDirs,
	const gchar *defaultFilename, gint maxFileCount,
	MakerDialogConfigFlags flags, MakerDialogConfigFileInterface *configInterface,
	gpointer userData);

/**
 * Free a configuration set.
 *
 * This function free the allocated memory of configuration set.
 * @param configSet A MakerDialog configuration set.
 */
void maker_dialog_config_set_free(MakerDialogConfigSet *configSet);

/**
 * Prepare files for a configuration set.
 *
 * This function prepares files for a configuration set by
 * searching the configuration files and checking the permissions of those
 * files.
 *
 * This function is usually used to initialized file based configuration back-end.
 *
 * @param configSet	A MakerDialog configuration set.
 * @param error		Error return location, or \c NULL.
 * @return \c TRUE if succeed; \c FALSE otherwise.
 * @since 0.3
 */
gboolean maker_dialog_config_set_prepare_files(MakerDialogConfigSet *configSet, MakerDialogError **error);

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

#endif /* MAKER_DIALOG_CONFIG_SET_H_ */
