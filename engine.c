/* KanjiPad - Japanese handwriting recognition front end
 * Copyright (C) 1997 Owen Taylor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include "jstroke/jstroke.h"

#define MAX_STROKES 32
#define BUFLEN 1024

static char *stroke_dicts[MAX_STROKES];
static char *progname;
static char *data_file;

void
load_database()
{
  FILE *file;
  int i;

  if (data_file)
    {
      file = fopen (data_file, "rb");
    }
  else
    {
      char *dir = g_strdup ("./");  //KP_LIBDIR
      char *fname = g_build_filename (dir, "jdata.dat", NULL);
      file = fopen (fname, "rb");
      
      if (!file)
	file = fopen ("jdata.dat", "rb");

      g_free (fname);
      g_free (dir);
    }

  if (!file)
    {
      fprintf(stderr,"%s: Can't open %s\n", progname,
	      data_file ? data_file : "jdata.dat");
      exit(1);
    }


  for (i=0;i<MAX_STROKES;i++)
    stroke_dicts[i] = NULL;

  while (1)
    {
      int n_read;
      unsigned int nstrokes;
      unsigned int len;
      int buf[2];

      n_read = fread (buf, sizeof(int), 2, file);
      
      nstrokes = GUINT32_FROM_BE(buf[0]);
      len = GUINT32_FROM_BE(buf[1]);

      if ((n_read != 2) || (nstrokes > MAX_STROKES))
	{
	  fprintf(stderr, "%s: Corrupt stroke database\n", progname);
	  exit(1);
	}

      if (nstrokes == 0)
	break;

      stroke_dicts[nstrokes] = malloc(len);
      n_read = fread(stroke_dicts[nstrokes], 1, len, file);

      if (n_read != len)
	{
	  fprintf(stderr, "%s: Corrupt stroke database", progname);
	  exit(1);
	}
    }
  
  fclose (file);
}

/* From Ken Lunde's _Understanding Japanese Information Processing_
   O'Reilly, 1993 */

void 
sjis2jis(unsigned char *p1, unsigned char *p2)
{
  unsigned char c1 = *p1;
  unsigned char c2 = *p2;
  int adjust = c2 < 159;
  int rowOffset = c1 < 160 ? 112 : 176;
  int cellOffset = adjust ? (c2 > 127 ? 32 : 31) : 126;
  
  *p1 = ((c1 - rowOffset) << 1) - adjust;
  *p2 -= cellOffset;  
}

GList*
process_strokes (GList *stroke_list)
{
  RawStroke strokes[MAX_STROKES];
  char *buffer = malloc(BUFLEN);
  int buflen = BUFLEN;
  int nstrokes = 0;
  GList* res = NULL;

  //set the stroke array from the stroke list
  while (stroke_list){
    int npoints = 0;
    GList *point_list = stroke_list->data;
    while (point_list){
      int x = ((GdkPoint *)point_list->data)->x;
      int y = ((GdkPoint *)point_list->data)->y;

      strokes[nstrokes].m_x[npoints] = x;
      strokes[nstrokes].m_y[npoints] = y;
      
      point_list = point_list->next;
      npoints++;
    }
    strokes[nstrokes].m_len = npoints;
    stroke_list = stroke_list->next;
    nstrokes++;
  }
  
  //get the score from the strokes
  if (nstrokes != 0 && stroke_dicts[nstrokes]){
      int i;
      ListMem *top_picks;
      StrokeScorer *scorer = StrokeScorerCreate (stroke_dicts[nstrokes], strokes, nstrokes);

      if (scorer){
	  StrokeScorerProcess(scorer, -1);
	  top_picks = StrokeScorerTopPicks(scorer);
	  StrokeScorerDestroy(scorer);
	  
	  for (i=0;i<top_picks->m_argc;i++){
	    unsigned char *c = g_new (unsigned char, 2);  //TODO free me
	    
	    c[0] = top_picks->m_argv[i][0];
	    c[1] = top_picks->m_argv[i][1];
	    sjis2jis(&c[0],&c[1]);

	    res = g_list_append (res, c);
	  }
	  
	  free(top_picks);
      }
  }

  
  return res;  
  
}
