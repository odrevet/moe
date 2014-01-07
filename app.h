#ifndef __APP__
#define __APP__
    
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>


#define UI_DEFINITIONS_FILE "ui.glade"


#define GET_UI_ELEMENT(TYPE, ELEMENT)   TYPE *ELEMENT = (TYPE *)	\
    app_get_ui_element(app, #ELEMENT);
    
typedef struct app_
{
  GtkBuilder *definitions;
  GSList *objects;
    
  GdkRGBA *background_color;
  GdkRGBA *strokes_color;
  GdkRGBA *curstroke_color;

  gboolean annotate;
  gboolean auto_look_up;

  GList *strokes;     //list of strokes
  GList *curstroke;   //list of points
  gboolean instroke;  //a stroke is begin drawn
  
} App;
    
void app_init (App * );
GObject * app_get_ui_element (App * , const gchar * );


static gchar *
utf8_for_char (unsigned char ch[2]);


void
clear_guesses(App *app);

void
look_up (App *app);

gboolean
undo_stroke (App *app);

void
button_kanji_clicked (GtkWidget *widget, App *app);

#endif
