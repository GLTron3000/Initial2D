#ifndef AREA1_H
#define AREA1_H

#include "game.h"
#include "mydata.h"
#define ANIM1_TIME 20

void apply_image_transforms (gpointer data);
void update_area1_with_transforms (gpointer data);
void area1_init(gpointer data);
void draw_bezier_curve(cairo_t *cr, Control bez_points[4], double theta);
void generate_bezier_path(cairo_t *cr, Control bez_points[4], double theta, int is_first);
void generate_control_road(Road *road, double d);
void draw_bezier_curves_prolong(cairo_t *cr, Road *road, double theta);
void draw_control_labels(cairo_t *cr, PangoLayout *layout, Road *road);
void draw_control_polygons(cairo_t *cr, Road *road);
void set_anim1_mode(Mydata *my, int flag);

#endif /* AREA1_H */
