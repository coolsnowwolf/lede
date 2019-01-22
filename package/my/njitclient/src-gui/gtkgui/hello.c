
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "hello.h"

static void SendHelloWorld(gpointer);

int main(int argc, char *argv[])
{
	GtkWidget           *win = NULL;            /* The main window */
	GtkWidget           *vbox = NULL;           /* Packing box for the menu and toolbars */
	GtkWidget           *menubar = NULL;        /* The actual menubar */
	GtkWidget           *toolbar = NULL;        /* The actual toolbar */
	GtkActionGroup      *action_group = NULL;   /* Packing group for our Actions */
	GtkUIManager        *menu_manager = NULL;   /* The magic widget! */
	GError              *error = NULL;          /* For reporting exceptions or errors */
	
	/* Initialize translation text domain */
	bindtextdomain(g_config.package_name, g_config.locale_dir);
	bind_textdomain_codeset(g_config.package_name, "UTF-8");
	textdomain(g_config.package_name);

	/* Initialize GTK+ */
	g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
	gtk_init(&argc, &argv);
	g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

	/* Create the main window */
	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(win), _("Hello World"));
	gtk_window_set_default_size(GTK_WINDOW(win), 400, 300);
	gtk_widget_realize(win);

	/* Create a vertical box to hold menubar and toolbar */
	vbox = gtk_vbox_new(FALSE, 0);

	/* Create menus */
	action_group = gtk_action_group_new("TestActions");
	gtk_action_group_set_translation_domain(action_group, g_config.package_name);
	menu_manager = gtk_ui_manager_new();

	/* Load UI from XML string */
	const gchar *UI =
		"<ui>"
		"  <menubar name='MainMenuBar'>"
		"    <menu name='FileMenu' action='FileMenuAction'>"
		"      <menuitem name='Send' action='SendAction' />"
		"      <separator/>"
		"      <menuitem name='Quit' action='QuitAction' />"
		"      <placeholder name='FileMenuAdditions' />"
		"    </menu>"
		"  </menubar>"
		""
		"  <toolbar name='MainToolbar' action='MainMenuBarAction'>"
		"    <placeholder name='ToolItems'>"
		"      <separator/>"
		"      <toolitem name='Send' action='SendAction'/>"
		"      <separator/>"
		"    </placeholder>"
		"  </toolbar>"
		"</ui>"
		;
	error = NULL;
	gtk_ui_manager_add_ui_from_string(menu_manager, UI, strlen(UI), &error);

	if (error)
	{
	    g_message("building menus failed: %s", error->message);
	    g_error_free(error);
		error = NULL;
	}


	/* Create a list of entries which are passed to the Action constructor.
	 * This is a huge convenience over building Actions by hand.
	 */
	const GtkActionEntry entries[] = {
		/**********************************/
		{ "FileMenuAction", NULL,
		  _("_File"), NULL,
		  _("File operations"),
		  NULL
		},
		/**********************************/
		{ "SendAction", GTK_STOCK_EXECUTE,
		  _("_Send"), "<Ctrl>S",
		  _("Send hello world to console"),
		  G_CALLBACK(SendHelloWorld)
		},
		/**********************************/
		{ "QuitAction", GTK_STOCK_QUIT,
		  _("_Quit"), "<Ctrl>Q",
		  _("Quit"),
		  G_CALLBACK(gtk_main_quit)
		},
		/**********************************/
	};
	/* Pack up our objects:
	 * vbox -> win
	 * actions -> action_group
	 * action_group -> menu_manager
	 */
	gtk_container_add(GTK_CONTAINER(win), vbox);
	gtk_action_group_add_actions(action_group, entries, G_N_ELEMENTS(entries), NULL);
	gtk_ui_manager_insert_action_group(menu_manager, action_group, 0);

	/* Get the menubar and the toolbar and put them in the vertical packing box */
	menubar = gtk_ui_manager_get_widget(menu_manager, "/MainMenuBar");
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
	toolbar = gtk_ui_manager_get_widget(menu_manager, "/MainToolbar");
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);


	/* Connect signals */
	g_signal_connect(win, "destroy", gtk_main_quit, NULL);

	/* Make sure that the accelerators work */
	gtk_window_add_accel_group(GTK_WINDOW(win), 
	                           gtk_ui_manager_get_accel_group(menu_manager));

	/* Enter the main loop */
	gtk_widget_show_all(win);
	gtk_main();
	return (0);
}

static void SendHelloWorld(gpointer p)
{
	(void) p;
	g_print("%s\n", _("Hello world!"));
}
