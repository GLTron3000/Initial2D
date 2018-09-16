#ifndef MYDATA_H
#define MYDATA_H

#define MYDATA_MAGIC 0x46EA7E05

#include "game.h"
#include<sys/socket.h>
#include<arpa/inet.h>

enum { EDIT_NONE, EDIT_MOVE_CURVE, EDIT_ADD_CONTROL, EDIT_MOVE_CONTROL, EDIT_REMOVE_CONTROL, EDIT_LAST};

enum{ BSP_FIRST, BSP_PROLONG, BSP_DRAW, BSP_LAST};

enum{ MODE_NONE, MODE_MENU, MODE_EDITING, MODE_GAME, MODE_MULTI, MODE_LAN, MODE_LANC, MODE_LAST};

typedef struct {
	unsigned int magic;
	
	//WIN SETTINGS
	GtkWidget *window;
	char *title;
	int win_width;
	int win_height;

	//MOUSE
	double click_x;
	double click_y;
	double last_x;
	double last_y;
	int click_n;

	//LAYOUT FRAMES/BOX
	GtkWidget *area1;
	GtkWidget *vbox1;
	GtkWidget *menu_bar;
	GtkWidget *scroll;
	GtkWidget *frame1;
	GtkWidget *hbox1;
	GtkWidget *vbox_menu;
	GtkWidget *status;

	//MENUS
	GtkWidget *edit_radio[EDIT_LAST];
	GtkWidget *bsp_radios[BSP_LAST];
	int edit_mode;
	int bsp_mode;
	int game_mode;

	//VARIABLES
	Game game;
	char *save_name;
	char load_name[100];
	double rotate_angle;
	int show_edit;
	int current_control;
	int largeur_route;
	guint timeout1;
	int finish_line[CAR_MAX];
	double finish_time[CAR_MAX];
	int debug_mode;
	double start_time;
	int countdown_value;

	//FLAG KEY
	int flag_key_left, flag_key_right,  flag_key_up,  flag_key_down; //JOUEUR 1
	int flag_key_q, flag_key_d,  flag_key_z,  flag_key_s; //JOUEUR 2
	int flag_key_j, flag_key_l,  flag_key_i,  flag_key_k; //JOUEUR 3
	int flag_key_4, flag_key_6,  flag_key_8,  flag_key_5; //JOUEUR 4

	GdkPixbuf *ae86;
	GdkPixbuf *ae86red;

	//LAN
	int player_id;
	int server_ready;
	int sock;
	struct sockaddr_in server;
	char *server_ip;
	int srv_update;
	guint timeout2;
	int lan_mode;
	int get_score;
	int player_number;
	int win;
	int wait;
	GtkWidget *dialog;

} Mydata;


void init_mydata (Mydata *my);
Mydata *get_mydata (gpointer data);
void set_edit_mode (gpointer data, int mode);

#endif /* MYDATA_H */

