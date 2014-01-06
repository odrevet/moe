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
  GdkRGBA active_color = { 1, 0, 0, 1 };
  GdkRGBA inactive_color = { 0, 1, 0, 1 };
    
  app->active_color = g_new0 (GdkRGBA, 1);
  app->inactive_color = g_new0 (GdkRGBA, 1);
  app->background_color = g_new0 (GdkRGBA, 1);
  
  app->active_color = gdk_rgba_copy (&active_color);
  app->inactive_color = gdk_rgba_copy (&inactive_color);
  app->background_color = gdk_rgba_copy (&active_color);
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
  
  app_init_colors (app);

  load_database();
}
