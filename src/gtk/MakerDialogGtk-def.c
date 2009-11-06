/*=== Start toolkit interface definitions ===*/
static GValue *maker_dialog_widget_get_value_gtk(MakerDialogUi *ui, const gchar *key){
    return maker_dialog_gtk_get_widget_value(MAKER_DIALOG_GTK(ui->dlgObj),key);
}

static void maker_dialog_widget_set_value_gtk(MakerDialogUi *ui, const gchar *key, GValue *value){
    maker_dialog_gtk_set_widget_value(MAKER_DIALOG_GTK(ui->dlgObj),key,value);
}

static gpointer maker_dialog_construct_gtk(MakerDialogUi *ui, gpointer parentWindow, gboolean modal){
    MakerDialogGtk *dlg_gtk=maker_dialog_gtk_new_full(ui,parentWindow,modal);
    return (gpointer) dlg_gtk;
}

static gint maker_dialog_run_gtk(MakerDialogUi *ui){
    return gtk_dialog_run(GTK_DIALOG (ui->dlgObj));
}

static void maker_dialog_show_gtk(MakerDialogUi *ui){
    gtk_widget_show_all(GTK_WIDGET (ui->dlgObj));
}

static void maker_dialog_hide_gtk(MakerDialogUi *ui){
    gtk_widget_hide(GTK_WIDGET (ui->dlgObj));
}

static void maker_dialog_destroy_gtk(MakerDialogUi *ui){
    maker_dialog_gtk_destroy(MAKER_DIALOG_GTK( ui->dlgObj));
}

static MakerDialogToolkitInterface makerDialogToolkitInterface_gtk={
    maker_dialog_widget_get_value_gtk,
    maker_dialog_widget_set_value_gtk,
    maker_dialog_construct_gtk,
    maker_dialog_run_gtk,
    maker_dialog_show_gtk,
    maker_dialog_hide_gtk,
    maker_dialog_destroy_gtk
};

MakerDialogUi *maker_dialog_ui_use_gtk(MakerDialog *mDialog, gint *argc, gchar ***argv){
    if (gtk_init_check(argc, argv)){
	MakerDialogUi *ui=maker_dialog_ui_init(mDialog, &makerDialogToolkitInterface_gtk);
	ui->dlgObj=NULL;
	return ui;
    }
    return NULL;
}

/*=== End toolkit interface definitions ===*/

/*=== Start foreach functions ===*/
static void maker_dialog_gtk_construct_ui_PropFunc(MakerDialog *mDialog, MakerDialogPropertyContext *ctx, gpointer userData){
    MakerDialogGtk *dlg_gtk=MAKER_DIALOG_GTK(userData);
    maker_dialog_gtk_add_property_ui(dlg_gtk, ctx);
}

/*=== Start old version compatible functions ===*/
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
/*=== End old version compatible functions ===*/

/*=== Start misc utility functions ===*/
static const gchar *maker_dialog_gtk_get_translation_string(const gchar *str, MakerDialogPropertySpec *spec){
    if (spec-> flags & MAKER_DIALOG_PROPERTY_FLAG_HAS_TRANSLATION){
	if (spec-> translationContext)
	    return g_dpgettext2(NULL,spec->translationContext,str);
	else
	    return _(str);
    }
    return str;
}

static void g_string_chunk_free_wrap(gpointer ptr){
    g_string_chunk_free((GStringChunk *) ptr);
}

static MkdgColor GValue_get_GdkColor(GValue* value, GdkColor *color){
    MkdgColor colorValue=g_value_get_uint(value);
    color->red=((colorValue & 0xFF0000) >> 16)  * 0x101;
    color->green=((colorValue & 0x00FF00) >> 8) * 0x101;
    color->blue=(colorValue & 0x0000FF) * 0x101;
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] GValue_get_GdkColor() red=%X green=%X blue=%X colorValue=%X\n",
	    color->red,color->green,color->blue,colorValue);
    return colorValue;
}

static MkdgColor GValue_set_GdkColor(GValue* value, GdkColor *color){
    MkdgColor colorValue=(color->red/256<<16) + (color->green/256<<8) + color->blue/256;
    g_value_set_uint(value, colorValue);
    MAKER_DIALOG_DEBUG_MSG(4,"[I4] GValue_set_GdkColor() red=%X green=%X blue=%X colorValue=%X\n",
	    color->red,color->green,color->blue,colorValue);
    return colorValue;
}

/*=== End misc utility functions ===*/

/*=== Start listStore functions ===*/
static void listStore_append(GtkListStore *listStore, const gchar *listKey, const gchar *prompt){
    GtkTreeIter iter;

    MAKER_DIALOG_DEBUG_MSG(5,"[I5] Gtk:listStore_append(-, %s, %s): ",listKey, prompt);
    gtk_list_store_append (listStore, &iter);
    gtk_list_store_set( listStore, &iter,
	    0, listKey,
	    1, prompt,
	    -1);
}

static void listStore_prepend(GtkListStore *listStore, const gchar *listKey, const gchar *prompt){
    GtkTreeIter iter;

    MAKER_DIALOG_DEBUG_MSG(5,"[I5] Gtk:listStore_prepend(-, %s, %s): ",listKey, prompt);
    gtk_list_store_prepend (listStore, &iter);
    gtk_list_store_set( listStore, &iter,
	    0, listKey,
	    1, prompt,
	    -1);
}

static gint listStore_find_value(GtkListStore *listStore, GValue *value, MakerDialogPropertySpec *spec){
    int i=0,index=-1;
    GtkTreeIter iter;
    gchar *valueStr=maker_dialog_g_value_to_string(value, NULL);
    MAKER_DIALOG_DEBUG_MSG(5,"[I5] Gtk:listStore_find_value(-,%s,%s)", valueStr, spec->key);
    GValue val={0};
    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(listStore), &iter)){
	do{
	    gtk_tree_model_get_value (GTK_TREE_MODEL(listStore), &iter,0,&val);
	    if (strcmp(g_value_get_string(&val),valueStr)==0){
		index= i;
		break;
	    }
	    i++;
	    g_value_reset(&val);
	}while(gtk_tree_model_iter_next (GTK_TREE_MODEL(listStore), &iter));
	g_value_unset(&val);
    }
    if (index<0 && !(spec->flags & MAKER_DIALOG_PROPERTY_FLAG_FIXED_SET)){
	/* Add new item*/
	listStore_append(listStore, valueStr, valueStr);
	index=i;
    }
    MAKER_DIALOG_DEBUG_MSG(5,"[I5] Gtk:listStore_find_value(-,%s,%s) index=%d", valueStr, spec->key, index);
    g_free(valueStr);
    return index;
}

/*=== End listStore functions ===*/

static gint combo_find_value_index(GtkComboBox *combo, GValue *value, MakerDialogPropertySpec *spec){
    GtkListStore *listStore=GTK_LIST_STORE(gtk_combo_box_get_model(combo));
    return listStore_find_value(listStore, value, spec);
}

static GValue *combo_get_active_value(GtkComboBox *combo,GValue *value){
    GtkTreeIter iter;
    if (!gtk_combo_box_get_active_iter(combo,&iter)){
	if (GTK_IS_COMBO_BOX_ENTRY(combo)){
	    /* Edit from entry */
	    GtkWidget *entry=gtk_bin_get_child(GTK_BIN(combo));
	    g_value_init(value,G_TYPE_STRING);
	    g_value_set_string(value, gtk_entry_get_text (GTK_ENTRY(entry)));
	    return value;
	}
	return NULL;
    }
    GtkListStore *listStore=GTK_LIST_STORE(gtk_combo_box_get_model(combo));
    gtk_tree_model_get_value (GTK_TREE_MODEL(listStore), &iter,0,value);
    return value;
}

static gchar *widget_gen_id(const gchar *widget_label, const gchar *widget_type, gint serial){
    GString *strBuf=g_string_new(WIDGET_ID_PREFIX WIDGET_ID_SEPARATOR);
    g_string_append_printf(strBuf, "%s%s%s",
	    widget_label,  (widget_type)? WIDGET_ID_SEPARATOR : "",
	    (widget_type)? widget_type : "");
    if (serial>=0){
	g_string_append_printf( strBuf, WIDGET_ID_SEPARATOR "%d", serial);
    }
    return g_string_free(strBuf, FALSE);
}

/*=== Start Widget Callback function wraps ===*/

static gboolean validate_and_apply(MakerDialogPropertyContext *ctx){
    gboolean ret=TRUE;
    if (maker_dialog_ui_update(ctx->mDialog, ctx)){
	maker_dialog_apply_value(ctx->mDialog, ctx->spec->key);
    }else{
	gchar *prevString=maker_dialog_property_to_string(ctx);
	GValue *value=maker_dialog_widget_get_value_gtk(ctx->mDialog->ui, ctx->spec->key);
	gchar *newString=maker_dialog_g_value_to_string(value, NULL);
	g_warning(_("Invalid value: %s, Fall back to previous value: %s"), newString, prevString);
	g_free(prevString);
	g_free(newString);
	maker_dialog_g_value_free(value);
	maker_dialog_widget_set_value_gtk(ctx->mDialog->ui, ctx->spec->key, &ctx->value);
	ret=FALSE;
    }
    return ret;
}

static void widget_event_wrap(GtkWidget *widget, gpointer user_data){
    MakerDialogPropertyContext *ctx=(MakerDialogPropertyContext *)user_data;
    validate_and_apply(ctx);
}

static void on_combo_box_entry_change_wrap(GtkComboBox *comboBox, gpointer  user_data){
    if (gtk_combo_box_get_active (comboBox)<0){
	/* Only Typing. Wait for active signal */
    }else{
	MakerDialogPropertyContext *ctx=(MakerDialogPropertyContext *)user_data;
	validate_and_apply(ctx);
    }
}

static void on_radioButton_toggled_wrap(GtkRadioButton *button, gpointer user_data){
    MakerDialogPropertyContext *ctx=(MakerDialogPropertyContext *)user_data;
    /* Ignore the toggle-off event */
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
	validate_and_apply(ctx);
}

/*=== End Widget Callback function wraps ===*/

typedef enum{
    XML_TAMKDG_TYPE_NO_TAG,
    XML_TAMKDG_TYPE_EMPTY,
    XML_TAMKDG_TYPE_SHORT,
    XML_TAMKDG_TYPE_LONG,
    XML_TAMKDG_TYPE_BEGIN_ONLY,
    XML_TAMKDG_TYPE_END_ONLY,
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

//    if(type!=XML_TAMKDG_TYPE_NO_TAG){
//        g_string_append_printf(strBuf,"<%s%s%s%s%s>",
//                (type==XML_TAMKDG_TYPE_END_ONLY) ? "/": "",
//                (!isEmptyString(tagName))? tagName : "",
//                (!isEmptyString(attribute)) ? " ":"",  (!isEmptyString(attribute))? attribute : "",
//                (type==XML_TAMKDG_TYPE_EMPTY) ? "/": ""
//        );
//    }
//    if (type==XML_TAMKDG_TYPE_EMPTY)
//        return strBuf;
//    if (type==XML_TAMKDG_TYPE_BEGIN_ONLY)
//        return strBuf;
//    if (type==XML_TAMKDG_TYPE_END_ONLY)
//        return strBuf;

//    if (type==XML_TAMKDG_TYPE_LONG){
//        g_string_append_c(strBuf,'\n');
//    }

//    if (value){
//        if (type==XML_TAMKDG_TYPE_LONG || type==XML_TAMKDG_TYPE_NO_TAG){
//            append_indent_space(strBuf,indentLevel+1);
//            int i, valueLen=strlen(value);
//            for(i=0;i<valueLen;i++){
//                g_string_append_c(strBuf,value[i]);
//                if (value[i]=='\n'){
//                    append_indent_space(strBuf,indentLevel+1);
//                }
//            }
//            g_string_append_c(strBuf,'\n');
//            if (type==XML_TAMKDG_TYPE_LONG){
//                append_indent_space(strBuf,indentLevel);
//            }
//        }else{
//            g_string_append(strBuf,value);
//        }
//    }

//    if (type==XML_TAMKDG_TYPE_LONG || type==XML_TAMKDG_TYPE_SHORT){
//        g_string_append_printf(strBuf,"</%s>",tagName);
//    }
//    return strBuf;
//}

//static void xml_tags_write(FILE *outF, const gchar *tagName, XmlTagsType type,
//        const gchar *attribute, const gchar *value){
//    static int indentLevel=0;
//    if (type==XML_TAMKDG_TYPE_END_ONLY)
//        indentLevel--;

//    GString *strBuf=xml_tags_to_string(tagName, type, attribute, value, indentLevel);
//    MAKER_DIALOG_DEBUG_MSG(3,",xml_tags_write:%s",strBuf->str);
//    fprintf(outF,"%s\n",strBuf->str);

//    if (type==XML_TAMKDG_TYPE_BEGIN_ONLY)
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
//    xml_tags_write(sData->outF,"locale",XML_TAMKDG_TYPE_BEGIN_ONLY,buf,NULL);
//    xml_tags_write(sData->outF,"short",XML_TAMKDG_TYPE_SHORT,NULL, gettext(ctx->spec->label));
//    xml_tags_write(sData->outF,"long",XML_TAMKDG_TYPE_LONG,NULL, gettext(ctx->spec->tooltip));
//    xml_tags_write(sData->outF,"locale",XML_TAMKDG_TYPE_END_ONLY,NULL,NULL);
//}

//static void ctx_write_callback(gpointer data, gpointer user_data){
//    MakerDialogPropertyContext *ctx=(MakerDialogPropertyContext *) data;
//    SchemasFileData *sData=(SchemasFileData *) user_data;
//    xml_tags_write(sData->outF,"schema",XML_TAMKDG_TYPE_BEGIN_ONLY,NULL,NULL);
//    gchar buf[STRING_BUFFER_SIZE_DEFAULT];
//    g_snprintf(buf,STRING_BUFFER_SIZE_DEFAULT,"/schemas%s/%s",sData->schemasHome,ctx->spec->key);
//    xml_tags_write(sData->outF,"key",XML_TAMKDG_TYPE_SHORT,NULL,buf);
//    xml_tags_write(sData->outF,"applyto",XML_TAMKDG_TYPE_SHORT,NULL,buf+strlen("/schemas"));
//    xml_tags_write(sData->outF,"owner",XML_TAMKDG_TYPE_SHORT,NULL,sData->owner);
//    switch(ctx->spec->valueType){
//        case MKDG_TYPE_BOOLEAN:
//            xml_tags_write(sData->outF,"type",XML_TAMKDG_TYPE_SHORT,NULL,"bool");
//            break;
//        case MKDG_TYPE_INT:
//        case MKDG_TYPE_UINT:
//            xml_tags_write(sData->outF,"type",XML_TAMKDG_TYPE_SHORT,NULL,"int");
//            break;
//        case MKDG_TYPE_STRING:
//            xml_tags_write(sData->outF,"type",XML_TAMKDG_TYPE_SHORT,NULL,"string");
//            break;
//        default:
//            break;
//    }
//    if (ctx->spec->defaultValue){
//        xml_tags_write(sData->outF,"default",XML_TAMKDG_TYPE_SHORT,NULL,ctx->spec->defaultValue);
//    }
//    gchar **localeArray=g_strsplit_set(sData->locales,":;",-1);
//    int i;
//    for(i=0;localeArray[i]!=NULL;i++){
//        ctx_write_locale(ctx,sData,localeArray[i]);
//    }
//    setlocale(LC_ALL,NULL);
//    xml_tags_write(sData->outF,"schema",XML_TAMKDG_TYPE_END_ONLY,NULL,NULL);
//}


