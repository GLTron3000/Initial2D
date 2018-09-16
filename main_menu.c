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
#include "LAN_menu.h"
#include "LANC.h"

void reset_game(gpointer data){
	Mydata *my = get_mydata(data);
	
	remove_curve(&my->game.road.curve_central);
	remove_curve(&my->game.road.curve_right);
	remove_curve(&my->game.road.curve_left);

	set_anim1_mode(my, FALSE);
	
	my->game.car_count=0;
	for(int i=0; i< CAR_MAX; i++){
		my->finish_line[i]=-1;
		my->game.cars[i].finish=0;
		my->game.cars[i].speed=0;
	}
	
	my->flag_key_left = 0;
	my->flag_key_right = 0;
	my->flag_key_up = 0;
	my->flag_key_down = 0;

	my->flag_key_z = 0;
	my->flag_key_s = 0;
	my->flag_key_q = 0;
	my->flag_key_d = 0;

	my->flag_key_j = 0;
	my->flag_key_k = 0;
	my->flag_key_l = 0;
	my->flag_key_i = 0;

	my->flag_key_8 = 0;
	my->flag_key_4 = 0;
	my->flag_key_5 = 0;
	my->flag_key_6 = 0;

	my->lan_mode = 0;
}

gboolean splash_close(gpointer data){
	gtk_widget_destroy((GtkWidget*)data);
	gtk_main_quit ();
	printf("SPLASH CLOSE \n");
	return(FALSE);
}


void splash_screen(gpointer data){
	Mydata *my = get_mydata(data);
	int time=1000;
	GtkWidget *window;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gtk_widget_set_size_request (window, my->win_width, my->win_height);
	gtk_window_set_decorated(GTK_WINDOW (window), FALSE);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER_ALWAYS);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_window_fullscreen(GTK_WINDOW(window));

	GdkPixbufAnimation *animation;
	animation = gdk_pixbuf_animation_new_from_file("assets/madbox.gif", NULL);
	GtkWidget *image;
	image = gtk_image_new_from_animation(animation);

	gtk_container_add(GTK_CONTAINER(window), image);

	gtk_widget_show_all (window);
	g_timeout_add (time, splash_close, window);

	gtk_main ();
}


gboolean countdown(gpointer data){
	Mydata *my = get_mydata(data);
	
	if(my->countdown_value == 0){
		printf("GO!\n");
		my->countdown_value--;
		my->start_time=clock();
		return(FALSE);
	}
	else printf("%d\n",my->countdown_value);
	my->countdown_value--;
	return TRUE;	
}

void start_game(gpointer data, int mode_game){
	Mydata *my = get_mydata(data);
	Game *game = &my->game;
	printf("START GAME\n");
	if (mode_game) {
		compute_road_tracks (&game->road,0.1);
		place_cars_on_start (game);
		set_anim1_mode (my, TRUE);
		my->countdown_value=3;
		g_timeout_add (1000, countdown, my);
		refresh_area (my->area1);
	} else {
		set_anim1_mode (my, FALSE);
	}
	refresh_area (my->area1);
}

void menu_switch(gpointer data, int mode){
	Mydata *my = get_mydata(data);
	my->game_mode = mode;
	switch(mode){
		case MODE_EDITING :	gtk_widget_show (my->frame1);
					gtk_widget_show (my->status);
					gtk_widget_show (my->area1);
					gtk_widget_show (my->hbox1);
					gtk_widget_hide (my->vbox_menu);
		            my->show_edit=TRUE;
					my->bsp_mode=BSP_PROLONG;
					break;
		case MODE_GAME : gtk_widget_hide (my->frame1);
				gtk_widget_hide (my->status);
				gtk_widget_show (my->area1);
				gtk_widget_show (my->hbox1);
				gtk_widget_hide (my->vbox_menu);
				my->show_edit=FALSE;
				my->bsp_mode=BSP_DRAW;
				start_game(data,TRUE);
				break;
		case MODE_LAN : gtk_widget_hide (my->frame1);
				gtk_widget_hide (my->status);
				gtk_widget_show (my->area1);
				gtk_widget_show (my->hbox1);
				gtk_widget_hide (my->vbox_menu);
				my->show_edit=FALSE;
				my->bsp_mode=BSP_DRAW;
				start_game_LAN(data,TRUE);
				break;
		case MODE_MENU :gtk_widget_hide (my->frame1);
				gtk_widget_hide (my->status);
				gtk_widget_hide (my->area1);
				gtk_widget_hide (my->hbox1);
				gtk_widget_show (my->vbox_menu);
				my->show_edit=FALSE;
				start_game(data,FALSE);
				break;
	}
}

void multi_dialog(GtkWindow *parent, gpointer data){
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

void options_dialog(GtkWindow *parent, gpointer data){
	Mydata *my = get_mydata(data);
	GtkWidget *dialog, *label_title, *label_drift, *label_debug, *label_colision, *label_trail, *content_area, *hbox1, *hbox2, *hbox3, *hbox4, *drift_switch, *debug_switch, *col_switch, *trail_switch;
	GtkDialogFlags flags = GTK_DIALOG_MODAL;

	dialog = gtk_dialog_new_with_buttons ("Options", parent, flags, "Retour", GTK_RESPONSE_OK, NULL);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

	label_title = gtk_label_new("Options");	
	label_drift = gtk_label_new("Drift Mode");	
	label_debug = gtk_label_new("Debug Mode");
	label_colision = gtk_label_new("Colision voitures");
	label_trail = gtk_label_new("Trails effect");	

	hbox1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	hbox2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	hbox3 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	hbox4 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);

	drift_switch=gtk_switch_new();
	debug_switch=gtk_switch_new();
	col_switch=gtk_switch_new();
	trail_switch=gtk_switch_new();

	if(my->debug_mode) gtk_switch_set_active(GTK_SWITCH(debug_switch),TRUE);
	if(my->game.drift_mode) gtk_switch_set_active(GTK_SWITCH(drift_switch),TRUE);
	if(my->game.colision) gtk_switch_set_active(GTK_SWITCH(col_switch),TRUE);
	if(my->game.trail) gtk_switch_set_active(GTK_SWITCH(trail_switch),TRUE);
	
	gtk_container_add (GTK_CONTAINER (content_area), label_title);
	gtk_container_add (GTK_CONTAINER (content_area), hbox1);
	gtk_container_add (GTK_CONTAINER (content_area), hbox2);
	gtk_container_add (GTK_CONTAINER (content_area), hbox3);
	gtk_container_add (GTK_CONTAINER (content_area), hbox4);

	gtk_container_add (GTK_CONTAINER (hbox1), label_drift);
	gtk_container_add (GTK_CONTAINER (hbox1), drift_switch);
	gtk_container_add (GTK_CONTAINER (hbox2),label_debug);
	gtk_container_add (GTK_CONTAINER (hbox2), debug_switch);
	gtk_container_add (GTK_CONTAINER (hbox3),label_colision);
	gtk_container_add (GTK_CONTAINER (hbox3), col_switch);
	gtk_container_add (GTK_CONTAINER (hbox4),label_trail);
	gtk_container_add (GTK_CONTAINER (hbox4), trail_switch);

	gtk_widget_show_all (dialog);
	int result = gtk_dialog_run (GTK_DIALOG (dialog));
	switch (result){
		case GTK_RESPONSE_OK: my->debug_mode= gtk_switch_get_active(GTK_SWITCH(debug_switch));
				      my->game.drift_mode = gtk_switch_get_active(GTK_SWITCH(drift_switch));
				      my->game.colision = gtk_switch_get_active(GTK_SWITCH(col_switch));
					  my->game.trail = gtk_switch_get_active(GTK_SWITCH(trail_switch));
				      break;
		default: my->game.car_count= 0;
			 break;
  	}
	gtk_widget_destroy (dialog);
}

void on_button_options_clicked(GtkWidget *widget, gpointer data){
	Mydata *my = get_mydata(data);
	printf("MENU OPTIONS \n");
	options_dialog(GTK_WINDOW(my->window),data);
}

void on_button_lan_clicked(GtkWidget *widget, gpointer data){
	Mydata *my = get_mydata(data);
	printf("MENU LAN \n");
	lan_dialog(GTK_WINDOW(my->window), data);
}

void on_button_lanC_clicked(GtkWidget *widget, gpointer data){
	Mydata *my = get_mydata(data);
	printf("MENU LAN CHRONO \n");
	lanC_dialog(GTK_WINDOW(my->window), data);
}

void on_button_multi_clicked(GtkWidget *widget, gpointer data){
	Mydata *my = get_mydata(data);
	printf("MODE MULTI \n");
	load_dialog(GTK_WINDOW(my->window), data);
	if(strcmp(my->load_name, "DEF")) load_track(data,my->load_name);
	else return;
	multi_dialog(GTK_WINDOW(my->window),data);
	if(my->game.car_count == 0) menu_switch(data, MODE_MENU); 
    menu_switch(data, MODE_GAME);
}

void on_button_edit_clicked(GtkWidget *widget, gpointer data){
	Mydata *my = get_mydata(data);
	printf("MODE EDIT \n");
	remove_curve(&my->game.road.curve_central);
	remove_curve(&my->game.road.curve_right);
	remove_curve(&my->game.road.curve_left);
	my->game.car_count=0;
    	menu_switch(data, MODE_EDITING);
}

void on_button_quit_clicked(GtkWidget *widget, gpointer data){
    	Mydata *my = get_mydata(data);
	printf ("Closing game\n");
	gtk_widget_destroy(my->window);
}

void on_button_solo_clicked(GtkWidget *widget, gpointer data){
	Mydata *my = get_mydata(data);
	printf("MODE SOLO\n");
	load_dialog(GTK_WINDOW(my->window), data);
	if(strcmp(my->load_name, "DEF")) load_track(data,my->load_name);
	else return;
	my->game.car_count=1;
   	menu_switch(data, MODE_GAME);
}


void main_menu_init(gpointer data){
	Mydata *my = get_mydata(data);

	my->vbox_menu = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);

	GtkWidget *logo = gtk_image_new_from_file("assets/logo.png");
	GtkWidget *button_solo = gtk_button_new_with_label("SOLO");
	GtkWidget *button_multi = gtk_button_new_with_label("MULTI");
	GtkWidget *button_lan = gtk_button_new_with_label("LAN");
	GtkWidget *button_lanC = gtk_button_new_with_label("LAN CHRONO");	
	GtkWidget *button_edit = gtk_button_new_with_label("EDITOR");
	GtkWidget *button_options = gtk_button_new_with_label("OPTIONS");	
	GtkWidget *button_quit = gtk_button_new_with_label("QUIT");

	gtk_button_set_relief (GTK_BUTTON(button_solo), GTK_RELIEF_NONE);
	gtk_button_set_relief (GTK_BUTTON(button_multi), GTK_RELIEF_NONE);
	gtk_button_set_relief (GTK_BUTTON(button_lan), GTK_RELIEF_NONE);
	gtk_button_set_relief (GTK_BUTTON(button_lanC), GTK_RELIEF_NONE);
	gtk_button_set_relief (GTK_BUTTON(button_edit), GTK_RELIEF_NONE);
	gtk_button_set_relief (GTK_BUTTON(button_options), GTK_RELIEF_NONE);
	gtk_button_set_relief (GTK_BUTTON(button_quit), GTK_RELIEF_NONE);

	gtk_box_pack_start(GTK_BOX (my->vbox_menu), logo, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (my->vbox_menu), button_solo, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (my->vbox_menu), button_multi, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (my->vbox_menu), button_lan, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (my->vbox_menu), button_lanC, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (my->vbox_menu), button_edit, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (my->vbox_menu), button_options, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (my->vbox_menu), button_quit, FALSE, FALSE, 0);

	g_signal_connect(button_solo, "clicked", G_CALLBACK(on_button_solo_clicked), my);
	g_signal_connect(button_multi, "clicked", G_CALLBACK(on_button_multi_clicked), my);
	g_signal_connect(button_lan, "clicked", G_CALLBACK(on_button_lan_clicked), my);
	g_signal_connect(button_lanC, "clicked", G_CALLBACK(on_button_lanC_clicked), my);	
	g_signal_connect(button_edit, "clicked", G_CALLBACK(on_button_edit_clicked), my);
	g_signal_connect(button_options, "clicked", G_CALLBACK(on_button_options_clicked), my);
    g_signal_connect(button_quit, "clicked", G_CALLBACK(on_button_quit_clicked), my);
}
