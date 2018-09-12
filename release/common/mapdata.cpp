// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
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
   // bots such as the walkable faces, the location of the face corners, and finally a world
   // map is "drawn", separating each walkable face to the parallel and meridian it refers to.
   // Such a world map is very useful for the bots not to have to cycle through all the huge map
   // data when they want to bind just one face or just one face corner. It speeds up the
   // search by square the resolution of the world map, ie by parallels * meridians times.

   char bsp_file_path[256];
   char *mfile;
   int bsp_file_size;
   int map_file_size;
   bsp_dface_t *face;
   walkface_t *walkface;
   bool is_discardable;
   int face_index;
   int corner_index;
   int walkface_index;
   int i;
   int j;
   int contents;
   Vector v_corner;
   Vector v_center;

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
   memcpy (bsp_file.dmodels, mfile + ((bsp_dheader_t *) mfile)->lumps[LUMP_MODELS].fileofs, ((bsp_dheader_t *) mfile)->lumps[LUMP_MODELS].filelen);
   memcpy (bsp_file.dvertexes, mfile + ((bsp_dheader_t *) mfile)->lumps[LUMP_VERTEXES].fileofs, ((bsp_dheader_t *) mfile)->lumps[LUMP_VERTEXES].filelen);
   memcpy (bsp_file.dplanes, mfile + ((bsp_dheader_t *) mfile)->lumps[LUMP_PLANES].fileofs, ((bsp_dheader_t *) mfile)->lumps[LUMP_PLANES].filelen);
   memcpy (bsp_file.dfaces, mfile + ((bsp_dheader_t *) mfile)->lumps[LUMP_FACES].fileofs, ((bsp_dheader_t *) mfile)->lumps[LUMP_FACES].filelen);
   memcpy (bsp_file.dsurfedges, mfile + ((bsp_dheader_t *) mfile)->lumps[LUMP_SURFEDGES].fileofs, ((bsp_dheader_t *) mfile)->lumps[LUMP_SURFEDGES].filelen);
   memcpy (bsp_file.dedges, mfile + ((bsp_dheader_t *) mfile)->lumps[LUMP_EDGES].fileofs, ((bsp_dheader_t *) mfile)->lumps[LUMP_EDGES].filelen);

   FREE_FILE (mfile); // everything is loaded, free the BSP file

   // get a quick access to the world's bounding box
   map.v_worldmins = bsp_file.dmodels[0].mins;
   map.v_worldmaxs = bsp_file.dmodels[0].maxs;

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
      is_discardable = FALSE; // assume face is valid unless known otherwise

      // if this face is NOT walkable (i.e, normal NOT pointing straight up OR it's a ceiling)
      // Some faces are completely reverted -- thanks a lot evy for the bugfix
      if (((bsp_file.dplanes[face->planenum].normal.z < 0.707106) && (face->side == 0))
          || ((bsp_file.dplanes[face->planenum].normal.z > -0.707106) && (face->side == 1)))
         continue; // discard this face

      v_center = g_vecZero; // prepare for computation of the center of the face

      // face MAY be walkable, check if it's not a top or side face (useless in the mesh...)
      for (corner_index = 0; corner_index < face->numedges; corner_index++)
      {
         v_corner = GetDFaceCornerByIndex (face, corner_index); // get each corner successively...
         v_center = v_center + v_corner; // add this corner for the center's computation

         if ((v_corner.z == map.v_worldmaxs.z)
             || (v_corner.x == map.v_worldmins.x) || (v_corner.x == map.v_worldmaxs.x)
             || (v_corner.y == map.v_worldmins.y) || (v_corner.y == map.v_worldmaxs.y))
            is_discardable = TRUE; // is it a top or side face ? then it's discardable
      }

      if (is_discardable)
         continue; // if this face was meant to be discarded, don't process it

      v_center = v_center / face->numedges;
      contents = POINT_CONTENTS (v_center + Vector (0, 0, 32));

      if ((contents == CONTENTS_SOLID) || (contents == CONTENTS_SKY))
         continue; // if nobody can stand on this face, discard it too

      map.walkfaces_count++; // we know this map to have one walkable face more
   }

   // now allocate enough memory for the faces array to hold the right number of walkable faces
   map.walkfaces = (walkface_t *) malloc (map.walkfaces_count * sizeof (walkface_t));
   if (map.walkfaces == NULL)
      TerminateOnError ("LookDownOnTheWorld(): malloc() failure on %d kbytes for walkfaces array!\n", map.walkfaces_count * sizeof (walkface_t) / 1024);

   // also allocate the minimal space for each bucket of the map topology hashtable
   for (i = 0; i < map.parallels_count; i++)
      for (j = 0; j < map.meridians_count; j++)
      {
         map.topology[i][j].faces = (walkface_t **) malloc (sizeof (walkface_t *));
         if (map.topology[i][j].faces == NULL)
            TerminateOnError ("LookDownOnTheWorld(): malloc() failure on %d bytes for first-time topological sector [%d, %d] allocation\n", sizeof (walkface_t *), i, j);
         map.topology[i][j].faces[0] = &map.walkfaces[0]; // failsafe pointer
         map.topology[i][j].faces_count = 0; // reset the faces count
      }

   // then, translate each walkable face of the BSP file into an engine-independent structure,
   // describing each face by its corners

   walkface_index = 0; // first reset the walkable faces index

   // loop through all the faces of the BSP file...
   for (face_index = 0; face_index < bsp_file.dmodels[0].numfaces; face_index++)
   {
      face = &bsp_file.dfaces[bsp_file.dmodels[0].firstface + face_index]; // quick access to face
      is_discardable = FALSE; // assume face is valid unless known otherwise

      // if this face is NOT walkable (i.e, normal NOT pointing straight up OR it's a ceiling)
      // Some faces are completely reverted -- thanks a lot evy for the bugfix
      if (((bsp_file.dplanes[face->planenum].normal.z < 0.707106) && (face->side == 0))
          || ((bsp_file.dplanes[face->planenum].normal.z > -0.707106) && (face->side == 1)))
         continue; // discard this face

      v_center = g_vecZero; // prepare for computation of the center of the face

      // face MAY be walkable, check if it's not a top or side face (useless in the mesh...)
      for (corner_index = 0; corner_index < face->numedges; corner_index++)
      {
         v_corner = GetDFaceCornerByIndex (face, corner_index); // get each corner successively...
         v_center = v_center + v_corner; // add this corner for the center's computation

         if ((v_corner.z == map.v_worldmaxs.z)
             || (v_corner.x == map.v_worldmins.x) || (v_corner.x == map.v_worldmaxs.x)
             || (v_corner.y == map.v_worldmins.y) || (v_corner.y == map.v_worldmaxs.y))
            is_discardable = TRUE; // is it a top or side face ? then it's discardable
      }

      if (is_discardable)
         continue; // if this face was meant to be discarded, don't process it

      v_center = v_center / face->numedges;
      contents = POINT_CONTENTS (v_center + Vector (0, 0, 32));

      if ((contents == CONTENTS_SOLID) || (contents == CONTENTS_SKY))
         continue; // if nobody can stand on this face, discard it too

      // allocate enough memory to hold all this face's corner information
      map.walkfaces[walkface_index].corner_count = (int) face->numedges; // number of edges
      map.walkfaces[walkface_index].v_corners = (Vector *) malloc (face->numedges * sizeof (Vector));
      if (map.walkfaces[walkface_index].v_corners == NULL)
         TerminateOnError ("LookDownOnTheWorld(): malloc() failure on %d bytes for %d-element corners array on walkface %d\n", face->numedges * sizeof (Vector), face->numedges, walkface_index);

      walkface = &map.walkfaces[walkface_index]; // quick access to walkface

      // face is walkable, loop though the edges and get the vertexes...
      for (corner_index = 0; corner_index < face->numedges; corner_index++)
      {
         v_corner = GetDFaceCornerByIndex (face, corner_index); // get each corner successively...
         walkface->v_corners[corner_index] = v_corner; // and store it as a corner of the polygon
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


Vector GetDFaceCornerByIndex (bsp_dface_t *dface, int corner_index)
{
   // this function finds, translates and returns the vector location of the specified corner
   // on the BSP dface pointed to by dface.

   int edge;
   int edge_boundary;

   if (dface == NULL)
      return (g_vecZero); // reliability check

   if (corner_index >= dface->numedges)
      TerminateOnError ("GetDFaceCornerByIndex(): corner_index out of range\n");

   // get the coordinates of the vertex of this edge...
   edge = bsp_file.dsurfedges[dface->firstedge + corner_index];

   // if its index in the BSP tree is negative...
   if (edge < 0)
   {
      edge = -edge; // revert it
      edge_boundary = 1; // consider the other side of the segment
   }
   else
      edge_boundary = 0; // else consider the first side of the segment

   // locate and return the first vertice of this edge
   return (bsp_file.dvertexes[bsp_file.dedges[edge].v[edge_boundary]].point);
}


bool WalkfaceBelongsToSector (const walkface_t *pFace, int sector_i, int sector_j)
{
   // this function returns TRUE if the walkface pointed to by pFace belongs to the topological
   // sector whose position in the global array is [sector_i][sector_j], FALSE otherwise. The
   // local variables are defined static so as to speedup recursive calls of this function, which
   // is extensively used in LookDownOnTheWorld().

   // Math code courtesy of Paul "Cheesemonster" Murphy (thanks mate !).

   register int corner_index;
   static float sector_left;
   static float sector_right;
   static float sector_top;
   static float sector_bottom;
   static float y;
   static float a;
   static float x;
   static float b;
   static float angle;
   static float angle_diff;
   static float min_angle_diff;
   static bool min_angle_diff_found;
   static Vector v_bound1;
   static Vector v_bound2;
   static Vector segment_left;
   static Vector segment_right;

   // first compute the left, right, top and bottom coordinates indices of the sector
   sector_left = map.v_worldmins.x + sector_i * ((map.v_worldmaxs.x - map.v_worldmins.x) / map.parallels_count);
   sector_right = map.v_worldmins.x + (sector_i + 1) * ((map.v_worldmaxs.x - map.v_worldmins.x) / map.parallels_count);
   sector_bottom = map.v_worldmins.y + sector_j * ((map.v_worldmaxs.y - map.v_worldmins.y) / map.meridians_count);
   sector_top = map.v_worldmins.y + (sector_j + 1) * ((map.v_worldmaxs.y - map.v_worldmins.y) / map.meridians_count);

   angle = 0; // reset angle which will hold the sum of angles to face corners

   // state that we've not found the minimal angle difference between two corners of this face
   // for the point we consider to be "inside" it. This is very important. Since a face is a
   // convex polygon with N corners, the minimal angle difference between any point in this
   // face and any of two successive corners of the face must be (360 / N) degrees. Example:
   // if you are standing strictly in the middle of a square, you will be figuring four 90
   // degree angles between each corner of the face. Stand anywhere else inside the square and
   // you will necessarily have to figure AT LEAST one of these four angles to be MORE than
   // 90 degrees. This check ensures that in case you are standing kilometers away from the
   // face, all these angles summed up eventually still being very close to zero, the algorithm
   // won't be fooled by thinking that you are standing in it (in fact, you are not only OUT of
   // it, but KILOMETERS AWAY).
   min_angle_diff = 360 / pFace->corner_count;
   min_angle_diff_found = FALSE;

   // loop though the corners of this face...
   for (corner_index = 0; corner_index < pFace->corner_count; corner_index++)
   {
      // locate the first vertice of this edge
      v_bound1 = pFace->v_corners[corner_index];

      // locate the second vertice of this edge
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
      // The walkface may still be larger than the sector and the sector comprised "inside" it.

      // figure out the angle difference between this point and one corner of the sector
      angle_diff = fabs (AngleOfVectors ((Vector (v_bound1.x, v_bound1.y, 0) - Vector (sector_left, sector_bottom, 0)),
                                         (Vector (v_bound2.x, v_bound2.y, 0) - Vector (sector_left, sector_bottom, 0))));

      // is this angle GREATER than the minimal angle difference ?
      if (angle_diff > min_angle_diff)
         min_angle_diff_found = TRUE; // ha, this indicates that this face is NOT kilometers away

      // now sum up all the angles corner - point - next corner to see if we have 360 (thx botman)
      angle += angle_diff;

      // and go looping again for the next edge of the face...
   }

   // okay, here, NONE of the segment inclusion/intersection checks succeeded, so it's time to
   // check whether we're in that particular case where the sector is completely included within
   // the face, using the angle method we computed earlier.

   // has the minimal angle difference been NOT found ?
   if (!min_angle_diff_found)
      return (FALSE); // then this face is KILOMETERS away from the sector, so forget it.

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
   char bsp_file_path[256];
   char map_file_path[256];
   int bsp_file_size;
   int map_file_size;
   int i;
   int j;
   int face_index;
   int corner_index;
   int array_index;

   // first look for a valid worldmap file...
   sprintf (bsp_file_path, "maps/%s.bsp", server.map_name); // build BSP file path
   sprintf (map_file_path, "%s/knowledge/%s/%s.map", GameConfig.racc_basedir, GameConfig.mod_name, server.map_name); // build map file path

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
      TerminateOnError ("LoadWorldMap(): Unable to allocate enough memory for world analysis\n");
   for (face_index = 0; face_index < map.walkfaces_count; face_index++)
   {
      fread (&map.walkfaces[face_index].corner_count, sizeof (long), 1, fp); // read # of corners for this face
      map.walkfaces[face_index].v_corners = (Vector *) malloc (map.walkfaces[face_index].corner_count * sizeof (Vector));
      if (map.walkfaces[face_index].v_corners == NULL)
         TerminateOnError ("LoadWorldMap(): Unable to allocate enough memory for world analysis\n");
      for (corner_index = 0; corner_index < map.walkfaces[face_index].corner_count; corner_index++)
         fread (&map.walkfaces[face_index].v_corners[corner_index], sizeof (Vector), 1, fp);
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
            TerminateOnError ("LoadWorldMap(): Unable to allocate enough memory for world analysis\n");

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
   // [walkfaces] - array of the walkable faces data, involving their corners
   // [topology] - number of parallels and meridians in this map, followed by the sector hashtable

   FILE *fp;
   char map_file_path[256];
   int i;
   int j;
   int face_index;
   int corner_index;
   int array_index;
   int size;

   // build the world map file path
   sprintf (map_file_path, "%s/knowledge/%s/%s.map", GameConfig.racc_basedir, GameConfig.mod_name, server.map_name);

   fp = fopen (map_file_path, "wb"); // open or create such a file in binary write mode
   if (fp == NULL)
      TerminateOnError ("SaveWorldMap(): Unable to save new worldmap to %s\n", map_file_path);

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
         fwrite (&map.walkfaces[face_index].v_corners[corner_index], sizeof (Vector), 1, fp); // write the corner
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


void InsertNavLink (navnode_t *node, navnode_t *node_from, Vector v_origin, short reachability, Vector v_connectvelocity)
{
   // this function inserts a navlink into the node pointed to by node COMING FROM the node
   // pointed to by node_from (don't forget the pathfinder runs *backwards*) at the position
   // specified as v_origin, having the specified reachability and connect velocity. This
   // last parameter is only relevant if the CONNECTIVITY_JUMP flag is set in the navlink's
   // reachability. This function cycles through all the existing navlinks of node, and in case
   // a similar navlink to the one we are inserting already exists, this one gets updated with
   // the new content. If no such navlink exist, a new one is created unless the whole navlink
   // list for this node is already full, in which case a random one gets overwritten.

   static char index;
   static bool already_exists;

   already_exists = FALSE; // assume link doesn't exist yet

   // loop through all the entrypoints the bot knows for this face's node
   for (index = 0; index < node->links_count; index++)
      if (node->links[index].node_from == node_from)
      {
         already_exists = TRUE; // remember this link already exists
         break; // break when an entrypoint to this face already exists in the bot's nav brain
      }

   // does a similar one NOT exist yet AND the list is already full ?
   if (index == 8)
      index = RandomLong (0, 7); // then pick a slot at random (erase a previous link)

   // write the new navlink (destination, reachability, origin). This is written
   // directly in bot's memory since we linked the pointer to there just above.
   node->links[index].node_from = node_from;
   node->links[index].reachability = reachability;
   node->links[index].v_origin = v_origin;
   node->links[index].v_connectvelocity = v_connectvelocity;

   // if navigation debug level is high AND we're spectating around, tell us what we've found out
   if ((DebugLevel.navigation > 1) && ((pListenserverPlayer->v_origin - v_origin).Length () < 50))
      printf ("%s navlink %d: %s\n",
              (node->links_count < 8 ? "Creating" : "Updating"),
              index,
              (reachability & REACHABILITY_FALLEDGE ? "FALL" :
               (reachability & REACHABILITY_LADDER ? "LADDER" :
                (reachability & REACHABILITY_ELEVATOR ? "ELEVATOR" :
                 (reachability & REACHABILITY_PLATFORM ? "PLATFORM" :
                  (reachability & REACHABILITY_CONVEYOR ? "CONVEYOR" :
                   (reachability & REACHABILITY_TRAIN ? "TRAIN" :
                    (reachability & REACHABILITY_LONGJUMP ? "LONGJUMP" :
                     (reachability & REACHABILITY_SWIM ? "SWIM" :
                      (reachability & REACHABILITY_TELEPORTER ? "TELEPORTER" :
                       (reachability & REACHABILITY_JUMP ? "JUMP" :
                        (reachability & REACHABILITY_CROUCH ? "CROUCH" :
                         "normal reachability"))))))))))));

   // have we found NO previous entrypoint for this face ?
   if ((index == node->links_count) && (node->links_count < 8))
      node->links_count++; // this node holds now one link more (up to 8)

   return; // finished, link created/updated
}


void RemoveNavLink (navnode_t *node, navlink_t *bad_link)
{
   // this function makes the bot whose player structure is pointed to by pPlayer forget the
   // navlink pointed to by bad_link that is currently an element of the navlinks array held by
   // the node pointed to by node that the bot knows. If the bad link isn't found in node's list
   // of navlinks, then the function does nothing ; else, this link is removed from the array
   // and all the other elements after this one get shuffled one rank down (so as not to have
   // any free slot in the array). One exception though: we WON'T EVER remove a navlink if it's
   // the ONLY navlink in the navnode (so as to never have "orphaned" navnodes).

   static char index;

   if (node->links_count <= 0)
      return; // cancel if too few links in the node's navlinks list

   // find the link to be removed in the navlinks array of this navnode
   index = 0; // start with link #0
   while ((index < node->links_count) && (&node->links[index] != bad_link))
      index++;

   // have we found it ?
   if (index < node->links_count)
   {
      node->links_count--; // this node has now one link less (since we're removing it)

      // if navigation debug level is high, draw the link we're removing
      if (DebugLevel.navigation > 0)
      {
         if (DebugLevel.navigation > 1)
            printf ("Removing navlink %d: %s\n",
                    index,
                    (bad_link->reachability & REACHABILITY_FALLEDGE ? "FALL" :
                     (bad_link->reachability & REACHABILITY_LADDER ? "LADDER" :
                      (bad_link->reachability & REACHABILITY_ELEVATOR ? "ELEVATOR" :
                       (bad_link->reachability & REACHABILITY_PLATFORM ? "PLATFORM" :
                        (bad_link->reachability & REACHABILITY_CONVEYOR ? "CONVEYOR" :
                         (bad_link->reachability & REACHABILITY_TRAIN ? "TRAIN" :
                          (bad_link->reachability & REACHABILITY_LONGJUMP ? "LONGJUMP" :
                           (bad_link->reachability & REACHABILITY_SWIM ? "SWIM" :
                            (bad_link->reachability & REACHABILITY_TELEPORTER ? "TELEPORTER" :
                             (bad_link->reachability & REACHABILITY_JUMP ? "JUMP" :
                              (bad_link->reachability & REACHABILITY_CROUCH ? "CROUCH" :
                               "normal reachability"))))))))))));
         UTIL_DrawLine (bad_link->v_origin + Vector (0, 0, -25),
                        bad_link->v_origin + Vector (0, 0, +25),
                        20, 255, 255, 0);
         UTIL_DrawWalkface (node->walkface, 20, 255, 255, 0);
      }
   }

   // and shuffle the rest of the links one element down (skipped if link was not found)
   while (index < node->links_count)
   {
      node->links[index] = node->links[index + 1]; // shuffle the list down
      index++;
   }

   return; // done, link is removed and list sorted again
}


void ShowTheWayAroundToBots (player_t *pPlayer)
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

   player_t *pAIPlayer;
   walkface_t *new_face;
   navnode_t *new_node;
   navnode_t *node_from;
   int client_index;
   char link_index;
   Vector v_center;
   test_result_t tr;

   // is the current framerate LOWER than 10 fps ?
   if (server.time - server.previous_time > 0.10)
   {
      pPlayer->pFaceAtFeet = NULL; // then don't track this player
      pPlayer->face_reachability = 0; // the lag will probably falsify all our results

      return; // we need at least 10Hz sampling rate to do a good job
   }

   // REACHABILITY RECORDINGS

   // is this player jumping over an obstacle ?
   if ((pPlayer->environment == ENVIRONMENT_GROUND)
       && (pPlayer->input_buttons & INPUT_KEY_JUMP)
       && !(pPlayer->prev.input_buttons & INPUT_KEY_JUMP))
   {
      pPlayer->face_reachability |= REACHABILITY_JUMP; // remember it
      pPlayer->v_lastjump = pPlayer->v_origin; // also remember where this player last jumped
   }

   // is this player ducking under an obstacle ?
   if (pPlayer->input_buttons & INPUT_KEY_DUCK)
   {
      // determine if this player NEEDS to duck...
      tr = PlayerTestHull (pPlayer,
                           pPlayer->v_origin + Vector (0, 0, -GameConfig.ducking_origin_height + GameConfig.standing_origin_height),
                           pPlayer->v_origin + Vector (0, 0, -GameConfig.ducking_origin_height + GameConfig.standing_origin_height + 1),
                           FALSE);
      if (tr.fraction < 1.0)
         pPlayer->face_reachability |= REACHABILITY_CROUCH; // yes, so remember it
   }

   // is this player falling fast enough to get some damage ?
   if ((pPlayer->environment == ENVIRONMENT_MIDAIR)
       && (pPlayer->v_velocity.z < -GameConfig.max_safefall_speed))
      pPlayer->face_reachability |= REACHABILITY_FALLEDGE; // remember it

   // has this player just spawned instantly on another point in the map ?
   if ((mission.start_time + 2.0 < server.time)
       && (pPlayer->prev.v_origin - pPlayer->v_origin).Length () > 300)
      pPlayer->face_reachability |= REACHABILITY_TELEPORTER; // remember it

   // is this player moving abnormally fast (meaning he's using the longjump) ?
   if (pPlayer->v_velocity.Length2D () > 500)
      pPlayer->face_reachability |= REACHABILITY_LONGJUMP; // remember it

   // is this player completely surrounded by water (i.e, swimming) ?
   if (pPlayer->environment == ENVIRONMENT_WATER)
      pPlayer->face_reachability |= REACHABILITY_SWIM; // remember it

   // is this player climbing a ladder ?
   if ((pPlayer->environment == ENVIRONMENT_LADDER) && (pPlayer->v_velocity.z != 0))
      pPlayer->face_reachability |= REACHABILITY_LADDER; // remember it

   // else if the engine knows a ground entity for this player...
   else if (!FNullEnt (pPlayer->pEntity->v.groundentity))
   {
      // is this player riding an elevator ?
      if ((pPlayer->v_velocity.z != 0)
          && (strcmp ("func_door", STRING (pPlayer->pEntity->v.groundentity->v.classname)) == 0))
         pPlayer->face_reachability |= REACHABILITY_ELEVATOR; // remember it

      // else is this player riding a bobbing platform ?
      else if ((pPlayer->v_velocity != g_vecZero)
               && (strcmp ("func_train", STRING (pPlayer->pEntity->v.groundentity->v.classname)) == 0))
         pPlayer->face_reachability |= REACHABILITY_PLATFORM; // remember it

      // else is this player standing on a conveyor ?
      else if ((pPlayer->v_velocity != g_vecZero)
               && (strcmp ("func_conveyor", STRING (pPlayer->pEntity->v.groundentity->v.classname)) == 0))
         pPlayer->face_reachability |= REACHABILITY_CONVEYOR; // remember it

      // else is this player riding a train ?
      else if ((pPlayer->v_velocity != g_vecZero)
               && (strcmp ("func_tracktrain", STRING (pPlayer->pEntity->v.groundentity->v.classname)) == 0))
         pPlayer->face_reachability |= REACHABILITY_TRAIN; // remember it
   }

   // END OF REACHABILITY RECORDINGS

   // now it's time to record the paths

   if ((pPlayer->environment != ENVIRONMENT_GROUND)
       || ((pPlayer->pFaceAtFeet != NULL) && (pPlayer->v_velocity == g_vecZero)))
      return; // don't do it while the player is in mid-air or not moving (useless)

   // check for the face this player is walking on currently
   new_face = WalkfaceUnder (pPlayer); // get this player's ground face

   // REACHABILITY CORRECTIONS

   // is this player landing from a jump but on the same walkface ?
   if ((pPlayer->environment == ENVIRONMENT_GROUND)
       && (pPlayer->prev.environment == ENVIRONMENT_MIDAIR)
       && (new_face == pPlayer->pFaceAtFeet))
      pPlayer->face_reachability &= ~REACHABILITY_JUMP; // then forget about this jump

   // is this player sloshing through water (i.e, walking with feet in water) ?
   if (pPlayer->environment == ENVIRONMENT_SLOSHING)
      pPlayer->face_reachability &= ~REACHABILITY_FALLEDGE; // then falling down here can't hurt

   // END OF REACHABILITY CORRECTIONS

   // now ensure we know both where the player came from AND where it is right now,
   // and check if this player has just moved onto another walkface
   if ((pPlayer->pFaceAtFeet != NULL) && (new_face != NULL)
       && (new_face != pPlayer->pFaceAtFeet))
   {
      // cycle through all bot slots to find the bots who have this player in sight
      for (client_index = 0; client_index < server.max_clients; client_index++)
      {
         pAIPlayer = &players[client_index]; // quick access to player

         if (!IsValidPlayer (pAIPlayer) || !pAIPlayer->is_racc_bot)
            continue; // discard invalid players and real clients

// DISABLING THE TWO LINES BELOW MAKE THE BOTS OMNISCIENT (helps building nav brains faster)
//         if ((DebugLevel.navigation == 0) && (pAIPlayer != pPlayer) && (BotCanSeeOfEntity (pAIPlayer, pPlayer->pEntity) == g_vecZero))
//            continue; // discard this bot if it doesn't have our player in sight
// DISABLING THE TWO LINES ABOVE MAKE THE BOTS OMNISCIENT (helps building nav brains faster)

         // find new node player just reached (its index is the same as the destination face
         // index) and find the starting node (it's the node at the starting face)
         new_node = &pAIPlayer->Bot.BotBrain.PathMemory[WalkfaceIndexOf (new_face)];
         node_from = &pAIPlayer->Bot.BotBrain.PathMemory[WalkfaceIndexOf (pPlayer->pFaceAtFeet)];

         // it's time to update this bot's path memory

         // is it a jump reachability ? if so, the navlink origin will be at the start of the jump
         if (pPlayer->face_reachability & REACHABILITY_JUMP)
            InsertNavLink (new_node, node_from, pPlayer->v_lastjump, pPlayer->face_reachability, pPlayer->v_velocity);
         else
            InsertNavLink (new_node, node_from, pPlayer->v_origin, pPlayer->face_reachability, g_vecZero);

         // is debug mode enabled AND do we want to debug navigation ?
         if (pPlayer->is_watched && (DebugLevel.navigation > 1))
         {
            // draw the face and the sector to which it belongs
            UTIL_DrawWalkface (new_node->walkface, 50, 0, 255, 0);
            UTIL_DrawSector (SectorUnder (pPlayer->v_origin), 50, 255, 0, 0);

            // draw each of the navlinks the bot knows
            for (link_index = 0; link_index < new_node->links_count; link_index++)
               UTIL_DrawNavlink (&new_node->links[link_index], 50);
         }
      }

      pPlayer->face_reachability = 0; // reset his walkface reachability now
   }

   // remember the face this player has been standing on for the next call of this function
   if (new_face != NULL)
      pPlayer->pFaceAtFeet = new_face; // but discard NULL walkfaces

   return; // finished the one-man show :)
}


sector_t *SectorUnder (Vector v_location)
{
   // this function returns the topological sector in which v_location is currently located. All
   // the local variables have been declared static to speedup recurrent calls of this function.

   static int i;
   static int j;

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


walkface_t *WalkfaceUnder (player_t *pPlayer)
{
   // this function returns the ground face supporting pPlayer on floor. All the local variables
   // have been made static to speedup recurrent calls of this function.
   // A player is GameConfig.bb_width units wide, and can climb up slopes up to 45°.

   register int face_index;
   register int corner_index;
   static double angle;
   static float angle_diff;
   static float min_angle_diff;
   static bool min_angle_diff_found;
   static float nearest_fraction;
   static sector_t *pSector;
   static walkface_t *pFace;
   static Vector v_entity_origin;
   static Vector v_hit_point;
   static Vector v_bound1;
   static Vector v_bound2;
   static test_result_t tr;

   // first reset the face pointer
   pFace = NULL;

   if (pPlayer->environment != ENVIRONMENT_GROUND)
      return (NULL); // reliability check

   // get a quick access to the entity origin
   v_entity_origin = BottomOriginOf (pPlayer->pEntity); // this entity has a bbox

   tr = PlayerTestLine (pPlayer,
                        v_entity_origin + Vector (0, 0, 10),
                        v_entity_origin + Vector (0, 0, -10));
   v_hit_point = tr.v_endposition; // remember the hit point

   // found something ?
   if (tr.fraction < 1.0)
   {
      if (tr.pHit != pWorldEntity)
         return (NULL); // ground is not the world entity ! return a NULL walkface
   }

   // else this entity is partially on the ground. This sucks, because we'll have to check the
   // 4 corners of its bounding box in order to see which one is the supporting one.
   else
   {
      nearest_fraction = 1.0; // set this to be sure

      // find out which corner of the bounding box is the supporting one
      v_entity_origin.x = pPlayer->v_origin.x - GameConfig.bb_width * 0.5; // corner 1
      v_entity_origin.y = pPlayer->v_origin.y - GameConfig.bb_width * 0.5;
      tr = PlayerTestLine (pPlayer,
                           v_entity_origin + Vector (0, 0, 10),
                           v_entity_origin + Vector (0, 0, -20));
      if ((tr.fraction < nearest_fraction) && (tr.pHit == pWorldEntity))
         v_hit_point = tr.v_endposition;

      v_entity_origin.x = pPlayer->v_origin.x - GameConfig.bb_width * 0.5; // corner 2
      v_entity_origin.y = pPlayer->v_origin.y + GameConfig.bb_width * 0.5;
      tr = PlayerTestLine (pPlayer,
                           v_entity_origin + Vector (0, 0, 10),
                           v_entity_origin + Vector (0, 0, -20));
      if ((tr.fraction < nearest_fraction) && (tr.pHit == pWorldEntity))
         v_hit_point = tr.v_endposition;

      v_entity_origin.x = pPlayer->v_origin.x + GameConfig.bb_width * 0.5; // corner 3
      v_entity_origin.y = pPlayer->v_origin.y + GameConfig.bb_width * 0.5;
      tr = PlayerTestLine (pPlayer,
                           v_entity_origin + Vector (0, 0, 10),
                           v_entity_origin + Vector (0, 0, -20));
      if ((tr.fraction < nearest_fraction) && (tr.pHit == pWorldEntity))
         v_hit_point = tr.v_endposition;

      v_entity_origin.x = pPlayer->v_origin.x + GameConfig.bb_width * 0.5; // corner 4
      v_entity_origin.y = pPlayer->v_origin.y - GameConfig.bb_width * 0.5;
      tr = PlayerTestLine (pPlayer,
                           v_entity_origin + Vector (0, 0, 10),
                           v_entity_origin + Vector (0, 0, -20));
      if ((tr.fraction < nearest_fraction) && (tr.pHit == pWorldEntity))
         v_hit_point = tr.v_endposition;

      if (nearest_fraction = 1.0)
         return (NULL); // if no frikkin hit point was found on the FOUR corners, give up. Bah.
   }

   // get the sector it is in the topology hashtable
   pSector = SectorUnder (v_hit_point);

   // loop through all the face we know to be in this topological zone
   for (face_index = 0; face_index < pSector->faces_count; face_index++)
   {
      pFace = pSector->faces[face_index]; // quick access to the face

      angle = 0; // reset angle

      // reset the minimal angle difference to be found
      min_angle_diff = 360 / pFace->corner_count;
      min_angle_diff_found = FALSE;

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

         // figure out the angle difference between this point and one corner of the sector
         angle_diff = fabs (AngleOfVectors ((v_bound1 - v_hit_point), (v_bound2 - v_hit_point)));

         // is this angle GREATER than the minimal angle difference ?
         if (angle_diff > min_angle_diff)
            min_angle_diff_found = TRUE; // ha, this indicates that this face is NOT kilometers away

         // now sum up all the angles corner - point - next corner to see if we have 360 (thx botman)
         angle += angle_diff;
      }

      // if the minimal angle difference has been found
      // AND the resulting angle is close to 360, then the point is likely to be on the face
      if (min_angle_diff_found && (fabs (WrapAngle (angle - 360)) < 0.1))
         return (pFace); // assume entity is on this face
   }

   // if navigation debug level is very high, let us know that we couldn't find any walkface
   if (pPlayer->is_watched && (DebugLevel.navigation > 2))
      ServerConsole_printf ("RACC: WalkfaceUnder() could not determine walkface under player %s\n", pPlayer->connection_name);

   return (NULL); // not found a face on which entity could be on...
}


walkface_t *WalkfaceAt (Vector v_location)
{
   // this function returns the ground face under v_location on the floor. The local variables
   // have been made static to speedup recurrent calls of this function.

   register int face_index, corner_index;
   static float angle;
   static float angle_diff;
   static float min_angle_diff;
   static bool min_angle_diff_found;
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

      // reset the minimal angle difference to be found
      min_angle_diff = 360 / pFace->corner_count;
      min_angle_diff_found = FALSE;

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

         // figure out the angle difference between this point and one corner of the sector
         angle_diff = fabs (AngleOfVectors ((v_bound1 - v_lowered_location), (v_bound2 - v_lowered_location)));

         // is this angle GREATER than the minimal angle difference ?
         if (angle_diff > min_angle_diff)
            min_angle_diff_found = TRUE; // ha, this indicates that this face is NOT kilometers away

         // now sum up all the angles corner - point - next corner to see if we have 360 (thx botman)
         angle += angle_diff;
      }

      // if the minimal angle difference has been found
      // AND the resulting angle is close to 360, then the point is likely to be on the face
      if (min_angle_diff_found && (fabs (WrapAngle (angle - 360)) < 0.1))
         return (pFace); // assume location vector is on this face
   }

   // if navigation debug level is very high, let us know that we couldn't find any walkface
   if (DebugLevel.navigation > 2)
      ServerConsole_printf ("RACC: WalkfaceUnder() could not determine walkface for location (%.1f, %.1f, %.1f)\n", v_location.x, v_location.y, v_location.z);

   return (NULL); // not found a face to which location vector could belong...
}


int WalkfaceIndexOf (walkface_t *walkface)
{
   // this function converts a walkface pointer into its corresponding index in the global
   // walkfaces array. Local variables have been declared static to speedup recurrent calls of
   // this function.

   static int index;

   if (walkface == NULL)
      TerminateOnError ("WalkfaceIndexOf(): function called with NULL walkface\n");

   // figure out the walkface index out of its offset in the walkfaces array
   index = ((unsigned long) walkface - (unsigned long) map.walkfaces) / sizeof (walkface_t);

   // check for the index validity (it must ALWAYS be valid, so bomb out on error)
   if ((index < 0) || (index > map.walkfaces_count - 1))
      TerminateOnError ("WalkfaceIndexOf(): bad face array index %d (range 0-%d)\n", index, map.walkfaces_count - 1);

   return (index); // looks like we found a valid index, so return it
}


Vector WalkfaceCenterOf (walkface_t *walkface)
{
   // this function computes and returns the vector origin of the middle (center) of the walkface
   // pointed to by walkface.

   static int index;
   Vector v_center;

   if (walkface == NULL)
      TerminateOnError ("WalkfaceCenterOf(): function called with NULL walkface\n");

   // cycle through all the corners and sum them up
   v_center = g_vecZero;
   for (index = 0; index < walkface->corner_count; index++)
      v_center = v_center + walkface->v_corners[index]; // sum up all the corners

   return (v_center / walkface->corner_count); // and return the averaged center
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
            free (map.walkfaces[i].v_corners); // free the walkable face corners
         map.walkfaces[i].v_corners = NULL;
      }

      free (map.walkfaces); // then free the walkable face memory space itself
   }
   map.walkfaces = NULL;

   return;
}
