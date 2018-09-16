#ifndef CURVE_H
#define CURVE_H


#define CONTROL_MAX 100
#define CURVE_MAX  10

typedef struct {
	double x, y;
} Control;

typedef struct {
	int control_count;
	double shift_x;
	double shift_y;
	Control controls[CONTROL_MAX];
} Curve;

typedef struct {
	int curve_count;
	Curve curves[CURVE_MAX];
} Curve_list;

typedef struct {
	Curve_list curve_list;
	int current_curve,
	current_control;
} Curve_infos;

void init_curve_infos (Curve_infos *ci);
int find_control(Curve *c, double x, double y);
int add_curve (Curve_infos *ci);
int add_control(Curve *c, double x, double y);
int move_control(Curve *c, double dx, double dy, int current_control);
int move_curve(Curve *c, double dx, double dy);
int remove_curve (Curve *c);
int remove_control (Curve *c, int current_control);
void compute_bezier_points_open(Curve *curve, int i, Control bez_points[4]);
void compute_bezier_points_close(Curve *curve, int i, Control bez_points[4]);
void convert_bsp3_to_bezier(double p[4], double b[4]);
void convert_bsp3_to_bezier_prolong_first(double p[3], double b[4]);
void convert_bsp3_to_bezier_prolong_last(double p[3], double b[4]);
void compute_bezier_points_prolong_first(Curve *curve, Control bez_points[4]);
void compute_bezier_points_prolong_last(Curve *curve, Control bez_points[4]);
double compute_bezier_cubic(double b[4], double t);
void store_sample (double x, double y, double sx[], double sy[], int *ind, int ind_max);
void sample_bezier_curve (Control bez_points[4], double theta, double sx[], double sy[], int *ind, int ind_max, int is_first);

#endif /* CURVE_H */
