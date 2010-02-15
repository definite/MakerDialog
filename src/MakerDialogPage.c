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

gboolean maker_dialog_page_name_is_empty(const gchar *pageName){
    if (maker_dialog_string_is_empty(pageName)){
	return TRUE;
    }
    if (strcmp(pageName, MAKER_DIALOG_PAGE_UNNAMED)==0){
	return TRUE;
    }
    return FALSE;
}

gboolean maker_dialog_group_name_is_empty(const gchar *groupName){
    if (maker_dialog_string_is_empty(groupName)){
	return TRUE;
    }
    if (strcmp(groupName, MAKER_DIALOG_GROUP_UNNAMED)==0){
	return TRUE;
    }
    return FALSE;
}

GNode *maker_dialog_find_page_node(MakerDialog *mDialog, const gchar *pageName){
    const gchar *pageName_tmp=(pageName)? pageName : MAKER_DIALOG_PAGE_UNNAMED;
    GNode *result=NULL;
    for(result=g_node_first_child(mDialog->pageRoot); result!=NULL; result=g_node_next_sibling(result)){
	if (strcmp(pageName_tmp, (gchar *) result->data)==0){
	    return result;
	}
    }
    return NULL;
}

GNode *maker_dialog_find_group_node(MakerDialog *mDialog, const gchar *pageName, const gchar *groupName){
    const gchar *groupName_tmp=(groupName)? groupName : MAKER_DIALOG_GROUP_UNNAMED;
    GNode *pageNode=maker_dialog_find_page_node(mDialog, pageName);
    GNode *result=NULL;
    for(result=g_node_first_child(pageNode); result!=NULL; result=g_node_next_sibling(result)){
	if (strcmp(groupName_tmp, (gchar *) result->data)==0){
	    return result;
	}
    }
    return NULL;
}

void maker_dialog_page_foreach_property(MakerDialog* mDialog, const gchar *pageName,
	MakerDialogEachGroupNodeFunc groupFunc, gpointer groupUserData,	MakerDialogEachPropertyFunc propFunc, gpointer propUserData){
    GNode *pageNode=maker_dialog_find_page_node(mDialog, pageName);
    g_assert(pageNode);
    GNode *groupNode=NULL;
    for(groupNode=g_node_first_child(pageNode);groupNode!=NULL; groupNode=g_node_next_sibling(groupNode)){
	if (groupFunc)
	    groupFunc(mDialog, pageNode, groupNode, groupUserData);
	maker_dialog_group_foreach_property(mDialog, (gchar *) pageNode->data, (gchar *) groupNode->data, propFunc, propUserData);
    }
}

void maker_dialog_pages_foreach_property(MakerDialog* mDialog, const gchar **pageNames,
	MakerDialogEachGroupNodeFunc groupFunc, gpointer groupUserData,	MakerDialogEachPropertyFunc propFunc, gpointer propUserData){
    if (pageNames){
	gsize i;
	for(i=0;pageNames[i]!=NULL;i++){
	    maker_dialog_page_foreach_property(mDialog, pageNames[i], groupFunc, groupUserData, propFunc, propUserData);
	}
    }else{
	GNode *pageNode=NULL;
	for(pageNode=g_node_first_child(mDialog->pageRoot);pageNode!=NULL; pageNode=g_node_next_sibling(pageNode)){
	    maker_dialog_page_foreach_property(mDialog, (gchar *) pageNode->data, groupFunc, groupUserData, propFunc, propUserData);
	}
    }
}

void maker_dialog_group_foreach_property(MakerDialog* mDialog, const gchar *pageName, const gchar *groupName, MakerDialogEachPropertyFunc  func, gpointer userData){
    MAKER_DIALOG_DEBUG_MSG(5, "[I5] group_foreach_property( , %s, %s, , )", (pageName)? pageName : "", (groupName)? groupName: "");
    GNode *groupNode=maker_dialog_find_group_node(mDialog, pageName, groupName);
    g_assert(groupNode);
    GNode *keyNode=NULL;
    for(keyNode=g_node_first_child(groupNode);keyNode!=NULL; keyNode=g_node_next_sibling(keyNode)){
	MakerDialogPropertyContext *ctx=(MakerDialogPropertyContext *) keyNode->data;
	func(mDialog, ctx, userData);
    }
}

void maker_dialog_foreach_page(MakerDialog *mDialog, MakerDialogEachPageFunc func, gpointer userData){
    GNode *pageNode=NULL;
    for(pageNode=g_node_first_child(mDialog->pageRoot);pageNode!=NULL; pageNode=g_node_next_sibling(pageNode)){
	func(mDialog, (gchar *) pageNode->data, userData);
    }
}

void maker_dialog_foreach_page_foreach_property(MakerDialog *mDialog,
	MakerDialogEachGroupNodeFunc groupFunc, gpointer groupUserData,	MakerDialogEachPropertyFunc propFunc, gpointer propUserData){
    maker_dialog_pages_foreach_property(mDialog, NULL, groupFunc, groupUserData, propFunc, propUserData);
}

MakerDialogNodeIter maker_dialog_page_iter_init(MakerDialog* mDialog){
    g_assert(mDialog->pageRoot);
    return g_node_first_child(mDialog->pageRoot);
}

gboolean maker_dialog_page_iter_has_next(MakerDialogNodeIter iter){
    return (iter!=NULL) ? TRUE: FALSE;
}

GNode *maker_dialog_page_iter_next(MakerDialogNodeIter *iter){
    if (maker_dialog_page_iter_has_next(*iter)){
	GNode *currNode=*iter;
	*iter=g_node_next_sibling(*iter);
	return currNode;
    }
    return NULL;
}

MakerDialogNodeIter maker_dialog_page_property_iter_init(MakerDialog* mDialog, const gchar *pageName){
    GNode *pageNode=maker_dialog_find_page_node(mDialog, pageName);
    g_assert(pageNode);
    GNode *groupNode=g_node_first_child(pageNode);
    if (!groupNode)
	return NULL;
    return g_node_first_child(groupNode);
}

gboolean maker_dialog_page_property_iter_has_next(MakerDialogNodeIter iter){
    return (iter!=NULL)? TRUE: FALSE;
}

MakerDialogPropertyContext *maker_dialog_page_property_iter_next(MakerDialogNodeIter *iter){
    if (maker_dialog_page_property_iter_has_next(*iter)){
	MakerDialogPropertyContext *ctx=(MakerDialogPropertyContext *) (*iter)->data;
	MakerDialogNodeIter nextIter=g_node_next_sibling(*iter);
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

