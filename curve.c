#include <string.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <math.h>
#include "curve.h"

void store_sample (double x, double y, double sx[], double sy[], int *ind, int ind_max){
	if (*ind >= ind_max) {
		fprintf (stderr, "%s: capacity exceeded \n", __func__);
		return;
	}
	sx[*ind] = x;
	sy[*ind] = y;
	*ind += 1;
}

/* Ã‰chantillonne une courbe de BÃ©zier avec le pas thÃ©ta.
         * Stocke les points dans sx[0..ind_max-1], sy[0..ind_max-1] Ã  partir de ind. 
         * Au retour, ind est le nouveau point d'insertion.
         * Fct inspirÃ©e de area1.c:generate_bezier_path() du TP6 (supprimÃ©e ici)
        */

void sample_bezier_curve (Control bez_points[4], double theta, double sx[], double sy[], int *ind, int ind_max, int is_first){
	double x, y, bx[4], by[4], t;

	for (int j = 0; j <= 3 ; j++) {
		bx[j] = bez_points[j].x;
		by[j] = bez_points[j].y;
	}

	for (t = is_first ? 0.0 : theta; t < 1.0; t += theta) {
		x = compute_bezier_cubic (bx, t);
		y = compute_bezier_cubic (by, t);
		store_sample (x, y, sx, sy, ind, ind_max);
	}

	if (t < 1.0) {
		x = compute_bezier_cubic (bx, 1.0);
		y = compute_bezier_cubic (by, 1.0);
		store_sample (x, y, sx, sy, ind, ind_max);
	}
}


double compute_bezier_cubic(double b[4], double t){
	double a=pow((1.0-t),3.0),c=pow((1.0-t),2.0),d=pow(t,2.0),e=pow(t,3.0);
	return a*b[0]+3*c*t*b[1]+3*(1-t)*d*b[2]+e*b[3];
}

void convert_bsp3_to_bezier(double p[4], double b[4]){
	double unsix=(double)1/6;
	b[0]=unsix*( p[0] + 4 * p[1] + p[2] );
	b[1]=unsix*(4*p[1]+2*p[2]);
	b[2]=unsix*(2*p[1]+4*p[2]);
	b[3]=unsix*(p[1]+4*p[2]+p[3]);
}

void convert_bsp3_to_bezier_prolong_first(double p[3], double b[4]){
	double unsix=(double)1/6;
	double untrois=(double)1/3;
	b[0]=p[0];
	b[1]=untrois*(2*p[0]+p[1]);
	b[2]=untrois*(p[0]+2*p[1]);
	b[3]=unsix*(p[0]+4*p[1]+p[2]);
}

void convert_bsp3_to_bezier_prolong_last(double p[3], double b[4]){
	double unsix=(double)1/6;
	double untrois=(double)1/3;
	b[0]=unsix*(p[0]+4*p[1]+p[2]);
	b[1]=untrois*(2*p[1]+p[2]);
	b[2]=untrois*(p[1]+2*p[2]);
	b[3]=p[2];
}


void compute_bezier_points_prolong_first(Curve *curve, Control bez_points[4]){
	double px[3], py[3], bx[4], by[4];
	for(int j=0; j<3;j++){
		px[j]=curve->controls[j].x;
		py[j]=curve->controls[j].y;
	}
	convert_bsp3_to_bezier_prolong_first(px,bx);
	convert_bsp3_to_bezier_prolong_first(py,by);

	for(int j=0; j<4 ;j++){
		bez_points[j].x=bx[j];
		bez_points[j].y=by[j];
	}
}

void compute_bezier_points_prolong_last(Curve *curve, Control bez_points[4]){
	double px[3], py[3], bx[4], by[4];
	for(int j=0; j<3;j++){
		px[j]=curve->controls[j+curve->control_count-3].x;
		py[j]=curve->controls[j+curve->control_count-3].y;
	}
	convert_bsp3_to_bezier_prolong_last(px,bx);
	convert_bsp3_to_bezier_prolong_last(py,by);

	for(int j=0; j<4 ;j++){
		bez_points[j].x=bx[j];
		bez_points[j].y=by[j];
	}
}

void compute_bezier_points_open(Curve *curve, int i, Control bez_points[4]){
	double px[4], py[4], px_bez[4], py_bez[4];
	for(int j=0; j<4;j++){
		px[j]=curve->controls[i+j].x;
		py[j]=curve->controls[i+j].y;
	}
	
	convert_bsp3_to_bezier(px,px_bez);
	convert_bsp3_to_bezier(py,py_bez);
	
	for(int j=0; j<4 ;j++){
		bez_points[j].x=px_bez[j];
		bez_points[j].y=py_bez[j];
	}
}

void compute_bezier_points_close(Curve *curve, int i, Control bez_points[4]){
	double px[4], py[4], px_bez[4], py_bez[4];
	
	for(int j=0; j<4;j++){
		if(i>=curve->control_count-3){
			px[j]=curve->controls[(i+j)%curve->control_count].x;
			py[j]=curve->controls[(i+j)%curve->control_count].y;
			
		}else{
			px[j]=curve->controls[i+j].x;
			py[j]=curve->controls[i+j].y;
		}
	}
	
	convert_bsp3_to_bezier(px,px_bez);
	convert_bsp3_to_bezier(py,py_bez);

	for(int j=0; j<4 ;j++){
		bez_points[j].x=px_bez[j];
		bez_points[j].y=py_bez[j];
	}
}


void init_curve_infos (Curve_infos *ci){
	ci->curve_list.curve_count=0;
	ci->current_curve=-1;
	ci->current_control=-1;
}

//OK
int add_control(Curve *c, double x, double y){	
	if(c->control_count >= CONTROL_MAX){
		return -1;	
	}


	int k = c->control_count;
	c->control_count++;
	c->controls[k].x=x;
	c->controls[k].y=y;
	return k;
}

//OK
int find_control(Curve *c, double x, double y){
	int i;
	for(i=0; i<c->control_count;i++){
		int dx=fabs(x-c->controls[i].x);
		int dy=fabs(y-c->controls[i].y);
		if(dx*dx+dy*dy<=5*5){
			return i;
		}
	}

	return -1;
}

//OK
int move_control(Curve *c, double dx, double dy, int current_control){
	if(current_control > c->control_count-1 || current_control == -1){	
		return -1;
	}

	c->controls[current_control].x+=dx;
	c->controls[current_control].y+=dy;
	return 0;
}

//OK
int move_curve(Curve *c, double dx, double dy){
	int i;
	for(i=0; i<c->control_count; i++){
		c->controls[i].x+=dx;
		c->controls[i].y+=dy;
	}
	return 0;
}

//OK
int remove_control (Curve *c, int current_control){
	if(current_control > c->control_count-1 || current_control == -1){	
		return -1;
	}
	
	int n=current_control;
	memmove (c->controls+n, c->controls+n+1, sizeof(Control)*(c->control_count-1-n));
	c->control_count--;
	
	return 0;
}

//OKAYISH
int remove_curve (Curve *c){
	int nb = c->control_count;
	for(int i=nb; i >= 0; i--){
		remove_control (c, i);
	}
	//memmove (c, c+1, sizeof(Curve)*0); //MARCHE PAS ?
	return 0;
}