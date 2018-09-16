#include <gtk/gtk.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include "curve.h"
#include "area1.h"
#include "utils.h"
#include "mydata.h"
#include "game.h"
#include "main_menu.h"
#include "gui.h"
#include "file.h"

void on_button_load_clicked(GtkWidget *widget, gpointer data){
	Mydata *my = get_mydata(data);
	load_dialog(GTK_WINDOW(my->window), data);
	if(strcmp(my->load_name, "DEF")) load_track(data,my->load_name);
}

void on_button_save_clicked(GtkWidget *widget, gpointer data){
	Mydata *my = get_mydata(data);
	save_dialog(GTK_WINDOW(my->window), data);
	if(my->save_name != NULL) save_track(data,my->save_name);
}

void on_button_back_clicked(GtkWidget *widget, gpointer data){
	menu_switch(data, MODE_MENU);
}

void on_button_start_clicked(GtkWidget *widget, gpointer data){
	start_game(data);
}

void on_edit_radio_toggled (GtkWidget *widget, gpointer data){
	Mydata *my = get_mydata(data);
	my->edit_mode=GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget),"Key"));
	printf("Edit_mode %d\n",my->edit_mode);

}

void on_bsp_radio_toggled(GtkWidget *widget, gpointer data){
	Mydata *my = get_mydata(data);
	my->bsp_mode=GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget),"mode"));
	printf("bsp_mode %d\n",my->bsp_mode);
	refresh_area(my->area1);
}

void on_button_reset_clicked(GtkWidget *widget, gpointer data){
	Mydata *my = get_mydata(data);
	printf("RESET\n");
	remove_curve(&my->game.road.curve_central);
	remove_curve(&my->game.road.curve_right);
	remove_curve(&my->game.road.curve_left);
	refresh_area(my->area1);
}

//FONCTIONS INIT
void window_init(GtkApplication* app, gpointer data){
	Mydata *my = get_mydata(data);
	my->window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW(my->window), my->title);
	gtk_window_set_default_size (GTK_WINDOW(my->window),my->win_width, my->win_height);
}

void layout_init(gpointer data){
	Mydata *my = get_mydata(data);
	
	//EDITING + GAME
	my->vbox1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
	my->scroll = gtk_scrolled_window_new(NULL,NULL);
	my->hbox1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	
	gtk_container_add (GTK_CONTAINER (my->window), my->vbox1);
	gtk_box_pack_start (GTK_BOX(my->vbox1), my->menu_bar, FALSE, FALSE, 0); //INUTILE ?
	gtk_box_pack_start (GTK_BOX (my->vbox1), my->hbox1, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (my->vbox1), my->vbox_menu, TRUE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (my->hbox1), my->frame1, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (my->hbox1), my->scroll, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (my->vbox1), my->status, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER (my->scroll), my->area1);

}


void status_init(gpointer data){
	Mydata *my = get_mydata(data);
	
	my->status = gtk_statusbar_new ();
	set_status(my->status,"Initial2D Starto");
}

void editing_init(gpointer data){
	Mydata *my = get_mydata(data);
	
	my->frame1 = gtk_frame_new("Editing");
	
	//EDIT MODE
	my->edit_radio[0] = gtk_radio_button_new_with_label(NULL,"Move curve");
	my->edit_radio[1] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (my->edit_radio[0]),"Add control");
	my->edit_radio[2] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (my->edit_radio[0]),"Move control");
	my->edit_radio[3] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (my->edit_radio[0]),"Remove control");
	
	//BSP
	my->bsp_radios[0] = gtk_radio_button_new_with_label_from_widget(NULL,"Prolongated");
	my->bsp_radios[1] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (my->bsp_radios[0]),"Draw");
	
	GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	GtkWidget *sep2 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	GtkWidget *sep3 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
	GtkWidget *button = gtk_button_new_with_label("Reset");
	GtkWidget *button2 = gtk_button_new_with_label("Start");
	GtkWidget *button3 = gtk_button_new_with_label("Back to menu");
	GtkWidget *button4 = gtk_button_new_with_label("Save");
	GtkWidget *button5 = gtk_button_new_with_label("Load");

	gtk_container_add(GTK_CONTAINER (my->frame1), vbox);
	gtk_box_pack_start(GTK_BOX (vbox), button3, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vbox), sep, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vbox), my->edit_radio[0], FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vbox), my->edit_radio[1], FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vbox), my->edit_radio[2], FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vbox), my->edit_radio[3], FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vbox), sep2, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vbox), my->bsp_radios[0], FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vbox), my->bsp_radios[1], FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vbox), sep3, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vbox), button, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vbox), button2, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vbox), button4, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vbox), button5, FALSE, FALSE, 0);

	//EDIT MODE
	g_object_set_data(G_OBJECT(my->edit_radio[0]),"Key",GINT_TO_POINTER(EDIT_MOVE_CURVE));
	g_object_set_data(G_OBJECT(my->edit_radio[1]),"Key",GINT_TO_POINTER(EDIT_ADD_CONTROL));
	g_object_set_data(G_OBJECT(my->edit_radio[2]),"Key",GINT_TO_POINTER(EDIT_MOVE_CONTROL));
	g_object_set_data(G_OBJECT(my->edit_radio[3]),"Key",GINT_TO_POINTER(EDIT_REMOVE_CONTROL));
	
	//BSP
	g_object_set_data(G_OBJECT(my->bsp_radios[0]),"mode",GINT_TO_POINTER(BSP_PROLONG));
	g_object_set_data(G_OBJECT(my->bsp_radios[1]),"mode",GINT_TO_POINTER(BSP_DRAW));
	
	//EDIT MODE
	g_signal_connect(my->edit_radio[0], "toggled", G_CALLBACK(on_edit_radio_toggled), my);
	g_signal_connect(my->edit_radio[1], "toggled", G_CALLBACK(on_edit_radio_toggled), my);
	g_signal_connect(my->edit_radio[2], "toggled", G_CALLBACK(on_edit_radio_toggled), my);
	g_signal_connect(my->edit_radio[3], "toggled", G_CALLBACK(on_edit_radio_toggled), my);
	
	//BSP
	g_signal_connect(my->bsp_radios[0], "toggled", G_CALLBACK(on_bsp_radio_toggled), my);
	g_signal_connect(my->bsp_radios[1], "toggled", G_CALLBACK(on_bsp_radio_toggled), my);

	g_signal_connect(button, "clicked", G_CALLBACK(on_button_reset_clicked), my);
	g_signal_connect(button2, "clicked", G_CALLBACK(on_button_start_clicked), my);	
	g_signal_connect(button3, "clicked", G_CALLBACK(on_button_back_clicked), my);
	g_signal_connect(button4, "clicked", G_CALLBACK(on_button_save_clicked), my);
	g_signal_connect(button5, "clicked", G_CALLBACK(on_button_load_clicked), my);		
}
