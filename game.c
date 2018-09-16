#include <gtk/gtk.h>
#include <math.h>
#include "curve.h"
#include "game.h"
#include "utils.h"

double compute_center(double x1, double x2){
	return (x1+x2)/2;
}

void place_cars_on_start (Game *game){
	double x1 = game->road.track_left.sample_x[0];
	double y1 = game->road.track_left.sample_y[0];
	double x2 = game->road.track_right.sample_x[0];
	double y2 = game->road.track_right.sample_y[0];
	double x3 = game->road.track_central.sample_x[0];
	double y3 = game->road.track_central.sample_y[0];

	for(int i=0; i < game->car_count; i++){
		game->cars[i].angle=get_horiz_angle_rad(x1,y1,x2,y2)-G_PI/2;
		game->cars[i].angle_d=game->cars[i].angle;
		game->cars[i].speed=0;
		game->cars[i].radius=10;

		//INIT TRAIL
		game->cars[i].trail_count=0;
		for(int j=0; j<100; j++){
			game->cars[i].trail_l_x[j]=-1;
			game->cars[i].trail_l_y[j]=-1;
			game->cars[i].trail_r_x[j]=-1;
			game->cars[i].trail_r_y[j]=-1;
		}

	}
	
	switch(game->car_count){
		case 1 : game->cars[0].x = x3;
			game->cars[0].y = y3;
			break;
		
		case 2 :
			game->cars[0].x = compute_center(x1,x3);
			game->cars[0].y = compute_center(y1,y3);
			game->cars[1].x = compute_center(x3,x2);
			game->cars[1].y = compute_center(y3,y2);
			break;

		case 3 :
			game->cars[0].x = compute_center(x1,x3);
			game->cars[0].y = compute_center(y1,y3);
			game->cars[1].x = compute_center(x3,x2);
			game->cars[1].y = compute_center(y3,y2);
			game->cars[2].x = x3;
			game->cars[2].y = y3;
			break;

		case 4 :
			game->cars[0].x = compute_center(compute_center(x1,x3),x1);
			game->cars[0].y = compute_center(compute_center(y1,y3),y1);
			game->cars[1].x = compute_center(compute_center(x1,x3),x3);
			game->cars[1].y = compute_center(compute_center(y1,y3),y3);
			game->cars[2].x = compute_center(compute_center(x2,x3),x3);
			game->cars[2].y = compute_center(compute_center(y2,y3),y3);
			game->cars[3].x = compute_center(compute_center(x2,x3),x2);
			game->cars[3].y = compute_center(compute_center(y2,y3),y2);
			break;
	}
}

void progress_game_next_step (Game *game){
	for(int i=0; i<game->car_count; i++){
		//TEST ARRIVE
		if(game->cars[i].finish){
			game->cars[i].accelerator=0;
			game->cars[i].direction=0;
		}
		//NOUVELLE VITESSE
		double speed=game->cars[i].speed;
		switch((int) game->cars[i].accelerator){
			case 1: if(speed <= 8)	speed+=0.8; break;
			case 0: if(speed <= 0.2 && speed >= -0.2) speed=0; else if(speed > 0) speed -=0.2; else if(speed < 0) speed+= 0.2; break;
			case -1: if(speed >= -3) speed-=0.4; break;	
		}
		game->cars[i].speed= speed;

		//NOUVEL ANGLE
		double xa=game->cars[i].x, ya=game->cars[i].y;
		if(game->cars[i].direction == 1){
			game->cars[i].angle+=0.1;
			
		}else if(game->cars[i].direction == -1){
			game->cars[i].angle-=0.1;
		}
		
		
		//SLIDE EFFECT
		if(game->cars[i].angle != game->cars[i].angle_d && game->drift_mode){
			if(game->cars[i].angle - game->cars[i].angle_d < 0)	game->cars[i].angle_d-= -(game->cars[i].angle - game->cars[i].angle_d)/20;
			if(game->cars[i].angle - game->cars[i].angle_d > 0)	game->cars[i].angle_d+= (game->cars[i].angle - game->cars[i].angle_d)/20;
			if(game->cars[i].angle - game->cars[i].angle_d <= 0.01 && game->cars[i].angle - game->cars[i].angle_d >= -0.01) game->cars[i].angle_d=game->cars[i].angle;
		}

		double x = xa + game->cars[i].speed * cos(game->cars[i].angle);	
		double y = ya + game->cars[i].speed * sin(game->cars[i].angle);
		if(game->drift_mode){
			x = xa + game->cars[i].speed * cos(game->cars[i].angle_d);	
			y = ya + game->cars[i].speed * sin(game->cars[i].angle_d);
		}

		//COLISION VOITURES
		if(game->colision==1){
			for(int j=0; j < game->car_count; j++){
				if(colision_voiture(game->cars[i], game->cars[j]) && i!=j){
					game->cars[i].angle = compute_colision_car(game->cars[i],game->cars[j]);
				}			
			}
		}	
		
		//COLISION TRACK
		//RIGHT
		for(int j=0; j < game->road.track_right.sample_count-2; j++){
			if(is_close_to_line(game->road.track_right.sample_x[j], game->road.track_right.sample_y[j], game->road.track_right.sample_x[j+1], game->road.track_right.sample_y[j+1], game->cars[i].x, game->cars[i].y, game->cars[i].radius)){
				game->road.track_right.close[j]=1;
				if(is_go_to_exterior(game->road.track_right.sample_x[j], game->road.track_right.sample_y[j], game->road.track_right.sample_x[j+1], game->road.track_right.sample_y[j+1], game->cars[i].x, game->cars[i].y, x, y)){
					if(is_crossing_line(game->road.track_right.sample_x[j], game->road.track_right.sample_y[j], game->road.track_right.sample_x[j+1], game->road.track_right.sample_y[j+1], game->cars[i].x, game->cars[i].y, game->cars[i].radius)){
						if(game->drift_mode) game->cars[i].speed = -game->cars[i].speed;
						else{
							game->cars[i].angle = compute_reflection_car(game->road.track_right.sample_x[j], game->road.track_right.sample_y[j], game->road.track_right.sample_x[j+1], game->road.track_right.sample_y[j+1], game->cars[i].angle);
						
							if(game->cars[i].speed>=2){
								game->cars[i].speed-=4;
							}	
						}
						x =xa + game->cars[i].speed * cos(game->cars[i].angle);	
						y =ya + game->cars[i].speed * sin(game->cars[i].angle);					
					}
				}	
			}
		}
		//LEFT
		for(int j=game->road.track_left.sample_count-1; j >= 1; j--){
			if(is_close_to_line(game->road.track_left.sample_x[j], game->road.track_left.sample_y[j], game->road.track_left.sample_x[j-1], game->road.track_left.sample_y[j-1], game->cars[i].x, game->cars[i].y, game->cars[i].radius)){
				game->road.track_left.close[j]=1;
				if(is_go_to_exterior(game->road.track_left.sample_x[j], game->road.track_left.sample_y[j], game->road.track_left.sample_x[j-1], game->road.track_left.sample_y[j-1], game->cars[i].x, game->cars[i].y, x, y)){
					if(is_crossing_line(game->road.track_left.sample_x[j], game->road.track_left.sample_y[j], game->road.track_left.sample_x[j-1], game->road.track_left.sample_y[j-1], game->cars[i].x, game->cars[i].y, game->cars[i].radius)){
						if(game->drift_mode) game->cars[i].speed = -game->cars[i].speed;
						else{
							game->cars[i].angle = compute_reflection_car(game->road.track_left.sample_x[j], game->road.track_left.sample_y[j], game->road.track_left.sample_x[j+1], game->road.track_left.sample_y[j+1], game->cars[i].angle);
							if(game->cars[i].speed>=2){
								game->cars[i].speed-=4;
							}
						}
						x =xa + game->cars[i].speed * cos(game->cars[i].angle);	
						y =ya + game->cars[i].speed * sin(game->cars[i].angle);							
					}
				}		
			}
		}

		
		game->cars[i].x	= x;
		game->cars[i].y = y;
		
		
		//TEST ARRIVE
		int nb = game->road.track_left.sample_count-1;
		if(is_close_to_line(game->road.track_left.sample_x[nb], game->road.track_left.sample_y[nb], game->road.track_right.sample_x[nb], game->road.track_right.sample_y[nb], game->cars[i].x, game->cars[i].y, game->cars[i].radius)){
			if(is_crossing_line(game->road.track_left.sample_x[nb], game->road.track_left.sample_y[nb], game->road.track_right.sample_x[nb], game->road.track_right.sample_y[nb], game->cars[i].x, game->cars[i].y, game->cars[i].radius)){
				printf("THE END ! %d\n",i);
				game->cars[i].finish=1;
			}
		}

		//TRAIL EFFECT
		if(game->cars[i].trail_count == 100) game->cars[i].trail_count=0;
		int trail_count = game->cars[i].trail_count;
		double t_x = (game->cars[i].radius+4)*cos(game->cars[i].angle);
		double t_y = (game->cars[i].radius+4)*sin(game->cars[i].angle);
		game->cars[i].trail_l_x[trail_count] = x - t_x + (game->cars[i].radius-2)*cos(game->cars[i].angle - G_PI/2);
		game->cars[i].trail_l_y[trail_count] = y - t_y + (game->cars[i].radius-2)*sin(game->cars[i].angle - G_PI/2); 
		game->cars[i].trail_r_x[trail_count] = x - t_x + (game->cars[i].radius-2)*cos(game->cars[i].angle + G_PI/2); 
		game->cars[i].trail_r_y[trail_count] = y - t_y + (game->cars[i].radius-2)*sin(game->cars[i].angle + G_PI/2); 
		game->cars[i].trail_count++;
	}
}

double compute_colision_car(Car car1, Car car2){
	return get_horiz_angle_rad(car1.x,car1.y,car2.x,car2.y)+G_PI/2;
}

int colision_voiture(Car car1, Car car2){
	int dx=fabs(car1.x-car2.x);
	int dy=fabs(car1.y-car2.y);
	if(dx*dx+dy*dy<=(car1.radius*2)*(car1.radius*2)){
		return 1;
	}
	return 0;
}

void do_vector_product(double xa, double ya, double za, double xb, double yb, double zb, double *xn, double *yn, double *zn){
	*xn = ya*zb - yb*za;
	*yn = -(xa*zb - xb*za);
	*zn = xa*yb - xb*ya;
}

int compute_intersection(double xs1, double ys1, double xs2, double ys2, double xt1, double yt1, double xt2, double yt2, double *xp, double *yp){
	double as, bs, cs, at, bt, ct, xi, yi, zi;	
	do_vector_product(xs1, ys1, 1, xs2, ys2, 1, &as, &bs, &cs);
	do_vector_product(xt1, yt1, 1, xt2, yt2, 1, &at, &bt, &ct);
	
	do_vector_product(as, bs, cs, at, bt, ct, &xi, &yi, &zi);
	if(zi==0) return 1;	
	*xp=xi/zi;
	*yp=yi/zi;
	return 0;	
}

void compute_normal_right(double xa,double ya,double xb,double yb,double *xn,double *yn){
	double x = ya-yb;
	double y = xb-xa;
	double n = sqrt(pow(x,2)+pow(y,2));
	if (n==0){
		*xn=0;
		*yn=0;
		return;	
	}
	*xn=x/n;
	*yn=y/n;
}

double get_horiz_angle_rad (double xa, double ya, double xb, double yb){
	//printf("xa : %f,ya : %f,xb : %f,yb : %f\n",xa,ya,xb,yb);
	double dx = xb-xa;
	double dy = yb-ya;
	double n = sqrt(pow(dx,2)+pow(dy,2));
	
	if(n==0) return 0;

	dx=dx/n;
	dy=dy/n;

	if(dy>0) return acos(dx);
	else return -acos(dx);
}

int is_close_to_line (double xa, double ya, double xb, double yb, double xc, double yc, double r){
	double xar = min(xa,xb)-r,
	       yar = min(ya,yb)-r,
	       xbr = max(xa,xb)+r, 
	       ybr = max(ya,yb)+r;
	if(xc >= xar && xc <= xbr){
		if(yc >= yar && yc <= ybr){
			return TRUE;	
		}	
	}
	return FALSE;
}

double compute_dist_point_to_line (double xa, double ya, double xb, double yb, double xc, double yc){
	double result;
	double a = xc - xa, b = xb-xa, c = yc-ya, d = yb - ya;
	result = a * d - b * c;
	result/=sqrt(pow(xa-xb,2)+ pow(ya-yb,2));
	return result;
}

int is_go_to_exterior(double xa, double ya, double xb, double yb, double xc, double yc, double xd, double yd){
	double d1 = compute_dist_point_to_line(xa,ya,xb,yb,xc,yc);
	double d2 = compute_dist_point_to_line(xa,ya,xb,yb,xd,yd);
	return d1>d2 ? TRUE:FALSE;
}

int is_crossing_line(double xa, double ya, double xb, double yb, double xc, double yc, double r){
	double d1 = compute_dist_point_to_line(xa,ya,xb,yb,xc,yc);
	return d1<r ? TRUE:FALSE;
}

double compute_reflection_car(double xa, double ya, double xb, double yb, double angle){
	double angle_line = get_horiz_angle_rad(xa,ya,xb,yb);
	double angle_reflect = 2 * angle_line - angle;
	//(angle_line + G_PI/2) + (angle_line + G_PI/2) -angle;
	
	if(angle_reflect < 0) angle_reflect+=2*G_PI;
	if(angle_reflect >= 2*G_PI) angle_reflect-=2*G_PI;
	
	//printf("ANGLE VOITURE : %f | TRACK %f > NEW %f \n", angle, angle_line, angle_reflect);
	
	return angle_reflect;
}

void compute_bezier_track (Curve *curve, Track *tra, double theta){
	Control bez_points[4];

	tra->sample_count = 0;
	if (curve->control_count < 3) return;

	compute_bezier_points_prolong_first (curve, bez_points);
	sample_bezier_curve (bez_points, theta, tra->sample_x, tra->sample_y,
	&tra->sample_count, SAMPLE_MAX, 1);

	for (int k = 0; k < curve->control_count-3; k++) {
		compute_bezier_points_open (curve, k, bez_points);
		sample_bezier_curve (bez_points, theta, tra->sample_x, tra->sample_y,
		&tra->sample_count, SAMPLE_MAX, 0);
	}

	compute_bezier_points_prolong_last (curve, bez_points);
	sample_bezier_curve (bez_points, theta, tra->sample_x, tra->sample_y,
	&tra->sample_count, SAMPLE_MAX, 0);
}


// Conversion des 3 curves en 3 tracks

void compute_road_tracks (Road *road, double theta){
	compute_bezier_track (&road->curve_left,    &road->track_left,    theta);
	compute_bezier_track (&road->curve_central, &road->track_central, theta);
	compute_bezier_track (&road->curve_right,   &road->track_right,   theta);
}

// Affichage d'un track : appeler avant les fonctions :
//  cairo_set_source_rgb
//  cairo_set_line_width
//  cairo_set_dash

void draw_track (cairo_t *cr, Track *track){
	if (track->sample_count == 0) return;

	cairo_move_to (cr, track->sample_x[0], track->sample_y[0]);

	for (int k = 1; k < track->sample_count; k++) cairo_line_to (cr, track->sample_x[k], track->sample_y[k]);

	cairo_stroke (cr);
}


void draw_road(cairo_t *cr, Road *road){
	//compute_road_tracks (road,0.2); //A DEPLACER DANS MENU
	//RIGHT A LENDROIT / LEFT A LENVERS

	//printf("DRAW ROAD | right: %d left: %d\n",road->track_right.sample_count,road->track_left.sample_count);	
	cairo_move_to(cr,road->track_right.sample_x[0],	road->track_right.sample_y[0]);
	for(int i=1; i < road->track_right.sample_count-1; i++){
		cairo_line_to(cr,road->track_right.sample_x[i],	road->track_right.sample_y[i]);
	}

	cairo_line_to(cr,road->track_left.sample_x[road->track_left.sample_count-1],	road->track_left.sample_y[road->track_left.sample_count-1]);
	for(int i=road->track_left.sample_count-2; i >= 0; i--){
		cairo_line_to(cr,road->track_left.sample_x[i],	road->track_left.sample_y[i]);
	}
	cairo_set_source_rgb(cr,0.2,0.2,0.2);
	cairo_fill(cr);
	
	//LIGNE CENTRAL
	double dashes[] = {10.0, 10.0};
	int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
	double offset = -50.0;
	
	cairo_set_source_rgb(cr,1,1,0);
	cairo_move_to(cr,road->track_right.sample_x[0],	road->track_right.sample_y[0]);
	for(int i=1; i < road->track_right.sample_count-1; i++){
		cairo_line_to(cr,road->track_right.sample_x[i],	road->track_right.sample_y[i]);
	}
	cairo_stroke(cr);
	
	cairo_move_to(cr,road->track_left.sample_x[0],	road->track_left.sample_y[0]);
	for(int i=1; i < road->track_left.sample_count-1; i++){
		cairo_line_to(cr,road->track_left.sample_x[i],	road->track_left.sample_y[i]);
	}
	cairo_stroke(cr);
	
	cairo_set_source_rgb(cr,1,1,1);
	cairo_set_dash (cr, dashes, ndash, offset);
	cairo_move_to(cr,road->track_central.sample_x[0], road->track_central.sample_y[0]);
	for(int i=1; i < road->track_central.sample_count-1; i++){
		cairo_line_to(cr,road->track_central.sample_x[i], road->track_central.sample_y[i]);
	}
	cairo_stroke (cr);
	
	//DETECTION
	cairo_set_dash(cr,0,0,0);	
	cairo_set_source_rgb(cr,1,0,0);
	for(int i=0; i < road->track_right.sample_count-1; i++){
		if(road->track_right.close[i]==1){
			cairo_move_to(cr,road->track_right.sample_x[i],	road->track_right.sample_y[i]);
			cairo_line_to(cr,road->track_right.sample_x[i+1], road->track_right.sample_y[i+1]);
			cairo_stroke(cr);
			road->track_right.close[i]=0;
		}
	}
		
	for(int i=0; i < road->track_left.sample_count-1; i++){
		if(road->track_left.close[i]==1){
			cairo_move_to(cr,road->track_left.sample_x[i-1], road->track_left.sample_y[i-1]);
			cairo_line_to(cr,road->track_left.sample_x[i], road->track_left.sample_y[i]);
			cairo_stroke(cr);
			road->track_left.close[i]=0;
		}
	}
}

void init_game (Game *game){
	game->trail=1;
	game->drift_mode=1;
	game->colision=0;
	game->car_count=0;
	game->score=0;
}