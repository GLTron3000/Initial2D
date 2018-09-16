#include <gtk/gtk.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<pthread.h>
#include"mydata.h"
#include"client.h"
#include"utils.h"
#include"file.h"
#include"area1.h"

gboolean on_timeout2(gpointer data){
	Mydata *my = get_mydata(data);
	printf("SRV_UPDATE 1\n");
	my->srv_update = 1;
	return TRUE;
}

void next_step_LAN(gpointer data){
	Mydata *my = get_mydata(data);
	
	char reply[20];
	if(my->player_id != 0){
		printf("CLIENT UPDATE \n");
		for(int i=0; i < my->game.car_count; i++){
			while(recv(my->sock, &reply, 20,0) == 0);//SYNC
			if(strcmp(reply,"SYNC")){
				recv(my->sock, &reply, 20,0);//X
				my->game.cars[i].x = atoi(reply);
				recv(my->sock, &reply, 20,0);//Y
				my->game.cars[i].y = atoi(reply);
				recv(my->sock, &reply, 20,0);//ANGLE
				my->game.cars[i].angle = atoi(reply);
				recv(my->sock, &reply, 20,0);//ANGLE_D
				my->game.cars[i].angle_d = atoi(reply);
				recv(my->sock, &reply, 20,0);//DIRECTION
				my->game.cars[i].direction = atoi(reply);
				recv(my->sock, &reply, 20,0);//FINISH
				my->game.cars[i].finish = atoi(reply);
				printf("CARS 0 | x:%f y:%f angle:%f angle_d:%f direction:%f finish:%d\n",my->game.cars[0].x,my->game.cars[0].y,my->game.cars[0].angle,my->game.cars[0].angle_d,my->game.cars[0].direction,my->game.cars[0].finish);
			}else usleep(50);
		}
		printf("CLIENT UPDATE END\n");
	}else if(my->player_id == 0){
		progress_game_next_step(&my->game);
	}
}

void connect_server(char *IP, int port, gpointer data){
    Mydata *my = get_mydata(data);

	struct sockaddr_in server; 
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	if(sock == -1) printf("Echec creation socket !\n");

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr(IP);

	printf("Tentative connection...");
	if(connect(sock, (struct sockaddr*) &server, sizeof(server)) < 0){
		puts("Echec\n");
		return;
	}
	printf("OK\n");

    my->server = server;
    my->sock = sock;
}

gboolean countdown_LAN(gpointer data){
	Mydata *my = get_mydata(data);
	
	if(my->countdown_value == 0){
		printf("GO!\n");
		my->countdown_value--;
		my->start_time=clock();
		return(FALSE);
	}else if(my->countdown_value < 0) return FALSE;
	else{
		printf("%d\n",my->countdown_value);
		my->countdown_value--;
		return TRUE;
	}
	return FALSE;
}

void start_game_LAN(gpointer data, int mode_game){
    Mydata *my = get_mydata(data);
	Game *game = &my->game;
    char reply[10];
	while(recv(my->sock, &reply, 20,0) == 0);
	int nb = atoi(reply);
	while(recv(my->sock, &reply, 20,0) == 0);
	my->game.car_count = atoi(reply);
	my->player_id=nb-1;
	printf("START GAME LAN\n Player N°%d Of %d | Waiting for ready signal...",nb,my->game.car_count );
    char map_name[100];
    while(recv(my->sock, map_name, 100,0) == 0);
    printf("OK MAP: -%s-\n",map_name);
    load_track(data,map_name);

	if (mode_game) {
        compute_road_tracks (&game->road,0.1);
		place_cars_on_start (game);
		set_anim1_mode (my, TRUE);
		my->countdown_value=3;
		g_timeout_add (1000, countdown_LAN, my);
		refresh_area (my->area1);
	} else {
		set_anim1_mode (my, FALSE);
	}
	refresh_area (my->area1);
}

void start_game_LAN_SRV(gpointer data, int mode_game){
    Mydata *my = get_mydata(data);
	Game *game = &my->game;

	my->server_ready=1;
	my->player_id=0;
    load_track(data, my->load_name);
    printf("All player Connected - Launching game...\n");

	my->timeout2 = g_timeout_add(1000, on_timeout2, my);

	if (mode_game) {
        compute_road_tracks (&game->road,0.1);
		place_cars_on_start (game);
		set_anim1_mode (my, TRUE);
		my->countdown_value=10;
		g_timeout_add (1000, countdown_LAN, my);
		refresh_area (my->area1);
	} else {
		set_anim1_mode (my, FALSE);
	}
	refresh_area (my->area1);
}