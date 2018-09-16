#include <gtk/gtk.h>
#include <stdarg.h>
#include "utils.h"

void refresh_area (GtkWidget *area){
	GdkWindow *win = gtk_widget_get_window (area);
	if (win == NULL) return; 
	gdk_window_invalidate_rect (win,NULL,FALSE);
}

void set_status (GtkWidget *status, const char *format, ...){
	char buf[1000];
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, sizeof(buf)-1,format,ap);
	va_end(ap);
	buf[sizeof(buf)-1]=0;
	gtk_statusbar_pop(GTK_STATUSBAR(status),0);
	gtk_statusbar_push(GTK_STATUSBAR(status),0,buf);
}

double min(double x, double y){
	double a = x<y ?x:y ;
	return a; 
}

double max(double x, double y){
	double a = x>y ?x:y ;
	return a; 
}

/*
GdkPixbuf * get_screenshot(){
    GdkPixbuf *screenshot;
    GdkWindow *root_window;
    gint x_orig, y_orig;
    gint width, height;
    root_window = gdk_get_default_root_window ();
    gdk_drawable_get_size (root_window, &width, &height);      
    gdk_window_get_origin (root_window, &x_orig, &y_orig);

    screenshot = gdk_pixbuf_get_from_drawable (NULL, root_window, NULL,
                                           x_orig, y_orig, 0, 0, width, height);
    return screenshot;
}*/
