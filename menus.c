#include <gtk/gtk.h>
#include "curve.h"
#include "area1.h"
#include "utils.h"
#include "mydata.h"
#include "menus.h"

/*
void menu_mode(gpointer data){
	Mydata *my = get_mydata(data);
        Game *game = &my->game;

        mode_jeu_active = TRUE;
        if (mode_jeu_active) {

            // Ces 3 fonctions Ã  implÃ©menter
            //copy_curve_infos_to_road (&my->curve_infos, &game->road);
            compute_road_tracks (&game->road,0.2);
            place_cars_on_start (game);

            set_anim1_mode (my, TRUE);
        } else {
            set_anim1_mode (my, FALSE);
        }
        refresh_area (my->area1);
}
*/

void on_item_quit_activate (GtkWidget *widget, gpointer data){
	Mydata *my = get_mydata(data);
	printf ("Closing window and app\n");
	gtk_widget_destroy(my->window);
}


void on_item_about_activate (GtkWidget *widget, gpointer data){
	const gchar *authors[] = { "A","B", NULL };
	Mydata *my = get_mydata(data);	
	gtk_show_about_dialog (NULL,"authors",authors,"program-name",my->title,"version","2.4","website","http://j.mp/optigra","logo-icon-name","face-cool",NULL);
}

void on_item_editing_activate (GtkWidget *widget, gpointer data){
	Mydata *my = get_mydata(data);
	my->show_edit= gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
	if(my->show_edit){
		set_status(my->status,"Editing is on");
		gtk_widget_show(my->frame1);
	}else{
		set_status(my->status,"Editing is off");
		gtk_widget_hide(my->frame1);
	}
}

void menus_init(gpointer data){
	Mydata *my = get_mydata(data);

	my->menu_bar = gtk_menu_bar_new();
	//Items
	GtkWidget *item_tools, *item_file, *item_help, *item_quit, *item_about, *item_editing;
	//Sub Items
	GtkWidget *sub_file, *sub_tools, *sub_help;
		
	//Menu Item
	item_file = gtk_menu_item_new_with_label("File");
	item_tools = gtk_menu_item_new_with_label("Tools");
	item_help = gtk_menu_item_new_with_label("Help");
	
	gtk_menu_shell_append(GTK_MENU_SHELL (my->menu_bar), item_file);
	gtk_menu_shell_append(GTK_MENU_SHELL (my->menu_bar), item_tools);
	gtk_menu_shell_append(GTK_MENU_SHELL (my->menu_bar), item_help);
	
	//Sub File
	sub_file=gtk_menu_new();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(item_file), sub_file);
	
	//Sub Tools
	sub_tools=gtk_menu_new();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(item_tools), sub_tools);
	
	//Sub Help
	sub_help=gtk_menu_new();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(item_help), sub_help);
	

	//Sub File Menu Items
	item_quit = gtk_menu_item_new_with_label("Quit");
	gtk_menu_shell_append(GTK_MENU_SHELL (sub_file), item_quit);
	
	//Sub Tools Menu Items
	item_editing = gtk_check_menu_item_new_with_label("Editing");
	gtk_menu_shell_append(GTK_MENU_SHELL (sub_tools), item_editing);
	
	//Sub Help Menu Items
	item_about = gtk_menu_item_new_with_label("About");
	gtk_menu_shell_append(GTK_MENU_SHELL (sub_help), item_about);
	
	//Signal
	g_signal_connect(item_quit, "activate", G_CALLBACK(on_item_quit_activate), my);
	g_signal_connect(item_about, "activate", G_CALLBACK(on_item_about_activate), my);
	g_signal_connect(item_editing, "activate", G_CALLBACK(on_item_editing_activate), my);
}
