#ifndef GAME_H
#define GAME_H
#include "curve.h"

#define SAMPLE_MAX 1000
#define CAR_MAX 12

typedef enum { GS_HELLO, GS_EDIT, GS_PLAYING, GS_PAUSE, GS_WON, GS_LOST } Game_state;
typedef enum { GM_NONE, GM_SINGLE, GM_MULTI, GM_SERVER, GM_CLIENT } Game_mode;
 
typedef struct {
	int sample_count;
	int close[SAMPLE_MAX];
	double sample_x[SAMPLE_MAX], sample_y[SAMPLE_MAX];
} Track;

typedef struct {
	Curve  curve_right, curve_central, curve_left;
	Track  track_right, track_central, track_left;
} Road;

typedef struct {
	double x, y, radius;
	double angle, angle_d, speed, direction, accelerator;
	int finish;
	double trail_l_x[100];
	double trail_l_y[100];
	double trail_r_x[100];
	double trail_r_y[100];
	int trail_count;
} Car;

typedef struct {
	Game_state state;
	Game_mode  mode;
	int drift_mode;
	int colision;
	int trail;
	int score;
	Car cars[CAR_MAX];
	int car_count;
	Road road;
}Game;

void init_game (Game *game);

void compute_normal_right(double xa,double ya,double xb,double yb,double *xn,double *yn);
void do_vector_product(double xa, double ya, double za, double xb, double yb, double zb, double *xn, double *yn, double *zn);
int compute_intersection(double xs1, double ys1, double xs2, double ys2, double xt1, double yt1, double xt2, double yt2, double *xp, double *yp);
double get_horiz_angle_rad (double xa, double ya, double xb, double yb);
int point_is_on_right (double xa, double ya, double xb, double yb, double xc, double yc);
double compute_dist_point_to_line (double xa, double ya, double xb, double yb, double xc, double yc);
void progress_game_next_step (Game *game);
void place_cars_on_start (Game *game);
void draw_road(cairo_t *cr, Road *road);
void compute_road_tracks (Road *road, double theta);
int is_go_to_exterior(double xa, double ya, double xb, double yb, double xc, double yc, double xd, double yd);
int is_crossing_line(double xa, double ya, double xb, double yb, double xc, double yc, double r);
double compute_reflection_car(double xa, double ya, double xb, double yb, double angle);
int is_close_to_line (double xa, double ya, double xb, double yb, double xc, double yc, double r);
int colision_voiture(Car car1, Car car2);
double compute_colision_car(Car car1, Car car2);
void init_game (Game *game);


#endif

