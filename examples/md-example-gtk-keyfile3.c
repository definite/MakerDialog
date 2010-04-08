#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <MakerDialog.h>
#include <MakerDialogUiGtk.h>

int main(int argc,char *argv[]){
    MakerDialogError  *cfgErr=NULL;
    gchar *dir=g_path_get_dirname (argv[0]);
    gchar *mkdgFile=g_strjoin("/",dir,"md-example.mkdg", NULL);
    MakerDialog *mDialog=maker_dialog_new_from_key_file(mkdgFile, &cfgErr);
    maker_dialog_set_args(mDialog,argc,argv);
    g_free(mkdgFile);
    g_free(dir);
    if (cfgErr){
	maker_dialog_error_print(cfgErr);
	g_error_free(cfgErr);
	exit(-1);
    }

    /* Configure */
    const gchar *searchDirs[]={"sys", "user" , NULL};
    const gchar *editingOptions[]={"Editing" , NULL};
    const gchar *keyOptions[]={"Keyboard" , NULL};

    if (!maker_dialog_module_load(mDialog, MAKER_DIALOG_MODULE_GKEYFILE,NULL)){
	g_print("GKeyFile not loaded!\n");
	exit(-1);
    }

    MakerDialogConfig *config=mDialog->config;
    if (!config){
	exit(1);
    }
    MakerDialogPropertyContext *ctx1=maker_dialog_get_property_context(mDialog,"fgColor");
    g_assert(ctx1);

    MakerDialogConfigSet *dlgCfgSet_editing=maker_dialog_config_set_new_full(editingOptions,
	    "md-example-editing.cfg", searchDirs, "md-example-editing.cfg", 2,
	    MAKER_DIALOG_CONFIG_FLAG_HIDE_DUPLICATE | MAKER_DIALOG_CONFIG_FLAG_HIDE_DEFAULT,
	    &MAKER_DIALOG_CONFIG_FILE_INTERFACE_KEY_FILE, NULL);
    MakerDialogConfigSet *dlgCfgSet_key=maker_dialog_config_set_new_full(keyOptions,
	    "md-example-key.cfg", searchDirs, "md-example-key.cfg", 2,
	    MAKER_DIALOG_CONFIG_FLAG_HIDE_DUPLICATE | MAKER_DIALOG_CONFIG_FLAG_HIDE_DEFAULT,
	    &MAKER_DIALOG_CONFIG_FILE_INTERFACE_KEY_FILE, NULL);

    maker_dialog_config_add_config_set(config, dlgCfgSet_editing, &cfgErr);
    maker_dialog_config_add_config_set(config, dlgCfgSet_key, &cfgErr);
    maker_dialog_config_open_all(config, &cfgErr);
    if (!maker_dialog_config_load_all(config, &cfgErr)){
	exit(cfgErr->code);
    }

    /* Set UI */
    if (!maker_dialog_module_load(mDialog, MAKER_DIALOG_MODULE_GTK2,NULL)){
	g_print("GTK2 not loaded!\n");
	exit(-1);
    }
    MakerDialogUi *ui=mDialog->ui;
    if (!ui){
	exit(1);
    }

    maker_dialog_ui_construct(ui,NULL,TRUE);
    gint ret=0;
    do{
        ret=maker_dialog_ui_run(ui);
    }while(ret!=MAKER_DIALOG_RESPONSE_DELETE_EVENT && ret!=MAKER_DIALOG_RESPONSE_CLOSE);

    maker_dialog_config_save_all(config, &cfgErr);
    maker_dialog_config_close_all(config, &cfgErr);
    maker_dialog_destroy(mDialog);
    exit(0);
}

