// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// dxffile.cpp
//

#include <racc.h>


// DXF buffer size (16 Mb should be enough)
#define DEBUG_DXF_SIZE 16000000


char *dxf_buffer;
long dxf_buffer_index;



void AddToDXF (unsigned short opcode, const char *szFmt, ...)
{
   // handy macros used to append data into the DXF buffer. Takes a set of strings (ala printf)

   va_list argptr;
   static char string[256];
   static char line_buffer[256];

   // concatenate all the arguments in one string
   va_start (argptr, szFmt);
   vsprintf (string, szFmt, argptr);
   va_end (argptr);

   sprintf (line_buffer, "\t%d\n%s\n", opcode, string); // build the line to append to DXF file
   sprintf (&dxf_buffer[dxf_buffer_index], line_buffer); // append line to DXF file at the end
   dxf_buffer_index += strlen (line_buffer); // save the new write pointer index

   return; // key/value pair has been added to DXF
}


void AddToDXF (unsigned short opcode, const long value)
{
   // overloaded version of the previous. This one takes a long integer

   static char line_buffer[256];

   sprintf (line_buffer, "\t%d\n%d\n", opcode, value); // build the line to append to DXF file
   sprintf (&dxf_buffer[dxf_buffer_index], line_buffer); // append line to DXF file at the end
   dxf_buffer_index += strlen (line_buffer); // save the new write pointer index

   return; // key/value pair has been added to DXF
}


void AddToDXF (unsigned short opcode, const double value)
{
   // overloaded version of the previous. This one takes a double precision real

   static char line_buffer[256];

   sprintf (line_buffer, "\t%d\n%f\n", opcode, value); // build the line to append to DXF file
   sprintf (&dxf_buffer[dxf_buffer_index], line_buffer); // append line to DXF file at the end
   dxf_buffer_index += strlen (line_buffer); // save the new write pointer index

   return; // key/value pair has been added to DXF
}


void InitDebugDXF (void)
{
   // this function allocates memory and clears the debug DXF buffer

   if (dxf_buffer)
      free (dxf_buffer); // reliability check, free DXF buffer if already allocated
   dxf_buffer = NULL;
   dxf_buffer = (char *) malloc (DEBUG_DXF_SIZE); // allocate memory
   if (dxf_buffer == NULL)
      TerminateOnError ("InitDebugDXF(): unable to allocate %d kbytes for DXF buffer!\n", DEBUG_DXF_SIZE / 1024);

   dxf_buffer_index = 0; // reset the DXF buffer pointer

   memset (dxf_buffer, 0, DEBUG_DXF_SIZE); // zero all the crap out
   return; // yes, it's as simple as that
}


void DrawLineInDebugDXF (const Vector v_from, const Vector v_to, unsigned char color, const char *layer_name)
{
   // this function adds a line in the DXF data starting from the location vector v_from to v_to,
   // using the specified color, on the specified layer. If the layer doesn't exist, it will be
   // created by the CAD software. Note that color should be an AutoCAD palette color index.

   // You wonder why I don't use strcat() perhaps ? Well, because strcat() is SLOOOOOOOOOOOOOOW.
   // It needs to run all along the string in order to find the terminator character and then
   // append from there. Better store the buffer write index as I do. Saves me UNCONCEIVABLE time.

   if (dxf_buffer == NULL)
   {
      TerminateOnError ("DrawLineInDebugDXF(): function called with NULL DXF buffer!\n");
      return; // reliability check: cancel if dxf buffer unallocated
   }

   // write the line header
   AddToDXF (0, "LINE"); // declare a new polyline
   AddToDXF (8, layer_name); // sets layer name (will be created if unexistent)

   // write the line starting point coordinates
   AddToDXF (10, v_from.x); // sets starting X
   AddToDXF (20, v_from.y); // sets starting Y
   AddToDXF (30, v_from.z); // sets starting Z

   // write the line end point coordinates
   AddToDXF (11, v_to.x); // sets end X
   AddToDXF (21, v_to.y); // sets end Y
   AddToDXF (31, v_to.z); // sets end Z

   return; // finished, line has been printed into the DXF buffer
}


void DrawWalkfaceInDebugDXF (const walkface_t *walkface, unsigned char color, const char *layer_name)
{
   // this function adds a polyface mesh in the DXF data representing the walkface pointed to by
   // walkface, using the specified color, on the specified layer. It is called by WriteDebugDXF()
   // which calls it often enough as necessary so as to draw each of the walkfaces of the global
   // navmesh. Note that color should be an AutoCAD palette color index.

   // You wonder why I don't use strcat() perhaps ? Well, because strcat() is SLOOOOOOOOOOOOOOW.
   // It needs to run all along the string in order to find the terminator character and then
   // append from there. Better store the buffer write index as I do. Saves me UNCONCEIVABLE time.

   int corner_index;
   static char line_buffer[256];

   if (dxf_buffer == NULL)
   {
      TerminateOnError ("DrawWalkfaceInDebugDXF(): function called with NULL DXF buffer!\n");
      return; // reliability check: cancel if dxf buffer unallocated
   }

   if (walkface == NULL)
   {
      TerminateOnError ("DrawWalkfaceInDebugDXF(): function called with NULL walkface!\n");
      return; // reliability check
   }

   // write the polyline header
   AddToDXF (0, "POLYLINE"); // declare a new polyline
   AddToDXF (66, (long) 1); // tell that we're starting a sequence
   AddToDXF (8, layer_name); // sets layer name (will be created if unexistent)
   AddToDXF (70, (long) 64); // 64 = 0x00100000, sets the "polyface mesh" option bit
   AddToDXF (71, (long) (2 * walkface->corner_count)); // sets the vertexes count (2 * corners)
   AddToDXF (72, (long) (2 * walkface->corner_count)); // sets the faces count (2 * corners)
   AddToDXF (62, (long) color); // sets the polyline color

   // loop through all the corners this walkface has and write the vertexes...
   for (corner_index = 0; corner_index < walkface->corner_count; corner_index++)
   {
      // top side of the face
      AddToDXF (0, "VERTEX"); // declare a new vertice
      AddToDXF (8, layer_name); // sets layer name (same as the polyline's)
      AddToDXF (10, walkface->v_corners[corner_index].x); // sets starting X
      AddToDXF (20, walkface->v_corners[corner_index].y); // sets starting Y
      AddToDXF (30, walkface->v_corners[corner_index].z); // sets starting Z
      AddToDXF (70, (long) 192); // 192 = 0x01100000, "polyface mesh" + "3D polygon mesh"

      // bottom side of the face
      AddToDXF (0, "VERTEX"); // declare a new vertice
      AddToDXF (8, layer_name); // sets layer name (same as the polyline's)
      AddToDXF (10, walkface->v_corners[corner_index].x); // sets starting X
      AddToDXF (20, walkface->v_corners[corner_index].y); // sets starting Y
      AddToDXF (30, walkface->v_corners[corner_index].z - 0.1); // sets starting Z a bit lower
      AddToDXF (70, (long) 192); // 192 = 0x01100000, "polyface mesh" + "3D polygon mesh"
   }

   // loop through all the corners again and write the side faces...
   for (corner_index = 0; corner_index < walkface->corner_count; corner_index++)
   {
      // first half (first triangle) of this side
      AddToDXF (0, "VERTEX"); // declare a new vertice (same chunk name as vertex)
      AddToDXF (8, layer_name); // sets layer name (same as the polyline's)
      AddToDXF (10, (long) 0); // starting X always 0 for face, but line is mandatory anyway
      AddToDXF (20, (long) 0); // starting Y always 0 for face, but line is mandatory anyway
      AddToDXF (30, (long) 0); // starting Z always 0 for face, but line is mandatory anyway
      AddToDXF (70, (long) 128); // 128 = 0x01000000, "edge"
      AddToDXF (71, (long) (1 + 2 * corner_index)); // 1st vertex = 1st corner
      AddToDXF (72, (long) (1 + 2 * corner_index + 1)); // 2nd vertex = vertex below 1st corner
      AddToDXF (73, (long) (1 + (corner_index + 1 < walkface->corner_count ? 2 * corner_index + 2 : 0)));

      // second half (second triangle) of this side
      AddToDXF (0, "VERTEX"); // declare a new face (same chunk name as vertex)
      AddToDXF (8, layer_name); // sets layer name (same as the polyline's)
      AddToDXF (10, (long) 0); // starting X always 0 for face, but line is mandatory anyway
      AddToDXF (20, (long) 0); // starting Y always 0 for face, but line is mandatory anyway
      AddToDXF (30, (long) 0); // starting Z always 0 for face, but line is mandatory anyway
      AddToDXF (70, (long) 128); // 128 = 0x01000000, "edge"
      AddToDXF (71, (long) (1 + 2 * corner_index + 1)); // 1st vertex = 1st corner
      AddToDXF (72, (long) (1 + (corner_index + 1 < walkface->corner_count ? 2 * corner_index + 2 : 0)));
      AddToDXF (73, (long) (1 + (corner_index + 1 < walkface->corner_count ? 2 * corner_index + 3 : 1)));
   }

   // TODO: write top and bottom faces too (but need to divide non-triangular walkfaces into
   // triangles, what a crap - bah, will do that latah *yawn*)

   // close the sequence
   AddToDXF (0, "SEQEND"); // it's not a new entity, it's we're finishing the current one

   return; // finished, walkface has been printed into the DXF buffer
}


void DrawSectorInDebugDXF (int sector_i, int sector_j, unsigned char color, const char *layer_name)
{
   // this function is a superset for DrawWalkfaceInDebugDXF(). It is used to draw topologic
   // sectors in the DXF buffer in much the same way as we draw walkfaces (i.e, as polyface
   // meshes). We are passing a dummy walkface pointer (statically defined) for this, which we
   // fill in before with the sector's info, as if the sector was itself a walkface.

   static walkface_t dummy_walkface;
   static Vector dummy_walkface_corners[4];
   float sector_left, sector_right, sector_bottom, sector_top;

   if (dxf_buffer == NULL)
   {
      TerminateOnError ("DrawSectorInDebugDXF(): function called with NULL DXF buffer!\n");
      return; // reliability check: cancel if dxf buffer unallocated
   }

   // first compute the sector limits...
   sector_left = map.v_worldmins.x + sector_i * ((map.v_worldmaxs.x - map.v_worldmins.x) / map.parallels_count);
   sector_right = map.v_worldmins.x + (sector_i + 1) * ((map.v_worldmaxs.x - map.v_worldmins.x) / map.parallels_count);
   sector_bottom = map.v_worldmins.y + sector_j * ((map.v_worldmaxs.y - map.v_worldmins.y) / map.meridians_count);
   sector_top = map.v_worldmins.y + (sector_j + 1) * ((map.v_worldmaxs.y - map.v_worldmins.y) / map.meridians_count);

   // and build a dummy walkface of the size of the sector (only the corners part, anyway)
   dummy_walkface.v_corners = dummy_walkface_corners;
   dummy_walkface.v_corners[0] = Vector (sector_left, sector_bottom, 0);
   dummy_walkface.v_corners[1] = Vector (sector_right, sector_bottom, 0);
   dummy_walkface.v_corners[2] = Vector (sector_right, sector_top, 0);
   dummy_walkface.v_corners[3] = Vector (sector_left, sector_top, 0);
   dummy_walkface.corner_count = 4;

   DrawWalkfaceInDebugDXF (&dummy_walkface, 7, layer_name); // then draw it

   return; // finished, sector has been printed into the DXF buffer
}


void WriteDebugDXF (const char *filename)
{
   // this function writes the global navmesh (walkfaces array) in a .DXF file to disk. We use
   // the ASCII format for better readability and backwards compatibility. The main advantage of
   // exporting to 3D vector formats such as .DXF is that everything that is exported is exported
   // at the 1:1 scale, i.e. the coordinates you will read in AutoCAD will be the coordinates in
   // the virtual world. The DXF is stored in the file specified by 'filename' (which can be a
   // relative path from the RACC base directory). The walkfaces and sectors colors can be
   // specified separately, but they need to belong to AutoCAD's standard palette.

   // WARNING: in a DXF file, most of the indices (such ase vertex indices) are 1 based.

   FILE *fp;

   if (dxf_buffer == NULL)
   {
      TerminateOnError ("WriteDebugDXF(): function called with NULL DXF buffer!\n");
      return; // reliability check: cancel if dxf buffer unallocated
   }

   // open (or create) the DXF file for writing in ASCII mode...
   fp = fopen (filename, "w");
   if (fp == NULL)
   {
      ServerConsole_printf ("RACC: WriteNavmeshInDebugDXF(): unable to open DXF file!\n");
      if (dxf_buffer)
         free (dxf_buffer); // cannot open file, free DXF buffer
      dxf_buffer = NULL;
      return; // cancel if error creating file
   }

   // write the DXF header...
   fprintf (fp, "\t999\n%s\n", racc_welcometext); // start with the welcome text
   fprintf (fp, "\t999\n" "Drawing generated on map '%s' for game '%s'\n", server.map_name, GameConfig.mod_name);
   fprintf (fp, "\t0\nSECTION\n"); // we're opening a section
   fprintf (fp, "\t2\nENTITIES\n"); // it's the "ENTITIES" section (the one with the drawing elements data)

   fputs (dxf_buffer, fp); // write the elements currently defined in the DXF buffer

   fprintf (fp, "\t0\nENDSEC\n"); // close the current section
   fprintf (fp, "\t0\nEOF\n"); // terminate the whole DXF file

   fclose (fp); // finished, close the DXF file

   if (dxf_buffer)
      free (dxf_buffer); // and free the DXF buffer
   dxf_buffer = NULL;

   return; // and return
}
