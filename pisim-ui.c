#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "pisim.h"

#define PROG_NAME "π Simulador"

#define PISIM_TYPE_APP (pisim_app_get_type())

#define PISIM_TYPE_WINDOW (pisim_app_window_get_type())
#define PISIM_APP_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), PISIM_TYPE_WINDOW, PisimAppWindow))
#define PISIM_WINDOW_CLASS(class) (G_TYPE_CHECK_CLASS_CAST((class), PISIM_TYPE_WINDOW, PisimAppWindowClass))

typedef struct _data_t {
	cairo_surface_t * plot;
	point_t * points;
	size_t size;
	size_t _max_size;
	long a, b, m;
} data_t;

#define DATA_INIT ((data_t) {NULL, NULL, 0, 0, MULTIP, ADITIV, MODULO})


static void draw_graph (GtkDrawingArea * drawing_area, cairo_t * cr,
                                   int width, int height, gpointer window);
static void parameter_change(GtkAdjustment * adj, gpointer window);

typedef struct _PisimAppWindow {
	GtkApplicationWindow parent;
	GtkWidget * canvas;
	GtkWidget * size_value;
	GtkWidget * pi_label;
	data_t context;
} PisimAppWindow;

typedef struct _PisimAppWindowClass {
	GtkApplicationWindowClass base_class;
} PisimAppWindowClass;

G_DEFINE_TYPE(PisimAppWindow, pisim_app_window, GTK_TYPE_APPLICATION_WINDOW);

static void pisim_app_window_init(PisimAppWindow * window) {
	GtkWidget * content;
	GtkWidget * draw;
	GtkWidget * control;
	GtkWidget * size_label;
	GtkAdjustment * size_adjust;
	GtkWidget * size_slider;
	GtkWidget * size_value;
	GtkWidget * pi_box;
	GtkWidget * pi_label;
	window->context = DATA_INIT;

	gtk_window_set_title(GTK_WINDOW(window), PROG_NAME);

	content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_set_homogeneous(GTK_BOX(content), FALSE);
	gtk_window_set_child(GTK_WINDOW(window), content);
	gtk_widget_show(content);

	draw = gtk_drawing_area_new();
	window->canvas = draw;
	gtk_widget_set_vexpand(draw, TRUE);
	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(draw), draw_graph,
	                               window, NULL);

	gtk_box_prepend(GTK_BOX(content), draw);

	control = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_set_homogeneous(GTK_BOX(control), TRUE);
	gtk_box_append(GTK_BOX(content), control);

	size_label = gtk_label_new("Nº de pontos");
	/* gtk_widget_set_halign(size_label, GTK_ALIGN_END); */
	gtk_label_set_xalign(GTK_LABEL(size_label), 1.0);
	gtk_box_append(GTK_BOX(control), size_label);

	size_adjust = gtk_adjustment_new(100, 1, 1500, 100, 100, 0);
	parameter_change(size_adjust, window);

	size_slider = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, size_adjust);
	gtk_scale_set_digits(GTK_SCALE(size_slider), 0);
	gtk_scale_set_has_origin(GTK_SCALE(size_slider), TRUE);
	gtk_box_append(GTK_BOX(control), size_slider);

	size_value = gtk_label_new("100");
	window->size_value = size_value;
	gtk_label_set_xalign(GTK_LABEL(size_value), 0.0);
	gtk_box_append(GTK_BOX(control), size_value);

	g_signal_connect(size_adjust, "value-changed", G_CALLBACK(parameter_change), window);

	pi_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_box_set_homogeneous(GTK_BOX(pi_box), TRUE);
	gtk_box_append(GTK_BOX(content), pi_box);

	pi_label = gtk_label_new("π ≈ ...");
	window->pi_label = pi_label;
	gtk_box_append(GTK_BOX(pi_box), pi_label);
}

static void draw_graph (GtkDrawingArea * drawing_area, cairo_t * cr,
                                   int width, int height, gpointer window) {
	int unit = MIN(width,height)/2.0;
	int center_x = width/2.0;
	int center_y = height/2.0;
	cairo_surface_t * mask;

	data_t * context = &((PisimAppWindow *) window)->context;

	cairo_set_source_rgba(cr, 0.06, 0.06, 0.06, 1.0);
	cairo_paint(cr);

	cairo_set_source_rgba(cr, 0.5, 0.5, 0.8, 1.0);
	cairo_set_line_width(cr, 2.0);
	cairo_rectangle(cr, center_x - unit, center_y - unit, 2*unit, 2*unit);
	cairo_stroke(cr);

	cairo_set_source_rgba(cr, 1, 1, 1, 0.5);
	cairo_set_line_width(cr, 1.0);
	cairo_arc(cr, center_x, center_y, unit, 0, 2*G_PI);
	cairo_stroke(cr);

	cairo_set_source_rgba(cr, 1, 0.2, 0.2, 0.6);
	cairo_set_line_width(cr, 1);
	cairo_move_to(cr, center_x, 0);
	cairo_rel_line_to(cr, 0, height);
	cairo_move_to(cr, 0, center_y);
	cairo_rel_line_to(cr, width, 0);
	cairo_stroke(cr);

	/* mask = cairo_surface_j */
	cairo_set_source_rgba(cr, 0.1, 0.1, 1.0, 0.8);
	cairo_mask_surface(cr, context->plot, 0, 0);
}

static void parameter_change(GtkAdjustment * adj, gpointer window) {
	double pi;
	size_t n = gtk_adjustment_get_value(adj);
	size_t max = gtk_adjustment_get_upper(adj);
	data_t * context = &((PisimAppWindow *) window)->context;
	GtkWidget * size_value = ((PisimAppWindow *) window)->size_value;
	GtkWidget * pi_label = ((PisimAppWindow *) window)->pi_label;
	context->size = n;
	point_t * p;
	cairo_t * cr;
	GtkWidget * draw = ((PisimAppWindow *) window)->canvas;
	int width = gtk_widget_get_width(draw),
		height = gtk_widget_get_height(draw),
		unit = MIN(height, width)/2.0;
	GtkNative * native = gtk_widget_get_native(draw);

	char value[32];
	snprintf(value, 32, "%zu", n);
	gtk_label_set_text(GTK_LABEL(size_value),
	                   value);

	if (context->_max_size == 0 && context->_max_size != max){
		context->_max_size = max;
		context->points = malloc(max * sizeof(point_t));
	}

	pi = calcpi(max, context->a, context->b, context->m, context->points);
	snprintf(value, 32, "π ≈ %10.8lf", pi);
	gtk_label_set_text(GTK_LABEL(pi_label),
	                   value);

	p = context->points;
	printf("%lf  %zu  %zu  %lf\n",pi, n, max, p[max-1].x);

	if (context->plot != NULL) cairo_surface_destroy(context->plot);
	context->plot = cairo_image_surface_create(CAIRO_FORMAT_A8, width, height);

	cr = cairo_create(context->plot);

	cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
	cairo_set_line_width(cr, 0);
	for (size_t i = 0; i < n; i++) {
		cairo_arc(cr, width/2.0 + p[i].x * unit,
					  height/2.0 + p[i].y * unit,
					  2.0, 0, 2*G_PI
				 );
		cairo_close_path(cr);
	}
	cairo_fill(cr);

	cairo_destroy(cr);

	gtk_widget_queue_draw(GTK_WIDGET(draw));
}

static void pisim_app_window_class_init(PisimAppWindowClass * class) {
}

typedef struct _PisimApp {
	GtkApplication parent;
} PisimApp;

typedef struct _PisimAppClass {
	GtkApplicationClass base_class;
} PisimAppClass;

G_DEFINE_TYPE(PisimApp, pisim_app, GTK_TYPE_APPLICATION);

static void pisim_app_init(PisimApp * app) {
}

static void pisim_app_activate(GApplication * app){
	PisimAppWindow * window;
	GtkWidget * header;

	window = g_object_new(PISIM_TYPE_WINDOW,
             "application", app,
             NULL);

	header = gtk_header_bar_new();
	gtk_window_set_titlebar(GTK_WINDOW(window), header);
	gtk_window_set_default_size(GTK_WINDOW(window), 500, 400);

	gtk_window_present(GTK_WINDOW(window));
}

static void pisim_app_class_init(PisimAppClass * class) {
	G_APPLICATION_CLASS(class)->activate = pisim_app_activate;
}


int main(int argc, char *argv[]) {
	PisimApp * pisim;
	int status;

	pisim = g_object_new(PISIM_TYPE_APP,
	                     "application-id", "br.pisim",
	                     "flags", G_APPLICATION_FLAGS_NONE,
	                     NULL);

	status = g_application_run(G_APPLICATION(pisim), argc, argv);
	g_object_unref(pisim);

	return status;
}
