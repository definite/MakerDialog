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
} MakerDialogConfigFlag;

typedef guint MakerDialogConfigFlags;

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
