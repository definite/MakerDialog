/*=== Start toolkit handler definitions ===*/
gpointer maker_dialog_construct_gtk(MakerDialog *dlg){
    MakerDialogGtk *dlg_gtk=maker_dialog_gtk_new_full(dlg);
    return (gpointer) dlg_gtk;
}

gint maker_dialog_run_gtk(MakerDialog *dlg){
    return gtk_dialog_run(GTK_DIALOG (dlg->handler->dialog_obj));
}

void maker_dialog_show_gtk(MakerDialog *dlg){
    gtk_dialog_show_all(GTK_DIALOG (dlg->handler->dialog_obj));
}

void maker_dialog_hide_gtk(MakerDialog *dlg){
    gtk_dialog_hide(GTK_DIALOG (dlg->handler->dialog_obj));
}

void maker_dialog_destroy_gtk(MakerDialog *dlg){
    gtk_widget_destroy(GTK_WIDGET (dlg->handler->dialog_obj));
}

void maker_dialog_component_set_value_gtk(MakerDialog *dlg, const gchar *key, GValue *value){
    maker_dialog_gtk_set_widget_value(MAKER_DIALOG_GTK(dlg->handler->dialog_obj),key,value);
}

const MakerDialogToolkitHandler makerDialogHandlerGtk={
    NULL,
    maker_dialog_construct_gtk,
    maker_dialog_run_gtk,
    maker_dialog_show_gtk,
    maker_dialog_hide_gtk,
    maker_dialog_destroy_gtk,
    maker_dialog_component_set_value_gtk
};

/*=== Start propertyTable foreach functions ===*/
void maker_dialog_construct_ui_GHFunc(gpointer key, gpointer value, gpointer user_data){
    gchar *propertyKey=(gchar *) key;
    MakerDialogPropertyContext *ctx=(MakerDialogPropertyContext *) value;
    MakerDialogGtk *dlg_gtk=MAKER_DIALOG_GTK(user_data);
    MAKER_DIALOG_DEBUG_MSG(2,"maker_dialog_construct_ui_GHFunc(%s,-,-)",ctx->spec->key);

    if (ctx->spec->pageName){
	if (!g_hash_table_lookup(dlg_gtk->_priv->notebookTable, (gconstpointer) ctx->spec->pageName)){
	    g_hash_table_insert(dlg_gtk->_priv->notebookTable, ctx->spec->pageName, ctx->spec->pageName);
	}
	g_hash_table_insert(dlg_gtk->_priv->notebookContentTable, ctx->spec->key, ctx->spec->pageName);
    }

    maker_dialog_gtk_add_
    dlg_gtk=0

    fucn
}

/*=== Start listStore functions ===*/
static void listStore_append(GtkListStore *listStore,const gchar *str,
	const gchar *translationContext,
	MakerDialogPropertyFlags propertyFlags){
    GtkTreeIter iter;
    gtk_list_store_append (listStore, &iter);

    if (propertyFlags & MAKER_DIALOG_PROPERTY_FLAG_HAS_TRANSLATION){
	if (translationContext || propertyFlags & MAKER_DIALOG_PROPERTY_FLAG_TRANSLATION_WITH_CONTEXT){
	    MAKER_DIALOG_DEBUG_MSG(5,"*** str=%s, _(str)=%s",str,g_dpgettext2(NULL,translationContext,str));
	    gtk_list_store_set (listStore, &iter,
		    0, str,
		    1, g_dpgettext2(NULL,translationContext,str),
		    -1);
	}else{
	    MAKER_DIALOG_DEBUG_MSG(5,"*** str=%s, _(str)=%s",str,_(str));
	    gtk_list_store_set (listStore, &iter,
		    0, str,
		    1, _(str),
		    -1);
	}
    }else{
	MAKER_DIALOG_DEBUG_MSG(5,"*** str=%s",str);
	gtk_list_store_set (listStore, &iter,
		0, str,
		-1);
    }

}

static gint listStore_find_string(GtkListStore *listStore,const gchar *str,
	const gchar *translationContext,
	MakerDialogPropertyFlags propertyFlags){
    g_assert(str);
    MAKER_DIALOG_DEBUG_MSG(4,"*** listStore_find_string(%s,%u)",str,propertyFlags);
    int i=0,index=-1;
    GtkTreeIter iter;
    GValue gValue={0};
    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(listStore), &iter)){
	do{
	    gtk_tree_model_get_value (GTK_TREE_MODEL(listStore), &iter,0,&gValue);
	    if (strcmp(str,g_value_get_string(&gValue))==0){
		index= i;
		break;
	    }
	    i++;
	    g_value_unset(&gValue);
	}while(gtk_tree_model_iter_next (GTK_TREE_MODEL(listStore), &iter));
    }
    if (index<0 && !(propertyFlags & MAKER_DIALOG_PROPERTY_FLAG_INEDITABLE)){
	/* Add new item*/
	listStore_append(listStore, str,translationContext,propertyFlags);
	index=i;
    }
    MAKER_DIALOG_DEBUG_MSG(4,",listStore_find_string(%s,%u) index=%d",str,propertyFlags,index);
    return index;
}

static gint combo_find_string_index(GtkComboBox *combo,const gchar *str,
	const gchar *translationContext,
	MakerDialogPropertyFlags propertyFlags){
    GtkListStore *listStore=GTK_LIST_STORE(gtk_combo_box_get_model(combo));
    return listStore_find_string(listStore,str, translationContext,propertyFlags);
}

static const gchar *combo_get_active_text(GtkComboBox *combo,GValue *gValue){
    GtkTreeIter iter;
    if (!gtk_combo_box_get_active_iter(combo,&iter)){
	return NULL;
    }
    GtkListStore *listStore=GTK_LIST_STORE(gtk_combo_box_get_model(combo));
    gtk_tree_model_get_value (GTK_TREE_MODEL(listStore), &iter,0,gValue);
    return g_value_get_string(gValue);
}


static void  propList_free_deep_callback(gpointer data, gpointer user_data){
    propertyContext_free(data);
}

static gchar *widget_get_id(gchar *buffer, gint buffer_size,
	const gchar *widget_label, const gchar *widget_type){
    g_snprintf(buffer,buffer_size,"%s%s%s%s",
	    WIDGET_ID_PREFIX, widget_label,
	    (widget_type)? "_" : "",
	    (widget_type)? widget_type : ""
	    );
    return buffer;
}

/*=== Start Widget Callback function wraps ===*/

static void on_comboBox_changed_wrap (GtkComboBox *comboBox, gpointer    user_data)
{
    MakerDialogPropertyContext *ctx=(PropertyContext *)user_data;
    GValue value={0};
    combo_get_active_text(comboBox, &value);
    MAKER_DIALOG_DEBUG_MSG(2,"on_comboBox_changed_wrap(), key=%s value=%s",ctx->spec->key,g_value_get_string(&value));
    ctx->spec->setFunc(ctx,&value);
}

static void on_entry_activate_wrap (GtkEntry *entry, gpointer    user_data)
{
    MakerDialogPropertyContext *ctx=(PropertyContext *)user_data;
    GValue value={0};
    g_value_init(&value, ctx->spec->valueType);
    g_value_set_string(&value,gtk_entry_get_text(entry));
    MAKER_DIALOG_DEBUG_MSG(2,"on_entry_activate_wrap(), key=%s value=%s",ctx->spec->key,g_value_get_string(&value));
    ctx->spec->setFunc(ctx,&value);
}

static void on_spinButton_value_changed_wrap (GtkSpinButton *button, gpointer    user_data)
{
    MakerDialogPropertyContext *ctx=(PropertyContext *)user_data;
    GValue value={0};
    g_value_init(&value, ctx->spec->valueType);
    switch(ctx->spec->valueType){
	case G_TYPE_INT:
	    g_value_set_int(&value,(gint) gtk_spin_button_get_value(button));
	    MAKER_DIALOG_DEBUG_MSG(2,"on_spinButton_value_changed_wrap(), key=%s value=%d",
		    ctx->spec->key,g_value_get_int(&value));
	    break;
	case G_TYPE_UINT:
	    g_value_set_uint(&value,(guint) gtk_spin_button_get_value(button));
	    MAKER_DIALOG_DEBUG_MSG(2,"on_spinButton_value_changed_wrap(), key=%s value=%u",
		    ctx->spec->key,g_value_get_uint(&value));
	    break;
	case G_TYPE_DOUBLE:
	    g_value_set_uint(&value, gtk_spin_button_get_value(button));
	    MAKER_DIALOG_DEBUG_MSG(2,"on_spinButton_value_changed_wrap(), key=%s value=%g",
		    ctx->spec->key,g_value_get_double(&value));
	    break;
	default:
	    break;
    }
    ctx->spec->setFunc(ctx,&value);
}

static void on_toggleButton_toggled_wrap (GtkToggleButton *button, gpointer    user_data)
{
    MakerDialogPropertyContext *ctx=(PropertyContext *)user_data;
    GValue value={0};
    g_value_init(&value, ctx->spec->valueType);
    g_value_set_boolean(&value, gtk_toggle_button_get_active(button));
    MAKER_DIALOG_DEBUG_MSG(2,"on_entry_activate_wrap(), key=%s value=%s",ctx->spec->key,g_value_get_string(&value));
    ctx->spec->setFunc(ctx,&value);
}

/*=== End of Widget Callback function wraps ===*/
    static gboolean isEmptyString(const gchar *str){
	if (!str)
	    return TRUE;
	if (str[0]=='\0')
	    return TRUE;
	return FALSE;
    }

typedef enum{
    XML_TAG_TYPE_NO_TAG,
    XML_TAG_TYPE_EMPTY,
    XML_TAG_TYPE_SHORT,
    XML_TAG_TYPE_LONG,
    XML_TAG_TYPE_BEGIN_ONLY,
    XML_TAG_TYPE_END_ONLY,
} XmlTagsType;
#define INDENT_SPACES 4

static void append_indent_space(GString *strBuf, gint indentLevel){
    int i,indentLen=indentLevel*INDENT_SPACES;
    for(i=0;i<indentLen;i++){
	g_string_append_c(strBuf,' ');
    }
}

static GString *xml_tags_to_string(const gchar *tagName, XmlTagsType type,
	const gchar *attribute, const gchar *value,gint indentLevel){
    GString *strBuf=g_string_new(NULL);
    append_indent_space(strBuf,indentLevel);

    if(type!=XML_TAG_TYPE_NO_TAG){
	g_string_append_printf(strBuf,"<%s%s%s%s%s>",
		(type==XML_TAG_TYPE_END_ONLY) ? "/": "",
		(!isEmptyString(tagName))? tagName : "",
		(!isEmptyString(attribute)) ? " ":"",  (!isEmptyString(attribute))? attribute : "",
		(type==XML_TAG_TYPE_EMPTY) ? "/": ""
	);
    }
    if (type==XML_TAG_TYPE_EMPTY)
	return strBuf;
    if (type==XML_TAG_TYPE_BEGIN_ONLY)
	return strBuf;
    if (type==XML_TAG_TYPE_END_ONLY)
	return strBuf;

    if (type==XML_TAG_TYPE_LONG){
	g_string_append_c(strBuf,'\n');
    }

    if (value){
	if (type==XML_TAG_TYPE_LONG || type==XML_TAG_TYPE_NO_TAG){
	    append_indent_space(strBuf,indentLevel+1);
	    int i, valueLen=strlen(value);
	    for(i=0;i<valueLen;i++){
		g_string_append_c(strBuf,value[i]);
		if (value[i]=='\n'){
		    append_indent_space(strBuf,indentLevel+1);
		}
	    }
	    g_string_append_c(strBuf,'\n');
	    if (type==XML_TAG_TYPE_LONG){
		append_indent_space(strBuf,indentLevel);
	    }
	}else{
	    g_string_append(strBuf,value);
	}
    }

    if (type==XML_TAG_TYPE_LONG || type==XML_TAG_TYPE_SHORT){
	g_string_append_printf(strBuf,"</%s>",tagName);
    }
    return strBuf;
}

static void xml_tags_write(FILE *outF, const gchar *tagName, XmlTagsType type,
	const gchar *attribute, const gchar *value){
    static int indentLevel=0;
    if (type==XML_TAG_TYPE_END_ONLY)
	indentLevel--;

    GString *strBuf=xml_tags_to_string(tagName, type, attribute, value, indentLevel);
    MAKER_DIALOG_DEBUG_MSG(3,",xml_tags_write:%s",strBuf->str);
    fprintf(outF,"%s\n",strBuf->str);

    if (type==XML_TAG_TYPE_BEGIN_ONLY)
	indentLevel++;
    g_string_free(strBuf,TRUE);
}

typedef struct{
    const gchar *schemasHome;
    const gchar *owner;
    const gchar *locales;
    FILE *outF;
} SchemasFileData;

static void ctx_write_locale(MakerDialogPropertyContext *ctx, SchemasFileData *sData, const gchar *localeStr){
    gchar buf[50];
    g_snprintf(buf,50,"name=\"%s\"",localeStr);
    setlocale(LC_MESSAGES,localeStr);
    xml_tags_write(sData->outF,"locale",XML_TAG_TYPE_BEGIN_ONLY,buf,NULL);
    xml_tags_write(sData->outF,"short",XML_TAG_TYPE_SHORT,NULL, gettext(ctx->spec->label));
    xml_tags_write(sData->outF,"long",XML_TAG_TYPE_LONG,NULL, gettext(ctx->spec->tooltip));
    xml_tags_write(sData->outF,"locale",XML_TAG_TYPE_END_ONLY,NULL,NULL);
}

static void ctx_write_callback(gpointer data, gpointer user_data){
    MakerDialogPropertyContext *ctx=(PropertyContext *) data;
    SchemasFileData *sData=(SchemasFileData *) user_data;
    xml_tags_write(sData->outF,"schema",XML_TAG_TYPE_BEGIN_ONLY,NULL,NULL);
    gchar buf[STRING_BUFFER_SIZE_DEFAULT];
    g_snprintf(buf,STRING_BUFFER_SIZE_DEFAULT,"/schemas%s/%s",sData->schemasHome,ctx->spec->key);
    xml_tags_write(sData->outF,"key",XML_TAG_TYPE_SHORT,NULL,buf);
    xml_tags_write(sData->outF,"applyto",XML_TAG_TYPE_SHORT,NULL,buf+strlen("/schemas"));
    xml_tags_write(sData->outF,"owner",XML_TAG_TYPE_SHORT,NULL,sData->owner);
    switch(ctx->spec->valueType){
	case G_TYPE_BOOLEAN:
	    xml_tags_write(sData->outF,"type",XML_TAG_TYPE_SHORT,NULL,"bool");
	    break;
	case G_TYPE_INT:
	case G_TYPE_UINT:
	    xml_tags_write(sData->outF,"type",XML_TAG_TYPE_SHORT,NULL,"int");
	    break;
	case G_TYPE_STRING:
	    xml_tags_write(sData->outF,"type",XML_TAG_TYPE_SHORT,NULL,"string");
	    break;
	default:
	    break;
    }
    if (ctx->spec->defaultValue){
	xml_tags_write(sData->outF,"default",XML_TAG_TYPE_SHORT,NULL,ctx->spec->defaultValue);
    }
    gchar **localeArray=g_strsplit_set(sData->locales,":;",-1);
    int i;
    for(i=0;localeArray[i]!=NULL;i++){
	ctx_write_locale(ctx,sData,localeArray[i]);
    }
    setlocale(LC_ALL,NULL);
    xml_tags_write(sData->outF,"schema",XML_TAG_TYPE_END_ONLY,NULL,NULL);
}

typedef struct{
    MakerDialogGtk *self;
    gint currentMaxWidth;
    const gchar *pageName;
    gfloat xalign;
    gfloat yalign;
} WidgetAlignment;

static void caculate_max_label_width_callback(gpointer key, gpointer value, gpointer user_data){
    WidgetAlignment *wAlignment=(WidgetAlignment *) user_data;
    if (!isEmptyString(wAlignment->pageName)){
	if (isEmptyString(value) || strcmp(wAlignment->pageName,value)!=0)
	    return;
    }
    gchar *keyStr=(gchar *) keyStr;

    GtkWidget *widget=maker_dialog_get_widget(wAlignment->self,key,"label");
    GtkRequisition requisition;
    gtk_widget_size_request (widget,&requisition);
    wAlignment->currentMaxWidth=MAX(wAlignment->currentMaxWidth, requisition.width);
}

static void set_label_width_callback(gpointer key, gpointer value, gpointer user_data){
    WidgetAlignment *wAlignment=(WidgetAlignment *) user_data;
    if (!isEmptyString(wAlignment->pageName)){
	if (isEmptyString(value) || strcmp(wAlignment->pageName,value)!=0)
	    return;
    }
    gchar *keyStr=(gchar *) keyStr;
    GtkWidget *widget=maker_dialog_get_widget(wAlignment->self,key,"label");
    gtk_widget_set_size_request(widget, wAlignment->currentMaxWidth,-1);
    gtk_misc_set_alignment (GTK_MISC(widget),wAlignment->xalign,wAlignment->yalign);
}

