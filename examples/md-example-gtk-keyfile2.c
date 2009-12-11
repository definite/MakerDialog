#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <MakerDialog.h>
#include <MakerDialogUiGtk.h>

int main(int argc,char *argv[]){
    MakerDialogError  *cfgErr=NULL;
    gchar *dir=g_path_get_dirname (argv[0]);
    gchar *mkdgFile=g_strjoin("/",dir,"md-example.mkdg", NULL);
    MakerDialog *mDialog=maker_dialog_load_from_key_file(mkdgFile, &cfgErr);
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

    MakerDialogConfig *config=maker_dialog_config_use_key_file(mDialog);
    if (!config){
	exit(1);
    }
    MakerDialogPropertyContext *ctx1=maker_dialog_get_property_context(mDialog,"fgColor");
    g_assert(ctx1);

    MakerDialogConfigSet *dlgCfgSet_editing=maker_dialog_config_set_new_full(editingOptions,
	    MAKER_DIALOG_CONFIG_FLAG_HIDE_DUPLICATE | MAKER_DIALOG_CONFIG_FLAG_HIDE_DEFAULT,
	    "md-example-editing.cfg", searchDirs, "md-example-editing.cfg",  2, NULL);
    MakerDialogConfigSet *dlgCfgSet_key=maker_dialog_config_set_new_full(keyOptions,
	    MAKER_DIALOG_CONFIG_FLAG_HIDE_DUPLICATE | MAKER_DIALOG_CONFIG_FLAG_HIDE_DEFAULT,
	    "md-example-key.cfg", searchDirs, "md-example-key.cfg",  2, NULL);
    maker_dialog_config_add_config_set(config, dlgCfgSet_editing, &cfgErr);
    maker_dialog_config_add_config_set(config, dlgCfgSet_key, &cfgErr);
    maker_dialog_config_open_all(config, &cfgErr);
    if (!maker_dialog_config_load_all(config, &cfgErr)){
	exit(cfgErr->code);
    }

    /* Set UI */
    MakerDialogUi *ui=maker_dialog_ui_use_gtk(mDialog, &argc, &argv);
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

