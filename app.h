#ifndef __APP__
#define __APP__
    
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>


#define UI_DEFINITIONS_FILE "ui.glade"


#define GET_UI_ELEMENT(TYPE, ELEMENT)   TYPE *ELEMENT = (TYPE *)	\
    app_get_ui_element(app, #ELEMENT);



#define DEFAULT_STROKE_SIZE 2
#define DEFAULT_GUESSES_FONT "Monospace 20"

//keeping a global track of the last point drawn
//to avoid to search it in drawing callback functions
GdkPoint *last_point;

typedef struct app_
{
  GtkBuilder *definitions;
  GSList *objects;

  GSettings *settings;

  cairo_surface_t *surface;  //back buffer
    
  GdkRGBA *background_color;
  GdkRGBA *strokes_color;

  gint stroke_size;
  
  gboolean annotate;
  gboolean auto_look_up;

  GList *strokes;     //list of strokes
  GList *curstroke;   //list of points
  
} App;
    
void app_init (App * );
GObject * app_get_ui_element (App * , const gchar * );


static gchar *
utf8_for_char (unsigned char ch[2]);

inline void
register_coord(gint16 x, gint16 y, App *app);
  
void
clear_guesses(App *app);

void
look_up (App *app);

gboolean
undo_stroke (App *app);

void
button_kanji_clicked (GtkWidget *widget, App *app);

void
drawingarea_reinit(GtkWidget *widget, App *app);

void draw_all_strokes(cairo_t *cr, App *app);

void drawing_area_refresh(App *app);
#endif
