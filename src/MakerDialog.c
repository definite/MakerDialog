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
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include "MakerDialog.h"
extern gint makerDialogVerboseLevel;

static MakerDialog *maker_dialog_new(){
    MakerDialog *mDialog=g_new(MakerDialog,1);
    mDialog->title=NULL;
    mDialog->buttonSpecs=NULL;
    mDialog->propertyTable=maker_dialog_property_table_new();
    mDialog->pageRoot=g_node_new(mDialog);
    mDialog->maxSizeInPixel.width=-1;
    mDialog->maxSizeInPixel.height=-1;
    mDialog->maxSizeInChar.width=-1;
    mDialog->maxSizeInChar.height=-1;
    mDialog->labelAlignment.x=0.0f;
    mDialog->labelAlignment.y=0.5f;
    mDialog->componentAlignment.x=0.0f;
    mDialog->componentAlignment.y=0.5f;
    if (getenv(MAKER_DLALOG_VERBOSE_ENV)){
	makerDialogVerboseLevel=atoi(getenv(MAKER_DLALOG_VERBOSE_ENV));
    }
    mDialog->ui=NULL;
    mDialog->config=NULL;
    mDialog->userData=NULL;
    return mDialog;
}

MakerDialog *maker_dialog_init(const gchar *title, MakerDialogButtonSpec *buttonSpecs){
    MakerDialog *mDialog=maker_dialog_new();
    mDialog->title=g_strdup(title);
    mDialog->buttonSpecs=buttonSpecs;
    return mDialog;
}

/*== Start set spec ===*/
static void maker_dialog_load_from_keyfile_section_main(MakerDialog *mDialog, GKeyFile *keyFile, MakerDialogError **error){
    MakerDialogError *cfgErr=NULL;
    if (g_key_file_has_group(keyFile, MAKER_DIALOG_SPEC_SECTION_MAIN)){
	if (g_key_file_has_key(keyFile, MAKER_DIALOG_SPEC_SECTION_MAIN, "title", &cfgErr)){
	    mDialog->title=g_key_file_get_string(keyFile, MAKER_DIALOG_SPEC_SECTION_MAIN, "title", &cfgErr);
	}else{
	    mDialog->title=g_strdup("Preference");
	}
	maker_dialog_error_handle(cfgErr, error);
	if (g_key_file_has_key(keyFile, MAKER_DIALOG_SPEC_SECTION_MAIN, "buttonResponseIds", &cfgErr)){
	    gchar *idBuf=g_key_file_get_string(keyFile, MAKER_DIALOG_SPEC_SECTION_MAIN, "buttonResponseIds", &cfgErr);
	    gchar **idStrs=maker_dialog_string_split_set(idBuf, ";", '\\', TRUE, -1);
	    gchar *buttonBuf=NULL;
	    gchar **buttonTexts=NULL;
	    if (g_key_file_has_key(keyFile, MAKER_DIALOG_SPEC_SECTION_MAIN, "buttonTexts", &cfgErr)){
		buttonBuf=g_key_file_get_string(keyFile, MAKER_DIALOG_SPEC_SECTION_MAIN, "buttonTexts", &cfgErr);
		buttonTexts=maker_dialog_string_split_set(buttonBuf, ";", '\\', TRUE, -1);
	    }
	    maker_dialog_error_handle(cfgErr, error);

	    gint i;
	    MakerDialogButtonSpec *spec=NULL;
	    GArray *specArray=g_array_new(FALSE, FALSE, sizeof(MakerDialogButtonSpec));
	    for(i=0;idStrs[i]!=NULL;i++){
		gint responseId=maker_dialog_button_parse_response_id(idStrs[i]);
		if (responseId!=MAKER_DIALOG_RESPONSE_NIL){
		    g_array_set_size(specArray, specArray->len+1);
		    spec=&g_array_index(specArray, MakerDialogButtonSpec, specArray->len-1);
		    spec->responseId=responseId;
		    if (buttonTexts && !maker_dialog_string_is_empty(buttonTexts[i])){
			spec->buttonText=g_strdup(buttonTexts[i]);
		    }else{
			spec->buttonText=NULL;
		    }
		}
	    }
	    g_array_set_size(specArray, specArray->len+1);
	    spec=&g_array_index(specArray, MakerDialogButtonSpec, specArray->len-1);
	    spec->responseId=MAKER_DIALOG_RESPONSE_NIL;
	    spec->buttonText=NULL;
	    mDialog->buttonSpecs=(MakerDialogButtonSpec *) g_array_free(specArray, FALSE);
	    g_free(idBuf);
	    g_strfreev(idStrs);
	    g_free(buttonBuf);
	    g_strfreev(buttonTexts);
	    g_assert(mDialog->buttonSpecs[0].responseId==MAKER_DIALOG_RESPONSE_CLOSE);
	}
    }
}

typedef void (* MakerDialogSetSpecFunc)(MakerDialogPropertySpec *spec, const gchar *prop, MkdgValue *mValue);
typedef struct{
    const gchar *prop;
    MkdgType mType;
    MakerDialogSetSpecFunc func;
} MakerDialogSetSpecData;

static MakerDialogIdDataPair mkdgWidgetControlData[]={
    {"SHOW",		{MAKER_DIALOG_WIDGET_CONTROL_SHOW}},
    {"HIDE",		{MAKER_DIALOG_WIDGET_CONTROL_HIDE}},
    {"SENSITIVE",	{MAKER_DIALOG_WIDGET_CONTROL_SENSITIVE}},
    {"INSENSITIVE",	{MAKER_DIALOG_WIDGET_CONTROL_INSENSITIVE}},
    {NULL,		{MAKER_DIALOG_WIDGET_CONTROL_NOTHING}},
};

static MakerDialogWidgetControl maker_dialog_widget_control_parse(const gchar *str){
    return maker_dialog_flag_parse(mkdgWidgetControlData, str, FALSE);
}

static MakerDialogIdDataPair mkdgWidgetRelationData[]={
    {"EQ",	{MAKER_DIALOG_RELATION_EQUAL}},
    {"NE",	{MAKER_DIALOG_RELATION_NOT_EQUAL}},
    {"LT",	{MAKER_DIALOG_RELATION_LESS}},
    {"LE",	{MAKER_DIALOG_RELATION_LESS_OR_EQUAL}},
    {"GT",	{MAKER_DIALOG_RELATION_GREATER}},
    {"GE",	{MAKER_DIALOG_RELATION_GREATER_OR_EQUAL}},
    {NULL,	{0}},
};

static MakerDialogRelation maker_dialog_widget_control_relation(const gchar *str){
    return (maker_dialog_id_parse(mkdgWidgetRelationData, str, FALSE))->data.v_int32;
}

static MakerDialogIdDataPair mkdgSpecFlagData[]={
    {"FIXED_SET",		{MAKER_DIALOG_PROPERTY_FLAG_FIXED_SET}},
    {"PREFER_RADIO_BUTTONS",	{MAKER_DIALOG_PROPERTY_FLAG_PREFER_RADIO_BUTTONS}},
    {NULL,			{0}},
};

static void maker_dialog_control_rule_set
(MakerDialogControlRule *rule, MakerDialogRelation relation, const gchar *testValue, const gchar *key, MakerDialogWidgetControl match, MakerDialogWidgetControl notMatch){
    rule->relation=relation;
    rule->testValue=g_strdup(testValue);
    rule->key=g_strdup(key);
    rule->match=match;
    rule->notMatch=notMatch;
}

static void mkdg_set_widget_control(MakerDialogPropertySpec *spec, const gchar *prop, MkdgValue *mValue){
    gchar **ctrlList=maker_dialog_string_split_set(maker_dialog_value_get_string(mValue), ";", '\\', FALSE, -1);
    gint i;
    GArray *ctrlArray=g_array_new(FALSE, FALSE, sizeof(MakerDialogControlRule));
    MakerDialogControlRule *rule=NULL;
    for(i=0;ctrlList[i]!=NULL;i++){
	gchar **strList=maker_dialog_string_split_set(ctrlList[i], ",", '\\', FALSE, 5);
	/* StrList[0] is relation */
	MakerDialogRelation relation=maker_dialog_widget_control_relation(strList[0]);
	if (relation<=0){
	    goto END_WIDGET_CONTROL_RULE;
	}
	g_array_set_size(ctrlArray, ctrlArray->len+1);
	rule=&g_array_index(ctrlArray, MakerDialogControlRule, ctrlArray->len-1);
	MakerDialogWidgetControl match=maker_dialog_widget_control_parse(strList[3]);
	MakerDialogWidgetControl notMatch=maker_dialog_widget_control_parse(strList[4]);
	maker_dialog_control_rule_set(rule, relation,strList[1],strList[2],match,notMatch);
END_WIDGET_CONTROL_RULE:
	g_strfreev(strList);
    }
    g_array_set_size(ctrlArray, ctrlArray->len+1);
    rule=&g_array_index(ctrlArray, MakerDialogControlRule, ctrlArray->len-1);
    maker_dialog_control_rule_set(rule, MAKER_DIALOG_RELATION_NIL, NULL, NULL, 0, 0);
    spec->rules=(MakerDialogControlRule *) g_array_free(ctrlArray, FALSE);
    g_strfreev(ctrlList);
}

static void mkdg_set_flags(MakerDialogPropertySpec *spec, const gchar *prop, MkdgValue *mValue){
    spec->flags=maker_dialog_flag_parse(mkdgSpecFlagData, maker_dialog_value_get_string(mValue), FALSE);
}

static void mkdg_set_string(MakerDialogPropertySpec *spec, const gchar *prop, MkdgValue *mValue){
    if (g_ascii_strcasecmp(prop, "defaultValue")==0){
	g_free((gchar *) spec->defaultValue);
	spec->defaultValue=g_strdup(maker_dialog_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(prop, "parseOption")==0){
	g_free((gchar *) spec->parseOption);
	spec->parseOption=g_strdup(maker_dialog_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(prop, "toStringFormat")==0){
	g_free((gchar *) spec->toStringFormat);
	spec->toStringFormat=g_strdup(maker_dialog_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(prop, "compareOption")==0){
	g_free((gchar *) spec->compareOption);
	spec->compareOption=g_strdup(maker_dialog_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(prop, "pageName")==0){
	g_free((gchar *) spec->pageName);
	spec->pageName=g_strdup(maker_dialog_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(prop, "groupName")==0){
	g_free((gchar *) spec->groupName);
	spec->groupName=g_strdup(maker_dialog_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(prop, "label")==0){
	g_free((gchar *) spec->label);
	spec->label=g_strdup(maker_dialog_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(prop, "translationContext")==0){
	g_free((gchar *) spec->translationContext);
	spec->translationContext=g_strdup(maker_dialog_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(prop, "tooltip")==0){
	g_free((gchar *) spec->tooltip);
	spec->tooltip=g_strdup(maker_dialog_value_get_string(mValue));
    }
}

static void mkdg_set_number(MakerDialogPropertySpec *spec, const gchar *prop, MkdgValue *mValue){
    if (g_ascii_strcasecmp(prop, "min")==0){
	spec->min=maker_dialog_value_get_double(mValue);
    }else if (g_ascii_strcasecmp(prop, "max")==0){
	spec->max=maker_dialog_value_get_double(mValue);
    }else if (g_ascii_strcasecmp(prop, "step")==0){
	spec->step=maker_dialog_value_get_double(mValue);
    }else if (g_ascii_strcasecmp(prop, "decimalDigits")==0){
	spec->decimalDigits=maker_dialog_value_get_int(mValue);
    }
}

static void mkdg_set_string_list(MakerDialogPropertySpec *spec, const gchar *prop, MkdgValue *mValue){
    gchar **strList=maker_dialog_string_split_set(maker_dialog_value_get_string(mValue), ";", '\\', FALSE, -1);
    if (g_ascii_strcasecmp(prop, "validValues")==0){
	if (spec->validValues){
	    g_strfreev(spec->validValues);
	}
	spec->validValues=strList;
	return;
    }else if (g_ascii_strcasecmp(prop, "imagePaths")==0){
	if (spec->imagePaths){
	    g_strfreev(spec->imagePaths);
	}
	spec->imagePaths=strList;
	return;
    }
    g_strfreev(strList);
}

static MakerDialogSetSpecData setSpecDatas[]={
    {"flags", 			MKDG_TYPE_STRING, mkdg_set_flags},
    {"defaultValue", 		MKDG_TYPE_STRING, mkdg_set_string},
    {"validValues", 		MKDG_TYPE_STRING, mkdg_set_string_list},
    {"parseOption", 		MKDG_TYPE_STRING, mkdg_set_string},
    {"toStringFormat", 		MKDG_TYPE_STRING, mkdg_set_string},
    {"compareOption", 		MKDG_TYPE_STRING, mkdg_set_string},
    {"min",	 		MKDG_TYPE_DOUBLE, mkdg_set_number},
    {"max",	 		MKDG_TYPE_DOUBLE, mkdg_set_number},
    {"step",	 		MKDG_TYPE_DOUBLE, mkdg_set_number},
    {"decimalDigits",		MKDG_TYPE_INT, mkdg_set_number},
    {"pageName", 		MKDG_TYPE_STRING, mkdg_set_string},
    {"groupName", 		MKDG_TYPE_STRING, mkdg_set_string},
    {"label", 			MKDG_TYPE_STRING, mkdg_set_string},
    {"translationContext", 	MKDG_TYPE_STRING, mkdg_set_string},
    {"tooltip",		 	MKDG_TYPE_STRING, mkdg_set_string},
    {"imagePaths",	 	MKDG_TYPE_STRING, mkdg_set_string_list},
    {"rules",	 		MKDG_TYPE_STRING, mkdg_set_widget_control},
    {NULL, MKDG_TYPE_INVALID, NULL},
};

static MakerDialogSetSpecData *find_set_spec_data(const gchar *prop){
    gint i;
    for(i=0; setSpecDatas[i].mType!=MKDG_TYPE_INVALID; i++){
	if (strcmp(prop, setSpecDatas[i].prop)==0)
	    return &setSpecDatas[i];
    }
    return NULL;
}

/*== End set spec ===*/
static void maker_dialog_load_from_keyfile_section_keys(MakerDialog *mDialog, GKeyFile *keyFile, MakerDialogError **error){
    MakerDialogError *cfgErr=NULL;
    gsize groupLen;
    gchar **groupList=g_key_file_get_groups(keyFile, &groupLen);
    gint i;
    for(i=0; groupList[i]!=NULL; i++){
	MAKER_DIALOG_DEBUG_MSG(4, "[I4] load_from_keyfile_section_keys() spec key=%s", groupList[i]);
	if (strcmp(groupList[i],MAKER_DIALOG_SPEC_SECTION_MAIN)==0)
	    continue;
	if (!g_key_file_has_key(keyFile, groupList[i], "valueType", &cfgErr)){
	    maker_dialog_error_handle(cfgErr,error);
	    continue;
	}
	gchar *valueTypeStr=g_key_file_get_string(keyFile, groupList[i], "valueType",  &cfgErr);
	if (cfgErr){
	    maker_dialog_error_handle(cfgErr,error);
	    g_free(valueTypeStr);
	    continue;
	}
	MAKER_DIALOG_DEBUG_MSG(5, "[I5] load_from_keyfile_section_keys() valueType=%s", groupList[i]);
	MkdgType mType=maker_dialog_type_parse(valueTypeStr);
	g_free(valueTypeStr);
	if (mType==MKDG_TYPE_INVALID)
	    continue;
	MakerDialogPropertySpec *spec=maker_dialog_property_spec_new(g_strdup(groupList[i]),mType);
	gchar **keyList=g_key_file_get_keys(keyFile, groupList[i], NULL,  &cfgErr);
	if (cfgErr){
	    maker_dialog_error_handle(cfgErr,error);
	    g_free(valueTypeStr);
	}
	gint j;
	for (j=0; keyList[j]!=NULL; j++){
	    MakerDialogSetSpecData *setSpecData=find_set_spec_data(keyList[j]);
	    if (!setSpecData)
		continue;
	    MkdgValue *mValue=maker_dialog_value_new(setSpecData->mType,  &cfgErr);
	    if (cfgErr){
		maker_dialog_error_handle(cfgErr,error);
	    }
	    switch(setSpecData->mType){
		case MKDG_TYPE_BOOLEAN:
		    maker_dialog_value_set_boolean(mValue, g_key_file_get_boolean(keyFile, groupList[i], keyList[j],  &cfgErr));
		    break;
		case MKDG_TYPE_INT:
		    maker_dialog_value_set_int(mValue, g_key_file_get_integer(keyFile, groupList[i], keyList[j],  &cfgErr));
		    break;
		case MKDG_TYPE_DOUBLE:
		    maker_dialog_value_set_double(mValue, g_key_file_get_double(keyFile, groupList[i], keyList[j],  &cfgErr));
		    break;
		case MKDG_TYPE_STRING:
		    maker_dialog_value_set_string(mValue, g_key_file_get_string(keyFile, groupList[i], keyList[j],  &cfgErr));
		    break;
		default:
		    maker_dialog_value_free(mValue);
		    continue;
	    }
	    if (cfgErr){
		maker_dialog_error_handle(cfgErr,error);
	    }
	    setSpecData->func(spec, keyList[j], mValue);
	    maker_dialog_value_free(mValue);
	}
	maker_dialog_add_property(mDialog, maker_dialog_property_context_new(spec, NULL));
	g_strfreev(keyList);
    }
    g_strfreev(groupList);
}

MakerDialog *maker_dialog_load_from_key_file(const gchar *filename, MakerDialogError **error){
    GKeyFile *keyFile=g_key_file_new();
    MakerDialogError *cfgErr=NULL;
    MakerDialog *mDialog=NULL;
    if (!g_key_file_load_from_file(keyFile, filename, G_KEY_FILE_NONE, &cfgErr)){
	maker_dialog_error_handle(cfgErr, error);
	goto FINAL_LOAD_FROM_KEYFILE;
    }
    mDialog=maker_dialog_new();
    maker_dialog_load_from_keyfile_section_main(mDialog, keyFile, &cfgErr);
    maker_dialog_error_handle(cfgErr,error);
    maker_dialog_load_from_keyfile_section_keys(mDialog, keyFile, &cfgErr);
    maker_dialog_error_handle(cfgErr,error);
    mDialog->flags|= MAKER_DIALOG_FLAG_FREE_ALL;
FINAL_LOAD_FROM_KEYFILE:
    g_key_file_free(keyFile);

    return mDialog;
}

static GNode *maker_dialog_prepare_page_node(MakerDialog *mDialog, const gchar *pageName){
    const gchar *pageName_tmp=(pageName)? pageName : MAKER_DIALOG_PAGE_UNNAMED;
    GNode *result=maker_dialog_find_page_node(mDialog, (gpointer) pageName_tmp);
    if (!result){
	result=g_node_new((gpointer) pageName_tmp);
	g_node_append(mDialog->pageRoot,result);
    }
    return result;
}

static GNode *maker_dialog_prepare_group_node(MakerDialog *mDialog, const gchar *pageName, const gchar *groupName){
    const gchar *groupName_tmp=(groupName)? groupName : MAKER_DIALOG_GROUP_UNNAMED;
    GNode *pageNode=maker_dialog_prepare_page_node(mDialog, pageName);
    GNode *result=maker_dialog_find_group_node(mDialog, pageName, (gpointer) groupName_tmp);
    if (!result){
	result=g_node_new((gpointer) groupName_tmp);
	g_node_append(pageNode,result);
    }
    return result;
}

void maker_dialog_add_property(MakerDialog *mDialog, MakerDialogPropertyContext *ctx){
    MAKER_DIALOG_DEBUG_MSG(2, "[I2] add_property( , %s)",ctx->spec->key);
    maker_dialog_property_table_insert(mDialog->propertyTable, ctx);
    GNode *propGroupNode=maker_dialog_prepare_group_node(mDialog, ctx->spec->pageName, ctx->spec->groupName);
    GNode *propKeyNode=g_node_new((gpointer) ctx->spec->key);
    g_node_append(propGroupNode,propKeyNode);
//    maker_dialog_property_get_default(ctx->spec);
    ctx->mDialog=mDialog;
}

void maker_dialog_destroy(MakerDialog *mDialog){
    MAKER_DIALOG_DEBUG_MSG(3, "[I3] destroy()");
    if (mDialog->ui){
	maker_dialog_ui_destroy(mDialog->ui);
    }
    if (mDialog->config){
        maker_dialog_config_free(mDialog->config);
    }

    g_node_destroy(mDialog->pageRoot);
    maker_dialog_property_table_destroy(mDialog->propertyTable);
    g_free(mDialog->title);
    if (mDialog->flags & MAKER_DIALOG_FLAG_FREE_ALL){
	/* Free button specs */
	gint i;
	for(i=0;mDialog->buttonSpecs[i].responseId!=MAKER_DIALOG_RESPONSE_NIL;i++){
	    g_free((gchar *) mDialog->buttonSpecs[i].buttonText);
	}
	g_free(mDialog->buttonSpecs);
    }
    g_free(mDialog);
}

MkdgValue *maker_dialog_get_value(MakerDialog *mDialog, const gchar *key){
    MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(mDialog, key);
    if (ctx->flags & MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_HAS_VALUE)
	return NULL;
    return ctx->value;
}

MakerDialogPropertyContext *maker_dialog_get_property_context(MakerDialog *mDialog, const gchar *key){
    return maker_dialog_property_table_lookup(mDialog->propertyTable, key);
}

gboolean maker_dialog_apply_value(MakerDialog *mDialog, const gchar *key){
    MAKER_DIALOG_DEBUG_MSG(2,"[I2] apply_value( , %s)",key);
    MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(mDialog, key);

    gboolean ret=TRUE;
    if (ctx->validateFunc && (!ctx->validateFunc(ctx->spec, ctx->value))){
	/* Value is invalid. */
	ret=FALSE;
    }
    if (ret && ctx->applyFunc){
	ctx->applyFunc(ctx,ctx->value);
	ctx->flags &= ~MAKER_DIALOG_PROPERTY_CONTEXT_FLAG_UNAPPLIED;
    }else{
	ret=FALSE;
    }

    return ret;
}

gboolean maker_dialog_set_value(MakerDialog *mDialog, const gchar *key, MkdgValue *value){
    MAKER_DIALOG_DEBUG_MSG(2,"[I2] set_value( , %s, )", key);
    MakerDialogPropertyContext *ctx=maker_dialog_get_property_context(mDialog, key);
    if (!value){
	return maker_dialog_property_set_default(ctx);
    }
    if (MAKER_DIALOG_DEBUG_RUN(3)){
	gchar *str=maker_dialog_value_to_string(value, ctx->spec->toStringFormat);
	g_debug("[I3] set_value( , %s, ) value=%s",ctx->spec->key, str);
	g_free(str);
    }
    gboolean ret=TRUE;
    if (ctx->validateFunc && (!ctx->validateFunc(ctx->spec, value))){
	/* Value is invalid. */
	ret=FALSE;
    }
    if (ret){
	if  (mDialog->ui){
	    if (mDialog->ui->toolkitInterface->widget_set_value){
		mDialog->ui->toolkitInterface->widget_set_value(mDialog->ui, ctx->spec->key, value);
	    }else{
		ret=FALSE;
	    }
	}
	maker_dialog_property_set_value_fast(ctx, value, -2);
    }
    return ret;
}

