#include "app.h"
    
GObject *
app_get_ui_element (App * app, const gchar * name)
{
  const gchar *s;
  GSList *list;
    
  list = app->objects;
    
  do {
    s = gtk_buildable_get_name (list->data);
    
    if (strcmp (s, name) == 0) {
      return list->data;
    }
    
  } while (list = g_slist_next (list));
    
  return NULL;
}
    
void
app_init_colors (App * app)
{
  app->background_color = g_new0 (GdkRGBA, 1);
  app->strokes_color = g_new0 (GdkRGBA, 1);
  app->curstroke_color = g_new0 (GdkRGBA, 1);
  
  //if no custom colors saved, fetch system colors
  GET_UI_ELEMENT (GtkWidget, window1);
  GtkStyleContext *context;
  context = gtk_widget_get_style_context (window1);

  gtk_style_context_get_background_color(context,
					 GTK_STATE_FLAG_NORMAL,
					 app->background_color);

  gtk_style_context_get_color(context,
			      GTK_STATE_FLAG_NORMAL,
			      app->strokes_color);

  gtk_style_context_get_background_color(context,
					 GTK_STATE_FLAG_SELECTED,
					 app->curstroke_color);
}
    
void
app_init (App * app)
{
  GError *err = NULL;
    
  app->definitions = gtk_builder_new ();
    
  gtk_builder_add_from_file (app->definitions,
			     UI_DEFINITIONS_FILE, &err);
    
  if (err != NULL) {
    g_printerr
      ("Error while loading app definitions file: %s\n",
       err->message);
    g_error_free (err);
    gtk_main_quit ();
  }
    
  gtk_builder_connect_signals (app->definitions, app);
    
  app->objects = gtk_builder_get_objects (app->definitions);

  app->instroke = FALSE;
  
  app->annotate = FALSE;
  app->auto_look_up = TRUE;
  
  app_init_colors (app);

  app->stroke_size = 2;

  //Set the guesses results font and size
  GET_UI_ELEMENT(GtkBox, box_guesses);
  PangoFontDescription    *fd = NULL;
  fd = pango_font_description_from_string ("Monospace 16");
  gtk_widget_modify_font (box_guesses, fd);
 
  load_database();
}

//Utils and core functions

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

void
clear_guesses(App *app)
{
  GET_UI_ELEMENT(GtkBox, box_guesses);
  
  GList *children, *iter;
  children = gtk_container_get_children(GTK_CONTAINER(box_guesses));
  for(iter = children; iter != NULL; iter = g_list_next(iter))
    gtk_widget_destroy(GTK_WIDGET(iter->data));
  g_list_free(children);
}

void
look_up (App *app)
{
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

gboolean
undo_stroke (App *app)
{
  if(!g_list_length(app->strokes) > 0){
    return FALSE;
  }
    
  GList *last_stroke = g_list_last(app->strokes);
  app->strokes = g_list_remove (app->strokes, last_stroke->data);

  //redraw the graph drawing area
  GET_UI_ELEMENT(GtkDrawingArea, drawingarea_kanji);
  gtk_widget_queue_draw(drawingarea_kanji);

  if(app->auto_look_up){
    clear_guesses(app);
    look_up(app);
  }
  
}

void
button_kanji_clicked (GtkWidget *widget, App *app)
{

  gchar* label = gtk_button_get_label(widget);
  g_print("%s", label);  //print the kanji to stdout

  //copy the kanji to the clipboard
  GtkClipboard *clipboard;
  clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  gtk_clipboard_clear(clipboard);                                         
  gtk_clipboard_set_text(clipboard, label, strlen(label));
  
}
