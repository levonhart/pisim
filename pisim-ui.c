#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "pisim.h"

#define PROG_NAME "π Simulador"
#define NPOINTS 100
#define MAXPTS 1500
#define SMULTIP 0x9696969696
#define SADITIV 0x00800AAAAAAAAAABp1
#define SMODULO 0x07FFFFFFFFFFFFFFp1
#define SLIDERSTEPS 30

#define SURFACERES 1080

#define PISIM_TYPE_APP (pisim_app_get_type())
#define PISIM_APP(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), PISIM_TYPE_APP, PisimApp))
#define PISIM_APP_CLASS(class) (G_TYPE_CHECK_CLASS_CAST((class), PISIM_TYPE_APP, PisimAppClass))

#define PISIM_TYPE_WINDOW (pisim_app_window_get_type())
#define PISIM_APP_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), PISIM_TYPE_WINDOW, PisimAppWindow))
#define PISIM_WINDOW_CLASS(class) (G_TYPE_CHECK_CLASS_CAST((class), PISIM_TYPE_WINDOW, PisimAppWindowClass))

const char init_labels[] = "100\0"                       // 0 - 3
							"165573515712001\0"          // 4 - 19
							"36040525142993579\0"        // 20 - 37
							"1152921504606846975\0";     // 38 - 57

typedef struct _data_t {
	cairo_surface_t * plot;
	point_t * points;
	size_t size;
	size_t _max_size;
	long a, b, m;
} data_t;

#define DATA_INIT ((data_t) {NULL, NULL, 0, 0, MULTIP, ADITIV, MODULO})

typedef struct _PisimAppWindow {
	GtkApplicationWindow parent;
	GtkWidget * canvas;
	GtkWidget * size_value;
	GtkWidget * multip_value;
	GtkWidget * aditiv_value;
	GtkWidget * modulo_value;
	GtkWidget * pi_label;

	data_t context;
} PisimAppWindow;

typedef struct _PisimAppWindowClass {
	GtkApplicationWindowClass base_class;
} PisimAppWindowClass;

G_DEFINE_TYPE(PisimAppWindow, pisim_app_window, GTK_TYPE_APPLICATION_WINDOW);

static void draw_graph (GtkDrawingArea * drawing_area, cairo_t * cr,
                                   int width, int height, gpointer window);
static void size_change(GtkAdjustment * adj, gpointer window);
static void multip_change(GtkAdjustment * adj, gpointer window);
static void aditiv_change(GtkAdjustment * adj, gpointer window);
static void modulo_change(GtkAdjustment * adj, gpointer window);

static void parameter_update(PisimAppWindow * window);

static void pisim_app_window_dispose(GObject * object) {
	PisimAppWindow * window = PISIM_APP_WINDOW(object);

	if (window->context.plot != NULL) {
		cairo_surface_destroy(window->context.plot);
	}
	if (window->context.points != NULL) {
		free(window->context.points);
	}

	window->context = DATA_INIT;

	G_OBJECT_CLASS(pisim_app_window_parent_class)->dispose(object);
}

/* static void pisim_app_window_finalize(GObject * object) { */
/*     PisimAppWindow * window = PISIM_APP_WINDOW(object); */
/*  */
/*     G_OBJECT_CLASS(pisim_app_window_parent_class)->finalize(object); */
/* } */

static void pisim_app_window_init(PisimAppWindow * window) {
	GtkWidget * content;
	GtkWidget * draw;
	GtkWidget * control;
	GtkWidget * size_label;
	GtkWidget * multip_label;
	GtkWidget * aditiv_label;
	GtkWidget * modulo_label;
	GtkAdjustment * size_adjust;
	GtkAdjustment * multip_adjust;
	GtkAdjustment * aditiv_adjust;
	GtkAdjustment * modulo_adjust;
	GtkWidget * size_slider;
	GtkWidget * size_value;
	GtkWidget * multip_slider;
	GtkWidget * multip_value;
	GtkWidget * aditiv_slider;
	GtkWidget * aditiv_value;
	GtkWidget * modulo_slider;
	GtkWidget * modulo_value;
	GtkWidget * pi_box;
	GtkWidget * pi_label;
	window->context = DATA_INIT;

	window->context.points = malloc(MAXPTS * sizeof(point_t));
	window->context._max_size = MAXPTS;

	gtk_window_set_title(GTK_WINDOW(window), PROG_NAME);

	content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	/* window->content = content; */
	gtk_box_set_homogeneous(GTK_BOX(content), FALSE);
	gtk_window_set_child(GTK_WINDOW(window), content);
	gtk_widget_show(content);

	draw = gtk_drawing_area_new();
	window->canvas = draw;
	/* g_object_ref(draw); */
	gtk_widget_set_vexpand(draw, TRUE);
	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(draw), draw_graph,
	                               window, NULL);

	gtk_box_prepend(GTK_BOX(content), draw);

	control = gtk_grid_new();
	/* window->control = control; */
	gtk_grid_set_column_homogeneous(GTK_GRID(control), TRUE);
	gtk_box_append(GTK_BOX(content), control);

	size_label = gtk_label_new("Nº de pontos");
	/* window->size_label = size_label; */
	/* gtk_widget_set_halign(size_label, GTK_ALIGN_END); */
	gtk_label_set_xalign(GTK_LABEL(size_label), 1.0);
	gtk_grid_attach(GTK_GRID(control), size_label, 0, 0, 1, 1);

	size_adjust = gtk_adjustment_new(NPOINTS, 1, MAXPTS,
			MAXPTS/SLIDERSTEPS, 10 * MAXPTS/SLIDERSTEPS, 0);
	/* window->size_adjust = size_adjust; */

	size_slider = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, size_adjust);
	/* window->size_slider = size_slider; */
	gtk_scale_set_digits(GTK_SCALE(size_slider), 0);
	gtk_scale_set_has_origin(GTK_SCALE(size_slider), TRUE);
	gtk_grid_attach(GTK_GRID(control), size_slider, 1, 0, 1, 1);

	size_value = gtk_label_new(init_labels);
	window->size_value = size_value;
	/* g_object_ref(size_value); */
	gtk_label_set_xalign(GTK_LABEL(size_value), 0.0);
	gtk_label_set_selectable(GTK_LABEL(size_value), TRUE);
	gtk_grid_attach(GTK_GRID(control), size_value, 2, 0, 1, 1);

	g_signal_connect(size_adjust, "value-changed", G_CALLBACK(size_change), window);


	multip_label = gtk_label_new("Multiplicador");
	/* window->multip_label = multip_label; */
	gtk_label_set_xalign(GTK_LABEL(multip_label), 1.0);
	gtk_grid_attach(GTK_GRID(control), multip_label, 0, 1, 1, 1);

	multip_adjust = gtk_adjustment_new(SMULTIP, 1, 2 * SMULTIP,
			2 * SMULTIP / SLIDERSTEPS, 10 * 2 * SMULTIP / SLIDERSTEPS, 0);
	/* window->multip_adjust = multip_adjust; */
	/* multip_change(multip_adjust, window); */

	multip_slider = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, multip_adjust);
	/* window->multip_slider = multip_slider; */
	gtk_scale_set_digits(GTK_SCALE(multip_slider), 0);
	gtk_scale_set_has_origin(GTK_SCALE(multip_slider), TRUE);
	gtk_grid_attach(GTK_GRID(control), multip_slider, 1, 1, 1, 1);

	multip_value = gtk_label_new(init_labels + 4);
	window->multip_value = multip_value;
	/* g_object_ref(multip_value); */
	gtk_label_set_xalign(GTK_LABEL(multip_value), 0.0);
	gtk_label_set_selectable(GTK_LABEL(multip_value), TRUE);
	gtk_grid_attach(GTK_GRID(control), multip_value, 2, 1, 1, 1);

	g_signal_connect(multip_adjust, "value-changed", G_CALLBACK(multip_change), window);

	aditiv_label = gtk_label_new("Aditivo");
	/* window->aditiv_label = aditiv_label; */
	gtk_label_set_xalign(GTK_LABEL(aditiv_label), 1.0);
	gtk_grid_attach(GTK_GRID(control), aditiv_label, 0, 2, 1, 1);

	aditiv_adjust = gtk_adjustment_new(SADITIV, 1, 2 * SADITIV,
			2 * SADITIV/SLIDERSTEPS, 10 * 2 * SADITIV/SLIDERSTEPS, 0);
	/* window->aditiv_adjust = aditiv_adjust; */
	/* aditiv_change(aditiv_adjust, window); */

	aditiv_slider = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, aditiv_adjust);
	/* window->aditiv_slider = aditiv_slider; */
	gtk_scale_set_digits(GTK_SCALE(aditiv_slider), 0);
	gtk_scale_set_has_origin(GTK_SCALE(aditiv_slider), TRUE);
	gtk_grid_attach(GTK_GRID(control), aditiv_slider, 1, 2, 1, 1);

	aditiv_value = gtk_label_new(init_labels + 20);
	window->aditiv_value = aditiv_value;
	/* g_object_ref(aditiv_value); */
	gtk_label_set_xalign(GTK_LABEL(aditiv_value), 0.0);
	gtk_label_set_selectable(GTK_LABEL(aditiv_value), TRUE);
	gtk_grid_attach(GTK_GRID(control), aditiv_value, 2, 2, 1, 1);

	g_signal_connect(aditiv_adjust, "value-changed", G_CALLBACK(aditiv_change), window);

	modulo_label = gtk_label_new("Módulo");
	/* window->modulo_label = modulo_label; */
	gtk_label_set_xalign(GTK_LABEL(modulo_label), 1.0);
	gtk_grid_attach(GTK_GRID(control), modulo_label, 0, 3, 1, 1);

	modulo_adjust = gtk_adjustment_new(SMODULO, 1, 2 * SMODULO,
			2 * SMODULO/SLIDERSTEPS, 10 * 2 * SMODULO/SLIDERSTEPS, 0);
	/* window->modulo_adjust = modulo_adjust; */
	/* modulo_change(modulo_adjust, window); */

	modulo_slider = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, modulo_adjust);
	/* window->modulo_slider = modulo_slider; */
	gtk_scale_set_digits(GTK_SCALE(modulo_slider), 0);
	gtk_scale_set_has_origin(GTK_SCALE(modulo_slider), TRUE);
	gtk_grid_attach(GTK_GRID(control), modulo_slider, 1, 3, 1, 1);

	modulo_value = gtk_label_new(init_labels + 38);
	window->modulo_value = modulo_value;
	/* g_object_ref(modulo_value); */
	gtk_label_set_xalign(GTK_LABEL(modulo_value), 0.0);
	gtk_label_set_selectable(GTK_LABEL(modulo_value), TRUE);
	gtk_grid_attach(GTK_GRID(control), modulo_value, 2, 3, 1, 1);

	g_signal_connect(modulo_adjust, "value-changed", G_CALLBACK(modulo_change), window);

	pi_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	/* window->pi_box = pi_box; */
	gtk_box_set_homogeneous(GTK_BOX(pi_box), TRUE);
	gtk_box_append(GTK_BOX(content), pi_box);

	pi_label = gtk_label_new("π ≈ ...");
	window->pi_label = pi_label;
	/* g_object_ref(pi_label); */
	gtk_label_set_selectable(GTK_LABEL(pi_label), TRUE);
	gtk_box_append(GTK_BOX(pi_box), pi_label);

	gtk_adjustment_set_value(size_adjust, NPOINTS);
	size_change(size_adjust,window);
}

static void draw_graph (GtkDrawingArea * drawing_area, cairo_t * cr,
                                   int width, int height, gpointer window) {
	int unit = MIN(width,height)/2.0;
	int center_x = width/2.0;
	int center_y = height/2.0;
	double scale = 2. * unit/SURFACERES;
	cairo_surface_t * mask;

	data_t * context = &PISIM_APP_WINDOW(window)->context;

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
	cairo_scale(cr, scale, scale);
	cairo_mask_surface(cr, context->plot, (center_x - unit)/scale,
	                                      (center_y - unit)/scale);
}

static void size_change(GtkAdjustment * adj, gpointer window) {
	size_t n = gtk_adjustment_get_value(adj),
		   max = gtk_adjustment_get_upper(adj);
	char value[8];
	GtkWidget * size_value = PISIM_APP_WINDOW(window)->size_value;
	data_t * context = &PISIM_APP_WINDOW(window)->context;

	context->size = n;

	snprintf(value, 8, "%zu", n);
	gtk_label_set_text(GTK_LABEL(size_value), value);

	parameter_update(PISIM_APP_WINDOW(window));
}

static void multip_change(GtkAdjustment * adj, gpointer window) {
	long a = 1 + 4 * gtk_adjustment_get_value(adj);
	char value[32];
	GtkWidget * multip_value = PISIM_APP_WINDOW(window)->multip_value;
	data_t * context = &PISIM_APP_WINDOW(window)->context;

	snprintf(value, 32, "%ld", a);
	gtk_label_set_text(GTK_LABEL(multip_value), value);

	parameter_update(PISIM_APP_WINDOW(window));
}

static void aditiv_change(GtkAdjustment * adj, gpointer window) {
	long b = gtk_adjustment_get_value(adj);
	char value[32];
	GtkWidget * aditiv_value = PISIM_APP_WINDOW(window)->aditiv_value;
	data_t * context = &PISIM_APP_WINDOW(window)->context;

	snprintf(value, 32, "%ld", b);
	gtk_label_set_text(GTK_LABEL(aditiv_value), value);

	parameter_update(PISIM_APP_WINDOW(window));
}

static void modulo_change(GtkAdjustment * adj, gpointer window) {
	long m = 1 + 2 * gtk_adjustment_get_value(adj);
	char value[32];
	GtkWidget * modulo_value = PISIM_APP_WINDOW(window)->modulo_value;
	data_t * context = &PISIM_APP_WINDOW(window)->context;

	snprintf(value, 32, "%ld", m);
	gtk_label_set_text(GTK_LABEL(modulo_value), value);

	parameter_update(PISIM_APP_WINDOW(window));
}

static void parameter_update(PisimAppWindow * window) {
	double pi;
	char value[32];
	GtkWidget * pi_label = window->pi_label;
	data_t * context = &(window->context);
	size_t n = context->size,
		   max = context->_max_size;
	point_t * p;
	cairo_t * cr;
	GtkWidget * draw = window->canvas;
	int width = SURFACERES, height = SURFACERES,
		unit = MIN(height, width)/2.0;
	GtkNative * native = gtk_widget_get_native(draw);


	pi = calcpi(context->_max_size, context->a, context->b,
			    context->m, context->points);
	snprintf(value, 32, "π ≈ %10.8lf", pi);
	gtk_label_set_text(GTK_LABEL(pi_label),
	                   value);

	p = context->points;
	printf("%lf  %zu  %zu  %lf\n",pi, context->size, context->_max_size, p[context->_max_size-1].x);

	if (context->plot != NULL) cairo_surface_destroy(context->plot);
	context->plot = cairo_image_surface_create(CAIRO_FORMAT_A8, width, height);

	cr = cairo_create(context->plot);

	cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
	cairo_set_line_width(cr, 0);
	for (size_t i = 0; i < context->size; i++) {
		cairo_arc(cr, width/2.0 + p[i].x * unit,
					  height/2.0 + p[i].y * unit,
					  5.0, 0, 2*G_PI
				 );
		cairo_close_path(cr);
	}
	cairo_fill(cr);

	cairo_destroy(cr);

	gtk_widget_queue_draw(GTK_WIDGET(draw));
}

static void pisim_app_window_class_init(PisimAppWindowClass * class) {
	GObjectClass * g_object_class = G_OBJECT_CLASS(class);

	g_object_class->dispose = pisim_app_window_dispose;
}

typedef struct _PisimApp {
	GtkApplication parent;
	PisimAppWindow * main;
	GtkWidget * header;
} PisimApp;

typedef struct _PisimAppClass {
	GtkApplicationClass base_class;
} PisimAppClass;

G_DEFINE_TYPE(PisimApp, pisim_app, GTK_TYPE_APPLICATION);

static void pisim_app_init(PisimApp * app) {
}

/* static void pisim_app_dispose(GObject * object) { */
/*     PisimApp * app = PISIM_APP(object); */
/*     g_clear_object(&app->main); */
/*     g_clear_object(&app->header); */
/*  */
/*     G_OBJECT_CLASS(pisim_app_parent_class)->dispose(object); */
/* } */

static void pisim_app_activate(GApplication * app){
	PisimAppWindow * window;
	GtkWidget * header;

	window = g_object_new(PISIM_TYPE_WINDOW,
             "application", app,
             NULL);
	PISIM_APP(app)->main = window;
	/* g_object_ref(window); */

	header = gtk_header_bar_new();
	PISIM_APP(app)->header = header;
	/* g_object_ref(header); */

	gtk_window_set_titlebar(GTK_WINDOW(window), header);
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

	gtk_window_present(GTK_WINDOW(window));
}

static void pisim_app_class_init(PisimAppClass * class) {
	/* GObjectClass * g_object_class = G_OBJECT_CLASS(class); */
	GApplicationClass * application_class = G_APPLICATION_CLASS(class);

	application_class->activate = pisim_app_activate;
	/* g_object_class->dispose = pisim_app_dispose; */
}


int main(int argc, char *argv[]) {
	PisimApp * pisim;
	int status;

	pisim = g_object_new(PISIM_TYPE_APP,
	                     "application-id", "br.pisim",
	                     "flags", G_APPLICATION_FLAGS_NONE,
	                     NULL);

	status = g_application_run(G_APPLICATION(pisim), argc, argv);
	/* g_object_unref(pisim->main); */
	/* g_object_unref(pisim->header); */
	g_object_unref(pisim);

	return status;
}
