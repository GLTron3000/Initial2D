#ifndef MAIN_MENU_H
#define MAIN_MENU_H

void start_game(gpointer data);
void reset_game(gpointer data);
void main_menu_init(gpointer data);
void menu_switch(gpointer data, int mode);
void splash_screen(gpointer data);
void multi_dialog(GtkWindow *parent, gpointer data);

#endif /* MAIN_MENU_H */
