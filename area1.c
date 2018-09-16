#include <gtk/gtk.h>
#include <math.h>
#include <time.h>
#include "utils.h"
#include "curve.h"
#include "mydata.h"
#include "game.h"
#include "main_menu.h"
#include "font.h"
#include "client.h"
#include "LANC.h"
#include "area1.h"

int finish(gpointer data, int car){
	Mydata *my = get_mydata(data);
	
	int i;
	for(i=0; i < my->game.car_count; i++){
		if(car == my->finish_line[i]) return 1;	
	}
	return 0;
}

int all_finish(gpointer data){
	Mydata *my = get_mydata(data);
	
	int i;
	for(i=0; i < my->game.car_count; i++){
		if(my->finish_line[i] == -1) return 0;	
	}
	return 1;
}

void finish_dialog(GtkWindow *parent, gpointer data){
	Mydata *my = get_mydata(data);
	
	GtkWidget *dialog, *content_area;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons ("Finito", parent, flags, "Ok", GTK_RESPONSE_NONE, NULL);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	
	

	for(int i=0; i < my->game.car_count; i++){
		char linu[100];
		switch(my->finish_line[i]){
			case 0 : sprintf(linu,"Joueur %d WHITE | %6.3fs",my->finish_line[i]+1,((my->finish_time[i]-my->start_time)/CLOCKS_PER_SEC)*10); break;
			case 1 : sprintf(linu,"Joueur %d RED   | %6.3fs",my->finish_line[i]+1,((my->finish_time[i]-my->start_time)/CLOCKS_PER_SEC)*10); break;
			case 2 : sprintf(linu,"Joueur %d BLUE  | %6.3fs",my->finish_line[i]+1,((my->finish_time[i]-my->start_time)/CLOCKS_PER_SEC)*10); break;
			case 3 : sprintf(linu,"Joueur %d GREEN | %6.3fs",my->finish_line[i]+1,((my->finish_time[i]-my->start_time)/CLOCKS_PER_SEC)*10); break;
		}
		GtkWidget *label = gtk_label_new(linu);
		gtk_container_add (GTK_CONTAINER (content_area), label);
	}
	gtk_widget_show_all (dialog);
	
	g_signal_connect_swapped (dialog,"response",G_CALLBACK (gtk_widget_destroy),dialog);

}

gboolean on_timeout1(gpointer data) {
	Mydata *my = get_mydata(data);
	Game *game = &my->game;
	
	//1
	game->cars[0].direction = (my->flag_key_left && !my->flag_key_right) ? -1 :
		(my->flag_key_right && !my->flag_key_left) ? 1 : 0;

	game->cars[0].accelerator = (my->flag_key_down && !my->flag_key_up) ? -1 :
		(my->flag_key_up && !my->flag_key_down) ? 1 : 0;
	

	//2
	game->cars[1].direction = (my->flag_key_q && !my->flag_key_d) ? -1 :
		(my->flag_key_d && !my->flag_key_q) ? 1 : 0;

	game->cars[1].accelerator = (my->flag_key_s && !my->flag_key_z) ? -1 :
		(my->flag_key_z && !my->flag_key_s) ? 1 : 0;


	//3
	game->cars[2].direction = (my->flag_key_j && !my->flag_key_l) ? -1 :
		(my->flag_key_l && !my->flag_key_j) ? 1 : 0;

	game->cars[2].accelerator = (my->flag_key_k && !my->flag_key_i) ? -1 :
		(my->flag_key_i && !my->flag_key_k) ? 1 : 0;

	//4
	game->cars[3].direction = (my->flag_key_4 && !my->flag_key_6) ? -1 :
		(my->flag_key_6 && !my->flag_key_4) ? 1 : 0;

	game->cars[3].accelerator = (my->flag_key_5 && !my->flag_key_8) ? -1 :
		(my->flag_key_8 && !my->flag_key_5) ? 1 : 0;
	
    //LAN MODE
    if(my->player_id != -1 && my->lan_mode == 2){
        if(my->countdown_value < 0) next_step_LAN(data);
        refresh_area(my->area1);
        return TRUE;
    }

	if(my->countdown_value < 0) progress_game_next_step(game);
	
	//FINISH LINE
	for(int i=0; i < game->car_count; i++){
		if(game->cars[i].finish && !finish(data,i)){
			int j=0;
			while(my->finish_line[j] != -1 && j<game->car_count) j++;
			my->finish_line[j]=i;
			my->finish_time[j]=clock();
		}
	}

	if(all_finish(data)){
        if(my->lan_mode){
            if(my->player_id == 0) wait_score_LANC(data);
            else send_score_LANC(data);
        }else finish_dialog(GTK_WINDOW(my->window),data);
		reset_game(data);
		menu_switch(data,MODE_MENU);
	}
	refresh_area(my->area1);
	return TRUE;
}

void set_anim1_mode(Mydata *my, int flag) {
    if (flag) {
        if (my->timeout1 != 0) return;
        my->timeout1 = g_timeout_add(ANIM1_TIME, on_timeout1, my);

        set_status(my->status, "Animation 1 started");
    } else {
        if (my->timeout1 == 0) return;
        g_source_remove(my->timeout1);
        my->timeout1 = 0;

        set_status(my->status, "Animation 1 stopped");
    }
}

void generate_control_road(Road *road, double d) {
    if (road->curve_central.control_count < 2) return;
    int nb = road->curve_central.control_count;

    double bx[2], by[2];
    bx[0] = road->curve_central.controls[nb - 2].x;
    by[0] = road->curve_central.controls[nb - 2].y;
    bx[1] = road->curve_central.controls[nb - 1].x;
    by[1] = road->curve_central.controls[nb - 1].y;

    double xn, yn;
    compute_normal_right(bx[0], by[0], bx[1], by[1], &xn, &yn);
    double xd0, yd0, xd1, yd1, xg0, yg0, xg1, yg1;
    xd0 = bx[0] + xn*d;
    xd1 = bx[1] + xn*d;
    yd0 = by[0] + yn*d;
    yd1 = by[1] + yn*d;

    xg0 = bx[0] - xn*d;
    xg1 = bx[1] - xn*d;
    yg0 = by[0] - yn*d;
    yg1 = by[1] - yn*d;

    //add curve D

    // add control *2
    add_control(&road->curve_right, xd0, yd0);

    //add curve G

    //add control
    add_control(&road->curve_left, xg0, yg0);


    //REECRIRE ADD CONTROL AVEC CURVE EN ARG

    if (road->curve_central.control_count > 2) {
        double xdm, ydm, xgm, ygm;
        //NOMBRE DE POINTS
        int nb_dg = road->curve_right.control_count;
        //DROITE
        compute_intersection(road->curve_right.controls[nb_dg - 3].x, road->curve_right.controls[nb_dg - 3].y, road->curve_right.controls[nb_dg - 2].x, road->curve_right.controls[nb_dg - 2].y, xd0, yd0, xd1, yd1, &xdm, &ydm);

        //GAUCHE
        compute_intersection(road->curve_left.controls[nb_dg - 3].x, road->curve_left.controls[nb_dg - 3].y, road->curve_left.controls[nb_dg - 2].x, road->curve_left.controls[nb_dg - 2].y, xg0, yg0, xg1, yg1, &xgm, &ygm);

        remove_control(&road->curve_right, nb_dg - 1);
        remove_control(&road->curve_right, nb_dg - 2);
        remove_control(&road->curve_left, nb_dg - 1);
        remove_control(&road->curve_left, nb_dg - 2);

        add_control(&road->curve_right, xdm, ydm);
        add_control(&road->curve_left, xgm, ygm);
    }

    add_control(&road->curve_right, xd1, yd1);
    add_control(&road->curve_left, xg1, yg1);
}

void generate_control_road_from_control(Road *road, double d, int current_control) {
    if (road->curve_central.control_count < 2) return;
    int nb = road->curve_central.control_count;

    double bx[2], by[2];
    bx[0] = road->curve_central.controls[nb - 2].x;
    by[0] = road->curve_central.controls[nb - 2].y;
    bx[1] = road->curve_central.controls[nb - 1].x;
    by[1] = road->curve_central.controls[nb - 1].y;

    double xn, yn;
    compute_normal_right(bx[0], by[0], bx[1], by[1], &xn, &yn);
    double xd0, yd0, xd1, yd1, xg0, yg0, xg1, yg1;
    xd0 = bx[0] + xn*d;
    xd1 = bx[1] + xn*d;
    yd0 = by[0] + yn*d;
    yd1 = by[1] + yn*d;

    xg0 = bx[0] - xn*d;
    xg1 = bx[1] - xn*d;
    yg0 = by[0] - yn*d;
    yg1 = by[1] - yn*d;

    //add curve D

    // add control *2
    add_control(&road->curve_right, xd0, yd0);

    //add curve G

    //add control
    add_control(&road->curve_left, xg0, yg0);


    //REECRIRE ADD CONTROL AVEC CURVE EN ARG
    if (road->curve_central.control_count > 2) {
        for (int i = current_control - 1; i < road->curve_central.control_count; i++) {
            double xdm, ydm, xgm, ygm;
            //NOMBRE DE POINTS
            int nb_dg = i;
            //DROITE
            compute_intersection(road->curve_right.controls[nb_dg - 3].x, road->curve_right.controls[nb_dg - 3].y, road->curve_right.controls[nb_dg - 2].x, road->curve_right.controls[nb_dg - 2].y, xd0, yd0, xd1, yd1, &xdm, &ydm);

            //GAUCHE
            compute_intersection(road->curve_left.controls[nb_dg - 3].x, road->curve_left.controls[nb_dg - 3].y, road->curve_left.controls[nb_dg - 2].x, road->curve_left.controls[nb_dg - 2].y, xg0, yg0, xg1, yg1, &xgm, &ygm);

            remove_control(&road->curve_right, nb_dg - 1);
            remove_control(&road->curve_right, nb_dg - 2);
            remove_control(&road->curve_left, nb_dg - 1);
            remove_control(&road->curve_left, nb_dg - 2);

            add_control(&road->curve_right, xdm, ydm);
            add_control(&road->curve_left, xgm, ygm);
        }
    }

    add_control(&road->curve_right, xd1, yd1);
    add_control(&road->curve_left, xg1, yg1);
}

void draw_bezier_curve(cairo_t *cr, Control bez_points[4], double theta) {
    double t = 0.0, bx[4], by[4];
    int i;
    for (i = 0; i < 4; i++) {
        bx[i] = bez_points[i].x;
        by[i] = bez_points[i].y;
    }
    double x, y;
    cairo_set_source_rgb(cr, 1, 0, 1);
    while (t <= 1) {
        x = compute_bezier_cubic(bx, t);
        y = compute_bezier_cubic(by, t);
        if (t == 0.0) {
            cairo_move_to(cr, x, y);
        } else {
            cairo_line_to(cr, x, y);
        }
        t = t + theta;
    }
    cairo_stroke(cr);
}

void draw_bezier_curves_prolong(cairo_t *cr, Road *road, double theta) {
    int j;
    Control bez_points[4];
    if (road->curve_central.control_count < 2) return;

    if (road->curve_central.control_count == 2) {
        //CENTRAL
        cairo_move_to(cr, road->curve_central.controls[0].x, road->curve_central.controls[0].y);
        cairo_line_to(cr, road->curve_central.controls[1].x, road->curve_central.controls[1].y);
        cairo_set_source_rgb(cr, 1, 0, 1);
        cairo_stroke(cr);
        //RIGHT
        cairo_move_to(cr, road->curve_right.controls[0].x, road->curve_right.controls[0].y);
        cairo_line_to(cr, road->curve_right.controls[1].x, road->curve_right.controls[1].y);
        cairo_set_source_rgb(cr, 1, 0, 1);
        cairo_stroke(cr);
        //LEFT
        cairo_move_to(cr, road->curve_left.controls[0].x, road->curve_left.controls[0].y);
        cairo_line_to(cr, road->curve_left.controls[1].x, road->curve_left.controls[1].y);
        cairo_set_source_rgb(cr, 1, 0, 1);
        cairo_stroke(cr);
        return;
    }

    //CENTRAL
    for (j = 0; j < road->curve_central.control_count - 1; j++) {
        if (j == 0) {
            compute_bezier_points_prolong_first(&road->curve_central, bez_points);
        } else if (j == road->curve_central.control_count - 2) {
            compute_bezier_points_prolong_last(&road->curve_central, bez_points);
        } else {
            compute_bezier_points_open(&road->curve_central, j - 1, bez_points);
        }
        draw_bezier_curve(cr, bez_points, theta);
    }

    //RIGHT
    for (j = 0; j < road->curve_right.control_count - 1; j++) {
        if (j == 0) {
            compute_bezier_points_prolong_first(&road->curve_right, bez_points);
        } else if (j == road->curve_right.control_count - 2) {
            compute_bezier_points_prolong_last(&road->curve_right, bez_points);
        } else {
            compute_bezier_points_open(&road->curve_right, j - 1, bez_points);
        }
        draw_bezier_curve(cr, bez_points, theta);
    }

    //LEFT
    for (j = 0; j < road->curve_left.control_count - 1; j++) {
        if (j == 0) {
            compute_bezier_points_prolong_first(&road->curve_left, bez_points);
        } else if (j == road->curve_left.control_count - 2) {
            compute_bezier_points_prolong_last(&road->curve_left, bez_points);
        } else {
            compute_bezier_points_open(&road->curve_left, j - 1, bez_points);
        }
        draw_bezier_curve(cr, bez_points, theta);
    }

}

void draw_control_labels(cairo_t *cr, PangoLayout *layout, Road *road) {
    int i;
    font_set_name(layout, "Sans, 8");
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    for (i = 0; i < road->curve_central.control_count; i++) {
        font_draw_text(cr, layout, FONT_BC, road->curve_central.controls[i].x, road->curve_central.controls[i].y - 3, "%d", i);
    }
}

void draw_control_polygons(cairo_t *cr, Road *road) {
    int i;
    //SEGMENTS
    //CENTRAL
    for (i = 0; i < road->curve_central.control_count; i++) {
        if (i == 0) {
            cairo_move_to(cr, road->curve_central.controls[i].x, road->curve_central.controls[i].y);
        } else {
            cairo_line_to(cr, road->curve_central.controls[i].x, road->curve_central.controls[i].y);
        }
    }
    cairo_set_source_rgb(cr, 1, 1, 0);
    cairo_stroke(cr);

    //RIGHT
    for (i = 0; i < road->curve_right.control_count; i++) {
        if (i == 0) {
            cairo_move_to(cr, road->curve_right.controls[i].x, road->curve_right.controls[i].y);
        } else {
            cairo_line_to(cr, road->curve_right.controls[i].x, road->curve_right.controls[i].y);
        }
    }
    cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
    cairo_stroke(cr);

    //LEFT
    for (i = 0; i < road->curve_left.control_count; i++) {
        if (i == 0) {
            cairo_move_to(cr, road->curve_left.controls[i].x, road->curve_left.controls[i].y);
        } else {
            cairo_line_to(cr, road->curve_left.controls[i].x, road->curve_left.controls[i].y);
        }
    }
    cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
    cairo_stroke(cr);


    //CARRE CENTRAL
    cairo_set_source_rgb(cr, 0, 0, 1.0);
    for (i = 0; i < road->curve_central.control_count; i++) {
        cairo_rectangle(cr, road->curve_central.controls[i].x - 3, road->curve_central.controls[i].y - 3, 6, 6);
    }
    cairo_stroke(cr);
}

//A DEBUG VOITURES MULTIPLES
void draw_car(cairo_t *cr, Car car, GdkPixbuf *car_model, int i, int debug) {
	cairo_set_dash(cr,0,0,0);
	
    //TRAIL
    switch(i){
        case 0 : cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 0.5); break;
        case 1 : cairo_set_source_rgba(cr, 0.5, 0.1, 0.1, 0.5); break;
        case 2 : cairo_set_source_rgba(cr, 0.1, 0.4, 0.5, 0.5); break;
        case 3 : cairo_set_source_rgba(cr, 0.1, 0.5, 0.1, 0.5); break;
    }
    for(int i=0; i < 100; i++){
        if(i == car.trail_count) cairo_move_to(cr, car.trail_l_x[i], car.trail_l_y[i]);
        else if(car.trail_l_x[i] != -1){ cairo_line_to(cr, car.trail_l_x[i], car.trail_l_y[i]); }
    }
    cairo_stroke(cr);

    for(int i=0; i < 100; i++){
        if(i == car.trail_count) cairo_move_to(cr, car.trail_r_x[i], car.trail_r_y[i]);
        else if(car.trail_r_x[i] != -1){ cairo_line_to(cr, car.trail_r_x[i], car.trail_r_y[i]); }
    }
    cairo_stroke(cr);
    
	//CAR MODEL
	cairo_surface_t *image;
    	char path[100];
	sprintf(path,"assets/ae86/%d0.png",i);
    	if(i==0){
       		if(car.direction == 1) sprintf(path,"assets/ae86/%d1.png",i);
        	else if(car.direction == -1) sprintf(path,"assets/ae86/%d2.png",i);
    	}
	image = cairo_image_surface_create_from_png(path);
	
	double width = cairo_image_surface_get_width(image), height = cairo_image_surface_get_height(image);
	cairo_translate(cr, car.x, car.y);
	cairo_rotate(cr, car.angle);
	cairo_translate(cr, -width / 2.0, -height / 2.0);
	cairo_set_source_surface(cr, image, 0, 0);
	cairo_paint(cr);
	
	cairo_translate(cr, width / 2.0, height / 2.0);
	cairo_rotate(cr, - car.angle);
	cairo_translate(cr, -car.x, -car.y);

    
	if(debug){
		//DEBUG MODEL
	    	cairo_set_source_rgb(cr, 0, 0, 1);
		cairo_arc(cr, car.x, car.y, 10.0, 0, 3.14 * 2);

		//LINE
		double x = car.x + car.radius*cos(car.angle)*2;	
		double y = car.y + car.radius*sin(car.angle)*2;
		cairo_move_to(cr,car.x,car.y);
		cairo_line_to(cr,x,y);
		cairo_stroke(cr);

	   	//LINE D
		x = car.x + car.radius*cos(car.angle_d)*2;	
		y = car.y + car.radius*sin(car.angle_d)*2;
	   	cairo_set_source_rgb(cr, 0, 1, 0);
		cairo_move_to(cr,car.x,car.y);
		cairo_line_to(cr,x,y);
		cairo_stroke(cr);

		//SPEED LINE
		x = car.x + car.speed * cos(car.angle);	
		y = car.y + car.speed * sin(car.angle);
		cairo_set_source_rgb(cr, 1, 0, 0);
		cairo_move_to(cr,car.x,car.y);
		cairo_line_to(cr,x,y);
		cairo_stroke(cr);
	}
}

//--------------------------AREA CALLBACK--------------------

gboolean on_area1_key_release(GtkWidget *area, GdkEvent *event, gpointer data) {
    GdkEventKey *evk = &event->key;
    Mydata *my = get_mydata(data);
    //printf("%s: GDK_KEY_%s\n", __func__, gdk_keyval_name(evk->keyval));
    switch (evk->keyval) {
	//1
	case GDK_KEY_Left: my->flag_key_left = 0;
	break;
	case GDK_KEY_Right: my->flag_key_right = 0;
	break;
	case GDK_KEY_Up: my->flag_key_up = 0;
	break;
	case GDK_KEY_Down: my->flag_key_down = 0;
	break;
	
	//2
	case GDK_KEY_q: my->flag_key_q = 0;
	break;
	case GDK_KEY_d: my->flag_key_d = 0;
	break;
	case GDK_KEY_z: my->flag_key_z = 0;
	break;
	case GDK_KEY_s: my->flag_key_s = 0;
	break;
	
	//3
	case GDK_KEY_j: my->flag_key_j = 0;
	break;
	case GDK_KEY_k: my->flag_key_k = 0;
	break;
	case GDK_KEY_l: my->flag_key_l = 0;
	break;
	case GDK_KEY_i: my->flag_key_i = 0;
	break;

	//4
	case GDK_KEY_KP_8: my->flag_key_8 = 0;
	break;
	case GDK_KEY_KP_4: my->flag_key_4 = 0;
	break;
	case GDK_KEY_KP_5: my->flag_key_5 = 0;
	break;
	case GDK_KEY_KP_6: my->flag_key_6 = 0;
	break;
    }
    return TRUE;
}

gboolean on_area1_button_press(GtkWidget *area, GdkEvent *event, gpointer data) {
    Mydata *my = get_mydata(data);
    GdkEventButton *evb = &event->button;
    printf("%s: %u %.1f %.1f\n", __func__, evb->button, evb->x, evb->y);
    my->click_x = evb->x;
    my->click_y = evb->y;
    my->click_n = 1;
    int c = -1;

    if (evb->button == 1 && my->show_edit == TRUE) {
        switch (my->edit_mode) {
            case EDIT_ADD_CONTROL:
                add_control(&my->game.road.curve_central, evb->x, evb->y);
                generate_control_road(&my->game.road, my->largeur_route);
                break;
            case EDIT_MOVE_CONTROL:
                c = find_control(&my->game.road.curve_central, evb->x, evb->y);
                my->current_control = c;
                break;
            case EDIT_MOVE_CURVE:

                break;
            case EDIT_REMOVE_CONTROL:
                c = find_control(&my->game.road.curve_central, evb->x, evb->y);
                remove_control(&my->game.road.curve_central, c);
                remove_control(&my->game.road.curve_right, c);
                remove_control(&my->game.road.curve_left, c);
                break;
        }
    }
    refresh_area(my->area1);
    return TRUE;
}

gboolean on_area1_button_release(GtkWidget *area, GdkEvent *event, gpointer data) {
    Mydata *my = get_mydata(data);
    GdkEventButton *evb = &event->button;
    printf("%s: %u %.1f %.1f\n", __func__, evb->button, evb->x, evb->y);
    my->click_n = 0;
    refresh_area(my->area1);
    return TRUE;
}

gboolean on_area1_motion_notify(GtkWidget *area, GdkEvent *event, gpointer data) {
    Mydata *my = get_mydata(data);
    GdkEventMotion *evm = &event->motion;
    //printf ("%s: %.1f %.1f\n", __func__, evm->x, evm->y);
    if (my->click_n == 1) {
        my->last_x = my->click_x;
        my->last_y = my->click_y;
        my->click_x = evm->x;
        my->click_y = evm->y;
        if (my->show_edit == TRUE) {
            switch (my->edit_mode) {
                case EDIT_MOVE_CONTROL:
                    move_control(&my->game.road.curve_central, my->click_x - my->last_x, my->click_y - my->last_y, my->current_control);
                    //generate_control_road_from_control(&my->game.road, my->largeur_route, my->current_control);
                    break;
                case EDIT_MOVE_CURVE:
                    move_curve(&my->game.road.curve_central, my->click_x - my->last_x, my->click_y - my->last_y);
                    move_curve(&my->game.road.curve_right, my->click_x - my->last_x, my->click_y - my->last_y);
                    move_curve(&my->game.road.curve_left, my->click_x - my->last_x, my->click_y - my->last_y);
                    break;
            }
        }
        refresh_area(my->area1);
    }
    return TRUE;
}

gboolean on_area1_key_press(GtkWidget *area, GdkEvent *event, gpointer data) {
    Mydata *my = get_mydata(data);
    GdkEventKey *evk = &event->key;
    //printf("%s: GDK_KEY_%s\n", __func__, gdk_keyval_name(evk->keyval));
    switch (evk->keyval) {
	case GDK_KEY_Escape: menu_switch(data, MODE_MENU);
	break;
	//1
	case GDK_KEY_Left: my->flag_key_left = 1;
	break;
	case GDK_KEY_Right: my->flag_key_right = 1;
	break;
	case GDK_KEY_Up: my->flag_key_up = 1;
	break;
	case GDK_KEY_Down: my->flag_key_down = 1;
	break;
	
	//2
	case GDK_KEY_q: my->flag_key_q = 1;
	break;
	case GDK_KEY_d: my->flag_key_d = 1;
	break;
	case GDK_KEY_z: my->flag_key_z = 1;
	break;
	case GDK_KEY_s: my->flag_key_s = 1;
	break;

	//3
	case GDK_KEY_j: my->flag_key_j = 1;
	break;
	case GDK_KEY_k: my->flag_key_k = 1;
	break;
	case GDK_KEY_l: my->flag_key_l = 1;
	break;
	case GDK_KEY_i: my->flag_key_i = 1;
	break;

	//4
	case GDK_KEY_KP_8: my->flag_key_8 = 1;
	break;
	case GDK_KEY_KP_4: my->flag_key_4 = 1;
	break;
	case GDK_KEY_KP_5: my->flag_key_5 = 1;
	break;
	case GDK_KEY_KP_6: my->flag_key_6 = 1;
	break;
    }
    return TRUE;
}

//A MODIFIER POUR Road

gboolean on_area1_draw(GtkWidget *area, cairo_t *cr, gpointer data) {
	Mydata *my = get_mydata(data);
	//int width = gtk_widget_get_allocated_width(area);
	//int height = gtk_widget_get_allocated_height(area);
	//printf("on_area1_draw : largeur %d hauteur %d \n",width,height);
	//AFFICHAGE ROUTE

	switch (my->bsp_mode) {
	case BSP_PROLONG:   draw_control_polygons(cr, &my->game.road);
		            draw_bezier_curves_prolong(cr, &my->game.road, 0.01);
		            PangoLayout *layout = pango_cairo_create_layout(cr);
		            draw_control_labels(cr, layout, &my->game.road);
		            g_object_unref(layout);
		            break;
	case BSP_DRAW:  draw_road(cr, &my->game.road);
		        break;
	}

	//AFFICHAGE VOITURE
	for (int i = 0; i < my->game.car_count; i++) {
		draw_car(cr, my->game.cars[i], my->ae86, i, my->debug_mode);
	}
	
	//COUNTDOWN
	if(my->countdown_value != -1 && my->bsp_mode != BSP_PROLONG){
		char text[4];
		int x, y;
		cairo_set_line_width (cr, 1.5);
		cairo_set_source_rgb (cr, 1, 0, 0);
		PangoLayout *layout = pango_cairo_create_layout (cr);

		if(my->countdown_value > 0) sprintf(text,"%d",my->countdown_value);
		else if(my->countdown_value == 0)sprintf(text,"GO!");
        else return TRUE;

		gtk_window_get_size(GTK_WINDOW(my->window), &x, &y);
		
		PangoFontDescription *desc;
		desc = pango_font_description_from_string ("Serif, bold 40");
		pango_layout_set_font_description (layout, desc);
		pango_font_description_free (desc);

		int w, h;
		pango_layout_get_pixel_size (layout, &w, &h);
		cairo_move_to (cr, x/2 - w/2., y/2 - h/2.);
		

		pango_layout_set_text (layout, text, -1);
		pango_cairo_show_layout (cr, layout);
		g_object_unref (layout);
	}

    //WAIT TEXT
    if(my->wait == 1){
        printf("WIAT \n");
        char text[100];
		int x, y;
		cairo_set_line_width (cr, 1.5);
		cairo_set_source_rgb (cr, 1, 0, 0);
		PangoLayout *layout = pango_cairo_create_layout (cr);

		sprintf(text,"WAITING FOR OTHER PLAYERS");

		gtk_window_get_size(GTK_WINDOW(my->window), &x, &y);
		
		PangoFontDescription *desc;
		desc = pango_font_description_from_string ("Serif, bold 40");
		pango_layout_set_font_description (layout, desc);
		pango_font_description_free (desc);

		int w, h;
		pango_layout_get_pixel_size (layout, &w, &h);
		cairo_move_to (cr, x/2 - w/2., y/2 - h/2.);
		

		pango_layout_set_text (layout, text, -1);
		pango_cairo_show_layout (cr, layout);
		g_object_unref (layout);
    }


	return TRUE;
}
    //--------------------------AREA INIT--------------------

    void area1_init(gpointer data) {
        Mydata *my = get_mydata(data);


        //Image
        my->area1 = gtk_drawing_area_new();

        g_signal_connect(my->area1, "draw", G_CALLBACK(on_area1_draw), my);

        g_signal_connect(my->area1, "key-press-event", G_CALLBACK(on_area1_key_press), my);
        g_signal_connect(my->area1, "key-release-event", G_CALLBACK(on_area1_key_release), my);
        g_signal_connect(my->area1, "button-press-event", G_CALLBACK(on_area1_button_press), my);
        g_signal_connect(my->area1, "button-release-event", G_CALLBACK(on_area1_button_release), my);
        g_signal_connect(my->area1, "motion-notify-event", G_CALLBACK(on_area1_motion_notify), my);
        gtk_widget_add_events(my->area1, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_POINTER_MOTION_MASK);

        gtk_widget_set_can_focus(my->area1, TRUE);

    }

