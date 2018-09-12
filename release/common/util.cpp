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
// Rational Autonomous Cybernetic Commandos AI
//
// util.cpp
//

// TODO: finish putting comments all over the place

#include "racc.h"


// dummy global used in FindWeaponBy...()
weapon_t unknown_weapon =
{
   "", "", 0, 0, 0, 0, 0,
   {0, 0, 0, 0, 0, 0, FALSE, FALSE, FALSE, FALSE, 0, "", "", {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, 0, 0, FALSE},
   {0, 0, 0, 0, 0, 0, FALSE, FALSE, FALSE, FALSE, 0, "", "", {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, 0, 0, FALSE}
};



Vector UTIL_VecToAngles (const Vector &v_VectorIn)
{
   static float v_AnglesOut[3];

   if (v_VectorIn == g_vecZero)
      return (g_vecZero);

   VEC_TO_ANGLES (v_VectorIn, v_AnglesOut);

   return (WrapAngles (Vector (v_AnglesOut)));
}


void UTIL_TraceLine (const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t *pentIgnore, TraceResult *ptr)
{
   // this function traces a line dot by dot, starting from vecStart in the direction of vecEnd,
   // ignoring or not monsters (depending on the value of igmon, ignore_monsters or
   // dont_ignore_monsters), and stops at the first obstacle encountered, returning the results
   // of the trace in the TraceResult structure ptr. Such results are (amongst others) the
   // distance traced, the hit surface, the hit plane vector normal, etc. See the TraceResult
   // structure for details. This function allows to specify whether the trace starts "inside"
   // an entity's polygonal model, and if so, to specify that entity in pentIgnore in order to
   // ignore it as a possible obstacle.
   // this is an overloaded prototype to add IGNORE_GLASS in the same way as IGNORE_MONSTERS work.

   TRACE_LINE (vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE) | (ignoreGlass ? 0x100 : 0), pentIgnore, ptr);
}


void UTIL_TraceLine (const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr)
{
   // this function traces a line dot by dot, starting from vecStart in the direction of vecEnd,
   // ignoring or not monsters (depending on the value of igmon, ignore_monsters or
   // dont_ignore_monsters), and stops at the first obstacle encountered, returning the results
   // of the trace in the TraceResult structure ptr. Such results are (amongst others) the
   // distance traced, the hit surface, the hit plane vector normal, etc. See the TraceResult
   // structure for details. This function allows to specify whether the trace starts "inside"
   // an entity's polygonal model, and if so, to specify that entity in pentIgnore in order to
   // ignore it as a possible obstacle.

   TRACE_LINE (vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE), pentIgnore, ptr);
}


void UTIL_TraceHull (const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t *pentIgnore, TraceResult *ptr)
{
   // this function traces a hull dot by dot, starting from vecStart in the direction of vecEnd,
   // ignoring or not monsters (depending on the value of igmon, ignore_monsters or
   // dont_ignore_monsters), and stops at the first obstacle encountered, returning the results
   // of the trace in the TraceResult structure ptr, just like TraceLine. Hulls that can be traced
   // (by parameter hull_type) are point_hull (a line), head_hull (size of a crouching player),
   // human_hull (a normal body size) and large_hull (for monsters?). Not all the hulls in the
   // game can be traced here, this function is just useful to give a relative idea of spatial
   // reachability (i.e. can a hostage pass through that tiny hole ?) Also like TraceLine, this
   // function allows to specify whether the trace starts "inside" an entity's polygonal model,
   // and if so, to specify that entity in pentIgnore in order to ignore it as an obstacle.

   TRACE_HULL (vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE), hullNumber, pentIgnore, ptr);
}


edict_t *UTIL_FindEntityInSphere (edict_t *pentStart, const Vector &vecCenter, float flRadius)
{
   // this function returns the first successive element in the list of entities that is
   // located inside a sphere which center is vecCenter and within a radius of flRadius,
   // starting the search at entity pentStart. Since orphan pointers are common within the
   // Half-Life engine, we have to do a check for the returned entity validity.

   edict_t *pEntity = FIND_ENTITY_IN_SPHERE (pentStart, vecCenter, flRadius);

   if (!FNullEnt (pEntity))
      return (pEntity); // only return certified valid entities

   return (NULL); // avoid returning orphan pointers
}


edict_t *UTIL_FindEntityByString (edict_t *pentStart, const char *szKeyword, const char *szValue)
{
   // this function returns the first successive element in the list of entities that have
   // szValue in the szKeyword field of their entity variables entvars_t structure, starting the
   // search at entity pentStart. Also here, beware of orphan pointers !!!

   edict_t *pEntity = FIND_ENTITY_BY_STRING (pentStart, szKeyword, szValue);

   if (!FNullEnt (pEntity))
      return (pEntity); // only return certified valid entities

   return (NULL); // avoid returning orphan pointers
}


bool IsAlive (edict_t *pEdict)
{
   if (FNullEnt (pEdict))
      return (FALSE); // reliability check

   return (((pEdict->v.deadflag == DEAD_NO) || (pEdict->v.deadflag == DEAD_DYING))
           && (pEdict->v.health > 0)
           && !(pEdict->v.flags & FL_NOTARGET)
           && (pEdict->v.takedamage != DAMAGE_NO));
}


bool IsInPlayerFOV (edict_t *pPlayer, Vector v_location)
{
   // this function returns TRUE if the spatial vector location v_location is located inside
   // the player whose entity is pPlayer's field of view cone, FALSE otherwise.

   static Vector v_deviation;

   if (!IsValidPlayer (pPlayer))
      return (FALSE); // reliability check

   // compute deviation angles (angles between pPlayer's forward direction and v_location)
   v_deviation = WrapAngles (UTIL_VecToAngles (v_location - (pPlayer->v.origin + pPlayer->v.view_ofs))
                             - pPlayer->v.v_angle);

   // is v_location outside pPlayer's FOV width (90 degree) ?
   if (fabs (v_deviation.x) > 45)
      return (FALSE); // then v_location is not visible

   // is v_location outside pPlayer's FOV height (60 degree: consider the 4:3 screen ratio) ?
   if (fabs (v_deviation.y) > 30)
      return (FALSE); // then v_location is not visible

   return (TRUE); // else v_location has to be in pPlayer's field of view cone
}


bool FVisible (const Vector &vecOrigin, edict_t *pEdict)
{
   TraceResult tr;

   if (FNullEnt (pEdict))
      return (FALSE); // reliability check

   // don't look through water
   if ((POINT_CONTENTS (vecOrigin) == CONTENTS_WATER)
       != (POINT_CONTENTS (pEdict->v.origin + pEdict->v.view_ofs) == CONTENTS_WATER))
      return (FALSE);

   UTIL_TraceLine (pEdict->v.origin + pEdict->v.view_ofs, vecOrigin, ignore_monsters, ignore_glass, pEdict, &tr);
   if (tr.flFraction == 1.0)
      return (TRUE); // line of sight is valid.

   return (FALSE); // line of sight is not established
}


bool IsReachable (Vector v_dest, edict_t *pEdict)
{
   TraceResult tr;
   float curr_height, last_height, distance;
   Vector v_check = pEdict->v.origin;
   Vector v_direction = (v_dest - v_check).Normalize (); // 1 unit long

   if (FNullEnt (pEdict))
      return (FALSE); // reliability check

   // check for special case of both the bot and its destination being underwater...
   if ((POINT_CONTENTS (pEdict->v.origin) == CONTENTS_WATER) && (POINT_CONTENTS (v_dest) == CONTENTS_WATER))
      return (TRUE); // if so, assume it's reachable

   // now check if distance to ground increases more than jump height
   // at points between source and destination...

   UTIL_TraceLine (v_check, v_check + Vector (0, 0, -1000), ignore_monsters, pEdict, &tr);

   last_height = tr.flFraction * 1000.0; // height from ground
   distance = (v_dest - v_check).Length (); // distance from goal

   while (distance > 40.0)
   {
      v_check = v_check + v_direction * 40.0; // move 40 units closer to the goal...

      UTIL_TraceLine (v_check, v_check + Vector (0, 0, -1000), ignore_monsters, pEdict, &tr);

      curr_height = tr.flFraction * 1000.0; // height from ground

      // is the difference between last and current height higher that the max jump height ?
      if ((last_height - curr_height) > 63.0)
         return (FALSE); // if so, assume it's NOT reachable

      last_height = curr_height; // backup current height
      distance = (v_dest - v_check).Length (); // update distance to goal
   }

   return (TRUE); // this point is reachable
}


bool IsAtHumanHeight (Vector v_location)
{
   // this function returns TRUE if the vector specified as the parameter (v_location) is at
   // human height from the floor, that is, not above the absmax of a player standing at this
   // location, FALSE otherwise. One TraceLine is involved.

   TraceResult tr;

   // trace down from v_location to see if it is at human standing height from the ground
   UTIL_TraceLine (v_location, v_location + Vector (0, 0, -72), ignore_monsters, NULL, &tr);

   if (tr.flFraction < 1.0)
      return (TRUE); // ground was found

   return (FALSE); // ground was not found, seems like v_location is in mid-air or outside the map
}


Vector DropAtHumanHeight (Vector v_location)
{
   // this function takes the vector specified as the parameter (v_location) and traces straight
   // down to find a location along the same Z axis that reflects the human standing height. It
   // returns then a raised or lowered version of the original vector. One TraceLine is involved.

   TraceResult tr;

   // trace down from v_location and return a vector at human standing height from the ground
   UTIL_TraceLine (v_location, v_location + Vector (0, 0, -9999), ignore_monsters, NULL, &tr);

   if (tr.flFraction < 1.0)
      return (tr.vecEndPos + Vector (0, 0, 54)); // ground was found, return a lowered vector

   return (g_vecZero); // aargh, ground was not found !
}


Vector DropToFloor (Vector v_location)
{
   // this function takes the vector specified as the parameter (v_location) and traces straight
   // down to find a location along the same Z axis that reflects the ground height. It returns
   // then a "dropped to floor" version of the original vector. One TraceLine is involved.

   TraceResult tr;

   // trace down from v_location and return a vector at ground height
   UTIL_TraceLine (v_location, v_location + Vector (0, 0, -9999), ignore_monsters, NULL, &tr);

   if (tr.flFraction < 1.0)
      return (tr.vecEndPos); // ground was found, return a lowered vector

   return (g_vecZero); // aargh, ground was not found !
}


inline Vector GetGunPosition (edict_t *pEdict)
{
   // this expanded function returns the vector origin of a gun in the hands of a player entity
   // pointed to by pPlayer, assuming that player guns fire at player eye's position. This is
   // very often the case, indeed.

   return (pEdict->v.origin + pEdict->v.view_ofs);
}


inline Vector VecBModelOrigin (edict_t *pEdict)
{
   // this expanded function returns the vector origin of a bounded entity, assuming that any
   // entity that has a bounding box has its center at the center of the bounding box itself.

   return (pEdict->v.absmin + (pEdict->v.size * 0.5));
}


void UTIL_DrawDots (edict_t *pClient, Vector start, Vector end)
{
   // this function draws a dotted line visible from the client side of the player whose player
   // entity is pointed to by pClient, from the vector location start to the vector location end,
   // which is supposed to last 30 seconds.

   if (pClient == NULL)
      return; // reliability check

   // send this client a packet telling him to draw a dotted line using the specified parameters
   MESSAGE_BEGIN (MSG_ONE_UNRELIABLE, GetUserMsgId ("TempEntity"), NULL, pClient);
   WRITE_BYTE (TE_SHOWLINE);
   WRITE_COORD (start.x);
   WRITE_COORD (start.y);
   WRITE_COORD (start.z);
   WRITE_COORD (end.x);
   WRITE_COORD (end.y);
   WRITE_COORD (end.z);
   MESSAGE_END ();

   return; // finished
}


void UTIL_DrawLine (edict_t *pClient, Vector start, Vector end, int life, unsigned char red, unsigned char green, unsigned char blue)
{
   // this function draws a line visible from the client side of the player whose player entity
   // is pointed to by pClient, from the vector location start to the vector location end,
   // which is supposed to last life tenths seconds, and having the color defined by RGB.

   if (pClient == NULL)
      return; // reliability check

   // send this client a packet telling him to draw a beam using the specified parameters
   MESSAGE_BEGIN (MSG_ONE_UNRELIABLE, GetUserMsgId ("TempEntity"), NULL, pClient);
   WRITE_BYTE (TE_BEAMPOINTS);
   WRITE_COORD (start.x);
   WRITE_COORD (start.y);
   WRITE_COORD (start.z);
   WRITE_COORD (end.x);
   WRITE_COORD (end.y);
   WRITE_COORD (end.z);
   WRITE_SHORT (beam_model);
   WRITE_BYTE (1); // framestart
   WRITE_BYTE (10); // framerate
   WRITE_BYTE (life); // life in 0.1's
   WRITE_BYTE (10); // width
   WRITE_BYTE (0); // noise
   WRITE_BYTE (red); // red component of RGB color
   WRITE_BYTE (green); // green component of RGB color
   WRITE_BYTE (blue); // blue component of RGB color
   WRITE_BYTE (255); // brightness
   WRITE_BYTE (255); // speed
   MESSAGE_END ();

   return; // finished
}


void UTIL_DrawBox (edict_t *pClient, Vector bbmin, Vector bbmax, int life, unsigned char red, unsigned char green, unsigned char blue)
{
   // this function draws an axial box (i.e, a box whose faces are parallel to the map's axises)
   // visible from the client side of the player whose player entity is pointed to by pClient,
   // encompassing the bounding box defined by the two vector locations bbmin and bbmax, for a
   // duration of life tenths seconds, and having the color defined by RGB.

   if (pClient == NULL)
      return; // reliability check

   // bottom square
   UTIL_DrawLine (pClient, Vector (bbmin.x, bbmin.y, bbmin.z), Vector (bbmax.x, bbmin.y, bbmin.z), life, red, green, blue);
   UTIL_DrawLine (pClient, Vector (bbmax.x, bbmin.y, bbmin.z), Vector (bbmax.x, bbmax.y, bbmin.z), life, red, green, blue);
   UTIL_DrawLine (pClient, Vector (bbmax.x, bbmax.y, bbmin.z), Vector (bbmin.x, bbmax.y, bbmin.z), life, red, green, blue);
   UTIL_DrawLine (pClient, Vector (bbmin.x, bbmax.y, bbmin.z), Vector (bbmin.x, bbmin.y, bbmin.z), life, red, green, blue);

   // verticals
   UTIL_DrawLine (pClient, Vector (bbmin.x, bbmin.y, bbmin.z), Vector (bbmin.x, bbmin.y, bbmax.z), life, red, green, blue);
   UTIL_DrawLine (pClient, Vector (bbmax.x, bbmin.y, bbmin.z), Vector (bbmax.x, bbmin.y, bbmax.z), life, red, green, blue);
   UTIL_DrawLine (pClient, Vector (bbmax.x, bbmax.y, bbmin.z), Vector (bbmax.x, bbmax.y, bbmax.z), life, red, green, blue);
   UTIL_DrawLine (pClient, Vector (bbmin.x, bbmax.y, bbmin.z), Vector (bbmin.x, bbmax.y, bbmax.z), life, red, green, blue);

   // top square
   UTIL_DrawLine (pClient, Vector (bbmin.x, bbmin.y, bbmax.z), Vector (bbmax.x, bbmin.y, bbmax.z), life, red, green, blue);
   UTIL_DrawLine (pClient, Vector (bbmax.x, bbmin.y, bbmax.z), Vector (bbmax.x, bbmax.y, bbmax.z), life, red, green, blue);
   UTIL_DrawLine (pClient, Vector (bbmax.x, bbmax.y, bbmax.z), Vector (bbmin.x, bbmax.y, bbmax.z), life, red, green, blue);
   UTIL_DrawLine (pClient, Vector (bbmin.x, bbmax.y, bbmax.z), Vector (bbmin.x, bbmin.y, bbmax.z), life, red, green, blue);

   return; // finished
}


void UTIL_DrawWalkface (edict_t *pClient, walkface_t *pFace, int life, unsigned char red, unsigned char green, unsigned char blue)
{
   // this function is a higher level wrapper for UTIL_DrawLine() which purpose is to simplify
   // the computation of the boundaries of the face pointed to by face. It draws lines visible
   // from the client side of the player whose player entity is pointed to by pClient, around the
   // perimeter of the face pointed to by pFace, which is supposed to last life tenths seconds,
   // and having the color defined by RGB.

   int corner_index;
   Vector v_bound1, v_bound2;

   if (!IsValidPlayer (pClient))
      return; // reliability check

   if (pFace == NULL)
      TerminateOnError ("RACC: UTIL_DrawWalkface() called with NULL walkface\n");

   // draw the perimeter around the face
   for (corner_index = 0; corner_index < pFace->corner_count; corner_index++)
   {
      // locate the first vertice of this corner and raise it 2 units up for better visibility
      v_bound1 = pFace->v_corners[corner_index] + Vector (0, 0, 2);

      // locate the second vertice of this corner and raise it 2 units up for better visibility
      if (corner_index < pFace->corner_count - 1)
         v_bound2 = pFace->v_corners[corner_index + 1] + Vector (0, 0, 2); // next corner in the array
      else
         v_bound2 = pFace->v_corners[0] + Vector (0, 0, 2); // loop back to corner zero at last corner

      // draw a line between these 2 points
      UTIL_DrawLine (pClient, v_bound1, v_bound2, life, red, green, blue);
   }

   return; // finished
}


void UTIL_DrawNavlink (edict_t *pClient, navlink_t *pLink, int life)
{
   // this function is a higher level wrapper for UTIL_DrawLine() which purpose is to simplify
   // the drawing of the navigation link pointed to by pLink, represented under the form of a
   // glowing vertical bar, and being 30 units high. This effect is only visible from the client
   // side of the player whose player entity is pointed to by pClient, during life tenths seconds,
   // and having a color defined by the type of reachability this link has.

   unsigned char rgb[3];

   if (!IsValidPlayer (pClient))
      return; // reliability check

   if (pLink == NULL)
      TerminateOnError ("RACC: UTIL_DrawNavlink() called with NULL navlink\n");

   // set the navlink color according to its reachability
   if (pLink->reachability & REACHABILITY_FALLEDGE)
   {
      rgb[0] = 0; // BLUE color for falledge reachability
      rgb[1] = 0;
      rgb[2] = 255;
   }
   else if (pLink->reachability & REACHABILITY_LADDER)
   {
      rgb[0] = 0; // GREEN color for ladder reachability
      rgb[1] = 255;
      rgb[2] = 0;
   }
   else if (pLink->reachability & REACHABILITY_ELEVATOR)
   {
      rgb[0] = 0; // CYAN color for elevator reachability
      rgb[1] = 255;
      rgb[2] = 255;
   }
   else if (pLink->reachability & REACHABILITY_PLATFORM)
   {
      rgb[0] = 255; // RED color for platform reachability
      rgb[1] = 0;
      rgb[2] = 0;
   }
   else if (pLink->reachability & REACHABILITY_CONVEYOR)
   {
      rgb[0] = 255; // PURPLE color for conveyor reachability
      rgb[1] = 0;
      rgb[2] = 255;
   }
   else if (pLink->reachability & REACHABILITY_TRAIN)
   {
      rgb[0] = 255; // YELLOW color for train reachability
      rgb[1] = 255;
      rgb[2] = 0;
   }
   else
   {
      rgb[0] = 255; // WHITE color for normal reachability
      rgb[1] = 255;
      rgb[2] = 255;
   }

   // draw the navlink
   UTIL_DrawLine (pClient, pLink->v_origin + Vector (0, 0, -15), pLink->v_origin + Vector (0, 0, 15), life, rgb[0], rgb[1], rgb[2]);

   return; // finished
}


void UTIL_DrawSector (edict_t *pClient, sector_t *pSector, int life, unsigned char red, unsigned char green, unsigned char blue)
{
   // this function is a higher level wrapper for UTIL_DrawLine() which purpose is to simplify
   // the computation of the boundaries of the map topologic sector to which belongs the spatial
   // location v_origin. It draws a line visible from the client side of the player whose player
   // entity is pointed to by pClient, around the perimeter of the sector to which v_origin
   // belongs, supposed to last life tenths seconds, and having the color defined by RGB.

   int i, j;
   float sector_left, sector_right, sector_top, sector_bottom;

   if (!IsValidPlayer (pClient))
      return; // reliability check

   // we have to determine the position in the topological array where pSector is located

   // loop through array, stop when bucket found (could be faster...)
   for (i = 0; i < map.parallels_count; i++)
      for (j = 0; j < map.meridians_count; j++)
         if (&map.topology[i][j] == pSector)
         {
            // now compute the left, right, top and bottom coordinates indices of the sector
            sector_left = map.v_worldmins.x + (map.v_worldmaxs.x - map.v_worldmins.x) / map.parallels_count * i;
            sector_right = map.v_worldmins.x + (map.v_worldmaxs.x - map.v_worldmins.x) / map.parallels_count * (i + 1);
            sector_bottom = map.v_worldmins.y + (map.v_worldmaxs.y - map.v_worldmins.y) / map.meridians_count * j;
            sector_top = map.v_worldmins.y + (map.v_worldmaxs.y - map.v_worldmins.y) / map.meridians_count * (j + 1);

            // and draw the perimeter around the sector (hopefully the player sees it)
            UTIL_DrawLine (pClient, Vector (sector_left, sector_top, pClient->v.origin.z + 1), Vector (sector_left, sector_bottom, pClient->v.origin.z + 1), life, red, green, blue);
            UTIL_DrawLine (pClient, Vector (sector_left, sector_bottom, pClient->v.origin.z + 1), Vector (sector_right, sector_bottom, pClient->v.origin.z + 1), life, red, green, blue);
            UTIL_DrawLine (pClient, Vector (sector_right, sector_bottom, pClient->v.origin.z + 1), Vector (sector_right, sector_top, pClient->v.origin.z + 1), life, red, green, blue);
            UTIL_DrawLine (pClient, Vector (sector_right, sector_top, pClient->v.origin.z + 1), Vector (sector_left, sector_top, pClient->v.origin.z + 1), life, red, green, blue);

            return; // finished
         }

   // sector not found in topological array ?? WTF ???
   TerminateOnError ("UTIL_DrawSector(): function called for unknown sector!\n");
}


int printf (const char *fmt, ...)
{
   // this function prints a message on the screen. If we are running a dedicated server, the
   // text will be printed on the server console, else if we are running a listen server, it
   // will be printed in game on the listen server client's HUD chat area. Since it's basically
   // a redefinition of the standard C libraries printf() function, it has to be the same type,
   // hence the integer return value.

   va_list argptr;
   static char string[1024];

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   vsprintf (string, fmt, argptr);
   va_end (argptr);

   // are we running a listen server ?
   if (!server.is_dedicated && IsValidPlayer (pListenserverEntity))
   {
      MESSAGE_BEGIN (MSG_ONE, GetUserMsgId ("SayText"), NULL, pListenserverEntity); // then print to HUD
      WRITE_BYTE (ENTINDEX (pListenserverEntity));
      WRITE_STRING (string);
      MESSAGE_END ();
   }
   else
      pfnServerPrint (string); // else print to console

   // if developer mode is on...
   if (server.developer_level > 0)
      LogToFile ("(server HUD): %s", string); // also log this message to the logfile

   return (0); // printf() HAS to return a value
}


int ServerConsole_printf (const char *fmt, ...)
{
   // this function asks the engine to print a message on the server console

   va_list argptr;
   static char string[1024];

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   vsprintf (string, fmt, argptr);
   va_end (argptr);

   (*g_engfuncs.pfnServerPrint) (string); // print to console

   // if developer mode is on...
   if (server.developer_level > 0)
      if (string[0] == '.')
         LogToFile (string); // also log this message to the logfile (not prefixing dots)
      else
         LogToFile ("(server console): %s", string); // also log this message to the logfile

   return (0); // printf() HAS to return a value
}


float WrapAngle (float angle_to_wrap)
{
   // this function adds or substracts 360 enough times needed to the angle_to_wrap angle in
   // order to set it into the range -180/+180 and returns the resulting angle. Letting the
   // engine have a hand on angles that are outside these bounds may cause the game to freeze
   // by screwing up the engine math code.

   static float angle;
   angle = angle_to_wrap; // update this function's static variable with the angle to wrap

   // check for wraparound of angle
   if (angle >= 180)
      angle -= 360 * ((int) (angle / 360) + 1);
   if (angle < -180)
      angle += 360 * ((int) (angle / 360) + 1);

   return (angle);
}


float WrapAngle360 (float angle_to_wrap)
{
   // this function adds or substracts 360 enough times needed to the angle_to_wrap angle in
   // order to set it into the range +0/+360 and returns the resulting angle Letting the
   // engine have a hand on angles that are outside these bounds may cause the game to freeze
   // by screwing up the engine math code.

   static float angle;
   angle = angle_to_wrap; // update this function's static variable with the angle to wrap

   // check for wraparound of angle
   if (angle >= 360)
      angle -= 360 * (int) (angle / 360);
   if (angle < 0)
      angle += 360 * ((int) (angle / 360) + 1);

   return (angle);
}


Vector WrapAngles (Vector &angles_to_wrap)
{
   // this function adds or substracts 360 enough times needed to every of the three components
   // of the axial angles structure angles_to_wrap in order to set them into the range -180/+180
   // and returns the resulting axial angles structure.

   static Vector angles;
   angles = angles_to_wrap; // update this function's static variable with the angles to wrap

   // check for wraparound of angles
   if (angles.x >= 180)
      angles.x -= 360 * ((int) (angles.x / 360) + 1);
   if (angles.x < -180)
      angles.x += 360 * ((int) (angles.x / 360) + 1);
   if (angles.y >= 180)
      angles.y -= 360 * ((int) (angles.y / 360) + 1);
   if (angles.y < -180)
      angles.y += 360 * ((int) (angles.y / 360) + 1);
   if (angles.z >= 180)
      angles.z -= 360 * ((int) (angles.z / 360) + 1);
   if (angles.z < -180)
      angles.z += 360 * ((int) (angles.z / 360) + 1);

   return (angles);
}


Vector WrapAngles360 (Vector &angles_to_wrap)
{
   // this function adds or substracts 360 enough times needed to every of the three components
   // of the axial angles structure angles_to_wrap in order to set them into the range +0/+360
   // and returns the resulting axial angles structure.

   static Vector angles;
   angles = angles_to_wrap; // update this function's static variable with the angles to wrap

   // check for wraparound of angles
   if (angles.x >= 360)
      angles.x -= 360 * (int) (angles.x / 360);
   if (angles.x < 0)
      angles.x += 360 * ((int) (angles.x / 360) + 1);
   if (angles.y >= 360)
      angles.y -= 360 * (int) (angles.y / 360);
   if (angles.y < 0)
      angles.y += 360 * ((int) (angles.y / 360) + 1);
   if (angles.z >= 360)
      angles.z -= 360 * (int) (angles.z / 360);
   if (angles.z < 0)
      angles.z += 360 * ((int) (angles.z / 360) + 1);

   return (angles);
}


float AngleOfVectors (Vector v1, Vector v2)
{
   // this function returns the angle in degrees between the v1 and v2 vectors, regardless of
   // the axial planes (ie, considering the plane formed by the v1 and v2 vectors themselves)

   static float v1norm_dotprod_v2norm;
   static Vector normalized_v1, normalized_v2;

   if ((v1 == g_vecZero) || (v2 == g_vecZero))
      return (0); // reliability check (avoid zero divide)

   // normalize v1 and v2 (tip from botman)
   normalized_v1 = v1.Normalize ();
   normalized_v2 = v2.Normalize ();

   // reminder: dot product = (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z)
   v1norm_dotprod_v2norm = normalized_v1.x * normalized_v2.x
                           + normalized_v1.y * normalized_v2.y
                           + normalized_v1.z * normalized_v2.z;

   // how on Earth come that a dotproduct of normalized vectors can outbound [-1, 1] ???
   // A couple 'forestry worker' casts to double seem to make the problem occur less often, but
   // still, we have to check the value to ensure it will be in range...

   if (v1norm_dotprod_v2norm < -1)
      return (180); // reliability check (avoid acos range error)
   else if (v1norm_dotprod_v2norm > 1)
      return (0); // reliability check (avoid acos range error)

   return (WrapAngle (acos (v1norm_dotprod_v2norm) * 180 / M_PI));
}


Vector BotAngleToLocation (bot_t *pBot, Vector v_dest)
{
   Vector v_angle;

   if (!IsValidPlayer (pBot->pEdict))
      return (0); // reliability check

   // return the absolute value of angle to destination
   v_angle = UTIL_VecToAngles (v_dest - GetGunPosition (pBot->pEdict))
             - WrapAngles (pBot->pEdict->v.v_angle);

   return (WrapAngles (v_angle));
}


int UTIL_GetNearestOrderableBotIndex (edict_t *pAskingEntity)
{
   float distance[RACC_MAX_CLIENTS], nearest_distance = 9999;
   int i, index = -1, bot_team, player_team = GetTeam (pAskingEntity);

   // cycle all bots to find the nearest one which is not currently "used" by other player
   for (i = 0; i < RACC_MAX_CLIENTS; i++)
   {
      if (bots[i].pEdict == NULL)
         continue; // skip unregistered bot slots
            
      bot_team = GetTeam (bots[i].pEdict);

      // is this bot dead OR inactive OR asking entity itself OR used by another OR hostile OR not visible ?
      if (!players[i].is_alive || !bots[i].is_active || (bots[i].pEdict == pAskingEntity)
         || ((bots[i].pBotUser != NULL) && (bots[i].pBotUser != pAskingEntity))
         || !((bot_team == player_team) || (team_allies[bot_team] & (1 << player_team)))
         || (BotCanSeeOfEntity (&bots[i], pAskingEntity) == g_vecZero))
         continue; // if so, skip to the next bot

      // how far away is the bot?
      distance[i] = (pAskingEntity->v.origin - bots[i].pEdict->v.origin).Length ();
            
      if (distance[i] < nearest_distance)
      {
         nearest_distance = distance[i]; // update bot distances
         index = i; // keep track of the closest bot
      }
   }

   return (index); // return index of the nearest orderable bot
}


int UTIL_GetNearestUsableBotIndex (edict_t *pAskingEntity)
{
   float distance[RACC_MAX_CLIENTS], nearest_distance = 9999;
   int i, index = -1, bot_team, player_team = GetTeam (pAskingEntity);

   // cycle all bots to find the nearest one which is not currently "used"
   for (i = 0; i < RACC_MAX_CLIENTS; i++)
   {
      if (bots[i].pEdict == NULL)
         continue; // skip unregistered bot slots
            
      bot_team = GetTeam (bots[i].pEdict);

      // is this bot dead OR inactive OR asking entity itself OR used OR hostile OR not visible ?
      if (!players[i].is_alive || !bots[i].is_active
         || (bots[i].pEdict == pAskingEntity) || (bots[i].pBotUser != NULL)
         || !((bot_team == player_team) || (team_allies[bot_team] & (1 << player_team)))
         || (BotCanSeeOfEntity (&bots[i], pAskingEntity) == g_vecZero))
         continue; // if so, skip to the next bot

      // how far away is the bot?
      distance[i] = (pAskingEntity->v.origin - bots[i].pEdict->v.origin).Length ();
            
      if (distance[i] < nearest_distance)
      {
         nearest_distance = distance[i]; // update bot distances
         index = i; // keep track of the closest bot
      }
   }

   return (index); // return index of the nearest usable bot
}


int UTIL_GetNearestUsedBotIndex (edict_t *pAskingEntity)
{
   float distance[RACC_MAX_CLIENTS], nearest_distance = 9999;
   int i, index = -1, bot_team, player_team = GetTeam (pAskingEntity);

   // cycle all bots to find the nearest one which is not currently "used" by other player
   for (i = 0; i < RACC_MAX_CLIENTS; i++)
   {
      if (bots[i].pEdict == NULL)
         continue; // skip unregistered bot slots
            
      bot_team = GetTeam (bots[i].pEdict);

      // is this bot dead OR inactive OR asking entity itself OR not used by asking entity ?
      if (!players[i].is_alive || !bots[i].is_active
         || (bots[i].pEdict == pAskingEntity) || (bots[i].pBotUser != pAskingEntity))
         continue; // if so, skip to the next bot

      // how far away is the bot?
      distance[i] = (pAskingEntity->v.origin - bots[i].pEdict->v.origin).Length ();
            
      if (distance[i] < nearest_distance)
      {
         nearest_distance = distance[i]; // update bot distances
         index = i; // keep track of the closest bot
      }
   }

   return (index); // return index of the nearest used bot
}


void TerminateOnError (const char *fmt, ...)
{
   // this function terminates the game because of an error and prints the message string pointed
   // to by fmt both in the server console and in a messagebox.

   va_list argptr;
   char string[1024];

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   vsprintf (string, fmt, argptr);
   va_end (argptr);

   ServerConsole_printf (string); // print to console
   MessageBox (0, string, "RACC - Error", NULL); // print to message box

   // if developer mode is on...
   if (server.developer_level > 0)
      LogToFile ("FATAL ERROR: %s", string); // also log this error to the logfile

   // free everything that's freeable
   FreeAllTheStuff ();

   // once everything is freed, just exit
   ALERT (at_error, string); // tell the engine to exit with an error code
   exit (1);
}


void MakeTeams (void)
{
   char teamlist[512];
   char *team;
   int i = 0;

   // reset the team names array and team count first
   memset (&team_names, 0, sizeof (team_names));
   num_teams = 0;

   // retrieve the team list CVAR
   strcpy (teamlist, CVAR_GET_STRING ("mp_teamlist"));
   team = strtok (teamlist, ";"); // initialize the token search

   // store each name of the team list in the array
   while (team != NULL && *team)
   {
      // check that team isn't defined twice
      for (i = 0; i < num_teams; i++)
         if (strcmp (team, team_names[i]) == 0)
            break;

      // if sure it isn't
      if (i == num_teams)
      {
         strcpy (team_names[num_teams], team); // add to teams array
         num_teams++; // increment team number
      }

      team = strtok (NULL, ";"); // process the next one
   }

   return; // finished, mp_teamlist CVAR processed
}


void InitGameLocale (void)
{
   FILE *fp;
   char line_buffer[256];

   // get the game language from the sierra.inf file
   strcpy (server.language, "english"); // defaults to english first
   fp = fopen ("sierra.inf", "r"); // opens file readonly

   if (fp == NULL)
      return; // sierra.inf file missing, can't determine game language

   // reads the file line per line
   while (fgets (line_buffer, 255, fp) != NULL)
   {
      if ((line_buffer[0] == '\n') || ((line_buffer[0] == '/') && (line_buffer[1] == '/')))
         continue; // ignore line if void or commented

      // if that line tells the standard American/English language definition
      if (strncmp ("ShortTitle=HALFLIFE", line_buffer, 19) == 0)
      {
         strcpy (server.language, "english");
         fclose (fp);
         return;
      }

      // else if that line tells the French language definition
      else if (strncmp ("ShortTitle=HLIFEFR", line_buffer, 18) == 0)
      {
         strcpy (server.language, "french");
         fclose (fp);
         return;
      }

      // else if that line tells the Deutsch language definition
      else if (strncmp ("ShortTitle=HLIFEDE", line_buffer, 18) == 0)
      {
         strcpy (server.language, "german");
         fclose (fp);
         return;
      }

      // else if that line tells the Italian language definition
      else if (strncmp ("ShortTitle=HLIFEIT", line_buffer, 18) == 0)
      {
         strcpy (server.language, "italian");
         fclose (fp);
         return;
      }

      // else if that line tells the Spanish language definition
      else if (strncmp ("ShortTitle=HLIFEES", line_buffer, 18) == 0)
      {
         strcpy (server.language, "spanish");
         fclose (fp);
         return;
      }
   }
}


void InitLogFile (void)
{
   // this function reinitializes the log file, erasing any previous content. It is meant to be
   // called when the server boots up.

   DebugLevel.fp = fopen (RACC_LOGFILE_PATH, "w"); // open the log file in ASCII write mode, discard content

   // bomb out on error if unable to open the log file
   if (DebugLevel.fp == NULL)
      TerminateOnError ("InitLogFile(): unable to open log file (\"" RACC_LOGFILE_PATH "\")\n");

   fprintf (DebugLevel.fp, "RACC log file started\n\n"); // dump an init string

   return; // and return
}


void LogToFile (const char *fmt, ...)
{
   // this function logs a message to the message log file somewhere in the racc directory

   va_list argptr;
   static char string[1024];

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   vsprintf (string, fmt, argptr);
   va_end (argptr);

   // is the debug log file NOT open yet ?
   if (DebugLevel.fp == NULL)
      DebugLevel.fp = fopen (RACC_LOGFILE_PATH, "a"); // open the log file in ASCII append mode, keep content

   // bomb out on error if unable to open the log file
   if (DebugLevel.fp == NULL)
      TerminateOnError ("LogToFile(): unable to open log file (\"" RACC_LOGFILE_PATH "\")\n");

   fprintf (DebugLevel.fp, string); // dump the string into the file
   return; // and return
}


void InitPlayerBones (void)
{
   // this function is in charge of opening the playerbones.cfg file in the knowledge/MOD
   // directory, and filling the player bones indices database accordingly. Bones are part of the
   // animation model that can rotate, so that the engine can render players that actually look
   // like they run, walk, wave their arms and so on. Each bone is a reference point to which
   // vertices of the model's polygons are linked, so that the bone acts as the "rotation center"
   // for these faces. Like if you said, that all your right foot has the ability to do
   // *intrinsically*, is to rotate around your right thigh, which nothing but the truth. For the
   // engine and the MOD code to identify the different parts of the body, each bone is indexed by
   // an unique identification number. We use them to make the bots aim at a specific part of the
   // enemy's body (bear in mind that a bot doesn't know natively what an arm, a leg or an elbow
   // is, all a bot might see is called "waypoints" and "polygons"), that's why we need to know
   // these numbers to call the GetBonePosition() engine function later on. To do so we open and
   // read the contents of the playerbones.cfg file. Such a task should be performed only once,
   // preferably at GameDLLInit(), since player bones aren't likely to change between each map.

   FILE *fp;
   char filename[256];
   char line_buffer[256];
   char bone_name[256];
   int length;

   // first reset the player bone numbers
   memset (&playerbones, 0, sizeof (playerbones));

   // open the "playerbones.cfg" file in the knowledge directory, in the MOD's folder
   sprintf (filename, "racc/knowledge/%s/playerbones.cfg", server.mod_name);
   fp = fopen (filename, "r");
   if (fp == NULL)
   {
      ServerConsole_printf ("RACC: Unable to figure out player morphology (playerbones.cfg file not found)\n");
      return; // if the file doesn't exist, then the bot won't know any bone at all
   }

   // for each line in the file...
   while (fgets (line_buffer, 256, fp) != NULL)
   {
      length = strlen (line_buffer); // get length of line
      if ((length > 0) && (line_buffer[length - 1] == '\n'))
         length--; // remove any final '\n'
      line_buffer[length] = 0; // terminate the string

      if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#'))
         continue; // ignore line if void or commented

      // this line looks like a valid data line, figure out which bone it is talking about
      sprintf (bone_name, GetField (line_buffer, 0));

      // given the bone specified, fill in the corresponding bone number from the file
      if (strcmp (bone_name, "pelvis") == 0)
         playerbones.pelvis = atoi (GetField (line_buffer, 1)); // it's the pelvis
      else if (strcmp (bone_name, "spine") == 0)
         playerbones.spine = atoi (GetField (line_buffer, 1)); // it's the bottom of the spine
      else if (strcmp (bone_name, "spine1") == 0)
         playerbones.spine1 = atoi (GetField (line_buffer, 1)); // it's the 1st quarter of spine
      else if (strcmp (bone_name, "spine2") == 0)
         playerbones.spine2 = atoi (GetField (line_buffer, 1)); // it's the half-high spine
      else if (strcmp (bone_name, "spine3") == 0)
         playerbones.spine3 = atoi (GetField (line_buffer, 1)); // it's the 3rd quarter of spine
      else if (strcmp (bone_name, "neck") == 0)
         playerbones.neck = atoi (GetField (line_buffer, 1)); // it's the neck
      else if (strcmp (bone_name, "head") == 0)
         playerbones.head = atoi (GetField (line_buffer, 1)); // it's the head
      else if (strcmp (bone_name, "left_clavicle") == 0)
         playerbones.left_clavicle = atoi (GetField (line_buffer, 1)); // it's the left clavicle
      else if (strcmp (bone_name, "left_upperarm") == 0)
         playerbones.left_upperarm = atoi (GetField (line_buffer, 1)); // it's the left upperarm
      else if (strcmp (bone_name, "left_forearm") == 0)
         playerbones.left_forearm = atoi (GetField (line_buffer, 1)); // it's the left forearm
      else if (strcmp (bone_name, "left_hand") == 0)
         playerbones.left_hand = atoi (GetField (line_buffer, 1)); // it's the left hand
      else if (strcmp (bone_name, "left_finger0") == 0)
         playerbones.left_finger0 = atoi (GetField (line_buffer, 1)); // it's in the left thumb
      else if (strcmp (bone_name, "left_finger01") == 0)
         playerbones.left_finger01 = atoi (GetField (line_buffer, 1)); // it's the thumb extremity
      else if (strcmp (bone_name, "left_finger1") == 0)
         playerbones.left_finger1 = atoi (GetField (line_buffer, 1)); // it's in the left fingers
      else if (strcmp (bone_name, "left_finger11") == 0)
         playerbones.left_finger11 = atoi (GetField (line_buffer, 1)); // it's the fingers extremity
      else if (strcmp (bone_name, "left_thigh") == 0)
         playerbones.left_thigh = atoi (GetField (line_buffer, 1)); // it's the left thigh
      else if (strcmp (bone_name, "left_calf") == 0)
         playerbones.left_calf = atoi (GetField (line_buffer, 1)); // it's the left calf
      else if (strcmp (bone_name, "left_foot") == 0)
         playerbones.left_foot = atoi (GetField (line_buffer, 1)); // it's the foot extremity
      else if (strcmp (bone_name, "right_clavicle") == 0)
         playerbones.right_clavicle = atoi (GetField (line_buffer, 1)); // it's the right clavicle
      else if (strcmp (bone_name, "right_upperarm") == 0)
         playerbones.right_upperarm = atoi (GetField (line_buffer, 1)); // it's the right upperarm
      else if (strcmp (bone_name, "right_forearm") == 0)
         playerbones.right_forearm = atoi (GetField (line_buffer, 1)); // it's the right forearm
      else if (strcmp (bone_name, "right_hand") == 0)
         playerbones.right_hand = atoi (GetField (line_buffer, 1)); // it's the right hand
      else if (strcmp (bone_name, "right_finger0") == 0)
         playerbones.right_finger0 = atoi (GetField (line_buffer, 1)); // it's in the right thumb
      else if (strcmp (bone_name, "right_finger01") == 0)
         playerbones.right_finger01 = atoi (GetField (line_buffer, 1)); // it's the thumb extremity
      else if (strcmp (bone_name, "right_finger1") == 0)
         playerbones.right_finger1 = atoi (GetField (line_buffer, 1)); // it's in the right fingers
      else if (strcmp (bone_name, "right_finger11") == 0)
         playerbones.right_finger11 = atoi (GetField (line_buffer, 1)); // it's the fingers extremity
      else if (strcmp (bone_name, "right_thigh") == 0)
         playerbones.right_thigh = atoi (GetField (line_buffer, 1)); // it's the right thigh
      else if (strcmp (bone_name, "right_calf") == 0)
         playerbones.right_calf = atoi (GetField (line_buffer, 1)); // it's the right calf
      else if (strcmp (bone_name, "right_foot") == 0)
         playerbones.right_foot = atoi (GetField (line_buffer, 1)); // it's the foot extremity
   }

   fclose (fp); // finished parsing the file, close it

   // finished figuring out player morphology, print out a nice notification message
   ServerConsole_printf ("RACC: Player morphology learned from file\n");
   return;
}


void InitWeapons (void)
{
   // this function is in charge of opening the weaponsounds.cfg file in the knowledge/MOD
   // directory, and filling the gunshot sounds database accordingly. Each type of weapon gets
   // attributed 4 different sounds, depending on the attack used on them, and on the special
   // mode the weapon is in (e.g, silenced or not). Such a task should be performed once and only
   // once, preferably at GameDLLInit(), since gunshot sounds aren't likely to change between
   // each map.

   FILE *fp;
   char filename[256];
   char line_buffer[256];
   char key[256], value[256];
   int length, index;
   bool section_open;

   // start off by resetting completely the weapons database
   memset (weapons, 0, sizeof (weapons));
   weapon_count = 0; //set the weapons count to zero

   // open the "weaponspecs.cfg" file in the knowledge directory, in the MOD's folder
   sprintf (filename, "racc/knowledge/%s/weaponspecs.cfg", server.mod_name);
   fp = fopen (filename, "r");
   if (fp == NULL)
   {
      ServerConsole_printf ("RACC: ALERT: Unable to build weapons database (weaponspecs.cfg file not found)\n");
      return; // if the file doesn't exist, then the bot won't know any gunshot sound at all
   }

   section_open = FALSE; // no section has been open yet in the config file

   // for each line in the file...
   while ((fgets (line_buffer, 256, fp) != NULL) && (weapon_count < MAX_WEAPONS))
   {
      length = strlen (line_buffer); // get length of line
      if ((length > 0) && (line_buffer[length - 1] == '\n'))
         length--; // remove any final '\n'
      line_buffer[length] = 0; // terminate the string

      if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#'))
         continue; // ignore line if void or commented

      // is it the start of a new section ?
      if (line_buffer[0] == '[')
      {
         index = 1; // let's check for a closing brace to validate this section
         while ((index < length) && (line_buffer[index] != ']'))
            index++; // reach end of field and see if the section is valid
         if (index == length)
            continue; // if end of line reached but no closing bracket found, section is invalid

         // we found the start of a valid section, so now if another section was already open
         // and being read, we need to close it and advance to the next slot

         if (section_open)
            weapon_count++; // skip to next slot, we know now one weapon more

         line_buffer[index] = 0; // terminate the string at the closing brace

         section_open = TRUE; // declare that a section is being read
         strcpy (weapons[weapon_count].classname, &line_buffer[1]); // read weapon name from section
      }

      if (!section_open)
         continue; // if no section is open yet, skip that line

      // this line looks like a valid data line, read what sort of key it is and store the data
      strcpy (key, GetConfigKey (line_buffer));
      strcpy (value, GetConfigValue (line_buffer));

      // generic data
      if (strcmp (key, "model") == 0)
         strcpy (weapons[weapon_count].model, value);
      else if (strcmp (key, "id") == 0)
         weapons[weapon_count].id = atoi (value);
      else if (strcmp (key, "slot") == 0)
         weapons[weapon_count].slot = atoi (value);
      else if (strcmp (key, "position") == 0)
         weapons[weapon_count].position = atoi (value);
      else if (strcmp (key, "flags") == 0)
         weapons[weapon_count].flags = atoi (value);

      // primary rail
      else if (strcmp (key, "primary.use_percent") == 0)
         weapons[weapon_count].primary.use_percent = atoi (value);
      else if (strcmp (key, "primary.min_range") == 0)
         weapons[weapon_count].primary.min_range = atof (value);
      else if (strcmp (key, "primary.max_range") == 0)
         weapons[weapon_count].primary.max_range = atof (value);
      else if (strcmp (key, "primary.type_of_ammo") == 0)
         weapons[weapon_count].primary.type_of_ammo = atoi (value);
      else if (strcmp (key, "primary.min_ammo") == 0)
         weapons[weapon_count].primary.min_ammo = atoi (value);
      else if (strcmp (key, "primary.max_ammo") == 0)
         weapons[weapon_count].primary.max_ammo = atoi (value);
      else if (strcmp (key, "primary.can_use_underwater") == 0)
         weapons[weapon_count].primary.can_use_underwater = (atoi (value) > 0);
      else if (strcmp (key, "primary.should_hold") == 0)
         weapons[weapon_count].primary.should_hold = (atoi (value) > 0);
      else if (strcmp (key, "primary.should_charge") == 0)
         weapons[weapon_count].primary.should_charge = (atoi (value) > 0);
      else if (strcmp (key, "primary.charge_delay") == 0)
         weapons[weapon_count].primary.charge_delay = atof (value);
      else if (strcmp (key, "primary.sound1") == 0)
         strcpy (weapons[weapon_count].primary.sound1, value);
      else if (strcmp (key, "primary.sound2") == 0)
         strcpy (weapons[weapon_count].primary.sound2, value);
      else if (strcmp (key, "primary.min_delay") == 0)
      {
         weapons[weapon_count].primary.min_delay[0] = atof (GetField (value, 0));
         weapons[weapon_count].primary.min_delay[1] = atof (GetField (value, 1));
         weapons[weapon_count].primary.min_delay[2] = atof (GetField (value, 2));
         weapons[weapon_count].primary.min_delay[3] = atof (GetField (value, 3));
         weapons[weapon_count].primary.min_delay[4] = atof (GetField (value, 4));
      }
      else if (strcmp (key, "primary.max_delay") == 0)
      {
         weapons[weapon_count].primary.max_delay[0] = atof (GetField (value, 0));
         weapons[weapon_count].primary.max_delay[1] = atof (GetField (value, 1));
         weapons[weapon_count].primary.max_delay[2] = atof (GetField (value, 2));
         weapons[weapon_count].primary.max_delay[3] = atof (GetField (value, 3));
         weapons[weapon_count].primary.max_delay[4] = atof (GetField (value, 4));
      }

      // secondary rail
      else if (strcmp (key, "secondary.use_percent") == 0)
         weapons[weapon_count].secondary.use_percent = atoi (value);
      else if (strcmp (key, "secondary.min_range") == 0)
         weapons[weapon_count].secondary.min_range = atof (value);
      else if (strcmp (key, "secondary.max_range") == 0)
         weapons[weapon_count].secondary.max_range = atof (value);
      else if (strcmp (key, "secondary.type_of_ammo") == 0)
         weapons[weapon_count].secondary.type_of_ammo = atoi (value);
      else if (strcmp (key, "secondary.min_ammo") == 0)
         weapons[weapon_count].secondary.min_ammo = atoi (value);
      else if (strcmp (key, "secondary.max_ammo") == 0)
         weapons[weapon_count].secondary.max_ammo = atoi (value);
      else if (strcmp (key, "secondary.can_use_underwater") == 0)
         weapons[weapon_count].secondary.can_use_underwater = (atoi (value) > 0);
      else if (strcmp (key, "secondary.should_hold") == 0)
         weapons[weapon_count].secondary.should_hold = (atoi (value) > 0);
      else if (strcmp (key, "secondary.should_charge") == 0)
         weapons[weapon_count].secondary.should_charge = (atoi (value) > 0);
      else if (strcmp (key, "secondary.charge_delay") == 0)
         weapons[weapon_count].secondary.charge_delay = atof (value);
      else if (strcmp (key, "secondary.sound1") == 0)
         strcpy (weapons[weapon_count].secondary.sound1, value);
      else if (strcmp (key, "secondary.sound2") == 0)
         strcpy (weapons[weapon_count].secondary.sound2, value);
      else if (strcmp (key, "secondary.min_delay") == 0)
      {
         weapons[weapon_count].secondary.min_delay[0] = atof (GetField (value, 0));
         weapons[weapon_count].secondary.min_delay[1] = atof (GetField (value, 1));
         weapons[weapon_count].secondary.min_delay[2] = atof (GetField (value, 2));
         weapons[weapon_count].secondary.min_delay[3] = atof (GetField (value, 3));
         weapons[weapon_count].secondary.min_delay[4] = atof (GetField (value, 4));
      }
      else if (strcmp (key, "secondary.max_delay") == 0)
      {
         weapons[weapon_count].secondary.max_delay[0] = atof (GetField (value, 0));
         weapons[weapon_count].secondary.max_delay[1] = atof (GetField (value, 1));
         weapons[weapon_count].secondary.max_delay[2] = atof (GetField (value, 2));
         weapons[weapon_count].secondary.max_delay[3] = atof (GetField (value, 3));
         weapons[weapon_count].secondary.max_delay[4] = atof (GetField (value, 4));
      }
   }

   // end of file has been reached

   // if a section was being read...
   if (section_open)
   {
      section_open = FALSE; // close it, no section open anymore
      weapon_count++; // we have recorded the last section, that makes one weapon more
   }

   fclose (fp); // finished parsing the file, close it

   // print out how many weapons we read
   ServerConsole_printf ("RACC: Bot weapon specs database identified %d weapons\n", weapon_count);
   return;
}


void LoadBotProfiles (void)
{
   // this function is called each time a new server starts, typically in ServerActivate(). It
   // fills one by one the bot personality slots with the info read from the bot chat, talk and
   // knowledge directories and the "profiles.cfg" file that holds the bots names, skin, logo
   // and skill of the bots.

   // TODO: logo doesn't work
   // TODO: make search through files OS-independent

   FILE *fp;
   HANDLE hFile;
   WIN32_FIND_DATA pFindFileData;
   HRESULT search_result = TRUE;
   char path[256];
   char line_buffer[256];
   char current_language[32];
   int length, index;
   char field[256];

   // do some cleanup first
   memset (bots, 0, sizeof (bots)); // reset the bots array before they start to connect
   memset (profiles, 0, sizeof (profiles));
   memset (bot_affirmative, 0, sizeof (bot_affirmative));
   memset (bot_negative, 0, sizeof (bot_negative));
   memset (bot_hello, 0, sizeof (bot_hello));
   memset (bot_laugh, 0, sizeof (bot_laugh));
   memset (bot_whine, 0, sizeof (bot_whine));
   memset (bot_idle, 0, sizeof (bot_idle));
   memset (bot_follow, 0, sizeof (bot_follow));
   memset (bot_stop, 0, sizeof (bot_stop));
   memset (bot_stay, 0, sizeof (bot_stay));
   memset (bot_help, 0, sizeof (bot_help));
   memset (bot_cant, 0, sizeof (bot_cant));
   memset (bot_bye, 0, sizeof (bot_bye));
   profile_count = 0;

   // read the bots names from the file
   fp = fopen ("racc/profiles.cfg", "r"); // opens file readonly
   if (fp != NULL)
   {
      while ((profile_count < RACC_MAX_PROFILES) && (fgets (line_buffer, 256, fp) != NULL)) // reads line per line
      {
         length = strlen (line_buffer); // get length of line
         if (length > 0)
            if (line_buffer[length - 1] == '\n')
               length--; // remove any final '\n'
         line_buffer[length] = 0; // terminate the string

         if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#'))
            continue; // ignore line if void or commented

         // name
         sprintf (field, GetField (line_buffer, 0)); // read the bot's name
         field[32] = 0; // truncate it in case it exceeds the maximal length
         sprintf (profiles[profile_count].name, field); // and store it in the array

         // skin
         sprintf (field, GetField (line_buffer, 1)); // read the bot's skin
         field[32] = 0; // truncate it in case it exceeds the maximal length
         sprintf (profiles[profile_count].skin, field); // and store it in the array

         // logo
         sprintf (field, GetField (line_buffer, 2)); // read the bot's logo
         field[32] = 0; // truncate it in case it exceeds the maximal length
         sprintf (profiles[profile_count].logo, field); // and store it in the array

         // nationality
         sprintf (field, GetField (line_buffer, 3)); // read the bot nationality
         if (strcmp (field, "french") == 0)
            profiles[profile_count].nationality = NATIONALITY_FRENCH; // add value to nationalities array
         else if (strcmp (field, "german") == 0)
            profiles[profile_count].nationality = NATIONALITY_GERMAN; // add value to nationalities array
         else if (strcmp (field, "italian") == 0)
            profiles[profile_count].nationality = NATIONALITY_ITALIAN; // add value to nationalities array
         else if (strcmp (field, "spanish") == 0)
            profiles[profile_count].nationality = NATIONALITY_SPANISH; // add value to nationalities array
         else
            profiles[profile_count].nationality = NATIONALITY_ENGLISH; // defaults to english if unknown

         // skill
         sprintf (field, GetField (line_buffer, 4)); // read the bot's skill
         profiles[profile_count].skill = atoi (field); // store value in an integer

         // force skill in bounds
         if (profiles[profile_count].skill < 1)
            profiles[profile_count].skill = 1;
         else if (profiles[profile_count].skill > 5)
            profiles[profile_count].skill = 5;

         // use auto assign for team and class
         profiles[profile_count].team = -1;
         profiles[profile_count].subclass = -1;

         profile_count++; // we have one more bot in the array
      }

      fclose (fp); // all profiles loaded, close the profiles.cfg file
      ServerConsole_printf ("RACC: %d profiles loaded\n", profile_count);
   }

   // else error opening profiles.cfg file
   else
      ServerConsole_printf ("RACC: unable to find profiles.cfg, no profiles loaded!\n");

   // load bot text and audio chat for ALL nationalities...
   for (index = 0; index < 5; index++)
   {
      if (index == NATIONALITY_ENGLISH)
         strcpy (current_language, "english");
      else if (index == NATIONALITY_FRENCH)
         strcpy (current_language, "french");
      else if (index == NATIONALITY_GERMAN)
         strcpy (current_language, "german");
      else if (index == NATIONALITY_ITALIAN)
         strcpy (current_language, "italian");
      else if (index == NATIONALITY_SPANISH)
         strcpy (current_language, "spanish");

      // Build affirmative messages array
      affirmative_count[index] = 0; // first reset the count
      sprintf (path, "racc/chat/%s/affirmative.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((affirmative_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_affirmative[index][affirmative_count[index]], line_buffer); // we have a valid line
            affirmative_count[index]++;
         }
         fclose (fp);
      }

      // Build bye messages array
      bye_count[index] = 0; // first reset the count
      sprintf (path, "racc/chat/%s/bye.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((bye_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_bye[index][bye_count[index]], line_buffer); // we have a valid line
            bye_count[index]++;
         }
         fclose (fp);
      }

      // Build cant messages array
      cant_count[index] = 0; // first reset the count
      sprintf (path, "racc/chat/%s/cantfollow.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((cant_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_cant[index][cant_count[index]], line_buffer); // we have a valid line
            cant_count[index]++;
         }
         fclose (fp);
      }

      // Build follow messages array
      follow_count[index] = 0; // first reset the count
      sprintf (path, "racc/chat/%s/follow.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((follow_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_follow[index][follow_count[index]], line_buffer); // we have a valid line
            follow_count[index]++;
         }
         fclose (fp);
      }

      // Build hello messages array
      hello_count[index] = 0; // first reset the count
      sprintf (path, "racc/chat/%s/hello.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((hello_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_hello[index][hello_count[index]], line_buffer); // we have a valid line
            hello_count[index]++;
         }
         fclose (fp);
      }

      // Build help messages array
      help_count[index] = 0; // first reset the count
      sprintf (path, "racc/chat/%s/help.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((help_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_help[index][help_count[index]], line_buffer); // we have a valid line
            help_count[index]++;
         }
         fclose (fp);
      }

      // Build idle messages array
      idle_count[index] = 0; // first reset the count
      sprintf (path, "racc/chat/%s/idle.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((idle_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_idle[index][idle_count[index]], line_buffer); // we have a valid line
            idle_count[index]++;
         }
         fclose (fp);
      }

      // Build laugh messages array
      laugh_count[index] = 0; // first reset the count
      sprintf (path, "racc/chat/%s/laugh.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((laugh_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_laugh[index][laugh_count[index]], line_buffer); // we have a valid line
            laugh_count[index]++;
         }
         fclose (fp);
      }

      // Build negative messages array
      negative_count[index] = 0; // first reset the count
      sprintf (path, "racc/chat/%s/negative.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((negative_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_negative[index][negative_count[index]], line_buffer); // we have a valid line
            negative_count[index]++;
         }
         fclose (fp);
      }

      // Build stay messages array
      stay_count[index] = 0; // first reset the count
      sprintf (path, "racc/chat/%s/stay.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((stay_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_stay[index][stay_count[index]], line_buffer); // we have a valid line
            stay_count[index]++;
         }
         fclose (fp);
      }

      // Build stop messages array
      stop_count[index] = 0; // first reset the count
      sprintf (path, "racc/chat/%s/stop.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((stop_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_stop[index][stop_count[index]], line_buffer); // we have a valid line
            stop_count[index]++;
         }
         fclose (fp);
      }

      // Build whine messages array
      whine_count[index] = 0; // first reset the count
      sprintf (path, "racc/chat/%s/whine.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((whine_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_whine[index][whine_count[index]], line_buffer); // we have a valid line
            whine_count[index]++;
         }
         fclose (fp);
      }

      // look for any affirmative voice samples
      audio_affirmative_count[index] = 0; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/sound/racc/%s/affirmative*.wav", server.mod_name, current_language);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         audio_affirmative_count[index]++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any alert voice samples
      audio_alert_count[index] = 0; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/sound/racc/%s/alert*.wav", server.mod_name, current_language);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         audio_alert_count[index]++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any attacking voice samples
      audio_attacking_count[index] = 0; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/sound/racc/%s/attacking*.wav", server.mod_name, current_language);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         audio_attacking_count[index]++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any firstspawn voice samples
      audio_firstspawn_count[index] = 0; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/sound/racc/%s/firstspawn*.wav", server.mod_name, current_language);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         audio_firstspawn_count[index]++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any inposition voice samples
      audio_inposition_count[index] = 0; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/sound/racc/%s/inposition*.wav", server.mod_name, current_language);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         audio_inposition_count[index]++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any negative voice samples
      audio_negative_count[index] = 0; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/sound/racc/%s/negative*.wav", server.mod_name, current_language);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         audio_negative_count[index]++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any report voice samples
      audio_report_count[index] = 0; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/sound/racc/%s/report*.wav", server.mod_name, current_language);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         audio_report_count[index]++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any reporting voice samples
      audio_reporting_count[index] = 0; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/sound/racc/%s/reporting*.wav", server.mod_name, current_language);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         audio_reporting_count[index]++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any seegrenade voice samples
      audio_seegrenade_count[index] = 0; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/sound/racc/%s/seegrenade*.wav", server.mod_name, current_language);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         audio_seegrenade_count[index]++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any takingdamage voice samples
      audio_takingdamage_count[index] = 0; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/sound/racc/%s/takingdamage*.wav", server.mod_name, current_language);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         audio_takingdamage_count[index]++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any throwgrenade voice samples
      audio_throwgrenade_count[index] = 0; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/sound/racc/%s/throwgrenade*.wav", server.mod_name, current_language);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         audio_throwgrenade_count[index]++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any victory voice samples
      audio_victory_count[index] = 0; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/sound/racc/%s/victory*.wav", server.mod_name, current_language);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         audio_victory_count[index]++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search
   }

   // profiles are loaded, now ensure the max_bots variable is in bounds
   if (((profile_count > 0) && (server.max_bots > profile_count))
       || (server.max_bots < 0) || (server.max_bots > 31))
      server.max_bots = profile_count; // adjust max_bots to the bot list count

   return; // finished
}


void PrecacheStuff (void)
{
   // this is the function that precaches the stuff we need by the server side, such as the
   // entity model for the entities used in the fakeclient illumination bugfix, and the sprites
   // used for displaying the HLVoice icon above the bot's head when they talk.

   FILE *fp;
   char line_buffer[256];

   if (dummyent_model == 0)
      dummyent_model = PRECACHE_MODEL ("models/mechgibs.mdl"); // used to create fake entities

   if (beam_model == 0)
      beam_model = PRECACHE_MODEL ("sprites/lgtning.spr"); // used to trace beams

   if (speaker_model == 0)
      speaker_model = PRECACHE_MODEL ("sprites/voiceicon.spr"); // used to display speaker icon

   // get voice icon altitude above players head from game config file in the valve directory
   voiceicon_height = 45;// stay at default value 45 if file not found
   if ((fp = fopen ("valve/scripts/voicemodel.txt", "r")) != NULL)
   {
      if (fgets (line_buffer, 255, fp) != NULL)
         voiceicon_height = atoi (line_buffer); // read the whole line and look for a number
      fclose (fp); // close the file
   }

   return;
}


void SpawnDoor (edict_t *pDoorEntity)
{
   edict_t *pFakeEntity = NULL;

   if (FNullEnt (pDoorEntity))
      return; // reliability check

   if (strncmp (STRING (pDoorEntity->v.netname), "secret", 6) == 0)
      return; // skip secret doors

   pFakeEntity = CREATE_NAMED_ENTITY (MAKE_STRING ("info_target")); // create door origin entity
   Spawn (pFakeEntity); // spawn it
   pFakeEntity->v.origin = VecBModelOrigin (pDoorEntity); // same origin as door, obviously
   pFakeEntity->v.takedamage = DAMAGE_NO; // doesn't allow it to take damage
   pFakeEntity->v.solid = SOLID_NOT; // make it invisible
   pFakeEntity->v.movetype = MOVETYPE_NOCLIP; // no clip
   pFakeEntity->v.classname = MAKE_STRING ("door_origin"); // sets a name for it
   pFakeEntity->v.rendermode = kRenderNormal; // normal rendering mode
   pFakeEntity->v.renderfx = kRenderFxNone; // no special FX
   pFakeEntity->v.renderamt = 0; // ???
   pFakeEntity->v.owner = pDoorEntity; // sets the real door as the owner of the origin entity

   if (dummyent_model == 0)
      dummyent_model = PRECACHE_MODEL ("models/mechgibs.mdl"); // used to create fake entities

   SET_MODEL (pFakeEntity, "models/mechgibs.mdl"); // sets it a model

   return; // done, door is safe
}


void LoadSymbols (const char *filename)
{
   // the purpose of this function is to perfect the bot DLL interfacing. Having all the
   // MOD entities listed and linked to their proper function with LINK_ENTITY_TO_FUNC is
   // not enough, procs are missing, and that's the reason why most bot DLLs don't allow
   // to run single player games. This function loads the symbols in the game DLL by hand,
   // strips their MSVC-style case mangling, and builds an exports array which supercedes
   // the one the engine would get afterwards from the MOD DLL, which can't pass through
   // the bot DLL. This way we are sure that *nothing is missing* in the interfacing. Note
   // this is a fix for WIN32 systems only. But since UNIX systems only host dedicated
   // servers, there's no need to run single-player games on them.

   #ifdef _WIN32
   {
      FILE *fp;
      DOS_HEADER dos_header;
      LONG nt_signature;
      PE_HEADER pe_header;
      SECTION_HEADER section_header;
      OPTIONAL_HEADER optional_header;
      LONG edata_offset;
      LONG edata_delta;
      EXPORT_DIRECTORY export_directory;
      LONG name_offset;
      LONG ordinal_offset;
      LONG function_offset;
      char function_name[256], ch;
      int i, j;
      void *game_GiveFnptrsToDll;

      // reset function names array first
      for (i = 0; i < num_ordinals; i++)
         p_FunctionNames[i] = NULL;

      // open MOD DLL file in binary read mode
      fp = fopen (filename, "rb"); // can't fail to do this, since we LoadLibrary()'ed it before

      fread (&dos_header, sizeof (dos_header), 1, fp); // get the DOS header
      fseek (fp, dos_header.e_lfanew, SEEK_SET);
      fread (&nt_signature, sizeof (nt_signature), 1, fp); // get the NT signature
      fread (&pe_header, sizeof (pe_header), 1, fp); // get the PE header
      fread (&optional_header, sizeof (optional_header), 1, fp); // get the optional header

      edata_offset = optional_header.DataDirectory[0].VirtualAddress; // no edata by default
      edata_delta = 0;

      // cycle through all sections of the PE header to look for edata
      for (i = 0; i < pe_header.NumberOfSections; i++)
         if (strcmp ((char *) section_header.Name, ".edata") == 0)
         {
            edata_offset = section_header.PointerToRawData; // if found, save its offset
            edata_delta = section_header.VirtualAddress - section_header.PointerToRawData;
         }

      fseek (fp, edata_offset, SEEK_SET);
      fread (&export_directory, sizeof (export_directory), 1, fp); // get the export directory

      num_ordinals = export_directory.NumberOfNames; // save number of ordinals

      ordinal_offset = export_directory.AddressOfNameOrdinals - edata_delta; // save ordinals offset
      fseek (fp, ordinal_offset, SEEK_SET);
      p_Ordinals = (WORD *) malloc (num_ordinals * sizeof (WORD)); // allocate space for ordinals
      fread (p_Ordinals, num_ordinals * sizeof (WORD), 1, fp); // get the list of ordinals

      function_offset = export_directory.AddressOfFunctions - edata_delta; // save functions offset
      fseek (fp, function_offset, SEEK_SET);
      p_Functions = (DWORD *) malloc (num_ordinals * sizeof (DWORD)); // allocate space for functions
      fread (p_Functions, num_ordinals * sizeof (DWORD), 1, fp); // get the list of functions

      name_offset = export_directory.AddressOfNames - edata_delta; // save names offset
      fseek (fp, name_offset, SEEK_SET);
      p_Names = (DWORD *) malloc (num_ordinals * sizeof (DWORD)); // allocate space for names
      fread (p_Names, num_ordinals * sizeof (DWORD), 1, fp); // get the list of names

      // cycle through all function names and fill in the exports array
      for (i = 0; i < num_ordinals; i++)
      {
         if (fseek (fp, p_Names[i] - edata_delta, SEEK_SET) != -1)
         {
            j = 0; // start at beginning of string

            // while end of file is not reached
            while ((ch = fgetc (fp)) != EOF)
            {
               function_name[j] = ch; // store what is read in the name variable
               if (ch == 0)
                  break; // return the name with the trailing \0
               j++;
            }

            // allocate space
            p_FunctionNames[i] = (char *) malloc (strlen (function_name) + 1);

            // is this a MSVC C++ mangled name ?
            if (function_name[0] == '?')
            {
               j = 1; // skip the leading '?'

               // while the first @@ is not reached
               while (!((function_name[j] == '@') && (function_name[j + 1] == '@')))
               {
                  p_FunctionNames[i][j - 1] = function_name[j]; // store what is read in the name variable
                  if (function_name[j + 1] == 0)
                     break; // return the name
                  j++;
               }

               p_FunctionNames[i][j] = 0; // terminate string at the "@@"
            }

            // else no change needed
            else
               strcpy (p_FunctionNames[i], function_name);
         }
      }

      fclose (fp); // close MOD DLL file

      // cycle through all function names to find the GiveFnptrsToDll function
      for (i = 0; i < num_ordinals; i++)
      {
         if (strcmp ("GiveFnptrsToDll", p_FunctionNames[i]) == 0)
         {
            game_GiveFnptrsToDll = (void *) GetProcAddress (h_Library, "GiveFnptrsToDll");
            base_offset = (unsigned long) (game_GiveFnptrsToDll) - p_Functions[p_Ordinals[i]];
            break; // base offset has been saved
         }
      }
   }
   #endif
}


void FakeClientCommand (edict_t *pFakeClient, const char *fmt, ...)
{
   // the purpose of this function is to provide fakeclients (bots) with the same client
   // command-scripting advantages (putting multiple commands in one line between semicolons)
   // as real players. It is an improved version of botman's FakeClientCommand, in which you
   // supply directly the whole string as if you were typing it in the bot's "console". It
   // is supposed to work exactly like the pfnClientCommand (server-sided client command).

   va_list argptr;
   static char command[256];
   int length, fieldstart, fieldstop, i, index, stringindex = 0;

   if (!IsValidPlayer (pFakeClient))
      return; // reliability check

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   vsprintf (command, fmt, argptr);
   va_end (argptr);

   if ((command == NULL) || (*command == 0))
      return; // if nothing in the command buffer, return

   isFakeClientCommand = TRUE; // set the "fakeclient command" flag
   length = strlen (command); // get the total length of the command string

   // process all individual commands (separated by a semicolon) one each a time
   while (stringindex < length)
   {
      fieldstart = stringindex; // save field start position (first character)
      while ((stringindex < length) && (command[stringindex] != ';'))
         stringindex++; // reach end of field
      if (command[stringindex - 1] == '\n')
         fieldstop = stringindex - 2; // discard any trailing '\n' if needed
      else
         fieldstop = stringindex - 1; // save field stop position (last character before semicolon or end)
      for (i = fieldstart; i <= fieldstop; i++)
         g_argv[i - fieldstart] = command[i]; // store the field value in the g_argv global string
      g_argv[i - fieldstart] = 0; // terminate the string
      stringindex++; // move the overall string index one step further to bypass the semicolon

      index = 0;
      fake_arg_count = 0; // let's now parse that command and count the different arguments

      // count the number of arguments
      while (index < i - fieldstart)
      {
         while ((index < i - fieldstart) && (g_argv[index] == ' '))
            index++; // ignore spaces

         // is this field a group of words between quotes or a single word ?
         if (g_argv[index] == '"')
         {
            index++; // move one step further to bypass the quote
            while ((index < i - fieldstart) && (g_argv[index] != '"'))
               index++; // reach end of field
            index++; // move one step further to bypass the quote
         }
         else
            while ((index < i - fieldstart) && (g_argv[index] != ' '))
               index++; // this is a single word, so reach the end of field

         fake_arg_count++; // we have processed one argument more
      }

      ClientCommand (pFakeClient); // tell now the MOD DLL to execute this ClientCommand...
   }

   g_argv[0] = 0; // when it's done, reset the g_argv field
   isFakeClientCommand = FALSE; // reset the "fakeclient command" flag
   fake_arg_count = 0; // and the argument count
}


const char *GetField (const char *string, int field_number)
{
   // This function gets and returns a particuliar field in a string where several fields are
   // concatenated. Fields can be words, or groups of words between quotes ; separators may be
   // white space or tabs. A purpose of this function is to provide bots with the same Cmd_Argv
   // convenience the engine provides to real clients. This way the handling of real client
   // commands and bot client commands is exactly the same, just have a look in engine.cpp
   // for the hooking of pfnCmd_Argc, pfnCmd_Args and pfnCmd_Argv, which redirects the call
   // either to the actual engine functions (when the caller is a real client), either on
   // our function here, which does the same thing, when the caller is a bot.

   static char field[256];
   int length, i, index = 0, field_count = 0, fieldstart, fieldstop;

   field[0] = 0; // reset field
   length = strlen (string); // get length of string

   // while we have not reached end of line
   while ((index < length) && (field_count <= field_number))
   {
      while ((index < length) && ((string[index] == ' ') || (string[index] == '\t')))
         index++; // ignore spaces or tabs

      // is this field multi-word between quotes or single word ?
      if (string[index] == '"')
      {
         index++; // move one step further to bypass the quote
         fieldstart = index; // save field start position
         while ((index < length) && (string[index] != '"'))
            index++; // reach end of field
         fieldstop = index - 1; // save field stop position
         index++; // move one step further to bypass the quote
      }
      else
      {
         fieldstart = index; // save field start position
         while ((index < length) && ((string[index] != ' ') && (string[index] != '\t')))
            index++; // reach end of field
         fieldstop = index - 1; // save field stop position
      }

      // is this field we just processed the wanted one ?
      if (field_count == field_number)
      {
         for (i = fieldstart; i <= fieldstop; i++)
            field[i - fieldstart] = string[i]; // store the field value in a string
         field[i - fieldstart] = 0; // terminate the string
         break; // and stop parsing
      }

      field_count++; // we have parsed one field more
   }

   return (&field[0]); // returns the wanted field
}


const char *GetConfigKey (const char *config_string)
{
   // This function gets and returns the name of a configuration key in a configuration line
   // (configuration lines are text lines in format 'key="string_value"' or 'key=numeral_value'
   // that are read from config files such as .ini files). If no key is found a pointer to an
   // empty static string is returned.

   static char key[256];
   int length, i, index = 0, fieldstart, fieldstop;

   key[0] = 0; // reset key string
   length = strlen (config_string); // get length of config string

   index = 0;
   while ((index < length) && ((config_string[index] == ' ') || (config_string[index] == '\t')))
      index++; // ignore leading spaces or tabs in configuration string

   if (config_string[index] == '[')
      return (&key[0]); // if first character is a bracket, then it's not a key, it's a section

   fieldstart = index; // save possible key start position
   while ((index < length) && (config_string[index] != '='))
      index++; // reach the key/value separator

   if (index == length)
      return (&key[0]); // if separator not found, then it's not a key/value pair

   index--; // step back before the separator
   while ((index > fieldstart) && ((config_string[index] == ' ') || (config_string[index] == '\t')))
      index--; // ignore trailing spaces or tabs in configuration string

   fieldstop = index; // save key stop position

   for (i = fieldstart; i <= fieldstop; i++)
      key[i - fieldstart] = config_string[i]; // store the key in a string
   key[i - fieldstart] = 0; // terminate the string

   return (&key[0]); // return the key name
}


const char *GetConfigValue (const char *config_string)
{
   // This function gets and returns a configuration value in a configuration line (configuration
   // lines are text lines in format 'key="string_value"' or 'key=numeral_value' that are read
   // from config files such as .ini files). If no key is found a pointer to an empty static
   // string is returned.

   static char value[256];
   int length, i, index = 0, fieldstart, fieldstop;

   value[0] = 0; // reset value string
   length = strlen (config_string); // get length of config string

   index = 0;
   while ((index < length) && (config_string[index] != '='))
      index++; // reach the key/value separator

   if (index == length)
      return (&value[0]); // if separator not found, then it's not a key/value pair

   index++; // move one step further to bypass the separator
   while ((index < length) && ((config_string[index] == ' ') || (config_string[index] == '\t')))
      index++; // ignore leading spaces or tabs to value

   // is this value multi-word between quotes or single word ?
   if (config_string[index] == '"')
   {
      index++; // move one step further to bypass the quote
      fieldstart = index; // save value start position
      while ((index < length) && (config_string[index] != '"'))
         index++; // reach end of value
      fieldstop = index - 1; // save value stop position
   }
   else
   {
      fieldstart = index; // save value start position
      index = length - 1; // jump to end of string
      while ((index > fieldstart) && ((config_string[index] == ' ') || (config_string[index] == '\t') || (config_string[index] == 0x0D) || (config_string[index] == 0x0A)))
         index--; // scan backwards until end of value
      fieldstop = index; // save value stop position
   }


   for (i = fieldstart; i <= fieldstop; i++)
      value[i - fieldstart] = config_string[i]; // store the field value in a string
   value[i - fieldstart] = 0; // terminate the string

   return (&value[0]); // and return the value
}


int GetUserMsgId (const char *msg_name)
{
   // this function returns the user message id number of network messages named msg_name that
   // have been registered by the game DLL into the engine at game start. Local variables have
   // been made static to speedup recurrent calls of this function.

   register int i;

   // is it a standard engine message (i.e, NOT a user message, already registered by engine) ?
   if (strcmp ("TempEntity", msg_name) == 0)
      return (SVC_TEMPENTITY); // return the correct message ID
   else if (strcmp ("Intermission", msg_name) == 0)
      return (SVC_INTERMISSION); // return the correct message ID
   else if (strcmp ("CDTrack", msg_name) == 0)
      return (SVC_CDTRACK); // return the correct message ID
   else if (strcmp ("WeaponAnim", msg_name) == 0)
      return (SVC_WEAPONANIM); // return the correct message ID
   else if (strcmp ("RoomType", msg_name) == 0)
      return (SVC_ROOMTYPE); // return the correct message ID
   else if (strcmp ("Director", msg_name) == 0)
      return (SVC_DIRECTOR); // return the correct message ID

   // cycle through our known user message types array
   for (i = 0; i < usermsgs_count; i++)
      if (strcmp (usermsgs[i].name, msg_name) == 0)
         return (usermsgs[i].id); // return the id of the user message named msg_name

   // unregistered user message ! print an alert if in developer mode
   if (server.developer_level > 0)
      ServerConsole_printf ("ALERT: message \"%s\" unregistered!\n", msg_name);

   return (-1);
}


const char *GetUserMsgName (int msg_type)
{
   // this function is the counterpart of GetUserMsgId(), in that it returns the user message
   // name of the recorded message whose ID number is msg_type. Local variables have been made
   // static to speedup recurrent calls of this function.

   register int i;

   // is it a standard engine message (i.e, NOT a user message, already registered by engine) ?
   if (msg_type == SVC_TEMPENTITY)
      return ("TempEntity"); // return the correct message name
   else if (msg_type == SVC_INTERMISSION)
      return ("Intermission"); // return the correct message name
   else if (msg_type == SVC_CDTRACK)
      return ("CDTrack"); // return the correct message name
   else if (msg_type == SVC_WEAPONANIM)
      return ("WeaponAnim"); // return the correct message name
   else if (msg_type == SVC_ROOMTYPE)
      return ("RoomType"); // return the correct message name
   else if (msg_type == SVC_DIRECTOR)
      return ("Director"); // return the correct message name

   // cycle through our known user message types array
   for (i = 0; i < usermsgs_count; i++)
      if (usermsgs[i].id == msg_type)
         return (usermsgs[i].name); // return the name of the user message having the msg_type id

   // unknown user message ! print an alert if in developer mode
   if (server.developer_level > 0)
      ServerConsole_printf ("ALERT: message type \"%d\" unknown!\n", msg_type);

   return (NULL); // unknown user message
}


weapon_t *FindWeaponByName (const char *weapon_name)
{
   // given a weapon name, this function finds the actual weapon entry in the global weapons
   // database, so that we get access to its parameters (rails, usage, type of ammo, sound, etc.)
   // If weapon name is not found in the database, return an empty static structure.

   register int index;

   if (weapon_name == NULL)
      return (&unknown_weapon); // reliability check

   // for each weapon in the global array, compare its name with that one
   for (index = 0; index < weapon_count; index++)
      if (strcmp (weapon_name, weapons[index].classname) == 0)
         return (&weapons[index]); // when found, return a pointer to the weapon entry

   // damnit, weapon not found !
   ServerConsole_printf ("RACC: FindWeaponByName(): weapon \"%s\" not found in database\n", weapon_name);

   return (&unknown_weapon); // return empty weapon
}


weapon_t *FindWeaponByModel (const char *weapon_model)
{
   // given a weapon model, this function finds the actual weapon entry in the global weapons
   // database, so that we get access to its parameters (rails, usage, type of ammo, sound, etc.)
   // If weapon name is not found in the database, return an empty static structure.

   register int index;

   if (weapon_model == NULL)
      return (&unknown_weapon); // reliability check

   // for each weapon in the global array, check if the model matchs (skip "models/p_" prefix)
   for (index = 0; index < weapon_count; index++)
      if ((weapons[index].model[0] != 0) && (stricmp (weapon_model + 9, weapons[index].model + 9) == 0))
         return (&weapons[index]); // when found, return a pointer to the weapon entry

   // damnit, weapon not found !
   ServerConsole_printf ("RACC: FindWeaponByModel(): weapon model \"%s\" not found in database\n", weapon_model);

   return (&unknown_weapon); // return empty weapon
}


weapon_t *FindWeaponById (const int weapon_id)
{
   // given a weapon id, this function finds the actual weapon entry in the global weapons
   // database, so that we get access to its parameters (rails, usage, type of ammo, sound, etc.)
   // If weapon id is not found in the database, return an empty static structure.

   register int index;

   if (weapon_id < 0)
      return (&unknown_weapon); // reliability check

   // for each weapon in the global array, check if the ids match
   for (index = 0; index < weapon_count; index++)
      if (weapons[index].id == weapon_id)
         return (&weapons[index]); // when found, return a pointer to the weapon entry

   // damnit, weapon not found !
   ServerConsole_printf ("RACC: FindWeaponById(): weapon id \"%d\" not found in database\n", weapon_id);

   return (&unknown_weapon); // return empty weapon
}


int WeaponIndexOf (weapon_t *weapon)
{
   // this function converts a weapon pointer into its corresponding index in the global weapons
   // walkfaces database. Local variables have been declared static to speedup recurrent calls
   // of this function.

   register int index;

   if (weapon == NULL)
      TerminateOnError ("WeaponIndexOf(): function called with NULL weapon\n");

   index = -1; // first set index to a bad value, for later error checking
   index = ((unsigned long) weapon - (unsigned long) weapons) / sizeof (weapon_t);

   // check for the index validity (it must ALWAYS be valid, so bomb out on error)
   if ((index < 0) || (index > weapon_count - 1))
      TerminateOnError ("WeaponIndexOf(): bad weapon array index %d (range 0-%d)\n", index, weapon_count - 1);

   return (index); // looks like we found a valid index, so return it
}


sound_t *FindSoundByFilename (const char *sound_filename)
{
   // given a sound filename, this function finds the actual sound entry in the global sounds
   // database, so that we get access to its parameters (volume, duration, etc.)
   // If no sound with that filename is found in the database, return an empty static structure.

   // TODO: pre-sort sounds and perform faster binary search here.

   register int index;
   static sound_t unknown_sound = { "", 0.0, 0.0 };

   if (sound_filename == NULL)
      return (&unknown_sound); // reliability check

   // for each sound in the global array, check if the filename matchs
   for (index = 0; index < sound_count; index++)
      if (strcmp (sounds[index].file_path, sound_filename) == 0)
         return (&sounds[index]); // when found, return a pointer to the sound entry

   // damnit, sound not found !
   if (DebugLevel.ears > 0)
      ServerConsole_printf ("RACC: FindSoundByFilename(): sound \"%s\" not found in database\n", sound_filename);

   return (&unknown_sound); // return empty weapon
}


void EstimateNextFrameDuration (void)
{
   // this function is a low-pass filter that estimates the amount of time the current frame is
   // going to last, based on some average of the duration of the previous ones. It is used to
   // tell the engine how long the movement of the bots should extend inside the current frame.
   // It is very important for it to be exact, else one can experience bizarre problems, such as
   // bots getting stuck into each other. The reason is that the collision boxes of the bots are
   // only valid to the engine for the duration of this movement. It should not be over-estimated
   // either, else the bot's movement will be too slow, giving them a "moonwalker" look.
   // The method used here is Rich 'TheFatal' Whitehouse's. Alternates are Tobias 'Killaruna'
   // Heimann's and Leon 'Jehannum' Hartwig's. Which one is the best for the job is left as an
   // exercise to the reader, for I haven't been able to decide myself.

   switch (server.msec_method)
   {
      default:
         server.msec_method = METHOD_RICH; // use Rich Whitehouse's method by default

      case (METHOD_PM):
         // my own method for computing the msec value

         server.msecval = (int) (gpGlobals->frametime * 1000);
         if (server.msecval < 1)
            server.msecval = 1;
         else if (server.msecval > 255)
            server.msecval = 255;

         break;

      case (METHOD_RICH):
         // Rich's method for computing the msec value

         if (server.msecdel <= *server.time)
         {
            if (server.msecnum > 0)
               server.msecval = 450.0 / server.msecnum;

            server.msecdel = *server.time + 0.5; // next check in half a second
            server.msecnum = 0;
         }
         else
            server.msecnum++;

         if (server.msecval < 1)
            server.msecval = 1; // don't allow the msec delay to be null
         else if (server.msecval > 100)
            server.msecval = 100; // don't allow it to last longer than 100 milliseconds either

         break;

      case (METHOD_LEON):
         // Leon's method for computing the msec value

         server.msecval = (int) ((*server.time - server.previous_time) * 1000);
         if (server.msecval > 255)
            server.msecval = 255;

         break;

      case (METHOD_TOBIAS):
         // Tobias's method for computing the msec value

         if ((server.msecdel + server.msecnum / 1000) < *server.time - 0.5)
         {
            server.msecdel = *server.time - 0.05; // after pause
            server.msecnum = 0;
         }

         if (server.msecdel > *server.time)
         {
            server.msecdel = *server.time - 0.05; // after map changes
            server.msecnum = 0;
         }

         server.msecval = (*server.time - server.msecdel) * 1000 - server.msecnum; // optimal msec value since start of 1 sec period
         server.msecnum = (*server.time - server.msecdel) * 1000; // value ve have to add to reach optimum

         // do we have to start a new 1 sec period ?
         if (server.msecnum > 1000)
         {
            server.msecdel += server.msecnum / 1000;
            server.msecnum = 0;
         }

         if (server.msecval < 5)
            server.msecval = 5; // don't allow the msec delay to be too low
         else if (server.msecval > 255)
            server.msecval = 255; // don't allow it to last longer than 255 milliseconds either

         break;
   }

   return; // estimation completed, the estimated value is held in the msecval global variable.
}


void SendWelcomeMessage (edict_t *pClient)
{
   // this function sends a welcome message to incoming clients, under the form of a white text
   // that fades in, holds a few seconds and then fades out in the center of the screen. A test
   // is made that cancels the function if the client to send the message to is a bot, as sending
   // such messages (HUD text) to bots would crash the server, the bots having no client DLL. I
   // know we're supposed to have the checks that fit well in engine.cpp for this, but never
   // trust anyone but yourself, mama said. Ah also, a little sound is played on the client side
   // along with the message, and BTW, this function behaves a little particularly when it's my
   // birthday... :) :) :)

   static char welcome_text[512];

   if (!IsValidPlayer (pClient))
      return; // reliability check

   if (pClient->v.flags & (FL_FAKECLIENT | FL_THIRDPARTYBOT))
      return; // also don't send messages to bots

   // is today my birthday ? :D
   if (IsMyBirthday ())
   {
      // send the "special birthday" welcome message to this client
      MESSAGE_BEGIN (MSG_ONE_UNRELIABLE, GetUserMsgId ("TempEntity"), NULL, pClient);
      WRITE_BYTE (TE_TEXTMESSAGE);
      WRITE_BYTE (1); // channel
      WRITE_SHORT (-8192); // x coordinates * 8192
      WRITE_SHORT (-8192); // y coordinates * 8192
      WRITE_BYTE (2); // effect (fade in/out)
      WRITE_BYTE (255); // initial RED
      WRITE_BYTE (255); // initial GREEN
      WRITE_BYTE (255); // initial BLUE
      WRITE_BYTE (1); // initial ALPHA
      WRITE_BYTE (RANDOM_LONG (0, 255)); // effect RED
      WRITE_BYTE (RANDOM_LONG (0, 255)); // effect GREEN
      WRITE_BYTE (RANDOM_LONG (0, 255)); // effect BLUE
      WRITE_BYTE (1); // effect ALPHA
      WRITE_SHORT (25); // fade-in time in seconds * 256
      WRITE_SHORT (50); // fade-out time in seconds * 256
      WRITE_SHORT (2048); // hold time in seconds * 256
      WRITE_SHORT (2048); // effect time in seconds * 256

      // build the welcome text
      sprintf (welcome_text, "%s\n"
                             "\n"
                             "\n"
                             "\n"
                             "Today is 28 february - it's the author's birthday!\n"
                             "Happy Birthday PMB !!!\n"
                             "Send him an email to pm@racc-ai.com", racc_welcometext);

      WRITE_STRING (welcome_text); // it's my birthday!!
      MESSAGE_END (); // end

      // play the "special birthday" welcome sound on this client (hehe)
      CLIENT_COMMAND (pClient, "play barney/coldone.wav\n");
   }

   // looks like it's just a normal, boring day
   else
   {
      // send the welcome message to this client
      MESSAGE_BEGIN (MSG_ONE_UNRELIABLE, GetUserMsgId ("TempEntity"), NULL, pClient);
      WRITE_BYTE (TE_TEXTMESSAGE);
      WRITE_BYTE (1); // channel
      WRITE_SHORT (-8192); // x coordinates * 8192
      WRITE_SHORT (-8192); // y coordinates * 8192
      WRITE_BYTE (0); // effect (fade in/out)
      WRITE_BYTE (255); // initial RED
      WRITE_BYTE (255); // initial GREEN
      WRITE_BYTE (255); // initial BLUE
      WRITE_BYTE (1); // initial ALPHA
      WRITE_BYTE (255); // effect RED
      WRITE_BYTE (0); // effect GREEN
      WRITE_BYTE (0); // effect BLUE
      WRITE_BYTE (1); // effect ALPHA
      WRITE_SHORT (256); // fade-in time in seconds * 256
      WRITE_SHORT (512); // fade-out time in seconds * 256
      WRITE_SHORT (256); // hold time in seconds * 256

      // build the welcome text
      sprintf (welcome_text, "\n\n\n\n\n%s", racc_welcometext);

      WRITE_STRING (welcome_text); // send the welcome message
      MESSAGE_END (); // end

      // play welcome sound on this client
      if (FileExists ("cstrike/sound/" RACC_WELCOMESOUND))
         CLIENT_COMMAND (pClient, "play " RACC_WELCOMESOUND "\n"); // normal welcome sound
      else
         CLIENT_COMMAND (pClient, "play barney/guyresponsible.wav"); // fallback sound
   }
}


void MakeVersion (void)
{
   const char *compile_date = __DATE__;
   char year[5], month[3], day[3];

   // get the year under the form YYYY
   strncpy (year, compile_date + 7, 4);
   year[4] = 0; // terminate the string

   // translate the month string under the form MM
   if (strncmp ("Jan", compile_date, 3) == 0)
      strcpy (month, "01");
   else if (strncmp ("Feb", compile_date, 3) == 0)
      strcpy (month, "02");
   else if (strncmp ("Mar", compile_date, 3) == 0)
      strcpy (month, "03");
   else if (strncmp ("Apr", compile_date, 3) == 0)
      strcpy (month, "04");
   else if (strncmp ("May", compile_date, 3) == 0)
      strcpy (month, "05");
   else if (strncmp ("Jun", compile_date, 3) == 0)
      strcpy (month, "06");
   else if (strncmp ("Jul", compile_date, 3) == 0)
      strcpy (month, "07");
   else if (strncmp ("Aug", compile_date, 3) == 0)
      strcpy (month, "08");
   else if (strncmp ("Sep", compile_date, 3) == 0)
      strcpy (month, "09");
   else if (strncmp ("Oct", compile_date, 3) == 0)
      strcpy (month, "10");
   else if (strncmp ("Nov", compile_date, 3) == 0)
      strcpy (month, "11");
   else if (strncmp ("Dec", compile_date, 3) == 0)
      strcpy (month, "12");
   else
      strcpy (month, "??"); // unable to understand the month string, WTH ???
   month[2] = 0; // terminate the string

   // get the day under the form DD
   strncpy (day, compile_date + 4, 2);
   day[2] = 0; // terminate the string

   // build the version string and the welcome text string
   sprintf (racc_version, "%s%s%s", year, month, day);
   sprintf (racc_welcometext, "RACC version %s - http://www.racc-ai.com", racc_version);

   return; // finished
}


bool IsMyBirthday (void)
{
   // hahaha :D big grin to @$3.1415rin for the idea !!!

	time_t now = time (NULL);
	tm *tm_now = localtime (&now);
	
   // is it my birthday ?
	if ((tm_now->tm_mon == 1) && (tm_now->tm_mday == 28))
		return (TRUE); // w00t, today is 28 february, it's my birthday ! :D

   // bah crap
   return (FALSE);
}


const char *GetModName (void)
{
   // this function asks the engine for the MOD directory path, then takes its last element
   // which is the MOD's directory name (i.e. for Counter-Strike, it is "cstrike"), and returns
   // a string pointer describing it.

   char mod_name[256];
   int length, fieldstart, fieldstop;

   GET_GAME_DIR (mod_name); // ask the engine for the MOD directory path

   length = strlen (mod_name); // get the length of the returned string

   // format the returned string to get the last directory name
   fieldstop = length - 1;
   while (((mod_name[fieldstop] == '\\') || (mod_name[fieldstop] == '/')) && (fieldstop > 0))
      fieldstop--; // shift back any trailing separator

   fieldstart = fieldstop;
   while ((mod_name[fieldstart] != '\\') && (mod_name[fieldstart] != '/') && (fieldstart > 0))
      fieldstart--; // shift back to the start of the last subdirectory name

   if ((mod_name[fieldstart] == '\\') || (mod_name[fieldstart] == '/'))
      fieldstart++; // if we reached a separator, step over it

   // now copy the formatted string back onto itself character per character
   for (length = fieldstart; length <= fieldstop; length++)
      mod_name[length - fieldstart] = mod_name[length];
   mod_name[length - fieldstart] = 0; // terminate the string

   return (&mod_name[0]); // and return a pointer to it
}


bool FileExists (char *filename)
{
   // this function tests if a file exists by attempting to open it, and returns TRUE if the
   // file exists and can be opened, FALSE otherwise.

   FILE *fp = fopen (filename, "rb"); // try to open the file

   // have we got a valid file pointer in return ?
   if (fp != NULL)
   {
      fclose (fp); // then the file exists, close it
      return (TRUE); // ...and return TRUE
   }

   return (FALSE); // failed to open the file, assume it doesn't exist
}


bool IsValidPlayer (edict_t *pPlayer)
{
   // this function returns TRUE if the player entity passed is taken by a player. Checking for
   // the player's netname seems to be the only reliable method of assessing that a player slot
   // is currently in use, since the Half-Life engine never frees the players' private data.

   if (FNullEnt (pPlayer) || pPlayer->free)
      return (FALSE); // reliability check

   if (!(pPlayer->v.flags & FL_CLIENT))
      return (FALSE); // if this entity is not explicitly marked as a player entity, it's invalid

   if (STRING (pPlayer->v.netname)[0] == 0)
      return (FALSE); // if this player entity records no netname, then no player is taking it

   return (TRUE); // player entity has a netname, a player must be taking this slot
}


bool IsOnFloor (edict_t *pEntity)
{
   // this function returns TRUE if pEntity is spatially supported by a floor, FALSE otherwise

   if (pEntity->v.flags & FL_CLIENT)
      return ((pEntity->v.flags & (FL_ONGROUND | FL_PARTIALGROUND)) != 0); // read player ground state directly from flags
   else
      return (ENT_IS_ON_FLOOR (pEntity) > 0); // ask the engine for normal entities
}


bool IsFlying (edict_t *pEntity)
{
   // this function returns whether pEntity isn't currently affected by gravity (but still
   // collides with the world) or not. Useful for checking for players who are on ladders.

   return (pEntity->v.movetype == MOVETYPE_FLY); // read the gravity state directly from entvars
}


void FreeAllTheStuff (void)
{
   // this function is in charge of freeing all the memory space we allocated, because the DLL
   // is going to shutdown. Of course a check is made upon the space pointer we want to free not
   // to free it twice in case it would have already been done (malloc and free implementations
   // are usually so crappy they hardly ever give error messages, rather crash without warning).
   // For safety reasons, we also reset the pointer to NULL, in order not to try to access it
   // later. Yeah, this should never happen, but who knows. Note, it is always safest to free
   // things in the reverse order they have been allocated, because of interdependency reasons
   // (for example, the map topology hashtable relies on the map walkfaces array). Hence we
   // start by freeing the bots data, then we move on the global stuff.

   int index;

   FreeMapData (); // free the map data

   // for each bot we loaded, see if we need to free its brain memory space
   for (index = 0; index < RACC_MAX_CLIENTS; index++)
   {
      // free this bot's HAL brain
      HAL_FreeDictionary (bots[index].BotBrain.banned_keywords);
      HAL_FreeDictionary (bots[index].BotBrain.auxiliary_keywords);
      HAL_FreeSwap (bots[index].BotBrain.swappable_keywords);
      HAL_EmptyModel (&bots[index].BotBrain.HAL_model);
      HAL_FreeDictionary (bots[index].BotBrain.input_words);
      HAL_FreeDictionary (bots[index].BotBrain.bot_words);
      HAL_FreeDictionary (bots[index].BotBrain.keys);
      HAL_FreeDictionary (bots[index].BotBrain.replies);

      // free this bot's pathmachine
      BotShutdownPathMachine (&bots[index]);

      // do we need to free this bot's nav brain ?
      if (bots[index].BotBrain.PathMemory)
         free (bots[index].BotBrain.PathMemory); // then free the navigation nodes array
      bots[index].BotBrain.PathMemory = NULL;
   }

   // free our table of exported symbols (only on Win32 platforms)
   if (p_Ordinals)
      free (p_Ordinals);
   p_Ordinals = NULL;
   if (p_Functions)
      free (p_Functions);
   p_Functions = NULL;
   if (p_Names)
      free (p_Names);
   p_Names = NULL;

   for (index = 0; index < num_ordinals; index++)
   {
      if (p_FunctionNames[index])
         free (p_FunctionNames[index]); // free the table of exported symbols
      p_FunctionNames[index] = NULL;
   }

   // do we need to free the DLL library space ? (of course we do)
   if (h_Library)
      FreeLibrary (h_Library); // free the DLL library space
   h_Library = NULL;

   // do we need to close the debug log file ?
   if (DebugLevel.fp != NULL)
      fclose (DebugLevel.fp); // close the file
   DebugLevel.fp = NULL;

   return;
}


void ServerCommand (void)
{
   // this function is the dedicated server command handler for the new RACC server command we
   // registered at game start. It will be called by the engine each time a server command that
   // starts with "racc" is entered in the server console. It works exactly the same way as
   // ClientCommand() does, using the CmdArgc() and CmdArgv() facilities of the engine. Argv(0)
   // is the server command itself (here "racc") and the next ones are its arguments. Just like
   // the stdio command-line parsing in C when you write "long main (long argc, char **argv)".

   char pcmd[128], arg1[128], arg2[128], arg3[128];
   char server_cmd[128];
   Vector v_from, v_to;
   long bot_index, i, sector_i, sector_j;
   MFILE *mfp;

   // get the command and up to 3 arguments
   strcpy (pcmd, CMD_ARGV (1));
   strcpy (arg1, CMD_ARGV (2));
   strcpy (arg2, CMD_ARGV (3));
   strcpy (arg3, CMD_ARGV (4));

   // have we been requested for help ?
   if ((strcmp (pcmd, "help") == 0) || (strcmp (pcmd, "?") == 0))
   {
      // then display a nice help page
      ServerConsole_printf ("%s\n", racc_welcometext);
      ServerConsole_printf ("  -- Available server commands:\n");
      ServerConsole_printf ("racc add - Add a bot to the current game\n");
      ServerConsole_printf ("racc kick - Disconnect a bot from the current game\n");
      ServerConsole_printf ("racc kickall - Disconnect all bots from the current game\n");
      ServerConsole_printf ("racc killall - Kill all bots inside the current game\n");
      ServerConsole_printf ("racc autofill - Enable/Disable bots filling up the server automatically\n");
      ServerConsole_printf ("racc internetmode - Enable/Disable bots leaving and joining randomly\n");
      ServerConsole_printf ("racc chatmode - Display/Change bot chat mode\n");
      ServerConsole_printf ("racc forceteam - Get/Set the team to force bots into\n");
      ServerConsole_printf ("racc minbots - Get/Set the minimal number of bots on the server\n");
      ServerConsole_printf ("racc maxbots - Get/Set the maximal number of bots on the server\n");
      ServerConsole_printf ("racc viewprofiles - Display the bot profiles database\n");
      ServerConsole_printf ("racc viewricochetsounds - Display the bot ricochet sounds database\n");
      ServerConsole_printf ("racc viewsounds - Display the bot sounds database\n");
      ServerConsole_printf ("racc viewweapons - Display the bot weapons database\n");
      ServerConsole_printf ("racc viewbones - Display the bot bones database\n");
      ServerConsole_printf ("racc mesh2dxf - Dump the global navmesh in a DXF file\n");
      ServerConsole_printf ("racc mesh2bmp - Dump the global navmesh in a BMP file\n");
      ServerConsole_printf ("racc sector2dxf - Dump specified sector mesh in DXF file (if no args, dump all)\n");
      ServerConsole_printf ("racc sector2bmp - Dump specified sector mesh in BMP file (if no args, dump all)\n");
      ServerConsole_printf ("racc trainhal - Train all bot's HAL with the specified file\n");
      ServerConsole_printf ("racc botcount - Display the number of bots currently in the game\n");
      ServerConsole_printf ("racc playercount - Display the total number of players currently in the game\n");
      ServerConsole_printf ("racc time - Display the current map play time\n");
      ServerConsole_printf ("racc debug - [developers only] get/set the various debug modes\n");
      ServerConsole_printf ("racc botstat - [developers only] display a bot's miscellaneous stats\n");
      ServerConsole_printf ("racc botorder - [developers only] force a bot to issue a ClientCommand\n");
      ServerConsole_printf ("racc msec - [developers only] get/set the msec calculation method\n");
   }

   // else do we want to add a bot ?
   else if (strcmp (pcmd, "add") == 0)
   {
      BotCreate (); // slap a bot in
      server.bot_check_time = *server.time + 10.0; // delay a while before checking the bot counts
   }

   // else do we want to kick one bot ?
   else if (strcmp (pcmd, "kick") == 0)
   {
      // cycle through all bot slots and kick the first we find
      for (bot_index = 0; bot_index < RACC_MAX_CLIENTS; bot_index++)
         if (bots[bot_index].is_active && IsValidPlayer (bots[bot_index].pEdict))
         {
            sprintf (server_cmd, "kick #%d\n", ENTINDEX (bots[bot_index].pEdict)); // build the kick command string
            SERVER_COMMAND (server_cmd); // boot the bot out
            server.bot_check_time = *server.time + 0.5; // delay a while before checking the bot counts
            break; // stop looping (don't kick the whole population, eh)
         }
   }

   // else do we want to kick ALL the bots ?
   else if (strcmp (pcmd, "kickall") == 0)
   {
      // cycle through all bot slots and kick all those we find
      for (bot_index = 0; bot_index < RACC_MAX_CLIENTS; bot_index++)
         if (bots[bot_index].is_active && IsValidPlayer (bots[bot_index].pEdict))
         {
            sprintf (server_cmd, "kick \"%s\"\n", STRING (bots[bot_index].pEdict->v.netname)); // build the kick command string
            SERVER_COMMAND (server_cmd); // boot this bot out
         }
      server.bot_check_time = *server.time + 0.5; // delay a while before checking the bot counts
   }

   // else do we want to slaughter all the bots in a bloodbath ?
   else if (strcmp (pcmd, "killall") == 0)
   {
      // cycle through all bot slots and kill all those we find
      for (bot_index = 0; bot_index < RACC_MAX_CLIENTS; bot_index++)
         if (bots[bot_index].is_active && IsValidPlayer (bots[bot_index].pEdict))
         {
            bots[bot_index].pEdict->v.frags++; // increment its frag count not to count this as a suicide
            ClientKill (bots[bot_index].pEdict); // force this bot to suicide
         }
   }

   // else do we want to change the autofill feature ?
   else if (strcmp (pcmd, "autofill") == 0)
   {
      if ((arg1 != NULL) && (*arg1 != 0))
         server.is_autofill = (atoi (arg1) == 1); // change the variable state when specified
      ServerConsole_printf ("Automatic server filling is %s\n", (server.is_autofill ? "ENABLED" : "DISABLED")); // print feature status
   }

   // else do we want to change the internet mode feature ?
   else if (strcmp (pcmd, "internetmode") == 0)
   {
      if ((arg1 != NULL) && (*arg1 != 0))
         server.is_internetmode = (atoi (arg1) == 1); // change the variable state when specified
      ServerConsole_printf ("Internet mode is %s\n", (server.is_internetmode ? "ENABLED" : "DISABLED")); // print feature status
   }

   // else do we want to change the chat mode feature ?
   else if (strcmp (pcmd, "chatmode") == 0)
   {
      if ((arg1 != NULL) && (*arg1 != 0))
         server.is_internetmode = (atoi (arg1) == 1); // change the variable state when specified
      ServerConsole_printf ("Bot chat mode is %d (%s)\n", server.bot_chat_mode,
                            ((server.bot_chat_mode == BOT_CHAT_TEXTAUDIO) ? "TEXT + AUDIO" :
                             ((server.bot_chat_mode == BOT_CHAT_AUDIOONLY) ? "AUDIO ONLY" :
                              ((server.bot_chat_mode == BOT_CHAT_TEXTONLY) ? "TEXT ONLY" : "NONE"))));
   }

   // else do we want to get or set the bot forced teams feature ?
   else if (strcmp (pcmd, "forceteam") == 0)
   {
      // have we been passed an argument ?
      if ((arg1 != NULL) && (*arg1 != 0))
      {
         // either we are specified to stop forcing bots, or the team to force them into
         if (stricmp ("NONE", arg1) == 0)
            server.bot_forced_team[0] = 0; // disable team forcing
         else
            sprintf (server.bot_forced_team, arg1); // set new team to force
      }

      // is bot team forcing finally enabled ?
      if (server.bot_forced_team[0] != 0)
      {
         ServerConsole_printf ("Bots forced to team %s\n", server.bot_forced_team); // print variable status
         ServerConsole_printf ("To disable, enter: 'racc forceteam NONE'\n"); // and help message
      }
      else
         ServerConsole_printf ("Bots team forcing is DISABLED\n"); // print variable status
   }

   // else do we want to get or set the minimal number of bots ?
   else if (strcmp (pcmd, "minbots") == 0)
   {
      if ((arg1 != NULL) && (*arg1 != 0))
         server.min_bots = atoi (arg1); // if there's an argument, set min_bots to it
      ServerConsole_printf ("Minimal number of bots is %d\n", server.min_bots); // print variable status
   }

   // else do we want to get or set the maximal number of bots ?
   else if (strcmp (pcmd, "maxbots") == 0)
   {
      if ((arg1 != NULL) && (*arg1 != 0))
         server.max_bots = atoi (arg1); // if there's an argument, set max_bots to it
      ServerConsole_printf ("Maximal number of bots is %d\n", server.max_bots); // print variable status
   }

   // else do we want to display the profiles database ?
   else if (strcmp (pcmd, "viewprofiles") == 0)
   {
      ServerConsole_printf ("Bot profiles:\n"); // tell what we're about to do

      // cycle through all profiles and display them
      for (i = 0; i < profile_count; i++)
         ServerConsole_printf ("name '%s', model '%s', logo '%s', nation '%s', skill %d, team %d, class %d\n",
                               profiles[i].name,
                               profiles[i].skin,
                               profiles[i].logo,
                               (profiles[i].nationality == NATIONALITY_FRENCH ? "french" : (profiles[i].nationality == NATIONALITY_GERMAN ? "german" : (profiles[i].nationality == NATIONALITY_ITALIAN ? "italian" : (profiles[i].nationality == NATIONALITY_SPANISH ? "spanish" : "english")))),
                               profiles[i].skill,
                               profiles[i].team,
                               profiles[i].subclass);

      ServerConsole_printf ("   %d profiles.\n", profile_count); // display the count
   }

   // else do we want to display the ricochet sounds database ?
   else if (strcmp (pcmd, "viewricochetsounds") == 0)
   {
      ServerConsole_printf ("Ricochet sounds:\n"); // tell what we're about to do

      // cycle through all ricochet sounds and display them
      for (i = 0; i < ricochetsound_count; i++)
         ServerConsole_printf ("texture '%c', ricochet \"%s\"\n",
                               ricochetsounds[i].texture_type,
                               ricochetsounds[i].file_path);

      ServerConsole_printf ("   %d ricochet sounds.\n", ricochetsound_count); // display the count
   }

   // else do we want to display the global sounds database ?
   else if (strcmp (pcmd, "viewsounds") == 0)
   {
      ServerConsole_printf ("Global sounds:\n"); // tell what we're about to do

      // cycle through all sounds and display them
      for (i = 0; i < sound_count; i++)
         ServerConsole_printf ("'%s', loudness %.2f, duration %.2f\n",
                               sounds[i].file_path,
                               sounds[i].loudness,
                               sounds[i].duration);

      ServerConsole_printf ("   %d sounds.\n", sound_count); // display the count
   }

   // else do we want to display the weapons database ?
   else if (strcmp (pcmd, "viewweapons") == 0)
   {
      ServerConsole_printf ("Weapons:\n"); // tell what we're about to do

      // cycle through all weapons and display them
      for (i = 0; i < weapon_count; i++)
         ServerConsole_printf ("[%s]\n"
                               "model=\"%s\"\n"
                               "id=%d\n"
                               "slot=%d\n"
                               "position=%d\n"
                               "flags=%d\n"
                               "primary.use_percent=%d\n"
                               "primary.min_range=%.0f\n"
                               "primary.max_range=%.0f\n"
                               "primary.type_of_ammo=%d\n"
                               "primary.min_ammo=%d\n"
                               "primary.max_ammo=%d\n"
                               "primary.can_use_underwater=%d\n"
                               "primary.should_hold=%d\n"
                               "primary.should_charge=%d\n"
                               "primary.charge_delay=%.1f\n"
                               "primary.sound1=\"%s\"\n"
                               "primary.sound2=\"%s\"\n"
                               "primary.min_delay=%.1f %.1f %.1f %.1f %.1f\n"
                               "primary.max_delay=%.1f %.1f %.1f %.1f %.1f\n"
                               "secondary.use_percent=%d\n"
                               "secondary.min_range=%.0f\n"
                               "secondary.max_range=%.0f\n"
                               "secondary.type_of_ammo=%d\n"
                               "secondary.min_ammo=%d\n"
                               "secondary.max_ammo=%d\n"
                               "secondary.can_use_underwater=%d\n"
                               "secondary.should_hold=%d\n"
                               "secondary.should_charge=%d\n"
                               "secondary.charge_delay=%.1f\n"
                               "secondary.sound1=\"%s\"\n"
                               "secondary.sound2=\"%s\"\n"
                               "secondary.min_delay=%.1f %.1f %.1f %.1f %.1f\n"
                               "secondary.max_delay=%.1f %.1f %.1f %.1f %.1f\n"
                               "\n",
                               weapons[i].classname,
                               weapons[i].model,
                               weapons[i].id,
                               weapons[i].slot,
                               weapons[i].position,
                               weapons[i].flags,
                               weapons[i].primary.use_percent,
                               weapons[i].primary.min_range,
                               weapons[i].primary.max_range,
                               weapons[i].primary.type_of_ammo,
                               weapons[i].primary.min_ammo,
                               weapons[i].primary.max_ammo,
                               weapons[i].primary.can_use_underwater,
                               weapons[i].primary.should_hold,
                               weapons[i].primary.should_charge,
                               weapons[i].primary.charge_delay,
                               weapons[i].primary.sound1,
                               weapons[i].primary.sound2,
                               weapons[i].primary.min_delay[0], weapons[i].primary.min_delay[1], weapons[i].primary.min_delay[2], weapons[i].primary.min_delay[3], weapons[i].primary.min_delay[4],
                               weapons[i].primary.max_delay[0], weapons[i].primary.max_delay[1], weapons[i].primary.max_delay[2], weapons[i].primary.max_delay[3], weapons[i].primary.max_delay[4],
                               weapons[i].secondary.use_percent,
                               weapons[i].secondary.min_range,
                               weapons[i].secondary.max_range,
                               weapons[i].secondary.type_of_ammo,
                               weapons[i].secondary.min_ammo,
                               weapons[i].secondary.max_ammo,
                               weapons[i].secondary.can_use_underwater,
                               weapons[i].secondary.should_hold,
                               weapons[i].secondary.should_charge,
                               weapons[i].secondary.charge_delay,
                               weapons[i].secondary.sound1,
                               weapons[i].secondary.sound2,
                               weapons[i].secondary.min_delay[0], weapons[i].secondary.min_delay[1], weapons[i].secondary.min_delay[2], weapons[i].secondary.min_delay[3], weapons[i].secondary.min_delay[4],
                               weapons[i].secondary.max_delay[0], weapons[i].secondary.max_delay[1], weapons[i].secondary.max_delay[2], weapons[i].secondary.max_delay[3], weapons[i].secondary.max_delay[4]);

      ServerConsole_printf ("   %d weapons.\n", weapon_count); // display the count
   }

   // else do we want to display the bones database ?
   else if (strcmp (pcmd, "viewbones") == 0)
   {
      ServerConsole_printf ("Bones:\n"); // tell what we're about to do

      // display all the bones with their numbers
      ServerConsole_printf ("bone 0, 'pelvis', number %d\n"
                            "bone 1, 'spine', number %d\n"
                            "bone 2, 'spine1', number %d\n"
                            "bone 3, 'spine2', number %d\n"
                            "bone 4, 'spine3', number %d\n"
                            "bone 5, 'neck', number %d\n"
                            "bone 6, 'head', number %d\n"
                            "bone 7, 'left clavicle', number %d\n"
                            "bone 8, 'left upperarm', number %d\n"
                            "bone 9, 'left forearm', number %d\n"
                            "bone 10, 'left hand', number %d\n"
                            "bone 11, 'left finger0', number %d\n"
                            "bone 12, 'left finger01', number %d\n"
                            "bone 13, 'left finger1', number %d\n"
                            "bone 14, 'left finger11', number %d\n"
                            "bone 15, 'left thigh', number %d\n"
                            "bone 16, 'left calf', number %d\n"
                            "bone 17, 'left foot', number %d\n"
                            "bone 18, 'right clavicle', number %d\n"
                            "bone 19, 'right upperarm', number %d\n"
                            "bone 20, 'right forearm', number %d\n"
                            "bone 21, 'right hand', number %d\n"
                            "bone 22, 'right finger0', number %d\n"
                            "bone 23, 'right finger01', number %d\n"
                            "bone 24, 'right finger1', number %d\n"
                            "bone 25, 'right finger11', number %d\n"
                            "bone 26, 'right thigh', number %d\n"
                            "bone 27, 'right calf', number %d\n"
                            "bone 28, 'right foot', number %d\n"
                            "\n",
                            playerbones.pelvis,
                            playerbones.spine,
                            playerbones.spine1,
                            playerbones.spine2,
                            playerbones.spine3,
                            playerbones.neck,
                            playerbones.head,
                            playerbones.left_clavicle,
                            playerbones.left_upperarm,
                            playerbones.left_forearm,
                            playerbones.left_hand,
                            playerbones.left_finger0,
                            playerbones.left_finger01,
                            playerbones.left_finger1,
                            playerbones.left_finger11,
                            playerbones.left_thigh,
                            playerbones.left_calf,
                            playerbones.left_foot,
                            playerbones.right_clavicle,
                            playerbones.right_upperarm,
                            playerbones.right_forearm,
                            playerbones.right_hand,
                            playerbones.right_finger0,
                            playerbones.right_finger01,
                            playerbones.right_finger1,
                            playerbones.right_finger11,
                            playerbones.right_thigh,
                            playerbones.right_calf,
                            playerbones.right_foot);

      ServerConsole_printf ("   29 bones.\n"); // display the count
   }

   // else do we want to draw a DXF file of the global map mesh ?
   else if (strcmp (pcmd, "mesh2dxf") == 0)
   {
      // build the DXF file name
      sprintf (arg3, "racc/knowledge/%s/%s-MESH.dxf", server.mod_name, server.map_name);
      ServerConsole_printf ("RACC: drawing global mesh (%d walkfaces) to DXF...\n", map.walkfaces_count);

      InitDebugDXF (); // first init the debug DXF buffer

      // cycle through all walkfaces in the navmesh...
      for (i = 0; i < map.walkfaces_count; i++)
      {
         sprintf (arg1, "Walkface_%d", i); // build layer name after walkface number
         DrawWalkfaceInDebugDXF (&map.walkfaces[i], 7, arg1); // and draw this walkface
      }

      WriteDebugDXF (arg3); // and then draw it
      ServerConsole_printf ("DXF file created: '%s'\n", arg3); // and tell us we're done
   }

   // else do we want to draw a BMP file of the global map mesh ?
   else if (strcmp (pcmd, "mesh2bmp") == 0)
   {
      // build the BMP file name
      sprintf (arg3, "racc/knowledge/%s/%s-MESH.bmp", server.mod_name, server.map_name);
      ServerConsole_printf ("RACC: drawing global mesh (%d walkfaces) to bitmap file...\n", map.walkfaces_count);

      InitDebugBitmap (); // first init the debug bitmap buffer

      // cycle through all walkfaces in the navmesh...
      for (i = 0; i < map.walkfaces_count; i++)
         DrawWalkfaceInDebugBitmap (&map.walkfaces[i], 1); // and draw them

      WriteDebugBitmap (arg3); // and then draw it
      ServerConsole_printf ("BMP file created: '%s'\n", arg3); // and tell us we're done
   }

   // else do we want to draw a DXF file of a particuliar sector of the global map mesh ?
   else if (strcmp (pcmd, "sector2dxf") == 0)
   {
      // have we been specified sector coordinates ?
      if ((arg1 != NULL) && (*arg1 != 0) && (arg2 != NULL) && (*arg2 != 0))
      {
         sector_i = atoi (arg1); // get the sector parallel
         if ((sector_i < 0) || (sector_i >= map.parallels_count))
         {
            ServerConsole_printf ("%d: no such parallel in map '%s'\n", sector_i, server.map_name);
            return; // check for parallel validity
         }
         sector_j = atoi (arg2); // get the sector parallel
         if ((sector_j < 0) || (sector_j >= map.meridians_count))
         {
            ServerConsole_printf ("%d: no such meridian in map '%s'\n", sector_j, server.map_name);
            return; // check for parallel validity
         }

         // build the DXF file name
         sprintf (arg3, "racc/knowledge/%s/%s-%d-%d-MESH.dxf", server.mod_name, server.map_name, sector_i, sector_j);
         ServerConsole_printf ("RACC: drawing sector [%d,%d] (%d walkfaces) to DXF file...\n", sector_i, sector_j, map.walkfaces_count);

         InitDebugDXF (); // first init the debug DXF buffer

         // cycle through all walkfaces in the specified sector...
         for (i = 0; i < map.topology[sector_i][sector_j].faces_count; i++)
         {
            sprintf (arg1, "Walkface_%d", i); // build layer name after walkface number
            DrawWalkfaceInDebugDXF (map.topology[sector_i][sector_j].faces[i], 7, arg1); // and draw this walkface
         }

         sprintf (arg1, "Sector_%d_%d", sector_i, sector_j); // build layer name after sector ID
         DrawSectorInDebugDXF (sector_i, sector_j, 5, arg1);  // now draw the sector itself
         WriteDebugDXF (arg3); // and then write the DXF file to disk
         ServerConsole_printf ("DXF file created: '%s'\n", arg3); // and tell us we're done
      }

      // no sector specified, let's dump the whole crap out !
      else
      {
         // tell us what we're doing...
         ServerConsole_printf ("RACC: drawing ALL sectors to DXF file...\n");

         // build the DXF file name
         sprintf (arg3, "racc/knowledge/%s/%s-MESH.dxf", server.mod_name, server.map_name);

         InitDebugDXF (); // first init the debug DXF buffer

         // loop through all sectors...
         for (sector_i = 0; sector_i < map.parallels_count; sector_i++)
            for (sector_j = 0; sector_j < map.meridians_count; sector_j++)
            {
               sprintf (arg1, "Sector_%d_%d", sector_i, sector_j); // build layer name after sector ID

               // cycle through all walkfaces in the specified sector...
               for (i = 0; i < map.topology[sector_i][sector_j].faces_count; i++)
                  DrawWalkfaceInDebugDXF (map.topology[sector_i][sector_j].faces[i], 7, arg1); // and draw this walkface

               DrawSectorInDebugDXF (sector_i, sector_j, 5, arg1);  // now draw the sector itself
            }

         WriteDebugDXF (arg3); // and then write the DXF file to disk
         ServerConsole_printf ("All sectors dumped.\n"); // and tell us we're done
      }
   }

   // else do we want to draw a BMP file of a particuliar sector of the global map mesh ?
   else if (strcmp (pcmd, "sector2bmp") == 0)
   {
      // have we been specified sector coordinates ?
      if ((arg1 != NULL) && (*arg1 != 0) && (arg2 != NULL) && (*arg2 != 0))
      {
         sector_i = atoi (arg1); // get the sector parallel
         if ((sector_i < 0) || (sector_i >= map.parallels_count))
         {
            ServerConsole_printf ("%d: no such parallel in map '%s'\n", sector_i, server.map_name);
            return; // check for parallel validity
         }
         sector_j = atoi (arg2); // get the sector parallel
         if ((sector_j < 0) || (sector_j >= map.meridians_count))
         {
            ServerConsole_printf ("%d: no such meridian in map '%s'\n", sector_j, server.map_name);
            return; // check for parallel validity
         }

         // build the BMP file name
         sprintf (arg3, "racc/knowledge/%s/%s-%d-%d-MESH.bmp", server.mod_name, server.map_name, sector_i, sector_j);
         ServerConsole_printf ("RACC: drawing sector [%d,%d] (%d walkfaces) to bitmap file...\n", sector_i, sector_j, map.walkfaces_count);

         InitDebugBitmap (); // first init the debug bitmap buffer

         // cycle through all walkfaces in the specified sector...
         for (i = 0; i < map.topology[sector_i][sector_j].faces_count; i++)
            DrawWalkfaceInDebugBitmap (map.topology[sector_i][sector_j].faces[i], 1); // and draw them

         DrawSectorInDebugBitmap (sector_i, sector_j, 5);  // now draw the sector itself
         WriteDebugBitmap (arg3); // and then write the BMP file to disk
         ServerConsole_printf ("BMP file created: '%s'\n", arg3); // and tell us we're done
      }

      // no sector specified, let's dump the whole crap out !
      else
      {
         // tell us what we're doing...
         ServerConsole_printf ("RACC: drawing ALL sectors to bitmap files...\n");

         // loop through all sectors...
         for (sector_i = 0; sector_i < map.parallels_count; sector_i++)
            for (sector_j = 0; sector_j < map.meridians_count; sector_j++)
            {
               // build the BMP file name
               sprintf (arg3, "racc/knowledge/%s/%s-%d-%d-MESH.bmp", server.mod_name, server.map_name, sector_i, sector_j);

               InitDebugBitmap (); // first init the debug bitmap buffer

               // cycle through all walkfaces in the specified sector...
               for (i = 0; i < map.topology[sector_i][sector_j].faces_count; i++)
                  DrawWalkfaceInDebugBitmap (map.topology[sector_i][sector_j].faces[i], 1); // and draw them

               DrawSectorInDebugBitmap (sector_i, sector_j, 5);  // now draw the sector itself
               WriteDebugBitmap (arg3); // and then write the BMP file to disk
            }

         ServerConsole_printf ("All sectors dumped.\n"); // and tell us we're done
      }
   }

   // else do we want to train our bot's HAL with a text file ?
   else if (strcmp (pcmd, "trainhal") == 0)
   {
      // this one is prolly unsafe, anyway - better notify the caller of it...
      ServerConsole_printf ("trainhal: WARNING: DANGEROUS FACILITY!\n");

      // have we specified the name of the file to mess with ?
      if ((arg1 != NULL) && (*arg1 != 0) && FileExists (arg1))
      {
         ServerConsole_printf ("RACC: bot HALs learning from file"); // progress bar start message

         mfp = mfopen (arg1, "r"); // open file (can't fail to do this, FileExists() returned TRUE)

         // have we specified a number of lines to skip ?
         if ((arg2 != NULL) && (*arg2 != 0))
            for (i = 0; i < atoi (arg2); i++)
               mfgets (server_cmd, 100, mfp); // skip as many lines as necessary

         // get one line in the file, line after line until the count is reached
         i = 0;
         while ((mfgets (server_cmd, 100, mfp) != NULL) && (i < atoi (arg3)))
         {
            server_cmd[100] = 0; // terminate the string just to be sure
            if (server_cmd[strlen (server_cmd) - 1] == '\n')
               server_cmd[strlen (server_cmd) - 1] = 0; // get rid of the carriage return, eventually

            // for every bot in game...
            for (bot_index = 0; bot_index < RACC_MAX_CLIENTS; bot_index++)
               if (bots[bot_index].is_active && IsValidPlayer (bots[bot_index].pEdict))
               {
                  // break this line into an array of words and make the bot learn it
                  HAL_MakeWords (UpperCase (server_cmd), bots[bot_index].BotBrain.input_words);
                   HAL_Learn (&bots[bot_index].BotBrain.HAL_model, bots[bot_index].BotBrain.input_words);
               }

            if (i % 10 == 0)
               ServerConsole_printf ("."); // print a trailing dot as progress bar every 10 lines

            i++; // we read one line more
         }

         ServerConsole_printf (" skipped %d, read %d\n", atoi (arg2), atoi (arg3)); // terminate progress bar
         ServerConsole_printf ("done\n");
         mfclose (mfp); // learning process complete, close the file
      }
      else
         ServerConsole_printf ("trainhal: must specify filename, lines to skip, lines to read\n");
   }

   // else do we want to display the bot count ?
   else if (strcmp (pcmd, "botcount") == 0)
      ServerConsole_printf ("%d bots in game\n", bot_count); // display the bot count

   // else do we want to display the player count (including bots) ?
   else if (strcmp (pcmd, "playercount") == 0)
      ServerConsole_printf ("%d players in game\n", player_count); // display the player count

   // else do we want to display the server time ?
   else if (strcmp (pcmd, "time") == 0)
      ServerConsole_printf ("Current map play time: %f seconds\n", *server.time); // display server time

   // else do we want to mess around with the debug levels ?
   else if (strcmp (pcmd, "debug") == 0)
   {
      // have we specified the debug switch or a debug command to mess with ?
      if ((arg1 != NULL) && (*arg1 != 0))
      {
         // given the debug level specified, take the appropriate action
         if (strcmp ("eyes", arg1) == 0)
         {
            if ((arg2 != NULL) && (*arg2 != 0))
               DebugLevel.eyes = atoi (arg2); // if there's an argument, set debug level to it
            ServerConsole_printf ("Vision debug level is %d\n", DebugLevel.eyes); // print debug level
         }
         else if (strcmp ("ears", arg1) == 0)
         {
            if ((arg2 != NULL) && (*arg2 != 0))
               DebugLevel.ears = atoi (arg2); // if there's an argument, set debug level to it
            ServerConsole_printf ("Hearing debug level is %d\n", DebugLevel.ears); // print debug level
         }
         else if (strcmp ("body", arg1) == 0)
         {
            if ((arg2 != NULL) && (*arg2 != 0))
               DebugLevel.body = atoi (arg2); // if there's an argument, set debug level to it
            ServerConsole_printf ("Feeling debug level is %d\n", DebugLevel.body); // print debug level
         }
         else if (strcmp ("legs", arg1) == 0)
         {
            if ((arg2 != NULL) && (*arg2 != 0))
               DebugLevel.legs = atoi (arg2); // if there's an argument, set debug level to it
            ServerConsole_printf ("Movement debug level is %d\n", DebugLevel.legs); // print debug level
         }
         else if (strcmp ("hand", arg1) == 0)
         {
            if ((arg2 != NULL) && (*arg2 != 0))
               DebugLevel.hand = atoi (arg2); // if there's an argument, set debug level to it
            ServerConsole_printf ("Weapon usage debug level is %d\n", DebugLevel.hand); // print debug level
         }
         else if (strcmp ("chat", arg1) == 0)
         {
            if ((arg2 != NULL) && (*arg2 != 0))
               DebugLevel.chat = atoi (arg2); // if there's an argument, set debug level to it
            ServerConsole_printf ("Chat debug level is %d\n", DebugLevel.chat); // print debug level
         }

         // else do we want to enable or disable a particular AI vector ?
         else if ((strcmp ("enable", arg1) == 0) || (strcmp ("disable", arg1) == 0))
         {
            // have we been specified a vector name at all ?
            if ((arg2 != NULL) && (*arg2 != 0))
            {
               // given the AI vector to enable, take the appropriate action
               if (strcmp ("eyes", arg2) == 0)
               {
                  DebugLevel.eyes_disabled = (arg1[0] == 'd'); // enable/disable eyes
                  ServerConsole_printf ("AI sensitive vector 'eyes' %s\n", (!DebugLevel.eyes_disabled ? "ENABLED" : "DISABLED"));
               }
               else if (strcmp ("ears", arg2) == 0)
               {
                  DebugLevel.ears_disabled = (arg1[0] == 'd'); // enable/disable ears
                  ServerConsole_printf ("AI sensitive vector 'ears' %s\n", (!DebugLevel.ears_disabled ? "ENABLED" : "DISABLED"));
               }
               else if (strcmp ("body", arg2) == 0)
               {
                  DebugLevel.body_disabled = (arg1[0] == 'd'); // enable/disable body
                  ServerConsole_printf ("AI sensitive vector 'body' %s\n", (!DebugLevel.body_disabled ? "ENABLED" : "DISABLED"));
               }
               else if (strcmp ("legs", arg2) == 0)
               {
                  DebugLevel.legs_disabled = (arg1[0] == 'd'); // enable/disable legs
                  ServerConsole_printf ("AI motile vector 'legs' %s\n", (!DebugLevel.legs_disabled ? "ENABLED" : "DISABLED"));
               }
               else if (strcmp ("hand", arg2) == 0)
               {
                  DebugLevel.hand_disabled = (arg1[0] == 'd'); // enable/disable hand
                  ServerConsole_printf ("AI motile vector 'hand' %s\n", (!DebugLevel.hand_disabled ? "ENABLED" : "DISABLED"));
               }
               else if (strcmp ("chat", arg2) == 0)
               {
                  DebugLevel.chat_disabled = (arg1[0] == 'd'); // enable/disable chat
                  ServerConsole_printf ("AI motile vector 'chat' %s\n", (!DebugLevel.chat_disabled ? "ENABLED" : "DISABLED"));
               }
               else
                  ServerConsole_printf ("RACC: Not an AI vector '%s'\n", arg2); // bad vector
            }
            else
               ServerConsole_printf ("RACC: must specify an AI vector\n"); // nothing specified
         }

         // special debug switches
         else if (strcmp ("observer", arg1) == 0)
         {
            DebugLevel.is_observer ^= TRUE; // switch debug level on/off (XOR it)
            ServerConsole_printf ("Observer mode is %s\n", (DebugLevel.is_observer ? "ENABLED" : "DISABLED"));
         }
         else if (strcmp ("peacemode", arg1) == 0)
         {
            DebugLevel.is_peacemode ^= TRUE; // switch debug level on/off (XOR it)
            ServerConsole_printf ("Peace mode is %s\n", (DebugLevel.is_peacemode ? "ENABLED" : "DISABLED"));
         }
         else if (strcmp ("dontfind", arg1) == 0)
         {
            DebugLevel.is_dontfindmode ^= TRUE; // switch debug level on/off (XOR it)
            ServerConsole_printf ("Don't find mode is %s\n", (DebugLevel.is_dontfindmode ? "ENABLED" : "DISABLED"));
         }
         else if (strcmp ("inhumanturns", arg1) == 0)
         {
            DebugLevel.is_inhumanturns ^= TRUE; // switch debug level on/off (XOR it)
            ServerConsole_printf ("Inhuman turns mode is %s\n", (DebugLevel.is_inhumanturns ? "ENABLED" : "DISABLED"));
         }
         else if (strcmp ("nav", arg1) == 0)
         {
            if ((arg2 != NULL) && (*arg2 != 0))
               DebugLevel.navigation = atoi (arg2); // if there's an argument, set debug level to it
            ServerConsole_printf ("Navigation debug level is %d\n", DebugLevel.navigation); // print debug level
         }

         // else do we want to completely pause the AI ?
         else if (strcmp ("pause", arg1) == 0)
         {
            DebugLevel.is_paused ^= TRUE; // switch pause on/off (XOR it)
            ServerConsole_printf ("AI is %s\n", (DebugLevel.is_paused ? "PAUSED" : "running")); // print debug level
         }
         else
            ServerConsole_printf ("RACC: No debug level for '%s'\n", arg1); // typo error ?
      }

      // else if nothing has been specified, just print out the debug levels
      else
      {
         ServerConsole_printf ("AI vectors:\n");
         ServerConsole_printf ("   Eyes %s (debug level %d)\n", (!DebugLevel.eyes_disabled ? "ENABLED" : "DISABLED"), DebugLevel.eyes);
         ServerConsole_printf ("   Ears %s (debug level %d)\n", (!DebugLevel.ears_disabled ? "ENABLED" : "DISABLED"), DebugLevel.ears);
         ServerConsole_printf ("   Body %s (debug level %d)\n", (!DebugLevel.body_disabled ? "ENABLED" : "DISABLED"), DebugLevel.body);
         ServerConsole_printf ("   Legs %s (debug level %d)\n", (!DebugLevel.legs_disabled ? "ENABLED" : "DISABLED"), DebugLevel.legs);
         ServerConsole_printf ("   Hand %s (debug level %d)\n", (!DebugLevel.hand_disabled ? "ENABLED" : "DISABLED"), DebugLevel.hand);
         ServerConsole_printf ("   Chat %s (debug level %d)\n", (!DebugLevel.chat_disabled ? "ENABLED" : "DISABLED"), DebugLevel.chat);
         ServerConsole_printf ("special switches:\n");
         ServerConsole_printf ("   Observer mode is %s\n", (DebugLevel.is_observer ? "ENABLED" : "DISABLED"));
         ServerConsole_printf ("   Peace mode is %s\n", (DebugLevel.is_peacemode ? "ENABLED" : "DISABLED"));
         ServerConsole_printf ("   Don't find mode is %s\n", (DebugLevel.is_dontfindmode ? "ENABLED" : "DISABLED"));
         ServerConsole_printf ("   Inhuman turns mode is %s\n", (DebugLevel.is_inhumanturns ? "ENABLED" : "DISABLED"));
         ServerConsole_printf ("   Navigation debug level: %d\n", DebugLevel.navigation);
         ServerConsole_printf ("AI is %s\n", (DebugLevel.is_paused ? "PAUSED" : "running")); // print debug level
      }
   }

   // else do we want to display some dull-ass boring bot stats ?
   else if (strcmp (pcmd, "botstat") == 0)
   {
      // cycle through all bot slots and display stats for all of them
      for (bot_index = 0; bot_index < RACC_MAX_CLIENTS; bot_index++)
         ServerConsole_printf ("%s (skin '%s') - skill:%d - team:%d - class:%d - health:%d - armor:%d - pause time:%f\n",
                               STRING (bots[bot_index].pEdict->v.netname),
                               g_engfuncs.pfnInfoKeyValue ((*g_engfuncs.pfnGetInfoKeyBuffer) (bots[bot_index].pEdict), "model"),
                               bots[bot_index].pProfile->skill,
                               GetTeam (bots[bot_index].pEdict),
                               bots[bot_index].pEdict->v.playerclass,
                               bots[bot_index].pEdict->v.health,
                               bots[bot_index].pEdict->v.armorvalue,
                               bots[bot_index].f_pause_time);

      ServerConsole_printf ("End of list\n"); // tell we are finished
   }

   // else do we want to force a bot to issue a client command ?
   else if (strcmp (pcmd, "botorder") == 0)
   {
      // check if we've got the right number of arguments
      if ((arg1 != NULL) && (*arg1 != 0) && (arg2 != NULL) && (*arg2 != 0))
      {
         // we've got the right number of arguments, so find the bot we want according to arg #1
         for (bot_index = 0; bot_index < RACC_MAX_CLIENTS; bot_index++)
            if (strcmp (STRING (bots[bot_index].pEdict->v.netname), arg1) == 0)
            {
               printf ("BOT %s executes command \"%s\"\n", STRING (bots[bot_index].pEdict->v.netname), arg2);
               FakeClientCommand (bots[bot_index].pEdict, arg2); // and let it execute the client command
               break; // no need to search further
            }
      }
      else
         ServerConsole_printf ("botorder: syntax error\n"
                               "Usage is: racc botorder bot_name \"client_command\"\n"); // syntax error
   }

   // else do we want to get or set the frame time estimation method ?
   else if (strcmp (pcmd, "msec") == 0)
   {
      if ((arg1 != NULL) && (*arg1 != 0))
         server.msec_method = atoi (arg1); // if there's an argument, change method to the one specified
      ServerConsole_printf ("Msec computation method is METHOD_%s\n", (server.msec_method == METHOD_TOBIAS ? "TOBIAS" : (server.msec_method == METHOD_LEON ? "LEON" : (server.msec_method == METHOD_RICH ? "RICH" : "PM"))));
   }

   // what sort of RACC server command is that ??
   else
   {
      ServerConsole_printf ("RACC: Unknown command \"%s\"\n", pcmd);
      ServerConsole_printf ("Type \"racc help\" for list of available commands.\n");
   }

   return; // finished processing the server command
}
