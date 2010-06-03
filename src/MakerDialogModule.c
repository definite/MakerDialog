#include <dlfcn.h>
#include "MakerDialogModule.h"
#include "MakerDialog.h"
static MkdgIdPair mkdgModuleData[]={
    {"GCONF2",			MKDG_MODULE_GCONF2},
    {"GKEYFILE",		MKDG_MODULE_GKEYFILE},
    {"GTK2",			MKDG_MODULE_GTK2},
    {NULL,			MKDG_MODULE_INVALID},
};

MKDG_MODULE mkdg_module_parse(const gchar *moduleName){
    return mkdg_id_parse(mkdgModuleData, moduleName, FALSE);
}

static gboolean mkdg_module_load_private(Mkdg *mDialog, const gchar *modulePath, MkdgError **error){
    MKDG_DEBUG_MSG(5, "[I5] module_load_private( , %s, )", modulePath);
    gpointer lib_handle=dlopen(modulePath, RTLD_NOW);
    gboolean (* module_init_func)(Mkdg*);
    MkdgError *cfgErr=NULL;
    if (!lib_handle){
	cfgErr=mkdg_error_new(MKDG_ERROR_LOADER_CANT_LOAD_LIB, ": %s", modulePath);
	mkdg_error_handle(cfgErr,error);
	return FALSE;
    }

    module_init_func=dlsym(lib_handle, "mkdg_module_init");
    gchar *errMsg=dlerror();
    if (errMsg!=NULL){
	cfgErr=mkdg_error_new(MKDG_ERROR_LOADER_CANT_LOAD_SYM, ": %s", "mkdg_module_init");
	mkdg_error_handle(cfgErr,error);
	dlclose(lib_handle);
	return FALSE;
    }
    return module_init_func(mDialog);
}

gboolean mkdg_module_load(Mkdg *mDialog, MKDG_MODULE module, MkdgError **error){
    switch(module){
	case MKDG_MODULE_GCONF2:
	    return mkdg_module_load_private(mDialog, "libMkdgGConf2.so", error);
	case MKDG_MODULE_GKEYFILE:
	    return mkdg_module_load_private(mDialog, "libMkdgGKeyFile.so", error);
	case MKDG_MODULE_GTK2:
	    return mkdg_module_load_private(mDialog, "libMkdgGtk2.so", error);
	    break;
	default:
	    break;
    }
    return FALSE;
}

