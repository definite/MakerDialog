#include <dlfcn.h>
#include "MakerDialogModule.h"
#include "MakerDialog.h"
static MakerDialogIdPair mkdgModuleData[]={
    {"GCONF2",			MAKER_DIALOG_MODULE_GCONF2},
    {"GKEYFILE",		MAKER_DIALOG_MODULE_GKEYFILE},
    {"GTK2",			MAKER_DIALOG_MODULE_GTK2},
    {NULL,			MAKER_DIALOG_MODULE_INVALID},
};

MAKER_DIALOG_MODULE maker_dialog_module_parse(const gchar *moduleName){
    return maker_dialog_id_parse(mkdgModuleData, moduleName, FALSE);
}

static gboolean maker_dialog_module_load_private(MakerDialog *mDialog, const gchar *modulePath, MakerDialogError **error){
    MAKER_DIALOG_DEBUG_MSG(5, "[I5] module_load_private( , %s, )", modulePath);
    gpointer lib_handle=dlopen(modulePath, RTLD_NOW);
    gboolean (* module_init_func)(MakerDialog*);
    MakerDialogError *cfgErr=NULL;
    if (!lib_handle){
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_LOADER_CANT_LOAD_LIB, ": %s", modulePath);
	maker_dialog_error_handle(cfgErr,error);
	return FALSE;
    }

    module_init_func=dlsym(lib_handle, "maker_dialog_module_init");
    gchar *errMsg=dlerror();
    if (errMsg!=NULL){
	cfgErr=maker_dialog_error_new(MAKER_DIALOG_ERROR_LOADER_CANT_LOAD_SYM, ": %s", "maker_dialog_module_init");
	maker_dialog_error_handle(cfgErr,error);
	dlclose(lib_handle);
	return FALSE;
    }
    return module_init_func(mDialog);
}

gboolean maker_dialog_module_load(MakerDialog *mDialog, MAKER_DIALOG_MODULE module, MakerDialogError **error){
    switch(module){
	case MAKER_DIALOG_MODULE_GCONF2:
	    return maker_dialog_module_load_private(mDialog, "libMakerDialogGConf2.so", error);
	case MAKER_DIALOG_MODULE_GKEYFILE:
	    return maker_dialog_module_load_private(mDialog, "libMakerDialogGKeyFile.so", error);
	case MAKER_DIALOG_MODULE_GTK2:
	    return maker_dialog_module_load_private(mDialog, "libMakerDialogGtk2.so", error);
	    break;
	default:
	    break;
    }
    return FALSE;
}

