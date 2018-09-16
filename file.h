#ifndef FILE_H
#define FILE_H

void save_track(gpointer data,char* filename);
void load_track(gpointer data, char * filename);
void load_dialog(GtkWindow *parent, gpointer data);
void save_dialog(GtkWindow *parent, gpointer data);

#endif /* FILE_H */
