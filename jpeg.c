/**************************************************
 * RCSId: $Id: jpeg.c,v 1.3 2018/02/02 23:16:16 ralblas Exp $
 *
 * Satellite tracker 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: jpeg.c,v $
 * Revision 1.3  2018/02/02 23:16:16  ralblas
 * _
 *
 *
 **************************************************/
/*******************************************************************
 * Copyright (C) 2000 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
 * 02111-1307, USA.
 ********************************************************************/
#include <stdio.h>
#include <setjmp.h>
#include <malloc.h>
#include "jinclude.h"
#include "jpeglib.h"
#include "cdjpeg.h"
#include <gtk/gtk.h>

#if __ADD_JPEG__ == 1
struct my_error_mgr
{
  struct jpeg_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
};
typedef struct my_error_mgr *my_error_ptr;
METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}
void open_rd_jpeg(FILE *fpi,struct jpeg_decompress_struct *cinfo,
                  JSAMPARRAY *buffer,  int *row_stride)
{
  static struct my_error_mgr jerr;
  cinfo->err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if (setjmp(jerr.setjmp_buffer))
  {
    /* If we get here, the JPEG code has signaled an error.
     */
    jpeg_destroy_decompress(cinfo);
    return;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src(cinfo, fpi);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(cinfo, TRUE);

  /* Step 5: Start decompressor */

  (void) jpeg_start_decompress(cinfo);

  *row_stride = cinfo->output_width * cinfo->output_components;
  /* Make a one-row-high sample array that will go away when done with image */
  *buffer = (*cinfo->mem->alloc_sarray)
		((j_common_ptr) cinfo, JPOOL_IMAGE, *row_stride, 1);

}

void close_rd_jpeg(struct jpeg_decompress_struct *cinfo,JSAMPARRAY buffer)
{
  (void) jpeg_finish_decompress(cinfo);
  jpeg_destroy_decompress(cinfo);
}

int jpg2str_1b(FILE *fpi,guchar **str,int *W,int *H)
{
  struct jpeg_decompress_struct cinfo;
  JSAMPARRAY buffer;		/* Output row buffer */
  int width;

  if (!fpi) return 'f';
  rewind(fpi);
  open_rd_jpeg(fpi,&cinfo,&buffer,&width);

  *W=width/3;
  *H=cinfo.output_height;

  *str=malloc(cinfo.output_height*width);
  if (!str) return 'm';
  while (cinfo.output_scanline < cinfo.output_height)
  {
    int x;
    int y=cinfo.output_scanline;
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);

    for (x=0; x<width; x++)
    {
      (*str)[x+y*width]=buffer[0][x];
    }
  }
  close_rd_jpeg(&cinfo,buffer);
  return 0;
}

void read_jpg_line(guchar *istr,int picwidth, int picy,guint16 *line_r,guint16 *line_g,guint16 *line_b)
{
  int x,ipos;

  for (x=0; x<picwidth; x++)
  {
    ipos=(x+picy*picwidth)*3;
    line_r[x]=istr[ipos+0];
    line_g[x]=istr[ipos+1];
    line_b[x]=istr[ipos+2];
  }
}
#endif
