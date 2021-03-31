#ifndef ENGINE
#define ENGINE

#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "jstroke/jstroke.h"

#define MAX_STROKES 32

static char *stroke_dicts[MAX_STROKES];
static char *data_file;

void load_database();

GList *
process_strokes(GList *stroke_list);

#endif
