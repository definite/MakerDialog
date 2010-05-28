/*
 * Copyright © 2010  Red Hat, Inc. All rights reserved.
 * Copyright © 2010  Ding-Yi Chen <dchen at redhat.com>
 *
 *  This file is part of MakerDailog.
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
 * @file MakerDialogConfigDef.h
 * Data type definition of configuration module for Mkdg.
 *
 * Data type definition of configuration module for Mkdg.
 *
 * @since 0.3; was included in MkdgConfig.h prior 0.2.
 */
#ifndef MKDG_CONFIG_DEF_H_
#define MKDG_CONFIG_DEF_H_

/**
 * Enumeration of configuration flags.
 *
 * Enumeration of configuration flags, which tunes the behavior of configuration interface and configure file.
 */
typedef enum{
    /**
     * Open all config files as read-only.
     *
     * Open all config files as read-only.
     */
    MKDG_CONFIG_FLAG_READONLY=		0x2,

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
    MKDG_CONFIG_FLAG_NO_OVERRIDE=	0x4,

    /**
     * Do not apply the loaded value.
     *
     * By default, after the new value is loaded,
     * the applyFunc() defined in property context will be called accordingly.
     *
     * This flag stops this behavior.
     */
    MKDG_CONFIG_FLAG_NO_APPLY=		0x8,

    /**
     * Stop on error.
     *
     * When encounter errors, the default behavior is trying to continue the loading.
     *
     * Set this flags to stop on error.
     */
    MKDG_CONFIG_FLAG_STOP_ON_ERROR=	0x10,

    /**
     * Hide the default values in configure file.
     * This flag may be ignored if the configuration back-end does not support
     * it.
     */
    MKDG_CONFIG_FLAG_HIDE_DEFAULT=	0x20,

    /**
     * Hide duplicated values in configure file.
     * This flag may be ignored if the configuration back-end does not support
     * it.
     */
    MKDG_CONFIG_FLAG_HIDE_DUPLICATE=	0x40,

} MKDG_CONFIG_FLAG;

/**
 * Data structure for configuration flags.
 *
 * Data structure for configuration flags.
 */
typedef MkdgFlags MkdgConfigFlags;

/**
 * Enumeration of configuration file flags.
 *
 * Enumeration of configuration file flags, which are used in configuration
 * file function.
 *
 * Flags lesser than 0x100 are for user-defined flags.
 * These flags should be set and used inside the configuration back-end.
 */
typedef enum{
    MKDG_CONFIG_FILE_FLAG_HAS_CONTENT=	0x100,	//!< File object has content, so preload functions can proceed.
    MKDG_CONFIG_FILE_FLAG_BUFFERED=	0x200,	//!< Property is loaded to MakerDialog config buffer.
    MKDG_CONFIG_FILE_FLAG_CANT_WRITE=	0x400,	//!< File cannot be written.
} MKDG_CONFIG_FILE_FLAG;

/**
 * Data structure for configuration file flags.
 *
 * Data structure for configuration file flags.
 */
typedef MkdgFlags MkdgConfigFileFlags;

/**
 * Configuration buffer which stores the contents of pre-loaded properties.
 *
 * Configuration buffer stores the contents of pre-loaded properties.
 * This buffer can then be loaded to Mkdg, or save only the modified
 * parts.
 */
typedef struct{
    GHashTable *keyValueTable;			//!< Hash table whose key is configuration key, value is associated value for that key.
} MkdgConfigBuffer;

typedef struct _MkdgConfigSet		MkdgConfigSet;
typedef struct _MkdgConfigFile		MkdgConfigFile;
typedef struct _MkdgConfigFileInterface	MkdgConfigFileInterface;


#endif /* MKDG_CONFIG_DEF_H_ */

