#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include<unistd.h>
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
#include "client.h"
#include "LANC.h"

#define PORT 128456

typedef struct{
	Mydata *my;
	int socket_client;
	int thread_nb;
} MyThreadData;

void end_dialog(GtkWindow *parent, char *reply){
	GtkWidget *dialog, *content_area;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons ("Finito", parent, flags, "Ok", GTK_RESPONSE_NONE, NULL);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

	GtkWidget *label = gtk_label_new(reply);
	gtk_container_add (GTK_CONTAINER (content_area), label);
	gtk_widget_show_all (dialog);
	
	g_signal_connect_swapped (dialog,"response",G_CALLBACK (gtk_widget_destroy),dialog);

}

void wait_score_LANC(gpointer data){
	Mydata *my = get_mydata(data);
	my->wait = 1;
	refresh_area(my->area1);
	while(my->get_score != my->player_number-1){usleep(1);}
	my->server_ready = 1;
	sleep(1);
	my->wait = 0;
	if(my->win == 1)end_dialog(GTK_WINDOW(my->window),"YOU LOSE");
	else end_dialog(GTK_WINDOW(my->window),"YOU WIN");
}

void send_score_LANC(gpointer data){
	Mydata *my = get_mydata(data);
	double f_time = my->finish_time[0]-my->start_time;
	char score[20];
	sprintf(score,"%f",f_time);
	send(my->sock, score, strlen(score),0);
	my->wait = 1;

	char reply[10];
	refresh_area(my->area1);
	while(recv(my->sock, &reply, 20,0) == 0);
	my->wait = 0;
	reply[strlen(reply)-1] = 0; //REMOVE %
	end_dialog(GTK_WINDOW(my->window),reply);
}

void *connection_handler_LANC(void *args){
	MyThreadData *mytd = args;

	int sock = mytd->socket_client;
	Mydata *my = get_mydata(mytd->my);

	char nb[50];
	//PLAYER N°
	sleep(1);
	sprintf(nb,"%d",mytd->thread_nb+1);
	send(sock, nb, strlen(nb),0);

	//MAP
	sleep(1);
	sprintf(nb,"%s",my->load_name);
	send(sock, nb, strlen(nb),0);

	//WAIT FOR TIME
	char reply[10];
	while(recv(sock, &reply, 20,0) == 0);
	double score = atoi(reply);

	my->get_score++;
	//WAIT FOR ALL TO FINISH
	while(my->server_ready == 0){usleep(1);}

	if(score < my->finish_time[0]-my->start_time){
		sprintf(nb,"YOU WIN");
		printf("SEND WIN\n");
		my->win = 1;
	}
	else{sprintf(nb,"YOU LOSE"); printf("SEND LOSE\n"); }
	send(sock, nb, strlen(nb),0);

	free(mytd);
	return 0;
}

void init_server_LANC(int nb_player, gpointer data){
	struct sockaddr_in server, client; 
	int sock_server = socket(AF_INET, SOCK_STREAM, 0);

	if(sock_server == -1) printf("SRV - Echec creation socket !\n");

	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = INADDR_ANY;
	printf("SRV - Bind...");
	if(bind(sock_server, (struct sockaddr *) &server, sizeof(server)) < 0){
		puts("Echec\n");
		return;
	}
	printf("OK\n");

	listen(sock_server, 3);

	int c = sizeof(struct sockaddr_in);
	int sock_client;
	int i=1;
	while((sock_client = accept(sock_server, (struct sockaddr *) &client, (socklen_t *)&c))){
		if(sock_client < 0) printf("Echec\n");

		printf("N°%d OK\n",i);
		pthread_t sniffer_thread;
		MyThreadData *args = malloc(sizeof *args);
		args->socket_client = sock_client;
		args->my = data;
		args->thread_nb = i;
		pthread_create(&sniffer_thread, NULL, connection_handler_LANC, args);
		i++;
		if(i >= nb_player){ break;}
	}
}

void clientC_dialog(GtkWindow *parent, gpointer data){
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

void on_button_clientC_clicked(GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
	
    clientC_dialog(GTK_WINDOW(my->window), data);
    connect_server(my->server_ip, PORT, data);
	my->wait = 1;
	refresh_area(my->area1);

	char reply[10];
	while(recv(my->sock, &reply, 20,0) == 0); //WAIT FOR PLAYER ID
	int nb = atoi(reply);
	my->player_id=nb-1;
	printf("START GAME LAN CHRONO => Player N°%d | Waiting for map... \n",nb);
    char map_name[100];
    while(recv(my->sock, map_name, 100,0) == 0){} //WAIT FOR MAP

	map_name[strlen(map_name)-1] = 0; //REMOVE %
    printf("OK MAP: -%s-\n",map_name);
    load_track(data,map_name);

	my->game.car_count=1;
	my->lan_mode=1;
	gtk_widget_destroy (my->dialog);
	my->wait = 0;
	menu_switch(data, MODE_GAME);
}

void on_button_serverC_clicked(GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
	load_dialog(GTK_WINDOW(my->window), data); //SELECT MAP FOR LAN
	if(strcmp(my->load_name, "DEF")) load_track(data,my->load_name);
	else return;
	
	/*multi_dialog(GTK_WINDOW(my->window),data); //SELECT NUMBER OF PLAYER FOR LAN
	if(my->game.car_count == 0) menu_switch(data, MODE_MENU);
	*/

	my->wait = 1;
	refresh_area(my->area1);
	
	init_server_LANC(2,data);
	my->player_number = 2;
	my->game.car_count = 1;
	my->lan_mode=1;
	my->player_id=0;
	my->wait = 0;
	gtk_widget_destroy (my->dialog);
	menu_switch(data, MODE_GAME);
}

void lanC_dialog(GtkWindow *parent, gpointer data){
	Mydata *my = get_mydata(data);
	GtkWidget *button_s, *button_c, *content_area;
	GtkDialogFlags flags = GTK_DIALOG_MODAL;

	my->dialog = gtk_dialog_new_with_buttons ("Select mode", parent, flags, "Cancel", GTK_RESPONSE_CANCEL, NULL);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (my->dialog));
	button_s = gtk_button_new_with_label("Create Server");
    button_c = gtk_button_new_with_label("Join Server");
	
	gtk_container_add (GTK_CONTAINER (content_area), button_s);
	gtk_container_add (GTK_CONTAINER (content_area), button_c);

    g_signal_connect(button_s, "clicked", G_CALLBACK(on_button_serverC_clicked), my);
	g_signal_connect(button_c, "clicked", G_CALLBACK(on_button_clientC_clicked), my);

	gtk_widget_show_all (my->dialog);
	gtk_dialog_run (GTK_DIALOG (my->dialog));
}