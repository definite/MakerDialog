/*
 * Copyright © 2009  Red Hat, Inc. All rights reserved.
 * Copyright © 2009  Ding-Yi Chen <dchen at redhat.com>
 *
 *  This file is part of Mkdg.
 *
 *  Mkdg is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Mkdg is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Mkdg.  If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * @file MakerDialogPage.h
 * Page Specification and Manipulation.
 *
 * A Mkdg page, like a category, holds properties that have semantic
 * relation.
 *
 * In terms of UI, a Mkdg page is normally shown as a notebook page like pages in GNotebook.
 * Properties under that page is displayed in the page.
 *
 * A page can also has its own configuration set and corresponding files,
 * if configuration back-end support that.
 *
 * In a page, Mkdg groups further groups properties that address the same issue,
 * and UI is rendered accordingly.
 * A group does not have special meaning in configuration set, however.
 *
 */
#ifndef MKDG_PAGE_H_
#define MKDG_PAGE_H_
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
#define MKDG_PAGE_UNNAMED "_UNPAGED_"

/**
 * The group name for no group.
 *
 * If key does not have a group name associate with it,
 * then this name is assigned as the page name.
 */
#define MKDG_GROUP_UNNAMED "_UNGROUPED_"

/**
 * Prototype of callback function for foreach property function.
 *
 * The callback function should implement the actions for each property.
 * It will be called by mkdg_page_foreach_property() and mkdg_pages_foreach_property().
 *
 * @param mDialog 	A Mkdg.
 * @param ctx  		The property context.
 * @param userData 	User data to be passed into the callback.
 * @see mkdg_pages_foreach_property().
 */
typedef void (* MkdgEachPropertyFunc)(Mkdg *mDialog, MkdgPropertyContext *ctx, gpointer userData);

/**
 * Prototype of callback function for foreach property function.
 *
 * The callback function should implement the actions for each property.
 * It will be called by mkdg_page_foreach_property() and mkdg_pages_foreach_property().
 *
 * @param mDialog 	A Mkdg.
 * @param pageNode 	The page node to be working on.
 * @param groupNode 	The group node to be working on.
 * @param userData 	User data to be passed into the callback.
 * @see mkdg_pages_foreach_property().
 */
typedef void (* MkdgEachGroupNodeFunc)(Mkdg *mDialog, GNode *pageNode, GNode *groupNode, gpointer userData);


/**
 * Prototype of callback function for foreach page function.
 *
 * The callback function should implement the actions for each property.
 * It will be called by mkdg_foreach_page().
 *
 * @param mDialog 	A Mkdg.
 * @param pageName  	Current page name.
 * @param userData 	User data to be passed into the callback.
 * @see mkdg_foreach_page().
 */
typedef void (* MkdgEachPageFunc)(Mkdg *mDialog, const gchar *pageName, gpointer userData);

/**
 * Whether the page name is empty.
 *
 * Whether the page name is empty, that is,
 * page name is NULL, "", or MKDG_PAGE_UNNAMED.
 *
 * @param pageName	Page name to be tested.
 * @return TRUE if page name is empty; FALSE otherwise.
 */
gboolean mkdg_page_name_is_empty(const gchar *pageName);

/**
 * Whether the group name is empty.
 *
 * Whether the group name is empty, that is,
 * group name is NULL, "", or MKDG_GROUP_UNNAMED.
 *
 * @param groupName	Group name to be tested.
 * @return TRUE if group name is empty; FALSE otherwise.
 */
gboolean mkdg_group_name_is_empty(const gchar *groupName);

/**
 * Find the page node by page name.
 *
 * Find the page node by page name.
 * @param mDialog A Mkdg.
 * @param pageName Page name to be found.
 * @return GNode that contains the page name; NULL if no such node.
 */
GNode *mkdg_find_page_node(Mkdg *mDialog, const gchar *pageName);

/**
 * Find the group node by page name and group name.
 *
 * Find the group node by page name and group name.
 * @param mDialog A Mkdg.
 * @param pageName Matching page name.
 * @param groupName Matching group name.
 * @return GNode that contains the group name under given page name; NULL if no such node.
 */
GNode *mkdg_find_group_node(Mkdg *mDialog, const gchar *pageName, const gchar *groupName);

/**
 * Call callback for each property in a page.
 *
 * Calls the given function for each property in a page.
 *
 * @param mDialog 		A Mkdg.
 * @param pageName 		The page to be working on.
 * @param groupFunc 		The callback to be run for each group.
 * @param groupUserData 	User data to pass to \a groupFunc.
 * @param propFunc 		The callback to be run for each property.
 * @param propUserData 		User data to pass to \a propFunc.
 */
void mkdg_page_foreach_property(Mkdg* mDialog, const gchar *pageName, MkdgEachGroupNodeFunc groupFunc, gpointer groupUserData,
	MkdgEachPropertyFunc propFunc, gpointer propUserData);

/**
 * Call callback for each property in certain pages.
 *
 * Calls the given function for each property in certain pages.
 *
 * @param mDialog 	A Mkdg.
 * @param pageNames 	Page names to be included in the execution. \c NULL for all pages.
 * @param groupFunc 		The callback to be run for each group.
 * @param groupUserData 	User data to pass to \a groupFunc.
 * @param propFunc 		The callback to be run for each property.
 * @param propUserData 		User data to pass to \a propFunc.
 * @see mkdg_foreach_page_foreach_property().
 */
void mkdg_pages_foreach_property(Mkdg* mDialog, const gchar **pageNames,
	MkdgEachGroupNodeFunc groupFunc, gpointer groupUserData,	MkdgEachPropertyFunc propFunc, gpointer propUserData);


/**
 * Call callback for each property in a group.
 *
 * Calls the given function for each property in a group.
 *
 * @param mDialog 		A Mkdg.
 * @param pageName 		The page to be working on.
 * @param groupFunc 		The callback to be run for each group.
 * @param groupUserData 	User data to pass to \a groupFunc.
 * @param propFunc 		The callback to be run for each property.
 * @param propUserData 		User data to pass to \a propFunc.
 */
void mkdg_group_foreach_property(Mkdg* mDialog, const gchar *pageName, const gchar *groupName, MkdgEachPropertyFunc  func, gpointer userData);

/**
 * Call callback for each page.
 *
 * Call callback for each page.
 *
 * @param mDialog 	A Mkdg.
 * @param func 		The callback to be run for each page.
 * @param userData 	User data to pass to \a func
 * @see mkdg_foreach_page_foreach_property().
 */
void mkdg_foreach_page(Mkdg *mDialog, MkdgEachPageFunc func, gpointer userData);

/**
 * Call callback for each property in all pages.
 *
 * Call callback for each property in all pages.
 * This function is essentially mkdg_pages_foreach_property(mDialog, NULL, groupFunc, groupUserData, propFunc, propUserData).
 *
 * @param mDialog 		A Mkdg.
 * @param groupFunc 		The callback to be run for each group.
 * @param groupUserData 	User data to pass to \a groupFunc.
 * @param propFunc 		The callback to be run for each property.
 * @param propUserData 		User data to pass to \a propFunc.
 * @see mkdg_pages_foreach_property().
 */
void mkdg_foreach_page_foreach_property(Mkdg *mDialog,
	MkdgEachGroupNodeFunc groupFunc, gpointer groupUserData,	MkdgEachPropertyFunc propFunc, gpointer propUserData);

/**
 * Data type of Mkdg node iteration for a page.
 *
 * Data type of Mkdg node iteration for a page.
 */
typedef GNode *MkdgNodeIter;

/**
 * Initialize a Mkdg page iteration handle .
 *
 * This function initializes a Mkdg page iteration.
 * It returns a GNode pointer to the first page as iteration handle,
 * or \c NULL if no page is defined.
 *
 * @param mDialog 		A Mkdg.
 * @return The Iteration handle; or \c NULL if no page.
 */
MkdgNodeIter mkdg_page_iter_init(Mkdg* mDialog);

/**
 * Whether the iteration has more elements.
 *
 * This function returns whether the page iteration has more elements
 * that store pages.
 *
 * @param iter 			A Mkdg node iteration handle.
 * @return \c TRUE if \a iter has more elements; \c FALSE otherwise.
 */
gboolean mkdg_page_iter_has_next(MkdgNodeIter iter);

/**
 * Return the next element in the iteration.
 *
 * This function returns the next element,
 * namely the GNode that contain page,
 * in the iteration.
 *
 * @param iter 			A Mkdg node iteration handle.
 * @return The next element; or \c NULL if not such element.
 */
GNode *mkdg_page_iter_next(MkdgNodeIter *iter);

/**
 * Initialize a Mkdg property iteration handle for a page.
 *
 * This function initializes a Mkdg property iteration for a page.
 * It returns a GNode pointer to the first property as iteration handle,
 * or \c NULL if no such page.
 *
 * @param mDialog 		A Mkdg.
 * @param pageName 		The page to be working on.
 * @return The Iteration handle; or \c NULL if no such page.
 */
MkdgNodeIter mkdg_page_property_iter_init(Mkdg* mDialog, const gchar *pageName);

/**
 * Whether the iteration has more elements.
 *
 * This function returns whether the iteration has more elements
 * that store properties.
 *
 * @param iter 			A Mkdg node iteration handle.
 * @return \c TRUE if \a iter has more elements; \c FALSE otherwise.
 */
gboolean mkdg_page_property_iter_has_next(MkdgNodeIter iter);

/**
 * Return the next element in the iteration.
 *
 * This function returns the next element,
 * namely the GNode that contain property,
 * in the iteration.
 *
 * @param iter 			A Mkdg node iteration handle.
 * @return The next element; or \c NULL if not such element.
 */
MkdgPropertyContext *mkdg_page_property_iter_next(MkdgNodeIter *iter);

#endif /* MKDG_PAGE_H_ */
