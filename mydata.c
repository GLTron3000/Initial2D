#include <gtk/gtk.h>
#include <stdio.h>
#include "curve.h"
#include "mydata.h"

void init_mydata (Mydata *my){
	my->title = "Initial2D";
	my->win_width = 1000;
	my->win_height = 800;
	my->magic = MYDATA_MAGIC;
	my->save_name = NULL;	
	my->click_x=0;
	my->click_y=0;
	my->click_n=0;
	my->show_edit=FALSE;
	my->edit_mode=EDIT_ADD_CONTROL;
	my->bsp_mode=BSP_PROLONG;
	my->game_mode=MODE_MENU;
	my->current_control=0;
	init_game (&my->game);
	my->largeur_route=100;
	my->timeout1 = 0;

	//INIT KEY JOUEURS
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

	my->debug_mode = 0;
	my->start_time = 0;
	my->countdown_value = 3;	
	
	for(int i=0; i< CAR_MAX; i++){
		my->finish_line[i]=-1;
		my->finish_time[i]=-1;
	}

	//LAN
	my->player_id = -1;	
	my->server_ready = 0;
	my->sock = 0;
	my->srv_update = 0;
	my->timeout2 = 0;
	my->lan_mode = 0;
	my->get_score = 0;
	my->player_number = 0;
	my->win = 0;
	my->wait = 0;
}


// Cette fonction permet de tester si le data que l’on a recuper ́e dans
// une callback contient bien my ; sinon, cela veut dire que :
//  - soit on a oubli ́e de transmettre my dans g_signal_connect,
//  - soit on s’est tromp ́e dans le nombre de param`etres de la callback.
Mydata *get_mydata (gpointer data){
	if (data == NULL) {
		fprintf (stderr, "get_mydata: NULL data\n"); 
		return NULL;
	}
	if (((Mydata *)data)->magic != MYDATA_MAGIC) {
		fprintf (stderr, "get_mydata: bad magic number\n"); 
		return NULL;
	}
	return data;
}

void set_edit_mode (gpointer data, int mode){
	Mydata *my = get_mydata(data);
	if(mode<1 ||mode>7){return;}
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(my->edit_radio[mode-1]),TRUE);
	
}
