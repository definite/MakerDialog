#include <stdlib.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <MakerDialog.h>
#include <MakerDialogUiGtk.h>
#include <MakerDialogConfigGConf.h>
#include "md-example.c"

int main(int argc,char *argv[]){
    g_type_init();
    MakerDialog *mDialog=mkdg_init("Hello World!", buttonSpecs);
    gint i=0;
    for(i=0; propSpecs[i].key!=NULL; i++){
	mkdg_add_property(mDialog,
		mkdg_property_context_new_full( &propSpecs[i], NULL, NULL, applyFunc));
    }

    MakerDialogConfig *config=mkdg_config_use_gconf(mDialog);
    if (!config){
	exit(1);
    }
    MakerDialogConfigSet *configSet=mkdg_config_set_new_full(
	    NULL, "/apps/md-example-gtk-gconf", NULL, NULL,  -1,
	    0, &MKDG_CONFIG_FILE_INTERFACE_GCONF, NULL);
    mkdg_config_add_config_set(config, configSet, NULL);
    mkdg_config_open_all(config, NULL);
    if (!mkdg_config_load_all(config, NULL)){
	exit(-1);
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

