#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <locale.h>
#include <stdlib.h>
#include "MakerDialog.h"
#include "MakerDialogConfigGConf.h"
#include "../examples/md-example.c"

static gchar *schemasFilename=NULL;
static gchar *schemasHome=NULL;
static gchar *owner=NULL;
static gchar *localeStr=NULL;
static gint indentSpace=4;
static const gchar localeDefault[]="C";

static const GOptionEntry entries[] =
{
    { "indent", 'i', 0, G_OPTION_ARG_INT, &indentSpace,
	"Number of spaces for indent. Default is 4.",
	"[num]" },
    { "schemasHome", 's', 0, G_OPTION_ARG_STRING, &schemasHome,
	"The \"home directory\" of GConf schemas.",
	"[str]" },
    { "owner", 'w', 0, G_OPTION_ARG_STRING, &owner,
	"GConf owner, usually the project name.",
	"[str]" },
    { "locale", 'l', 0, G_OPTION_ARG_STRING, &localeStr,
	"Supported locale. Use ';' to separate locales.",
	"[str]" },
    { NULL },
};

gint main (gint argc, gchar *argv[])
{
    GOptionContext *context;
    GError *error=NULL;

    g_type_init();

    /* Init i18n messages */
    setlocale (LC_ALL, "");
    bindtextdomain(PROJECT_NAME, DATADIR "/locale");
    textdomain(PROJECT_NAME);
    context = g_option_context_new("schemasFile");

    g_option_context_add_main_entries (context, entries, "MkdgGConfSchemas");

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
	g_printf("Option parsing failed: %s\n", error->message);
	exit (-1);
    }
    g_option_context_free (context);

    if (!localeStr)
	localeStr=(gchar *)localeDefault;
    if (argc<2){
	fprintf(stderr,"Specify filename of outputing schemas file!\n");
	exit (-1);
    }
    schemasFilename=argv[1];
    if (!owner){
	fprintf(stderr,"Use -w to specify owner!\n");
	exit (-1);
    }
    if (!schemasHome){
	fprintf(stderr,"Use -s to specify schemasHome!\n");
	exit (-1);
    }
    Mkdg *mDialog=mkdg_init("Hello World!", NULL);
    gint i=0;
    for(i=0; propSpecs[i].key!=NULL; i++){
	mkdg_add_property(mDialog,
		mkdg_property_context_new_full( &propSpecs[i], NULL, NULL, NULL));
    }

    if (mkdg_config_gconf_write_schemas_file(mDialog, schemasFilename, indentSpace, schemasHome, owner, localeStr, NULL)){
	exit(0);
    }
    return 1;
}

