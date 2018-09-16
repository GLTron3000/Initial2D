#include <gtk/gtk.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<pthread.h>
#include"mydata.h"
#include"server.h"

typedef struct{
	Mydata *my;
	int socket_client;
	int thread_nb;
} MyThreadData;

void *connection_handler(void *args){
	MyThreadData *mytd = args;

	int sock = mytd->socket_client;
	Mydata *my = get_mydata(mytd->my);
	
	char nb[10];
	sprintf(nb,"%d",mytd->thread_nb+1);
	printf("SRV - Thread %s lancé \n",nb);
	//PLAYER N°
	sleep(1);
	printf("SRV - Thread %s envoi OK\n",nb);
	send(sock, nb, strlen(nb),0);

	//CAR COUNT
	sleep(1);
	sprintf(nb,"%d",my->game.car_count);
	send(sock, nb, strlen(nb),0);

	while(my->server_ready == 0);

	//MAP
	sleep(1);
	printf("SRV - Thread %s envoi MAP %s \n",nb, my->load_name);
	send(sock, my->load_name, strlen(my->load_name),0);

	printf("SRV - Thread %s Launching Update Sequence %d\n",nb,my->srv_update);
	while(my->srv_update >= 0){
		printf("SRV - update 0\n");
		if(my->srv_update == 1){
			//SEND GAME STRUCT
			char msg[20];
			//Cars
			printf("SRV - Thread %s envoi de cars[]\n",nb);
			usleep(1);
			for(int i=0; i < my->game.car_count; i++){
				sprintf(msg,"SYNC"); //SYNC
				send(sock, msg, strlen(msg),0);
				usleep(10);	
				sprintf(msg,"%f",my->game.cars[i].x); //X
				send(sock, msg, strlen(msg),0);
				usleep(10);
				sprintf(msg,"%f",my->game.cars[i].y); //Y
				send(sock, msg, strlen(msg),0);
				usleep(10);
				sprintf(msg,"%f",my->game.cars[i].angle); //ANGLE
				send(sock, msg, strlen(msg),0);
				usleep(10);
				sprintf(msg,"%f",my->game.cars[i].angle_d); //ANGLE_D
				send(sock, msg, strlen(msg),0);
				usleep(10);
				sprintf(msg,"%f",my->game.cars[i].direction); //DIRECTION
				send(sock, msg, strlen(msg),0);
				usleep(10);
				sprintf(msg,"%d",my->game.cars[i].finish); //FINISH
				send(sock, msg, strlen(msg),0);
			}
			my->srv_update = 0;
		}
	}
	free(mytd);
	return 0;
}

void init_server(int nb_player, gpointer data){
	struct sockaddr_in server, client; 
	int sock_server = socket(AF_INET, SOCK_STREAM, 0);

	if(sock_server == -1) printf("SRV - Echec creation socket !\n");

	server.sin_family = AF_INET;
	server.sin_port = htons(454545);
	server.sin_addr.s_addr = INADDR_ANY;
	printf("SRV - Bind...");
	if(bind(sock_server, (struct sockaddr *) &server, sizeof(server)) < 0){
		puts("Echec\n");
		return;
	}
	printf("OK\n");

	listen(sock_server, 3);

	printf("SRV - Attente connection entrante... %d joueurs attendu \n",nb_player-1);
	int c = sizeof(struct sockaddr_in);
	int sock_client;
	int i=1;
	while((sock_client = accept(sock_server, (struct sockaddr *) &client, (socklen_t *)&c))){
		if(sock_client < 0) printf("Echec\n");

		printf("N°%d OK\n",i);
		char *client_ip = inet_ntoa(client.sin_addr);
		int client_port = ntohs(client.sin_port);
		printf("SRV - CLIENT N°%d | IP : %s | PORT : %d \n",i,client_ip,client_port);

		pthread_t sniffer_thread;
		MyThreadData *args = malloc(sizeof *args);
		args->socket_client = sock_client;
		args->my = data;
		args->thread_nb = i;
		pthread_create(&sniffer_thread, NULL, connection_handler, args);
		i++;
		if(i >= nb_player){ break;}
	}
	printf("SRV - ALL CLIENT CONNECTED\n");
}



void fork_server(gpointer data){
	Mydata *my = get_mydata(data);
	
	//pid_t p = fork();
	//if(p==0){
	init_server(my->game.car_count, data);
	//}
}
