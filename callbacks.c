#include "app.h"

G_MODULE_EXPORT gboolean
drawingarea_kanji_draw_cb (GtkWidget *widget, cairo_t *cr, App *app)
{
  guint width, height;
  GList *stroke_list;
  gint16 x;
  gint16 y;
  int i=1;
      
  cairo_set_line_width(cr, app->stroke_size);

  //if a stroke is being drawn, draw lines between all points of the current stroke
  if(app->instroke){
    gdk_cairo_set_source_rgba (cr, app->curstroke_color);
    
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

  gdk_cairo_set_source_rgba (cr, app->strokes_color);
  
  //draw lines between all points of all strokes
  stroke_list = app->strokes;
  while (stroke_list){
    GList *point_list = stroke_list->data;

    //if annotate, display the stroke number near the first point
    if(app->annotate){
      x = ((GdkPoint *)point_list->data)->x;
      y = ((GdkPoint *)point_list->data)->y;
    

      cairo_move_to(cr, x - 10, y - 10);
      gchar stroke_number[2];             //no kanji stroke count with more than 2 digits
      sprintf(stroke_number, "%d", i);
      cairo_show_text(cr, stroke_number);
    }
    
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
    i++;
  }
  cairo_stroke(cr);

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
  GdkPoint *current_point;

  if(app->instroke && event->state & GDK_BUTTON1_MASK){
    current_point = g_new (GdkPoint, 1);
    current_point->x = event->x;
    current_point->y = event->y;

    //get the last point before inserting the current one
    //in order to know where to partially redraw
    GdkPoint *last_point = g_list_last(app->curstroke)->data;
      
    app->curstroke = g_list_append (app->curstroke, current_point);

    //partially redraw the kanji drawing area,between the prev x y to current x y
    gint draw_x, draw_y, draw_width, draw_height;

    //set the top left point of the rect to redraw
    gint line_width = 2;
    draw_x = (current_point->x < last_point->x ? current_point->x : last_point->x) - line_width; 
    draw_y = (current_point->y < last_point->y ? current_point->y : last_point->y) - line_width;
    draw_width = abs(current_point->x - last_point->x) + line_width;
    draw_height = abs(current_point->y - last_point->y) + line_width;
      
    GET_UI_ELEMENT(GtkDrawingArea, drawingarea_kanji);
    //gtk_widget_queue_draw(drawingarea_kanji);
    gtk_widget_queue_draw_area(drawingarea_kanji, draw_x, draw_y, draw_width, draw_height);

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
menuitem_annotate_toggled_cb(GtkWidget *widget, App *app) {
  gboolean annotate = gtk_check_menu_item_get_active(widget);
  app->annotate = annotate;

  //redraw the graph drawing area
  GET_UI_ELEMENT(GtkDrawingArea, drawingarea_kanji);
  gtk_widget_queue_draw(drawingarea_kanji);  
}

G_MODULE_EXPORT gboolean
menuitem_clear_activate_cb(GtkWidget *widget, App *app) {
  button_erase_clicked_cb(widget, app);
}

G_MODULE_EXPORT gboolean
button_undo_clicked_cb(GtkWidget *widget, App *app) {
  undo_stroke(app);
}

G_MODULE_EXPORT gboolean
menuitem_preferences_activate_cb(GtkWidget *widget, App *app) {
  GET_UI_ELEMENT(GtkDialog, dialog_settings);
  gint result = gtk_dialog_run(dialog_settings);

  switch (result){
  case GTK_RESPONSE_ACCEPT:
    break;
  default:
    
    break;
  }
  gtk_widget_hide (dialog_settings);  
}

G_MODULE_EXPORT gboolean
imagemenuitem_about_activate_cb(GtkWidget *widget, App *app) {
  GET_UI_ELEMENT(GtkAboutDialog, aboutdialog1);
  gtk_dialog_run(aboutdialog1);
  gtk_widget_hide (aboutdialog1);
}


// Settings dialogue
G_MODULE_EXPORT gboolean
button_ok_clicked_cb(GtkWidget *widget, App *app) {
  
}

G_MODULE_EXPORT gboolean
colorbutton_background_color_set_cb(GtkWidget *widget, App *app) {
  gtk_color_chooser_get_rgba(widget, app->background_color);
}

G_MODULE_EXPORT gboolean
colorbutton_strokes_color_set_cb(GtkWidget *widget, App *app) {
  gtk_color_chooser_get_rgba(widget, app->strokes_color);
}

G_MODULE_EXPORT gboolean
colorbutton_curstroke_color_set_cb(GtkWidget *widget, App *app) {
  gtk_color_chooser_get_rgba(widget, app->curstroke_color);
}
