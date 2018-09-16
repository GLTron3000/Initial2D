#include <gtk/gtk.h>
#include <string.h>
#include "curve.h"
#include "utils.h"
#include "menus.h"
#include "area1.h"
#include "gui.h"
#include "main_menu.h"
#include "mydata.h"

void on_app_activate (GtkApplication* app, gpointer user_data){
	Mydata *my = get_mydata(user_data);
	g_object_set (gtk_settings_get_default(),"gtk-shell-shows-menubar", FALSE, NULL);
	
	window_init(app,my);
	menus_init(my);
	area1_init(my);
	status_init(my);
	editing_init(my);
	main_menu_init(my);
	layout_init(my);
	init_game(&my->game);

	//gtk_window_fullscreen(GTK_WINDOW(my->window));
	splash_screen(my);
	
	gtk_widget_show_all (my->window);

	gtk_widget_hide (my->frame1);
	gtk_widget_hide (my->status);
	gtk_widget_hide (my->area1);
	gtk_widget_hide (my->hbox1);
	gtk_widget_hide (my->menu_bar);

	
}


int main (int argc, char *argv[]){
	Mydata my;
	init_mydata(&my);

	GtkApplication *app;
	int status;

	app = gtk_application_new (NULL, G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "activate",G_CALLBACK(on_app_activate), &my);
	status = g_application_run (G_APPLICATION(app), argc, argv);
	g_object_unref (app);
	
	return status;
}

