#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <MakerDialog.h>
#include <MakerDialogUiGtk.h>

int main(int argc,char *argv[]){
    MakerDialogError  *cfgErr=NULL;
    gchar *dir=g_path_get_dirname (argv[0]);
    gchar *mkdgFile=g_strjoin("/",dir,"md-example.mkdg", NULL);
    MakerDialog *mDialog=mkdg_new_from_key_file(mkdgFile, &cfgErr);
    g_free(mkdgFile);
    g_free(dir);
    if (cfgErr){
	mkdg_error_print(cfgErr);
	g_error_free(cfgErr);
	exit(-1);
    }

    /* Configure */
    const gchar *searchDirs[]={"sys", "user" , NULL};
    const gchar *editingOptions[]={"Editing" , NULL};
    const gchar *keyOptions[]={"Keyboard" , NULL};

    MakerDialogConfig *config=mkdg_config_use_key_file(mDialog);
    if (!config){
	exit(1);
    }
    MakerDialogPropertyContext *ctx1=mkdg_get_property_context(mDialog,"fgColor");
    g_assert(ctx1);

    MakerDialogConfigSet *dlgCfgSet_editing=mkdg_config_set_new_full(editingOptions,
	    "md-example-editing.cfg", searchDirs, "md-example-editing.cfg", 2,
	    MKDG_CONFIG_FLAG_HIDE_DUPLICATE | MKDG_CONFIG_FLAG_HIDE_DEFAULT,
	    &MKDG_CONFIG_FILE_INTERFACE_KEY_FILE, NULL);
    MakerDialogConfigSet *dlgCfgSet_key=mkdg_config_set_new_full(keyOptions,
	    "md-example-key.cfg", searchDirs, "md-example-key.cfg", 2,
	    MKDG_CONFIG_FLAG_HIDE_DUPLICATE | MKDG_CONFIG_FLAG_HIDE_DEFAULT,
	    &MKDG_CONFIG_FILE_INTERFACE_KEY_FILE, NULL);

    mkdg_config_add_config_set(config, dlgCfgSet_editing, &cfgErr);
    mkdg_config_add_config_set(config, dlgCfgSet_key, &cfgErr);
    mkdg_config_open_all(config, &cfgErr);
    if (!mkdg_config_load_all(config, &cfgErr)){
	exit(cfgErr->code);
    }

    /* Set UI */
    MakerDialogUi *ui=mkdg_ui_use_gtk(mDialog, &argc, &argv);
    if (!ui){
	exit(1);
    }

    mkdg_ui_construct(ui,NULL,TRUE);
    gint ret=0;
    do{
        ret=mkdg_ui_run(ui);
    }while(ret!=MKDG_RESPONSE_DELETE_EVENT && ret!=MKDG_RESPONSE_CLOSE);

    mkdg_config_save_all(config, &cfgErr);
    mkdg_config_close_all(config, &cfgErr);
    mkdg_destroy(mDialog);
    exit(0);
}

