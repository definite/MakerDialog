#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "MakerDialog.h"
#include "maker-dialog-gtk.h"

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


int main(int argc,char *argv[]){
    MakerDialog *dlg=maker_dialog_init("Hello World!", 1, &buttonSpec);
    printf("=== Test 1\n");
    printf("=== propSpec key=%s validType=%lu,  defaultValue=%s\n",propSpec.key, propSpec.valueType, propSpec.defaultValue);
    maker_dialog_add_property(dlg,
	    maker_dialog_property_context_new( &propSpec, NULL, NULL));
    printf("Test 2\n");
    maker_dialog_set_toolkit_handler(dlg, &makerDialogHandlerGtk);
    printf("Test 3\n");
    maker_dialog_construct(dlg);
    printf("Test 4\n");
    gint ret=0;
    do{
	printf("Test 5\n");
	ret=maker_dialog_run(dlg);
    }while(ret!=MAKER_DIALOG_RESPONSE_DELETE_EVENT && ret!=MAKER_DIALOG_RESPONSE_CLOSE);
    maker_dialog_hide(dlg);
    return 0;
}

