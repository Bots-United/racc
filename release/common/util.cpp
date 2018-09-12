// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// util.cpp
//

// TODO: finish putting comments all over the place

#include "racc.h"


player_t *CreateFakeClient (profile_t *pProfile)
{
   // this very engine-specific function creates a bot client according to the specified profile
   // and links its entity to a pPlayer pointer. In case the function fails, it returns NULL.
   // the function also creates an illumination entity to work around the HL engine bug according
   // to which it is impossible to get the correct value for a bot's illumination otherwise.

   edict_t *pEdict;
   player_t *pPlayer;
   char ip_address[32];
   char reject_reason[128];
   char *infobuffer;
   int index;

   pEdict = (*g_engfuncs.pfnCreateFakeClient) (pProfile->name); // create the fake client
   if (FNullEnt (pEdict))
      return (NULL); // cancel if unable to create fake client

   if (pEdict->pvPrivateData != NULL)
      FREE_PRIVATE (pEdict); // free our predecessor's private data
   pEdict->pvPrivateData = NULL; // fools the private data pointer 
   pEdict->v.frags = 0; // reset his frag count 

   // create the player entity by calling MOD's player() function
   CALL_GAME_ENTITY (PLID, "player", &pEdict->v);

   // link his entity to an useful pointer
   pPlayer = &players[ENTINDEX (pEdict) - 1];
   pPlayer->pEntity = pEdict;
   pPlayer->Bot.pProfile = pProfile;

   // initialize his weapons database pointers
   memset (&pPlayer->Bot.bot_weapons, 0, sizeof (pPlayer->Bot.bot_weapons));
   for (index = 0; index < MAX_WEAPONS; index++)
   {
      pPlayer->Bot.bot_weapons[index].hardware = &weapons[index];
      pPlayer->Bot.bot_weapons[index].primary_ammo = &pPlayer->Bot.bot_ammos[weapons[index].primary.type_of_ammo];
      pPlayer->Bot.bot_weapons[index].secondary_ammo = &pPlayer->Bot.bot_ammos[weapons[index].secondary.type_of_ammo];
   }
   pPlayer->Bot.current_weapon = &pPlayer->Bot.bot_weapons[0]; // set current weapon pointer to failsafe value

   index = ENTINDEX (pPlayer->pEntity); // get his client index
   infobuffer = GET_INFOKEYBUFFER (pPlayer->pEntity); // get his info buffer

   // set him some parameters in the infobuffer
   SET_CLIENT_KEYVALUE (index, infobuffer, "model", "gordon");
   SET_CLIENT_KEYVALUE (index, infobuffer, "rate", "3500.000000");
   SET_CLIENT_KEYVALUE (index, infobuffer, "cl_updaterate", "20");
   SET_CLIENT_KEYVALUE (index, infobuffer, "cl_lw", "1");
   SET_CLIENT_KEYVALUE (index, infobuffer, "cl_lc", "1");
   SET_CLIENT_KEYVALUE (index, infobuffer, "tracker", "0");
   SET_CLIENT_KEYVALUE (index, infobuffer, "cl_dlmax", "128");
   SET_CLIENT_KEYVALUE (index, infobuffer, "lefthand", "1");
   SET_CLIENT_KEYVALUE (index, infobuffer, "friends", "0");
   SET_CLIENT_KEYVALUE (index, infobuffer, "dm", "0");
   SET_CLIENT_KEYVALUE (index, infobuffer, "ah", "1");

   // let him connect to the server under its own name
   sprintf (ip_address, "127.0.0.%d:27005", 100 + index); // build it an unique address
   MDLL_ClientConnect (pPlayer->pEntity, pPlayer->Bot.pProfile->name, ip_address, reject_reason);

   // print a notification message on the dedicated server console if in developer mode
   if (server.is_dedicated && (server.developer_level > 0))
   {
      if (server.developer_level > 1)
      {
         ServerConsole_printf ("Server requiring authentication\n");
         ServerConsole_printf ("Client %s connected\n", STRING (pPlayer->pEntity->v.netname));
         ServerConsole_printf ("Adr: %s\n", ip_address);
      }
      ServerConsole_printf ("Verifying and uploading resources...\n");
      ServerConsole_printf ("Custom resources total 0 bytes\n");
      ServerConsole_printf ("  Decals:  0 bytes\n");
      ServerConsole_printf ("----------------------\n");
      ServerConsole_printf ("Resources to request: 0 bytes\n");
   }

   // let him actually join the game
   MDLL_ClientPutInServer (pPlayer->pEntity);

   // create his illumination entity if none exists yet (thanks to Tom Simpson for the fix)
   if (FNullEnt (pPlayer->Bot.pIllumination))
   {
      pPlayer->Bot.pIllumination = CREATE_NAMED_ENTITY (MAKE_STRING ("info_target"));
      MDLL_Spawn (pPlayer->Bot.pIllumination); // spawn it
      pPlayer->Bot.pIllumination->v.movetype = MOVETYPE_NOCLIP; // set its movement to no clipping
      pPlayer->Bot.pIllumination->v.nextthink = server.time; // needed to make it think
      SET_MODEL (pPlayer->Bot.pIllumination, "models/mechgibs.mdl"); // sets it a model
   }

   return (pPlayer); // alright, our new fake client is created, return it
}


void MoveFakeClient (player_t *pPlayer)
{
   // this function builds the engine-specific input buttons bitmap variable corresponding to
   // the engine-independent player button states structure of pPlayer, and tells the engine
   // to schedule this fakeclient's movement for the current frame. Additionally, it tells the
   // engine to make the bot's illumination entity follow it (the bot's illumination entity is
   // a special entity created as a workaround to an HL engine bug disallowing getting correct
   // values when we request a bot's illumination directly).

   bot_legs_t *pBotLegs;

   pBotLegs = &pPlayer->Bot.BotLegs; // quick access to bot legs
   pPlayer->pEntity->v.button = 0; // reset buttons
   pBotLegs->forward_speed = 0; // reset move_speed
   pBotLegs->strafe_speed = 0; // reset strafe_speed

   // check successively for each of this player's buttons
   if (pPlayer->input_buttons & INPUT_KEY_FORWARD)
   {
      pPlayer->pEntity->v.button |= IN_FORWARD; // this player is moving forward
      pBotLegs->forward_speed = pPlayer->pEntity->v.maxspeed;
   }
   if (pPlayer->input_buttons & INPUT_KEY_BACKWARDS)
   {
      pPlayer->pEntity->v.button |= IN_BACK; // this player is moving backwards
      pBotLegs->forward_speed = -pPlayer->pEntity->v.maxspeed;
   }
   if (pPlayer->input_buttons & INPUT_KEY_TURNLEFT)
   {
      pPlayer->pEntity->v.button |= IN_LEFT; // this player is turning left
      BotSetIdealYaw (pPlayer, pPlayer->Bot.BotHand.ideal_angles.y + 5);
   }
   if (pPlayer->input_buttons & INPUT_KEY_TURNRIGHT)
   {
      pPlayer->pEntity->v.button |= IN_RIGHT; // this player is turning right
      BotSetIdealYaw (pPlayer, pPlayer->Bot.BotHand.ideal_angles.y - 5);
   }
   if (pPlayer->input_buttons & INPUT_KEY_STRAFELEFT)
   {
      pPlayer->pEntity->v.button |= IN_MOVELEFT; // this player is strafing left
      pBotLegs->strafe_speed = -pPlayer->pEntity->v.maxspeed;
   }
   if (pPlayer->input_buttons & INPUT_KEY_STRAFERIGHT)
   {
      pPlayer->pEntity->v.button |= IN_MOVERIGHT; // this player is strafing right
      pBotLegs->strafe_speed = pPlayer->pEntity->v.maxspeed;
   }
   if (pPlayer->input_buttons & INPUT_KEY_JUMP)
      pPlayer->pEntity->v.button |= IN_JUMP; // this player is jumping
   if (pPlayer->input_buttons & INPUT_KEY_DUCK)
      pPlayer->pEntity->v.button |= IN_DUCK; // this player is ducking
   if (pPlayer->input_buttons & INPUT_KEY_PRONE)
      pPlayer->pEntity->v.button |= IN_ALT1; // this player is proning
   if (pPlayer->input_buttons & INPUT_KEY_WALK)
   {
      pPlayer->pEntity->v.button |= IN_RUN; // this player is walking instead of running
      pBotLegs->forward_speed *= GameConfig.walk_speed_factor; // forward walk
      pBotLegs->strafe_speed *= GameConfig.walk_speed_factor; // side walk
   }
   if (pPlayer->input_buttons & INPUT_KEY_USE)
      pPlayer->pEntity->v.button |= IN_USE; // this player is taking an action upon some interactive entity
   if (pPlayer->input_buttons & INPUT_KEY_FIRE1)
      pPlayer->pEntity->v.button |= IN_ATTACK; // this player is firing his primary fire
   if (pPlayer->input_buttons & INPUT_KEY_FIRE2)
      pPlayer->pEntity->v.button |= IN_ATTACK2; // this player is firing his secondary fire
   if (pPlayer->input_buttons & INPUT_KEY_RELOAD)
      pPlayer->pEntity->v.button |= IN_RELOAD; // this player is reloading his weapon
   if (pPlayer->input_buttons & INPUT_KEY_SPRAY)
      pPlayer->pEntity->v.impulse = 201; // this player is spraying his logo
   if (pPlayer->input_buttons & INPUT_KEY_LIGHT)
      pPlayer->pEntity->v.impulse = 100; // this player is flashing his light
   if (pPlayer->input_buttons & INPUT_KEY_DISPLAYSCORE)
      pPlayer->pEntity->v.button |= IN_SCORE; // this player is displaying the scores grid

   // tell the engine to schedule the actual fakeclient movement for msecval milliseconds
   (*g_engfuncs.pfnRunPlayerMove) (pPlayer->pEntity,
                                   WrapAngles (pPlayer->v_angle),
                                   pBotLegs->forward_speed,
                                   pBotLegs->strafe_speed,
                                   0,
                                   pPlayer->pEntity->v.button,
                                   pPlayer->pEntity->v.impulse,
                                   server.msecval);

   // if this fakeclient has an illumination entity (RedFox's engine bug fix, thanks m8t :))
   if (!FNullEnt (pPlayer->Bot.pIllumination))
   {
      SET_ORIGIN (pPlayer->Bot.pIllumination, pPlayer->v_origin); // make his light entity follow him
      if (pPlayer->Bot.pIllumination->v.nextthink + 0.1 < server.time)
         pPlayer->Bot.pIllumination->v.nextthink = server.time + 0.2; // make it think at 5 Hertz
   }

   return; // finished
}


edict_t *FindEntityInSphere (edict_t *pStartEntity, const Vector &v_center, float radius)
{
   // this function returns the first successive element in the list of entities that is
   // located inside a sphere which center is vecCenter and within a radius of flRadius,
   // starting the search at entity pentStart. Since orphan pointers are common within the
   // Half-Life engine, we have to do a check for the returned entity validity.

   edict_t *pEdict;
   int start_index;
   int index;

   // do we want to start the search at a particular entity ?
   if (!FNullEnt (pStartEntity))
      start_index = ENTINDEX (pStartEntity); // if so, find out which index it is
   else
      start_index = 0; // else just start at the first one we find

   // cycle through all entities in game
   for (index = start_index; index < server.max_entities; index++)
   {
      pEdict = INDEXENT (index + 1); // get a pointer to this entity

      if (FNullEnt (pEdict))
         continue; // skip invalid edicts

      if ((OriginOf (pEdict) - v_center).Length () <= radius)
         return (pEdict); // return the first one we find that's inside the desired radius
   }

   return (NULL); // darn, not found anything
}


edict_t *FindEntityByString (edict_t *pStartEntity, const char *keyword, const char *value)
{
   // this function returns the first successive element in the list of entities that have
   // szValue in the szKeyword field of their entity variables entvars_t structure, starting the
   // search at entity pentStart. Also here, beware of orphan pointers !!!

   edict_t *pEdict;
   int start_index;
   int index;
   int keyword_type;

   // do we want to start the search at a particular entity ?
   if (!FNullEnt (pStartEntity))
      start_index = ENTINDEX (pStartEntity); // if so, find out which index it is
   else
      start_index = 0; // else just start at the first one we find

   // see what type of search it is
   if (strcmp ("classname", keyword) == 0)
      keyword_type = 1; // target
   else if (strcmp ("netname", keyword) == 0)
      keyword_type = 2; // netname
   else if (strcmp ("model", keyword) == 0)
      keyword_type = 3; // model
   else if (strcmp ("target", keyword) == 0)
      keyword_type = 4; // target
   else if (strcmp ("targetname", keyword) == 0)
      keyword_type = 5; // targetname

   // cycle through all entities in game
   for (index = start_index; index < server.max_entities; index++)
   {
      pEdict = INDEXENT (index + 1); // get a pointer to this entity

      if (FNullEnt (pEdict))
         continue; // skip invalid edicts

      // given the type of keyword it is, see if this entity matches the value we want
      if ((keyword_type == 1) && (strcmp (value, STRING (pEdict->v.classname)) == 0))
         return (pEdict); // return the first one we find
      else if ((keyword_type == 2) && (strcmp (value, STRING (pEdict->v.netname)) == 0))
         return (pEdict); // return the first one we find
      else if ((keyword_type == 3) && (strcmp (value, STRING (pEdict->v.model)) == 0))
         return (pEdict); // return the first one we find
      else if ((keyword_type == 4) && (strcmp (value, STRING (pEdict->v.target)) == 0))
         return (pEdict); // return the first one we find
      else if ((keyword_type == 5) && (strcmp (value, STRING (pEdict->v.targetname)) == 0))
         return (pEdict); // return the first one we find
   }

   return (NULL); // darn, not found anything
}


test_result_t PlayerTestLine (player_t *pPlayer, const Vector &vecStart, const Vector &vecEnd)
{
   // this function traces a line dot by dot, starting from vecStart in the direction of vecEnd,
   // ignoring monsters, and stops at the first obstacle encountered, returning the results
   // of the trace in the test_result_t structure.

   TraceResult tr;
   test_result_t results;

   // do the actual trace
   TRACE_HULL (vecStart,
               vecEnd,
               0x0001, // don't ignore glass, ignore monsters
               0, // point_hull
               pPlayer->pEntity,
               &tr);

   // convert the results
   results.fraction = tr.flFraction;
   results.pHit = tr.pHit;
   results.v_endposition = tr.vecEndPos;
   results.v_normal = tr.vecPlaneNormal;

   return (results); // and return them
}


test_result_t PlayerTestHull (player_t *pPlayer, const Vector &vecStart, const Vector &vecEnd, bool crouching_player)
{
   // this function traces a hull dot by dot, starting from vecStart in the direction of vecEnd,
   // ignoring monsters, and stops at the first obstacle encountered, returning the results
   // of the trace in the test_result_t structure. Hulls can be traced either of the size of a
   // crouching player, or of a normal body size, depending on the value of the crouching_player
   // boolean parameter.

   TraceResult tr;
   test_result_t results;
   int hull_type;

   if (crouching_player)
      hull_type = 3; // head_hull
   else
      hull_type = 1; // human_hull

   // do the actual trace
   TRACE_HULL (vecStart,
               vecEnd,
               0x0001, // don't ignore glass, ignore monsters
               hull_type,
               pPlayer->pEntity,
               &tr);

   // convert the results
   results.fraction = tr.flFraction;
   results.pHit = tr.pHit;
   results.v_endposition = tr.vecEndPos;
   results.v_normal = tr.vecPlaneNormal;

   return (results); // and return them
}


test_result_t TestVisibility (const Vector &vecStart, const Vector &vecEnd, edict_t *pTarget)
{
   // this function traces a line dot by dot, starting from vecStart in the direction of vecEnd,
   // NOT ignoring monsters, and stops at the first obstacle encountered, returning the results
   // of the trace in the test_result_t structure. The entity pointed to by pTarget is ignored
   // during the trace.

   TraceResult tr;
   test_result_t results;

   // do the actual trace
   TRACE_HULL (vecStart,
               vecEnd,
               0x0100, // ignore glass, don't ignore monsters
               0, // point_hull
               pTarget,
               &tr);

   // convert the results
   results.fraction = tr.flFraction;
   results.pHit = tr.pHit;
   results.v_endposition = tr.vecEndPos;
   results.v_normal = tr.vecPlaneNormal;

   return (results); // and return them
}


bool PlayerCanReach (player_t *pPlayer, Vector v_destination)
{
   // this function returns TRUE when the vector location specified by v_destination is reachable
   // by pPlayer, FALSE otherwise. It fires down test lines all the way to destination and check
   // whether the difference between two test lines is greater than the maximal jump height, in
   // which cases it assumes it is not reachable.

   test_result_t tr;
   float curr_height, last_height, distance;
   Vector v_check = pPlayer->v_origin;
   Vector v_direction = (v_destination - v_check).Normalize (); // 1 unit long

   // check for special case of both the bot and its destination being underwater...
   if ((POINT_CONTENTS (pPlayer->v_origin) == CONTENTS_WATER)
       && (POINT_CONTENTS (v_destination) == CONTENTS_WATER))
      return (TRUE); // if so, assume it's reachable

   // now check if distance to ground increases more than jump height
   // at points between source and destination...

   tr = PlayerTestHull (pPlayer, v_check, v_check + Vector (0, 0, -1000), TRUE);
   last_height = tr.fraction * 1000.0; // height from ground
   distance = (v_destination - v_check).Length (); // distance from goal

   // while we've not reached the goal
   while (distance > 40.0)
   {
      v_check = v_check + v_direction * 40.0; // move 40 units closer to the goal...

      tr = PlayerTestHull (pPlayer, v_check, v_check + Vector (0, 0, -1000), TRUE);
      curr_height = tr.fraction * 1000.0; // height from ground

      // is the difference between last and current height higher that the max jump height ?
      if ((last_height - curr_height) > 63.0)
         return (FALSE); // if so, assume it's NOT reachable

      last_height = curr_height; // backup current height
      distance = (v_destination - v_check).Length (); // update distance to goal
   }

   return (TRUE); // this point is reachable
}


bool PlayerAimIsOver (player_t *pPlayer, edict_t *pTarget)
{
   // this function returns TRUE if pPlayer's crosshair is on the entity pointed to by pTarget.

   TraceResult tr;

   // do the actual trace
   TRACE_HULL (pPlayer->v_eyeposition,
               pPlayer->v_eyeposition + pPlayer->v_forward * 10000,
               0x0100, // ignore_glass, don't ignore monsters
               0, // point_hull
               pPlayer->pEntity,
               &tr);

   // returns whether the hit entity is the target entity or not
   return (tr.pHit == pTarget);
}


bool IsAtHumanHeight (Vector v_location)
{
   // this function returns TRUE if the vector specified as the parameter (v_location) is at
   // human height from the floor, that is, not above the absmax of a player standing at this
   // location, FALSE otherwise. One TraceLine is involved.

   TraceResult tr;

   // trace down from v_location to see if it is at human standing height from the ground
   TRACE_HULL (v_location + Vector (0, 0, 1),
               v_location + Vector (0, 0, -73),
               0x0001, // don't ignore glass, ignore monsters
               0, // point_hull
               NULL,
               &tr);

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
   TRACE_HULL (v_location + Vector (0, 0, 1),
               v_location + Vector (0, 0, -9999),
               0x0001, // don't ignore glass, ignore monsters
               0, // point_hull
               NULL,
               &tr);

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
   TRACE_HULL (v_location + Vector (0, 0, 1),
               v_location + Vector (0, 0, -9999),
               0x0001, // don't ignore glass, ignore monsters
               0, // point_hull
               NULL,
               &tr);

   if (tr.flFraction < 1.0)
      return (tr.vecEndPos); // ground was found, return a lowered vector

   return (g_vecZero); // aargh, ground was not found !
}


/*inline */Vector GetGunPosition (edict_t *pEdict)
{
   // this expanded function returns the vector origin of a gun in the hands of a player entity
   // pointed to by pPlayer, assuming that player guns fire at player eye's position. This is
   // very often the case, indeed.

   return (pEdict->v.origin + pEdict->v.view_ofs);
}


/*inline */Vector OriginOf (edict_t *pEdict)
{
   // this expanded function returns the vector origin of an entity, assuming that any
   // entity that has a bounding box has its center at the center of the bounding box itself.

   // has this entity a bounding box ?
   if (pEdict->v.absmin != g_vecZero)
      return (pEdict->v.absmin + (pEdict->v.size * 0.5)); // then compute its center

   return (pEdict->v.origin); // else return its point-based origin
}


/*inline */Vector BottomOriginOf (edict_t *pEdict)
{
   // this expanded function returns the vector origin of the bottom of an entity, assuming that
   // any entity that has a bounding box has its center at the center of the bounding box itself.
   // Its bottom is then given by lowering this center to the mins height.

   // has this entity a bounding box ?
   if (pEdict->v.absmin != g_vecZero)
      return (Vector (pEdict->v.absmin.x + (pEdict->v.size.x * 0.5),
                      pEdict->v.absmin.y + (pEdict->v.size.y * 0.5),
                      pEdict->v.absmin.z)); // then compute its bottom center

   return (pEdict->v.origin); // else return its point-based origin
}


/*inline */Vector ReachableOriginOf (edict_t *pEdict)
{
   // this expanded function returns a reachable vector origin inside the bounding box of an
   // entity, reachable meaning that a walkface exists under the returned location.

   Vector v_origin;
   sector_t *pSector;
   walkface_t *pFace;
   walkface_t *pNearestFace;
   float distance;
   float nearest_distance;
   int face_index;
   int corner_index;

   v_origin = OriginOf (pEdict); // quick access to the entity's origin

   // does a walkface exist below that location ?
   if (WalkfaceAt (v_origin) != NULL)
      return (DropAtHumanHeight (v_origin)); // then return its human height origin

   // else we must find out the nearest walkface, which is almost always the one which has the
   // nearest corner from our destination

   // get the sector it is in the topology hashtable
   pSector = SectorUnder (v_origin);

   // start searching for the face which has the nearest corner
   pNearestFace = NULL;
   nearest_distance = 9999.0;

   // loop through all the face we know to be in this topological zone
   for (face_index = 0; face_index < pSector->faces_count; face_index++)
   {
      pFace = pSector->faces[face_index]; // quick access to the face

      // loop though the corners of this face...
      for (corner_index = 0; corner_index < pFace->corner_count; corner_index++)
      {
         // get this corner's distance from our origin
         distance = (pFace->v_corners[corner_index] - v_origin).Length ();

         // is this corner closer than the best one we've found so far ?
         if (distance < nearest_distance)
         {
            nearest_distance = distance; // update nearest distance
            pNearestFace = pFace; // and remember this face is the nearest
         }
      }
   }

   // have we found something ?
   if (pNearestFace != NULL)
      return (DropAtHumanHeight (WalkfaceCenterOf (pNearestFace))); // return this face's center

   // else if navigation debug level is high, let us know that we couldn't find anything
   if (DebugLevel.navigation > 1)
      ServerConsole_printf ("RACC: WalkfaceUnder() could not determine reachable walkface under (%.1f, %.1f, %.1f)\n", v_origin.x, v_origin.y, v_origin.z);

   return (v_origin); // and return the damn origin. *sigh*
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
   if (IsValidPlayer (pListenserverPlayer))
   {
      MESSAGE_BEGIN (MSG_ONE, GetUserMsgId ("SayText"), NULL, pListenserverPlayer->pEntity); // then print to HUD
      WRITE_BYTE (ENTINDEX (pListenserverPlayer->pEntity));
      WRITE_STRING (string);
      MESSAGE_END ();
   }
   else
      SERVER_PRINT (string); // else print to console

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

   SERVER_PRINT (string); // print to console

   // if developer mode is on...
   if (server.developer_level > 0)
      if (string[0] == '.')
         LogToFile (string); // also log this message to the logfile (not prefixing dots)
      else
         LogToFile ("(server console): %s", string); // also log this message to the logfile

   return (0); // printf() HAS to return a value
}


void TerminateOnError (const char *fmt, ...)
{
   // this function terminates the game because of an error and prints the message string
   // pointed to by fmt both in the server console and in a messagebox.

   va_list argptr;
   char string[1024];

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   vsprintf (string, fmt, argptr);
   va_end (argptr);

   // send the error message to console, messagebox, and also in the logfile for developers
   ServerConsole_printf ("FATAL ERROR: %s", string); // print to console
   MessageBox (0, string, "RACC - Error", NULL); // print to root-level message box

   // do we need to close the debug log file ?
   if (DebugLevel.fp != NULL)
      fclose (DebugLevel.fp); // close the file
   DebugLevel.fp = NULL;

   // is it better to just exit or shall we crash here to help debugging ?
   if (server.developer_level > 0)
      string[0] /= (string[0] = 0); // do a nice zero divide error for developers :)

   exit (1); // normal exit for non-developers (with error condition)
}


void InitLogFile (void)
{
   // this function reinitializes the log file, erasing any previous content. It is meant to be
   // called when the server boots up.

   char file_path[256];

   // build the log file full path
   sprintf (file_path, "%s/%s", GameConfig.racc_basedir, GameConfig.logfile_path);

   DebugLevel.fp = fopen (file_path, "w"); // open the log file in ASCII write mode, discard content

   // bomb out on error if unable to open the log file
   if (DebugLevel.fp == NULL)
      TerminateOnError ("InitLogFile(): unable to open log file (\"%s\")\n", GameConfig.logfile_path);

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

   // bomb out on error if log file not open yet
   if (DebugLevel.fp == NULL)
      TerminateOnError ("LogToFile(): log file not opened yet (\"%s\")\n", GameConfig.logfile_path);

   fprintf (DebugLevel.fp, string); // dump the string into the file
   return; // and return
}


void InitGameConfig (void)
{
   // this function is in charge of opening the game.cfg file in the knowledge/MOD directory,
   // and filling the game configuration database accordingly. Such a task should be performed
   // only once, and ought to be the first init function called by the bot DLL.

   FILE *fp;
   char filename[256];
   char line_buffer[256];
   char key_name[256];
   int index;
   int length;

   // first reset the game config structure
   memset (&GameConfig, 0, sizeof (GameConfig));

   // see if we're running a listen server or not (we do if hl.exe is running)
   if (GetModuleHandle ("hl.exe") != NULL)
      server.is_dedicated = FALSE; // hl.exe found, NOT a dedicated server
   else
      server.is_dedicated = TRUE; // either Linux hlserver or Win32 hlds.exe (dedicated)

   // get the running bot DLL path (Win32 stuff!!!)
   GetModuleFileName (GetModuleHandle ("racc.dll"), filename, 256);
   index = strlen (filename); // start scan position at end of string

   // we have now: /game_path/racc/release/MOD_NAME/release/racc.dll

   while ((index > 0) && (filename[index] != '\\') && (filename[index] != '/') && (filename[index] != ':'))
      index--; // go back one character in the string until a field separator is found
   filename[index] = 0; // and terminate the string here

   // we have now: /game_path/racc/release/MOD_NAME/release

   while ((index > 0) && (filename[index] != '\\') && (filename[index] != '/') && (filename[index] != ':'))
      index--; // go back one character in the string until a field separator is found
   filename[index] = 0; // and terminate the string here

   // we have now: /game_path/racc/release/MOD_NAME

   while ((index > 0) && (filename[index] != '\\') && (filename[index] != '/') && (filename[index] != ':'))
      index--; // go back one character in the string until a field separator is found
   strcpy (GameConfig.mod_name, &filename[index + 1]); // save away the MOD name...
   filename[index] = 0; // and terminate the string here

   // we have now: /game_path/racc/release

   while ((index > 0) && (filename[index] != '\\') && (filename[index] != '/') && (filename[index] != ':'))
      index--; // go back one character in the string until a field separator is found
   filename[index] = 0; // and terminate the string here

   // we have now: /game_path/racc... well, this is just the RACC base directory :)

   strcpy (GameConfig.racc_basedir, filename); // so just save it away

   // now open the "game.cfg" file in the knowledge directory, in the MOD's folder
   sprintf (filename, "%s/knowledge/%s/game.cfg", GameConfig.racc_basedir, GameConfig.mod_name);
   fp = fopen (filename, "r");
   if (fp == NULL)
      TerminateOnError ("RACC: Unable to figure out game base configuration (game.cfg file not found in %s)\n", filename);

   // for each line in the file...
   while (fgets (line_buffer, 256, fp) != NULL)
   {
      length = strlen (line_buffer); // get length of line
      if ((length > 0) && (line_buffer[length - 1] == '\n'))
         length--; // remove any final '\n'
      line_buffer[length] = 0; // terminate the string

      if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
          || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
         continue; // ignore line if void or commented

      // this line looks like a valid data line, figure out which key it is talking about
      sprintf (key_name, GetConfigKey (line_buffer));

      // given the specified key, fill in the corresponding data field from the file
      if (strcmp (key_name, "language") == 0)
         strcpy (GameConfig.language, GetConfigValue (line_buffer)); // it's the game language
      else if (strcmp (key_name, "logfile") == 0)
         strcpy (GameConfig.logfile_path, GetConfigValue (line_buffer)); // it's the log file
      else if (strcmp (key_name, "welcome_sound") == 0)
         strcpy (GameConfig.welcomesound_path, GetConfigValue (line_buffer)); // it's the welcome sound
      else if (strcmp (key_name, "distributor") == 0)
         strcpy (GameConfig.distributor_name, GetConfigValue (line_buffer)); // it's the distributor
      else if (strcmp (key_name, "url") == 0)
         strcpy (GameConfig.distributor_url, GetConfigValue (line_buffer)); // it's the distr. website URL

      else if (strcmp (key_name, "max_walk_speed") == 0)
         GameConfig.max_walk_speed = atof (GetConfigValue (line_buffer)); // max walk speed
      else if (strcmp (key_name, "max_fall_speed") == 0)
         GameConfig.max_safefall_speed = atof (GetConfigValue (line_buffer)); // max fall speed
      else if (strcmp (key_name, "walk_speed_factor") == 0)
         GameConfig.walk_speed_factor = atof (GetConfigValue (line_buffer)); // walk speed factor
      else if (strcmp (key_name, "max_hearing_distance") == 0)
         GameConfig.max_hearing_distance = atof (GetConfigValue (line_buffer)); // hearing distance

      else if (strcmp (key_name, "player_bb_width") == 0)
         GameConfig.bb_width = atof (GetConfigValue (line_buffer)); // player bounding box width
      else if (strcmp (key_name, "player_bb_depth") == 0)
         GameConfig.bb_depth = atof (GetConfigValue (line_buffer)); // player bounding box depth
      else if (strcmp (key_name, "player_bb_height") == 0)
         GameConfig.bb_height = atof (GetConfigValue (line_buffer)); // player bounding box height
      else if (strcmp (key_name, "standing_origin_height") == 0)
         GameConfig.standing_origin_height = atof (GetConfigValue (line_buffer)); // standing origin height
      else if (strcmp (key_name, "ducking_origin_height") == 0)
         GameConfig.ducking_origin_height = atof (GetConfigValue (line_buffer)); // standing origin height
   }

   fclose (fp); // finished parsing the file, close it

   // finished figuring out game base configuration, print out a nice notification message
   ServerConsole_printf ("RACC: Game base configuration learned from file\n");
   return;
}


void InitPlayerBones (void)
{
   // this function is in charge of opening the GameConfig.playerbones.cfg file in the knowledge/MOD
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
   // read the contents of the GameConfig.playerbones.cfg file. Such a task should be performed only once,
   // preferably at GameDLLInit(), since player bones aren't likely to change between each map.

   FILE *fp;
   char filename[256];
   char line_buffer[256];
   char bone_name[256];
   int length;

   // first reset the player bone numbers
   memset (&GameConfig.playerbones, 0, sizeof (GameConfig.playerbones));

   // open the "playerbones.cfg" file in the knowledge directory, in the MOD's folder
   sprintf (filename, "%s/knowledge/%s/playerbones.cfg", GameConfig.racc_basedir, GameConfig.mod_name);
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

      if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
          || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
         continue; // ignore line if void or commented

      // this line looks like a valid data line, figure out which bone it is talking about
      sprintf (bone_name, GetField (line_buffer, 0));

      // given the bone specified, fill in the corresponding bone number from the file
      if (strcmp (bone_name, "pelvis") == 0)
         GameConfig.playerbones.pelvis = atoi (GetField (line_buffer, 1)); // it's the pelvis
      else if (strcmp (bone_name, "spine") == 0)
         GameConfig.playerbones.spine = atoi (GetField (line_buffer, 1)); // it's the bottom of the spine
      else if (strcmp (bone_name, "spine1") == 0)
         GameConfig.playerbones.spine1 = atoi (GetField (line_buffer, 1)); // it's the 1st quarter of spine
      else if (strcmp (bone_name, "spine2") == 0)
         GameConfig.playerbones.spine2 = atoi (GetField (line_buffer, 1)); // it's the half-high spine
      else if (strcmp (bone_name, "spine3") == 0)
         GameConfig.playerbones.spine3 = atoi (GetField (line_buffer, 1)); // it's the 3rd quarter of spine
      else if (strcmp (bone_name, "neck") == 0)
         GameConfig.playerbones.neck = atoi (GetField (line_buffer, 1)); // it's the neck
      else if (strcmp (bone_name, "head") == 0)
         GameConfig.playerbones.head = atoi (GetField (line_buffer, 1)); // it's the head
      else if (strcmp (bone_name, "left_clavicle") == 0)
         GameConfig.playerbones.left_clavicle = atoi (GetField (line_buffer, 1)); // it's the left clavicle
      else if (strcmp (bone_name, "left_upperarm") == 0)
         GameConfig.playerbones.left_upperarm = atoi (GetField (line_buffer, 1)); // it's the left upperarm
      else if (strcmp (bone_name, "left_forearm") == 0)
         GameConfig.playerbones.left_forearm = atoi (GetField (line_buffer, 1)); // it's the left forearm
      else if (strcmp (bone_name, "left_hand") == 0)
         GameConfig.playerbones.left_hand = atoi (GetField (line_buffer, 1)); // it's the left hand
      else if (strcmp (bone_name, "left_finger0") == 0)
         GameConfig.playerbones.left_finger0 = atoi (GetField (line_buffer, 1)); // it's in the left thumb
      else if (strcmp (bone_name, "left_finger01") == 0)
         GameConfig.playerbones.left_finger01 = atoi (GetField (line_buffer, 1)); // it's the thumb extremity
      else if (strcmp (bone_name, "left_finger1") == 0)
         GameConfig.playerbones.left_finger1 = atoi (GetField (line_buffer, 1)); // it's in the left fingers
      else if (strcmp (bone_name, "left_finger11") == 0)
         GameConfig.playerbones.left_finger11 = atoi (GetField (line_buffer, 1)); // it's the fingers extremity
      else if (strcmp (bone_name, "left_thigh") == 0)
         GameConfig.playerbones.left_thigh = atoi (GetField (line_buffer, 1)); // it's the left thigh
      else if (strcmp (bone_name, "left_calf") == 0)
         GameConfig.playerbones.left_calf = atoi (GetField (line_buffer, 1)); // it's the left calf
      else if (strcmp (bone_name, "left_foot") == 0)
         GameConfig.playerbones.left_foot = atoi (GetField (line_buffer, 1)); // it's the foot extremity
      else if (strcmp (bone_name, "right_clavicle") == 0)
         GameConfig.playerbones.right_clavicle = atoi (GetField (line_buffer, 1)); // it's the right clavicle
      else if (strcmp (bone_name, "right_upperarm") == 0)
         GameConfig.playerbones.right_upperarm = atoi (GetField (line_buffer, 1)); // it's the right upperarm
      else if (strcmp (bone_name, "right_forearm") == 0)
         GameConfig.playerbones.right_forearm = atoi (GetField (line_buffer, 1)); // it's the right forearm
      else if (strcmp (bone_name, "right_hand") == 0)
         GameConfig.playerbones.right_hand = atoi (GetField (line_buffer, 1)); // it's the right hand
      else if (strcmp (bone_name, "right_finger0") == 0)
         GameConfig.playerbones.right_finger0 = atoi (GetField (line_buffer, 1)); // it's in the right thumb
      else if (strcmp (bone_name, "right_finger01") == 0)
         GameConfig.playerbones.right_finger01 = atoi (GetField (line_buffer, 1)); // it's the thumb extremity
      else if (strcmp (bone_name, "right_finger1") == 0)
         GameConfig.playerbones.right_finger1 = atoi (GetField (line_buffer, 1)); // it's in the right fingers
      else if (strcmp (bone_name, "right_finger11") == 0)
         GameConfig.playerbones.right_finger11 = atoi (GetField (line_buffer, 1)); // it's the fingers extremity
      else if (strcmp (bone_name, "right_thigh") == 0)
         GameConfig.playerbones.right_thigh = atoi (GetField (line_buffer, 1)); // it's the right thigh
      else if (strcmp (bone_name, "right_calf") == 0)
         GameConfig.playerbones.right_calf = atoi (GetField (line_buffer, 1)); // it's the right calf
      else if (strcmp (bone_name, "right_foot") == 0)
         GameConfig.playerbones.right_foot = atoi (GetField (line_buffer, 1)); // it's the foot extremity
   }

   fclose (fp); // finished parsing the file, close it

   // finished figuring out player morphology, print out a nice notification message
   ServerConsole_printf ("RACC: Player morphology learned from file\n");
   return;
}


void InitDefaultLikelevels (void)
{
   // this function is in charge of opening the likelevels.cfg file in the knowledge/MOD
   // directory, and filling the default likelevels accordingly. Such a task should be performed
   // only once, preferably at GameDLLInit(), since default likelevels aren't likely to change
   // between each map.

   FILE *fp;
   char filename[256];
   char line_buffer[256];
   char likelevel_name[256];
   int length;

   // first reset the default likelevels
   default_likelevels.ladder = 1; // reachability #1: this bot's like level of ladders
   default_likelevels.falledge = 1; // reachability #2: this bot's like level of falls
   default_likelevels.elevator = 1; // reachability #3: this bot's like level about elevators
   default_likelevels.platform = 1; // reachability #4: this bot's like level of bobbing platforms
   default_likelevels.conveyor = 1; // reachability #5: this bot's like level of conveyors (belts and travolators)
   default_likelevels.train = 1; // reachability #6: this bot's like level of trains
   default_likelevels.longjump = 1; // reachability #7: this bot's like level of long jump modules
   default_likelevels.swim = 1; // reachability #8: this bot's like level of deep water
   default_likelevels.teleporter = 1; // reachability #9: this bot's like level of teleporters
   default_likelevels.jump = 1; // reachability #10: this bot's like level of jumps
   default_likelevels.crouch = 1; // reachability #11: this bot's like level of crouched passages
   default_likelevels.unknown1 = 1; // reachability #12
   default_likelevels.unknown2 = 1; // reachability #13
   default_likelevels.unknown3 = 1; // reachability #14
   default_likelevels.unknown4 = 1; // reachability #15
   default_likelevels.unknown5 = 1; // reachability #16

   // open the "likelevels.cfg" file in the knowledge directory, in the MOD's folder
   sprintf (filename, "%s/knowledge/%s/likelevels.cfg", GameConfig.racc_basedir, GameConfig.mod_name);
   fp = fopen (filename, "r");
   if (fp == NULL)
   {
      ServerConsole_printf ("RACC: Unable to read default likelevels (likelevels.cfg file not found)\n");
      return; // if the file doesn't exist, then the bot will assume each likelevel is 1
   }

   // for each line in the file...
   while (fgets (line_buffer, 256, fp) != NULL)
   {
      length = strlen (line_buffer); // get length of line
      if ((length > 0) && (line_buffer[length - 1] == '\n'))
         length--; // remove any final '\n'
      line_buffer[length] = 0; // terminate the string

      if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
          || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
         continue; // ignore line if void or commented

      // this line looks like a valid data line, figure out which likelevel it is talking about
      sprintf (likelevel_name, GetField (line_buffer, 0));

      // given the bone specified, fill in the corresponding bone number from the file
      if (strcmp (likelevel_name, "ladder") == 0)
         default_likelevels.ladder = atoi (GetField (line_buffer, 1)); // ladder
      else if (strcmp (likelevel_name, "falledge") == 0)
         default_likelevels.falledge = atoi (GetField (line_buffer, 1)); // falledge
      else if (strcmp (likelevel_name, "elevator") == 0)
         default_likelevels.elevator = atoi (GetField (line_buffer, 1)); // elevator
      else if (strcmp (likelevel_name, "platform") == 0)
         default_likelevels.platform = atoi (GetField (line_buffer, 1)); // platform
      else if (strcmp (likelevel_name, "conveyor") == 0)
         default_likelevels.conveyor = atoi (GetField (line_buffer, 1)); // conveyor
      else if (strcmp (likelevel_name, "train") == 0)
         default_likelevels.train = atoi (GetField (line_buffer, 1)); // train
      else if (strcmp (likelevel_name, "longjump") == 0)
         default_likelevels.longjump = atoi (GetField (line_buffer, 1)); // longjump
      else if (strcmp (likelevel_name, "swim") == 0)
         default_likelevels.swim = atoi (GetField (line_buffer, 1)); // swim
      else if (strcmp (likelevel_name, "teleporter") == 0)
         default_likelevels.teleporter = atoi (GetField (line_buffer, 1)); // teleporter
      else if (strcmp (likelevel_name, "jump") == 0)
         default_likelevels.jump = atoi (GetField (line_buffer, 1)); // jump
      else if (strcmp (likelevel_name, "crouch") == 0)
         default_likelevels.crouch = atoi (GetField (line_buffer, 1)); // crouch
      else if (strcmp (likelevel_name, "unknown1") == 0)
         default_likelevels.unknown1 = atoi (GetField (line_buffer, 1)); // unknown1
      else if (strcmp (likelevel_name, "unknown2") == 0)
         default_likelevels.unknown2 = atoi (GetField (line_buffer, 1)); // unknown2
      else if (strcmp (likelevel_name, "unknown3") == 0)
         default_likelevels.unknown3 = atoi (GetField (line_buffer, 1)); // unknown3
      else if (strcmp (likelevel_name, "unknown4") == 0)
         default_likelevels.unknown4 = atoi (GetField (line_buffer, 1)); // unknown4
      else if (strcmp (likelevel_name, "unknown5") == 0)
         default_likelevels.unknown5 = atoi (GetField (line_buffer, 1)); // unknown5
   }

   fclose (fp); // finished parsing the file, close it

   // finished figuring out player morphology, print out a nice notification message
   ServerConsole_printf ("RACC: Default likelevels learned from file\n");
   return;
}


void LoadBotProfiles (void)
{
   // this function is called each time a new server starts, when it has been just put online. It
   // fills one by one the bot personality slots with the info read from the "profiles.cfg" file
   // that holds the bots names, skin, logo and skill of the bots.

   // TODO: logo doesn't work

   FILE *fp;
   char path[256];
   char line_buffer[256];
   int length, index;
   char field[256];

   // reset the bots array before they start to connect
   for (index = 0; index < server.max_clients; index++)
      memset (&players[index].Bot, 0, sizeof (players[index].Bot));

   // had we already allocated memory space for bot profiles ?
   if (profiles != NULL)
      free (profiles); // if so, free it
   profiles = NULL;
   profile_count = 0; // and reset the profiles count to zero

   // mallocate the minimal space for profiles
   profiles = (profile_t *) malloc (sizeof (profile_t));
   if (profiles == NULL)
      TerminateOnError ("LoadBotProfiles(): malloc() failure for bot profiles on %d bytes (out of memory ?)\n", sizeof (profile_t));

   // read the bots names from the file
   sprintf (path, "%s/profiles.cfg", GameConfig.racc_basedir);
   fp = fopen (path, "r"); // opens file readonly
   if (fp != NULL)
   {
      // read line per line
      while (fgets (line_buffer, 256, fp) != NULL)
      {
         length = strlen (line_buffer); // get length of line
         if (length > 0)
            if (line_buffer[length - 1] == '\n')
               length--; // remove any final '\n'
         line_buffer[length] = 0; // terminate the string

         if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
             || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
            continue; // ignore line if void or commented

         // we have another profile line, must allocate some space more
         profiles = (profile_t *) realloc (profiles, (profile_count + 1) * sizeof (profile_t));
         if (profiles == NULL)
            TerminateOnError ("LoadBotProfiles(): realloc() failure for bot profiles on %d bytes (out of memory ?)\n", (profile_count + 1) * sizeof (profile_t));

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
         field[32] = 0; // truncate it in case it exceeds the maximal length
         sprintf (profiles[profile_count].nationality, field); // and store it in the array

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
   }
   else
      ServerConsole_printf ("RACC: WARNING: Unable to find profiles.cfg, no profiles loaded!\n");

   ServerConsole_printf ("RACC: %d profiles loaded\n", profile_count); // print how many we found

   // profiles are loaded, now ensure the max_bots variable is in bounds
   if (((profile_count > 0) && (server.max_bots > profile_count))
       || (server.max_bots < 0) || (server.max_bots > 31))
      server.max_bots = profile_count; // adjust max_bots to the bot list count

   return; // finished
}


void InitLanguages (void)
{
   // this function is called each time a new server starts, as soon as it is online. It fills
   // fills the bot languages database with the info read from the "racc/talk" directory
   // structure.

   // TODO: make directory search OS-independent

   FILE *fp;
   HANDLE hSampleFile;
   HANDLE hLanguageDir;
   WIN32_FIND_DATA pSampleFileData;
   WIN32_FIND_DATA pLanguageDirData;
   HRESULT samplefile_search_result = TRUE;
   HRESULT languagedir_search_result = TRUE;
   char path[256];
   char line_buffer[256];

   // had we already allocated memory space for bot languages ?
   if (languages != NULL)
      free (languages); // if so, free it
   languages = NULL;
   language_count = 0; // and reset the languages count to zero

   // mallocate the minimal space for languages
   languages = (bot_language_t *) malloc (sizeof (bot_language_t));
   if (languages == NULL)
      TerminateOnError ("InitLanguages(): malloc() failure for bot languages on %d bytes (out of memory ?)\n", sizeof (bot_language_t));

   // see how many language subdirectories there are in racc/talk

   // find a first one...
   languagedir_search_result = TRUE; // reset the error flag to its initial state
   sprintf (path, "%s/talk/*", GameConfig.racc_basedir);
   hLanguageDir = FindFirstFile (path, &pLanguageDirData);
   if (hLanguageDir == INVALID_HANDLE_VALUE)
      languagedir_search_result = FALSE; // if nonexistent, then stop

   // now for each of them...
   while (languagedir_search_result)
   {
      // is it NOT a directory OR is it "." or ".." ?
      if (!(pLanguageDirData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
          || (strcmp (pLanguageDirData.cFileName, ".") == 0)
          || (strcmp (pLanguageDirData.cFileName, "..") == 0))
      {
         languagedir_search_result = FindNextFile (hLanguageDir, &pLanguageDirData); // skip it
         continue; // and go and find a handle on the next item
      }

      // we have another language dir, must allocate some space more
      languages = (bot_language_t *) realloc (languages, (language_count + 1) * sizeof (bot_language_t));
      if (languages == NULL)
         TerminateOnError ("InitLanguages(): realloc() failure for bot languages on %d bytes (out of memory ?)\n", (language_count + 1) * sizeof (bot_language_t));

      // store away the language it is
      strcpy (languages[language_count].language, pLanguageDirData.cFileName);

      // Build affirmative messages array
      languages[language_count].text.affirmative_count = 0; // first reset the count
      sprintf (path, "%s/talk/%s/affirmative.txt", GameConfig.racc_basedir, languages[language_count].language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((languages[language_count].text.affirmative_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
                || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (languages[language_count].text.affirmative[languages[language_count].text.affirmative_count], line_buffer); // we have a valid line
            languages[language_count].text.affirmative_count++;
         }
         fclose (fp);
      }

      // Build bye messages array
      languages[language_count].text.bye_count = 0; // first reset the count
      sprintf (path, "%s/talk/%s/bye.txt", GameConfig.racc_basedir, languages[language_count].language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((languages[language_count].text.bye_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
                || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (languages[language_count].text.bye[languages[language_count].text.bye_count], line_buffer); // we have a valid line
            languages[language_count].text.bye_count++;
         }
         fclose (fp);
      }

      // Build cant messages array
      languages[language_count].text.cant_count = 0; // first reset the count
      sprintf (path, "%s/talk/%s/cantfollow.txt", GameConfig.racc_basedir, languages[language_count].language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((languages[language_count].text.cant_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
                || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (languages[language_count].text.cant[languages[language_count].text.cant_count], line_buffer); // we have a valid line
            languages[language_count].text.cant_count++;
         }
         fclose (fp);
      }

      // Build follow messages array
      languages[language_count].text.follow_count = 0; // first reset the count
      sprintf (path, "%s/talk/%s/follow.txt", GameConfig.racc_basedir, languages[language_count].language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((languages[language_count].text.follow_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
                || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (languages[language_count].text.follow[languages[language_count].text.follow_count], line_buffer); // we have a valid line
            languages[language_count].text.follow_count++;
         }
         fclose (fp);
      }

      // Build hello messages array
      languages[language_count].text.hello_count = 0; // first reset the count
      sprintf (path, "%s/talk/%s/hello.txt", GameConfig.racc_basedir, languages[language_count].language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((languages[language_count].text.hello_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
                || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (languages[language_count].text.hello[languages[language_count].text.hello_count], line_buffer); // we have a valid line
            languages[language_count].text.hello_count++;
         }
         fclose (fp);
      }

      // Build help messages array
      languages[language_count].text.help_count = 0; // first reset the count
      sprintf (path, "%s/talk/%s/help.txt", GameConfig.racc_basedir, languages[language_count].language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((languages[language_count].text.help_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
                || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (languages[language_count].text.help[languages[language_count].text.help_count], line_buffer); // we have a valid line
            languages[language_count].text.help_count++;
         }
         fclose (fp);
      }

      // Build idle messages array
      languages[language_count].text.idle_count = 0; // first reset the count
      sprintf (path, "%s/talk/%s/idle.txt", GameConfig.racc_basedir, languages[language_count].language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((languages[language_count].text.idle_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
                || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (languages[language_count].text.idle[languages[language_count].text.idle_count], line_buffer); // we have a valid line
            languages[language_count].text.idle_count++;
         }
         fclose (fp);
      }

      // Build laugh messages array
      languages[language_count].text.laugh_count = 0; // first reset the count
      sprintf (path, "%s/talk/%s/laugh.txt", GameConfig.racc_basedir, languages[language_count].language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((languages[language_count].text.laugh_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
                || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (languages[language_count].text.laugh[languages[language_count].text.laugh_count], line_buffer); // we have a valid line
            languages[language_count].text.laugh_count++;
         }
         fclose (fp);
      }

      // Build negative messages array
      languages[language_count].text.negative_count = 0; // first reset the count
      sprintf (path, "%s/talk/%s/negative.txt", GameConfig.racc_basedir, languages[language_count].language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((languages[language_count].text.negative_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
                || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (languages[language_count].text.negative[languages[language_count].text.negative_count], line_buffer); // we have a valid line
            languages[language_count].text.negative_count++;
         }
         fclose (fp);
      }

      // Build stay messages array
      languages[language_count].text.stay_count = 0; // first reset the count
      sprintf (path, "%s/talk/%s/stay.txt", GameConfig.racc_basedir, languages[language_count].language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((languages[language_count].text.stay_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
                || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (languages[language_count].text.stay[languages[language_count].text.stay_count], line_buffer); // we have a valid line
            languages[language_count].text.stay_count++;
         }
         fclose (fp);
      }

      // Build stop messages array
      languages[language_count].text.stop_count = 0; // first reset the count
      sprintf (path, "%s/talk/%s/stop.txt", GameConfig.racc_basedir, languages[language_count].language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((languages[language_count].text.stop_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
                || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (languages[language_count].text.stop[languages[language_count].text.stop_count], line_buffer); // we have a valid line
            languages[language_count].text.stop_count++;
         }
         fclose (fp);
      }

      // Build whine messages array
      languages[language_count].text.whine_count = 0; // first reset the count
      sprintf (path, "%s/talk/%s/whine.txt", GameConfig.racc_basedir, languages[language_count].language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((languages[language_count].text.whine_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
                || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (languages[language_count].text.whine[languages[language_count].text.whine_count], line_buffer); // we have a valid line
            languages[language_count].text.whine_count++;
         }
         fclose (fp);
      }

      // look for any affirmative voice samples
      languages[language_count].audio.affirmative_count = 0; // first reset the count
      samplefile_search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/talk/%s/affirmative*.wav", GameConfig.racc_basedir, languages[language_count].language);
      hSampleFile = FindFirstFile (path, &pSampleFileData);
      if (hSampleFile == INVALID_HANDLE_VALUE)
         samplefile_search_result = FALSE; // if nonexistent, then stop
      while (samplefile_search_result) // for each element found...
      {
         languages[language_count].audio.affirmative_count++; // increment the count
         samplefile_search_result = FindNextFile (hSampleFile, &pSampleFileData); // go and find a handle on the next file
      }
      FindClose (hSampleFile); // close the file search

      // look for any alert voice samples
      languages[language_count].audio.alert_count = 0; // first reset the count
      samplefile_search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/talk/%s/alert*.wav", GameConfig.racc_basedir, languages[language_count].language);
      hSampleFile = FindFirstFile (path, &pSampleFileData);
      if (hSampleFile == INVALID_HANDLE_VALUE)
         samplefile_search_result = FALSE; // if nonexistent, then stop
      while (samplefile_search_result) // for each element found...
      {
         languages[language_count].audio.alert_count++; // increment the count
         samplefile_search_result = FindNextFile (hSampleFile, &pSampleFileData); // go and find a handle on the next file
      }
      FindClose (hSampleFile); // close the file search

      // look for any attacking voice samples
      languages[language_count].audio.attacking_count = 0; // first reset the count
      samplefile_search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/talk/%s/attacking*.wav", GameConfig.racc_basedir, languages[language_count].language);
      hSampleFile = FindFirstFile (path, &pSampleFileData);
      if (hSampleFile == INVALID_HANDLE_VALUE)
         samplefile_search_result = FALSE; // if nonexistent, then stop
      while (samplefile_search_result) // for each element found...
      {
         languages[language_count].audio.attacking_count++; // increment the count
         samplefile_search_result = FindNextFile (hSampleFile, &pSampleFileData); // go and find a handle on the next file
      }
      FindClose (hSampleFile); // close the file search

      // look for any firstspawn voice samples
      languages[language_count].audio.firstspawn_count = 0; // first reset the count
      samplefile_search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/talk/%s/firstspawn*.wav", GameConfig.racc_basedir, languages[language_count].language);
      hSampleFile = FindFirstFile (path, &pSampleFileData);
      if (hSampleFile == INVALID_HANDLE_VALUE)
         samplefile_search_result = FALSE; // if nonexistent, then stop
      while (samplefile_search_result) // for each element found...
      {
         languages[language_count].audio.firstspawn_count++; // increment the count
         samplefile_search_result = FindNextFile (hSampleFile, &pSampleFileData); // go and find a handle on the next file
      }
      FindClose (hSampleFile); // close the file search

      // look for any inposition voice samples
      languages[language_count].audio.inposition_count = 0; // first reset the count
      samplefile_search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/talk/%s/inposition*.wav", GameConfig.racc_basedir, languages[language_count].language);
      hSampleFile = FindFirstFile (path, &pSampleFileData);
      if (hSampleFile == INVALID_HANDLE_VALUE)
         samplefile_search_result = FALSE; // if nonexistent, then stop
      while (samplefile_search_result) // for each element found...
      {
         languages[language_count].audio.inposition_count++; // increment the count
         samplefile_search_result = FindNextFile (hSampleFile, &pSampleFileData); // go and find a handle on the next file
      }
      FindClose (hSampleFile); // close the file search

      // look for any negative voice samples
      languages[language_count].audio.negative_count = 0; // first reset the count
      samplefile_search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/talk/%s/negative*.wav", GameConfig.racc_basedir, languages[language_count].language);
      hSampleFile = FindFirstFile (path, &pSampleFileData);
      if (hSampleFile == INVALID_HANDLE_VALUE)
         samplefile_search_result = FALSE; // if nonexistent, then stop
      while (samplefile_search_result) // for each element found...
      {
         languages[language_count].audio.negative_count++; // increment the count
         samplefile_search_result = FindNextFile (hSampleFile, &pSampleFileData); // go and find a handle on the next file
      }
      FindClose (hSampleFile); // close the file search

      // look for any report voice samples
      languages[language_count].audio.report_count = 0; // first reset the count
      samplefile_search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/talk/%s/report*.wav", GameConfig.racc_basedir, languages[language_count].language);
      hSampleFile = FindFirstFile (path, &pSampleFileData);
      if (hSampleFile == INVALID_HANDLE_VALUE)
         samplefile_search_result = FALSE; // if nonexistent, then stop
      while (samplefile_search_result) // for each element found...
      {
         languages[language_count].audio.report_count++; // increment the count
         samplefile_search_result = FindNextFile (hSampleFile, &pSampleFileData); // go and find a handle on the next file
      }
      FindClose (hSampleFile); // close the file search

      // look for any reporting voice samples
      languages[language_count].audio.reporting_count = 0; // first reset the count
      samplefile_search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/talk/%s/reporting*.wav", GameConfig.racc_basedir, languages[language_count].language);
      hSampleFile = FindFirstFile (path, &pSampleFileData);
      if (hSampleFile == INVALID_HANDLE_VALUE)
         samplefile_search_result = FALSE; // if nonexistent, then stop
      while (samplefile_search_result) // for each element found...
      {
         languages[language_count].audio.reporting_count++; // increment the count
         samplefile_search_result = FindNextFile (hSampleFile, &pSampleFileData); // go and find a handle on the next file
      }
      FindClose (hSampleFile); // close the file search

      // look for any seegrenade voice samples
      languages[language_count].audio.seegrenade_count = 0; // first reset the count
      samplefile_search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/talk/%s/seegrenade*.wav", GameConfig.racc_basedir, languages[language_count].language);
      hSampleFile = FindFirstFile (path, &pSampleFileData);
      if (hSampleFile == INVALID_HANDLE_VALUE)
         samplefile_search_result = FALSE; // if nonexistent, then stop
      while (samplefile_search_result) // for each element found...
      {
         languages[language_count].audio.seegrenade_count++; // increment the count
         samplefile_search_result = FindNextFile (hSampleFile, &pSampleFileData); // go and find a handle on the next file
      }
      FindClose (hSampleFile); // close the file search

      // look for any takingdamage voice samples
      languages[language_count].audio.takingdamage_count = 0; // first reset the count
      samplefile_search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/talk/%s/takingdamage*.wav", GameConfig.racc_basedir, languages[language_count].language);
      hSampleFile = FindFirstFile (path, &pSampleFileData);
      if (hSampleFile == INVALID_HANDLE_VALUE)
         samplefile_search_result = FALSE; // if nonexistent, then stop
      while (samplefile_search_result) // for each element found...
      {
         languages[language_count].audio.takingdamage_count++; // increment the count
         samplefile_search_result = FindNextFile (hSampleFile, &pSampleFileData); // go and find a handle on the next file
      }
      FindClose (hSampleFile); // close the file search

      // look for any throwgrenade voice samples
      languages[language_count].audio.throwgrenade_count = 0; // first reset the count
      samplefile_search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/talk/%s/throwgrenade*.wav", GameConfig.racc_basedir, languages[language_count].language);
      hSampleFile = FindFirstFile (path, &pSampleFileData);
      if (hSampleFile == INVALID_HANDLE_VALUE)
         samplefile_search_result = FALSE; // if nonexistent, then stop
      while (samplefile_search_result) // for each element found...
      {
         languages[language_count].audio.throwgrenade_count++; // increment the count
         samplefile_search_result = FindNextFile (hSampleFile, &pSampleFileData); // go and find a handle on the next file
      }
      FindClose (hSampleFile); // close the file search

      // look for any victory voice samples
      languages[language_count].audio.victory_count = 0; // first reset the count
      samplefile_search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "%s/talk/%s/victory*.wav", GameConfig.racc_basedir, languages[language_count].language);
      hSampleFile = FindFirstFile (path, &pSampleFileData);
      if (hSampleFile == INVALID_HANDLE_VALUE)
         samplefile_search_result = FALSE; // if nonexistent, then stop
      while (samplefile_search_result) // for each element found...
      {
         languages[language_count].audio.victory_count++; // increment the count
         samplefile_search_result = FindNextFile (hSampleFile, &pSampleFileData); // go and find a handle on the next file
      }
      FindClose (hSampleFile); // close the file search

      language_count++; // we found one language more
      languagedir_search_result = FindNextFile (hLanguageDir, &pLanguageDirData); // go and find a handle on the next dir
   }

   FindClose (hLanguageDir); // close the directory search
   ServerConsole_printf ("RACC: %d languages loaded\n", language_count); // print how many we found

   return; // finished
}


void PrecacheStuff (void)
{
   // this is the function that precaches the stuff we need by the server side, such as the
   // entity model for the entities used in the fakeclient illumination bugfix, and the sprites
   // used for displaying beams in debug mode.

   PRECACHE_MODEL ("models/mechgibs.mdl"); // used to create fake entities
   beam_model = PRECACHE_MODEL ("sprites/lgtning.spr"); // used to trace beams

   return;
}


void SpawnDoor (edict_t *pDoorEntity)
{
   edict_t *pFakeEntity = NULL;

   if (strncmp (STRING (pDoorEntity->v.netname), "secret", 6) == 0)
      return; // skip secret doors

   pFakeEntity = CREATE_NAMED_ENTITY (MAKE_STRING ("info_target")); // create door origin entity
   MDLL_Spawn (pFakeEntity); // spawn it
   SET_ORIGIN (pFakeEntity, OriginOf (pDoorEntity)); // same origin as door, obviously
   pFakeEntity->v.takedamage = DAMAGE_NO; // doesn't allow it to take damage
   pFakeEntity->v.solid = SOLID_NOT; // make it invisible
   pFakeEntity->v.movetype = MOVETYPE_NOCLIP; // no clip
   pFakeEntity->v.classname = MAKE_STRING ("door_origin"); // sets a name for it
   pFakeEntity->v.rendermode = kRenderNormal; // normal rendering mode
   pFakeEntity->v.renderfx = kRenderFxNone; // no special FX
   pFakeEntity->v.renderamt = 0; // ???
   pFakeEntity->v.owner = pDoorEntity; // sets the real door as the owner of the origin entity
   SET_MODEL (pFakeEntity, "models/mechgibs.mdl"); // sets it a model

   return; // done, door is safe
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

   if (FNullEnt (pFakeClient))
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

      MDLL_ClientCommand (pFakeClient); // tell now the MOD DLL to execute this ClientCommand...
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

   while ((length > 0) && ((string[length - 1] == '\n') || (string[length - 1] == '\r')))
      length--; // discard trailing newlines

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
      while ((index < length) && (config_string[index] != ' ') && (config_string[index] != '\t')
                              && (config_string[index] != 0x0D) && (config_string[index] != '\n'))
         index++; // reach end of value
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
   // have been registered by the game DLL into the engine at game start.

   return (GET_USER_MSG_ID (PLID, msg_name, NULL)); // metamod provides a handy facility for that
}


const char *GetUserMsgName (int msg_type)
{
   // this function is the counterpart of GetUserMsgId(), in that it returns the user message
   // name of the recorded message whose ID number is msg_type.

   return (GET_USER_MSG_NAME (PLID, msg_type, NULL)); // metamod provides a handy facility for that
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

   if (server.msec_method == METHOD_DEBUG)
      server.msecval = 2; // fixed msec value

   // Rich's method for computing the msec value
   else if (server.msec_method == METHOD_WHITEHOUSE)
   {
      if (server.msecdel <= server.time)
      {
         if (server.msecnum > 0)
            server.msecval = 500.0 / server.msecnum; // Rich was using 450 instead of 500

         server.msecdel = server.time + 0.5; // next check in half a second
         server.msecnum = 0;
      }
      else
         server.msecnum++;

      if (server.msecval < 1)
         server.msecval = 1; // don't allow the msec delay to be null
      else if (server.msecval > 100)
         server.msecval = 100; // don't allow it to last longer than 100 milliseconds either
   }

   // Leon's method for computing the msec value
   else if (server.msec_method == METHOD_HARTWIG)
   {
      server.msecval = (int) ((server.time - server.previous_time) * 1000);
      if (server.msecval > 255)
         server.msecval = 255;
   }

   // Tobias's method for computing the msec value
   else if (server.msec_method == METHOD_HEIMANN)
   {
      if ((server.msecdel + server.msecnum / 1000) < server.time - 0.5)
      {
         server.msecdel = server.time - 0.05; // after pause
         server.msecnum = 0;
      }

      if (server.msecdel > server.time)
      {
         server.msecdel = server.time - 0.05; // after map changes
         server.msecnum = 0;
      }

      server.msecval = (server.time - server.msecdel) * 1000 - server.msecnum; // optimal msec value since start of 1 sec period
      server.msecnum = (server.time - server.msecdel) * 1000; // value ve have to add to reach optimum

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
   }

   return; // estimation completed, the estimated value is held in the msecval global variable.
}


void APlayerHasConnected (player_t *pPlayer)
{
   // We can here keep track of both bots and players
   // counts on occurence, since bots connect the server just like the way normal client do,
   // and their third party bot flag is already supposed to be set then. If it's a bot which
   // is connecting, we also have to awake its brain(s) by reading them from the disk.

   // is this client a bot ?
   if (pPlayer->is_racc_bot)
   {
      BotHALLoadBrain (pPlayer); // load this bot's HAL brain
      BotNavLoadBrain (pPlayer); // load this bot's nav brain
      BotInitPathMachine (pPlayer); // initialize this bot's pathmachine
      bot_count++; // increment the bot count as we are certain now this bot is connected
   }

   // reset all his player structure BUT the bot structure
   pPlayer->is_alive = FALSE;
   pPlayer->v_origin = g_vecZero;
   pPlayer->v_eyeposition = g_vecZero;
   pPlayer->v_forward = g_vecZero;
   pPlayer->v_right = g_vecZero;
   pPlayer->v_up = g_vecZero;
   pPlayer->angles = g_vecZero;
   pPlayer->v_angle = g_vecZero;
   pPlayer->pFaceAtFeet = NULL;
   pPlayer->face_reachability = 0;
   pPlayer->step_sound_time = 0;
   pPlayer->proximityweapon_swing_time = 0;
   memset (&pPlayer->tr, 0, sizeof (pPlayer->tr));

   pPlayer->welcome_time = server.time + 3.0; // send welcome message in 3 sec
   pPlayer->is_connected = TRUE; // this player is connected
   player_count++; // increment the player count as we are certain now this client is connected

   // are we reaching the max player count with this client ?
   if (player_count == server.max_clients)
      server.bot_check_time = server.time; // see if we need to disconnect a bot to allow future connections

   return;
}


void APlayerHasDisconnected (player_t *pPlayer)
{
   // This function is called when the main code loop detected that a client just disconnected.
   // It's time to update the bots and players counts, and in case it's a bot, to back its
   // brain(s) up to disk. We also try to notice when a listenserver client disconnects, so as
   // to reset his entity pointer for safety. There are still a few server frames to go once a
   // listen server client disconnects, and we don't want to send him any sort of message then.
   // It is the more or less symmetrical pending function to AClientHasConnected().

   // was this client a bot ?
   if (pPlayer->is_racc_bot)
   {
      BotShutdownPathMachine (pPlayer); // shutdown our bot's pathmachine
      BotNavSaveBrain (pPlayer); // save our bot's nav brain
      BotHALSaveBrain (pPlayer); // save our bot's HAL brain
      bot_count--; // decrement the bot count as we know this bot is disconnecting
   }

   // reset his player structure completely, INCLUDING the bot structure
   memset (pPlayer, 0, sizeof (*pPlayer));

   pPlayer->welcome_time = 0; // don't send any welcome message anymore
   pPlayer->is_connected = FALSE; // this player is NOT connected anymore
   player_count--; // decrement the player count as we know this client is disconnected

   return;
}


void SendWelcomeMessage (player_t *pPlayer)
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
   static char welcome_sound_path[256];

   if (!IsValidPlayer (pPlayer))
      return; // reliability check

   if (pPlayer->is_racc_bot)
      return; // also don't send messages to bots

   // is today my birthday ? :D
   if (IsMyBirthday ())
   {
      // w00t, it's my birthday!!! send the "special birthday" welcome message to this client
      MESSAGE_BEGIN (MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, pPlayer->pEntity);
      WRITE_BYTE (TE_TEXTMESSAGE);
      WRITE_BYTE (255); // channel
      WRITE_SHORT (-8192); // x coordinates * 8192
      WRITE_SHORT (-8192); // y coordinates * 8192
      WRITE_BYTE (2); // effect (fade in/out)
      WRITE_BYTE (255); // initial RED
      WRITE_BYTE (255); // initial GREEN
      WRITE_BYTE (255); // initial BLUE
      WRITE_BYTE (1); // initial ALPHA
      WRITE_BYTE (RandomLong (0, 255)); // effect RED
      WRITE_BYTE (RandomLong (0, 255)); // effect GREEN
      WRITE_BYTE (RandomLong (0, 255)); // effect BLUE
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
                             "Send him an email to pm@bots-united.com", racc_welcometext);

      WRITE_STRING (welcome_text); // it's my birthday!!
      MESSAGE_END (); // end

      // play the "special birthday" welcome sound on this client (hehe)
      CLIENT_COMMAND (pPlayer->pEntity, "play barney/coldone.wav\n");
   }

   // looks like it's just a normal, boring day
   else
   {
      // send the welcome message to this client
      MESSAGE_BEGIN (MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, pPlayer->pEntity);
      WRITE_BYTE (TE_TEXTMESSAGE);
      WRITE_BYTE (255); // channel
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
      if (FileExists (GameConfig.welcomesound_path))
         CLIENT_COMMAND (pPlayer->pEntity, "play %s\n", GameConfig.welcomesound_path); // normal welcome sound
      else
         CLIENT_COMMAND (pPlayer->pEntity, "play barney/guyresponsible.wav\n"); // fallback sound
   }
}


void MakeVersion (void)
{
   // this function builds the version number string and the welcome message string out of it.
   // The version number string is a 8 digit string describing the date at which the program was
   // compiled, in which the digits are arranged under the form "YYYYMMDD" (year, month, day).
   // This order allow earlier dates to be represented by a smaller 8-digit number and later
   // dates to be represented by greater ones. We use the __DATE__ standard C macro to get the
   // program compile date string ; this string being under the form "Mmm DD, YYYY" such as in
   // "Mar 4, 2003", we need to process it in order to convert it into our own YYYYMMDD format.

   const char *compile_date = __DATE__;
   char year[5], month[3], day[3], temp[8];

   // get the year under the form YYYY
   strncpy (temp, compile_date + 7, 4);
   temp[4] = 0; // terminate the string
   sprintf (year, "%04d", atoi (temp)); // and format the 4 digits

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
   strncpy (temp, compile_date + 4, 2);
   temp[2] = 0; // terminate the string
   sprintf (day, "%02d", atoi (temp)); // and format the 2 digits

   // build the version string and the welcome text string
   sprintf (racc_version, "%s%s%s", year, month, day);
   sprintf (racc_welcometext, "RACC version %s - %s", racc_version, GameConfig.distributor_url);

   // also fill in the metamod plugin info structure
   Plugin_info.version = racc_version;
   Plugin_info.date = racc_version;

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


bool FileExists (const char *pathname)
{
   // this function tests if a file exists, and returns TRUE if it is the case, FALSE otherwise.

   return (access (pathname, 0) != -1); // standard C library stuff, courtesy of Whistler
}


void DllAttaching (void)
{
   // this function MUST be the first one called after the bot DLL is being attached by the
   // system. No matter which architecture you intend to port RACC for, you *MUST* call this
   // function as soon as possible anytime the bot DLL is run. We do this in GameDLLInit().

   lsrand (time (NULL)); // initialize the system's random number generator

   server.is_multiplayer = (atoi (CVAR_GET_STRING ("deathmatch")) > 0); // see if it's multiplayer
   server.min_bots = 0; // initialize the minimal number of bots to zero (no minimum)
   server.max_bots = -1; // initialize the maximal number of bots to -1 (no maximum)
   server.bot_chat_mode = BOT_CHAT_TEXTAUDIO; // initialize the bot chat mode to text+audio
   server.is_autofill = TRUE; // initialize server filling (bots filling up server) to TRUE
   server.is_internetmode = TRUE; // initialize internet mode (bots joining/leaving) to TRUE
   server.msec_method = METHOD_WHITEHOUSE; // choose a frame duration estimation method
   server.time = 0; // and set the clock up
   server.previous_time = 99999; // start value of previous time must be above current time

   InitGameConfig (); // build the game config database (first function to be called)
   MakeVersion(); // build the bot version string and welcome text
   InitLogFile (); // initialize the RACC log file
   InitPlayerBones (); // build the player bones database
   InitDefaultLikelevels (); // build the default likelevels database
   InitLanguages (); // build the bot languages database
   InitWeapons (); // build the weapons database
   InitSounds (); // build the game sounds database
   InitFootstepSounds (); // build the footstep sounds database
   InitRicochetSounds (); // build the ricochet sounds database

   return; // bot DLL initalization finished
}


void DllDetaching (void)
{
   // this function MUST be called when the bot DLL is being detached by the system. No matter
   // which architecture you intend to port RACC for, you *MUST* call this function when the
   // DLL detachs (because of a program shutdown). If you're porting it for Linux for example,
   // have the _fini() function have a walk around here.

   FreeAllTheStuff (); // free all the stuff upon DLL detaching

   return; // bot DLL uninitalization finished
}


float ProcessTime (void)
{
   // this function returns the time in seconds elapsed since the executable process started.
   // The rollover check ensures the program will continue running after clock() will have
   // overflown its integer value (it does so every 24 days or so). With this rollover check
   // we have a lifetime of more than billion years, w00t!
   // thanks to botmeister for the rollover check idea.

   static long current_clock;
   static long prev_clock = 0;
   static long rollover_count = 0;
   static double time_in_seconds;
   static double rollover = ((double) LONG_MAX + 1) / CLOCKS_PER_SEC; // fixed, won't move
   
   current_clock = clock (); // get system clock

   // has the clock overflown ?
   if (current_clock < prev_clock)
      rollover_count++; // omg, it has, we're running for more than 24 days!

   // now convert the time to seconds since last rollover
   time_in_seconds = (double) current_clock / CLOCKS_PER_SEC; // convert clock to seconds

   prev_clock = current_clock; // keep track of current time for future calls of this function

   // and return the time in seconds, adding the overflow differences if necessary.
   return (time_in_seconds + rollover * rollover_count);
}


bool IsValidPlayer (player_t *pPlayer)
{
   // this function returns TRUE if the player entity passed is taken by a player. Checking for
   // the player's netname seems to be the only reliable method of assessing that a player slot
   // is currently in use, since the Half-Life engine never frees the players' private data.

   if ((pPlayer == NULL) || !pPlayer->is_connected || (pPlayer->connection_name[0] == 0))
      return (FALSE); // if bad pointer OR free slot OR client has no netname then invalid player

   return (TRUE); // player entity has a netname, a player must be taking this slot
}


bool IsAlive (edict_t *pEdict)
{
   // this function returns TRUE if the entity pointed to by pEdict is alive and can be shot at,
   // FALSE otherwise.

   if (FNullEnt (pEdict))
      return (FALSE); // reliability check

   // return whether the conditions of pEdict for being alive and shootable are met
   return (((pEdict->v.deadflag == DEAD_NO) || (pEdict->v.deadflag == DEAD_DYING))
           && (pEdict->v.health > 0)
           && !(pEdict->v.flags & FL_NOTARGET)
           && (pEdict->v.takedamage != DAMAGE_NO));
}


bool IsBreakable (edict_t *pEdict)
{
   // this function returns TRUE if the entity pointed to by pEdict is breakable and can be
   // destroyed, FALSE otherwise.

   if (FNullEnt (pEdict))
      return (FALSE); // reliability check

   // return whether the conditions of pEdict for being breakable and destroyable are met
   return ((pEdict->v.takedamage != DAMAGE_NO) && !(pEdict->v.flags & FL_WORLDBRUSH)
           && (pEdict->v.health > 0)
           && (strcmp ("func_breakable", STRING (pEdict->v.classname)) == 0));
}


bool IsInFieldOfView (edict_t *pEdict, Vector v_location)
{
   // this function returns TRUE if the spatial vector location v_location is located inside
   // the field of view cone of the entity pEdict, FALSE otherwise. It is assumed that entities
   // have a human-like field of view, that is, about 90 degrees.

   static Vector v_deviation;

   if (FNullEnt (pEdict))
      return (FALSE); // reliability check

   // compute deviation angles (angles between pPlayer's forward direction and v_location)
   v_deviation = WrapAngles (VecToAngles (v_location - GetGunPosition (pEdict)) - pEdict->v.v_angle);

   // is v_location outside pPlayer's FOV width (90 degree) ?
   if (fabs (v_deviation.y) > pEdict->v.fov / 2)
      return (FALSE); // then v_location is not visible

   // is v_location outside pPlayer's FOV height (60 degree: consider the 4:3 screen ratio) ?
   if (fabs (v_deviation.x) > pEdict->v.fov / 3)
      return (FALSE); // then v_location is not visible

   return (TRUE); // else v_location has to be in pPlayer's field of view cone
}


float IlluminationOf (edict_t *pEdict)
{
   // this function returns a value between 0 and 100 corresponding to the entity's illumination.
   // Thanks to William van der Sterren for the human-like illumination filter computation. We
   // only consider noticeable the illuminations between 0 and 30 percent of the maximal value,
   // else it's too bright to be taken in account and we return the full illumination. The HL
   // engine allows entities to have illuminations up to 300 (hence the 75 as 30% of 300).

   // we have to call OUR pfnGetEntityIllum and not the engine's because of the fakeclient
   // illumination bug (fixed by Tom Simpson).

   int entity_index;
   edict_t *pIlluminationEdict;

   entity_index = ENTINDEX (pEdict) - 1; // get entity index

   // if pEdict is a bot, we had to create an invisible entity to correctly retrieve the
   // fakeclient's illumination (thanks to Tom Simpson from FoxBot for this engine bug fix)
   if ((entity_index >= 0) && (entity_index < 32) && players[entity_index].is_racc_bot)
      pIlluminationEdict = players[entity_index].Bot.pIllumination;
   else
      pIlluminationEdict = pEdict;

   // ask the engine for the entity illumination and filter it so as to return a usable value
   return (100 * sqrt (min (75.0, (float) GETENTITYILLUM (pIlluminationEdict)) / 75.0));
}


bool IsInvisible (edict_t *pEdict)
{
   // this function returns whether the entity pointed to by pEdict is invisible or not,
   // considering that invisible entities are either not drawn, or have no model.

   // returns TRUE if this entity is not visible (i.e, not drawn or no model set)
   return ((pEdict->v.effects & EF_NODRAW)
           || (pEdict->v.model == NULL)
           || (STRING (pEdict->v.model)[0] == 0));
}


unsigned char EnvironmentOf (edict_t *pEdict)
{
   // this function returns a character bitmap describing the current environment of the entity
   // pointed to by pEdict, such as whether this entity is on floor or not, in midair, or
   // surrounded by water partly or completely.

   if (pEdict->v.movetype == MOVETYPE_FLY)
      return (ENVIRONMENT_LADDER); // entity/client is on a ladder
   else if (pEdict->v.waterlevel == 2)
      return (ENVIRONMENT_SLOSHING); // entity/client is sloshing in water
   else if (pEdict->v.waterlevel == 3)
      return (ENVIRONMENT_WATER); // entity/client is surrounded by water
   else if (pEdict->v.flags & (FL_ONGROUND | FL_PARTIALGROUND))
      return (ENVIRONMENT_GROUND); // client is on ground
   else
      return (ENVIRONMENT_MIDAIR); // entity/client must be falling in mid-air

   return (ENVIRONMENT_UNKNOWN); // unable to determine entity/client environment
}


unsigned long InputButtonsOf (edict_t *pEdict)
{
   // this function returns a long integer bitmap describing the current input states of this
   // player's keyboard in an engine-independent fashion.

   static unsigned long input_buttons;

   // first reset the buttons structure to be returned
   input_buttons = INPUT_KEY_NONE;

   // now check successively for each of this player's buttons
   if (pEdict->v.button & IN_FORWARD)
      input_buttons |= INPUT_KEY_FORWARD; // this player is pressing FORWARD
   if (pEdict->v.button & IN_BACK)
      input_buttons |= INPUT_KEY_BACKWARDS; // this player is pressing BACKWARDS
   if (pEdict->v.button & IN_LEFT)
      input_buttons |= INPUT_KEY_TURNLEFT; // this player is turning left
   if (pEdict->v.button & IN_RIGHT)
      input_buttons |= INPUT_KEY_TURNRIGHT; // this player is turning right
   if (pEdict->v.button & IN_MOVELEFT)
      input_buttons |= INPUT_KEY_STRAFELEFT; // this player is pressing STRAFE LEFT
   if (pEdict->v.button & IN_MOVERIGHT)
      input_buttons |= INPUT_KEY_STRAFERIGHT; // this player is pressing STRAFE RIGHT
   if (pEdict->v.button & IN_JUMP)
      input_buttons |= INPUT_KEY_JUMP; // this player is jumping
   if (pEdict->v.button & IN_DUCK)
      input_buttons |= INPUT_KEY_DUCK; // this player is ducking
   if (pEdict->v.button & IN_ALT1)
      input_buttons |= INPUT_KEY_PRONE; // this player is proning
   if (pEdict->v.button & IN_RUN)
      input_buttons |= INPUT_KEY_WALK; // this player is walking instead of running
   if (pEdict->v.button & IN_USE)
      input_buttons |= INPUT_KEY_USE; // this player is taking an action upon something
   if (pEdict->v.button & IN_ATTACK)
      input_buttons |= INPUT_KEY_FIRE1; // this player is firing his primary fire
   if (pEdict->v.button & IN_ATTACK2)
      input_buttons |= INPUT_KEY_FIRE2; // this player is firing his secondary fire
   if (pEdict->v.button & IN_RELOAD)
      input_buttons |= INPUT_KEY_RELOAD; // this player is reloading his weapon
   if (pEdict->v.impulse == 201)
      input_buttons |= INPUT_KEY_SPRAY; // this player is spraying his logo
   if (pEdict->v.impulse == 100)
      input_buttons |= INPUT_KEY_LIGHT; // this player is turning his flashlight on/off
   if (pEdict->v.button & IN_SCORE)
      input_buttons |= INPUT_KEY_DISPLAYSCORE; // this player is displaying the scores grid

   return (input_buttons); // finished
}


void TheServerHasJustStarted (void)
{
   // This function is called at the FIRST video frame running on the server. Hence we can use
   // it to free and reinit anything which is map-specific. Freeing the map data is vital, since
   // in case of a map change, the server restarts, and thus this function will get called,
   // however nowhere will the mallocated map data will have been freed. Since a new map means
   // new BSP data to interpret, better do some house cleanup first.

   int client_index;
   player_t *pPlayer;

   // first off, print a welcome message on the server console to tell that we're running
   ServerConsole_printf ("\n");
   ServerConsole_printf ("   %s\n", racc_welcometext);
   ServerConsole_printf ("   This program comes with ABSOLUTELY NO WARRANTY; see license for details.\n");
   ServerConsole_printf ("   This is free software, you are welcome to redistribute it the way you want.\n");
   ServerConsole_printf ("\n");

   // VERY IMPORTANT, check who has connected and who has disconnected since the last map.
   for (client_index = 0; client_index < server.max_clients; client_index++)
   {
      pPlayer = &players[client_index]; // quick access to player

      // was this client recorded as connected ?
      if (pPlayer->is_connected)
      {
         APlayerHasDisconnected (pPlayer); // then this client just disconnected
         pPlayer->pEntity = NULL; // clear his entity pointer
      }
   }

   // retrieve some CVARs
   server.is_teamplay = (atoi (CVAR_GET_STRING ("mp_teamplay")) > 0);
   server.does_footsteps = (atoi (CVAR_GET_STRING ("mp_footsteps")) > 0);
   server.developer_level = atoi (CVAR_GET_STRING ("developer"));
   server.max_clients = gpGlobals->maxClients; // update the maximum number of clients
   server.max_entities = gpGlobals->maxEntities; // update the maximum number of entities

   // if we're in developer mode, enable the AI console
   if (server.developer_level > 0)
   {
      DebugLevel.aiconsole = TRUE;

      if (DebugLevel.eyes == 0)
         DebugLevel.eyes = 1;
      if (DebugLevel.ears == 0)
         DebugLevel.ears = 1;
      if (DebugLevel.body == 0)
         DebugLevel.body = 1;
      if (DebugLevel.legs == 0)
         DebugLevel.legs = 1;
      if (DebugLevel.hand == 0)
         DebugLevel.hand = 1;
      if (DebugLevel.chat == 0)
         DebugLevel.chat = 1;
      if (DebugLevel.cognition == 0)
         DebugLevel.cognition = 1;
      if (DebugLevel.navigation == 0)
         DebugLevel.navigation = 2;
   }

   // now read the map's BSP data, (re)load the bot profiles, and draw the world map (ie, fill
   // in the navigation hashtable).
   strcpy (server.map_name, STRING (gpGlobals->mapname)); // get map name
   LookDownOnTheWorld (); // look down on the world and sort the walkable surfaces
   LoadBotProfiles (); // load profiles

   player_count = 0; // no players connected yet
   bot_count = 0; // no bots either
   server.start_time = ProcessTime (); // see what time it is
   server.time = 0; // reset the server time since we're just started
   server.bot_check_time = 0.01; // ...and start adding bots now
   server.just_booted = FALSE; // server initialization finished

   mission.finished = TRUE; // reset the mission flag

   return; // the server is ready for bots now
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
   for (index = 0; index < server.max_clients; index++)
   {
      // free this bot's HAL brain
      HAL_FreeDictionary (players[index].Bot.BotBrain.banned_keywords);
      HAL_FreeDictionary (players[index].Bot.BotBrain.auxiliary_keywords);
      HAL_FreeSwap (players[index].Bot.BotBrain.swappable_keywords);
      HAL_EmptyModel (&players[index].Bot.BotBrain.HAL_model);
      HAL_FreeDictionary (players[index].Bot.BotBrain.input_words);
      HAL_FreeDictionary (players[index].Bot.BotBrain.bot_words);
      HAL_FreeDictionary (players[index].Bot.BotBrain.keys);
      HAL_FreeDictionary (players[index].Bot.BotBrain.replies);

      // free this bot's pathmachine
      BotShutdownPathMachine (&players[index]);

      // do we need to free this bot's nav brain ?
      if (players[index].Bot.BotBrain.PathMemory)
         free (players[index].Bot.BotBrain.PathMemory); // then free the navigation nodes array
      players[index].Bot.BotBrain.PathMemory = NULL;
   }

   // had we already allocated memory space for bot profiles ?
   if (profiles != NULL)
      free (profiles); // if so, free it
   profiles = NULL;
   profile_count = 0; // and reset the profiles count to zero

   // had we already allocated memory space for ricochet sounds ?
   if (ricochetsounds != NULL)
      free (ricochetsounds); // if so, free it
   ricochetsounds = NULL;
   ricochetsound_count = 0; // and reset the sounds count to zero

   // had we already allocated memory space for footstep sounds ?
   if (footstepsounds != NULL)
      free (footstepsounds); // if so, free it
   footstepsounds = NULL;
   footstepsound_count = 0; // and reset the sounds count to zero

   // had we already allocated memory space for bot sounds ?
   if (sounds != NULL)
      free (sounds); // if so, free it
   sounds = NULL;
   sound_count = 0; // and reset the sounds count to zero

   // had we already allocated memory space for weapons ?
   if (weapons != NULL)
      free (weapons); // if so, free it
   weapons = NULL;
   weapon_count = 0; // and reset the weapon count to zero

   // had we already allocated memory space for bot languages ?
   if (languages != NULL)
      free (languages); // if so, free it
   languages = NULL;
   language_count = 0; // and reset the languages count to zero

   // do we need to close the debug log file ?
   if (DebugLevel.fp != NULL)
      fclose (DebugLevel.fp); // close the file
   DebugLevel.fp = NULL;

   return;
}
