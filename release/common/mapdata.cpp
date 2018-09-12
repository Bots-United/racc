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
// The author wants to credit Paul "Cheesemonster" Murphy for his handy math skills that helped
// me a lot during the writing of the navmesh building code
//
// Rational Autonomous Cybernetic Commandos AI
//
// mapdata.cpp
//

#include "racc.h"


void LookDownOnTheWorld (void)
{
   // this function loads and interprets the map BSP file at server boot start. It opens the map
   // file named filename, reads its contents, parses through the different BSP lumps and fills
   // the map BSP data structure "map", so that we can access to much more geometric info about
   // the virtual world than the engine would lets us know normally. The BSP loading code comes
   // with heavy simplification and major rewriting from Zoner's Half-Life tools source code.
   // Once this process has been done once, a .map file is created which holds the relevant map
   // data the bots make use of, and this file is loaded instead of redoing the BSP analysis
   // process from the start the next time the server is booted. In extenso, this function
   // sort the BSP data and the walkable faces in a handy hashtable). It checks whether a world
   // map file already exists for the map currently booted, and if so, opens it and use the data
   // therein. Else if it is a new map that has never been used with RACC which is booted for
   // the first time, its purpose is to retrieve, compute and sort various map data used by the
   // bots such as the walkable faces, the location of the face delimiters, and finally a world
   // map is "drawn", separating each walkable face to the parallel and meridian it refers to.
   // Such a world map is very useful for the bots not to have to cycle through all the huge map
   // data when they want to bind just one face or just one face delimiter. It speeds up the
   // search by square the resolution of the world map, ie by parallels * meridians times.

   char bsp_file_path[256], *mfile;
   int bsp_file_size, map_file_size = 0;
   dface_t *face;
   walkface_t *walkface;
   int face_index, edge_index, edge, edge_boundary, walkface_index, i, j;
   Vector v_bound1, v_bound2, v_middle;

   // do some cleanup first
   FreeMapData ();
   memset (&bsp_file, 0, sizeof (bsp_file));
   memset (&map, 0, sizeof (map));

   // if a world map already exists...
   if (LoadWorldMap ())
      return; // return if the loading process completed successfully 

   // map not found, out of date or failed loading, we have to build a new one
   ServerConsole_printf ("RACC: Looking down on the world"); // tell people what we are doing

   // load the bsp file and get its actual size (can't fail to do this, the map already booted)
   sprintf (bsp_file_path, "maps/%s.bsp", server.map_name); // build BSP file path
   mfile = (char *) LOAD_FILE_FOR_ME (bsp_file_path, &bsp_file_size); // load bsp file

   // read the MODELS, VERTEXES, PLANES, FACES, SURFEDGES and EDGES lumps of the BSP file
   memcpy (bsp_file.dmodels, mfile + ((dheader_t *) mfile)->lumps[LUMP_MODELS].fileofs, ((dheader_t *) mfile)->lumps[LUMP_MODELS].filelen);
   memcpy (bsp_file.dvertexes, mfile + ((dheader_t *) mfile)->lumps[LUMP_VERTEXES].fileofs, ((dheader_t *) mfile)->lumps[LUMP_VERTEXES].filelen);
   memcpy (bsp_file.dplanes, mfile + ((dheader_t *) mfile)->lumps[LUMP_PLANES].fileofs, ((dheader_t *) mfile)->lumps[LUMP_PLANES].filelen);
   memcpy (bsp_file.dfaces, mfile + ((dheader_t *) mfile)->lumps[LUMP_FACES].fileofs, ((dheader_t *) mfile)->lumps[LUMP_FACES].filelen);
   memcpy (bsp_file.dsurfedges, mfile + ((dheader_t *) mfile)->lumps[LUMP_SURFEDGES].fileofs, ((dheader_t *) mfile)->lumps[LUMP_SURFEDGES].filelen);
   memcpy (bsp_file.dedges, mfile + ((dheader_t *) mfile)->lumps[LUMP_EDGES].fileofs, ((dheader_t *) mfile)->lumps[LUMP_EDGES].filelen);

   FREE_FILE (mfile); // everything is loaded, free the BSP file

   // get a quick access to the world's bounding box
   map.v_worldmins = bsp_file.dmodels[0].mins;
   map.v_worldmaxs = bsp_file.dmodels[0].maxs;

   // completely reset all the topology array
   memset (&map.topology, 0, sizeof (map.topology));

   // compute the number of sectors this map should have (300 units-sized sectors by default)
   map.parallels_count = (map.v_worldmaxs.x - map.v_worldmins.x) / 300;
   map.meridians_count = (map.v_worldmaxs.y - map.v_worldmins.y) / 300;

   // don't allow the parallels and meridians count to be higher than the maximal value allowed
   if (map.parallels_count > MAX_MAP_PARALLELS)
      map.parallels_count = MAX_MAP_PARALLELS; // bound the number of parallels up to MAX_MAP_PARALLELS
   if (map.meridians_count > MAX_MAP_PARALLELS)
      map.meridians_count = MAX_MAP_MERIDIANS; // bound the number of meridians up to MAX_MAP_MERIDIANS

   map.walkfaces_count = 0; // first reset the number of walkable faces we know this map has

   // loop through all the faces of the BSP file and count the number of walkable faces...
   for (face_index = 0; face_index < bsp_file.dmodels[0].numfaces; face_index++)
   {
      face = &bsp_file.dfaces[bsp_file.dmodels[0].firstface + face_index]; // quick access to the face

      // if this face is NOT walkable (i.e, normal NOT pointing straight up OR it's a ceiling)
      if ((bsp_file.dplanes[face->planenum].normal.z < 0.707106)
          || ((bsp_file.dplanes[face->planenum].normal.z ==  1) && (face->side == 1)))
         continue; // discard this face

      map.walkfaces_count++; // we know this map to have one walkable face more
   }

   // now allocate enough memory for the faces array to hold the right number of walkable faces
   map.walkfaces = (walkface_t *) malloc (map.walkfaces_count * sizeof (walkface_t));
   if (map.walkfaces == NULL)
      TerminateOnError ("RACC: LookDownOnTheWorld(): malloc() failure on %d kbytes for walkfaces array!\n", map.walkfaces_count * sizeof (walkface_t) / 1024);

   // also allocate the minimal space for each bucket of the map topology hashtable
   for (i = 0; i < map.parallels_count; i++)
      for (j = 0; j < map.meridians_count; j++)
      {
         map.topology[i][j].faces = (walkface_t **) malloc (sizeof (walkface_t *));
         if (map.topology[i][j].faces == NULL)
            TerminateOnError ("LookDownOnTheWorld(): malloc() failure on %d bytes for first-time topological sector [%d, %d] allocation\n", sizeof (walkface_t *), i, j);
         map.topology[i][j].faces[0] = &map.walkfaces[0]; // failsafe pointer (FIXME: ugly)
      }

   // then, translate each walkable face of the BSP file into an engine-independent structure,
   // describing each face by its corners and its face delimiters (middle of face edges)

   walkface_index = 0; // first reset the walkable faces index

   // loop through all the faces of the BSP file...
   for (face_index = 0; face_index < bsp_file.dmodels[0].numfaces; face_index++)
   {
      face = &bsp_file.dfaces[bsp_file.dmodels[0].firstface + face_index]; // quick access to face

      // if this face is NOT walkable (i.e, normal NOT pointing straight up OR it's a ceiling)
      if ((bsp_file.dplanes[face->planenum].normal.z < 0.707106)
          || ((bsp_file.dplanes[face->planenum].normal.z ==  1) && (face->side == 1)))
         continue; // discard this face

      // allocate enough memory to hold all this face's corner information
      map.walkfaces[walkface_index].corner_count = (int) face->numedges; // number of edges
      map.walkfaces[walkface_index].v_corners = (Vector *) malloc (face->numedges * sizeof (Vector));
      if (map.walkfaces[walkface_index].v_corners == NULL)
         TerminateOnError ("LookDownOnTheWorld(): malloc() failure on %d bytes for corners array on walkface %d\n", face->numedges * sizeof (Vector), walkface_index);
      map.walkfaces[walkface_index].v_delimiters = (Vector *) malloc (face->numedges * sizeof (Vector));
      if (map.walkfaces[walkface_index].v_delimiters == NULL)
         TerminateOnError ("LookDownOnTheWorld(): malloc() failure on %d bytes for delimiters array on walkface %d\n", face->numedges * sizeof (Vector), walkface_index);

      walkface = &map.walkfaces[walkface_index]; // quick access to walkface

      // face is walkable, loop though the edges and get the vertexes...
      for (edge_index = 0; edge_index < face->numedges; edge_index++)
      {
         // get the coordinates of the vertex of this edge...
         edge = bsp_file.dsurfedges[face->firstedge + edge_index];

         // if its index in the BSP tree is negative...
         if (edge < 0)
         {
            edge = -edge; // revert it
            edge_boundary = 1; // consider the other side of the segment
         }
         else
            edge_boundary = 0; // else consider the first side of the segment

         // locate the first vertice of this edge
         v_bound1 = bsp_file.dvertexes[bsp_file.dedges[edge].v[edge_boundary]].point;

         // get the coordinates of the vertex of the next edge...
         edge = bsp_file.dsurfedges[face->firstedge + ((edge_index + 1) % face->numedges)];

         // if its index in the BSP tree is negative...
         if (edge < 0)
         {
            edge = -edge; // revert it
            edge_boundary = 1; // consider the other side of the segment
         }
         else
            edge_boundary = 0; // else consider the first side of the segment

         // locate the second vertice of this edge
         v_bound2 = bsp_file.dvertexes[bsp_file.dedges[edge].v[edge_boundary]].point;

         // compute the middle of this edge (i.e, the face delimiter)
         v_middle = (v_bound1 + v_bound2) * 0.5;

         walkface->v_corners[edge_index] = v_bound1; // store corner
         walkface->v_delimiters[edge_index] = v_middle; // store delimiter
      }

      // for each latitude/longitude sector of the map topology array...
      for (i = 0; i < map.parallels_count; i++)
         for (j = 0; j < map.meridians_count; j++)
         {
            // does this face belong to this topologic sector ?
            if (WalkfaceBelongsToSector (walkface, i, j))
            {
               // reallocate enough space for this sector to hold one walkface more
               map.topology[i][j].faces = (walkface_t **) realloc (map.topology[i][j].faces, (map.topology[i][j].faces_count + 1) * sizeof (walkface_t *));
               if (map.topology[i][j].faces == NULL)
                  TerminateOnError ("LookDownOnTheWorld(): realloc() failure on %d bytes for topological sector [%d, %d] allocation\n", (map.topology[i][j].faces_count + 1) * sizeof (walkface_t *), i, j);

               // now store a pointer to this walkable face in this topological sector
               map.topology[i][j].faces[map.topology[i][j].faces_count] = walkface;
               map.topology[i][j].faces_count++; // this topological sector holds now one face more
            }
         }

      // if we've computed 64 faces more in the BSP faces pool...
      if (!(face_index & 0x3F))
         ServerConsole_printf ("."); // print a trailing dot as a progress bar

      walkface_index++; // we know now one walkable face more
   }

   // once we've got all the sorting done, it's time to save our worldmap to disk
   map_file_size = SaveWorldMap (bsp_file_size);

   // and terminate the progress bar
   ServerConsole_printf (" done\n   %d parallels, %d meridians, %.2f kilobytes world data\n",
                         map.parallels_count, map.meridians_count, (float) map_file_size / 1024);

   return;
}


bool WalkfaceBelongsToSector (const walkface_t *pFace, int sector_i, int sector_j)
{
   // this function returns TRUE if the walkface pointed to by pFace belongs to the topological
   // sector whose position in the global array is [sector_i][sector_j], FALSE otherwise. The
   // local variables are defined static so as to speedup recursive calls of this function, which
   // is extensively used in LookDownOnTheWorld().

   // Math code courtesy of Paul "Cheesemonster" Murphy (HUGE thanks mate !).

   register int corner_index;
   static float sector_left, sector_right, sector_top, sector_bottom, y, a, x, b, angle;
   static Vector v_bound1, v_bound2, segment_left, segment_right;

   // first compute the left, right, top and bottom coordinates indices of the sector
   sector_left = map.v_worldmins.x + sector_i * ((map.v_worldmaxs.x - map.v_worldmins.x) / map.parallels_count);
   sector_right = map.v_worldmins.x + (sector_i + 1) * ((map.v_worldmaxs.x - map.v_worldmins.x) / map.parallels_count);
   sector_bottom = map.v_worldmins.y + sector_j * ((map.v_worldmaxs.y - map.v_worldmins.y) / map.meridians_count);
   sector_top = map.v_worldmins.y + (sector_j + 1) * ((map.v_worldmaxs.y - map.v_worldmins.y) / map.meridians_count);

   angle = 0; // reset angle which will hold the sum of angles to face corners

   // loop though the corners of this face...
   for (corner_index = 0; corner_index < pFace->corner_count; corner_index++)
   {
      // locate the first vertice of this corner
      v_bound1 = pFace->v_corners[corner_index];

      // locate the second vertice of this corner
      if (corner_index < pFace->corner_count - 1)
         v_bound2 = pFace->v_corners[corner_index + 1]; // next corner in the array
      else
         v_bound2 = pFace->v_corners[0]; // loop back to corner zero at last corner

      // now we have one of the edges of the face, let's see if this segment is included within
      // the sector, completely crosses it, or crosses at least one of the sector's boundaries ;
      // in this case, our face will belong to this sector indeed. But beware! face could STILL
      // belong to sector if it was much larger than the sector itself, and the sector would be
      // COMPLETELY comprised within the face (in this case, the sector would have one face and
      // only one, this face). To check for that extra situation, we use a method that Jeffrey
      // 'botman' Broome inspired us, i.e, we take one of the corner of the sector, and see if the
      // sum of the angles that are (edge N bound 1 -> corner of sector -> edge N bound 2) for
      // each edge N of the face, is 360 ; in this case, the corner of the sector will be just
      // on top of face, and it'll mean that face belongs to the specified sector. That's why we
      // first go for the inclusion/intersection check, and after that we go for the angles.

      // its obvious it is in sector, if one or both points are inside the box
      if (((v_bound1.x >= sector_left) && (v_bound1.x <= sector_right) && (v_bound1.y >= sector_bottom) && (v_bound1.y <= sector_top))
          || ((v_bound2.x >= sector_left) && (v_bound2.x <= sector_right) && (v_bound2.y >= sector_bottom) && (v_bound2.y <= sector_top)))
         return (TRUE); // if so, segment belongs to sector

      // at this point, NONE of the segment bounds are inside the box

      // is this segment vertical ?
      if (v_bound1.x == v_bound2.x)
      {
         // is this vertical segment comprised between left and righ boundaries of the sector ?
         if ((v_bound1.x >= sector_left) && (v_bound1.x <= sector_right))
         {
            // if so, it belongs to sector if, and only if, the bottom bound is lower than the
            // bottom limit of the sector and the top bound is higher than the top limit of it.

            // is the first bound the bottom bound ?
            if (v_bound1.y < v_bound2.y)
            {
               if ((v_bound1.y <= sector_bottom) && (v_bound2.y >= sector_top))
                  return (TRUE); // segment crosses the sector completely
            }
            else
            {
               if ((v_bound2.y <= sector_bottom) && (v_bound1.y >= sector_top))
                  return (TRUE); // segment crosses the sector completely
            }
         }

         // if this is not the case, either the segment is :
         // - completely on the left of the sector
         // - completely on the right of the sector
         // - completely beyond the top of the sector
         // - or it is completely under the bottom of it.
      }

      // else is this segment horizontal ?
      else if (v_bound1.y == v_bound2.y)
      {
         // is horizontal segment comprised between the bottom and top boundaries of the sector ?
         if ((v_bound1.y >= sector_bottom) && (v_bound1.y <= sector_top))
         {
            // if so, it belongs to sector if, and only if, the left bound is beyond the left
            // limit of the sector and the right bound is beyond the right limit of it.

            // is the first bound the left bound ?
            if (v_bound1.x < v_bound2.x)
            {
               if ((v_bound1.x <= sector_left) && (v_bound2.x >= sector_right))
                  return (TRUE); // segment crosses the sector completely
            }
            else
            {
               if ((v_bound2.x <= sector_left) && (v_bound1.x >= sector_right))
                  return (TRUE); // segment crosses the sector completely
            }
         }

         // if this is not the case, either the segment is :
         // - completely on the left of the sector
         // - completely on the right of the sector
         // - completely beyond the top of the sector
         // - or it is completely under the bottom of it.
      }

      // else segment is neither horizontal nor vertical, but just bent (grr)
      else
      {
         // we have to compute the bastard's equation.

         // arrange the bounds of the segment in the right order relatively to the X axis
         if (v_bound1.x < v_bound2.x)
         {
            segment_left = v_bound1;
            segment_right = v_bound2;
         }
         else
         {
            segment_left = v_bound2;
            segment_right = v_bound1;
         }

         // the equation of a line is under the form y = ax + b, we need to find a and b.
         a = (v_bound2.x - v_bound1.y) / (v_bound2.x - v_bound1.x); // compute the slope : a
         b = v_bound1.y - a * v_bound1.x; // compute the Y offset at origin : b

         // at this point, none of the segment bounds are inside the box ; but now, we do know its
         // equation, so we can check if the segment intersects either of the sector's boundaries.

         // compute coordinates of intersection point of line and sector's LEFT bound
         // coordinates are : (x, y)
         x = sector_left;
         y = a * x + b;

         // does this intersection point belongs to both segments ?
         if ((x >= segment_left.x) && (x <= segment_right.x) && (y >= segment_left.y) && (y <= segment_right.y)
             && (y >= sector_bottom) && (y <= sector_top))
            return (TRUE); // the segment crosses the sector's LEFT boundary

         // compute coordinates of intersection point of line and sector's RIGHT bound
         // coordinates are : (x, y)
         x = sector_right;
         y = a * x + b;

         // does this intersection point belongs to both segments ?
         if ((x >= segment_left.x) && (x <= segment_right.x) && (y >= segment_left.y) && (y <= segment_right.y)
             && (y >= sector_bottom) && (y <= sector_top))
            return (TRUE); // the segment crosses the sector's LEFT boundary

         // at this point, none of the segment bounds are inside the box NOR does it intersects
         // the left and right boundaries of the sector ; let's turn its equation upside down so
         // that we have x = f(y)

         // compute coordinates of intersection point of line and sector's TOP bound
         // coordinates are : (x, y)
         y = sector_top;
         x = (y - b) / a;

         // does this intersection point belongs to both segments ?
         if ((x >= segment_left.x) && (x <= segment_right.x) && (y >= segment_left.y) && (y <= segment_right.y)
             && (x >= sector_left) && (x <= sector_right))
            return (TRUE); // the segment crosses the sector's TOP boundary

         // compute coordinates of intersection point of line and sector's BOTTOM bound
         // coordinates are : (x, y)
         y = sector_bottom;
         x = (y - b) / a;

         // does this intersection point belongs to both segments ?
         if ((x >= segment_left.x) && (x <= segment_right.x) && (y >= segment_left.y) && (y <= segment_right.y)
             && (x >= sector_left) && (x <= sector_right))
            return (TRUE); // the segment crosses the sector's BOTTOM boundary
      }

      // at this point, we are assured that none of the segment bounds are inside the box AND
      // that it intersects NONE of the four segments that make the boundaries of the sector.

      // now sum up all the angles corner - point - next corner to see if we have 360 (thx botman)
      angle += fabs (AngleOfVectors ((Vector (v_bound1.x, v_bound1.y, 0) - Vector (sector_left, sector_bottom, 0)),
                                     (Vector (v_bound2.x, v_bound2.y, 0) - Vector (sector_left, sector_bottom, 0))));

      // and go looping again for the next edge of the face...
   }

   // okay, here, NONE of the segment inclusion/intersection checks succeeded, so it's time to
   // check whether we're in that particular case where the sector is completely included within
   // the face, using the angle method we computed earlier.

   // if the resulting angle is close to 360, then the point is likely to be on the face
   if (fabs (WrapAngle (angle - 360)) < 0.1)
      return (TRUE); // sector is included within this face, so face belongs to sector

   return (FALSE); // all our checks failed, face can't possibly belong to sector, return FALSE.
}


bool LoadWorldMap (void)
{
   // this function checks if a world map created by LookDownOnTheWorld() exists in a .map file
   // on disk, and in case it does, opens and loads it, and returns TRUE if the loading process
   // completed successfully. If such a map doesn't exist or an error occured in the loading
   // process, this function returns FALSE. The format of a .map file is divided into chunks.
   // For an explanation of the different chunks of which .map files are made, refer to the
   // SaveWorldMap() function.

   FILE *fp;
   MFILE mfp;
   char bsp_file_path[256], map_file_path[256];
   int bsp_file_size, map_file_size, i, j, face_index, corner_index, array_index;

   // first look for a valid worldmap file...
   sprintf (bsp_file_path, "maps/%s.bsp", server.map_name); // build BSP file path
   sprintf (map_file_path, "racc/knowledge/%s/%s.map", server.mod_name, server.map_name); // build map file path

   // load the bsp file and get its actual size (can't fail to do this, the map is already booting)
   memset (&mfp, 0, sizeof (mfp));
   mfp.data = (char *) LOAD_FILE_FOR_ME (bsp_file_path, &bsp_file_size); // load bsp file
   FREE_FILE (mfp.data); // and close it

   // load the map file and get the size it claims the BSP file to have
   fp = fopen (map_file_path, "rb");
   if (fp == NULL)
      return (FALSE); // map file not found, return an error condition
   fseek (fp, 0, SEEK_SET); // seek at start of file
   fseek (fp, sizeof ("RACCMAP"), SEEK_CUR); // skip the RACCMAP tag
   fseek (fp, sizeof ("[filesize]"), SEEK_CUR); // skip the filesize chunk tag
   fread (&map_file_size, sizeof (map_file_size), 1, fp); // and read the map file size there
   fclose (fp); // close the worldmap file (we just needed this data)

   // is the recorded file size NOT the same as the actual BSP file size ?
   if (bsp_file_size != map_file_size)
      return (FALSE); // map file is badly authenticated, return an error condition

   // else world map file is OK, load its data completely this time
   fp = fopen (map_file_path, "rb"); // open this file again in binary read mode
   if (fp == NULL)
      return (FALSE); // error, can't open worldmap file

   fseek (fp, 0, SEEK_SET); // seek at start of file
   fseek (fp, sizeof ("RACCMAP"), SEEK_CUR); // skip the RACCMAP tag
   fseek (fp, sizeof ("[filesize]"), SEEK_CUR); // skip the filesize chunk tag
   fseek (fp, sizeof (long), SEEK_CUR); // skip the filesize chunk data (we already read it)
   fseek (fp, sizeof ("[worldsize]"), SEEK_CUR); // skip the worldsize chunk tag

   // read the world size chunk data
   fread (&map.v_worldmins, sizeof (Vector), 1, fp);
   fread (&map.v_worldmaxs, sizeof (Vector), 1, fp);

   fseek (fp, sizeof ("[walkfaces]"), SEEK_CUR); // seek at start of walkable faces chunk data

   // read the walkable faces lump (and mallocate for it on the fly...)
   fread (&map.walkfaces_count, sizeof (long), 1, fp);
   if (map.walkfaces_count == 0)
      return (FALSE); // error, no walkable faces have been recorded

   ServerConsole_printf ("RACC: Fetching world map"); // tell people what we are doing

   map.walkfaces = (walkface_t *) malloc (map.walkfaces_count * sizeof (walkface_t));
   if (map.walkfaces == NULL)
      TerminateOnError ("Fatal: Unable to allocate enough memory for world analysis\n");
   for (face_index = 0; face_index < map.walkfaces_count; face_index++)
   {
      fread (&map.walkfaces[face_index].corner_count, sizeof (long), 1, fp); // read # of corners for this face
      map.walkfaces[face_index].v_corners = (Vector *) malloc (map.walkfaces[face_index].corner_count * sizeof (Vector));
      if (map.walkfaces[face_index].v_corners == NULL)
         TerminateOnError ("Fatal: Unable to allocate enough memory for world analysis\n");
      map.walkfaces[face_index].v_delimiters = (Vector *) malloc (map.walkfaces[face_index].corner_count * sizeof (Vector));
      if (map.walkfaces[face_index].v_delimiters == NULL)
         TerminateOnError ("Fatal: Unable to allocate enough memory for world analysis\n");
      for (corner_index = 0; corner_index < map.walkfaces[face_index].corner_count; corner_index++)
      {
         fread (&map.walkfaces[face_index].v_corners[corner_index], sizeof (Vector), 1, fp);
         fread (&map.walkfaces[face_index].v_delimiters[corner_index], sizeof (Vector), 1, fp);
      }
   }

   fseek (fp, sizeof ("[topology]"), SEEK_CUR); // seek at start of topology chunk data

   // read the topology chunk data (and mallocate for it on the fly...)
   fread (&map.parallels_count, sizeof (map.parallels_count), 1, fp);
   fread (&map.meridians_count, sizeof (map.meridians_count), 1, fp);
   for (i = 0; i < map.parallels_count; i++)
   {
      for (j = 0; j < map.meridians_count; j++)
      {
         fread (&map.topology[i][j].faces_count, sizeof (long), 1, fp); // read # faces in sector

         // if we actually need to mallocate some space for the face pointers, do it
         if (map.topology[i][j].faces_count > 0)
            map.topology[i][j].faces = (walkface_t **) malloc (map.topology[i][j].faces_count * sizeof (walkface_t *));
         else
            map.topology[i][j].faces = (walkface_t **) malloc (sizeof (walkface_t *)); // failsafe pointer

         // check for validity of malloced space
         if (map.topology[i][j].faces == NULL)
            TerminateOnError ("Fatal: Unable to allocate enough memory for world analysis\n");

         // init the first pointer to some failsafe value
         map.topology[i][j].faces[0] = &map.walkfaces[0];

         // translate each face array index into a pointer
         for (face_index = 0; face_index < map.topology[i][j].faces_count; face_index++)
         {
            fread (&array_index, sizeof (long), 1, fp);
            
            // test this index against overflow
            if ((array_index < 0) || (array_index >= map.walkfaces_count))
               TerminateOnError ("LoadWorldMap(): bad face array index %d (max %d) at [%d][%d], index %d/%d\n", array_index, map.walkfaces_count - 1, i, j, face_index, map.topology[i][j].faces_count);

            map.topology[i][j].faces[face_index] = (walkface_t *) ((unsigned long) map.walkfaces + array_index * sizeof (walkface_t));

            // test this pointer against access violation (pointers are plain evil)
            if ((map.topology[i][j].faces[face_index] < &map.walkfaces[0]) || (map.topology[i][j].faces[face_index] > &map.walkfaces[map.walkfaces_count - 1]))
               TerminateOnError ("LoadWorldMap(): bad face pointer %d (range %d - %d) at [%d][%d], index %d/%d\n", map.topology[i][j].faces[face_index], &map.walkfaces[0], &map.walkfaces[map.walkfaces_count - 1], i, j, face_index, map.topology[i][j].faces_count);
         }
      }

      ServerConsole_printf ("."); // print a trailing dot as a progress bar
   }

   // finished, terminate the progress bar
   ServerConsole_printf (" done\n   %d parallels, %d meridians, %.2f kilobytes world data\n",
                         map.parallels_count, map.meridians_count, (float) ftell (fp) / 1024);

   fclose (fp); // close the file
   return (TRUE); // and return the error state (no error)
}


int SaveWorldMap (int bsp_file_size)
{
   // this function saves the loaded world map created by LookDownOnTheWorld() in a .map file on
   // disk. The format of this file is divided into chunks. The first chunk is an authentication
   // tag, which has the value "RACCMAP" followed by an end of string null marker. All the other
   // chunks are explicitly named in ASCII characters in the file, preceding the chunk data. The
   // chunks coming after the "RACCMAP" marker are :
   // [filesize] - tells the size of the BSP file this world map has been drawn for
   // [worldsize] - gives info about the virtual world's bounding box mins and maxs limits
   // [walkfaces] - array of the walkable faces data, involving their corners and delimiters
   // [topology] - number of parallels and meridians in this map, followed by the sector hashtable

   FILE *fp;
   char map_file_path[256];
   int i, j, face_index, corner_index, array_index, size;

   // build the world map file path
   sprintf (map_file_path, "racc/knowledge/%s/%s.map", server.mod_name, server.map_name);

   fp = fopen (map_file_path, "wb"); // open or create such a file in binary write mode
   if (fp == NULL)
      TerminateOnError ("Unable to save new worldmap to %s\n", map_file_path);

   fseek (fp, 0, SEEK_SET); // seek at start

   // write the authentication chunk
   fwrite ("RACCMAP", sizeof ("RACCMAP"), 1, fp);

   // don't write the file size chunk yet (will do this when the map will be fully saved)
   // this ensure an error in the save process won't let a badly authenticated file on disk
   fwrite ("[filesize]", sizeof ("[filesize]"), 1, fp);
   fwrite ("\0\0\0\0", sizeof (long), 1, fp); // fill the field with zeroes (temporarily)

   // write the world size chunk
   fwrite ("[worldsize]", sizeof ("[worldsize]"), 1, fp);
   fwrite (&map.v_worldmins, sizeof (Vector), 1, fp);
   fwrite (&map.v_worldmaxs, sizeof (Vector), 1, fp);

   // write the walkable faces chunk
   fwrite ("[walkfaces]", sizeof ("[walkfaces]"), 1, fp);
   fwrite (&map.walkfaces_count, sizeof (long), 1, fp); // write the number of faces
   for (face_index = 0; face_index < map.walkfaces_count; face_index++)
   {
      fwrite (&map.walkfaces[face_index].corner_count, sizeof (long), 1, fp); // write # of corners for this face
      for (corner_index = 0; corner_index < map.walkfaces[face_index].corner_count; corner_index++)
      {
         fwrite (&map.walkfaces[face_index].v_corners[corner_index], sizeof (Vector), 1, fp); // write the corner
         fwrite (&map.walkfaces[face_index].v_delimiters[corner_index], sizeof (Vector), 1, fp); // write the face delimiter
      }
   }

   // write the topology chunk
   fwrite ("[topology]", sizeof ("[topology]"), 1, fp);
   fwrite (&map.parallels_count, sizeof (long), 1, fp);
   fwrite (&map.meridians_count, sizeof (long), 1, fp);
   for (i = 0; i < map.parallels_count; i++)
   {
      for (j = 0; j < map.meridians_count; j++)
      {
         fwrite (&map.topology[i][j].faces_count, sizeof (long), 1, fp); // write # faces in sector
         for (face_index = 0; face_index < map.topology[i][j].faces_count; face_index++)
         {
            // translate the pointer address into an array relative index
            array_index = WalkfaceIndexOf (map.topology[i][j].faces[face_index]);
            fwrite (&array_index, sizeof (long), 1, fp);
         }
      }

      ServerConsole_printf ("."); // print a trailing dot as a progress bar
   }

   size = (int) ftell (fp); // get the file size, i.e our current position before we rewind

   // now we're ready to write the authentication lump
   fseek (fp, 0, SEEK_SET); // rewind the file
   fseek (fp, sizeof ("RACCMAP"), SEEK_CUR); // skip the RACCMAP tag
   fseek (fp, sizeof ("[filesize]"), SEEK_CUR); // skip the filesize chunk tag
   fwrite (&bsp_file_size, sizeof (long), 1, fp); // and write the file size

   fclose (fp); // finished, close the file

   return (size); // and return the world data size
}


void ShowTheWayAroundToBots (edict_t *pPlayer)
{
   // this is the function that makes the bots monitor the player's movement. When a player
   // incidentially calls this function, he builds an information structure about his movement,
   // so that all the bots who can see him can remember that this player passed from one place
   // to another, where and how he did. Typically, each frame a player moves, this function is
   // called upon him, in order to keep track of the "reachability" of his goal (either he needs
   // to climb a ladder, wait on a platform, fall in a pit or anything else), and then, when it
   // is detected that he just reached another walkface, all the bots in sight are told to update
   // their navigation links array so that they remember this path later. I just wasn't feeling
   // like doing heaps and lots of unreliable math at server boot time to build the navigation
   // arrays straight out of the navmesh. Crafty, eh ? =)

   walkface_t *new_face;
   navnode_t *new_node, *node_from;
   int player_index, bot_index, face_index;
   char link_index;

   player_index = ENTINDEX (pPlayer) - 1; // get this player's index

   // is this player falling fast enough to get some damage ?
   if ((pPlayer->v.flFallVelocity > MAX_SAFEFALL_SPEED) && !IsOnFloor (pPlayer))
      players[player_index].face_reachability |= REACHABILITY_FALLEDGE; // remember it

   // else is this player moving abnormally fast (meaning he's using the longjump) ?
   else if (pPlayer->v.velocity.Length2D () > 500)
      players[player_index].face_reachability |= REACHABILITY_LONGJUMP; // remember it

   // else is this player climbing a ladder ?
   else if ((pPlayer->v.velocity.z != 0) && IsFlying (pPlayer) && !IsOnFloor (pPlayer))
      players[player_index].face_reachability |= REACHABILITY_LADDER; // remember it

   // else if the engine knows a ground entity for this player...
   else if (!FNullEnt (pPlayer->v.groundentity))
   {
      // is this player riding an elevator ?
      if ((pPlayer->v.velocity.z != 0)
          && (strcmp ("func_door", STRING (pPlayer->v.groundentity->v.classname)) == 0))
         players[player_index].face_reachability |= REACHABILITY_ELEVATOR; // remember it

      // else is this player riding a bobbing platform ?
      else if ((pPlayer->v.velocity != g_vecZero)
               && (strcmp ("func_train", STRING (pPlayer->v.groundentity->v.classname)) == 0))
         players[player_index].face_reachability |= REACHABILITY_PLATFORM; // remember it

      // else is this player standing on a conveyor ?
      else if ((pPlayer->v.velocity != g_vecZero)
               && (strcmp ("func_conveyor", STRING (pPlayer->v.groundentity->v.classname)) == 0))
         players[player_index].face_reachability |= REACHABILITY_CONVEYOR; // remember it

      // else is this player riding a train ?
      else if ((pPlayer->v.velocity != g_vecZero)
               && (strcmp ("func_tracktrain", STRING (pPlayer->v.groundentity->v.classname)) == 0))
         players[player_index].face_reachability |= REACHABILITY_TRAIN; // remember it
   }

   // now if the player is standing on some ground...
   if (IsOnFloor (pPlayer))
   {
      // check for the face this player is walking on currently
      new_face = WalkfaceUnder (pPlayer); // get this player's ground face

      // now ensure we know both where the player came from AND where it is right now,
      // and check if this player has just moved onto another walkface
      if ((players[player_index].pFaceAtFeet != NULL) && (new_face != NULL)
          && (new_face != players[player_index].pFaceAtFeet))
      {
         // cycle through all bot slots to find the bots who have this player in sight
         for (bot_index = 0; bot_index < *server.max_clients; bot_index++)
         {
            if (!bots[bot_index].is_active || FNullEnt (bots[bot_index].pEdict))
               continue; // discard unused slots

            if ((DebugLevel.navigation == 0) && (bots[bot_index].pEdict != pPlayer) && (BotCanSeeOfEntity (&bots[bot_index], pPlayer) == g_vecZero))
               continue; // discard this bot if it doesn't have our player in sight

            // find new node player just reached (its index is the same as the destination face index)
            new_node = &bots[bot_index].BotBrain.PathMemory[WalkfaceIndexOf (new_face)];

            // loop through all the entrypoints the bot knows for this face's node
            for (link_index = 0; link_index < new_node->links_count; link_index++)
               if (new_node->links[link_index].node_from->walkface == players[player_index].pFaceAtFeet)
                  break; // break when an entrypoint to this face already exists in the bot's nav brain

            // it's time to update this bot's path memory

            // if there are already too much navlinks for this node, erase one
            if (link_index == 8)
               link_index = RANDOM_LONG (0, 7); // pick a slot at random

            // find the starting face index (it's the same as the navnode index) and find the
            // starting node (it's the node at the starting face)
            face_index = WalkfaceIndexOf (players[player_index].pFaceAtFeet);
            node_from = &bots[bot_index].BotBrain.PathMemory[face_index];

            // write the new navlink (destination, reachability, origin). This is written
            // directly in bot's memory since we linked the pointer to there just above.
            new_node->links[link_index].node_from = node_from;
            new_node->links[link_index].reachability = players[player_index].face_reachability;
            new_node->links[link_index].v_origin = pPlayer->v.origin;

            // if navigation debug level is VERY high, tell us what the bot found out
            if (DebugLevel.navigation > 2)
               ServerConsole_printf ("Bot %s found a way to get from walkface %d to %d by %s!\n",
                                     STRING (bots[bot_index].pEdict->v.netname),
                                     face_index, WalkfaceIndexOf (new_face),
                                     (players[player_index].face_reachability & REACHABILITY_FALLEDGE ? "FALLING" :
                                      (players[player_index].face_reachability & REACHABILITY_LADDER ? "LADDER" :
                                       (players[player_index].face_reachability & REACHABILITY_ELEVATOR ? "ELEVATOR" :
                                        (players[player_index].face_reachability & REACHABILITY_PLATFORM ? "PLATFORM" :
                                         (players[player_index].face_reachability & REACHABILITY_CONVEYOR ? "CONVEYOR" :
                                          (players[player_index].face_reachability & REACHABILITY_TRAIN ? "TRAIN" :
                                           (players[player_index].face_reachability & REACHABILITY_LONGJUMP ? "LONGJUMP" :
                                            "normal wandering (incl.jumping)"))))))));

            // have we found NO previous entrypoint for this face ?
            if ((link_index == new_node->links_count) && (new_node->links_count < 8))
               new_node->links_count++; // this node holds now one link more (up to 8)

            // is this player the listen server client AND do we want to debug navigation ?
            if ((pPlayer == pListenserverEntity) && (DebugLevel.navigation > 1))
            {
               // draw the face and the sector to which it belongs
               UTIL_DrawWalkface (pListenserverEntity, new_node->walkface, 600, 0, 255, 0);
               UTIL_DrawSector (pListenserverEntity, SectorUnder (pPlayer), 50, 255, 0, 0);

               // draw each of the navlinks the bot knows
               for (link_index = 0; link_index < new_node->links_count; link_index++)
                  UTIL_DrawNavlink (pListenserverEntity, &new_node->links[link_index], 600);
            }
         }

         players[player_index].face_reachability = 0; // reset his face's reachability now
      }

      // remember the face this player has been standing on for the next call of this function
      if (new_face != NULL)
         players[player_index].pFaceAtFeet = new_face; // but discard NULL walkfaces
   }

   return; // finished the one-man show :)
}


sector_t *SectorUnder (edict_t *pEntity)
{
   // this function returns the topological sector in which pEntity is currently located. All
   // the local variables have been declared static to speedup recurrent calls of this function.

   static int i, j;
   static Vector v_entity_origin;

   if (FNullEnt (pEntity) || !IsOnFloor (pEntity))
      return (NULL); // reliability check

   // get a quick access to the entity origin
   if (pEntity->v.absmin != g_vecZero)
      v_entity_origin = (pEntity->v.absmin + pEntity->v.absmax) / 2; // this entity has a bbox
   else
      v_entity_origin = pEntity->v.origin; // this entity is a point-based entity

   // compute the latitude and longitude in the topologic array
   i = (int) ((v_entity_origin.x - map.v_worldmins.x) / (map.v_worldmaxs.x - map.v_worldmins.x) * map.parallels_count);
   j = (int) ((v_entity_origin.y - map.v_worldmins.y) / (map.v_worldmaxs.y - map.v_worldmins.y) * map.meridians_count);

   // handle the cases where the delimiter is just on the right or upper edge of the array...
   if (i > map.parallels_count - 1)
      i = map.parallels_count - 1;
   if (j > map.meridians_count - 1)
      j = map.meridians_count - 1;

   return (&map.topology[i][j]); // and return a pointer to the appropriate sector
}


sector_t *SectorUnder (Vector v_location)
{
   // this function returns the topological sector in which v_location is currently located. All
   // the local variables have been declared static to speedup recurrent calls of this function.

   static int i, j;

   // compute the latitude and longitude in the topologic array
   i = (int) ((v_location.x - map.v_worldmins.x) / (map.v_worldmaxs.x - map.v_worldmins.x) * map.parallels_count);
   j = (int) ((v_location.y - map.v_worldmins.y) / (map.v_worldmaxs.y - map.v_worldmins.y) * map.meridians_count);

   // handle the cases where the delimiter is just on the right or upper edge of the array...
   if (i > map.parallels_count - 1)
      i = map.parallels_count - 1;
   if (j > map.meridians_count - 1)
      j = map.meridians_count - 1;

   return (&map.topology[i][j]); // and return a pointer to the appropriate sector
}


walkface_t *WalkfaceUnder (edict_t *pEntity)
{
   // this function returns the ground face supporting pEntity on floor. All the local variables
   // have been made static to speedup recurrent calls of this function.

   register int face_index, corner_index;
   static float f_bbcornerdist;
   static double angle;
   static sector_t *pSector;
   static walkface_t *pFace;
   static Vector v_entity_origin, v_bound1, v_bound2;
   static TraceResult tr;

   // first reset the face pointer
   pFace = NULL;

   if (FNullEnt (pEntity) || !IsOnFloor (pEntity))
      return (NULL); // reliability check

   // get a quick access to the entity origin
   if (pEntity->v.absmin != g_vecZero)
      v_entity_origin = (pEntity->v.absmin + pEntity->v.absmax) / 2; // this entity has a bbox
   else
      v_entity_origin = pEntity->v.origin; // this entity is a point-based entity

   UTIL_TraceLine (v_entity_origin, v_entity_origin + Vector (0, 0, -9999), ignore_monsters, pEntity, &tr);
   if (tr.flFraction == 1.0)
      return (NULL); // can't find the ground ! return a NULL walkface
   if (tr.pHit != pWorldEntity)
      return (NULL); // ground is not the world entity ! return a NULL walkface

   // get the sector it is in the topology hashtable
   pSector = SectorUnder (tr.vecEndPos);

   // loop through all the face we know to be in this topological zone
   for (face_index = 0; face_index < pSector->faces_count; face_index++)
   {
      pFace = pSector->faces[face_index]; // quick access to the face

      angle = 0; // reset angle

      // loop though the corners of this face...
      for (corner_index = 0; corner_index < pFace->corner_count; corner_index++)
      {
         // locate the first vertice of this corner
         v_bound1 = pFace->v_corners[corner_index];

         // locate the second vertice of this corner
         if (corner_index < pFace->corner_count - 1)
            v_bound2 = pFace->v_corners[corner_index + 1]; // next corner in the array
         else
            v_bound2 = pFace->v_corners[0]; // loop back to corner zero at last corner

         // sum up all the angles corner - entity - next corner and check if we have 360 (thx botman)
         angle += fabs (AngleOfVectors ((v_bound1 - tr.vecEndPos), (v_bound2 - tr.vecEndPos)));
      }

      // if the resulting angle is close to 360, then the point is likely to be on the face
      if (fabs (WrapAngle (angle - 360)) < 0.1)
         return (pFace); // assume entity is on this face
   }

   // if navigation debug level is very high, let us know that we couldn't find any walkface
   if (DebugLevel.navigation > 2)
      ServerConsole_printf ("RACC: WalkfaceUnder() could not determine walkface under entity #%d (%s)\n", ENTINDEX (pEntity), STRING (pEntity->v.classname));

   return (NULL); // not found a face on which entity could be on...
}


walkface_t *WalkfaceUnder (Vector v_location)
{
   // this function returns the ground face under v_location on the floor. The local variables
   // have been made static to speedup recurrent calls of this function.

   register int face_index, corner_index;
   static float angle;
   static sector_t *pSector;
   static walkface_t *pFace;
   static Vector v_lowered_location, v_bound1, v_bound2;

   // first reset the face pointer
   pFace = NULL;

   // get a quick access to a lowered version of v_location
   v_lowered_location = DropToFloor (v_location); // drop v_location on the floor

   if (v_lowered_location == g_vecZero)
      return (NULL); // reliability check

   // get the sector it is in the topology hashtable
   pSector = SectorUnder (v_lowered_location);

   // loop through all the face we know to be in this topological sector
   for (face_index = 0; face_index < pSector->faces_count; face_index++)
   {
      pFace = pSector->faces[face_index]; // quick access to the face

      angle = 0; // reset angle

      // loop though the corners of this face...
      for (corner_index = 0; corner_index < pFace->corner_count; corner_index++)
      {
         // locate the first vertice of this corner
         v_bound1 = pFace->v_corners[corner_index];

         // locate the second vertice of this corner
         if (corner_index < pFace->corner_count - 1)
            v_bound2 = pFace->v_corners[corner_index + 1]; // next corner in the array
         else
            v_bound2 = pFace->v_corners[0]; // loop back to corner zero at last corner

         // sum up all the angles corner - location - next corner and check if we have 360 (thx botman)
         angle += fabs (AngleOfVectors ((v_bound1 - v_lowered_location), (v_bound2 - v_lowered_location)));
      }

      // if the resulting angle is close to 360, then the point is likely to be on the face
      if (fabs (WrapAngle (angle - 360)) < 0.1)
         return (pFace); // assume location vector is on this face
   }

   // if navigation debug level is very high, let us know that we couldn't find any walkface
   if (DebugLevel.navigation > 2)
      ServerConsole_printf ("RACC: WalkfaceUnder() could not determine walkface for location (%.1f, %.1f, %.1f)\n", v_location.x, v_location.y, v_location.z);

   return (NULL); // not found a face to which location vector could belong...
}


Vector NearestDelimiterOf (edict_t *pEntity)
{
   // this function is a superset of the WalkfaceUnder() function. It returns, amongst the
   // delimiters of the ground face supporting pEntity on the floor, the closest delimiter to
   // pEntity. Local variables have been made static to speedup recurrent calls of this function.

   static walkface_t *pFace;
   static Vector v_entity_bottom, v_nearest;
   static float distance, nearest_distance;
   register int index;

   if (FNullEnt (pEntity) || !IsOnFloor (pEntity))
      return (g_vecZero); // reliability check

   // get a quick access to the entity bottom origin
   if (pEntity->v.absmin != g_vecZero)
   {
      v_entity_bottom = (pEntity->v.absmin + pEntity->v.absmax) / 2; // take center of bounding box
      v_entity_bottom.z = pEntity->v.absmin.z; // lower z coordinate to entity's absmin
   }
   else
      v_entity_bottom = pEntity->v.origin; // else take the origin the engine knows

   pFace = WalkfaceUnder (pEntity); // get the walkable face it is on

   if (pFace == NULL)
      return (g_vecZero); // reliability check

   nearest_distance = 9999.0;

   // for each delimiter on this face, loop for the nearest
   for (index = 0; index < pFace->corner_count; index++)
   {
      distance = (pFace->v_delimiters[index] - v_entity_bottom).Length (); // delimiter to entity
      if (distance < nearest_distance)
      {
         nearest_distance = distance; // remember the nearest distance
         v_nearest = pFace->v_delimiters[index]; // remember the nearest delimiter
      }
   }

   return (v_nearest); // and return it
}


int WalkfaceIndexOf (walkface_t *walkface)
{
   // this function converts a walkface pointer into its corresponding index in the global
   // walkfaces array. Local variables have been declared static to speedup recurrent calls of
   // this function.

   static int index;

   if (walkface == NULL)
      TerminateOnError ("WalkfaceIndexOf(): function called with NULL walkface\n");

   index = -1; // first set index to a bad value, for later error checking
   index = ((unsigned long) walkface - (unsigned long) map.walkfaces) / sizeof (walkface_t);

   // check for the index validity (it must ALWAYS be valid, so bomb out on error)
   if ((index < 0) || (index > map.walkfaces_count - 1))
      TerminateOnError ("WalkfaceIndexOf(): bad face array index %d (range 0-%d)\n", index, map.walkfaces_count - 1);

   return (index); // looks like we found a valid index, so return it
}


void FreeMapData (void)
{
   // this function is in charge of freeing all the memory space allocated for the map data,
   // because the map is being shutdown. A check is made upon the space pointer we want to free
   // not to free it twice in case it'd have already been done (malloc and free implementations
   // are usually so crappy they hardly ever give error messages, rather crash without warning).
   // For safety reasons, we also reset the pointer to NULL, for further malloc()s not to be
   // confused about what to allocate and where. Note, it is always safest to free things in the
   // reverse order they have been allocated, because of interdependency reasons (for example,
   // the map topology hashtable relies on the map walkfaces array).

   int i, j;

   // for each slot in the topological hashtable, see if we need to free something there
   for (i = 0; i < map.parallels_count; i++)
      for (j = 0; j < map.meridians_count; j++)
      {
         // do we need to free this slot in the map's topological hashtable ?
         if (map.topology[i][j].faces)
            free (map.topology[i][j].faces); // don't forget to free the hasthable data...
         map.topology[i][j].faces = NULL;
      }

   // do we need to free the map's walkable faces database ? (probably)
   if (map.walkfaces)
   {
      // for each walkable face slot in the array...
      for (i = 0; i < map.walkfaces_count; i++)
      {
         if (map.walkfaces[i].v_corners)
            free (map.walkfaces[i].v_corners); // free the walkable face corners...
         map.walkfaces[i].v_corners = NULL;

         if (map.walkfaces[i].v_delimiters)
            free (map.walkfaces[i].v_delimiters); // ...and free the walkable face delimiters
         map.walkfaces[i].v_delimiters = NULL;
      }

      free (map.walkfaces); // then free the walkable face memory space itself
   }
   map.walkfaces = NULL;

   return;
}
