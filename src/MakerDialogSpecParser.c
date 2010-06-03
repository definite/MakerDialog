#include <string.h>
#include "MakerDialog.h"
#include "MakerDialogSpecParser.h"

const gchar *MKDG_SPEC_SECTION_NAMES[]={
    "_MAIN_",
    "_CONFIG_",
    "_CONFIG_SET_",
    "_UI_",
    "_PROPERTY_",
    NULL
};

/*=== Start Spec data prototype ===*/
typedef struct _MkdgSpecData MkdgSpecData;

/*
 * @param specData	Spec data
 * @param attr		Spec Attribute to be set.
 * @param value		String representation of value, to be set to attribute.
 * @retval \c TRUE for acceptable cases: value is read successfully.
 * @retval \c FALSE for unacceptable cases: fail to read value, or no value
 * for essential attr.
 *
 */
typedef gboolean (* MkdgSpecAttrParser)(MkdgSpecData *specData, gpointer attr, const gchar *value);

/*
 * Essential attr
 *
 * Esasential attrs are the attributes that will be used in _new functions.
 */
#define MKDG_SPEC_ESSENTIAL			0x1

/*
 * Don't applied default if value is not present
 */
#define MKDG_SPEC_NO_DEFAULT			0x2

struct _MkdgSpecData{
    MkdgSpecSection             section;
    const gchar			*attr;
    const gchar			*defaultValue;		//<! Only for optional attribute.
    MkdgType			type;
    MkdgFlags		flags;
    MkdgSpecAttrParser	parser;
};

#define MKDG_SPEC_DATA_END { MKDG_SPEC_SECTION_INVALID, NULL, NULL, MKDG_TYPE_INVALID, 0, NULL}
#define MKDG_SPEC_SECTION_PROPERTY_STRING	"_PROPERTY_"

static gboolean mkdg_spec_attr_parser_check_type(MkdgSpecData *specData){
    if (specData->type<=MKDG_TYPE_INVALID){
	return FALSE;
    }if (specData->type>=MKDG_TYPE_NONE){
	return FALSE;
    }
    return TRUE;
}

static gboolean mkdg_spec_attr_parser_default(MkdgSpecData *specData, gpointer attr_ptr, const gchar *value){
    if (!mkdg_spec_attr_parser_check_type(specData)){
	return FALSE;
    }
    MkdgValue *mValue=mkdg_value_new(specData->type,NULL);
    MkdgValue *ret=mkdg_value_from_string(mValue, value, NULL);
    if (ret==NULL){
	if (specData->flags & MKDG_SPEC_ESSENTIAL){
	    // Essential attr that don't have value.
	    mkdg_value_free(mValue);
	    return FALSE;
	}
	mkdg_value_from_string(mValue, specData->defaultValue, NULL);
    }
    mkdg_value_extract(mValue, attr_ptr);
    mkdg_value_free(mValue);
    return TRUE;
}

static gboolean mkdg_spec_attr_parser_flags(MkdgSpecData *specData, gpointer attr_ptr, const gchar *value){
    if (!mkdg_spec_attr_parser_check_type(specData)){
	return FALSE;
    }
    g_assert(specData->type==MKDG_TYPE_UINT32);
    MkdgConfigFlags ret=0;
    switch(specData->section){
	case MKDG_SPEC_SECTION_MAIN:
	    break;
	case MKDG_SPEC_SECTION_CONFIG:
	    ret=mkdg_config_flags_parse(value);
	    break;
	case MKDG_SPEC_SECTION_CONFIG_SET:
	    ret=mkdg_config_set_flags_parse(value);
	    break;
	case MKDG_SPEC_SECTION_UI:
	    return TRUE;
	case MKDG_SPEC_SECTION_PROPERTY:
	    break;
	default:
	    return FALSE;
    }
    MkdgConfigFlags *attr_ptr_casted = (MkdgConfigFlags *) attr_ptr;
    *attr_ptr_casted=ret;
    return TRUE;
}

static gboolean mkdg_spec_attr_parser_widget_control(MkdgSpecData *specData, gpointer attr_ptr, const gchar *value){
    if (!mkdg_spec_attr_parser_check_type(specData)){
	return FALSE;
    }
    MkdgWidgetControl ret=mkdg_widget_control_parse(value);
    MkdgWidgetControl *attr_ptr_casted = (MkdgWidgetControl *) attr_ptr;
    *attr_ptr_casted=ret;
    return TRUE;
}

#ifndef PREFERRED_UI_INTERFACE
#define PREFERRED_UI_INTERFACE	"GTK2"
#endif

#ifndef PREFERRED_CONFIG_INTERFACE
#define PREFERRED_CONFIG_INTERFACE	"GKEYFILE"
#endif

MkdgSpecData specDatas_main[]={
    {MKDG_SPEC_SECTION_MAIN_STRING,		"title",		"Properties",
	MKDG_TYPE_STRING,	0,
	mkdg_spec_attr_parser_default},
    {MKDG_SPEC_SECTION_MAIN_STRING,		"buttonSpec",		"",
	MKDG_TYPE_STRING,	0,
	mkdg_spec_attr_parser_default},
    {MKDG_SPEC_SECTION_MAIN_STRING,		"maxSizeInPixelX",	"-1",
	MKDG_TYPE_INT,		0,
	mkdg_spec_attr_parser_default},
    {MKDG_SPEC_SECTION_MAIN_STRING,		"maxSizeInPixelY",	"-1",
	MKDG_TYPE_INT,		0,
	mkdg_spec_attr_parser_default},
    {MKDG_SPEC_SECTION_MAIN_STRING,		"maxSizeInCharX",	"-1",
	MKDG_TYPE_INT,		0,
	mkdg_spec_attr_parser_default},
    {MKDG_SPEC_SECTION_MAIN_STRING,		"maxSizeInCharY",	"-1",
	MKDG_TYPE_INT,		0,
	mkdg_spec_attr_parser_default},
    {MKDG_SPEC_SECTION_MAIN_STRING,		"maxSizeInCharY",	"-1",
	MKDG_TYPE_INT,		0,
	mkdg_spec_attr_parser_default},
    {MKDG_SPEC_SECTION_MAIN_STRING,		"labelAlignmentX",	"0.5f",
	MKDG_TYPE_FLOAT,	0,
	mkdg_spec_attr_parser_default},
    {MKDG_SPEC_SECTION_MAIN_STRING,		"labelAlignmentY",	"0.5f",
	MKDG_TYPE_FLOAT,	0,
	mkdg_spec_attr_parser_default},
    {MKDG_SPEC_SECTION_MAIN_STRING,		"componentAlignmentX",	"0.5f",
	MKDG_TYPE_FLOAT,	0,
	mkdg_spec_attr_parser_default},
    {MKDG_SPEC_SECTION_MAIN_STRING,		"componentAlignmentY",	"0.5f",
	MKDG_TYPE_FLOAT,	0,
	mkdg_spec_attr_parser_default},

    {MKDG_SPEC_SECTION_UI_STRING,		"interfaces",		PREFERRED_UI_INTERFACE,
	MKDG_TYPE_STRING_LIST,	0,
	mkdg_spec_attr_parser_default},
    {MKDG_SPEC_SECTION_UI_STRING,		"flags",		"0",
	MKDG_TYPE_UINT32,		0,
	mkdg_spec_attr_parser_flags},

    {MKDG_SPEC_SECTION_CONFIG_STRING,		"interfaces",		PREFERRED_CONFIG_INTERFACE,
	MKDG_TYPE_STRING_LIST,	0,
	mkdg_spec_attr_parser_default},
    {MKDG_SPEC_SECTION_CONFIG_STRING,		"flags",		"0",
	MKDG_TYPE_UINT,		0,
	mkdg_spec_attr_parser_flags},
    MKDG_SPEC_DATA_END
};

MkdgSpecData specDatas_configSet[]={
    {MKDG_SPEC_SECTION_CONFIG_SET_STRING,	"interfaces",		PREFERRED_CONFIG_INTERFACE,
	MKDG_TYPE_STRING_LIST,	MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_default},
    {MKDG_SPEC_SECTION_CONFIG_SET_STRING,	"pageName",		NULL,
	MKDG_TYPE_STRING_LIST,	MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_default},
    {MKDG_SPEC_SECTION_CONFIG_SET_STRING,	"filePattern",		NULL,
	MKDG_TYPE_STRING,	MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_default},
    {MKDG_SPEC_SECTION_CONFIG_SET_STRING,	"searchDirs",		NULL,
	MKDG_TYPE_STRING_LIST,	MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_default},
    {MKDG_SPEC_SECTION_CONFIG_SET_STRING,	"defaultFilename",	NULL,
	MKDG_TYPE_STRING,	MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_default},
    {MKDG_SPEC_SECTION_CONFIG_SET_STRING,	"maxFileCount",		"-1",
	MKDG_TYPE_INT,		0,
	mkdg_spec_attr_parser_default},
    {MKDG_SPEC_SECTION_CONFIG_SET_STRING,	"flags",		"0",
       MKDG_TYPE_UINT32,		0,
       mkdg_spec_attr_parser_flags},
   MKDG_SPEC_DATA_END
};

MkdgSpecData specDatas_prop[]={
    {MKDG_SECTION_PROPERITY,	"valueType",		"-1",
	MKDG_TYPE_INT,		MKDG_SPEC_ESSENTIAL | MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_default},
    {MKDG_SECTION_PROPERITY,	"flags",		"0",
	MKDG_TYPE_UINT32,		0,
	mkdg_spec_attr_parser_flags},
    {MKDG_SECTION_PROPERITY,	"defaultValue",		NULL,
	MKDG_TYPE_STRING, 	MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_default},
    {MKDG_SECTION_PROPERITY,	"validValues",		NULL,
	MKDG_TYPE_STRING_LIST,	MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_default},
    {MKDG_SECTION_PROPERITY,	"parseOption",		NULL,
	MKDG_TYPE_STRING,	MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_default},
    {MKDG_SECTION_PROPERITY,	"toStringFormat",		NULL,
	MKDG_TYPE_STRING,	MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_default},
    {MKDG_SECTION_PROPERITY,	"compareOption"			NULL,
	MKDG_TYPE_STRING,	MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_default},
    {MKDG_SECTION_PROPERITY,	"min"				NULL,
	MKDG_TYPE_DOUBLE,	MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_default},
    {MKDG_SECTION_PROPERITY,	"max"				NULL,
	MKDG_TYPE_DOUBLE,	MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_default},
    {MKDG_SECTION_PROPERITY,	"step"				NULL,
	MKDG_TYPE_DOUBLE,	MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_default},
    {MKDG_SECTION_PROPERITY,	"decimalDigits"			"2",
	MKDG_TYPE_INT,	0,
	mkdg_spec_attr_parser_default},
    {MKDG_SECTION_PROPERITY,	"pageName",			MKDG_PAGE_UNNAMED,
	MKDG_TYPE_STRING,	0,
	mkdg_spec_attr_parser_default},
    {MKDG_SECTION_PROPERITY,	"groupName",			MKDG_GROUP_UNNAMED,
	MKDG_TYPE_STRING,	0,
	mkdg_spec_attr_parser_default},
    {MKDG_SECTION_PROPERITY,	"label",			NULL,
	MKDG_TYPE_STRING,	MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_label},
    {MKDG_SECTION_PROPERITY,	"trnaslationContext",		NULL,
	MKDG_TYPE_STRING,	MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_default},
    {MKDG_SECTION_PROPERITY,	"tooltip",			NULL,
	MKDG_TYPE_STRING,	MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_default},
    {MKDG_SECTION_PROPERITY,	"imagePaths",			NULL,
	MKDG_TYPE_STRING_LIST,	MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_default},
    {MKDG_SECTION_PROPERITY,	"rules",			NULL,
	MKDG_TYPE_STRING_LIST,	MKDG_SPEC_NO_DEFAULT,
	mkdg_spec_attr_parser_rule},
    MKDG_SPEC_DATA_END
};

typedef void (* MkdgSetSpecFunc)(MkdgPropertySpec *spec, const gchar *attr, MkdgValue *mValue);
typedef struct{
    const gchar *attr;
    MkdgType mType;
    MkdgSetSpecFunc func;
} MkdgSetSpecData;

/*
 * Silence the error if the key is not found.
 * This is useful of optional options
 */
static gchar *mkdg_g_keyfile_has_key_then_get_string(GKeyFile *keyFile, const gchar *keyFileGroup, const gchar *keyFileKey){
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
//static gboolean mkdg_spec_parser_load_attr(gpointer attr, MkdgType mType, GKeyFile *keyFile, const gchar *keyFileGroup, const gchar *keyFileKey,  MkdgError **error){
//    gboolean essential=(error) ? TRUE : FALSE;
//    MkdgError *cfgErr=NULL;
//    if (!g_key_file_has_group(keyFile,keyFileGroup)){
//        if (essential)
//            cfgErr=

//    }

//        return !essential;
//    if (!g_key_file_has_group(keyFile,keyFileGroup))
//        return !essential;
//}

static void mkdg_set_widget_control(MkdgPropertySpec *spec, const gchar *attr, MkdgValue *mValue){
    gchar **ctrlList=mkdg_string_split_set(mkdg_value_get_string(mValue), ";", '\\', FALSE, -1);
    gint i;
    GArray *ctrlArray=g_array_new(FALSE, FALSE, sizeof(MkdgControlRule));
    MkdgControlRule *rule=NULL;
    for(i=0;ctrlList[i]!=NULL;i++){
	gchar **strList=mkdg_string_split_set(ctrlList[i], ",", '\\', FALSE, 5);
	/* StrList[0] is relation */
	MkdgRelation relation=mkdg_relation_parse(strList[0]);
	if (relation<=0){
	    goto END_WIDGET_CONTROL_RULE;
	}
	g_array_set_size(ctrlArray, ctrlArray->len+1);
	rule=&g_array_index(ctrlArray, MkdgControlRule, ctrlArray->len-1);
	MkdgWidgetControl match=mkdg_widget_control_parse(strList[3]);
	MkdgWidgetControl notMatch=mkdg_widget_control_parse(strList[4]);
	mkdg_control_rule_set(rule, relation,strList[1],strList[2],match,notMatch);
END_WIDGET_CONTROL_RULE:
	g_strfreev(strList);
    }
    g_array_set_size(ctrlArray, ctrlArray->len+1);
    rule=&g_array_index(ctrlArray, MkdgControlRule, ctrlArray->len-1);
    mkdg_control_rule_set(rule, MKDG_RELATION_NIL, NULL, NULL, 0, 0);
    spec->rules=(MkdgControlRule *) g_array_free(ctrlArray, FALSE);
    g_strfreev(ctrlList);
}

static void mkdg_set_flags(MkdgPropertySpec *spec, const gchar *attr, MkdgValue *mValue){
    spec->flags=mkdg_property_flags_parse(mkdg_value_get_string(mValue));
}

static void mkdg_set_string(MkdgPropertySpec *spec, const gchar *attr, MkdgValue *mValue){
    if (g_ascii_strcasecmp(attr, "defaultValue")==0){
	g_free((gchar *) spec->defaultValue);
	spec->defaultValue=g_strdup(mkdg_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(attr, "parseOption")==0){
	g_free((gchar *) spec->parseOption);
	spec->parseOption=g_strdup(mkdg_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(attr, "toStringFormat")==0){
	g_free((gchar *) spec->toStringFormat);
	spec->toStringFormat=g_strdup(mkdg_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(attr, "compareOption")==0){
	g_free((gchar *) spec->compareOption);
	spec->compareOption=g_strdup(mkdg_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(attr, "pageName")==0){
	g_free((gchar *) spec->pageName);
	spec->pageName=g_strdup(mkdg_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(attr, "groupName")==0){
	g_free((gchar *) spec->groupName);
	spec->groupName=g_strdup(mkdg_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(attr, "label")==0){
	g_free((gchar *) spec->label);
	spec->label=g_strdup(mkdg_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(attr, "translationContext")==0){
	g_free((gchar *) spec->translationContext);
	spec->translationContext=g_strdup(mkdg_value_get_string(mValue));
    }else if (g_ascii_strcasecmp(attr, "tooltip")==0){
	g_free((gchar *) spec->tooltip);
	spec->tooltip=g_strdup(mkdg_value_get_string(mValue));
    }
}

static void mkdg_set_number(MkdgPropertySpec *spec, const gchar *attr, MkdgValue *mValue){
    if (g_ascii_strcasecmp(attr, "min")==0){
	spec->min=mkdg_value_get_double(mValue);
    }else if (g_ascii_strcasecmp(attr, "max")==0){
	spec->max=mkdg_value_get_double(mValue);
    }else if (g_ascii_strcasecmp(attr, "step")==0){
	spec->step=mkdg_value_get_double(mValue);
    }else if (g_ascii_strcasecmp(attr, "decimalDigits")==0){
	spec->decimalDigits=mkdg_value_get_int(mValue);
    }
}

static void mkdg_set_string_list(MkdgPropertySpec *spec, const gchar *attr, MkdgValue *mValue){
    gchar **strList=mkdg_string_split_set(mkdg_value_get_string(mValue), ";", '\\', FALSE, -1);
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

static MkdgSetSpecData setSpecDatas[]={
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

static MkdgSetSpecData *find_set_spec_data(const gchar *attr){
    gint i;
    for(i=0; setSpecDatas[i].mType!=MKDG_TYPE_INVALID; i++){
	if (strcmp(attr, setSpecDatas[i].attr)==0)
	    return &setSpecDatas[i];
    }
    return NULL;
}

/*== End set spec ===*/
static void mkdg_new_from_key_file_section_keys(Mkdg *mDialog, GKeyFile *keyFile, MkdgError **error){
    MkdgError *cfgErr=NULL;
    gsize groupLen;
    gchar **groupList=g_key_file_get_groups(keyFile, &groupLen);
    gint i;
    for(i=0; groupList[i]!=NULL; i++){
	MKDG_DEBUG_MSG(4, "[I4] load_from_keyfile_section_keys() spec key=%s", groupList[i]);
	if (strcmp(groupList[i],MKDG_SPEC_SECTION_MAIN_STRING)==0)
	    continue;
	if (!g_key_file_has_key(keyFile, groupList[i], "valueType", &cfgErr)){
	    mkdg_error_handle(cfgErr,error);
	    continue;
	}
	gchar *valueTypeStr=g_key_file_get_string(keyFile, groupList[i], "valueType",  &cfgErr);
	if (cfgErr){
	    mkdg_error_handle(cfgErr,error);
	    g_free(valueTypeStr);
	    continue;
	}
	MKDG_DEBUG_MSG(5, "[I5] load_from_keyfile_section_keys() valueType=%s", groupList[i]);
	MkdgType mType=mkdg_type_parse(valueTypeStr);
	g_free(valueTypeStr);
	if (mType==MKDG_TYPE_INVALID)
	    continue;
	MkdgPropertySpec *spec=mkdg_property_spec_new(g_strdup(groupList[i]),mType);
	gchar **keyList=g_key_file_get_keys(keyFile, groupList[i], NULL,  &cfgErr);
	if (cfgErr){
	    mkdg_error_handle(cfgErr,error);
	    g_free(valueTypeStr);
	}
	gint j;
	for (j=0; keyList[j]!=NULL; j++){
	    MkdgSetSpecData *setSpecData=find_set_spec_data(keyList[j]);
	    if (!setSpecData)
		continue;
	    MkdgValue *mValue=mkdg_value_new(setSpecData->mType,  &cfgErr);
	    if (cfgErr){
		mkdg_error_handle(cfgErr,error);
	    }
	    switch(setSpecData->mType){
		case MKDG_TYPE_BOOLEAN:
		    mkdg_value_set_boolean(mValue, g_key_file_get_boolean(keyFile, groupList[i], keyList[j],  &cfgErr));
		    break;
		case MKDG_TYPE_INT:
		    mkdg_value_set_int(mValue, g_key_file_get_integer(keyFile, groupList[i], keyList[j],  &cfgErr));
		    break;
		case MKDG_TYPE_DOUBLE:
		    mkdg_value_set_double(mValue, g_key_file_get_double(keyFile, groupList[i], keyList[j],  &cfgErr));
		    break;
		case MKDG_TYPE_STRING:
		    mkdg_value_set_string(mValue, g_key_file_get_string(keyFile, groupList[i], keyList[j],  &cfgErr));
		    break;
		default:
		    mkdg_value_free(mValue);
		    continue;
	    }
	    if (cfgErr){
		mkdg_error_handle(cfgErr,error);
	    }
	    setSpecData->func(spec, keyList[j], mValue);
	    mkdg_value_free(mValue);
	}
	mkdg_add_property(mDialog, mkdg_property_context_new(spec, NULL));
	g_strfreev(keyList);
    }
    g_strfreev(groupList);
}


static void mkdg_new_from_key_file_section_main(Mkdg *mDialog, GKeyFile *keyFile, MkdgError **error){
    MkdgError *cfgErr=NULL;
    if (!g_key_file_has_group(keyFile, MKDG_SPEC_SECTION_MAIN_STRING))
	return;
    if (g_key_file_has_key(keyFile, MKDG_SPEC_SECTION_MAIN_STRING, "title", &cfgErr)){
	mDialog->title=g_key_file_get_string(keyFile, MKDG_SPEC_SECTION_MAIN_STRING, "title", &cfgErr);
    }else{
	mDialog->title=g_strdup("Preference");
    }
    mkdg_error_handle(cfgErr, error);
    if (g_key_file_has_key(keyFile, MKDG_SPEC_SECTION_MAIN_STRING, "buttonResponseIds", &cfgErr)){
	gchar *idBuf=g_key_file_get_string(keyFile, MKDG_SPEC_SECTION_MAIN_STRING, "buttonResponseIds", &cfgErr);
	gchar **idStrs=mkdg_string_split_set(idBuf, ";", '\\', TRUE, -1);
	gchar *buttonBuf=NULL;
	gchar **buttonTexts=NULL;
	if (g_key_file_has_key(keyFile, MKDG_SPEC_SECTION_MAIN_STRING, "buttonTexts", &cfgErr)){
	    buttonBuf=g_key_file_get_string(keyFile, MKDG_SPEC_SECTION_MAIN_STRING, "buttonTexts", &cfgErr);
	    buttonTexts=mkdg_string_split_set(buttonBuf, ";", '\\', TRUE, -1);
	}
	mkdg_error_handle(cfgErr, error);

	gint i;
	MkdgButtonSpec *spec=NULL;
	GArray *specArray=g_array_new(FALSE, FALSE, sizeof(MkdgButtonSpec));
	for(i=0;idStrs[i]!=NULL;i++){
	    gint responseId=mkdg_parse_button_response_id(idStrs[i]);
	    if (responseId!=MKDG_RESPONSE_NIL){
		g_array_set_size(specArray, specArray->len+1);
		spec=&g_array_index(specArray, MkdgButtonSpec, specArray->len-1);
		spec->responseId=responseId;
		if (buttonTexts && !mkdg_string_is_empty(buttonTexts[i])){
		    spec->buttonText=g_strdup(buttonTexts[i]);
		}else{
		    spec->buttonText=NULL;
		}
	    }
	}
	g_array_set_size(specArray, specArray->len+1);
	spec=&g_array_index(specArray, MkdgButtonSpec, specArray->len-1);
	spec->responseId=MKDG_RESPONSE_NIL;
	spec->buttonText=NULL;
	mDialog->buttonSpecs=(MkdgButtonSpec *) g_array_free(specArray, FALSE);
	g_free(idBuf);
	g_strfreev(idStrs);
	g_free(buttonBuf);
	g_strfreev(buttonTexts);
	g_assert(mDialog->buttonSpecs[0].responseId==MKDG_RESPONSE_CLOSE);
    }
}

static void mkdg_new_from_key_file_section_config(Mkdg *mDialog, GKeyFile *keyFile, MkdgError **error){
    MkdgError *cfgErr=NULL;
    if (!g_key_file_has_group(keyFile, MKDG_SPEC_SECTION_CONFIG_STRING))
	return;
    MkdgConfig *config=mkdg_config_new(mDialog);
    if (g_key_file_has_key(keyFile, MKDG_SPEC_SECTION_CONFIG_STRING, "flags", &cfgErr)){
	gchar *flagsStr=g_key_file_get_string(keyFile, MKDG_SPEC_SECTION_CONFIG_STRING, "flags", &cfgErr);
	config->flags=mkdg_config_flags_parse(flagsStr);
	g_free(flagsStr);
    }

    gboolean loaded=FALSE;
    if (g_key_file_has_key(keyFile, MKDG_SPEC_SECTION_CONFIG_STRING, "configInterface", &cfgErr)){
	gchar *configInterfaceStr=g_key_file_get_string(keyFile, MKDG_SPEC_SECTION_CONFIG_STRING, "configInterface", &cfgErr);
	if (cfgErr==NULL){
	    gchar **configInterfaceStrList=g_strsplit(configInterfaceStr,";",-1);
	    gint i;
	    for (i=0; configInterfaceStrList[i]!=NULL;i++){
		MKDG_MODULE module=mkdg_module_parse(configInterfaceStrList[i]);
		switch(module){
		    case MKDG_MODULE_GCONF2:
		    case MKDG_MODULE_GTK2:
			if (mkdg_module_load(mDialog, module, &cfgErr)){
			    loaded=TRUE;
			}else{
			    mkdg_error_handle(cfgErr, error);
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
	    mkdg_error_handle(cfgErr, error);
	}
	g_free(configInterfaceStr);
    }else{
	mkdg_error_handle(cfgErr, error);
    }
}

//static gboolean mkdg_spec_parser_config_set_load_attr(GKeyFile *keyFile,

static void mkdg_new_from_key_file_section_config_set(Mkdg *mDialog, GKeyFile *keyFile, MkdgError **error){
    MkdgError *cfgErr=NULL;
    gsize groupLen;
    gchar **groupList=g_key_file_get_groups(keyFile, &groupLen);
    gsize i;
    for(i=0; groupList[i]!=NULL && i < groupLen ; i++){
	if (!g_str_has_prefix(groupList[i], MKDG_SPEC_SECTION_CONFIG_SET_STRING)){
	    continue;
	}
	MKDG_DEBUG_MSG(4, "[I4] new_from_key_file_section_config_set() keyfileGroupName=%s", groupList[i]);
	MkdgConfigSet *configSet=mkdg_config_set_new();

	gchar *valueStr=NULL;
	if ((valueStr=mkdg_g_keyfile_has_key_then_get_string(keyFile, groupList[i], "pageNames"))!=NULL){
//	    configSet->pageNames=valueStr;
	}else{
//	    configSet->pageNames=NULL;
	}
    }
}

Mkdg *mkdg_new_from_key_file(const gchar *filename, MkdgError **error){
    GKeyFile *keyFile=g_key_file_new();
    MkdgError *cfgErr=NULL;
    Mkdg *mDialog=NULL;
    if (!g_key_file_load_from_file(keyFile, filename, G_KEY_FILE_NONE, &cfgErr)){
	mkdg_error_handle(cfgErr, error);
	goto FINAL_LOAD_FROM_KEYFILE;
    }
    mDialog=mkdg_new();
    mkdg_new_from_key_file_section_main(mDialog, keyFile, &cfgErr);
    mkdg_error_handle(cfgErr,error);
    mkdg_new_from_key_file_section_keys(mDialog, keyFile, &cfgErr);
    mkdg_error_handle(cfgErr,error);
    mDialog->flags|= MKDG_FLAG_FREE_ALL;
FINAL_LOAD_FROM_KEYFILE:
    g_key_file_free(keyFile);

    return mDialog;
}

