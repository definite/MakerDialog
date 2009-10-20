/*=== Start toolkit handler definitions ===*/
static GValue *maker_dialog_component_get_value_gtk(MakerDialogUi *dlgUi, const gchar *key){
    return maker_dialog_gtk_get_widget_value(MAKER_DIALOG_GTK(dlgUi->dialogObj),key);
}

static void maker_dialog_component_set_value_gtk(MakerDialogUi *dlgUi, const gchar *key, GValue *value){
    maker_dialog_gtk_set_widget_value(MAKER_DIALOG_GTK(dlgUi->dialogObj),key,value);
}

static gpointer maker_dialog_construct_gtk(MakerDialogUi *dlgUi, gpointer parentWindow, gboolean modal){
    MakerDialogGtk *dlg_gtk=maker_dialog_gtk_new_full(dlgUi,parentWindow,modal);
    return (gpointer) dlg_gtk;
}

static gint maker_dialog_run_gtk(MakerDialogUi *dlgUi){
    return gtk_dialog_run(GTK_DIALOG (dlgUi->dialogObj));
}

static void maker_dialog_show_gtk(MakerDialogUi *dlgUi){
    gtk_widget_show_all(GTK_WIDGET (dlgUi->dialogObj));
}

static void maker_dialog_hide_gtk(MakerDialogUi *dlgUi){
    gtk_widget_hide(GTK_WIDGET (dlgUi->dialogObj));
}

static void maker_dialog_destroy_gtk(MakerDialogUi *dlgUi){
    maker_dialog_gtk_destroy(MAKER_DIALOG_GTK( dlgUi->dialogObj));
}

static MakerDialogToolkitHandler makerDialogToolkitHandler_gtk={
    maker_dialog_component_get_value_gtk,
    maker_dialog_component_set_value_gtk,
    maker_dialog_construct_gtk,
    maker_dialog_run_gtk,
    maker_dialog_show_gtk,
    maker_dialog_hide_gtk,
    maker_dialog_destroy_gtk
};

MakerDialogUi *maker_dialog_ui_use_gtk(MakerDialog *mDialog, gint *argc, gchar ***argv){
    if (gtk_init_check(argc, argv)){
	MakerDialogUi *dlgUi=maker_dialog_ui_init(mDialog, &makerDialogToolkitHandler_gtk);
	dlgUi->dialogObj=NULL;
	return dlgUi;
    }
    return NULL;
}

/*=== End toolkit handler definitions ===*/

/*=== Start propertyTable foreach functions ===*/
static void maker_dialog_construct_ui_GHFunc(gpointer key, gpointer value, gpointer user_data){
    gchar *propertyKey=(gchar *) key;
    MakerDialogPropertyContext *ctx=(MakerDialogPropertyContext *) value;
    MakerDialogGtk *dlg_gtk=MAKER_DIALOG_GTK(user_data);
    MAKER_DIALOG_DEBUG_MSG(3,"[I3] maker_dialog_construct_ui_GHFunc(%s,-,-)",propertyKey);

    if (ctx->spec->pageName){
	if (!dlg_gtk->dialog_notebook){
	    dlg_gtk->dialog_notebook=gtk_notebook_new();
	    gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dlg_gtk)->vbox), dlg_gtk->dialog_notebook, TRUE, TRUE, 0);
	}
	if (!g_hash_table_lookup(dlg_gtk->_priv->notebookTable, (gconstpointer) ctx->spec->pageName)){
	    GtkWidget *label=gtk_label_new(_(ctx->spec->pageName));
	    maker_dialog_gtk_widget_register(dlg_gtk, label, ctx->spec->pageName, "label");
//	    gtk_widget_show(label);

	    GtkWidget *vbox=gtk_vbox_new(dlg_gtk->vbox_homogeneous,dlg_gtk->vbox_spacing);
	    maker_dialog_gtk_widget_register(dlg_gtk, vbox, ctx->spec->pageName, "vbox");
//	    gtk_widget_show(vbox);
	    gtk_notebook_append_page (GTK_NOTEBOOK(dlg_gtk->dialog_notebook), vbox,label);
	    g_hash_table_insert(dlg_gtk->_priv->notebookTable,
		    (gpointer) ctx->spec->pageName, (gpointer) vbox);
	}
	g_hash_table_insert(dlg_gtk->_priv->notebookContentTable, (gchar *) ctx->spec->key,
		(gchar *) ctx->spec->pageName);
    }

    maker_dialog_gtk_add_property_ui(dlg_gtk, ctx);
}

static void maker_dialog_align_labels_GHFunc(gpointer key, gpointer value, gpointer user_data){
    gchar *pageName=(gchar *) key;
    MakerDialogGtk *dlg_gtk=MAKER_DIALOG_GTK(user_data);
    MAKER_DIALOG_DEBUG_MSG(3,"[I3] maker_dialog_align_labels_GHFunc(%s,-,-)",pageName);
    maker_dialog_gtk_align_labels(dlg_gtk, pageName, &(dlg_gtk->_priv->mDialog->labelAlignment));
}

#ifndef HAVE_G_ONCE_INIT_ENTER
static GMutex*   g_once_mutex=NULL;
static GCond    *g_once_cond = NULL;
static GSList*   g_once_init_list = NULL;

/**
 * Modified From glib=2.22.0
 */
gboolean
g_once_init_enter(volatile gsize *value_location)
{
    gboolean need_init = FALSE;
    if (!g_once_mutex)
	g_once_mutex=g_mutex_new();
    g_mutex_lock (g_once_mutex);
    if (!g_atomic_pointer_get (value_location))
    {
	if (!g_slist_find (g_once_init_list, (void*) value_location))
	{
	    need_init = TRUE;
	    g_once_init_list = g_slist_prepend (g_once_init_list, (void*) value_location);
	}
	else
	    do
		g_cond_wait (g_once_cond, g_once_mutex);
	    while (g_slist_find (g_once_init_list, (void*) value_location));
    }
    g_mutex_unlock (g_once_mutex);
    return need_init;
}

/**
 * From glib=2.22.0
 */
void
g_once_init_leave (volatile gsize *value_location,
	gsize           initialization_value)
{
    g_return_if_fail (!g_atomic_pointer_get (value_location));
    g_return_if_fail (initialization_value != 0);
    g_return_if_fail (g_once_init_list != NULL);

    g_atomic_pointer_set ((void**)value_location, (void*) initialization_value);
    g_mutex_lock (g_once_mutex);
    g_once_init_list = g_slist_remove (g_once_init_list, (void*) value_location);
    g_cond_broadcast (g_once_cond);
    g_mutex_unlock (g_once_mutex);
}
#endif /* HAVE_G_ONCE_INIT_ENTER */

#ifndef HAVE_G_DPGETTEXT2
/**
 * From glib=2.22.0
 */

static gboolean
_g_dgettext_should_translate (void)
{
    static gsize translate = 0;
    enum {
	SHOULD_TRANSLATE = 1,
	SHOULD_NOT_TRANSLATE = 2
    };

    if (G_UNLIKELY (g_once_init_enter (&translate)))
    {
	gboolean should_translate = TRUE;

	const char *default_domain     = textdomain (NULL);
	const char *translator_comment = gettext ("");
#ifndef G_OS_WIN32
	const char *translate_locale   = setlocale (LC_MESSAGES, NULL);
#else
	const char *translate_locale   = g_win32_getlocale ();
#endif
	/* We should NOT translate only if all the following hold:
	 *   - user has called textdomain() and set textdomain to non-default
	 *   - default domain has no translations
	 *   - locale does not start with "en_" and is not "C"
	 *
	 * Rationale:
	 *   - If text domain is still the default domain, maybe user calls
	 *     it later. Continue with old behavior of translating.
	 *   - If locale starts with "en_", we can continue using the
	 *     translations even if the app doesn't have translations for
	 *     this locale.  That is, en_UK and en_CA for example.
	 *   - If locale is "C", maybe user calls setlocale(LC_ALL,"") later.
	 *     Continue with old behavior of translating.
	 */
	if (0 != strcmp (default_domain, "messages") &&
		'\0' == *translator_comment &&
		0 != strncmp (translate_locale, "en_", 3) &&
		0 != strcmp (translate_locale, "C"))
	    should_translate = FALSE;

	g_once_init_leave (&translate,
		should_translate ?
		SHOULD_TRANSLATE :
		SHOULD_NOT_TRANSLATE);
    }

    return translate == SHOULD_TRANSLATE;
}

/**
 * From glib=2.22.0
 */
static G_CONST_RETURN gchar * g_dgettext (const gchar *domain,
	const gchar *msgid)
{
    if (domain && G_UNLIKELY (!_g_dgettext_should_translate ()))
	return msgid;

    return dgettext (domain, msgid);
}

/**
 * From glib=2.22.0
 */
static G_CONST_RETURN char *
g_dpgettext2 (const char *domain,
	const char *msgctxt,
	const char *msgid)
{
    size_t msgctxt_len = strlen (msgctxt) + 1;
    size_t msgid_len = strlen (msgid) + 1;
    const char *translation;
    char* msg_ctxt_id;

    msg_ctxt_id = g_alloca (msgctxt_len + msgid_len);

    memcpy (msg_ctxt_id, msgctxt, msgctxt_len - 1);
    msg_ctxt_id[msgctxt_len - 1] = '\004';
    memcpy (msg_ctxt_id + msgctxt_len, msgid, msgid_len);

    translation = g_dgettext (domain, msg_ctxt_id);

    if (translation == msg_ctxt_id)
    {
	/* try the old way of doing message contexts, too */
	msg_ctxt_id[msgctxt_len - 1] = '|';
	translation = g_dgettext (domain, msg_ctxt_id);

	if (translation == msg_ctxt_id)
	    return msgid;
    }

    return translation;
}
#endif  /* HAVE_G_DPGETTEXT2 */

/*=== Start listStore functions ===*/
static void listStore_append(GtkListStore *listStore,const gchar *str,
	const gchar *translationContext,
	MakerDialogPropertyFlags propertyFlags){
    GtkTreeIter iter;
    gtk_list_store_append (listStore, &iter);

    if (propertyFlags & MAKER_DIALOG_PROPERTY_FLAG_HAS_TRANSLATION){
	if (translationContext || propertyFlags & MAKER_DIALOG_PROPERTY_FLAG_TRANSLATION_WITH_CONTEXT){
	    MAKER_DIALOG_DEBUG_MSG(5,"[I5] str=%s, _(str)=%s",str,g_dpgettext2(NULL,translationContext,str));
	    gtk_list_store_set (listStore, &iter,
		    0, str,
		    1, g_dpgettext2(NULL,translationContext,str),
		    -1);
	}else{
	    MAKER_DIALOG_DEBUG_MSG(5,"[I5] str=%s, _(str)=%s",str,_(str));
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
		g_value_unset(&gValue);
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
    MakerDialogPropertyContext *ctx=(MakerDialogPropertyContext *)user_data;
    GValue value={0};
    combo_get_active_text(comboBox, &value);
    MAKER_DIALOG_DEBUG_MSG(2,"on_comboBox_changed_wrap(), key=%s value=%s",ctx->spec->key,g_value_get_string(&value));
    ctx->applyFunc(ctx,&value);
}

static void on_entry_activate_wrap (GtkEntry *entry, gpointer    user_data)
{
    MakerDialogPropertyContext *ctx=(MakerDialogPropertyContext *)user_data;
    GValue value={0};
    g_value_init(&value, ctx->spec->valueType);
    g_value_set_string(&value,gtk_entry_get_text(entry));
    MAKER_DIALOG_DEBUG_MSG(2,"on_entry_activate_wrap(), key=%s value=%s",ctx->spec->key,g_value_get_string(&value));
    ctx->applyFunc(ctx,&value);
}

static void on_spinButton_value_changed_wrap (GtkSpinButton *button, gpointer    user_data)
{
    MakerDialogPropertyContext *ctx=(MakerDialogPropertyContext *)user_data;
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
    ctx->applyFunc(ctx,&value);
}

static void on_toggleButton_toggled_wrap (GtkToggleButton *button, gpointer    user_data)
{
    MakerDialogPropertyContext *ctx=(MakerDialogPropertyContext *)user_data;
    GValue value={0};
    g_value_init(&value, ctx->spec->valueType);
    g_value_set_boolean(&value, gtk_toggle_button_get_active(button));
    MAKER_DIALOG_DEBUG_MSG(2,"on_entry_activate_wrap(), key=%s value=%s",ctx->spec->key,g_value_get_string(&value));
    ctx->applyFunc(ctx,&value);
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

//static void append_indent_space(GString *strBuf, gint indentLevel){
//    int i,indentLen=indentLevel*INDENT_SPACES;
//    for(i=0;i<indentLen;i++){
//        g_string_append_c(strBuf,' ');
//    }
//}

//static GString *xml_tags_to_string(const gchar *tagName, XmlTagsType type,
//        const gchar *attribute, const gchar *value,gint indentLevel){
//    GString *strBuf=g_string_new(NULL);
//    append_indent_space(strBuf,indentLevel);

//    if(type!=XML_TAG_TYPE_NO_TAG){
//        g_string_append_printf(strBuf,"<%s%s%s%s%s>",
//                (type==XML_TAG_TYPE_END_ONLY) ? "/": "",
//                (!isEmptyString(tagName))? tagName : "",
//                (!isEmptyString(attribute)) ? " ":"",  (!isEmptyString(attribute))? attribute : "",
//                (type==XML_TAG_TYPE_EMPTY) ? "/": ""
//        );
//    }
//    if (type==XML_TAG_TYPE_EMPTY)
//        return strBuf;
//    if (type==XML_TAG_TYPE_BEGIN_ONLY)
//        return strBuf;
//    if (type==XML_TAG_TYPE_END_ONLY)
//        return strBuf;

//    if (type==XML_TAG_TYPE_LONG){
//        g_string_append_c(strBuf,'\n');
//    }

//    if (value){
//        if (type==XML_TAG_TYPE_LONG || type==XML_TAG_TYPE_NO_TAG){
//            append_indent_space(strBuf,indentLevel+1);
//            int i, valueLen=strlen(value);
//            for(i=0;i<valueLen;i++){
//                g_string_append_c(strBuf,value[i]);
//                if (value[i]=='\n'){
//                    append_indent_space(strBuf,indentLevel+1);
//                }
//            }
//            g_string_append_c(strBuf,'\n');
//            if (type==XML_TAG_TYPE_LONG){
//                append_indent_space(strBuf,indentLevel);
//            }
//        }else{
//            g_string_append(strBuf,value);
//        }
//    }

//    if (type==XML_TAG_TYPE_LONG || type==XML_TAG_TYPE_SHORT){
//        g_string_append_printf(strBuf,"</%s>",tagName);
//    }
//    return strBuf;
//}

//static void xml_tags_write(FILE *outF, const gchar *tagName, XmlTagsType type,
//        const gchar *attribute, const gchar *value){
//    static int indentLevel=0;
//    if (type==XML_TAG_TYPE_END_ONLY)
//        indentLevel--;

//    GString *strBuf=xml_tags_to_string(tagName, type, attribute, value, indentLevel);
//    MAKER_DIALOG_DEBUG_MSG(3,",xml_tags_write:%s",strBuf->str);
//    fprintf(outF,"%s\n",strBuf->str);

//    if (type==XML_TAG_TYPE_BEGIN_ONLY)
//        indentLevel++;
//    g_string_free(strBuf,TRUE);
//}

typedef struct{
    const gchar *schemasHome;
    const gchar *owner;
    const gchar *locales;
    FILE *outF;
} SchemasFileData;

//static void ctx_write_locale(MakerDialogPropertyContext *ctx, SchemasFileData *sData, const gchar *localeStr){
//    gchar buf[50];
//    g_snprintf(buf,50,"name=\"%s\"",localeStr);
//    setlocale(LC_MESSAGES,localeStr);
//    xml_tags_write(sData->outF,"locale",XML_TAG_TYPE_BEGIN_ONLY,buf,NULL);
//    xml_tags_write(sData->outF,"short",XML_TAG_TYPE_SHORT,NULL, gettext(ctx->spec->label));
//    xml_tags_write(sData->outF,"long",XML_TAG_TYPE_LONG,NULL, gettext(ctx->spec->tooltip));
//    xml_tags_write(sData->outF,"locale",XML_TAG_TYPE_END_ONLY,NULL,NULL);
//}

//static void ctx_write_callback(gpointer data, gpointer user_data){
//    MakerDialogPropertyContext *ctx=(MakerDialogPropertyContext *) data;
//    SchemasFileData *sData=(SchemasFileData *) user_data;
//    xml_tags_write(sData->outF,"schema",XML_TAG_TYPE_BEGIN_ONLY,NULL,NULL);
//    gchar buf[STRING_BUFFER_SIZE_DEFAULT];
//    g_snprintf(buf,STRING_BUFFER_SIZE_DEFAULT,"/schemas%s/%s",sData->schemasHome,ctx->spec->key);
//    xml_tags_write(sData->outF,"key",XML_TAG_TYPE_SHORT,NULL,buf);
//    xml_tags_write(sData->outF,"applyto",XML_TAG_TYPE_SHORT,NULL,buf+strlen("/schemas"));
//    xml_tags_write(sData->outF,"owner",XML_TAG_TYPE_SHORT,NULL,sData->owner);
//    switch(ctx->spec->valueType){
//        case G_TYPE_BOOLEAN:
//            xml_tags_write(sData->outF,"type",XML_TAG_TYPE_SHORT,NULL,"bool");
//            break;
//        case G_TYPE_INT:
//        case G_TYPE_UINT:
//            xml_tags_write(sData->outF,"type",XML_TAG_TYPE_SHORT,NULL,"int");
//            break;
//        case G_TYPE_STRING:
//            xml_tags_write(sData->outF,"type",XML_TAG_TYPE_SHORT,NULL,"string");
//            break;
//        default:
//            break;
//    }
//    if (ctx->spec->defaultValue){
//        xml_tags_write(sData->outF,"default",XML_TAG_TYPE_SHORT,NULL,ctx->spec->defaultValue);
//    }
//    gchar **localeArray=g_strsplit_set(sData->locales,":;",-1);
//    int i;
//    for(i=0;localeArray[i]!=NULL;i++){
//        ctx_write_locale(ctx,sData,localeArray[i]);
//    }
//    setlocale(LC_ALL,NULL);
//    xml_tags_write(sData->outF,"schema",XML_TAG_TYPE_END_ONLY,NULL,NULL);
//}

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

    GtkWidget *widget=maker_dialog_gtk_get_widget(wAlignment->self,key,"label");
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
    GtkWidget *widget=maker_dialog_gtk_get_widget(wAlignment->self,key,"label");
    gtk_widget_set_size_request(widget, wAlignment->currentMaxWidth,-1);
    gtk_misc_set_alignment (GTK_MISC(widget),wAlignment->xalign,wAlignment->yalign);
}

