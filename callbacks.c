#include "app.h"


//Drawing area callbacks 
G_MODULE_EXPORT gboolean
drawingarea_kanji_configure_event_cb(GtkWidget         *widget,
				     GdkEventConfigure *event,
				     App *app)
{
  drawingarea_reinit(widget, app);
  return TRUE;
}


G_MODULE_EXPORT gboolean
drawingarea_kanji_draw_cb (GtkWidget *widget, cairo_t *cr, App *app)
{
  cairo_set_source_surface (cr, app->surface, 0, 0);
  cairo_paint (cr);

  return FALSE;
}

G_MODULE_EXPORT gboolean
drawingarea_kanji_button_press_event_cb(GtkWidget *widget, GdkEventButton *event, App *app)
{
  //if the mouse left button is pressed, create a new point at coord and append
  //it to the stroke list
  if (event->button == 1)register_coord(event->x, event->y, app);

  return FALSE;
}

G_MODULE_EXPORT gboolean
drawingarea_kanji_button_release_event_cb(GtkWidget *widget,  GdkEventButton *event, App *app)
{
  //append the current stroke to the stroke list
  app->strokes = g_list_append (app->strokes, app->curstroke);

  //finished to draw the stroke
  app->curstroke = NULL;

  //if auto look up, clear the guesses and look up
  if(app->auto_look_up){
    clear_guesses(app);
    look_up(app);
  }

  //if annotate, display the stroke number near the first point
  if(app->annotate){  
      drawing_area_refresh(app);
  }

    
  return FALSE;
}


G_MODULE_EXPORT gboolean
drawingarea_kanji_motion_notify_event_cb(GtkWidget *widget, GdkEventMotion *event, App *app)  {
  int x, y;
  GdkModifierType state;

  if (app->surface == NULL)
    return FALSE; //paranoia check, in case we haven't gotten a configure event

  gdk_window_get_device_position (event->window, event->device, &x, &y, &state);

  if(event->state & GDK_BUTTON1_MASK){
    
    //partially redraw the kanji drawing area,between the prev x y to current x y
    //set the top left point of the rect to redraw
    GdkRectangle update_rect;
    gint line_width = app->stroke_size;
    
    update_rect.x = (x < last_point->x ? x : last_point->x) - line_width / 2; 
    update_rect.y = (y < last_point->y ? y : last_point->y) - line_width / 2;
    update_rect.width = abs(x - last_point->x) + line_width;
    update_rect.height = abs(y - last_point->y) + line_width;

    /* Paint to the surface, where we store our state */
    cairo_t *cr = cairo_create (app->surface);
  
    //line
    cairo_set_line_width(cr, line_width);
    gdk_cairo_set_source_rgba (cr, app->strokes_color);
    cairo_move_to(cr, last_point->x, last_point->y);
    cairo_line_to(cr, x, y);
    cairo_stroke(cr);

    //DEBUG: draw invalidation rect
    /*GdkRGBA debug_color = {1, 0, 0, 1}; 
    gdk_cairo_set_source_rgba (cr, &debug_color);
    cairo_rectangle(cr, update_rect.x, update_rect.y, update_rect.width, update_rect.height);
    cairo_set_line_width(cr, 1);
    cairo_stroke(cr);*/
  
    cairo_destroy (cr);

    /* Now invalidate the affected region of the drawing area. */
    gdk_window_invalidate_rect (gtk_widget_get_window (widget),
				&update_rect,
				FALSE);


    // finished drawing, now register the point
    register_coord(x, y, app);
  }

  return TRUE;
}



//Button callbacks
/**
   free all strokes
*/
G_MODULE_EXPORT gboolean
button_erase_clicked_cb(GtkWidget *widget, App *app)  {
  GList *tmp_list;

  tmp_list = app->strokes;
  while (tmp_list){
    //pad_area_free_stroke (tmp_list->data);  //TODO
    tmp_list = tmp_list->next;
  }
  g_list_free (app->strokes);
  app->strokes = NULL;

  g_list_free (app->curstroke);
  app->curstroke = NULL;

  //remove all guesses from the UI
  clear_guesses(app);
 
}

G_MODULE_EXPORT gboolean
button_undo_clicked_cb(GtkWidget *widget, App *app) {
  undo_stroke(app);
  drawing_area_refresh(app);
}


//Menu callbacks
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

  GET_UI_ELEMENT(GtkDrawingArea, drawingarea_kanji);
  drawingarea_reinit(drawingarea_kanji, app);
  gtk_widget_queue_draw(drawingarea_kanji);    
 
}

G_MODULE_EXPORT gboolean
menuitem_clear_activate_cb(GtkWidget *widget, App *app) {
  button_erase_clicked_cb(widget, app);
}

G_MODULE_EXPORT gboolean
menuitem_preferences_activate_cb(GtkWidget *widget, App *app) {
  GET_UI_ELEMENT(GtkDialog, dialog_settings);
  gtk_dialog_run(dialog_settings);
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
  GET_UI_ELEMENT(GtkDialog, dialog_settings);
  gtk_widget_hide (dialog_settings);   
}

G_MODULE_EXPORT gboolean
colorbutton_background_color_set_cb(GtkWidget *widget, App *app) {
  gtk_color_chooser_get_rgba(widget, app->background_color);
  drawing_area_refresh(app);
}

G_MODULE_EXPORT gboolean
colorbutton_strokes_color_set_cb(GtkWidget *widget, App *app) {
  gtk_color_chooser_get_rgba(widget, app->strokes_color);
  drawing_area_refresh(app);   
}

G_MODULE_EXPORT gboolean
fontbutton_guesses_font_set_cb(GtkWidget *widget, App *app) {

  gchar* font_name = gtk_font_button_get_font_name(widget);
    
  GET_UI_ELEMENT(GtkBox, box_guesses);
  PangoFontDescription    *fd = NULL;
  fd = pango_font_description_from_string (font_name);
  gtk_widget_modify_font (box_guesses, fd);
}
