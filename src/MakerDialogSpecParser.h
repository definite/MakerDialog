/*
 * Copyright © 2010  Red Hat, Inc. All rights reserved.
 * Copyright © 2010  Ding-Yi Chen <dchen at redhat.com>
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
 * @file MakerDialogSpecParser.h
 * Parse the MakerDialog spec file.
 *
 * Instead of using array initialization to specify the behavior of
 * MakerDialog, developers can supply a MakerDialog spec file for
 * that purpose.
 *
 * This file lists definitions and function for spec file parsing.
 */
#ifndef MAKER_DIALOG_SPEC_PARSER_H_
#define MAKER_DIALOG_SPEC_PARSER_H_
#include "MakerDialogUtil.h"
#include "MakerDialogTypes.h"

/**
 * Name of main section in MakerDialog spec file.
 *
 * This is the GKeyFile group name for MAIN section of MakerDialog spec file.
 * MakerDialog-wide definition and flags are listed in the section.
 */
#define MAKER_DIALOG_SPEC_SECTION_MAIN "_MAIN_"

/**
 * Name of config section in MakerDialog spec file.
 *
 * This is the GKeyFile group name for CONFIG section of MakerDialog spec file.
 * MakerDialog config definition and flags are listed in the section.
 */
#define MAKER_DIALOG_SPEC_SECTION_CONFIG "_CONFIG_"

/**
 * Name prefix of config set section in MakerDialog spec file.
 *
 * This is the GKeyFile group name prefix for CONFIGSET section of MakerDialog spec file.
 * MakerDialog config set definition and flags are listed in the section.
 *
 * Note that the full cofig set name contains serial numbers.
 * Such as \c _CONFIG_SET_01_ , \c _CONFIG_SET_23_, etc.
 */
#define MAKER_DIALOG_SPEC_SECTION_CONFIG_SET "_CONFIG_SET_"

/**
 * Name of UI section in MakerDialog spec file.
 *
 * This is the GKeyFile group name for UI section of MakerDialog spec file.
 * MakerDialog UI definition and flags are listed in the section.
 */
#define MAKER_DIALOG_SPEC_SECTION_MAIN "_UI_"

/**
 * Load MakerDialog setting from a glib key file.
 *
 * This function loads MakerDialog setting and specs of properties
 * from a glib key file, whose format is similar to .desktop file.
 *
 * @param filename 	The file to load.
 * @param error		Returned error is stored here; or \c NULL to ignore error.
 * @return A newly allocated MakerDialog instance; or \c NULL if failed.
 */
MakerDialog *maker_dialog_new_from_key_file(const gchar *filename, MakerDialogError **error);

#endif /* MAKER_DIALOG_SPEC_PARSER_H_ */

