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
 * it defines pages of properties to be managed, pattern of configuration files,
 * directory to be searched, number of configuration to be found, and so on.
 *
 * This implementation enables applications that have system-wide configuration as default,
 * yet users are able to override the default with their own configuration file.
 *
 * Configuration needs at least one the configuration set, thus,
 * by default, all properties are handles by one configure set.
 * No need to explicitly define the pages it handles in MakerDialog spec file.
 *
 * You don't need to
 * This good.
 * If you don't have
 * spec does not mention ant
 * since MakerDialog 0.3, it
 * Of course, t
 *
 * Configuration properties can be put in to one or more disjoint sets,
 * namely, configuration sets. Property in the same
 *
 * A configuration set groups properties stores keys and values of configuration options under given pages.
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
 *
 * A configuration set stores a is an abstract s
 * This module defines configuration relative data structure, functions,
 * and an interface for bridging between MakerDialog and configuration
 * back-end such as GKeyFile, GConf, KCfg.
 *
 * In MakerDialog, a property is a configuration option which associate with a
 * value. According to property specification, MakerDialog loads and saves
 * property values to configuration files.
 */
#ifndef MAKER_DIALOG_CONFIG_SET_H_
#define MAKER_DIALOG_CONFIG_SET_H_

fd

#endif /* MAKER_DIALOG_CONFIG_SET_H_ */
