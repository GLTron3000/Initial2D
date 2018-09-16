#ifndef SERVER_H
#define SERVER_H


#define NUM_PORT 50013
#define BACKLOG 50
#define NB_CLIENTS 100
#define TAILLE_BUFFER 5

typedef struct{
    gpointer data;
    int *sock_client;
}server_data;

void fork_server(gpointer data);

#endif
