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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <glib/gi18n.h>
#include <locale.h>
#include "MakerDialog.h"

gboolean mkdg_page_name_is_empty(const gchar *pageName){
    if (mkdg_string_is_empty(pageName)){
	return TRUE;
    }
    if (strcmp(pageName, MKDG_PAGE_UNNAMED)==0){
	return TRUE;
    }
    return FALSE;
}

gboolean mkdg_group_name_is_empty(const gchar *groupName){
    if (mkdg_string_is_empty(groupName)){
	return TRUE;
    }
    if (strcmp(groupName, MKDG_GROUP_UNNAMED)==0){
	return TRUE;
    }
    return FALSE;
}

GNode *mkdg_find_page_node(Mkdg *mDialog, const gchar *pageName){
    const gchar *pageName_tmp=(pageName)? pageName : MKDG_PAGE_UNNAMED;
    GNode *result=NULL;
    for(result=g_node_first_child(mDialog->pageRoot); result!=NULL; result=g_node_next_sibling(result)){
	if (strcmp(pageName_tmp, (gchar *) result->data)==0){
	    return result;
	}
    }
    return NULL;
}

GNode *mkdg_find_group_node(Mkdg *mDialog, const gchar *pageName, const gchar *groupName){
    const gchar *groupName_tmp=(groupName)? groupName : MKDG_GROUP_UNNAMED;
    GNode *pageNode=mkdg_find_page_node(mDialog, pageName);
    GNode *result=NULL;
    for(result=g_node_first_child(pageNode); result!=NULL; result=g_node_next_sibling(result)){
	if (strcmp(groupName_tmp, (gchar *) result->data)==0){
	    return result;
	}
    }
    return NULL;
}

void mkdg_page_foreach_property(Mkdg* mDialog, const gchar *pageName,
	MkdgEachGroupNodeFunc groupFunc, gpointer groupUserData,	MkdgEachPropertyFunc propFunc, gpointer propUserData){
    GNode *pageNode=mkdg_find_page_node(mDialog, pageName);
    g_assert(pageNode);
    GNode *groupNode=NULL;
    for(groupNode=g_node_first_child(pageNode);groupNode!=NULL; groupNode=g_node_next_sibling(groupNode)){
	if (groupFunc)
	    groupFunc(mDialog, pageNode, groupNode, groupUserData);
	mkdg_group_foreach_property(mDialog, (gchar *) pageNode->data, (gchar *) groupNode->data, propFunc, propUserData);
    }
}

void mkdg_pages_foreach_property(Mkdg* mDialog, const gchar **pageNames,
	MkdgEachGroupNodeFunc groupFunc, gpointer groupUserData,	MkdgEachPropertyFunc propFunc, gpointer propUserData){
    if (pageNames){
	gsize i;
	for(i=0;pageNames[i]!=NULL;i++){
	    mkdg_page_foreach_property(mDialog, pageNames[i], groupFunc, groupUserData, propFunc, propUserData);
	}
    }else{
	GNode *pageNode=NULL;
	for(pageNode=g_node_first_child(mDialog->pageRoot);pageNode!=NULL; pageNode=g_node_next_sibling(pageNode)){
	    mkdg_page_foreach_property(mDialog, (gchar *) pageNode->data, groupFunc, groupUserData, propFunc, propUserData);
	}
    }
}

void mkdg_group_foreach_property(Mkdg* mDialog, const gchar *pageName, const gchar *groupName, MkdgEachPropertyFunc  func, gpointer userData){
    MKDG_DEBUG_MSG(5, "[I5] group_foreach_property( , %s, %s, , )", (pageName)? pageName : "", (groupName)? groupName: "");
    GNode *groupNode=mkdg_find_group_node(mDialog, pageName, groupName);
    g_assert(groupNode);
    GNode *keyNode=NULL;
    for(keyNode=g_node_first_child(groupNode);keyNode!=NULL; keyNode=g_node_next_sibling(keyNode)){
	MkdgPropertyContext *ctx=(MkdgPropertyContext *) keyNode->data;
	func(mDialog, ctx, userData);
    }
}

void mkdg_foreach_page(Mkdg *mDialog, MkdgEachPageFunc func, gpointer userData){
    GNode *pageNode=NULL;
    for(pageNode=g_node_first_child(mDialog->pageRoot);pageNode!=NULL; pageNode=g_node_next_sibling(pageNode)){
	func(mDialog, (gchar *) pageNode->data, userData);
    }
}

void mkdg_foreach_page_foreach_property(Mkdg *mDialog,
	MkdgEachGroupNodeFunc groupFunc, gpointer groupUserData,	MkdgEachPropertyFunc propFunc, gpointer propUserData){
    mkdg_pages_foreach_property(mDialog, NULL, groupFunc, groupUserData, propFunc, propUserData);
}

MkdgNodeIter mkdg_page_iter_init(Mkdg* mDialog){
    g_assert(mDialog->pageRoot);
    return g_node_first_child(mDialog->pageRoot);
}

gboolean mkdg_page_iter_has_next(MkdgNodeIter iter){
    return (iter!=NULL) ? TRUE: FALSE;
}

GNode *mkdg_page_iter_next(MkdgNodeIter *iter){
    if (mkdg_page_iter_has_next(*iter)){
	GNode *currNode=*iter;
	*iter=g_node_next_sibling(*iter);
	return currNode;
    }
    return NULL;
}

MkdgNodeIter mkdg_page_property_iter_init(Mkdg* mDialog, const gchar *pageName){
    GNode *pageNode=mkdg_find_page_node(mDialog, pageName);
    g_assert(pageNode);
    GNode *groupNode=g_node_first_child(pageNode);
    if (!groupNode)
	return NULL;
    return g_node_first_child(groupNode);
}

gboolean mkdg_page_property_iter_has_next(MkdgNodeIter iter){
    return (iter!=NULL)? TRUE: FALSE;
}

MkdgPropertyContext *mkdg_page_property_iter_next(MkdgNodeIter *iter){
    if (mkdg_page_property_iter_has_next(*iter)){
	MkdgPropertyContext *ctx=(MkdgPropertyContext *) (*iter)->data;
	MkdgNodeIter nextIter=g_node_next_sibling(*iter);
	if (nextIter==NULL){
	    GNode *nextGroupNode=g_node_next_sibling((*iter)->parent);
	    if (nextGroupNode){
		nextIter=g_node_first_child(nextGroupNode);
	    }else{
		nextIter=NULL;
	    }
	}
	*iter=nextIter;
	return ctx;
    }
    return NULL;
}

