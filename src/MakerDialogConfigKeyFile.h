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
 * @file MakerDialogConfigKeyFile.h
 * Glib key file: Manipulate INI like configuration file.
 *
 * MkdgConfigKeyFile lets you parse, edit or create files containing groups of key-value pairs.
 * This module is, in fact, a wrapper of GKeyFile in glib.
 *
 * Similar with GKeyFile, Key files consists of groups of key-value pairs, interspersed with comments.
 * Following example is from GKeyFile:
 * @code
 * # this is just an example
 * # there can be comments before the first group
 * [First Group]
 * Name=Key File Example\tthis value shows\nescaping
 * # localized strings are stored in multiple key-value pairs
 * Welcome=Hello
 * Welcome[de]=Hallo
 * Welcome[fr_FR]=Bonjour
 * Welcome[it]=Ciao
 * Welcome[be@latin]=Hello
 * [Another Group]
 * Numbers=2;20;-200;0
 * Booleans=true;false;true;true
 * @endcode
 *
 * Lines beginning with a '#' and blank lines are considered comments.
 *
 * Groups are started by a header line containing the group name enclosed in '[' and ']',
 * and ended implicitly by the start of the next group or the end of the file.
 * Each key-value pair must be contained in a group.
 *
 * Key-value pairs generally have the form key=value, with the exception of localized strings,
 * which have the form key[locale]=value, with a locale identifier of the form
 * @code
 * lang_country@modifier
 * @endcode
 * where country and modifier are optional.
 *
 *
 * Space before and after the '=' character are ignored. Newline, tab, carriage return and backslash
 * characters in value are escaped as \\n, \\t, \\r, and \\\\, respectively.
 * To preserve leading spaces in values, these can also be escaped as \\s.
 *
 * Key files can store strings (possibly with localized variants), integers, booleans and lists of these.
 * Lists are separated by a separator character, typically ';' or ','.
 * To use the list separator character in a value in a list, it has to be escaped by prefixing
 * it with a backslash.
 *
 * This syntax is obviously inspired by the .ini files commonly met on Windows,
 * but there are some important differences:
 *
 * -.ini files use the ';' character to begin comments, key files use the '#' character.
 * -Key files do not allow for ungrouped keys meaning only comments can precede the first group.
 * -Key files are always encoded in UTF-8.
 * -Key and Group names are case-sensitive, for example a group called [GROUP] is a different group from [group].
 *
 * .ini files don't have a strongly typed boolean entry type, they only have GetProfileInt.
 * In GKeyFile only true and false (in lower case) are allowed.
 *
 * Note that in contrast to the Desktop Entry Specification, groups in key files may contain
 * the same key multiple times; the last entry wins. Key files may also contain multiple groups
 * with the same name; they are merged together. Another difference is that keys and group names
 * in key files are not restricted to ASCII characters.
 */
#ifndef MKDG_CONFIG_KEY_FILE_H_
#define MKDG_CONFIG_KEY_FILE_H_
#include <glib.h>
#include <glib-object.h>
#include "MakerDialogConfig.h"

/**
 * The configure file interface for GKeyFile.
 *
 * The configure file interface for GKeyFile.
 */
extern MkdgConfigFileInterface MKDG_CONFIG_FILE_INTERFACE_KEY_FILE;

/**
 * New a MkdgConfig which use glib GKeyFile back-end.
 *
 * This function returns a newly allocated MkdgConfig
 * which use glib GKeyFile back-end.
 *
 * During construction, the new MkdgConfig is associated to the \a mDialog.
 * Thus, mkdg_destroy() can free the associated MkdgConfig as well.
 *
 * @param mDialog 	A MakerDialog.
 * @return A newly allocated MkdgConfig with glib GKeyFile back-end.
 * @see mkdg_config_new().
 */
MkdgConfig *mkdg_config_use_key_file(Mkdg *mDialog);

#endif /* MKDG_CONFIG_KEY_FILE_H_ */
