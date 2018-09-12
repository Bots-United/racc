// RACC - AI development project for first-person shooter games derived from Valve's Half-Life
// (http://www.racc-ai.com/)
//
// The game to engine interfacing code is based on the work done by Jeffrey 'botman' Broome
// (http://planethalflife.com/botman/)
//
// This project is partially based on the work done by Eric Bieschke in his BSDbot
// (http://gamershomepage.com/csbot/)
//
// This project is partially based on the work done by Brendan "Spyro" McCarthy in his ODD Bot
// (http://oddbot.hlfusion.com/)
//
// This project is partially based on the work done by Alistair 'eLiTe' Stewart in his TEAMbot
// (http://www.planethalflife.com/teambot/)
//
// The BMP writing functions in this file come primarily from botman's BSP slicer utility 
// (http://planethalflife.com/botman/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// bmpfile.cpp
//

#include <racc.h>


// width and height of the debug bitmap image
#define DEBUG_BMP_WIDTH 2000
#define DEBUG_BMP_HEIGHT 2000


char *bmp_buffer;



void InitDebugBitmap (void)
{
   // this function allocates memory and clears the debug bitmap buffer

   if (bmp_buffer)
      free (bmp_buffer); // reliability check, free BMP buffer if already allocated
   bmp_buffer = NULL;
   bmp_buffer = (char *) malloc (DEBUG_BMP_WIDTH * DEBUG_BMP_HEIGHT); // allocate memory
   if (bmp_buffer == NULL)
      TerminateOnError ("InitDebugBitmap(): unable to allocate %d kbytes for BMP buffer!\n", DEBUG_BMP_WIDTH * DEBUG_BMP_HEIGHT / 1024);

   memset (bmp_buffer, 0, DEBUG_BMP_WIDTH * DEBUG_BMP_HEIGHT); // zero all the crap out
   return; // yes, it's as simple as that
}


void DrawLineInDebugBitmap (const Vector v_from, const Vector v_to, unsigned char color)
{
   // blind copy of botman's Bresenham(). This function prints a vector line into a bitmap dot
   // matrix. The dot matrix (bmp_buffer) is a global array. The size of the bitmap is always
   // assumed to be DEBUG_BMP_WIDTH * DEBUG_BMP_HEIGHT pixels (currently 2000 * 2000 to fit with
   // the size of the universe, with an adaptative unit scale, up to 1 pixel = 10 vector units).

   int x0, y0, x1, y1;
   int dx, stepx, dy, stepy;
   int offset, fraction;
   float scalex, scaley, scale;

   if (bmp_buffer == NULL)
   {
      TerminateOnError ("DrawLineInDebugBitmap(): function called with NULL BMP buffer!\n");
      return; // reliability check: cancel if bmp buffer unallocated
   }

   // first compute the X and Y divider scale, and take the greatest of both
   scalex = (map.v_worldmaxs.x - map.v_worldmins.x) / DEBUG_BMP_WIDTH;
   scaley = (map.v_worldmaxs.y - map.v_worldmins.y) / DEBUG_BMP_HEIGHT;
   if (scalex > scaley)
      scale = scalex + scalex / 100; // add a little offset (margin) for safety
   else
      scale = scaley + scaley / 100; // add a little offset (margin) for safety

   // translate the world coordinates in image pixel coordinates
   x0 = (int) ((v_from.x - map.v_worldmins.x) / scale);
   y0 = (int) ((v_from.y - map.v_worldmins.y) / scale);
   x1 = (int) ((v_to.x - map.v_worldmins.x) / scale);
   y1 = (int) ((v_to.y - map.v_worldmins.y) / scale);

   dx = (x1 - x0) * 2;
   dy = (y1 - y0) * 2;
   if (dx < 0) { dx = -dx;  stepx = -1; } else stepx = 1;
   if (dy < 0) { dy = -dy;  stepy = -1; } else stepy = 1;

   offset = y0 * DEBUG_BMP_WIDTH + x0;
   if ((offset < 0) || (offset >= DEBUG_BMP_WIDTH * DEBUG_BMP_HEIGHT))
      TerminateOnError ("DrawLineInDebugBitmap(): bad BMP buffer index %d (range 0 - %d)\n", offset, DEBUG_BMP_WIDTH * DEBUG_BMP_HEIGHT);

   bmp_buffer[offset] = color; // draw the first point of the line

   // is the line rather horizontal than vertical ? We need to know this to determine the step
   // advance in the Bresenham grid, either we draw y = f(x), or x = f(y).
   if (dx > dy)
   {
      // the line is rather horizontal, we can draw it safely for incremental values of x

      fraction = 2 * dy - dx; // fraction of height in x0 pixel's 'square' where y0 should be

      // while we've not reached the end of the segment...
      while (x0 != x1)
      {
         // if y0 should rather be drawn on a different height than its previous height...
         if (fraction >= 0)
         {
            y0 += stepy; // draw it one pixel aside, then (depending on line orientation)
            fraction -= 2 * dx; // and reset its fraction (Bresenham, not sure I get the math)
         }
         x0 += stepx; // in either case, draw x0 one pixel aside its previous position
         fraction += 2 * dy; // and update y0's fraction (not sure I get the math - but whatever)

         // compute the offset in the BMP buffer corresponding to this point
         offset = y0 * DEBUG_BMP_WIDTH + x0;
         if ((offset < 0) || (offset >= DEBUG_BMP_WIDTH * DEBUG_BMP_HEIGHT))
            TerminateOnError ("DrawLineInDebugBitmap(): bad BMP buffer index %d (range 0 - %d)\n", offset, DEBUG_BMP_WIDTH * DEBUG_BMP_HEIGHT);

         bmp_buffer[offset] = color; // set this point to have the specified color
      }
   }
   else
   {
      // else the line is rather vertical, we NEED to draw it for incremental values of y (if we
      // did it for incremental values of x instead, we would drop half the pixels).

      fraction = 2 * dx - dy; // fraction of width in y0 pixel's 'square' where x0 should be

      // while we've not reached the end of the segment...
      while (y0 != y1)
      {
         // if x0 should rather be drawn on a different width than its previous width...
         if (fraction >= 0)
         {
            x0 += stepx; // draw it one pixel aside, then (depending on line orientation)
            fraction -= 2 * dy; // and reset its fraction (Bresenham, not sure I get the math)
         }
         y0 += stepy; // in either case, draw y0 one pixel aside its previous position
         fraction += 2 * dx; // and update x0's fraction (not sure I get the math - but whatever)

         // compute the offset in the BMP buffer corresponding to this point
         offset = y0 * DEBUG_BMP_WIDTH + x0;
         if ((offset < 0) || (offset >= DEBUG_BMP_WIDTH * DEBUG_BMP_HEIGHT))
            TerminateOnError ("DrawLineInDebugBitmap(): bad BMP buffer index %d (range 0 - %d)\n", offset, DEBUG_BMP_WIDTH * DEBUG_BMP_HEIGHT);

         bmp_buffer[offset] = color; // set this point to have the specified color
      }
   }

   return; // finished, segment has been printed into the BMP dot matrix
}


void DrawWalkfaceInDebugBitmap (const walkface_t *walkface, unsigned char color)
{
   // this function is a superset for DrawLineInDebugBitmap() which calls it often enough as
   // necessary so as to draw each of the edges of the walkface pointed to by walkface, using the
   // palette color specified by color.

   Vector v_from, v_to;
   int index;

   if (bmp_buffer == NULL)
   {
      TerminateOnError ("DrawWalkfaceInDebugBitmap(): function called with NULL BMP buffer!\n");
      return; // reliability check: cancel if bmp buffer unallocated
   }

   if (walkface == NULL)
   {
      TerminateOnError ("DrawWalkfaceInDebugBitmap(): function called with NULL walkface!\n");
      return; // reliability check
   }

   // for each walkface, cycle through all corners...
   for (index = 0; index < walkface->corner_count; index++)
   {
      // build all the walkface edges out of its corners
      v_from = walkface->v_corners[index]; // get the start corner
      if (index < walkface->corner_count - 1)
         v_to = walkface->v_corners[index + 1]; // take the next one for the end corner
      else
         v_to = walkface->v_corners[0]; // on last corner, loop back to corner #0

      DrawLineInDebugBitmap (v_from, v_to, color); // and draw them in the bitmap
   }

   return; // finished, walkface has been printed into the BMP dot matrix
}


void DrawSectorInDebugBitmap (int sector_i, int sector_j, unsigned char color)
{
   // this function is a superset for DrawLineInDebugBitmap() which calls it often enough as
   // necessary so as to draw each of the edges of the sector whose coordinates are [sector_i]
   // [sector_j], using the palette color specified by color. A range test is made to ensure
   // sector_i and sector_j do not exceed the parallels and meridians limits.

   float sector_left, sector_right, sector_top, sector_bottom;

   if (bmp_buffer == NULL)
   {
      TerminateOnError ("DrawSectorInDebugBitmap(): function called with NULL BMP buffer!\n");
      return; // reliability check: cancel if bmp buffer unallocated
   }

   if ((sector_i < 0) || (sector_i >= map.parallels_count))
   {
      TerminateOnError ("DrawSectorInDebugBitmap(): sector [%d, %d] out of range! (max [%d, %d])\n", sector_i, sector_j, map.parallels_count, map.meridians_count);
      return; // reliability check: bomb out if sector out of range
   }

   if ((sector_j < 0) || (sector_j >= map.meridians_count))
   {
      TerminateOnError ("DrawSectorInDebugBitmap(): sector [%d, %d] out of range! (max [%d, %d])\n", sector_i, sector_j, map.parallels_count, map.meridians_count);
      return; // reliability check: bomb out if sector out of range
   }

   // first compute the sector limits...
   sector_left = map.v_worldmins.x + sector_i * ((map.v_worldmaxs.x - map.v_worldmins.x) / map.parallels_count);
   sector_right = map.v_worldmins.x + (sector_i + 1) * ((map.v_worldmaxs.x - map.v_worldmins.x) / map.parallels_count);
   sector_bottom = map.v_worldmins.y + sector_j * ((map.v_worldmaxs.y - map.v_worldmins.y) / map.meridians_count);
   sector_top = map.v_worldmins.y + (sector_j + 1) * ((map.v_worldmaxs.y - map.v_worldmins.y) / map.meridians_count);

   // and draw the sector square
   DrawLineInDebugBitmap (Vector (sector_left, sector_bottom, 0), Vector (sector_left, sector_top, 0), color);
   DrawLineInDebugBitmap (Vector (sector_right, sector_bottom, 0), Vector (sector_right, sector_top, 0), color);
   DrawLineInDebugBitmap (Vector (sector_left, sector_bottom, 0), Vector (sector_right, sector_bottom, 0), color);
   DrawLineInDebugBitmap (Vector (sector_left, sector_top, 0), Vector (sector_right, sector_top, 0), color);

   return; // finished, sector has been printed into the BMP dot matrix
}


void WriteDebugBitmap (const char *filename)
{
   // this function writes the debug bitmap image buffer in a .BMP file to disk. The format is
   // 256 color and 2000 * 2000 pixels. The center of the world being roughly the center of the
   // bitmap. The bitmap is stored in the file specified by 'filename' (which can be a relative
   // path from the Half-Life base directory).

   FILE *fp;
   int data_start, file_size;
   unsigned long dummy;

   if (bmp_buffer == NULL)
   {
      TerminateOnError ("WriteDebugBitmap(): function called with NULL BMP buffer!\n");
      return; // reliability check: cancel if bmp buffer unallocated
   }

   // open (or create) the .bmp file for writing in binary mode...
   fp = fopen (filename, "wb");
   if (fp == NULL)
   {
      ServerConsole_printf ("RACC: WriteDebugBitmap(): unable to open BMP file!\n");
      if (bmp_buffer)
         free (bmp_buffer); // cannot open file, free DXF buffer
      bmp_buffer = NULL;
      return; // cancel if error creating file
   }

   // write the BMP header
   fwrite ("BM", 2, 1, fp); // write the BMP header tag
   fseek (fp, sizeof (unsigned long), SEEK_CUR); // skip the file size field (will write it last)
   fwrite ("\0\0", sizeof (short), 1, fp); // dump zeros in the first reserved field (unused)
   fwrite ("\0\0", sizeof (short), 1, fp); // dump zeros in the second reserved field (unused)
   fseek (fp, sizeof (unsigned long), SEEK_CUR); // skip the data start field (will write it last)

   // write the info header
   dummy = 40;
   fwrite (&dummy, sizeof (unsigned long), 1, fp); // write the info header size (does 40 bytes)
   dummy = DEBUG_BMP_WIDTH;
   fwrite (&dummy, sizeof (long), 1, fp); // write the image width (2000 px)
   dummy = DEBUG_BMP_HEIGHT;
   fwrite (&dummy, sizeof (long), 1, fp); // write the image height (2000 px)
   dummy = 1;
   fwrite (&dummy, sizeof (short), 1, fp); // write the # of planes (1)
   dummy = 8;
   fwrite (&dummy, sizeof (short), 1, fp); // write the bit count (8)
   dummy = 0;
   fwrite (&dummy, sizeof (unsigned long), 1, fp); // write the compression id (no compression)
   dummy = DEBUG_BMP_WIDTH * DEBUG_BMP_HEIGHT;
   fwrite (&dummy, sizeof (unsigned long), 1, fp); // write the image size (2000 * 2000)
   dummy = 0;
   fwrite (&dummy, sizeof (long), 1, fp); // write the X pixels per meter (not specified)
   fwrite (&dummy, sizeof (long), 1, fp); // write the Y pixels per meter (not specified)
   dummy = 256;
   fwrite (&dummy, sizeof (unsigned long), 1, fp); // write the # of colors used (all)
   fwrite (&dummy, sizeof (unsigned long), 1, fp); // write the # of important colors (wtf ?)

   // write the color palette (R, G, B, reserved byte)
   fputc (0x00, fp); fputc (0x00, fp); fputc (0x00, fp); fputc (0x00, fp); // 0=BLACK
   fputc (0xFF, fp); fputc (0xFF, fp); fputc (0xFF, fp); fputc (0x00, fp); // 1=WHITE
   fputc (0x80, fp); fputc (0x80, fp); fputc (0x80, fp); fputc (0x00, fp); // 2=GREY
   fputc (0xC0, fp); fputc (0xC0, fp); fputc (0xC0, fp); fputc (0x00, fp); // 3=SILVER
   fputc (0x80, fp); fputc (0x00, fp); fputc (0x00, fp); fputc (0x00, fp); // 4=DARK RED
   fputc (0xFF, fp); fputc (0x00, fp); fputc (0x00, fp); fputc (0x00, fp); // 5=RED
   fputc (0x80, fp); fputc (0x80, fp); fputc (0x00, fp); fputc (0x00, fp); // 6=DARK YELLOW
   fputc (0xFF, fp); fputc (0xFF, fp); fputc (0x00, fp); fputc (0x00, fp); // 7=YELLOW
   fputc (0x00, fp); fputc (0x80, fp); fputc (0x00, fp); fputc (0x00, fp); // 8=DARK GREEN
   fputc (0x00, fp); fputc (0xFF, fp); fputc (0x00, fp); fputc (0x00, fp); // 9=GREEN
   fputc (0x00, fp); fputc (0x00, fp); fputc (0x80, fp); fputc (0x00, fp); // 10=DARK BLUE
   fputc (0x00, fp); fputc (0x00, fp); fputc (0x80, fp); fputc (0x00, fp); // 11=BLUE
   fputc (0x80, fp); fputc (0x00, fp); fputc (0x80, fp); fputc (0x00, fp); // 12=DARK PURPLE
   fputc (0x80, fp); fputc (0x00, fp); fputc (0x80, fp); fputc (0x00, fp); // 13=PURPLE

   for (dummy = 14; dummy < 256; dummy++)
   {
      // fill out the rest of the palette with zeros
      fputc (0x00, fp); fputc (0x00, fp); fputc (0x00, fp); fputc (0x00, fp);
   }

   // write the actual image data
   data_start = ftell (fp); // get the data start position (that's where we are now)
   fwrite (bmp_buffer, DEBUG_BMP_WIDTH * DEBUG_BMP_HEIGHT, 1, fp); // write the image
   file_size = ftell (fp); // get the file size now that the image is dumped

   // now that we've dumped our data, we know the file size and the data start position

   fseek (fp, 0, SEEK_SET); // rewind
   fseek (fp, 2, SEEK_CUR); // skip the BMP header tag "BM"
   fwrite (&file_size, sizeof (unsigned long), 1, fp); // write the file size at its location
   fseek (fp, sizeof (short), SEEK_CUR); // skip the first reserved field
   fseek (fp, sizeof (short), SEEK_CUR); // skip the second reserved field
   fwrite (&data_start, sizeof (unsigned long), 1, fp); // write the data start at its location

   fclose (fp); // finished, close the BMP file

   if (bmp_buffer)
      free (bmp_buffer); // and free the BMP buffer
   bmp_buffer = NULL;

   return; // and return
}
