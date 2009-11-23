#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include "MakerDialog.h"
#include "MakerDialogUiGtk.h"
#include "MakerDialogConfigGConf.h"
#include "md-example.c"

int main(int argc,char *argv[]){
    g_type_init();
    MakerDialog *mDialog=maker_dialog_init("Hello World!", buttonSpecs);
    gint i=0;
    for(i=0; propSpecs[i].key!=NULL; i++){
	maker_dialog_add_property(mDialog,
		maker_dialog_property_context_new_full( &propSpecs[i], NULL, NULL, applyFunc));
    }

    MakerDialogConfig *config=maker_dialog_config_use_gconf(mDialog);
    if (!config){
	exit(1);
    }
    MakerDialogConfigSet *configSet=maker_dialog_config_set_new_full(NULL,0,
	    "/apps/md-example-gtk-gconf", NULL, NULL,  -1, NULL);
    maker_dialog_config_add_config_set(config, configSet, NULL);
    maker_dialog_config_open_all(config, NULL);
    if (!maker_dialog_config_load_all(config, NULL)){
	exit(-1);
    }
    g_debug("*** 1\n");
    /* Set UI */
    g_debug("*** 1\n");
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

