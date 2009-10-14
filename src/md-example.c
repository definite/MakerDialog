#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "MakerDialog.h"
#include "MakerDialogGtk.h"

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

MakerDialogPropertySpec propSpec=
    {"KBType", G_TYPE_STRING,
	MAKER_DIALOG_PROPERTY_FLAG_INEDITABLE | MAKER_DIALOG_PROPERTY_FLAG_HAS_TRANSLATION,
	"default", kbType_ids, 0.0, 30.0, 1.0, 0,
	"Keyboard",
      	N_("Keyboard Type"),
	"ibus-chewing",
	N_("Select keyboard layout of Zhuyin symbols."),
	NULL
    };

void KBType_applyFunc(MakerDialogPropertyContext *ctx, GValue *value){
    printf("KBType_apply: %s\n", g_value_get_string(value));
}

int main(int argc,char *argv[]){
    g_type_init();
    MakerDialog *mDialog=maker_dialog_init("Hello World!", 1, &buttonSpec);
    maker_dialog_add_property(mDialog,
	    maker_dialog_property_context_new_full( &propSpec, NULL, NULL, NULL, KBType_applyFunc));
    MakerDialogUi *dlgUi=maker_dialog_ui_use_gtk(mDialog, &argc, &argv);
    maker_dialog_ui_construct(dlgUi,NULL,TRUE);
    gint ret=0;
    do{
	ret=maker_dialog_ui_run(dlgUi);
    }while(ret!=MAKER_DIALOG_RESPONSE_DELETE_EVENT && ret!=MAKER_DIALOG_RESPONSE_CLOSE);

    maker_dialog_ui_destroy(dlgUi);
    return 0;
}

