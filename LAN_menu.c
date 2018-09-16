#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include "area1.h"
#include "mydata.h"
#include "game.h"
#include "curve.h"
#include "gui.h"
#include "utils.h"
#include "file.h"
#include "server.h"
#include "client.h"
#include "main_menu.h"

void server_dialog(GtkWindow *parent, gpointer data){
	Mydata *my = get_mydata(data);
	GtkWidget *dialog, *spin, *label, *content_area;
	GtkDialogFlags flags = GTK_DIALOG_MODAL;

	dialog = gtk_dialog_new_with_buttons ("Number of player", parent, flags, "GO", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	spin = gtk_spin_button_new_with_range (2,4,1);
	label = gtk_label_new("Enter number of player");
	
	gtk_container_add (GTK_CONTAINER (content_area), label);
	gtk_container_add (GTK_CONTAINER (content_area), spin);
	gtk_widget_show_all (dialog);
	int result = gtk_dialog_run (GTK_DIALOG (dialog));
	switch (result){
		case GTK_RESPONSE_OK: my->game.car_count=gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin));
				      break;
		default: my->game.car_count= 0;
			 break;
  	}
	gtk_widget_destroy (dialog);
}


void client_dialog(GtkWindow *parent, gpointer data){
    Mydata *my = get_mydata(data);
	
	GtkWidget *dialog, *entry, *label, *content_area;
	GtkDialogFlags flags = GTK_DIALOG_MODAL;

	dialog = gtk_dialog_new_with_buttons ("Connect to server", parent, flags, "Connect", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	entry = gtk_entry_new();
	label = gtk_label_new("Enter server IP");
	
	gtk_container_add (GTK_CONTAINER (content_area), label);
	gtk_container_add (GTK_CONTAINER (content_area), entry);
	gtk_widget_show_all (dialog);
	int result = gtk_dialog_run (GTK_DIALOG (dialog));
	switch (result){
		case GTK_RESPONSE_OK: my->server_ip = gtk_editable_get_chars(GTK_EDITABLE(entry),0,-1);
				      break;
		default: my->server_ip = NULL;
			 break;
  	}
	gtk_widget_destroy (dialog);
}

void on_button_client_clicked(GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
	
    client_dialog(GTK_WINDOW(my->window), data);
    connect_server(my->server_ip, 454545, data);
    menu_switch(data, MODE_LAN);
}

void on_button_server_clicked(GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
	load_dialog(GTK_WINDOW(my->window), data); //SELECT MAP FOR LAN
	if(strcmp(my->load_name, "DEF")) load_track(data,my->load_name);
	
	multi_dialog(GTK_WINDOW(my->window),data); //SELECT NUMBER OF PLAYER FOR LAN
	if(my->game.car_count == 0) menu_switch(data, MODE_MENU); 
    menu_switch(data, MODE_GAME);

    fork_server(data);
    gtk_widget_hide (my->frame1);
    gtk_widget_hide (my->status);
    gtk_widget_show (my->area1);
    gtk_widget_show (my->hbox1);
    gtk_widget_hide (my->vbox_menu);
    my->show_edit=FALSE;
    my->bsp_mode=BSP_DRAW;
    start_game_LAN_SRV(data,TRUE);
}

void lan_dialog(GtkWindow *parent, gpointer data){
	Mydata *my = get_mydata(data);
	GtkWidget *dialog, *button_s, *button_c, *label, *content_area;
	GtkDialogFlags flags = GTK_DIALOG_MODAL;

	dialog = gtk_dialog_new_with_buttons ("Select mode", parent, flags, "Cancel", GTK_RESPONSE_CANCEL, NULL);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	button_s = gtk_button_new_with_label("Create Server");
    button_c = gtk_button_new_with_label("Join Server");
	label = gtk_label_new("FEATURE EXPERIMENTALE NON FONCTIONNELLE");
	
	gtk_container_add (GTK_CONTAINER (content_area), label);
	gtk_container_add (GTK_CONTAINER (content_area), button_s);
	gtk_container_add (GTK_CONTAINER (content_area), button_c);

    g_signal_connect(button_s, "clicked", G_CALLBACK(on_button_server_clicked), my);
	g_signal_connect(button_c, "clicked", G_CALLBACK(on_button_client_clicked), my);

	gtk_widget_show_all (dialog);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}