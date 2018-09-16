#ifndef CLIENT_H
#define CLIENT_H

void connect_server(char *IP, int port, gpointer data);
void start_game_LAN(gpointer data, int mode_game);
void start_game_LAN_SRV(gpointer data, int mode_game);
void next_step_LAN(gpointer data);

#endif