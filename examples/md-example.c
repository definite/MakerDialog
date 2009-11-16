#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "MakerDialog.h"
#include "MakerDialogUiGtk.h"

MakerDialogButtonSpec buttonSpec={MAKER_DIALOG_RESPONSE_CLOSE, NULL};
const gchar *kbType_ids[]={
    N_("default"),
    N_("hsu"),
    N_("ibm"),
    N_("gin_yieh"),
    N_("eten"),
    N_("eten26"),
    N_("dvorak"),
    N_("dvorak_hsu"),
    N_("dachen_26"),
    N_("hanyu"),
    NULL
};

MakerDialogControlRule kbRules[]={
    {MAKER_DIALOG_RELATION_NOT_EQUAL, "hsu", "hsuSelKeyType",  MAKER_DIALOG_WIDGET_CONTROL_INSENSITIVE, 0 },
    {MAKER_DIALOG_RELATION_NOT_EQUAL, "dvorak_hsu", "hsuSelKeyType",  MAKER_DIALOG_WIDGET_CONTROL_INSENSITIVE, 0 },
    {MAKER_DIALOG_RELATION_EQUAL, "hsu", "hsuSelKeyType",  MAKER_DIALOG_WIDGET_CONTROL_SENSITIVE, 0 },
    {MAKER_DIALOG_RELATION_EQUAL, "dvorak_hsu", "hsuSelKeyType",  MAKER_DIALOG_WIDGET_CONTROL_SENSITIVE, 0 },
    {MAKER_DIALOG_RELATION_EQUAL, NULL, NULL, 0, 0 }
};

const gchar *selKeys_array[]={
    "1234567890",
    "asdfghjkl;",
    "asdfzxcv89",
    "asdfjkl789",
    "aoeuhtn789",
    "1234qweras",
    NULL
};

MakerDialogPropertySpec propSpecs[]={
    {"fgColor", MKDG_TYPE_COLOR,
	0,
	"0", NULL, NULL, NULL, 0.0, 0.0, 0.0, 0,
	"Editing",
	N_("Color"),
	N_("Foreground color"),
	"ibus-chewing",
	N_("Foreground color."),
	NULL, NULL, NULL
    },
   {"autoShiftCur", MKDG_TYPE_BOOLEAN,
	0,
	"0", NULL, NULL, NULL, 0.0, 0.0, 0.0, 0,
	"Editing",
	N_("Text"),
	N_("Auto move cursor"),
	"ibus-chewing",
	N_("Automatically move cursor to next character."),
	NULL, NULL, NULL
    },
    {"addPhraseDirection", MKDG_TYPE_BOOLEAN,
	0,
	"0", NULL, NULL, NULL, 0.0, 0.0, 0.0, 0,
	"Editing",
	N_("Text"),
	N_("Add phrases in the front"),
	"ibus-chewing",
	N_("Add phrases in the front"),
	NULL, NULL, NULL
    },
    {"easySymbolInput", MKDG_TYPE_BOOLEAN,
	0,
	"1", NULL, NULL, NULL, 0.0, 0.0, 0.0, 0,
	"Editing",
	N_("Text"),
	N_("Easy symbol input."),
	"ibus-chewing",
	N_("Easy symbol input."),
	NULL, NULL, NULL
    },

    {"KBType", MKDG_TYPE_STRING,
	MAKER_DIALOG_PROPERTY_FLAG_FIXED_SET | MAKER_DIALOG_PROPERTY_FLAG_HAS_TRANSLATION | MAKER_DIALOG_PROPERTY_FLAG_PREFER_RADIO_BUTTONS,
//	MAKER_DIALOG_PROPERTY_FLAG_FIXED_SET | MAKER_DIALOG_PROPERTY_FLAG_HAS_TRANSLATION ,
	"default", kbType_ids, NULL, NULL, 0.0, 25.0, 1.0, 0,
	"Keyboard",
	NULL,
	N_("Keyboard Type"),
	"ibus-chewing",
	N_("Select keyboard layout of Zhuyin symbols."),
	NULL, kbRules, NULL
    },
    {"selKeys", MKDG_TYPE_STRING,
	0,
	"1234567890", selKeys_array, NULL, NULL, 0.0, 25.0, 1.0, 0,
	"Keyboard",
	NULL,
	N_("Selection keys"),
	"ibus-chewing",
	N_("Keys used to select candidate. For example \"asdfghjkl;\", press 'a' to select the 1st candidate, 's' for 2nd, and so on."),
	NULL, NULL, NULL
    },
    {"hsuSelKeyType", MKDG_TYPE_INT,
	0,
	"1", NULL, NULL, NULL, 1.0, 2.0, 1.0, 0,
	"Keyboard",
	NULL,
	N_("Hsu's selection key"),
	"ibus-chewing",
	N_("Hsu's keyboard selection keys, 1 for asdfjkl789, 2 for asdfzxcv89 ."),
	NULL, NULL, NULL
    },
    MAKER_DIALOG_PROPERTY_SPEC_ENDER
};

void applyFunc(MakerDialogPropertyContext *ctx, GValue *value){
    printf("Key %s= %s\n", ctx->spec->key, maker_dialog_g_value_to_string(value,ctx->spec->toStringFormat));
}

int main(int argc,char *argv[]){
    g_type_init();
    MakerDialog *mDialog=maker_dialog_init("Hello World!", 1, &buttonSpec);
    gint i=0;
    for(i=0; propSpecs[i].key!=NULL; i++){
	maker_dialog_add_property(mDialog,
		maker_dialog_property_context_new_full( &propSpecs[i], NULL, NULL, applyFunc));
    }

    /* Configure */
    const gchar *searchDirs[]={"sys", "user" , NULL};
    const gchar *editingOptions[]={"Editing" , NULL};
    const gchar *keyOptions[]={"Keyboard" , NULL};

    GError *cfgErr=NULL;
    MakerDialogConfig *config=maker_dialog_config_use_key_file(mDialog);
    if (!config){
	exit(1);
    }
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

