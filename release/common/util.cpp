/***
*
*  Copyright (c) 1999, Valve LLC. All rights reserved.
*
*  This product contains software technology licensed from Id 
*  Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*  All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

// RACC - AI development project for first-person shooter games derivated from Valve's Half-Life
// (http://www.racc-ai.com/)
//
// This project is based on the work done by Jeffrey 'Botman' Broome in his HPB bot
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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "bot_common.h"

extern HINSTANCE h_Library;
extern WORD *p_Ordinals;
extern DWORD *p_Functions;
extern DWORD *p_Names;
extern char *p_FunctionNames[1024];
extern int num_ordinals;
extern unsigned long base_offset;
extern bool is_dedicated_server;
extern edict_t *listenserver_edict;
extern bot_t bots[32];
extern char language[32];
extern char *g_argv;
extern bool isFakeClientCommand;
extern int fake_arg_count;
extern char arg[128];
extern bool welcome_sent;
extern char bot_affirmative[5][100][256];
extern char bot_negative[5][100][256];
extern char bot_hello[5][100][256];
extern char bot_laugh[5][100][256];
extern char bot_whine[5][100][256];
extern char bot_idle[5][100][256];
extern char bot_follow[5][100][256];
extern char bot_stop[5][100][256];
extern char bot_stay[5][100][256];
extern char bot_help[5][100][256];
extern char bot_cant[5][100][256];
extern char bot_bye[5][100][256];
extern int affirmative_count[5];
extern int negative_count[5];
extern int hello_count[5];
extern int laugh_count[5];
extern int whine_count[5];
extern int idle_count[5];
extern int follow_count[5];
extern int stop_count[5];
extern int stay_count[5];
extern int help_count[5];
extern int cant_count[5];
extern int bye_count[5];
extern char bot_names[100][32];
extern char bot_skins[100][32];
extern char bot_logos[100][32];
extern int bot_nationalities[100];
extern int bot_skills[100];
extern int number_names;
extern int team_allies[4];
extern int beam_texture;
extern int speaker_texture;
extern int voiceicon_height;
extern bool footstep_sounds_on;
extern int message_ShowMenu;
extern int message_TextMsg;
extern int message_SayText;


Vector UTIL_VecToAngles (const Vector &vec)
{
   float rgflVecOut[3];

   if (vec != Vector (0, 0, 0))
      VEC_TO_ANGLES (vec, rgflVecOut);

   return UTIL_WrapAngles (Vector (rgflVecOut));
}


// Overloaded to add IGNORE_GLASS
void UTIL_TraceLine (const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t *pentIgnore, TraceResult *ptr)
{
   TRACE_LINE (vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE) | (ignoreGlass ? 0x100 : 0), pentIgnore, ptr);
}


void UTIL_TraceLine (const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr)
{
   TRACE_LINE (vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE), pentIgnore, ptr);
}


void UTIL_TraceHull (const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t *pentIgnore, TraceResult *ptr)
{
   TRACE_HULL (vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE), hullNumber, pentIgnore, ptr);
}


void UTIL_MakeVectors (const Vector &vecAngles)
{
   MAKE_VECTORS (UTIL_WrapAngles (vecAngles));
}


edict_t *UTIL_FindEntityInSphere (edict_t *pentStart, const Vector &vecCenter, float flRadius)
{
   edict_t *pEntity = FIND_ENTITY_IN_SPHERE (pentStart, vecCenter, flRadius);

   if (!FNullEnt (pEntity))
      return pEntity;

   return NULL;
}


edict_t *UTIL_FindEntityByString (edict_t *pentStart, const char *szKeyword, const char *szValue)
{
   edict_t *pEntity = FIND_ENTITY_BY_STRING (pentStart, szKeyword, szValue);

   if (!FNullEnt (pEntity))
      return pEntity;

   return NULL;
}


edict_t *UTIL_FindEntityByClassname (edict_t *pentStart, const char *szName)
{
   return UTIL_FindEntityByString (pentStart, "classname", szName);
}


edict_t *UTIL_FindEntityByTargetname (edict_t *pentStart, const char *szName)
{
   return UTIL_FindEntityByString (pentStart, "targetname", szName);
}


int UTIL_PointContents (const Vector &vec)
{
   return POINT_CONTENTS (vec);
}


void UTIL_SetSize (entvars_t *pev, const Vector &vecMin, const Vector &vecMax)
{
   SET_SIZE (ENT (pev), vecMin, vecMax);
}


void UTIL_SetOrigin (entvars_t *pev, const Vector &vecOrigin)
{
   SET_ORIGIN (ENT (pev), vecOrigin);
}


void ClientPrint (edict_t *pEntity, int msg_dest, const char *msg_name)
{
   if (pEntity == NULL)
      return; // reliability check

   if (message_TextMsg == 0)
      message_TextMsg = (*g_engfuncs.pfnRegUserMsg) ("TextMsg", -1); // register the TextMsg message in case not done yet

   pfnMessageBegin (MSG_ONE, message_TextMsg, NULL, pEntity);
   pfnWriteByte (msg_dest);
   pfnWriteString (msg_name);
   pfnMessageEnd ();
}


void UTIL_SayText (const char *pText, edict_t *pEdict)
{
   if (pEdict == NULL)
      return; // reliability check

   if (message_SayText == 0)
      message_SayText = (*g_engfuncs.pfnRegUserMsg) ("SayText", -1); // register the SayText message in case not done yet

   pfnMessageBegin (MSG_ONE, message_SayText, NULL, pEdict);
   pfnWriteByte (ENTINDEX (pEdict));
   pfnWriteString (pText);
   pfnMessageEnd ();
}


void UTIL_HostSay (edict_t *pSenderEntity, int teamonly, char *message)
{
   char text[128], *pc;
   int i;
   edict_t *pClient = NULL;

   // make sure the text has content
   for (pc = message; pc != NULL && *pc != 0; pc++)
      if (isprint (*pc) && !isspace (*pc))
      {
         pc = NULL; // we've found an alphanumeric character,  so text is valid
         break;
      }

   if (!IsAlive (pSenderEntity) || (pc != NULL))
      return; // sender dead OR no character found, so say nothing

   // turn on color set 2 (color on, no sound)
   if (teamonly)
      sprintf (text, "%c(TEAM) %s: ", 2, STRING (pSenderEntity->v.netname));
   else
      sprintf (text, "%c%s: ", 2, STRING (pSenderEntity->v.netname));

   i = sizeof (text) - 2 - strlen (text); // -2 for /n and null terminator

   if ((int) strlen (message) > i)
      message[i] = 0;

   strcat (text, message);
   strcat (text, "\n");

   // print to the sending client
   UTIL_SayText (text, pSenderEntity);

   // loop through all players
   while (((pClient = UTIL_FindEntityByClassname (pClient, "player")) != NULL) && !FNullEnt (pClient)) 
   {
      if (pClient == pSenderEntity)
         continue; // skip sender of message

      if (teamonly && (GetTeam (pClient) != GetTeam (pSenderEntity)))
         continue; // skip opponents

      // print to this teammate
      pfnMessageBegin (MSG_ONE, message_SayText, NULL, pClient);
      pfnWriteByte (ENTINDEX (pSenderEntity));
      pfnWriteString (text);
      pfnMessageEnd ();
   }
}


int BotCount (void)
{
   int i, count = 0;

   // cycle through all bot slots
   for (i = 0; i < 32; i++)
      if (bots[i].is_active && (bots[i].pEdict != NULL))
         count++; // sum up the active slots

   return (count);
}


int UTIL_GetBotIndex (const edict_t *pEdict)
{
   int index;

   for (index = 0; index < 32; index++)
      if (bots[index].pEdict == pEdict)
         return index;

   return -1; // return -1 if edict is not a bot
}


bool IsAlive (edict_t *pEdict)
{
   if (pEdict == NULL)
      return FALSE; // reliability check

   return ((pEdict->v.deadflag == DEAD_NO)
           && (pEdict->v.health > 0)
           && !(pEdict->v.flags & FL_NOTARGET)
           && (pEdict->v.takedamage != DAMAGE_NO));
}


bool FInViewCone (Vector pOrigin, edict_t *pEdict)
{
   Vector2D vec2LOS;
   float flDot;

   if (pEdict == NULL)
      return FALSE; // reliability check

   UTIL_MakeVectors (pEdict->v.angles);

   vec2LOS = (pOrigin - pEdict->v.origin).Make2D ();
   vec2LOS = vec2LOS.Normalize ();

   flDot = DotProduct (vec2LOS, gpGlobals->v_forward.Make2D ());

   // 60 degree field of view 
   if (flDot > 0.50)
      return TRUE;
   else
      return FALSE;
}


bool FVisible (const Vector &vecOrigin, edict_t *pEdict)
{
   TraceResult tr;

   if (pEdict == NULL)
      return FALSE; // reliability check

   // don't look through water
   if ((UTIL_PointContents (vecOrigin) == CONTENTS_WATER)
       != (UTIL_PointContents (pEdict->v.origin + pEdict->v.view_ofs) == CONTENTS_WATER))
      return FALSE;

   UTIL_TraceLine (pEdict->v.origin + pEdict->v.view_ofs, vecOrigin, ignore_monsters, ignore_glass, pEdict, &tr);
   if (tr.flFraction == 1.0)
      return TRUE; // line of sight is valid.

   return FALSE; // line of sight is not established
}


bool IsReachable (Vector v_dest, edict_t *pEdict)
{
   TraceResult tr;
   float curr_height, last_height, distance;
   Vector v_check = pEdict->v.origin;
   Vector v_direction = (v_dest - v_check).Normalize (); // 1 unit long

   if (pEdict == NULL)
      return FALSE; // reliability check

   // check for special case of both the bot and its destination being underwater...
   if ((POINT_CONTENTS (pEdict->v.origin) == CONTENTS_WATER) && (POINT_CONTENTS (v_dest) == CONTENTS_WATER))
      return TRUE; // if so, assume it's reachable

   // now check if distance to ground increases more than jump height
   // at points between source and destination...

   UTIL_TraceLine (v_check, v_check + Vector (0, 0, -1000), ignore_monsters, pEdict->v.pContainingEntity, &tr);

   last_height = tr.flFraction * 1000.0; // height from ground
   distance = (v_dest - v_check).Length (); // distance from goal

   while (distance > 40.0)
   {
      v_check = v_check + v_direction * 40.0; // move 40 units closer to the goal...

      UTIL_TraceLine (v_check, v_check + Vector (0, 0, -1000), ignore_monsters, pEdict->v.pContainingEntity, &tr);

      curr_height = tr.flFraction * 1000.0; // height from ground

      // is the difference between last and current height higher that the max jump height ?
      if ((last_height - curr_height) > 63.0)
         return FALSE; // if so, assume it's NOT reachable

      last_height = curr_height; // backup current height
      distance = (v_dest - v_check).Length (); // update distance to goal
   }

   return TRUE; // this point is reachable
}




bool IsAtHumanHeight (Vector v_location)
{
   TraceResult tr;

   // trace down from v_location to see if it is at human standing height from the ground
   UTIL_TraceLine (v_location, v_location + Vector (0, 0, -72), ignore_monsters, NULL, &tr);

   if (tr.flFraction < 1.0)
      return TRUE; // ground was found

   return FALSE; // ground was not found, seems like v_location is in mid-air or outside the map
}


Vector DropAtHumanHeight (Vector v_location)
{
   TraceResult tr;

   // trace down from v_location and return a vector at human standing height from the ground
   UTIL_TraceLine (v_location, v_location + Vector (0, 0, -9999), ignore_monsters, NULL, &tr);

   if (tr.flFraction < 1.0)
      return (tr.vecEndPos + Vector (0, 0, 54)); // ground was found, return a lowered vector

   return (Vector (0, 0, 0)); // aargh, ground was not found !
}


Vector GetGunPosition (edict_t *pEdict)
{
   return (pEdict->v.origin + pEdict->v.view_ofs);
}


Vector VecBModelOrigin (edict_t *pEdict)
{
   return pEdict->v.absmin + (pEdict->v.size * 0.5);
}


bool BotCheckForSounds (bot_t *pBot, edict_t *pPlayer)
{
   if ((pBot->pEdict == NULL) || (pPlayer == NULL))
      return FALSE; // reliability check

   // update sounds made by this player, alert bots if they are nearby...
   if (footstep_sounds_on)
   {
      // check if this player is moving fast enough to make sounds...
      if (pPlayer->v.velocity.Length2D () > 220.0)
      {
         // is the bot close enough to hear this sound?
         if ((pPlayer->v.origin - pBot->pEdict->v.origin).Length () < 500)
         {
            BotSetIdealYaw (pBot, UTIL_VecToAngles (pPlayer->v.origin - pBot->pEdict->v.origin).y);
            return TRUE;
         }
      }
   }

   return FALSE;
}


void UTIL_ShowMenu (edict_t *pEdict, int slots, int displaytime, bool needmore, char *pText)
{
   if (pEdict == NULL)
      return; // reliability check

   if (message_ShowMenu == 0)
      message_ShowMenu = (*g_engfuncs.pfnRegUserMsg) ("ShowMenu", -1);

   pfnMessageBegin (MSG_ONE, message_ShowMenu, NULL, pEdict);
   pfnWriteShort (slots);
   pfnWriteChar (displaytime);
   pfnWriteByte (needmore);
   pfnWriteString (pText);
   pfnMessageEnd ();
}


void UTIL_DrawBeam (edict_t *pEntity, Vector start, Vector end, int life, int width, int noise, int red, int green, int blue, int brightness, int speed)
{
   if (pEntity == NULL)
      return; // reliability check

   MESSAGE_BEGIN (MSG_ONE, SVC_TEMPENTITY, NULL, pEntity);
   WRITE_BYTE (TE_BEAMPOINTS);
   WRITE_COORD (start.x);
   WRITE_COORD (start.y);
   WRITE_COORD (start.z);
   WRITE_COORD (end.x);
   WRITE_COORD (end.y);
   WRITE_COORD (end.z);
   WRITE_SHORT (beam_texture);
   WRITE_BYTE (1); // framestart
   WRITE_BYTE (10); // framerate
   WRITE_BYTE (life); // life in 0.1's
   WRITE_BYTE (width); // width
   WRITE_BYTE (noise); // noise
   WRITE_BYTE (red); // r, g, b
   WRITE_BYTE (green); // r, g, b
   WRITE_BYTE (blue); // r, g, b
   WRITE_BYTE (brightness); // brightness
   WRITE_BYTE (speed); // speed
   MESSAGE_END ();
}


void UTIL_printf (char *fmt, ...)
{
   va_list argptr;
   static char string[1024];

   va_start (argptr, fmt);
   vsprintf (string, fmt, argptr);
   va_end (argptr);

   // are we running a dedicated server or a listen server ?
   if (is_dedicated_server)
      printf (string); // print to stdout
   else if (listenserver_edict != NULL)
      UTIL_SayText (string, listenserver_edict); // print to HUD
   else
      ALERT (at_console, string); // print to console
}


void UTIL_ServerConsole_printf (char *fmt, ...)
{
   va_list argptr;
   static char string[1024];

   va_start (argptr, fmt);
   vsprintf (string, fmt, argptr);
   va_end (argptr);

   // are we running a dedicated server or a listen server ?
   if (is_dedicated_server)
      printf (string); // print to stdout
   else
      ALERT (at_console, string); // print to console
}


void BotSetIdealYaw (bot_t *pBot, float value)
{
   if (pBot->pEdict == NULL)
      return; // reliability check

   pBot->pEdict->v.ideal_yaw = UTIL_WrapAngle (value);
}


void BotAddIdealYaw (bot_t *pBot, float value)
{
   if (pBot->pEdict == NULL)
      return; // reliability check

   pBot->pEdict->v.ideal_yaw = UTIL_WrapAngle (pBot->pEdict->v.ideal_yaw + value);
}


void BotSetIdealPitch (bot_t *pBot, float value)
{
   if (pBot->pEdict == NULL)
      return; // reliability check

   pBot->pEdict->v.idealpitch = UTIL_WrapAngle (value);
}


void BotAddIdealPitch (bot_t *pBot, float value)
{
   if (pBot->pEdict == NULL)
      return; // reliability check

   pBot->pEdict->v.idealpitch = UTIL_WrapAngle (pBot->pEdict->v.idealpitch + value);
}


void BotSetViewAngles (bot_t *pBot, Vector v_angles)
{
   if (pBot->pEdict == NULL)
      return; // reliability check

   pBot->pEdict->v.v_angle = UTIL_WrapAngles (v_angles);
}


void BotAddViewAngles (bot_t *pBot, Vector v_angles)
{
   if (pBot->pEdict == NULL)
      return; // reliability check

   pBot->pEdict->v.v_angle = UTIL_WrapAngles (pBot->pEdict->v.v_angle + v_angles);
}


float UTIL_WrapAngle (float angle_to_wrap)
{
   float angle = angle_to_wrap;

   // check for wraparound of angle
   if (angle > 180)
      angle -= 360 * ((int) (angle / 360) + 1);
   if (angle < -180)
      angle += 360 * ((int) (angle / 360) + 1);

   return angle;
}


float UTIL_EngineWrapAngle (float angle_to_wrap)
{
   float angle = angle_to_wrap;

   // check for wraparound of angle
   if (angle > 360)
      angle -= 360 * (int) (angle / 360);
   if (angle < 0)
      angle += 360 * ((int) (angle / 360) + 1);

   return angle;
}


Vector UTIL_WrapAngles (Vector angles_to_wrap)
{
   Vector angles = angles_to_wrap;

   // check for wraparound of angles
   if (angles.x > 180)
      angles.x -= 360 * ((int) (angles.x / 360) + 1);
   if (angles.x < -180)
      angles.x += 360 * ((int) (angles.x / 360) + 1);
   if (angles.y > 180)
      angles.y -= 360 * ((int) (angles.y / 360) + 1);
   if (angles.y < -180)
      angles.y += 360 * ((int) (angles.y / 360) + 1);
   if (angles.z > 180)
      angles.z -= 360 * ((int) (angles.z / 360) + 1);
   if (angles.z < -180)
      angles.z += 360 * ((int) (angles.z / 360) + 1);

   return angles;
}


Vector UTIL_EngineWrapAngles (Vector angles_to_wrap)
{
   Vector angles = angles_to_wrap;

   // check for wraparound of angles
   if (angles.x > 360)
      angles.x -= 360 * (int) (angles.x / 360);
   if (angles.x < 0)
      angles.x += 360 * ((int) (angles.x / 360) + 1);
   if (angles.y > 360)
      angles.y -= 360 * (int) (angles.y / 360);
   if (angles.y < 0)
      angles.y += 360 * ((int) (angles.y / 360) + 1);
   if (angles.z > 360)
      angles.z -= 360 * (int) (angles.z / 360);
   if (angles.z < 0)
      angles.z += 360 * ((int) (angles.z / 360) + 1);

   return angles;
}


float UTIL_AngleOfVectors (Vector v1, Vector v2)
{
   if ((v1.Length () == 0) || (v2.Length () == 0))
      return 0; // avoid zero divide

   return (UTIL_WrapAngle (acos (DotProduct (v1, v2) / (v1.Length () * v2.Length ())) * 180 / M_PI));
}


float BotAngleToLocation (bot_t *pBot, Vector dest)
{
   if (pBot->pEdict == NULL)
      return 0; // reliability check

   // return the absolute value of angle to destination
   float angle = abs (UTIL_EngineWrapAngle (pBot->pEdict->v.v_angle.y)
                      - UTIL_EngineWrapAngle (UTIL_VecToAngles (dest).y));

   // revert to -180 / 180 format
   if (angle > 180)
      angle = 360 - angle;

   return (UTIL_WrapAngle (angle));
}


int UTIL_GetEntityIllum (edict_t *pEdict)
{
   int illum = 0;
   edict_t *pFakeEntity = NULL;

   if (pEdict == NULL)
      return 0; // reliability check

   // if pEdict is NOT a fakeclient, return its standard illumination value
   if (!(pEdict->v.flags & FL_FAKECLIENT))
      illum = GETENTITYILLUM (pEdict);

   // else need to create a fake entity to correctly retrieve fakeclient's illumination
   // thanks to Tom Simpson for the trick
   else
   {
      pFakeEntity = CREATE_NAMED_ENTITY (MAKE_STRING ("info_target")); // create entity
      DispatchSpawn (pFakeEntity); // spawn it
      pFakeEntity->v.origin = pEdict->v.origin; // sets it origin at pEntity's origin
      pFakeEntity->v.takedamage = DAMAGE_NO; // doesn't allow it to take damage
      pFakeEntity->v.solid = SOLID_NOT; // make it invisible
      pFakeEntity->v.owner = pEdict; // make it belong to edict
      pFakeEntity->v.movetype = MOVETYPE_FLY; // no clip
      pFakeEntity->v.classname = MAKE_STRING ("entity_botlightvalue"); // sets a name for it
      pFakeEntity->v.nextthink = gpGlobals->time; // need to make it think
      pFakeEntity->v.rendermode = kRenderNormal; // normal rendering mode
      pFakeEntity->v.renderfx = kRenderFxNone; // no special FX
      pFakeEntity->v.renderamt = 0; // ???
      SET_MODEL (pFakeEntity, "models/mechgibs.mdl"); // sets it a model
      illum = GETENTITYILLUM (pFakeEntity); // save its illumination
      REMOVE_ENTITY (pFakeEntity); // don't forget to destroy it
   }

   return (illum);
}


int UTIL_GetNearestOrderableBotIndex (edict_t *pAskingEntity)
{
   float distance[32], nearest_distance = 9999;
   int i, index = -1, bot_team, player_team = GetTeam (pAskingEntity);

   // cycle all bots to find the nearest one which is not currently "used" by other player
   for (i = 0; i < 32; i++)
   {
      if (bots[i].pEdict == NULL)
         continue; // skip unregistered bot slots
            
      bot_team = GetTeam (bots[i].pEdict);

      // is this bot dead OR inactive OR asking entity itself OR used by another OR hostile OR not visible ?
      if (!IsAlive (bots[i].pEdict) || !bots[i].is_active || (bots[i].pEdict == pAskingEntity)
         || ((bots[i].pBotUser != NULL) && (bots[i].pBotUser != pAskingEntity))
         || !((bot_team == player_team) || (team_allies[bot_team] & (1 << player_team)))
         || (BotGetIdealAimVector (&bots[i], pAskingEntity) == Vector (0, 0, 0)))
         continue; // if so, skip to the next bot

      // how far away is the bot?
      distance[i] = (pAskingEntity->v.origin - bots[i].pEdict->v.origin).Length ();
            
      if (distance[i] < nearest_distance)
      {
         nearest_distance = distance[i]; // update bot distances
         index = i; // keep track of the closest bot
      }
   }

   return index; // return index of the nearest orderable bot
}


int UTIL_GetNearestUsableBotIndex (edict_t *pAskingEntity)
{
   float distance[32], nearest_distance = 9999;
   int i, index = -1, bot_team, player_team = GetTeam (pAskingEntity);

   // cycle all bots to find the nearest one which is not currently "used"
   for (i = 0; i < 32; i++)
   {
      if (bots[i].pEdict == NULL)
         continue; // skip unregistered bot slots
            
      bot_team = GetTeam (bots[i].pEdict);

      // is this bot dead OR inactive OR asking entity itself OR used OR hostile OR not visible ?
      if (!IsAlive (bots[i].pEdict) || !bots[i].is_active
         || (bots[i].pEdict == pAskingEntity) || (bots[i].pBotUser != NULL)
         || !((bot_team == player_team) || (team_allies[bot_team] & (1 << player_team)))
         || (BotGetIdealAimVector (&bots[i], pAskingEntity) == Vector (0, 0, 0)))
         continue; // if so, skip to the next bot

      // how far away is the bot?
      distance[i] = (pAskingEntity->v.origin - bots[i].pEdict->v.origin).Length ();
            
      if (distance[i] < nearest_distance)
      {
         nearest_distance = distance[i]; // update bot distances
         index = i; // keep track of the closest bot
      }
   }

   return index; // return index of the nearest usable bot
}


int UTIL_GetNearestUsedBotIndex (edict_t *pAskingEntity)
{
   float distance[32], nearest_distance = 9999;
   int i, index = -1, bot_team, player_team = GetTeam (pAskingEntity);

   // cycle all bots to find the nearest one which is not currently "used" by other player
   for (i = 0; i < 32; i++)
   {
      if (bots[i].pEdict == NULL)
         continue; // skip unregistered bot slots
            
      bot_team = GetTeam (bots[i].pEdict);

      // is this bot dead OR inactive OR asking entity itself OR not used by asking entity ?
      if (!IsAlive (bots[i].pEdict) || !bots[i].is_active
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

   return index; // return index of the nearest used bot
}


void GetGameLocale (void)
{
   FILE *fp;
   char line_buffer[256];

   // get the game language from the sierra.inf file
   fp = fopen ("sierra.inf", "r"); // opens file readonly

   if (fp != NULL)
   {
      // reads line per line
      while (fgets (line_buffer, 255, fp) != NULL)
      {
         if ((line_buffer[0] == '\n') || ((line_buffer[0] == '/') && (line_buffer[1] == '/')))
            continue; // ignore line if void or commented

         // if that line tells the standard American/English language definition
         if (strncmp ("ShortTitle=HALFLIFE", line_buffer, 19) == 0)
         {
            sprintf (language, "english");
            fclose (fp);
            return;
         }

         // else if that line tells the French language definition
         else if (strncmp ("ShortTitle=HLIFEFR", line_buffer, 18) == 0)
         {
            sprintf (language, "french");
            fclose (fp);
            return;
         }

         // else if that line tells the Deutsch language definition
         else if (strncmp ("ShortTitle=HLIFEDE", line_buffer, 18) == 0)
         {
            sprintf (language, "german");
            fclose (fp);
            return;
         }

         // else if that line tells the Italian language definition
         else if (strncmp ("ShortTitle=HLIFEIT", line_buffer, 18) == 0)
         {
            sprintf (language, "italian");
            fclose (fp);
            return;
         }

         // else if that line tells the Spanish language definition
         else if (strncmp ("ShortTitle=HLIFEES", line_buffer, 18) == 0)
         {
            sprintf (language, "spanish");
            fclose (fp);
            return;
         }
      }
   }

   sprintf (language, "english"); // defaults to english if not found
   return;
}


void InitCVARs (void)
{
   FILE *fp;
   char filename[256];
   char line_buffer[256];
   char cvar_name[256];
   char cvar_value[256];
   int length, index, i, fieldstart, fieldstop;

   // forcibly init some RACC CVARs at game start before the engine does it

   GET_GAME_DIR (filename); // get the current game directory and copy it to filename
   strcat (filename, "/game.cfg"); // append the file name to build the relative path

   fp = fopen (filename, "r"); // open the game.cfg file readonly
   if (fp != NULL)
   {
      // print a message on the server console
      UTIL_ServerConsole_printf ("RACC: anticipating game.cfg execution\n");

      // read line per line
      while (fgets (line_buffer, 255, fp) != NULL)
      {
         if ((line_buffer[0] == '\n') || ((line_buffer[0] == '/') && (line_buffer[1] == '/')))
            continue; // ignore line if void or commented

         length = strlen (line_buffer); // get length of line
         if (length > 0)
            if (line_buffer[length - 1] == '\n')
               length--; // remove any final '\n'
         line_buffer[length] = 0; // terminate the string
         index = 0; // position at start of the string

         // does this line deals about one of the CVARs we want to be set ?
         if ((strncmp (line_buffer, "\"racc_chatmode\"", 15) == 0)
             || (strncmp (line_buffer, "\"racc_voicechatmode\"", 20) == 0))
         {
            index++; // move one step further to bypass the quote (start of field)
            fieldstart = index; // save field start position (after the '"')
            while ((index < length) && (line_buffer[index] != '"'))
               index++; // reach end of field
            fieldstop = index - 1; // save field stop position (before the '"')
            for (i = fieldstart; i <= fieldstop; i++)
               cvar_name[i - fieldstart] = line_buffer[i]; // store the field value in a string
            cvar_name[i - fieldstart] = 0; // terminate the string
            index++; // move one step out of this field to bypass the quote

            while ((index < length) && ((line_buffer[index] == ' ') || (line_buffer[index] == '\t')))
               index++; // ignore any tabs or spaces

            index++; // move one step further to bypass the quote (start of field)
            fieldstart = index; // save field start position (after the '"')
            while ((index < length) && (line_buffer[index] != '"'))
               index++; // reach end of field
            fieldstop = index - 1; // save field stop position (before the '"')
            for (i = fieldstart; i <= fieldstop; i++)
               cvar_value[i - fieldstart] = line_buffer[i]; // store the field value in a string
            cvar_value[i - fieldstart] = 0; // terminate the string

            CVAR_SET_STRING (cvar_name, cvar_value); // we can now set this CVAR to its value
         }
      }

      fclose (fp); // close file
   }
}


void LoadBotProfiles (void)
{
   FILE *fp;
   char line_buffer[256];
   int length, index, i, fieldstart, fieldstop;
   char name[32];
   char skin[32];
   char logo[32];
   char nationality[32];

   // read the bots names from the file
   fp = fopen ("racc/profiles.cfg", "r"); // opens file readonly

   if (fp != NULL)
   {
      while ((number_names < 100) && (fgets (line_buffer, 256, fp) != NULL)) // reads line per line
      {
         length = strlen (line_buffer); // get length of line
         if (length > 0)
            if (line_buffer[length - 1] == '\n')
               length--; // remove any final '\n'
         line_buffer[length] = 0; // terminate the string

         index = 0; // let's now parse the line to get the different fields

         while ((index < length) && ((line_buffer[index] == ' ') || (line_buffer[index] == '\t')))
            index++; // ignore any tabs or spaces

         if (line_buffer[index] != '"')
            continue; // if not beginning by a quote, line is invalid, proceed to next one

         // name
         index++; // move one step further to bypass the quote (start of field)

         fieldstart = index; // save field start position (after the '"')
         while ((index < length) && (line_buffer[index] != '"'))
            index++; // reach end of field
         fieldstop = index - 1; // save field stop position (before the '"')
         if (fieldstop > fieldstart + 31)
            fieldstop = fieldstart + 31; // avoid stack overflows
         for (i = fieldstart; i <= fieldstop; i++)
            name[i - fieldstart] = line_buffer[i]; // store the field value in a string
         name[i - fieldstart] = 0; // terminate the string
         strncpy (bot_names[number_names], name, 32); // add value to names array
         bot_names[number_names][32] = 0; // terminate the string

         index++; // move one step out of this field to bypass the quote

         while ((index < length) && ((line_buffer[index] == ' ') || (line_buffer[index] == '\t')))
            index++; // ignore any tabs or spaces

         // skin
         fieldstart = index; // save field start position (first character)
         while ((index < length) && ((line_buffer[index] != ' ') && (line_buffer[index] != '\t')))
            index++; // reach end of field
         fieldstop = index - 1; // save field stop position (last character)
         if (fieldstop > fieldstart + 31)
            fieldstop = fieldstart + 31; // avoid stack overflows
         for (i = fieldstart; i <= fieldstop; i++)
            skin[i - fieldstart] = line_buffer[i]; // store the field value in a string
         skin[i - fieldstart] = 0; // terminate the string
         strncpy (bot_skins[number_names], skin, 32); // add value to skins array
         bot_skins[number_names][32] = 0; // terminate the string

         while ((index < length) && ((line_buffer[index] == ' ') || (line_buffer[index] == '\t')))
            index++; // ignore any tabs or spaces

         // logo
         fieldstart = index; // save field start position (first character)
         while ((index < length) && ((line_buffer[index] != ' ') && (line_buffer[index] != '\t')))
            index++; // reach end of field
         fieldstop = index - 1; // save field stop position (last character)
         if (fieldstop > fieldstart + 31)
            fieldstop = fieldstart + 31; // avoid stack overflows
         for (i = fieldstart; i <= fieldstop; i++)
            logo[i - fieldstart] = line_buffer[i]; // store the field value in a string
         logo[i - fieldstart] = 0; // terminate the string
         strncpy (bot_logos[number_names], logo, 32); // add value to logos array
         bot_logos[number_names][32] = 0; // terminate the string

         while ((index < length) && ((line_buffer[index] == ' ') || (line_buffer[index] == '\t')))
            index++; // ignore any tabs or spaces

         // nationality
         fieldstart = index; // save field start position (first character)
         while ((index < length) && ((line_buffer[index] != ' ') && (line_buffer[index] != '\t')))
            index++; // reach end of field
         fieldstop = index - 1; // save field stop position (last character)
         if (fieldstop > fieldstart + 31)
            fieldstop = fieldstart + 31; // avoid stack overflows
         for (i = fieldstart; i <= fieldstop; i++)
            nationality[i - fieldstart] = line_buffer[i]; // store the field value in a string
         nationality[i - fieldstart] = 0; // terminate the string
         if (strcmp (nationality, "french") == 0)
            bot_nationalities[number_names] = NATIONALITY_FRENCH; // add value to nationalities array
         else if (strcmp (nationality, "german") == 0)
            bot_nationalities[number_names] = NATIONALITY_GERMAN; // add value to nationalities array
         else if (strcmp (nationality, "italian") == 0)
            bot_nationalities[number_names] = NATIONALITY_ITALIAN; // add value to nationalities array
         else if (strcmp (nationality, "spanish") == 0)
            bot_nationalities[number_names] = NATIONALITY_SPANISH; // add value to nationalities array
         else
            bot_nationalities[number_names] = NATIONALITY_ENGLISH; // defaults to english if unknown

         while ((index < length) && ((line_buffer[index] == ' ') || (line_buffer[index] == '\t')))
            index++; // ignore any tabs or spaces

         // skill
         fieldstart = index; // save field start position (first character)
         while ((index < length) && ((line_buffer[index] != ' ') && (line_buffer[index] != '\t')))
            index++; // reach end of field
         bot_skills[number_names] = atoi (&line_buffer[fieldstart]); // store value in an integer

         // force skill in bounds
         if (bot_skills[number_names] < 1)
            bot_skills[number_names] = 1;
         else if (bot_skills[number_names] > 5)
            bot_skills[number_names] = 5;

         number_names++; // we have one more bot in the array
      }

      fclose (fp); // close file
   }
}


void LoadBotTextChat (void)
{
   FILE *fp;
   char path[256];
   char line_buffer[256];
   char current_language[32];

   // load bot text chat for ALL nationalities...
   for (int index = 0; index < 5; index++)
   {
      if (index == NATIONALITY_ENGLISH)
         sprintf (current_language, "english");
      else if (index == NATIONALITY_FRENCH)
         sprintf (current_language, "french");
      else if (index == NATIONALITY_GERMAN)
         sprintf (current_language, "german");
      else if (index == NATIONALITY_ITALIAN)
         sprintf (current_language, "italian");
      else if (index == NATIONALITY_SPANISH)
         sprintf (current_language, "spanish");

      // Build affirmative messages array
      sprintf (path, "racc/chat/%s/affirmative.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((affirmative_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_affirmative[index][affirmative_count[index]], line_buffer); // we have a valid line
            affirmative_count[index]++;
         }
         fclose (fp);
      }

      // Build bye messages array
      sprintf (path, "racc/chat/%s/bye.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((bye_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_bye[index][bye_count[index]], line_buffer); // we have a valid line
            bye_count[index]++;
         }
         fclose (fp);
      }

      // Build cant messages array
      sprintf (path, "racc/chat/%s/cantfollow.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((cant_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_cant[index][cant_count[index]], line_buffer); // we have a valid line
            cant_count[index]++;
         }
         fclose (fp);
      }

      // Build follow messages array
      sprintf (path, "racc/chat/%s/follow.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((follow_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_follow[index][follow_count[index]], line_buffer); // we have a valid line
            follow_count[index]++;
         }
         fclose (fp);
      }

      // Build hello messages array
      sprintf (path, "racc/chat/%s/hello.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((hello_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_hello[index][hello_count[index]], line_buffer); // we have a valid line
            hello_count[index]++;
         }
         fclose (fp);
      }

      // Build help messages array
      sprintf (path, "racc/chat/%s/help.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((help_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_help[index][help_count[index]], line_buffer); // we have a valid line
            help_count[index]++;
         }
         fclose (fp);
      }

      // Build idle messages array
      sprintf (path, "racc/chat/%s/idle.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((idle_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_idle[index][idle_count[index]], line_buffer); // we have a valid line
            idle_count[index]++;
         }
         fclose (fp);
      }

      // Build laugh messages array
      sprintf (path, "racc/chat/%s/laugh.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((laugh_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy(bot_laugh[index][laugh_count[index]], line_buffer); // we have a valid line
            laugh_count[index]++;
         }
         fclose (fp);
      }

      // Build negative messages array
      sprintf (path, "racc/chat/%s/negative.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((negative_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_negative[index][negative_count[index]], line_buffer); // we have a valid line
            negative_count[index]++;
         }
         fclose (fp);
      }

      // Build stay messages array
      sprintf (path, "racc/chat/%s/stay.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((stay_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_stay[index][stay_count[index]], line_buffer); // we have a valid line
            stay_count[index]++;
         }
         fclose (fp);
      }

      // Build stop messages array
      sprintf (path, "racc/chat/%s/stop.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((stop_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_stop[index][stop_count[index]], line_buffer); // we have a valid line
            stop_count[index]++;
         }
         fclose (fp);
      }

      // Build whine messages array
      sprintf (path, "racc/chat/%s/whine.txt", current_language);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((whine_count[index] < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_whine[index][whine_count[index]], line_buffer); // we have a valid line
            whine_count[index]++;
         }
         fclose (fp);
      }
   }
}


void PrecacheStuff (void)
{
   FILE *fp;
   char line_buffer[256];

   // print a welcome message on the server console
   UTIL_ServerConsole_printf ("\n   " RACC_WELCOMETEXT "\n");
   UTIL_ServerConsole_printf ("   This program comes with ABSOLUTELY NO WARRANTY; see license for details.\n");
   UTIL_ServerConsole_printf ("   This is free software, you are welcome to redistribute it the way you want.\n\n");

   // since the CVARs are not initialized yet at precache time, I am opening the game.cfg file
   // to manually set some of them at their proper values even before the engine does it
   InitCVARs ();

   beam_texture = PRECACHE_MODEL ("sprites/lgtning.spr"); // used to trace beams
   PRECACHE_MODEL ("models/mechgibs.mdl"); // used to create fake entities

   // only precache bot voice chat stuff if bot chat is allowed
   if ((CVAR_GET_FLOAT ("racc_chatmode") > 0) && (CVAR_GET_FLOAT ("racc_voicechatmode") > 0))
   {
      speaker_texture = PRECACHE_MODEL ("../racc/talk/voiceicon.spr"); // used to display speaker icon

      // gets voice icon altitude above head of players, stay at default value 45 if file not found
      if ((fp = fopen ("valve/scripts/voicemodel.txt", "r")) != NULL)
      {
         if (fgets (line_buffer, 255, fp) != NULL)
            voiceicon_height = atoi (line_buffer);
         fclose (fp);
      }

      // precache sounds used for bot audio system for the correct language
      if (strcmp (language, "english") == 0)
      {
         PRECACHE_SOUND ("../../racc/talk/english/affirmative0.wav");
         PRECACHE_SOUND ("../../racc/talk/english/affirmative1.wav");
         PRECACHE_SOUND ("../../racc/talk/english/affirmative2.wav");
         PRECACHE_SOUND ("../../racc/talk/english/affirmative3.wav");
         PRECACHE_SOUND ("../../racc/talk/english/affirmative4.wav");
         PRECACHE_SOUND ("../../racc/talk/english/affirmative5.wav");
         PRECACHE_SOUND ("../../racc/talk/english/alert0.wav");
         PRECACHE_SOUND ("../../racc/talk/english/alert1.wav");
         PRECACHE_SOUND ("../../racc/talk/english/alert2.wav");
         PRECACHE_SOUND ("../../racc/talk/english/attacking0.wav");
         PRECACHE_SOUND ("../../racc/talk/english/attacking1.wav");
         PRECACHE_SOUND ("../../racc/talk/english/attacking2.wav");
         PRECACHE_SOUND ("../../racc/talk/english/attacking3.wav");
         PRECACHE_SOUND ("../../racc/talk/english/attacking4.wav");
         PRECACHE_SOUND ("../../racc/talk/english/attacking5.wav");
         PRECACHE_SOUND ("../../racc/talk/english/firstspawn0.wav");
         PRECACHE_SOUND ("../../racc/talk/english/firstspawn1.wav");
         PRECACHE_SOUND ("../../racc/talk/english/firstspawn2.wav");
         PRECACHE_SOUND ("../../racc/talk/english/firstspawn3.wav");
         PRECACHE_SOUND ("../../racc/talk/english/firstspawn4.wav");
         PRECACHE_SOUND ("../../racc/talk/english/inposition0.wav");
         PRECACHE_SOUND ("../../racc/talk/english/inposition1.wav");
         PRECACHE_SOUND ("../../racc/talk/english/inposition2.wav");
         PRECACHE_SOUND ("../../racc/talk/english/inposition3.wav");
         PRECACHE_SOUND ("../../racc/talk/english/negative0.wav");
         PRECACHE_SOUND ("../../racc/talk/english/report0.wav");
         PRECACHE_SOUND ("../../racc/talk/english/report1.wav");
         PRECACHE_SOUND ("../../racc/talk/english/report2.wav");
         PRECACHE_SOUND ("../../racc/talk/english/report3.wav");
         PRECACHE_SOUND ("../../racc/talk/english/report4.wav");
         PRECACHE_SOUND ("../../racc/talk/english/reporting0.wav");
         PRECACHE_SOUND ("../../racc/talk/english/reporting1.wav");
         PRECACHE_SOUND ("../../racc/talk/english/reporting2.wav");
         PRECACHE_SOUND ("../../racc/talk/english/reporting3.wav");
         PRECACHE_SOUND ("../../racc/talk/english/reporting4.wav");
         PRECACHE_SOUND ("../../racc/talk/english/reporting5.wav");
         PRECACHE_SOUND ("../../racc/talk/english/reporting6.wav");
         PRECACHE_SOUND ("../../racc/talk/english/reporting7.wav");
         PRECACHE_SOUND ("../../racc/talk/english/reporting8.wav");
         PRECACHE_SOUND ("../../racc/talk/english/reporting9.wav");
         PRECACHE_SOUND ("../../racc/talk/english/reporting10.wav");
         PRECACHE_SOUND ("../../racc/talk/english/reporting11.wav");
         PRECACHE_SOUND ("../../racc/talk/english/reporting12.wav");
         PRECACHE_SOUND ("../../racc/talk/english/reporting13.wav");
         PRECACHE_SOUND ("../../racc/talk/english/reporting14.wav");
         PRECACHE_SOUND ("../../racc/talk/english/reporting15.wav");
         PRECACHE_SOUND ("../../racc/talk/english/reporting16.wav");
         PRECACHE_SOUND ("../../racc/talk/english/reporting17.wav");
         PRECACHE_SOUND ("../../racc/talk/english/seegrenade0.wav");
         PRECACHE_SOUND ("../../racc/talk/english/seegrenade1.wav");
         PRECACHE_SOUND ("../../racc/talk/english/seegrenade2.wav");
         PRECACHE_SOUND ("../../racc/talk/english/seegrenade3.wav");
         PRECACHE_SOUND ("../../racc/talk/english/takingdamage0.wav");
         PRECACHE_SOUND ("../../racc/talk/english/takingdamage1.wav");
         PRECACHE_SOUND ("../../racc/talk/english/takingdamage2.wav");
         PRECACHE_SOUND ("../../racc/talk/english/takingdamage3.wav");
         PRECACHE_SOUND ("../../racc/talk/english/takingdamage4.wav");
         PRECACHE_SOUND ("../../racc/talk/english/takingdamage5.wav");
         PRECACHE_SOUND ("../../racc/talk/english/takingdamage6.wav");
         PRECACHE_SOUND ("../../racc/talk/english/takingdamage7.wav");
         PRECACHE_SOUND ("../../racc/talk/english/throwgrenade0.wav");
         PRECACHE_SOUND ("../../racc/talk/english/throwgrenade1.wav");
         PRECACHE_SOUND ("../../racc/talk/english/throwgrenade2.wav");
         PRECACHE_SOUND ("../../racc/talk/english/throwgrenade3.wav");
         PRECACHE_SOUND ("../../racc/talk/english/throwgrenade4.wav");
         PRECACHE_SOUND ("../../racc/talk/english/victory0.wav");
         PRECACHE_SOUND ("../../racc/talk/english/victory1.wav");
         PRECACHE_SOUND ("../../racc/talk/english/victory2.wav");
         PRECACHE_SOUND ("../../racc/talk/english/victory3.wav");
         PRECACHE_SOUND ("../../racc/talk/english/victory4.wav");
      }
      else if (strcmp (language, "french") == 0)
      {
         PRECACHE_SOUND ("../../racc/talk/french/affirmative0.wav");
         PRECACHE_SOUND ("../../racc/talk/french/affirmative1.wav");
         PRECACHE_SOUND ("../../racc/talk/french/affirmative2.wav");
         PRECACHE_SOUND ("../../racc/talk/french/affirmative3.wav");
         PRECACHE_SOUND ("../../racc/talk/french/affirmative4.wav");
         PRECACHE_SOUND ("../../racc/talk/french/affirmative5.wav");
         PRECACHE_SOUND ("../../racc/talk/french/alert0.wav");
         PRECACHE_SOUND ("../../racc/talk/french/alert1.wav");
         PRECACHE_SOUND ("../../racc/talk/french/alert2.wav");
         PRECACHE_SOUND ("../../racc/talk/french/attacking0.wav");
         PRECACHE_SOUND ("../../racc/talk/french/attacking1.wav");
         PRECACHE_SOUND ("../../racc/talk/french/attacking2.wav");
         PRECACHE_SOUND ("../../racc/talk/french/attacking3.wav");
         PRECACHE_SOUND ("../../racc/talk/french/attacking4.wav");
         PRECACHE_SOUND ("../../racc/talk/french/attacking5.wav");
         PRECACHE_SOUND ("../../racc/talk/french/firstspawn0.wav");
         PRECACHE_SOUND ("../../racc/talk/french/firstspawn1.wav");
         PRECACHE_SOUND ("../../racc/talk/french/firstspawn2.wav");
         PRECACHE_SOUND ("../../racc/talk/french/firstspawn3.wav");
         PRECACHE_SOUND ("../../racc/talk/french/firstspawn4.wav");
         PRECACHE_SOUND ("../../racc/talk/french/inposition0.wav");
         PRECACHE_SOUND ("../../racc/talk/french/inposition1.wav");
         PRECACHE_SOUND ("../../racc/talk/french/inposition2.wav");
         PRECACHE_SOUND ("../../racc/talk/french/inposition3.wav");
         PRECACHE_SOUND ("../../racc/talk/french/negative0.wav");
         PRECACHE_SOUND ("../../racc/talk/french/report0.wav");
         PRECACHE_SOUND ("../../racc/talk/french/report1.wav");
         PRECACHE_SOUND ("../../racc/talk/french/report2.wav");
         PRECACHE_SOUND ("../../racc/talk/french/report3.wav");
         PRECACHE_SOUND ("../../racc/talk/french/report4.wav");
         PRECACHE_SOUND ("../../racc/talk/french/reporting0.wav");
         PRECACHE_SOUND ("../../racc/talk/french/reporting1.wav");
         PRECACHE_SOUND ("../../racc/talk/french/reporting2.wav");
         PRECACHE_SOUND ("../../racc/talk/french/reporting3.wav");
         PRECACHE_SOUND ("../../racc/talk/french/reporting4.wav");
         PRECACHE_SOUND ("../../racc/talk/french/reporting5.wav");
         PRECACHE_SOUND ("../../racc/talk/french/reporting6.wav");
         PRECACHE_SOUND ("../../racc/talk/french/reporting7.wav");
         PRECACHE_SOUND ("../../racc/talk/french/reporting8.wav");
         PRECACHE_SOUND ("../../racc/talk/french/reporting9.wav");
         PRECACHE_SOUND ("../../racc/talk/french/reporting10.wav");
         PRECACHE_SOUND ("../../racc/talk/french/reporting11.wav");
         PRECACHE_SOUND ("../../racc/talk/french/reporting12.wav");
         PRECACHE_SOUND ("../../racc/talk/french/reporting13.wav");
         PRECACHE_SOUND ("../../racc/talk/french/reporting14.wav");
         PRECACHE_SOUND ("../../racc/talk/french/reporting15.wav");
         PRECACHE_SOUND ("../../racc/talk/french/reporting16.wav");
         PRECACHE_SOUND ("../../racc/talk/french/reporting17.wav");
         PRECACHE_SOUND ("../../racc/talk/french/seegrenade0.wav");
         PRECACHE_SOUND ("../../racc/talk/french/seegrenade1.wav");
         PRECACHE_SOUND ("../../racc/talk/french/seegrenade2.wav");
         PRECACHE_SOUND ("../../racc/talk/french/seegrenade3.wav");
         PRECACHE_SOUND ("../../racc/talk/french/takingdamage0.wav");
         PRECACHE_SOUND ("../../racc/talk/french/takingdamage1.wav");
         PRECACHE_SOUND ("../../racc/talk/french/takingdamage2.wav");
         PRECACHE_SOUND ("../../racc/talk/french/takingdamage3.wav");
         PRECACHE_SOUND ("../../racc/talk/french/takingdamage4.wav");
         PRECACHE_SOUND ("../../racc/talk/french/takingdamage5.wav");
         PRECACHE_SOUND ("../../racc/talk/french/takingdamage6.wav");
         PRECACHE_SOUND ("../../racc/talk/french/takingdamage7.wav");
         PRECACHE_SOUND ("../../racc/talk/french/throwgrenade0.wav");
         PRECACHE_SOUND ("../../racc/talk/french/throwgrenade1.wav");
         PRECACHE_SOUND ("../../racc/talk/french/throwgrenade2.wav");
         PRECACHE_SOUND ("../../racc/talk/french/throwgrenade3.wav");
         PRECACHE_SOUND ("../../racc/talk/french/throwgrenade4.wav");
         PRECACHE_SOUND ("../../racc/talk/french/victory0.wav");
         PRECACHE_SOUND ("../../racc/talk/french/victory1.wav");
         PRECACHE_SOUND ("../../racc/talk/french/victory2.wav");
         PRECACHE_SOUND ("../../racc/talk/french/victory3.wav");
         PRECACHE_SOUND ("../../racc/talk/french/victory4.wav");
      }
      else if (strcmp (language, "german") == 0)
      {
         PRECACHE_SOUND ("../../racc/talk/german/affirmative0.wav");
         PRECACHE_SOUND ("../../racc/talk/german/affirmative1.wav");
         PRECACHE_SOUND ("../../racc/talk/german/affirmative2.wav");
         PRECACHE_SOUND ("../../racc/talk/german/affirmative3.wav");
         PRECACHE_SOUND ("../../racc/talk/german/affirmative4.wav");
         PRECACHE_SOUND ("../../racc/talk/german/affirmative5.wav");
         PRECACHE_SOUND ("../../racc/talk/german/alert0.wav");
         PRECACHE_SOUND ("../../racc/talk/german/alert1.wav");
         PRECACHE_SOUND ("../../racc/talk/german/alert2.wav");
         PRECACHE_SOUND ("../../racc/talk/german/attacking0.wav");
         PRECACHE_SOUND ("../../racc/talk/german/attacking1.wav");
         PRECACHE_SOUND ("../../racc/talk/german/attacking2.wav");
         PRECACHE_SOUND ("../../racc/talk/german/attacking3.wav");
         PRECACHE_SOUND ("../../racc/talk/german/attacking4.wav");
         PRECACHE_SOUND ("../../racc/talk/german/attacking5.wav");
         PRECACHE_SOUND ("../../racc/talk/german/firstspawn0.wav");
         PRECACHE_SOUND ("../../racc/talk/german/firstspawn1.wav");
         PRECACHE_SOUND ("../../racc/talk/german/firstspawn2.wav");
         PRECACHE_SOUND ("../../racc/talk/german/firstspawn3.wav");
         PRECACHE_SOUND ("../../racc/talk/german/firstspawn4.wav");
         PRECACHE_SOUND ("../../racc/talk/german/inposition0.wav");
         PRECACHE_SOUND ("../../racc/talk/german/inposition1.wav");
         PRECACHE_SOUND ("../../racc/talk/german/inposition2.wav");
         PRECACHE_SOUND ("../../racc/talk/german/inposition3.wav");
         PRECACHE_SOUND ("../../racc/talk/german/negative0.wav");
         PRECACHE_SOUND ("../../racc/talk/german/report0.wav");
         PRECACHE_SOUND ("../../racc/talk/german/report1.wav");
         PRECACHE_SOUND ("../../racc/talk/german/report2.wav");
         PRECACHE_SOUND ("../../racc/talk/german/report3.wav");
         PRECACHE_SOUND ("../../racc/talk/german/report4.wav");
         PRECACHE_SOUND ("../../racc/talk/german/reporting0.wav");
         PRECACHE_SOUND ("../../racc/talk/german/reporting1.wav");
         PRECACHE_SOUND ("../../racc/talk/german/reporting2.wav");
         PRECACHE_SOUND ("../../racc/talk/german/reporting3.wav");
         PRECACHE_SOUND ("../../racc/talk/german/reporting4.wav");
         PRECACHE_SOUND ("../../racc/talk/german/reporting5.wav");
         PRECACHE_SOUND ("../../racc/talk/german/reporting6.wav");
         PRECACHE_SOUND ("../../racc/talk/german/reporting7.wav");
         PRECACHE_SOUND ("../../racc/talk/german/reporting8.wav");
         PRECACHE_SOUND ("../../racc/talk/german/reporting9.wav");
         PRECACHE_SOUND ("../../racc/talk/german/reporting10.wav");
         PRECACHE_SOUND ("../../racc/talk/german/reporting11.wav");
         PRECACHE_SOUND ("../../racc/talk/german/reporting12.wav");
         PRECACHE_SOUND ("../../racc/talk/german/reporting13.wav");
         PRECACHE_SOUND ("../../racc/talk/german/reporting14.wav");
         PRECACHE_SOUND ("../../racc/talk/german/reporting15.wav");
         PRECACHE_SOUND ("../../racc/talk/german/reporting16.wav");
         PRECACHE_SOUND ("../../racc/talk/german/reporting17.wav");
         PRECACHE_SOUND ("../../racc/talk/german/seegrenade0.wav");
         PRECACHE_SOUND ("../../racc/talk/german/seegrenade1.wav");
         PRECACHE_SOUND ("../../racc/talk/german/seegrenade2.wav");
         PRECACHE_SOUND ("../../racc/talk/german/seegrenade3.wav");
         PRECACHE_SOUND ("../../racc/talk/german/takingdamage0.wav");
         PRECACHE_SOUND ("../../racc/talk/german/takingdamage1.wav");
         PRECACHE_SOUND ("../../racc/talk/german/takingdamage2.wav");
         PRECACHE_SOUND ("../../racc/talk/german/takingdamage3.wav");
         PRECACHE_SOUND ("../../racc/talk/german/takingdamage4.wav");
         PRECACHE_SOUND ("../../racc/talk/german/takingdamage5.wav");
         PRECACHE_SOUND ("../../racc/talk/german/takingdamage6.wav");
         PRECACHE_SOUND ("../../racc/talk/german/takingdamage7.wav");
         PRECACHE_SOUND ("../../racc/talk/german/throwgrenade0.wav");
         PRECACHE_SOUND ("../../racc/talk/german/throwgrenade1.wav");
         PRECACHE_SOUND ("../../racc/talk/german/throwgrenade2.wav");
         PRECACHE_SOUND ("../../racc/talk/german/throwgrenade3.wav");
         PRECACHE_SOUND ("../../racc/talk/german/throwgrenade4.wav");
         PRECACHE_SOUND ("../../racc/talk/german/victory0.wav");
         PRECACHE_SOUND ("../../racc/talk/german/victory1.wav");
         PRECACHE_SOUND ("../../racc/talk/german/victory2.wav");
         PRECACHE_SOUND ("../../racc/talk/german/victory3.wav");
         PRECACHE_SOUND ("../../racc/talk/german/victory4.wav");
      }
      else if (strcmp (language, "italian") == 0)
      {
         PRECACHE_SOUND ("../../racc/talk/italian/affirmative0.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/affirmative1.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/affirmative2.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/affirmative3.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/affirmative4.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/affirmative5.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/alert0.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/alert1.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/alert2.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/attacking0.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/attacking1.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/attacking2.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/attacking3.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/attacking4.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/attacking5.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/firstspawn0.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/firstspawn1.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/firstspawn2.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/firstspawn3.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/firstspawn4.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/inposition0.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/inposition1.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/inposition2.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/inposition3.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/negative0.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/report0.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/report1.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/report2.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/report3.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/report4.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/reporting0.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/reporting1.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/reporting2.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/reporting3.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/reporting4.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/reporting5.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/reporting6.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/reporting7.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/reporting8.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/reporting9.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/reporting10.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/reporting11.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/reporting12.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/reporting13.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/reporting14.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/reporting15.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/reporting16.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/reporting17.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/seegrenade0.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/seegrenade1.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/seegrenade2.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/seegrenade3.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/takingdamage0.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/takingdamage1.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/takingdamage2.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/takingdamage3.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/takingdamage4.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/takingdamage5.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/takingdamage6.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/takingdamage7.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/throwgrenade0.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/throwgrenade1.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/throwgrenade2.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/throwgrenade3.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/throwgrenade4.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/victory0.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/victory1.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/victory2.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/victory3.wav");
         PRECACHE_SOUND ("../../racc/talk/italian/victory4.wav");
      }
      else if (strcmp (language, "spanish") == 0)
      {
         PRECACHE_SOUND ("../../racc/talk/spanish/affirmative0.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/affirmative1.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/affirmative2.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/affirmative3.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/affirmative4.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/affirmative5.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/alert0.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/alert1.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/alert2.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/attacking0.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/attacking1.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/attacking2.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/attacking3.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/attacking4.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/attacking5.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/firstspawn0.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/firstspawn1.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/firstspawn2.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/firstspawn3.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/firstspawn4.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/inposition0.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/inposition1.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/inposition2.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/inposition3.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/negative0.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/report0.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/report1.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/report2.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/report3.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/report4.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/reporting0.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/reporting1.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/reporting2.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/reporting3.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/reporting4.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/reporting5.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/reporting6.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/reporting7.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/reporting8.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/reporting9.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/reporting10.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/reporting11.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/reporting12.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/reporting13.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/reporting14.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/reporting15.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/reporting16.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/reporting17.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/seegrenade0.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/seegrenade1.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/seegrenade2.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/seegrenade3.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/takingdamage0.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/takingdamage1.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/takingdamage2.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/takingdamage3.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/takingdamage4.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/takingdamage5.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/takingdamage6.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/takingdamage7.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/throwgrenade0.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/throwgrenade1.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/throwgrenade2.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/throwgrenade3.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/throwgrenade4.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/victory0.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/victory1.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/victory2.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/victory3.wav");
         PRECACHE_SOUND ("../../racc/talk/spanish/victory4.wav");
      }
   }
}


void SaveDoorsOrigins (void)
{
   edict_t *pEntity = NULL;
   edict_t *pFakeEntity = NULL;

   // loop through all slide doors
   while ((pEntity = UTIL_FindEntityByClassname (pEntity, "func_door")) != NULL)
   {
      if (strcmp (STRING (pEntity->v.netname), "secret_door") == 0)
         continue; // skip secret doors

      pFakeEntity = CREATE_NAMED_ENTITY (MAKE_STRING ("info_target")); // create door origin entity
      DispatchSpawn (pFakeEntity); // spawn it
      pFakeEntity->v.origin = VecBModelOrigin (pEntity); // same origin as door, obviously
      pFakeEntity->v.takedamage = DAMAGE_NO; // doesn't allow it to take damage
      pFakeEntity->v.solid = SOLID_NOT; // make it invisible
      pFakeEntity->v.movetype = MOVETYPE_NOCLIP; // no clip
      pFakeEntity->v.classname = MAKE_STRING ("door_origin"); // sets a name for it
      pFakeEntity->v.rendermode = kRenderNormal; // normal rendering mode
      pFakeEntity->v.renderfx = kRenderFxNone; // no special FX
      pFakeEntity->v.renderamt = 0; // ???
      pFakeEntity->v.owner = pEntity; // sets the real door as the owner of the origin entity
      SET_MODEL (pFakeEntity, "models/mechgibs.mdl"); // sets it a model
   }

   // loop through all rotating doors
   while ((pEntity = UTIL_FindEntityByClassname (pEntity, "func_door_rotating")) != NULL)
   {
      if (strcmp (STRING (pEntity->v.netname), "secret_door") == 0)
         continue; // skip secret doors

      pFakeEntity = CREATE_NAMED_ENTITY (MAKE_STRING ("info_target")); // create door origin entity
      DispatchSpawn (pFakeEntity); // spawn it
      pFakeEntity->v.origin = VecBModelOrigin (pEntity); // same origin as door, obviously
      pFakeEntity->v.takedamage = DAMAGE_NO; // doesn't allow it to take damage
      pFakeEntity->v.solid = SOLID_NOT; // make it invisible
      pFakeEntity->v.movetype = MOVETYPE_NOCLIP; // no clip
      pFakeEntity->v.classname = MAKE_STRING ("door_origin"); // sets a name for it
      pFakeEntity->v.rendermode = kRenderNormal; // normal rendering mode
      pFakeEntity->v.renderfx = kRenderFxNone; // no special FX
      pFakeEntity->v.renderamt = 0; // ???
      pFakeEntity->v.owner = pEntity; // sets the real door as the owner of the origin entity
      SET_MODEL (pFakeEntity, "models/mechgibs.mdl"); // sets it a model
   }

   return;
}


void LoadSymbols (char *filename)
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
   char function_name[256];
   int i;
   void *game_GiveFnptrsToDll;

   for (i = 0; i < num_ordinals; i++)
      p_FunctionNames[i] = NULL; // reset function names array

   fp = fopen (filename, "rb"); // open MOD DLL file in binary read mode
   fread (&dos_header, sizeof (dos_header), 1, fp); // get the DOS header

   fseek (fp, dos_header.e_lfanew, SEEK_SET);
   fread (&nt_signature, sizeof (nt_signature), 1, fp); // get the NT signature
   fread (&pe_header, sizeof (pe_header), 1, fp); // get the PE header
   fread (&optional_header, sizeof (optional_header), 1, fp); // get the optional header

   edata_offset = optional_header.DataDirectory[0].VirtualAddress; // no edata by default
   edata_delta = 0L; 

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
         GetFunctionName (function_name, fp); // get the string, strip it for the HL engine
         p_FunctionNames[i] = (char *) malloc (strlen (function_name) + 1); // allocate space
         StripNameForEngine (p_FunctionNames[i], function_name); // and store it in the array
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


void GetFunctionName (char *name, FILE *fp)
{
   char ch;

   // while end of file is not reached
   while ((ch = fgetc (fp)) != EOF)
   {
      *name++ = ch; // store what is read in the name variable
      if (ch == 0)
         break; // return the name with the trailing \0
   }
}


void StripNameForEngine (char *out_name, char *in_name)
{
   char *pos;

   // is this a MSVC C++ mangled name ?
   if (in_name[0] == '?')
   {
      // reach the two first @@
      if ((pos = strstr (in_name, "@@")) != NULL)
      {
         strcpy (out_name, &in_name[1]); // strip off the leading '?'
         out_name[pos - in_name - 1] = 0; // terminate string at the "@@"
         return;
      }
   }

   // else no change needed
   else
      strcpy (out_name, in_name);
}


unsigned long FUNCTION_FROM_NAME (const char *pName)
{
   // return the address of a named function
   for (int i = 0; i < num_ordinals; i++)
      if (strcmp (pName, p_FunctionNames[i]) == 0)
         return p_Functions[p_Ordinals[i]] + base_offset;

   return 0L; // couldn't find the function name to return address
}


const char *NAME_FOR_FUNCTION (unsigned long function)
{
   // return the name of function at address
   for (int i = 0; i < num_ordinals; i++)
      if ((function - base_offset) == p_Functions[p_Ordinals[i]])
         return p_FunctionNames[i];

   return NULL; // couldn't find the function address to return name
}


void FakeClientCommand (edict_t *pFakeClient, char *command)
{
   int length, index = 0;

   if ((command == NULL) || (*command == 0))
      return; // if no argument, return

   strcpy (g_argv, command); // first copy the command to the argv field
   length = strlen (g_argv); // get length of command
   if (length > 0)
      if (g_argv[length - 1] == '\n')
         length--; // remove any final '\n'
   g_argv[length] = 0; // terminate the string

   fake_arg_count = 0; // let's now parse the line and count the different fields

   // count the number of arguments
   while (index < length)
   {
      while ((index < length) && (g_argv[index] == ' '))
         index++; // ignore spaces

      // is this field a group of words between quotes or a single word ?
      if (g_argv[index] == '"')
      {
         index++; // move one step further to bypass the quote
         while ((index < length) && (g_argv[index] != '"'))
            index++; // reach end of field
         index++; // move one step further to bypass the quote
      }
      else
         while ((index < length) && (g_argv[index] != ' '))
            index++; // reach end of field

      fake_arg_count++; // we have processed one argument more
   }

   isFakeClientCommand = TRUE; // set the fakeclient flag
   ClientCommand (pFakeClient); // tell now the MOD DLL to execute the ClientCommand...
   g_argv[0] = 0; // when it's done, reset the g_argv field
   isFakeClientCommand = FALSE; // reset the fakeclient flag
   fake_arg_count = 0; // and the argument count
}


int Cmd_Argc (void)
{
   if (isFakeClientCommand)
      return fake_arg_count; // return the argument count

   return (*g_engfuncs.pfnCmd_Argc) ();
}


const char *Cmd_Args (void)
{
   if (isFakeClientCommand)
      return &g_argv[0]; // return the whole argument string

   return (*g_engfuncs.pfnCmd_Args) ();
}


const char *Cmd_Argv (int argc)
{
   if (isFakeClientCommand)
      return (GetArg (g_argv, argc)); // returns the wanted argument

   return (*g_engfuncs.pfnCmd_Argv) (argc);
}


const char *GetArg (const char *command, int arg_number)
{
   int length, i, index = 0, arg_count = 0, fieldstart, fieldstop;

   arg[0] = 0; // reset arg
   length = strlen (command); // get length of command

   // while we have not reached end of line
   while ((index < length) && (arg_count <= arg_number))
   {
      while ((index < length) && (command[index] == ' '))
         index++; // ignore spaces

      // is this field multi-word between quotes or single word ?
      if (command[index] == '"')
      {
         index++; // move one step further to bypass the quote
         fieldstart = index; // save field start position
         while ((index < length) && (command[index] != '"'))
            index++; // reach end of field
         fieldstop = index - 1; // save field stop position
         index++; // move one step further to bypass the quote
      }
      else
      {
         fieldstart = index; // save field start position
         while ((index < length) && (command[index] != ' '))
            index++; // reach end of field
         fieldstop = index - 1; // save field stop position
      }

      if (fieldstop > fieldstart + 127)
         fieldstop = fieldstart + 127; // avoid stack overflows

      // is this argument we just processed the wanted one ?
      if (arg_count == arg_number)
      {
         for (i = fieldstart; i <= fieldstop; i++)
            arg[i - fieldstart] = command[i]; // store the field value in a string
         arg[i - fieldstart] = 0; // terminate the string
      }

      arg_count++; // we have processed one argument more
   }

   return (&arg[0]); // returns the wanted argument
}


void UTIL_DisplaySpeakerIcon (bot_t *pBot, edict_t *pViewerClient, int duration)
{
   if ((pBot->pEdict == NULL) || (pViewerClient == NULL))
      return; // reliability check

   MESSAGE_BEGIN (MSG_ONE, SVC_TEMPENTITY, NULL, pViewerClient);
   WRITE_BYTE (TE_PLAYERATTACHMENT); // thanks to Count Floyd for the trick !
   WRITE_BYTE (ENTINDEX (pBot->pEdict)); // byte (entity index of pEdict)
   WRITE_COORD (voiceicon_height + pBot->pEdict->v.view_ofs.z + 34 * (pBot->BotMove.f_duck_time > gpGlobals->time)); // coord (vertical offset)
   WRITE_SHORT (speaker_texture); // short (model index of tempent)
   WRITE_SHORT (duration); // short (life * 10) e.g. 40 = 4 seconds
   MESSAGE_END (); 
}


void UTIL_DestroySpeakerIcon (bot_t *pBot, edict_t *pViewerClient)
{
   if ((pBot->pEdict == NULL) || (pViewerClient == NULL))
      return; // reliability check

   MESSAGE_BEGIN (MSG_ONE, SVC_TEMPENTITY, NULL, pViewerClient);
   WRITE_BYTE (TE_KILLPLAYERATTACHMENTS); // destroy all temporary entities attached to pBot
   WRITE_BYTE (ENTINDEX (pBot->pEdict)); // byte (entity index of pEdict)
   MESSAGE_END (); 
}


void UTIL_SendWelcomeMessage (void)
{
   edict_t *pClient = NULL;

   // loop through all players
   while (((pClient = UTIL_FindEntityByClassname (pClient, "player")) != NULL)
          && !FNullEnt (pClient) && !(pClient->v.flags & FL_FAKECLIENT))
   {
      // send the welcome message to this client
      (*g_engfuncs.pfnMessageBegin) (MSG_ONE, SVC_TEMPENTITY, NULL, pClient);
      (*g_engfuncs.pfnWriteByte) (TE_TEXTMESSAGE);
      (*g_engfuncs.pfnWriteByte) (1); // channel
      (*g_engfuncs.pfnWriteShort) (-8192); // x coordinates * 8192
      (*g_engfuncs.pfnWriteShort) (-8192); // y coordinates * 8192
      (*g_engfuncs.pfnWriteByte) (0); // effect (fade in/out)
      (*g_engfuncs.pfnWriteByte) (255); // initial RED
      (*g_engfuncs.pfnWriteByte) (255); // initial GREEN
      (*g_engfuncs.pfnWriteByte) (255); // initial BLUE
      (*g_engfuncs.pfnWriteByte) (1); // initial ALPHA
      (*g_engfuncs.pfnWriteByte) (255); // effect RED
      (*g_engfuncs.pfnWriteByte) (255); // effect GREEN
      (*g_engfuncs.pfnWriteByte) (255); // effect BLUE
      (*g_engfuncs.pfnWriteByte) (1); // effect ALPHA
      (*g_engfuncs.pfnWriteShort) (256); // fade-in time in seconds * 256
      (*g_engfuncs.pfnWriteShort) (512); // fade-out time in seconds * 256
      (*g_engfuncs.pfnWriteShort) (256); // hold time in seconds * 256
      (*g_engfuncs.pfnWriteString) (RACC_WELCOMETEXT); // send welcome message
      (*g_engfuncs.pfnMessageEnd) (); // end

      // play welcome sound on this client
      CLIENT_COMMAND (pClient, "play barney/guyresponsible.wav\n");
   }

   welcome_sent = TRUE;
}
