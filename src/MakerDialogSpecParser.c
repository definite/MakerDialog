#include <string.h>
#include "MakerDialog.h"
#include "MakerDialogSpecParser.h"

/*=== Start Spec data prototype ===*/
typedef struct _MakerDialogSpecData MakerDialogSpecData;

/*
 * @param specData	Spec data
 * @param attr		Spec Attribute to be set.
 * @param value		String representation of value, to be set to attribute.
 * @retval \c TRUE for acceptable cases: value is read successfully.
 * @retval \c FALSE for unacceptable cases: fail to read value, or no value
 * for essential attr.
 *
 */
typedef gboolean (* MakerDialogSpecAttrParser)(MakerDialogSpecData *specData, gpointer attr, const gchar *value);

/*
 * Essential attr
 *
 * Esasential attrs are the attributes that will be used in _new functions.
 */
#define MAKER_DIALOG_SPEC_ESSENTIAL			0x1

/*
 * Don't applied default if value is not present
 */
#define MAKER_DIALOG_SPEC_NO_DEFAULT			0x2

struct _MakerDialogSpecData{
    const gchar *page;
    const gchar *attr;
    const gchar *defaultValue;		//<! Only for optional attribute.
    MkdgType    type;
    guint32	flags;
    MakerDialogSpecAttrParser parser;
};

#define MKDG_SPEC_DATA_END { NULL, NULL, NULL, MKDG_TYPE_INVALID, 0, NULL, NULL}

static gboolean maker_dialog_spec_attr_parser_check_type(MakerDialogSpecData *specData){
    if (specData->type<=MKDG_TYPE_INVALID){
	return FALSE;
    }if (specData->type>=MKDG_TYPE_NONE){
	return FALSE;
    }
    return TRUE;
}

static gboolean maker_dialog_spec_attr_parser_default(MakerDialogSpecData *specData, gpointer attr_ptr, const gchar *value){
    if (!maker_dialog_spec_attr_parser_check_type(specData)){
	return FALSE;
    }
    MkdgValue *mValue=maker_dialog_value_new(specData->type,NULL);
    MkdgValue *ret=maker_dialog_value_from_string(mValue, value, NULL);
    if (ret==NULL){
	if (specData->flags & MAKER_DIALOG_SPEC_ESSENTIAL){
	    // Essential attr that don't have value.
	    maker_dialog_value_free(mValue);
	    return FALSE;
	}
	maker_dialog_value_from_string(mValue, specData->defaultValue, NULL);
    }
    maker_dialog_value_extract(mValue, attr_ptr);
    maker_dialog_value_free(mValue);
    return TRUE;
}

static gboolean maker_dialog_spec_attr_parser_flags_config(MakerDialogSpecData *specData, gpointer attr_ptr, const gchar *value){
    g_assert(specData->type==MKDG_TYPE_UINT32);
    MakerDialogConfigFlags ret=maker_dialog_config_flags_parse(const gchar *str);
    MakerDialogConfigFlags *flags_ptr = (MakerDialogConfigFlags *) attr_ptr;
    *flags_ptr=ret;
    return FALSE;
}

static gboolean maker_dialog_spec_attr_parser_flags_config_set(MakerDialogSpecData *specData, gpointer attr_ptr, const gchar *value){
    g_assert(specData->type==MKDG_TYPE_UINT32);
    MakerDialogConfigFlags ret=maker_dialog_config_set_flags_parse(const gchar *str);
    MakerDialogConfigFlags *flags_ptr = (MakerDialogConfigFlags *) attr_ptr;
    *flags_ptr=ret;
    return FALSE;
}

#ifndef PREFERRED_UI_INTERFACE
#define PREFERRED_UI_INTERFACE	"GTK2"
#endif

#ifndef PREFERRED_CONFIG_INTERFACE
#define PREFERRED_CONFIG_INTERFACE	"GKEYFILE"
#endif

MakerDialogSpecData specDatas_main[]={
    {MAKER_DIALOG_SPEC_SECTION_MAIN,		"title",		"Properties",
	MKDG_TYPE_STRING,	0,
	maker_dialog_spec_attr_parser_default},
    {MAKER_DIALOG_SPEC_SECTION_MAIN,		"buttonSpec",		"",
	MKDG_TYPE_STRING,	0,
	maker_dialog_spec_attr_parser_default},
    {MAKER_DIALOG_SPEC_SECTION_MAIN,		"maxSizeInPixelX",	"-1",
	MKDG_TYPE_INT,		0,
	maker_dialog_spec_attr_parser_default},
    {MAKER_DIALOG_SPEC_SECTION_MAIN,		"maxSizeInPixelY",	"-1",
	MKDG_TYPE_INT,		0,
	maker_dialog_spec_attr_parser_default},
    {MAKER_DIALOG_SPEC_SECTION_MAIN,		"maxSizeInCharX",	"-1",
	MKDG_TYPE_INT,		0,
	maker_dialog_spec_attr_parser_default},
    {MAKER_DIALOG_SPEC_SECTION_MAIN,		"maxSizeInCharY",	"-1",
	MKDG_TYPE_INT,		0,
	maker_dialog_spec_attr_parser_default},
    {MAKER_DIALOG_SPEC_SECTION_MAIN,		"maxSizeInCharY",	"-1",
	MKDG_TYPE_INT,		0,
	maker_dialog_spec_attr_parser_default},
    {MAKER_DIALOG_SPEC_SECTION_MAIN,		"labelAlignmentX",	"0.5f",
	MKDG_TYPE_FLOAT,	0,
	maker_dialog_spec_attr_parser_default},
    {MAKER_DIALOG_SPEC_SECTION_MAIN,		"labelAlignmentY",	"0.5f",
	MKDG_TYPE_FLOAT,	0,
	maker_dialog_spec_attr_parser_default},
    {MAKER_DIALOG_SPEC_SECTION_MAIN,		"componentAlignmentX",	"0.5f",
	MKDG_TYPE_FLOAT,	0,
	maker_dialog_spec_attr_parser_default},
    {MAKER_DIALOG_SPEC_SECTION_MAIN,		"componentAlignmentY",	"0.5f",
	MKDG_TYPE_FLOAT,	0,
	maker_dialog_spec_attr_parser_default},

    {MAKER_DIALOG_SPEC_SECTION_UI,		"interfaces",		PREFERRED_UI_INTERFACE,
	MKDG_TYPE_STRING_LIST,	0,
	maker_dialog_spec_attr_parser_default},
    {MAKER_DIALOG_SPEC_SECTION_UI,		"flags",		"0",
	MKDG_TYPE_UINT32,		0,
	maker_dialog_spec_attr_parser_flags},

    {MAKER_DIALOG_SPEC_SECTION_CONFIG,		"interfaces",		PREFERRED_CONFIG_INTERFACE,
	MKDG_TYPE_STRING_LIST,	0,
	maker_dialog_spec_attr_parser_default},
    {MAKER_DIALOG_SPEC_SECTION_CONFIG,		"flags",		"0",
	MKDG_TYPE_UINT,		0,
	maker_dialog_spec_attr_parser_flags},
    MKDG_SPEC_DATA_END
};

MakerDialogSpecData specDatas_configSet[]={
    {MAKER_DIALOG_SPEC_SECTION_CONFIG_SET,	"interfaces",		PREFERRED_CONFIG_INTERFACE,
	MKDG_TYPE_STRING_LIST,	MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_default},
    {MAKER_DIALOG_SPEC_SECTION_CONFIG_SET,	"pageName",		NULL,
	MKDG_TYPE_STRING_LIST,	MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_default},
    {MAKER_DIALOG_SPEC_SECTION_CONFIG_SET,	"filePattern",		NULL,
	MKDG_TYPE_STRING,	MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_default},
    {MAKER_DIALOG_SPEC_SECTION_CONFIG_SET,	"searchDirs",		NULL,
	MKDG_TYPE_STRING_LIST,	MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_default},
    {MAKER_DIALOG_SPEC_SECTION_CONFIG_SET,	"defaultFilename",	NULL,
	MKDG_TYPE_STRING,	MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_default},
    {MAKER_DIALOG_SPEC_SECTION_CONFIG_SET,	"maxFileCount",		"-1",
	MKDG_TYPE_INT,		0,
	maker_dialog_spec_attr_parser_default},
    {MAKER_DIALOG_SPEC_SECTION_CONFIG_SET,	"flags",		"0",
       MKDG_TYPE_UINT32,		0,
       maker_dialog_spec_attr_parser_flags},
   MKDG_SPEC_DATA_END
};


MakerDialogSpecData specDatas_prop[]={
    {"_PROPERITY_",	"valueType",		"-1",
	MKDG_TYPE_INT,		MAKER_DIALOG_SPEC_ESSENTIAL | MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_default},
    {"_PROPERITY_",	"flags",		"0",
	MKDG_TYPE_UINT32,		0,
	maker_dialog_spec_attr_parser_flags},
    {"_PROPERITY_",	"defaultValue",		NULL,
	MKDG_TYPE_STRING, 	MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_default},
    {"_PROPERITY_",	"validValues",		NULL,
	MKDG_TYPE_STRING_LIST,	MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_default},
    {"_PROPERITY_",	"parseOption",		NULL,
	MKDG_TYPE_STRING,	MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_default},
    {"_PROPERITY_",	"toStringFormat",		NULL,
	MKDG_TYPE_STRING,	MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_default},
    {"_PROPERITY_",	"compareOption"			NULL,
	MKDG_TYPE_STRING,	MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_default},
    {"_PROPERITY_",	"min"				NULL,
	MKDG_TYPE_DOUBLE,	MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_default},
    {"_PROPERITY_",	"max"				NULL,
	MKDG_TYPE_DOUBLE,	MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_default},
    {"_PROPERITY_",	"step"				NULL,
	MKDG_TYPE_DOUBLE,	MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_default},
    {"_PROPERITY_",	"decimalDigits"			"2",
	MKDG_TYPE_INT,	0,
	maker_dialog_spec_attr_parser_default},
    {"_PROPERITY_",	"pageName",			MAKER_DIALOG_PAGE_UNNAMED,
	MKDG_TYPE_STRING,	0,
	maker_dialog_spec_attr_parser_default},
    {"_PROPERITY_",	"groupName",			MAKER_DIALOG_GROUP_UNNAMED,
	MKDG_TYPE_STRING,	0,
	maker_dialog_spec_attr_parser_default},
    {"_PROPERITY_",	"label",			NULL,
	MKDG_TYPE_STRING,	MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_label},
    {"_PROPERITY_",	"trnaslationContext",		NULL,
	MKDG_TYPE_STRING,	MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_default},
    {"_PROPERITY_",	"tooltip",			NULL,
	MKDG_TYPE_STRING,	MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_default},
    {"_PROPERITY_",	"imagePaths",			NULL,
	MKDG_TYPE_STRING_LIST,	MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_default},
    {"_PROPERITY_",	"rules",			NULL,
	MKDG_TYPE_STRING_LIST,	MAKER_DIALOG_SPEC_NO_DEFAULT,
	maker_dialog_spec_attr_parser_widget_control},
    MKDG_SPEC_DATA_END
};


typedef void (* MakerDialogSetSpecFunc)(MakerDialogPropertySpec *spec, const gchar *attr, MkdgValue *mValue);
typedef struct{
    const gchar *attr;
    MkdgType mType;
    MakerDialogSetSpecFunc func;
} MakerDialogSetSpecData;

/*
 * Silence the error if the key is not found.
 * This is useful of optional options
 */
static gchar *maker_dialog_g_keyfile_has_key_then_get_string(GKeyFile *keyFile, const gchar *keyFileGroup, const gchar *keyFileKey){
    if (!g_key_file_has_group(keyFile,keyFileGroup))
	return NULL;
    if (!g_key_file_has_key(keyFile,keyFileGroup,keyFileKey, NULL))
	return NULL;
    return g_key_file_get_string(keyFile,keyFileGroup,keyFileKey, NULL);
}

/*
 * error set to NULL means the attr is optional.
 * error set to non-NULL means the attr is essential.
 *
 * @retval TRUE if essential attr has value or reading an optional attr.
 * @retval FALSE if essential attr has no value.
 */
//static gboolean maker_dialog_spec_parser_load_attr(gpointer attr, MkdgType mType, GKeyFile *keyFile, const gchar *keyFileGroup, const gchar *keyFileKey,  MakerDialogError **error){
//    gboolean essential=(error) ? TRUE : FALSE;
//    MakerDialogError *cfgErr=NULL;
//    if (!g_key_file_has_group(keyFile,keyFileGroup)){
//        if (essential)
//            cfgErr=

//    }

//        return !essential;
//    if (!g_key_file_has_group(keyFile,keyFileGroup))
//        return !essential;



//}


static void maker_dialog_control_rule_set
(MakerDialogControlRule *rule, MakerDialogRelation relation, const gchar *testValue, const gchar *key, MakerDialogWidgetControl match, MakerDialogWidgetControl notMatch){
    rule->relation=relation;
    rule->testValue=g_strdup(testValue);
    rule->key=g_strdup(key);
    rule->match=match;
    rule->notMatch=notMatch;
}

static void mkdg_set_widget_control(MakerDialogPropertySpec *spec, const gchar *attr, MkdgValue *mValue){
    gchar **ctrlList=maker_dialog_string_split_set(maker_dialog_value_get_string(mValue), ";", '\\', FALSE, -1);
    gint i;
    GArray *ctrlArray=g_array_new(FALSE, FALSE, sizeof(MakerDialogControlRule));
    MakerDialogControlRule *rule=NULL;
    for(i=0;ctrlList[i]!=NULL;i++){
	gchar **strList=maker_dialog_string_split_set(ctrlList[i], ",", '\\', FALSE, 5);
	/* StrList[0] is relation */
	MakerDialogRelation relation=maker_dialog_relation_parse(strList[0]);
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

static void mkdg_set_flags(MakerDialogPropertySpec *spec, const gchar *attr, MkdgValue *mValue){
    spec->flags=maker_dialog_property_flags_parse(maker_dialog_value_get_string(mValue));
}

static void mkdg_set_string(MakerDialogPropertySpec *spec, const gchar *attr, MkdgValue *mValue){
    if (g_ascii_strcasecmp(attr, "defaultValue")==0){
	g_free((gchar *) spec->defaultValue);
	spec->defaultValue=g_strdup(maker_dialog_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(attr, "parseOption")==0){
	g_free((gchar *) spec->parseOption);
	spec->parseOption=g_strdup(maker_dialog_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(attr, "toStringFormat")==0){
	g_free((gchar *) spec->toStringFormat);
	spec->toStringFormat=g_strdup(maker_dialog_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(attr, "compareOption")==0){
	g_free((gchar *) spec->compareOption);
	spec->compareOption=g_strdup(maker_dialog_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(attr, "pageName")==0){
	g_free((gchar *) spec->pageName);
	spec->pageName=g_strdup(maker_dialog_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(attr, "groupName")==0){
	g_free((gchar *) spec->groupName);
	spec->groupName=g_strdup(maker_dialog_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(attr, "label")==0){
	g_free((gchar *) spec->label);
	spec->label=g_strdup(maker_dialog_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(attr, "translationContext")==0){
	g_free((gchar *) spec->translationContext);
	spec->translationContext=g_strdup(maker_dialog_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(attr, "tooltip")==0){
	g_free((gchar *) spec->tooltip);
	spec->tooltip=g_strdup(maker_dialog_value_get_string(mValue));
    }
}

static void mkdg_set_number(MakerDialogPropertySpec *spec, const gchar *attr, MkdgValue *mValue){
    if (g_ascii_strcasecmp(attr, "min")==0){
	spec->min=maker_dialog_value_get_double(mValue);
    }else if (g_ascii_strcasecmp(attr, "max")==0){
	spec->max=maker_dialog_value_get_double(mValue);
    }else if (g_ascii_strcasecmp(attr, "step")==0){
	spec->step=maker_dialog_value_get_double(mValue);
    }else if (g_ascii_strcasecmp(attr, "decimalDigits")==0){
	spec->decimalDigits=maker_dialog_value_get_int(mValue);
    }
}

static void mkdg_set_string_list(MakerDialogPropertySpec *spec, const gchar *attr, MkdgValue *mValue){
    gchar **strList=maker_dialog_string_split_set(maker_dialog_value_get_string(mValue), ";", '\\', FALSE, -1);
    if (g_ascii_strcasecmp(attr, "validValues")==0){
	if (spec->validValues){
	    g_strfreev(spec->validValues);
	}
	spec->validValues=strList;
	return;
    }else if (g_ascii_strcasecmp(attr, "imagePaths")==0){
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

static MakerDialogSetSpecData *find_set_spec_data(const gchar *attr){
    gint i;
    for(i=0; setSpecDatas[i].mType!=MKDG_TYPE_INVALID; i++){
	if (strcmp(attr, setSpecDatas[i].attr)==0)
	    return &setSpecDatas[i];
    }
    return NULL;
}

/*== End set spec ===*/
static void maker_dialog_new_from_key_file_section_keys(MakerDialog *mDialog, GKeyFile *keyFile, MakerDialogError **error){
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


static void maker_dialog_new_from_key_file_section_main(MakerDialog *mDialog, GKeyFile *keyFile, MakerDialogError **error){
    MakerDialogError *cfgErr=NULL;
    if (!g_key_file_has_group(keyFile, MAKER_DIALOG_SPEC_SECTION_MAIN))
	return;
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
	    gint responseId=maker_dialog_parse_button_response_id(idStrs[i]);
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

static void maker_dialog_new_from_key_file_section_config(MakerDialog *mDialog, GKeyFile *keyFile, MakerDialogError **error){
    MakerDialogError *cfgErr=NULL;
    if (!g_key_file_has_group(keyFile, MAKER_DIALOG_SPEC_SECTION_CONFIG))
	return;
    MakerDialogConfig *config=maker_dialog_config_new(mDialog);
    if (g_key_file_has_key(keyFile, MAKER_DIALOG_SPEC_SECTION_CONFIG, "flags", &cfgErr)){
	gchar *flagsStr=g_key_file_get_string(keyFile, MAKER_DIALOG_SPEC_SECTION_CONFIG, "flags", &cfgErr);
	config->flags=maker_dialog_config_flags_parse(flagsStr);
	g_free(flagsStr);
    }

    gboolean loaded=FALSE;
    if (g_key_file_has_key(keyFile, MAKER_DIALOG_SPEC_SECTION_CONFIG, "configInterface", &cfgErr)){
	gchar *configInterfaceStr=g_key_file_get_string(keyFile, MAKER_DIALOG_SPEC_SECTION_CONFIG, "configInterface", &cfgErr);
	if (cfgErr==NULL){
	    gchar **configInterfaceStrList=g_strsplit(configInterfaceStr,";",-1);
	    gint i;
	    for (i=0; configInterfaceStrList[i]!=NULL;i++){
		MAKER_DIALOG_MODULE module=maker_dialog_module_parse(configInterfaceStrList[i]);
		switch(module){
		    case MAKER_DIALOG_MODULE_GCONF2:
		    case MAKER_DIALOG_MODULE_GTK2:
			if (maker_dialog_module_load(mDialog, module, &cfgErr)){
			    loaded=TRUE;
			}else{
			    maker_dialog_error_handle(cfgErr, error);
			}
			break;
		    default:
			break;
		}
		if (loaded)
		    break;
	    }
	    g_strfreev(configInterfaceStrList);
	}else{
	    maker_dialog_error_handle(cfgErr, error);
	}
	g_free(configInterfaceStr);
    }else{
	maker_dialog_error_handle(cfgErr, error);
    }
}

//static gboolean maker_dialog_spec_parser_config_set_load_attr(GKeyFile *keyFile,

static void maker_dialog_new_from_key_file_section_config_set(MakerDialog *mDialog, GKeyFile *keyFile, MakerDialogError **error){
    MakerDialogError *cfgErr=NULL;
    gsize groupLen;
    gchar **groupList=g_key_file_get_groups(keyFile, &groupLen);
    gsize i;
    for(i=0; groupList[i]!=NULL && i < groupLen ; i++){
	if (!g_str_has_prefix(groupList[i], MAKER_DIALOG_SPEC_SECTION_CONFIG_SET)){
	    continue;
	}
	MAKER_DIALOG_DEBUG_MSG(4, "[I4] new_from_key_file_section_config_set() keyfileGroupName=%s", groupList[i]);
	MakerDialogConfigSet *configSet=maker_dialog_config_set_new();

	gchar *valueStr=NULL;
	if ((valueStr=maker_dialog_g_keyfile_has_key_then_get_string(keyFile, groupList[i], "pageNames"))!=NULL){
//	    configSet->pageNames=valueStr;
	}else{
//	    configSet->pageNames=NULL;
	}
    }
}

MakerDialog *maker_dialog_new_from_key_file(const gchar *filename, MakerDialogError **error){
    GKeyFile *keyFile=g_key_file_new();
    MakerDialogError *cfgErr=NULL;
    MakerDialog *mDialog=NULL;
    if (!g_key_file_load_from_file(keyFile, filename, G_KEY_FILE_NONE, &cfgErr)){
	maker_dialog_error_handle(cfgErr, error);
	goto FINAL_LOAD_FROM_KEYFILE;
    }
    mDialog=maker_dialog_new();
    maker_dialog_new_from_key_file_section_main(mDialog, keyFile, &cfgErr);
    maker_dialog_error_handle(cfgErr,error);
    maker_dialog_new_from_key_file_section_keys(mDialog, keyFile, &cfgErr);
    maker_dialog_error_handle(cfgErr,error);
    mDialog->flags|= MAKER_DIALOG_FLAG_FREE_ALL;
FINAL_LOAD_FROM_KEYFILE:
    g_key_file_free(keyFile);

    return mDialog;
}

