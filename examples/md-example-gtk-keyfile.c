#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <MakerDialog.h>
#include <MakerDialogUiGtk.h>
#include "md-example.c"

int main(int argc,char *argv[]){
    MakerDialog *mDialog=mkdg_init("md-example", buttonSpecs);
    gint i=0;
    for(i=0; propSpecs[i].key!=NULL; i++){
	mkdg_add_property(mDialog,
		mkdg_property_context_new_full( &propSpecs[i], NULL, NULL, applyFunc));
    }

    /* Configure */
    const gchar *searchDirs[]={"sys", "user" , NULL};
    const gchar *editingOptions[]={"Editing" , NULL};
    const gchar *keyOptions[]={"Keyboard" , NULL};

    MakerDialogConfig *config=mkdg_config_use_key_file(mDialog);
    if (!config){
	exit(1);
    }
    MakerDialogConfigSet *dlgCfgSet_editing=mkdg_config_set_new_full(editingOptions,
	    "md-example-editing.cfg", searchDirs, "md-example-editing.cfg", 2,
	    MKDG_CONFIG_FLAG_HIDE_DUPLICATE | MKDG_CONFIG_FLAG_HIDE_DEFAULT,
	    &MKDG_CONFIG_FILE_INTERFACE_KEY_FILE, NULL);
    MakerDialogConfigSet *dlgCfgSet_key=mkdg_config_set_new_full(keyOptions,
	    "md-example-key.cfg", searchDirs, "md-example-key.cfg", 2,
	    MKDG_CONFIG_FLAG_HIDE_DUPLICATE | MKDG_CONFIG_FLAG_HIDE_DEFAULT,
	    &MKDG_CONFIG_FILE_INTERFACE_KEY_FILE, NULL);
    mkdg_config_add_config_set(config, dlgCfgSet_editing, NULL);
    mkdg_config_add_config_set(config, dlgCfgSet_key, NULL);
    mkdg_config_open_all(config, NULL);
    if (!mkdg_config_load_all(config, NULL)){
	exit(1);
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

    mkdg_config_save_all(config, NULL);
    mkdg_config_close_all(config, NULL);
    mkdg_destroy(mDialog);
    exit(0);
}

