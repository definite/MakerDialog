/*=== Start toolkit interface definitions ===*/
static MkdgValue *mkdg_widget_get_value_gtk(MkdgUi *ui, const gchar *key){
    return mkdg_gtk_get_widget_value(MKDG_GTK(ui->dlgObj),key);
}

static void mkdg_widget_set_value_gtk(MkdgUi *ui, const gchar *key, MkdgValue *value){
    mkdg_gtk_set_widget_value(MKDG_GTK(ui->dlgObj),key,value);
}

static void mkdg_widget_control_gtk(MkdgUi *ui, const gchar *key, MkdgWidgetControl control){
    mkdg_gtk_widget_control(MKDG_GTK(ui->dlgObj),key,control);
}

static gpointer mkdg_construct_gtk(MkdgUi *ui, gpointer parentWindow, gboolean modal){
    MkdgGtk *dlg_gtk=mkdg_gtk_new_full(ui,parentWindow,modal);
    return (gpointer) dlg_gtk;
}

static gint mkdg_run_gtk(MkdgUi *ui){
    return gtk_dialog_run(GTK_DIALOG (ui->dlgObj));
}

static void mkdg_show_gtk(MkdgUi *ui){
    gtk_widget_show_all(GTK_WIDGET (ui->dlgObj));
}

static void mkdg_hide_gtk(MkdgUi *ui){
    gtk_widget_hide(GTK_WIDGET (ui->dlgObj));
}

static void mkdg_destroy_gtk(MkdgUi *ui){
    mkdg_gtk_destroy(MKDG_GTK( ui->dlgObj));
}

static gpointer mkdg_get_widget_gtk(MkdgUi *ui, const gchar *key){
    return (gpointer) mkdg_gtk_get_widget(MKDG_GTK( ui->dlgObj), key);
}

static MkdgToolkitInterface makerDialogToolkitInterface_gtk={
    mkdg_widget_get_value_gtk,
    mkdg_widget_set_value_gtk,
    mkdg_widget_control_gtk,
    mkdg_construct_gtk,
    mkdg_run_gtk,
    mkdg_show_gtk,
    mkdg_hide_gtk,
    mkdg_destroy_gtk,
    mkdg_get_widget_gtk
};

MkdgUi *mkdg_ui_use_gtk(Mkdg *mDialog, gint *argc, gchar ***argv){
    if (gtk_init_check(argc, argv)){
	MkdgUi *ui=mkdg_ui_init(mDialog, &makerDialogToolkitInterface_gtk);
	return ui;
    }
    return NULL;
}

gboolean mkdg_module_init(Mkdg *mDialog){
    if (gtk_init_check(&mDialog->argc, &mDialog->argv)){
	mkdg_ui_init(mDialog, &makerDialogToolkitInterface_gtk);
	return TRUE;
    }
    return FALSE;
}



/*=== End toolkit interface definitions ===*/

/*=== Start foreach functions ===*/
static void mkdg_gtk_construct_ui_groupFunc(Mkdg *mDialog, GNode *pageNode, GNode *groupNode, gpointer userData){
    MkdgGtk *dlg_gtk=MKDG_GTK(userData);
    gchar *pageName=(gchar *) pageNode->data;
    gchar *groupName=(gchar *) groupNode->data;
    if (mkdg_group_name_is_empty(groupName) && g_node_n_children(pageNode)<=1){
	/* No group name */
	return;
    }
    GtkWidget *frameLabel=gtk_label_new(NULL);
    gchar *markup = g_markup_printf_escaped ("<span size=\"larger\" weight=\"bold\">%s</span>", _(groupName));
    gtk_label_set_markup (GTK_LABEL (frameLabel), markup);
    g_free (markup);
    GtkWidget *frame=gtk_frame_new(NULL);
    gtk_frame_set_label_widget(GTK_FRAME(frame),frameLabel);
    gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_NONE);
    mkdg_gtk_widget_register(dlg_gtk, frame, pageName, groupName, "groupFrame", -1);
    GtkWidget *box=gtk_vbox_new(FALSE,dlg_gtk->vbox_spacing);
    mkdg_gtk_widget_register(dlg_gtk, box, pageName, groupName, "box", -1);
    gtk_container_add(GTK_CONTAINER(frame), box);
    GtkWidget *pageBox=mkdg_gtk_get_page_vbox(dlg_gtk, pageName);
    gtk_container_add(GTK_CONTAINER(pageBox), frame);
}

static void mkdg_gtk_construct_ui_propFunc(Mkdg *mDialog, MkdgPropertyContext *ctx, gpointer userData){
    MkdgGtk *dlg_gtk=MKDG_GTK(userData);
    mkdg_gtk_add_property_ui(dlg_gtk, ctx);
}

static void mkdg_gtk_property_each_control_rule(MkdgPropertyContext *ctx, MkdgWidgetControl control, gpointer userData){
    MkdgGtk *dlg_gtk=MKDG_GTK(userData);
    mkdg_gtk_widget_control(dlg_gtk, ctx->spec->key, control);
}

static void mkdg_gtk_each_property_apply_control(gpointer hashKey, gpointer hashValue, gpointer userData){
    MkdgPropertyContext *ctx=(MkdgPropertyContext *) hashValue;
    mkdg_property_foreach_control_rule(ctx, mkdg_gtk_property_each_control_rule, userData);
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
static gchar *widget_gen_id(const gchar *widget_label, const gchar *subLabel, const gchar *widget_type, gint serial){
    GString *strBuf=g_string_new(NULL);
    g_string_append_printf(strBuf, "%s%s%s%s%s",
	    widget_label,  (widget_type)? WIDGET_ID_SEPARATOR : "",
	    (subLabel)? subLabel: "",  (widget_type)? WIDGET_ID_SEPARATOR : "",
	    (widget_type)? widget_type : "");
    if (serial>=0){
	g_string_append_printf( strBuf, WIDGET_ID_SEPARATOR "%d", serial);
    }
    return g_string_free(strBuf, FALSE);
}

static const gchar *mkdg_gtk_get_translation_string(const gchar *str, MkdgPropertySpec *spec){
    if (spec-> translationContext)
	return g_dpgettext2(NULL,spec->translationContext,str);
    return _(str);
}

static void g_string_chunk_free_wrap(gpointer ptr){
    g_string_chunk_free((GStringChunk *) ptr);
}

static MkdgColor MkdgValue_get_GdkColor(MkdgValue* value, GdkColor *color){
    MkdgColor colorValue=mkdg_value_get_color(value);
    color->red=((colorValue & 0xFF0000) >> 16)  * 0x101;
    color->green=((colorValue & 0x00FF00) >> 8) * 0x101;
    color->blue=(colorValue & 0x0000FF) * 0x101;
    MKDG_DEBUG_MSG(4,"[I4] MkdgValue_get_GdkColor() red=%X green=%X blue=%X colorValue=%X\n",
	    color->red,color->green,color->blue,colorValue);
    return colorValue;
}

static MkdgColor MkdgValue_set_GdkColor(MkdgValue* value, GdkColor *color){
    MkdgColor colorValue=(color->red/256<<16) + (color->green/256<<8) + color->blue/256;
    mkdg_value_set_color(value, colorValue);
    MKDG_DEBUG_MSG(4,"[I4] MkdgValue_set_GdkColor() red=%X green=%X blue=%X colorValue=%X\n",
	    color->red,color->green,color->blue,colorValue);
    return colorValue;
}

/*=== End misc utility functions ===*/

/*=== Start listStore functions ===*/
static void listStore_append(GtkListStore *listStore, const gchar *listKey, const gchar *prompt){
    GtkTreeIter iter;

    MKDG_DEBUG_MSG(5,"[I5] Gtk:listStore_append(-, %s, %s): ",listKey, prompt);
    gtk_list_store_append (listStore, &iter);
    gtk_list_store_set( listStore, &iter,
	    0, listKey,
	    1, prompt,
	    -1);
}

static void listStore_prepend(GtkListStore *listStore, const gchar *listKey, const gchar *prompt){
    GtkTreeIter iter;

    MKDG_DEBUG_MSG(5,"[I5] Gtk:listStore_prepend(-, %s, %s): ",listKey, prompt);
    gtk_list_store_prepend (listStore, &iter);
    gtk_list_store_set( listStore, &iter,
	    0, listKey,
	    1, prompt,
	    -1);
}

static gint listStore_find_value(GtkListStore *listStore, MkdgValue *value, MkdgPropertySpec *spec){
    int i=0,index=-1;
    GtkTreeIter iter;
    gchar *valueStr=mkdg_value_to_string(value, spec->toStringFormat);
    MKDG_DEBUG_MSG(5,"[I5] Gtk:listStore_find_value(-,%s,%s)", valueStr, spec->key);
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
    if (index<0 && !(spec->flags & MKDG_PROPERTY_FLAG_FIXED_SET)){
	/* Add new item*/
	listStore_append(listStore, valueStr, valueStr);
	index=i;
    }
    MKDG_DEBUG_MSG(5,"[I5] Gtk:listStore_find_value(-,%s,%s) index=%d", valueStr, spec->key, index);
    g_free(valueStr);
    return index;
}

/*=== End listStore functions ===*/

static gint combo_find_value_index(GtkComboBox *combo, MkdgValue *value, MkdgPropertySpec *spec){
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

/*=== Start Widget Callback function wraps ===*/

static gboolean validate_and_apply(MkdgPropertyContext *ctx){
    gboolean ret=TRUE;
    if (mkdg_ui_update(ctx->mDialog->ui, ctx)){
	mkdg_apply_value(ctx->mDialog, ctx->spec->key);
    }else{
	gchar *prevString=mkdg_property_to_string(ctx);
	MkdgValue *value=mkdg_widget_get_value_gtk(ctx->mDialog->ui, ctx->spec->key);
	gchar *newString=mkdg_value_to_string(value, NULL);
	g_warning(_("Invalid value: %s, Fall back to previous value: %s"), newString, prevString);
	g_free(prevString);
	g_free(newString);
	mkdg_value_free(value);
	mkdg_widget_set_value_gtk(ctx->mDialog->ui, ctx->spec->key, ctx->value);
	ret=FALSE;
    }
    return ret;
}

static void widget_event_wrap(GtkWidget *widget, gpointer userData){
    MKDG_DEBUG_MSG(6,"[I6] widget_event_wrap()");
    MkdgPropertyContext *ctx=(MkdgPropertyContext *)userData;
    validate_and_apply(ctx);
}

static void on_combo_box_entry_change_wrap(GtkComboBox *comboBox, gpointer  userData){
    MKDG_DEBUG_MSG(6,"[I6] on_combo_box_entry_change_wrap()");
    if (gtk_combo_box_get_active (comboBox)<0){
	/* Only Typing. Wait for active signal */
	return;
    }
    MkdgPropertyContext *ctx=(MkdgPropertyContext *) userData;
    if (!ctx->mDialog->ui->dlgObj){
	/* During initialization */
	return;
    }
    validate_and_apply(ctx);
}

static void on_radioButton_toggled_wrap(GtkRadioButton *button, gpointer userData){
    MKDG_DEBUG_MSG(6,"[I6] on_radioButton_toggled_wrap()");
    MkdgPropertyContext *ctx=(MkdgPropertyContext *)userData;
    /* Ignore the toggle-off event */
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
	validate_and_apply(ctx);
}

/*=== End Widget Callback function wraps ===*/

