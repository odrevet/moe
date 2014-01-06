#ifndef ENGINE
#define ENGINE
    
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>


void
load_database();

GList*
process_strokes (GList *stroke_list);


#endif
