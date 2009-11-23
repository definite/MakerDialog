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
 * @file MakerDialogPage.h
 * Page Specification and Manipulation.
 *
 * A MakerDialog page, like a category, holds properties that have semantic
 * relation.
 *
 * In terms of UI, a MakerDialog page is normally shown as a notebook page like pages in GNotebook.
 * Properties under that page is displayed in the page.
 *
 * A page can also has its own configuration set and corresponding files,
 * if configuration back-end support that.
 *
 * In a page, MakerDialog groups further groups properties that address the same issue,
 * and UI is rendered accordingly.
 * A group does not have special meaning in configuration set, however.
 *
 */
#ifndef MAKER_DIALOG_PAGE_H_
#define MAKER_DIALOG_PAGE_H_
#include <glib.h>
#include <glib-object.h>

/**
 * The page name for no page.
 *
 * If key does not have a page name associate with it,
 * then this name is assigned as the page name.
 * Because some of the configuration back-ends (such as glib KeyFile)
 * expect a page name associate with it.
 */
#define MAKER_DIALOG_PAGE_UNNAMED "_UNPAGED_"

/**
 * The group name for no group.
 *
 * If key does not have a group name associate with it,
 * then this name is assigned as the page name.
 */
#define MAKER_DIALOG_GROUP_UNNAMED "_UNGROUPED_"

/**
 * Prototype of callback function for foreach property function.
 *
 * The callback function should implement the actions for each property.
 * It will be called by maker_dialog_page_foreach_property() and maker_dialog_pages_foreach_property().
 *
 * @param mDialog 	A MakerDialog.
 * @param ctx  		The property context.
 * @param userData 	User data to be passed into the callback.
 * @see maker_dialog_pages_foreach_property().
 */
typedef void (* MakerDialogEachPropertyFunc)(MakerDialog *mDialog, MakerDialogPropertyContext *ctx, gpointer userData);

/**
 * Prototype of callback function for foreach property function.
 *
 * The callback function should implement the actions for each property.
 * It will be called by maker_dialog_page_foreach_property() and maker_dialog_pages_foreach_property().
 *
 * @param mDialog 	A MakerDialog.
 * @param pageNode 	The page node to be working on.
 * @param groupNode 	The group node to be working on.
 * @param userData 	User data to be passed into the callback.
 * @see maker_dialog_pages_foreach_property().
 */
typedef void (* MakerDialogEachGroupNodeFunc)(MakerDialog *mDialog, GNode *pageNode, GNode *groupNode, gpointer userData);


/**
 * Prototype of callback function for foreach page function.
 *
 * The callback function should implement the actions for each property.
 * It will be called by maker_dialog_foreach_page().
 *
 * @param mDialog 	A MakerDialog.
 * @param pageName  	Current page name.
 * @param userData 	User data to be passed into the callback.
 * @see maker_dialog_foreach_page().
 */
typedef void (* MakerDialogEachPageFunc)(MakerDialog *mDialog, const gchar *pageName, gpointer userData);


/**
 * Whether the page name is empty.
 *
 * Whether the page name is empty, that is,
 * page name is NULL, "", or MAKER_DIALOG_PAGE_UNNAMED.
 *
 * @param pageName	Page name to be tested.
 * @return TRUE if page name is empty; FALSE otherwise.
 */
gboolean maker_dialog_page_name_is_empty(const gchar *pageName);

/**
 * Whether the group name is empty.
 *
 * Whether the group name is empty, that is,
 * group name is NULL, "", or MAKER_DIALOG_GROUP_UNNAMED.
 *
 * @param groupName	Group name to be tested.
 * @return TRUE if group name is empty; FALSE otherwise.
 */
gboolean maker_dialog_group_name_is_empty(const gchar *groupName);


/**
 * Find the page node by page name.
 *
 * Find the page node by page name.
 * @param mDialog A MakerDialog.
 * @param pageName Page name to be found.
 * @return GNode that contains the page name; NULL if no such node.
 */
GNode *maker_dialog_find_page_node(MakerDialog *mDialog, const gchar *pageName);

/**
 * Find the group node by page name and group name.
 *
 * Find the group node by page name and group name.
 * @param mDialog A MakerDialog.
 * @param pageName Matching page name.
 * @param groupName Matching group name.
 * @return GNode that contains the group name under given page name; NULL if no such node.
 */
GNode *maker_dialog_find_group_node(MakerDialog *mDialog, const gchar *pageName, const gchar *groupName);

/**
 * Call callback for each property in a page.
 *
 * Calls the given function for each property in a page.
 *
 * @param mDialog 		A MakerDialog.
 * @param pageName 		The page to be working on.
 * @param groupFunc 		The callback to be run for each group.
 * @param groupUserData 	User data to pass to \a groupFunc.
 * @param propFunc 		The callback to be run for each property.
 * @param propUserData 		User data to pass to \a propFunc.
 */
void maker_dialog_page_foreach_property(MakerDialog* mDialog, const gchar *pageName, MakerDialogEachGroupNodeFunc groupFunc, gpointer groupUserData,
	MakerDialogEachPropertyFunc propFunc, gpointer propUserData);


/**
 * Call callback for each property in certain pages.
 *
 * Calls the given function for each property in certain pages.
 *
 * @param mDialog 	A MakerDialog.
 * @param pageNames 	Page names to be included in the execution. \c NULL for all pages.
 * @param groupFunc 		The callback to be run for each group.
 * @param groupUserData 	User data to pass to \a groupFunc.
 * @param propFunc 		The callback to be run for each property.
 * @param propUserData 		User data to pass to \a propFunc.
 * @see maker_dialog_foreach_page_foreach_property().
 */
void maker_dialog_pages_foreach_property(MakerDialog* mDialog, const gchar **pageNames,
	MakerDialogEachGroupNodeFunc groupFunc, gpointer groupUserData,	MakerDialogEachPropertyFunc propFunc, gpointer propUserData);

/**
 * Call callback for each page.
 *
 * Call callback for each page.
 *
 * @param mDialog 	A MakerDialog.
 * @param func 		The callback to be run for each page.
 * @param userData 	User data to pass to \a func
 * @see maker_dialog_foreach_page_foreach_property().
 */
void maker_dialog_foreach_page(MakerDialog *mDialog, MakerDialogEachPageFunc func, gpointer userData);

/**
 * Call callback for each property in all pages.
 *
 * Call callback for each property in all pages.
 * This function is essentially maker_dialog_pages_foreach_property(mDialog, NULL, groupFunc, groupUserData, propFunc, propUserData).
 *
 * @param mDialog 		A MakerDialog.
 * @param groupFunc 		The callback to be run for each group.
 * @param groupUserData 	User data to pass to \a groupFunc.
 * @param propFunc 		The callback to be run for each property.
 * @param propUserData 		User data to pass to \a propFunc.
 * @see maker_dialog_pages_foreach_property().
 */
void maker_dialog_foreach_page_foreach_property(MakerDialog *mDialog,
	MakerDialogEachGroupNodeFunc groupFunc, gpointer groupUserData,	MakerDialogEachPropertyFunc propFunc, gpointer propUserData);

#endif /* MAKER_DIALOG_PAGE_H_ */
