#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <MakerDialog.h>
#include <MakerDialogUiGtk.h>
#include "md-example.c"

int main(int argc,char *argv[]){
    MakerDialog *mDialog=maker_dialog_init("md-example", buttonSpecs);
    gint i=0;
    for(i=0; propSpecs[i].key!=NULL; i++){
	maker_dialog_add_property(mDialog,
		maker_dialog_property_context_new_full( &propSpecs[i], NULL, NULL, applyFunc));
    }

    /* Configure */
    const gchar *searchDirs[]={"sys", "user" , NULL};
    const gchar *editingOptions[]={"Editing" , NULL};
    const gchar *keyOptions[]={"Keyboard" , NULL};

    MakerDialogConfig *config=maker_dialog_config_use_key_file(mDialog);
    if (!config){
	exit(1);
    }
    MakerDialogConfigSet *dlgCfgSet_editing=maker_dialog_config_set_new_full(editingOptions,
	    "md-example-editing.cfg", searchDirs, "md-example-editing.cfg", 2,
	    MAKER_DIALOG_CONFIG_FLAG_HIDE_DUPLICATE | MAKER_DIALOG_CONFIG_FLAG_HIDE_DEFAULT,
	    &MAKER_DIALOG_CONFIG_FILE_INTERFACE_KEY_FILE, NULL);
    MakerDialogConfigSet *dlgCfgSet_key=maker_dialog_config_set_new_full(keyOptions,
	    "md-example-key.cfg", searchDirs, "md-example-key.cfg", 2,
	    MAKER_DIALOG_CONFIG_FLAG_HIDE_DUPLICATE | MAKER_DIALOG_CONFIG_FLAG_HIDE_DEFAULT,
	    &MAKER_DIALOG_CONFIG_FILE_INTERFACE_KEY_FILE, NULL);
    maker_dialog_config_add_config_set(config, dlgCfgSet_editing, NULL);
    maker_dialog_config_add_config_set(config, dlgCfgSet_key, NULL);
    maker_dialog_config_open_all(config, NULL);
    if (!maker_dialog_config_load_all(config, NULL)){
	exit(1);
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

    maker_dialog_config_save_all(config, NULL);
    maker_dialog_config_close_all(config, NULL);
    maker_dialog_destroy(mDialog);
    exit(0);
}

