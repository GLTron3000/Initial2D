#include <gtk/gtk.h>
#include "mydata.h"
#include "game.h"
#include "area1.h"
#include "curve.h"
#include "utils.h"
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

void load_dialog(GtkWindow *parent, gpointer data){
	//DIALOG BOX
	Mydata *my = get_mydata(data);
	
	GtkWidget *dialog, *label, *content_area, *list;
	GtkDialogFlags flags = GTK_DIALOG_MODAL;

	dialog = gtk_dialog_new_with_buttons ("Save yo track", parent, flags, "Load", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	label = gtk_label_new("Select map");
	list = gtk_list_box_new();

	gtk_container_add (GTK_CONTAINER (content_area), label);
	gtk_container_add (GTK_CONTAINER (content_area), list);
	

	//LISTE DIR + MAP	
	DIR *dp;
	struct dirent *ep;
	int i=0;
	dp = opendir ("save/");
	if (dp != NULL){
		while ((ep = readdir (dp))){
			if(strcmp(ep->d_name,".") == 0 || strcmp(ep->d_name,"..") == 0){
				continue;
			}
			GtkWidget *map;
			map=gtk_label_new(ep->d_name);
			gtk_list_box_insert (GTK_LIST_BOX(list),map,i);
			i++;		
		}
		(void) closedir (dp);
	}
	else perror ("Couldn't open the directory");

	gtk_widget_show_all (dialog);
	int result = gtk_dialog_run (GTK_DIALOG (dialog));
	GtkListBoxRow *select;
	switch (result){
		case GTK_RESPONSE_OK: select = gtk_list_box_get_selected_row(GTK_LIST_BOX(list));
				      GList *select_list = gtk_container_get_children(GTK_CONTAINER(select));
				      GtkWidget *child = GTK_WIDGET(select_list->data);
				      char tmp[100];
				      strcpy(tmp, gtk_label_get_label(GTK_LABEL(child)));
				      strcpy(my->load_name,tmp);
				      break;
		default: strcpy(my->load_name,"DEF");
			 break;
  	}
	gtk_widget_destroy (dialog);
}

void save_dialog(GtkWindow *parent, gpointer data){
	Mydata *my = get_mydata(data);
	
	GtkWidget *dialog, *entry, *label, *content_area;
	GtkDialogFlags flags = GTK_DIALOG_MODAL;

	dialog = gtk_dialog_new_with_buttons ("Save yo track", parent, flags, "Save", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	entry = gtk_entry_new();
	label = gtk_label_new("Enter save name");
	
	gtk_container_add (GTK_CONTAINER (content_area), label);
	gtk_container_add (GTK_CONTAINER (content_area), entry);
	gtk_widget_show_all (dialog);
	int result = gtk_dialog_run (GTK_DIALOG (dialog));
	switch (result){
		case GTK_RESPONSE_OK: my->save_name = gtk_editable_get_chars(GTK_EDITABLE(entry),0,-1);
				      break;
		default: my->save_name = NULL;
			 break;
  	}
	gtk_widget_destroy (dialog);
}

void check_save(){
	DIR *a = opendir("save/");
	if(a == NULL){
		mkdir("save", 0777);
		return;
	}
	closedir(a);
}

void check_dir(char * dirname){
	char path[100];
	sprintf(path,"save/%s",dirname);
	DIR *a = opendir(path);
	if(a == NULL){
		mkdir(path, 0777);
		return;
	}
	closedir(a);

}

void save_track(gpointer data,char* filename){
	Mydata *my = get_mydata(data);
	check_save();
	check_dir(filename);
	FILE *fp;
	char path[100];
	sprintf(path,"save/%s/track.txt",filename);
   	fp = fopen(path, "w");
	for(int i =0;i<my->game.road.curve_central.control_count;i++){//parcour de chaque control de curve central
   		fprintf(fp, "%f %f\n",my->game.road.curve_central.controls[i].x,my->game.road.curve_central.controls[i].y);
	}
	if(fp != NULL) fclose(fp);
}

void load_track(gpointer data, char * filename){
	Mydata *my = get_mydata(data);
	FILE *fp;
	int i=0;
	char path[100];
	sprintf(path,"save/%s/track.txt",filename);
   	fp = fopen(path, "r");
	if (fp == NULL){
		printf("fichier inexistant\n"); // afficher POP UP
		return;
	}
	remove_curve(&my->game.road.curve_central);
	remove_curve(&my->game.road.curve_right);
	remove_curve(&my->game.road.curve_left);
	double a=0;
	int j=0;
	my->game.road.curve_central.control_count=0;
	while(fscanf(fp, "%lf", &a) != EOF){
		if(i==0){
			my->game.road.curve_central.controls[j].x = a;
			i++;
		}else{
			my->game.road.curve_central.controls[j].y = a;
			i=0;
			my->game.road.curve_central.control_count++;
			j++;
			generate_control_road(&my->game.road, my->largeur_route);	
		}
		
	}
	refresh_area(my->area1);
	fclose(fp);
}

