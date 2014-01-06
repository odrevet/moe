#include "app.h"

typedef struct {
  gchar d[2];
} kp_wchar;

static gchar *
utf8_for_char (unsigned char ch[2])
{
  gchar *string_utf;
  GError *err = NULL;
  gchar str[3];

  str[0] = ch[0] + 0x80;
  str[1] = ch[1] + 0x80;
  str[2] = 0;

  string_utf = g_convert (str, -1, "UTF-8", "EUC-JP", NULL, NULL, &err);
  if (!string_utf)
    {
      g_printerr ("Cannot convert string from EUC-JP to UTF-8: %s\n",
		  err->message);
      exit (1);
    }

  return string_utf;
}

void button_kanji_clicked (GtkWidget *widget, App *app){

  gchar* label = gtk_button_get_label(widget);
  g_print("%s", label);  //print the kanji to stdout

  //copy the kanji to the clipboard
  GtkClipboard *clipboard;
  clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  gtk_clipboard_clear(clipboard);                                         
  gtk_clipboard_set_text(clipboard, label, strlen(label));
  
}

void clear_guesses(App *app){
  GET_UI_ELEMENT(GtkBox, box_guesses);
  
  GList *children, *iter;
  children = gtk_container_get_children(GTK_CONTAINER(box_guesses));
  for(iter = children; iter != NULL; iter = g_list_next(iter))
    gtk_widget_destroy(GTK_WIDGET(iter->data));
  g_list_free(children);
}

void
look_up (App *app) {
   GtkWidget *button;
   GList *list_guesses = process_strokes(app->strokes);
   
   //add guesses to the UI
   GET_UI_ELEMENT(GtkBox, box_guesses);
   
   while (list_guesses){
     unsigned char *c = list_guesses->data;
     
     unsigned char kanji[2];
     sprintf(kanji, "%2x%2x", c[0], c[1]);

     gchar *kanji_utf8 = utf8_for_char(c);
       
     button = gtk_button_new_with_label (kanji_utf8);

     g_signal_connect (button, "clicked",
		       G_CALLBACK (button_kanji_clicked), app);
  
     gtk_container_add(box_guesses, button);

     gtk_widget_show (button);
     
     list_guesses = list_guesses->next;
   }
  
}

G_MODULE_EXPORT gboolean
drawingarea_kanji_draw_cb (GtkWidget *widget, cairo_t *cr, App *app)
{
  guint width, height;
  GList *stroke_list;
  gint16 x;
  gint16 y;
      
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_set_line_width(cr, 2);

  //if a stroke is being drawn, draw lines between all points of the current stroke
  if(app->instroke){
    GList *point_list = app->curstroke;
    while (point_list){
      x = ((GdkPoint *)point_list->data)->x;
      y = ((GdkPoint *)point_list->data)->y;

      cairo_move_to(cr, x, y);
      point_list = point_list->next;

      if(point_list){
	x = ((GdkPoint *)point_list->data)->x;
	y = ((GdkPoint *)point_list->data)->y;
	cairo_line_to(cr, x, y);
      }
    }
    
    cairo_stroke(cr);
  }
      
  //draw lines between all points of all strokes
  stroke_list = app->strokes;
  while (stroke_list){
    GList *point_list = stroke_list->data;
    while (point_list){
      x = ((GdkPoint *)point_list->data)->x;
      y = ((GdkPoint *)point_list->data)->y;

      cairo_move_to(cr, x, y);
      point_list = point_list->next;

      if(point_list){
	x = ((GdkPoint *)point_list->data)->x;
        y = ((GdkPoint *)point_list->data)->y;
        cairo_line_to(cr, x, y);
      }
    }
    stroke_list = stroke_list->next;
  }
  cairo_stroke(cr);
  
  gdk_cairo_set_source_rgba (cr, app->background_color);
  cairo_fill (cr);

  return FALSE;  
}

G_MODULE_EXPORT gboolean
drawingarea_kanji_button_press_event_cb(GtkWidget *widget, GdkEventButton *event, App *app)
{
  //if the mouse left button is pressed, create a new point at coord and append it to the stroke list
  if (event->button == 1){
    GdkPoint *p = g_new (GdkPoint, 1);
    p->x = event->x;
    p->y = event->y;
    app->curstroke = g_list_append (app->curstroke, p);
    app->instroke = TRUE;  //a stroke is being drawn
  }

  //redraw the graph drawing area
  GET_UI_ELEMENT(GtkDrawingArea, drawingarea_kanji);
  gtk_widget_queue_draw(drawingarea_kanji);

  return FALSE;
}

G_MODULE_EXPORT gboolean
drawingarea_kanji_button_release_event_cb(GtkWidget *widget,  GdkEventButton *event, App *app)
{
  //append the current stroke to the stroke list
  app->strokes = g_list_append (app->strokes, app->curstroke);

  //finished to draw the stroke
  app->curstroke = NULL;
  app->instroke = FALSE;

  //redraw the graph drawing area
  GET_UI_ELEMENT(GtkDrawingArea, drawingarea_kanji);
  gtk_widget_queue_draw(drawingarea_kanji);

  //if auto look up, clear the guesses and look up
  if(app->auto_look_up){
    clear_guesses(app);
    look_up(app);
  }
    
  return FALSE;
}

G_MODULE_EXPORT gboolean
drawingarea_kanji_motion_notify_event_cb(GtkWidget *widget, GdkEventMotion *event, App *app)  {
  GdkPoint *p;

  if(app->instroke && event->state & GDK_BUTTON1_MASK){
    p = g_new (GdkPoint, 1);
    p->x = event->x;
    p->y = event->y;
    app->curstroke = g_list_append (app->curstroke, p);

    //redraw the graph drawing area
    GET_UI_ELEMENT(GtkDrawingArea, drawingarea_kanji);
    gtk_widget_queue_draw(drawingarea_kanji);

  }

  return FALSE;
}


/**
   free all strokes
*/
G_MODULE_EXPORT gboolean
button_erase_clicked_cb(GtkWidget *widget, App *app)  {
  GList *tmp_list;

  tmp_list = app->strokes;  //list of points
  while (tmp_list){
    //pad_area_free_stroke (tmp_list->data);  //TODO
    tmp_list = tmp_list->next;
  }
  g_list_free (app->strokes);
  app->strokes = NULL;

  g_list_free (app->curstroke);
  app->curstroke = NULL;

  //redraw the graph drawing area
  GET_UI_ELEMENT(GtkDrawingArea, drawingarea_kanji);
  gtk_widget_queue_draw(drawingarea_kanji);

  //remove all guesses from the UI
  clear_guesses(app);
 
}

G_MODULE_EXPORT gboolean
menuitem_auto_lookup_toggled_cb(GtkWidget *widget, App *app) {
  gboolean autolookup = gtk_check_menu_item_get_active(widget);
  app->auto_look_up = autolookup;
}

G_MODULE_EXPORT gboolean
menuitem_lookup_activate_cb(GtkWidget *widget, App *app) {
  clear_guesses(app);
  look_up(app);
}


G_MODULE_EXPORT gboolean
menuitem_clear_activate_cb(GtkWidget *widget, App *app) {
  
}

