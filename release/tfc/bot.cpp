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
// TFC version
//
// bot.cpp
//

#include <sys/types.h>
#include <sys/stat.h>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "bot_common.h"
#include "bot_specific.h"

extern HINSTANCE h_Library;
extern bool is_dedicated_server;
extern edict_t *listenserver_edict;
extern bot_t bots[32];
extern char bot_names[100][32];
extern char bot_logos[100][32];
extern int bot_nationalities[100];
extern int bot_skills[100];
extern int number_names;
extern int team_allies[4];
extern bool b_observer_mode;
extern bool b_botdontshoot;
extern bool b_botdontfind;
extern bot_weapon_t weapon_defs[MAX_WEAPONS];



inline edict_t *CREATE_FAKE_CLIENT (const char *netname)
{
   return (*g_engfuncs.pfnCreateFakeClient) (netname);
}

inline char *GET_INFOBUFFER (edict_t *e)
{
   return (*g_engfuncs.pfnGetInfoKeyBuffer) (e);
}

inline char *GET_INFO_KEY_VALUE (char *infobuffer, char *key)
{
   return (g_engfuncs.pfnInfoKeyValue (infobuffer, key));
}

inline void SET_CLIENT_KEY_VALUE (int clientIndex, char *infobuffer, char *key, char *value)
{
   (*g_engfuncs.pfnSetClientKeyValue) (clientIndex, infobuffer, key, value);
}


// this is the LINK_ENTITY_TO_CLASS function that creates a player (bot)
void player (entvars_t *pev)
{
   static LINK_ENTITY_FUNC otherClassName = NULL;
   if (otherClassName == NULL)
      otherClassName = (LINK_ENTITY_FUNC) GetProcAddress (h_Library, "player");
   if (otherClassName != NULL)
      (*otherClassName) (pev);
}


void BotReset (bot_t *pBot)
{
   pBot->msecnum = 0;
   pBot->msecdel = 0.0;
   pBot->msecval = 0.0;

   pBot->bot_health = 0;
   pBot->bot_armor = 0;
   pBot->bot_weapons = 0;
   pBot->blinded_time = 0.0;

   pBot->BotMove.f_max_speed = CVAR_GET_FLOAT ("sv_maxspeed");

   pBot->f_find_item_time = 0.0;

   pBot->ladder_dir = LADDER_UNKNOWN;
   pBot->f_start_use_ladder_time = 0.0;
   pBot->f_end_use_ladder_time = 0.0;

   if (RANDOM_LONG (1, 100) < 33)
      pBot->b_is_fearful = TRUE;
   else
      pBot->b_is_fearful = FALSE;
   pBot->BotMove.b_is_walking = FALSE;
   pBot->BotMove.b_emergency_walkback = FALSE;
   pBot->BotMove.f_forward_time = 0.0;
   pBot->BotMove.f_backwards_time = 0.0;
   pBot->BotMove.f_jump_time = 0.0;
   pBot->BotMove.f_duck_time = 0.0;
   pBot->BotMove.f_strafeleft_time = 0.0;
   pBot->BotMove.f_straferight_time = 0.0;

   pBot->f_exit_water_time = 0.0;

   pBot->pBotEnemy = NULL;
   pBot->v_lastseenenemy_position = Vector (0, 0, 0);
   pBot->f_see_enemy_time = 0.0;
   pBot->f_lost_enemy_time = 0.0;
   pBot->f_aim_adjust_time = 0.0;
   pBot->f_reload_time = -1;
   pBot->f_throwgrenade_time = 0;
   pBot->pBotUser = NULL;
   pBot->f_bot_use_time = 0.0;
   pBot->f_randomturn_time = gpGlobals->time;
   pBot->BotChat.b_saytext_killed = FALSE;
   pBot->BotChat.b_saytext_help = FALSE;
   pBot->b_help_asked = FALSE;
   pBot->f_bot_saytext_time = 0.0;
   pBot->f_bot_sayaudio_time = 0.0;
   pBot->BotChat.b_saytext_killed = FALSE;
   pBot->BotChat.b_sayaudio_affirmative = FALSE;
   pBot->BotChat.b_sayaudio_alert = FALSE;
   pBot->BotChat.b_sayaudio_attacking = FALSE;
   pBot->BotChat.b_sayaudio_firstspawn = FALSE;
   pBot->BotChat.b_sayaudio_inposition = FALSE;
   pBot->BotChat.b_sayaudio_negative = FALSE;
   pBot->BotChat.b_sayaudio_report = FALSE;
   pBot->BotChat.b_sayaudio_reporting = FALSE;
   pBot->BotChat.b_sayaudio_seegrenade = FALSE;
   pBot->BotChat.b_sayaudio_takingdamage = FALSE;
   pBot->BotChat.b_sayaudio_throwgrenade = FALSE;
   pBot->BotChat.b_sayaudio_victory = FALSE;

   pBot->f_shoot_time = gpGlobals->time;
   pBot->f_primary_charging = -1.0;
   pBot->f_secondary_charging = -1.0;
   pBot->charging_weapon_id = 0;

   pBot->f_rush_time = gpGlobals->time + RANDOM_FLOAT (15.0, 45.0);
   pBot->f_pause_time = 0.0;
   pBot->f_sound_update_time = 0.0;

   pBot->f_find_goal_time = 0;
   pBot->v_goal = Vector (0, 0, 0);
   pBot->v_place_to_keep = Vector (0, 0, 0);
   pBot->f_place_time = 0;
   pBot->f_camp_time = 0;
   pBot->f_reach_time = 0;
   pBot->f_samplefov_time = 0;
   pBot->v_reach_point = Vector (0,0,0);
   pBot->f_turncorner_time = gpGlobals->time + 5.0;

   pBot->bot_order = BOT_ORDER_NOORDER;
   pBot->f_order_time = 0;
   pBot->b_interact = FALSE;
   pBot->f_interact_time = 0;
   pBot->b_lift_moving = FALSE;
   pBot->f_spraying_logo_time = 0;
   pBot->b_logo_sprayed = FALSE;

   memset (&(pBot->current_weapon), 0, sizeof (pBot->current_weapon));
   memset (&(pBot->m_rgAmmo), 0, sizeof (pBot->m_rgAmmo));

   // if this bot slot has an associated edict (i.e. this bot "exists" for the engine)
   if (pBot->pEdict != NULL)
   {
      float spawn_angle = RANDOM_FLOAT (-180, 180); // choose a random spawn angle
      BotSetIdealPitch (pBot, 0); // reset pitch to 0 (level horizontally)
      BotSetIdealYaw (pBot, spawn_angle); // set his spawn angle
      BotSetViewAngles (pBot, Vector (0, spawn_angle, 0)); // set his view angles

      if (RANDOM_LONG (1,100) <= (56 - 2 * gpGlobals->maxClients))
      {
         pBot->BotChat.b_sayaudio_firstspawn = TRUE; // bot says 'go go go' or something like that
         pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.5, 10.0);
      }
   }
}


void BotCreate (const char *name, const char *logo, int nationality, int skill, int team, int bot_class)
{
   edict_t *pBotEdict;
   bot_t *pBot;
   char c_name[32];
   char c_logo[32];
   int i_nationality;
   int i_skill;
   int i_team;
   int i_class;
   int index, i;

   // first prevent some CVARs to reach forbidden values
   if (CVAR_GET_FLOAT ("racc_botforceteam") < -1)
      CVAR_SET_FLOAT ("racc_botforceteam", -1); // force racc_botforceteam in bounds
   if (CVAR_GET_FLOAT ("racc_botforceteam") > 3)
      CVAR_SET_FLOAT ("racc_botforceteam", 3); // force racc_botforceteam in bounds
   if (CVAR_GET_FLOAT ("racc_defaultbotnationality") < 0)
      CVAR_SET_FLOAT ("racc_defaultbotnationality", 0); // force racc_defaultbotskill in bounds
   if (CVAR_GET_FLOAT ("racc_defaultbotnationality") > 4)
      CVAR_SET_FLOAT ("racc_defaultbotnationality", 4); // force racc_defaultbotskill in bounds
   if (CVAR_GET_FLOAT ("racc_defaultbotskill") < 1)
      CVAR_SET_FLOAT ("racc_defaultbotskill", 1); // force racc_defaultbotskill in bounds
   if (CVAR_GET_FLOAT ("racc_defaultbotskill") > 5)
      CVAR_SET_FLOAT ("racc_defaultbotskill", 5); // force racc_defaultbotskill in bounds

   // have we been specified parameters ?
   if ((name != NULL) && (*name != 0))
   {
      strcpy (c_name, name); // we have a name, so use it

      if ((logo != NULL) && (*logo != 0))
         strcpy (c_logo, logo); // if we have a logo, use it too
      else
         strcpy (c_logo, "lambda.bmp"); // else use the default logo

      if ((nationality >= 0) && (nationality <= 4))
         i_nationality = nationality; // if we have a nationality, use it too
      else
         i_nationality = CVAR_GET_FLOAT ("racc_defaultbotnationality"); // else use the default nationality

      if ((skill >= 1) && (skill <= 5))
         i_skill = skill; // if we have a skill, use it too
      else
         i_skill = CVAR_GET_FLOAT ("racc_defaultbotskill"); // else use the default skill

      if ((skill >= 1) && (skill <= 5))
         i_team = team; // if we have a team, use it too
      else
         i_team = -1; // else use auto-assign

      if ((skill >= 1) && (skill <= 5))
         i_class = bot_class; // if we have a class, use it too
      else
         i_class = -1; // else use auto-assign
   }

   // else have we a list ?
   else if (number_names > 0)
   {
      // if so, see the names that are used and add one that is not used yet
      int count_used = 0;
      bool bot_names_used[100];

      // reset used names flag array
      for (index = 0; index < number_names; index++)
         bot_names_used[index] = FALSE;

      // cycle all bot slots
      for (i = 0; i < 32; i++)
      {
         // is this bot active ?
         if (bots[i].is_active && (bots[i].pEdict != NULL))
         {
            // cycle all names slots
            for (index = 0; index < number_names; index++)
            {
               // does the bot have the same name that this name slot ?
               if (strcmp (STRING (bots[i].pEdict->v.netname), bot_names[index]) == 0)
               {
                  bot_names_used[index] = TRUE; // this name is used, so flag it
                  count_used++; // increment the used names counter
               }
            }
         }
      }

      // if all the names are used, revert them to non-used
      if (count_used == number_names)
         for (index = 0; index < number_names; index++)
            bot_names_used[index] = FALSE;

      // pick up one that isn't
      do index = RANDOM_LONG (0, number_names - 1);
      while (bot_names_used[index]);

      // copy it to c_name, and the according logo and skill to c_logo and i_skill
      strcpy (c_name, bot_names[index]);
      strcpy (c_logo, bot_logos[index]);
      i_nationality = bot_nationalities[index];
      i_skill = bot_skills[index];

      // use auto-assign for team and class
      i_team = -1;
      i_class = -1;
   }

   // else use defaults
   else
   {
      strcpy (c_name, "Bot");
      strcpy (c_logo, "lambda.bmp");
      i_nationality = CVAR_GET_FLOAT ("racc_defaultbotnationality");
      i_skill = CVAR_GET_FLOAT ("racc_defaultbotskill");
      i_team = -1;
      i_class = -1;
   }

   // okay, now we have a name, a skin and a skill for our new bot

   // if fake client creation is successful...
   if (!FNullEnt (pBotEdict = CREATE_FAKE_CLIENT (c_name)))
   {
      char ptr[128]; // allocate space for message from ClientConnect
      int index = 0;

      if (pBotEdict->pvPrivateData != NULL)
         FREE_PRIVATE (pBotEdict); // free our predecessor's private data
      pBotEdict->pvPrivateData = NULL; // fools the private data pointer 
      pBotEdict->v.frags = 0; // reset his frag count 

      // create the player entity by calling MOD's player() function
      player (VARS (pBotEdict));

      // set him a default skin in the infobuffer
      SET_CLIENT_KEY_VALUE (ENTINDEX (pBotEdict), GET_INFOBUFFER (pBotEdict), "model", "helmet");

      // let him connect to the server under the name c_name
      ClientConnect (pBotEdict, c_name, "127.0.0.1", ptr);

      // print a notification message on the dedicated server console if in developer mode
      if ((is_dedicated_server) && (CVAR_GET_FLOAT ("developer") > 0))
      {
         if (CVAR_GET_FLOAT ("developer") > 1)
         {
            UTIL_ServerConsole_printf ("Server requiring authentication\n");
            UTIL_ServerConsole_printf ("Client %s connected\n", STRING (pBotEdict->v.netname));
            UTIL_ServerConsole_printf ("Adr: 127.0.0.1:27005\n");
         }
         UTIL_ServerConsole_printf ("Verifying and uploading resources...\n");
         UTIL_ServerConsole_printf ("Custom resources total 0 bytes\n");
         UTIL_ServerConsole_printf ("  Decals:  0 bytes\n");
         UTIL_ServerConsole_printf ("----------------------\n");
         UTIL_ServerConsole_printf ("Resources to request: 0 bytes\n");
      }

      // let him actually join the game
      ClientPutInServer (pBotEdict);

      // find a free slot for this bot
      while ((bots[index].is_active) && (index < 32))
         index++;

      // link his entity to an useful pointer
      pBot = &bots[index];
      pBot->pEdict = pBotEdict;

      // initialize all the variables for this bot...

      BotReset (pBot); // reset our bot for new round

      pBot->pEdict->v.flags |= FL_FAKECLIENT; // set the fake client flag
      pBot->is_active = TRUE; // set his 'is active' flag
      pBot->bot_nationality = i_nationality; // save his nationality
      pBot->bot_skill = i_skill; // save his skill
      pBot->bot_team = i_team; // save his team
      pBot->bot_class = i_class; // save his class
      pBot->pEdict->v.pitch_speed = BOT_PITCH_SPEED; // set the vertical speed at which he will turn
      pBot->pEdict->v.yaw_speed = BOT_YAW_SPEED; // set the horizontal speed at which he will turn

      // if internet mode is on...
      if (CVAR_GET_FLOAT ("racc_internetmode") > 0)
      {
         pBot->time_to_live = gpGlobals->time + RANDOM_LONG (300, 3600); // set him a TTL
         pBot->quit_game_time = pBot->time_to_live + RANDOM_FLOAT (3.0, 7.0); // disconnect time
      }
      else
      {
         pBot->time_to_live = -1; // don't set him a TTL (time to live)
         pBot->quit_game_time = -1; // so never quit
      }

      // say hello here
      if (RANDOM_LONG (1, 100) <= (86 - 2 * gpGlobals->maxClients))
      {
         pBot->BotChat.b_saytext_hello = TRUE;
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (1.0, 2.0);
      }

      pBot->f_bot_alone_timer = gpGlobals->time + RANDOM_LONG (30, 120); // set an idle delay
      pBot->b_not_started = TRUE; // tells bot to go and select team and class
   }
}


bool BotCheckForSpecialZones (bot_t *pBot)
{
   edict_t *pSpecialZone = NULL;

   if (pBot->pEdict == NULL)
      return FALSE; // reliability check

   if (b_botdontfind)
      return FALSE; // don't process if botdontfind is set

   // is there a special zone near here?
   while ((pSpecialZone = UTIL_FindEntityInSphere (pSpecialZone, pBot->pEdict->v.origin, 1000)) != NULL)
   {
      // check for a visible dropped flag
      if ((strcmp ("item_tfgoal", STRING (pSpecialZone->v.classname)) == 0)
          && (BotCanSeeThis (pBot, pSpecialZone->v.origin))
          && (FInViewCone (pSpecialZone->v.origin, pBot->pEdict))
          && ENT_IS_ON_FLOOR (pSpecialZone))
      {
         // is it our flag ?
         if (GetTeam (pBot->pEdict) == GetTeam (pSpecialZone))
         {
            BotReachPosition (pBot, pSpecialZone->v.origin); // go pick it up
            return TRUE; // bot is concerned by this special zone
         }
      }

      // check for a visible flag target
      else if ((strcmp ("info_tfgoal", STRING (pSpecialZone->v.classname)) == 0)
               && (BotCanSeeThisBModel (pBot, pSpecialZone))
               && (FInViewCone (VecBModelOrigin (pSpecialZone), pBot->pEdict)))
      {
         // both teams will head to the flag site. Bots will so "cruise" around
         // the flag, permanently looking for enemies.

         // let's run to that item if previous goal was not visible for more than 30s
         if (pBot->f_reach_time + 30 < gpGlobals->time)
         {
            BotReachPosition (pBot, VecBModelOrigin (pSpecialZone));
            return TRUE; // bot is concerned by this special zone
         }
      }
   }

   return FALSE; // bot found nothing interesting
}


bool BotCheckForGrenades (bot_t *pBot)
{
   edict_t *pGrenade = NULL;

   if (pBot->pEdict == NULL)
      return FALSE; // reliability check

   if (b_botdontfind)
      return FALSE; // don't process if botdontfind is set

   // is there an armed grenade near here?
   while ((pGrenade = UTIL_FindEntityInSphere (pGrenade, pBot->pEdict->v.origin, 300)) != NULL)
   {
      // check if entity is an armed grenade
      if ((strstr (STRING (pGrenade->v.classname), "grenade") != NULL)
          && (BotCanSeeThis (pBot, pGrenade->v.origin))
          && (FInViewCone (pGrenade->v.origin, pBot->pEdict)))
      {
         Vector bot_angles = UTIL_VecToAngles (pGrenade->v.origin - pBot->pEdict->v.origin);
         BotSetIdealYaw (pBot, bot_angles.y); // face the grenade...

         // ... and run away!!
         pBot->BotMove.f_backwards_time = gpGlobals->time + 0.5; // until the grenade explodes

         // reliability check: the v.owner entvars_t slot of pGrenade may be unregistered...
         if (pGrenade->v.owner != NULL)
         {
            // check if this grenade is our enemies'...
            int grenade_team = GetTeam (pGrenade->v.owner);
            int bot_team = GetTeam (pBot->pEdict);

            // if so, yell
            if ((bot_team != grenade_team) && !(team_allies[bot_team] & (1 << grenade_team)))
            {
               pBot->BotChat.b_sayaudio_seegrenade = TRUE; // bot says 'danger'
               pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.7, 1.5);
            }
         }

         return TRUE; // bot is concerned by this grenade
      }
   }

   return FALSE; // bot found nothing interesting
}


void BotCheckForItems (bot_t *pBot)
{
   edict_t *pent = NULL;
   edict_t *pPickupEntity = NULL;
   Vector pickup_origin;
   Vector entity_origin;
   bool can_pickup;
   float min_distance = 501;
   TraceResult tr;
   Vector vecStart;
   Vector vecEnd;
   int angle_to_entity;

   if (pBot->pEdict == NULL)
      return; // reliability check

   if (b_botdontfind)
      return; // don't process if botdontfind is set

   pBot->b_is_picking_item = FALSE;

   while ((pent = UTIL_FindEntityInSphere (pent, pBot->pEdict->v.origin, 5000)) != NULL)
   {
      can_pickup = FALSE; // assume can't use it until known otherwise

      // see if this is a "func_" type of entity (func_button, etc.)...
      if (strncmp ("func_", STRING (pent->v.classname), 5) == 0)
      {
         // BModels have 0,0,0 for origin so must use VecBModelOrigin...
         entity_origin = VecBModelOrigin (pent);

         vecStart = pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs;
         vecEnd = entity_origin;

         angle_to_entity = BotAngleToLocation (pBot, vecEnd - vecStart);

         // check if entity is outside field of view (+/- 60 degrees)
         if (angle_to_entity > 60)
            continue; // skip this item if bot can't "see" it

         // check if entity is a ladder (ladders are a special case)
         if (strcmp ("func_ladder", STRING (pent->v.classname)) == 0)
         {
            // force ladder origin to same z coordinate as bot since
            // the VecBModelOrigin is the center of the ladder.  For
            // LONG ladders, the center MAY be hundreds of units above
            // the bot.  Fake an origin at the same level as the bot...

            entity_origin.z = pBot->pEdict->v.origin.z;
            vecEnd = entity_origin;

            // trace a line from bot's eyes to func_ladder entity...
            UTIL_TraceLine (vecStart, vecEnd, ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

            // check if traced all the way up to the entity (didn't hit wall)
            if (tr.flFraction >= 1.0)
            {
               // always use the ladder if haven't used a ladder in at least 5 seconds...
               if (pBot->f_end_use_ladder_time + 5.0 < gpGlobals->time)
                  can_pickup = TRUE;
            }
         }
         else
         {
            // trace a line from bot's eyes to entity
            UTIL_TraceLine (vecStart, vecEnd, ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);
            
            // check if traced NEARLY all the way up to the entity
            if ((tr.flFraction > 0.8) && (tr.flFraction < 1.0))
            {
               // check if entity is stuff that can be blowed up by a some plastic...
               if ((strcmp ("func_wall_toggle", STRING (pent->v.classname)) == 0)
                        && (pBot->pEdict->v.playerclass == TFC_CLASS_DEMOMAN))
               {
                  // if close enough, drop plastic
                  if ((vecEnd - vecStart).Length () < 80)
                     FakeClientCommand (pBot->pEdict, "detstart 5");
                  
                  can_pickup = TRUE;
               }
            }

            // else if traced all the way up to the entity (didn't hit wall)
            else if (strcmp (STRING (pent->v.classname), STRING (tr.pHit->v.classname)) == 0)
            {
               // find distance to item for later use...
               float distance = (vecEnd - vecStart).Length ();

               // check if entity is a breakable...
               if ((strcmp ("func_breakable", STRING (pent->v.classname)) == 0)
                        && (pent->v.takedamage != DAMAGE_NO) && (pent->v.health > 0)
                        && !(pent->v.flags & FL_WORLDBRUSH) && (abs (vecEnd.z - vecStart.z) < 60))
               {
                  // check if close enough...
                  if (distance < 50)
                  {
                     if ((pBot->current_weapon.iId != TF_WEAPON_SPANNER)
                         || (pBot->current_weapon.iId != TF_WEAPON_KNIFE)
                         || (pBot->current_weapon.iId != TF_WEAPON_AXE))
                     {
                        // select a proximity weapon
                        if (pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
                           BotSelectItem (pBot, "tf_weapon_spanner");
                        else if (pBot->pEdict->v.playerclass == TFC_CLASS_SPY)
                           BotSelectItem (pBot, "tf_weapon_knife");
                        else
                           BotSelectItem (pBot, "tf_weapon_axe");
                     }
                     else
                     {
                        // point the weapon at the breakable and strike it
                        BotPointGun (pBot, UTIL_VecToAngles (vecEnd - GetGunPosition (pBot->pEdict)));
                        pBot->pEdict->v.button |= IN_ATTACK; // strike the breakable
                        pBot->f_reload_time = gpGlobals->time + RANDOM_LONG (1.5, 3.0); // set next time to reload
                     }
                  }

                  can_pickup = TRUE;
               }
            }
         }
      }
      else  // everything else...
      {
         entity_origin = pent->v.origin;

         vecStart = pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs;
         vecEnd = entity_origin;

         // find angles from bot origin to entity...
         angle_to_entity = BotAngleToLocation (pBot, vecEnd - vecStart);

         // check if entity is outside field of view (+/- 60 degrees)
         if (angle_to_entity > 60)
            continue;  // skip this item if bot can't "see" it

         // check if line of sight to object is not blocked (i.e. visible)
         if (BotCanSeeThis (pBot, vecEnd))
         {
            // check if entity is a weapon...
            if (strncmp ("weapon_", STRING (pent->v.classname), 7) == 0)
            {
               if (pent->v.effects & EF_NODRAW)
                  continue; // someone owns this weapon or it hasn't respawned yet

               can_pickup = TRUE;
            }

            // check if entity is ammo...
            else if (strncmp ("ammo_", STRING (pent->v.classname), 5) == 0)
            {
               if (pent->v.effects & EF_NODRAW)
                  continue; // the item is not visible (i.e. has not respawned)

               can_pickup = TRUE;
            }

            // check if entity is a healthkit...
            else if (strcmp ("item_healthkit", STRING (pent->v.classname)) == 0)
            {
               if (pent->v.effects & EF_NODRAW)
                  continue; // the item is not visible (i.e. has not respawned)

               // check if the bot can use this item...
               if (pBot->pEdict->v.health < 100)
                  can_pickup = TRUE;
            }

            // check if entity is a packed up weapons box...
            else if (strcmp ("weaponbox", STRING (pent->v.classname)) == 0)
               can_pickup = TRUE;

            // check if entity is the spot from laser
            else if (strcmp ("laser_spot", STRING (pent->v.classname)) == 0)
            {
            }

            // check if entity is a RPG rocket
            else if (strcmp ("tf_rpg_rocket", STRING (pent->v.classname)) == 0)
            {
               TraceResult tr;

               // choose a secure strafe direction, else walk backwards
               if (!BotCanFallOnTheLeft (pBot))
                  pBot->BotMove.f_strafeleft_time = gpGlobals->time + RANDOM_FLOAT (0.5, 2.0);
               else if (!BotCanFallOnTheRight (pBot))
                  pBot->BotMove.f_straferight_time = gpGlobals->time + RANDOM_FLOAT (0.5, 2.0);
               else
                  pBot->BotMove.f_backwards_time = gpGlobals->time + RANDOM_FLOAT (0.5, 2.0);
            }
         } // end if object is visible
      } // end else not "func_" entity

      if (can_pickup) // if the bot found something it can pickup...
      {
         float distance = (entity_origin - pBot->pEdict->v.origin).Length ();

         // see if it's the closest item so far...
         if (distance < min_distance)
         {
            min_distance = distance; // update the minimum distance
            pPickupEntity = pent; // remember this entity
            pickup_origin = entity_origin; // remember location of entity
         }
      }
   } // end while loop

   if (pPickupEntity != NULL)
   {
      pBot->b_is_picking_item = TRUE; // set bot picking item flag
      pBot->v_reach_point = pickup_origin; // save the location of item bot is trying to get
   }
   else
      pBot->b_is_picking_item = FALSE; // reset picking item flag
}


void BotThink (bot_t *pBot)
{
   float pitch_degrees;
   float yaw_degrees;

   if (pBot->pEdict == NULL)
      return; // reliability check

   pBot->pEdict->v.flags |= FL_FAKECLIENT;

   if (pBot->msecdel <= gpGlobals->time)
   {
      pBot->msecdel = gpGlobals->time + 0.5;
      if (pBot->msecnum > 0)
         pBot->msecval = 450.0 / pBot->msecnum;
      pBot->msecnum = 0;
   }
   else
      pBot->msecnum++;

   if (pBot->msecval < 1) // don't allow msec to be less than 1...
      pBot->msecval = 1;

   if (pBot->msecval > 100) // ...or greater than 100
      pBot->msecval = 100;

   pBot->pEdict->v.button = 0; // reset buttons pressed
   pBot->BotMove.f_move_speed = 0; // reset move_speed
   pBot->BotMove.f_strafe_speed = 0; // reset strafe_speed

   // handle bot speaking stuff
   BotSayText (pBot);
   BotSayAudio (pBot);

   // handle bot moving stuff
   BotMove (pBot);

   // if the bot hasn't selected stuff to start the game yet, go do that...
   if (pBot->b_not_started)
   {
      BotStartGame (pBot);
      g_engfuncs.pfnRunPlayerMove (pBot->pEdict, UTIL_WrapAngles (pBot->pEdict->v.v_angle), 0, 0, 0, 0, 0, pBot->msecval);
      return;
   }

   // is it time for the bot to leave the game? (depending on his time to live)
   if ((pBot->time_to_live > 0) && (pBot->time_to_live <= gpGlobals->time))
   {
      pBot->time_to_live = gpGlobals->time + 6.0; // don't say it twice (bad hack)
      pBot->f_pause_time = gpGlobals->time + 10.0; // stop the bot while he is leaving

      if (RANDOM_LONG (1, 100) <= (66 - 2 * gpGlobals->maxClients))
      {
         pBot->BotChat.b_saytext_bye = TRUE; // say goodbye
         pBot->f_bot_saytext_time = gpGlobals->time;
      }
   }

   // if the bot is dead, press fire to respawn...
   if (!IsAlive (pBot->pEdict))
   {
      BotReset (pBot); // reset our bot

      // was the bot killed by another player ?
      if (pBot->pKillerEntity != NULL)
      {
         if (RANDOM_LONG (1, 100) <= (56 - 2 * gpGlobals->maxClients))
         {
            pBot->BotChat.b_saytext_killed = TRUE;
            pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (5.0, 10.0);
         }
      }

      if (RANDOM_LONG (1, 100) <= 50)
         pBot->pEdict->v.button = IN_ATTACK;

      g_engfuncs.pfnRunPlayerMove (pBot->pEdict, Vector (0, 0, 0), 0, 0, 0, pBot->pEdict->v.button, 0, pBot->msecval);
      return;
   }

   // should the bot complain of being alone for a long time ?
   if ((pBot->f_bot_alone_timer > 0) && (pBot->f_bot_alone_timer <= gpGlobals->time))
   {
      pBot->f_bot_alone_timer = gpGlobals->time + RANDOM_LONG (30, 120); // sets new delay

      if (RANDOM_LONG (1, 100) <= (66 - 2 * gpGlobals->maxClients))
      {
         pBot->BotChat.b_saytext_idle = TRUE; // complain
         pBot->f_bot_saytext_time = gpGlobals->time;
      }

      // bot must be tired of hunting, so let him camp for a while around here...
      if (!((pBot->current_weapon.iId == TF_WEAPON_MEDIKIT)
            || (pBot->current_weapon.iId == TF_WEAPON_SPANNER)
            || (pBot->current_weapon.iId == TF_WEAPON_AXE)
            || (pBot->current_weapon.iId == TF_WEAPON_GL)
            || (pBot->current_weapon.iId == TF_WEAPON_FLAMETHROWER)
            || (pBot->current_weapon.iId == TF_WEAPON_IC)
            || (pBot->current_weapon.iId == TF_WEAPON_TRANQ)
            || (pBot->current_weapon.iId == TF_WEAPON_PL)
            || (pBot->current_weapon.iId == TF_WEAPON_KNIFE)
            || (pBot->current_weapon.iId == TF_WEAPON_GRENADE)))
         BotCanCampNearHere (pBot, pBot->pEdict->v.origin);

      // yeah, i'm lazy.

      // if bot is an engineer and has enough metal, drop a sentry gun here to waste some time
      if ((pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
          && (pBot->m_rgAmmo[weapon_defs[TF_WEAPON_SPANNER].iAmmo1] > 130))
      {
         FakeClientCommand (pBot->pEdict, "build 2");
         FakeClientCommand (pBot->pEdict, "rotatesentry180");
      }

      // else if bot is a spy, let's have carnival to scare our teammates
      else if (pBot->pEdict->v.playerclass == TFC_CLASS_SPY)
      {
         char command[32];
         sprintf (command, "disguise_enemy %d", RANDOM_LONG (1, 9)); // pickup random class
         FakeClientCommand (pBot->pEdict, command); // issue the command
      }
   }

   // should the bot yell for backup ?
   if (!pBot->b_help_asked && (pBot->pBotEnemy != NULL) && (pBot->bot_health <= 20))
      if (RANDOM_LONG (1, 100) <= (91 - 2 * gpGlobals->maxClients))
      {
         pBot->BotChat.b_saytext_help = TRUE; // yell
         pBot->f_bot_saytext_time = gpGlobals->time;
         pBot->b_help_asked = TRUE; // don't do it twice
      }

   // is it time to call for medic ?
   if (pBot->b_help_asked && (pBot->bot_health <= 20)
       && (pBot->f_reload_time > 0) && (pBot->f_reload_time < gpGlobals->time))
   {
      pBot->f_reload_time = 0;
      pBot->BotMove.b_is_walking = TRUE;
	   FakeClientCommand (pBot->pEdict, "saveme"); // call for medic
   }

   // has the bot been ordered something ?
   if ((pBot->bot_order != BOT_ORDER_NOORDER) && (pBot->f_order_time + 1.0 < gpGlobals->time))
      BotAnswerToOrder (pBot); // answer to this order

   // is the bot blinded ?
   if (pBot->blinded_time > gpGlobals->time)
   {
      if (pBot->idle_angle_time <= gpGlobals->time)
      {
         pBot->idle_angle_time = gpGlobals->time + RANDOM_FLOAT (0.5, 2.0);
         BotSetIdealYaw (pBot, pBot->idle_angle + RANDOM_FLOAT (-20, 20));
      }

      BotChangeYaw (pBot, pBot->pEdict->v.yaw_speed / 10); // turn slower towards ideal yaw
      pBot->BotMove.f_duck_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.0); // duck when blinded

      // pick up a random strafe direction
      if (RANDOM_LONG (1, 100) < 50)
         pBot->BotMove.f_strafeleft_time = gpGlobals->time + 0.1;
      else
         pBot->BotMove.f_straferight_time = gpGlobals->time + 0.1;

      if (RANDOM_LONG (0, 100) < 50)
         pBot->BotMove.b_emergency_walkback = TRUE;

      g_engfuncs.pfnRunPlayerMove (pBot->pEdict, UTIL_WrapAngles (pBot->pEdict->v.v_angle), pBot->BotMove.f_move_speed, pBot->BotMove.f_strafe_speed, 0, pBot->pEdict->v.button, 0, pBot->msecval);
      return;
   }
   else
      pBot->idle_angle = UTIL_WrapAngle (pBot->pEdict->v.v_angle.y); // the bot is not idle

   // check if time to check for player sounds (if don't already have enemy)
   if ((pBot->f_sound_update_time <= gpGlobals->time) && (pBot->pBotEnemy == NULL) && (!b_botdontshoot))
   {
      int ind;
      edict_t *pPlayer;

      pBot->f_sound_update_time = gpGlobals->time + 1.0;

      for (ind = 1; ind <= gpGlobals->maxClients; ind++)
      {
         pPlayer = INDEXENT (ind);

         if (!pPlayer || pPlayer->free || (pPlayer == pBot->pEdict))
            continue; // skip invalid players and skip self (i.e. this bot)

         if (b_observer_mode && (pPlayer->v.flags & FL_CLIENT))
            continue; // if observer mode enabled, don't listen to real players...

         // if this player is alive AND not teammate nor allie AND human-like
         if (IsAlive (pPlayer) && !((GetTeam (pBot->pEdict) == GetTeam (pPlayer)) || (team_allies[GetTeam (pBot->pEdict)] & (1 << GetTeam (pPlayer))))
             && ((pPlayer->v.flags & FL_CLIENT) || (pPlayer->v.flags & FL_FAKECLIENT)))
         {
            // check for sounds being made by other players...
            if (BotCheckForSounds (pBot, pPlayer))
               pBot->f_sound_update_time = gpGlobals->time + 10.0; // next check in 10 seconds
         }
      }
   }

   // turn towards ideal pitch and ideal yaw by pitch_speed and yaw_speed degrees
   pitch_degrees = BotChangePitch (pBot, pBot->pEdict->v.pitch_speed);
   yaw_degrees = BotChangeYaw (pBot, pBot->pEdict->v.yaw_speed);

   if (yaw_degrees <= 1.0)
      pBot->b_is_walking_straight = TRUE;
   else
      pBot->b_is_walking_straight = FALSE;

   if ((pitch_degrees >= 10) || (yaw_degrees >= 10))
      pBot->BotMove.b_is_walking = TRUE; // slow down if turning a lot

   // let's look for enemies...
   if (b_botdontshoot)
      pBot->pBotEnemy = NULL; // botdontshoot is set, bot won't look for enemies
   else
      pBot->pBotEnemy = BotCheckForEnemies (pBot); // see if there are visible enemies

   // avoid walls, corners and teammates
   if (pBot->f_avoid_time < gpGlobals->time)
      BotAvoidObstacles (pBot);

   // are there armed grenades near us ?
   if (BotCheckForGrenades (pBot))
      pBot->BotMove.b_emergency_walkback = TRUE;
   else
      pBot->BotMove.b_emergency_walkback = FALSE;

   // does an enemy exist?
   if (pBot->pBotEnemy != NULL)
   {
      BotShootAtEnemy (pBot); // shoot at the enemy
      pBot->f_pause_time = 0; // dont't pause if enemy exists
   }

   // else did the bot just lost his enemy ?
   else if (pBot->f_lost_enemy_time + 5.0 > gpGlobals->time)
   {
      // if bot is waiting for enemy to strike back, don't move
      if (pBot->f_pause_time > gpGlobals->time)
         pBot->f_lost_enemy_time = gpGlobals->time; // set lost enemy time to now

      // else rush after that coward one
      else if ((pBot->v_lastseenenemy_position - pBot->pEdict->v.origin).Length () > 50)
         BotReachPosition (pBot, pBot->v_lastseenenemy_position); // chase

      else
      {
         pBot->f_lost_enemy_time = 0.0; // here we are, seems that bot really lost enemy

         // if bot is an engineer and has enough metal, try to drop a sentry gun here
         if ((pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
             && (pBot->m_rgAmmo[weapon_defs[TF_WEAPON_SPANNER].iAmmo1] > 130))
         {
            FakeClientCommand (pBot->pEdict, "build 2");
            FakeClientCommand (pBot->pEdict, "rotatesentry180");
         }

         // else if bot is a spy, it could be useful to disguise now
         else if (pBot->pEdict->v.playerclass == TFC_CLASS_SPY)
         {
            char command[32];
            sprintf (command, "disguise_enemy %d", RANDOM_LONG (1, 9)); // pickup random class
            FakeClientCommand (pBot->pEdict, command); // issue the command
         }
      }
   }

   // else look for special zones
   else if (BotCheckForSpecialZones (pBot))
   {
      pBot->pEdict->v.angles.y = UTIL_WrapAngle (pBot->pEdict->v.v_angle.y); // body angles = view angles

      // check if the bot is stuck, not paused and NOT on a ladder since handled elsewhere
      if (BotIsStuck (pBot) && (pBot->pEdict->v.movetype != MOVETYPE_FLY) && (pBot->f_pause_time < gpGlobals->time))
         BotUnstuck (pBot); // try to unstuck our poor bot

      if (pBot->pEdict->v.flFallVelocity > pBot->BotMove.f_max_speed)
         pBot->fall_time = gpGlobals->time; // save bot fall time

      pBot->v_prev_position = pBot->pEdict->v.origin; // save previous position (for checking if stuck)
      g_engfuncs.pfnRunPlayerMove (pBot->pEdict, UTIL_WrapAngles (pBot->pEdict->v.v_angle), pBot->BotMove.f_move_speed, pBot->BotMove.f_strafe_speed, 0, pBot->pEdict->v.button, 0, pBot->msecval);
      return;
   }

   // is bot keeping a place ?
   if (pBot->v_place_to_keep != Vector (0, 0, 0))
      BotStayInPosition (pBot);

   // else is bot being "used" ?
   else if (pBot->pBotUser != NULL)
      BotFollowUser (pBot);

   // else may the bot spray a logo (don't spray if bot has an enemy) ?
   else if ((pBot->f_spraying_logo_time > gpGlobals->time) && (pBot->pBotEnemy == NULL))
   {
      pBot->BotMove.f_forward_time = 0; // don't move
      BotPointGun (pBot, Vector (-45, pBot->pEdict->v.v_angle.y, pBot->pEdict->v.v_angle.z)); // look down at 45 degree angle

      if ((pBot->f_spraying_logo_time - 0.75 < gpGlobals->time) && (!pBot->b_logo_sprayed))
      {
         // spray logo when finished looking down
         pBot->b_logo_sprayed = TRUE; // remember this is done
         pBot->BotMove.f_backwards_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.0); // move back

         // the "impulse 201" command is actually passed in pfnRunPlayerMove for fake clients...
         g_engfuncs.pfnRunPlayerMove (pBot->pEdict, UTIL_WrapAngles (pBot->pEdict->v.v_angle), 0, 0, 0, 0, 201, pBot->msecval);
         return;
      }

      pBot->f_reach_time = gpGlobals->time + 0.5; // don't reach point for half a second
   }

   // else if no enemy & nothing special to do...
   else if (pBot->pBotEnemy == NULL)
   {
      if (pBot->f_find_item_time < gpGlobals->time)
         BotCheckForItems (pBot); // if time to, see if there are any visible items
      else
         pBot->b_is_picking_item = FALSE;

      BotWander (pBot); // then just wander around
   }

   if (pBot->f_pause_time > gpGlobals->time) // is the bot "paused"?
      pBot->BotMove.f_forward_time = 0; // don't move while pausing

   // make the body face the same way the bot is looking
   pBot->pEdict->v.angles.y = UTIL_WrapAngle (pBot->pEdict->v.v_angle.y);

   if (pBot->pEdict->v.flFallVelocity > pBot->BotMove.f_max_speed)
      pBot->fall_time = gpGlobals->time; // save bot fall time

   pBot->v_prev_position = pBot->pEdict->v.origin; // save previous position (for checking if stuck)
   g_engfuncs.pfnRunPlayerMove (pBot->pEdict, UTIL_WrapAngles (pBot->pEdict->v.v_angle), pBot->BotMove.f_move_speed, pBot->BotMove.f_strafe_speed, 0, pBot->pEdict->v.button, 0, pBot->msecval);
   return;
}


void BotAnswerToOrder (bot_t *pBot)
{
   if ((pBot->pEdict == NULL) || (pBot->pAskingEntity == NULL))
      return; // reliability check

   // has the bot been asked to follow someone ?
   if (pBot->bot_order == BOT_ORDER_FOLLOW)
   {
      // does the bot want to follow the caller ?
      if ((pBot->pBotEnemy == NULL)
          && ((RANDOM_LONG (1, 100) < 80) && pBot->b_is_fearful)
              || ((RANDOM_LONG (1, 100) < 40) && !pBot->b_is_fearful))
      {
         pBot->v_place_to_keep = Vector (0, 0, 0); // don't stay in position anymore
         pBot->pBotUser = pBot->pAskingEntity; // mark this client as using the bot
         pBot->v_lastseenuser_position = pBot->pAskingEntity->v.origin; // remember last seen user position
         pBot->BotChat.b_saytext_follow = TRUE; // bot acknowledges
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
      }
      else
      {
         pBot->BotChat.b_saytext_negative = TRUE; // bot refuses
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
      }
   }

   // else has the bot been asked to check in ?
   else if (pBot->bot_order == BOT_ORDER_REPORT)
   {
      // does the bot want to answer the caller ?
      if (RANDOM_LONG (1, 100) < 66)
      {
         // does the bot have no enemy ?
         if (pBot->pBotEnemy == NULL)
         {
            pBot->BotChat.b_sayaudio_reporting = TRUE; // set him for reporting
            pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (1.0, 2.0);
         }
         else
         {
            pBot->BotChat.b_sayaudio_attacking = TRUE; // bot yells attack (audio)
            pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (1.0, 2.0);
         }
      }
   }

   // else has the bot been asked to keep a position ?
   else if (pBot->bot_order == BOT_ORDER_STAY)
   {
      // does the bot wants to obey the caller ?
      if ((pBot->pBotEnemy == NULL)
          && ((RANDOM_LONG (1, 100) < 80) && pBot->b_is_fearful)
              || ((RANDOM_LONG (1, 100) < 40) && !pBot->b_is_fearful))
      {
         pBot->v_place_to_keep = pBot->pEdict->v.origin; // position to stay in
         pBot->f_place_time = gpGlobals->time; // remember when we last saw the place to keep
         pBot->pBotUser = NULL; // free the user client slot
         pBot->v_lastseenuser_position = Vector (0, 0, 0); // forget last seen user position
         pBot->BotChat.b_saytext_stay = TRUE; // bot acknowledges
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
      }
      else
      {
         pBot->BotChat.b_saytext_negative = TRUE; // bot refuses
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
      }
   }

   // else has the bot been asked to rush on his own ?
   else if (pBot->bot_order == BOT_ORDER_GO)
   {
      pBot->v_place_to_keep = Vector (0, 0, 0); // don't stay in position anymore
      pBot->pBotUser = NULL; // free the user client slot
      pBot->v_lastseenuser_position = Vector (0, 0, 0); // forget last seen user position
      if (!pBot->b_is_fearful)
         pBot->f_rush_time = gpGlobals->time + RANDOM_FLOAT (15.0, 45.0); // rush if not fearful

      // does the bot want to answer the caller ?
      if (RANDOM_LONG (1, 100) < 66)
      {
         pBot->BotChat.b_saytext_affirmative = TRUE; // bot acknowledges
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 3.5);
      }
   }

   // else has the bot been asked to set the plastic's timer to five seconds ?
   else if (pBot->bot_order == BOT_ORDER_DETONATEPLASTIC_5SECONDS)
   {
      // only demomans may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_DEMOMAN)
      {
         pBot->BotChat.b_saytext_affirmative = TRUE; // bot acknowledges
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
         pBot->f_pause_time = gpGlobals->time + 5.0; // pause while planting plastic
         FakeClientCommand (pBot->pEdict, "detstart 5");
      }
      else
      {
         pBot->BotChat.b_saytext_negative = TRUE; // bot disagrees
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
      }
   }

   // else has the bot been asked to set the plastic's timer to five seconds ?
   else if (pBot->bot_order == BOT_ORDER_DETONATEPLASTIC_20SECONDS)
   {
      // only demomans may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_DEMOMAN)
      {
         pBot->BotChat.b_saytext_affirmative = TRUE; // bot acknowledges
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
         pBot->f_pause_time = gpGlobals->time + 5.0; // pause while planting plastic
         FakeClientCommand (pBot->pEdict, "detstart 20");
      }
      else
      {
         pBot->BotChat.b_saytext_negative = TRUE; // bot disagrees
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
      }
   }

   // else has the bot been asked to disarm the plastic ?
   else if (pBot->bot_order == BOT_ORDER_DISARMPLASTIC)
   {
      // only demomans may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_DEMOMAN)
      {
         pBot->BotChat.b_saytext_affirmative = TRUE; // bot acknowledges
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
         pBot->f_pause_time = gpGlobals->time + 5.0; // pause while defusing plastic
         FakeClientCommand (pBot->pEdict, "detstop");
      }
      else
      {
         pBot->BotChat.b_saytext_negative = TRUE; // bot disagrees
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
      }
   }

   // else has the bot been asked to disguise as an enemy ?
   else if (pBot->bot_order == BOT_ORDER_DISGUISEENEMY)
   {
      // only spies may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_SPY)
      {
         char command[32];
         pBot->BotChat.b_saytext_affirmative = TRUE; // bot acknowledges
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
         sprintf (command, "disguise_enemy %d", RANDOM_LONG (1, 9)); // pickup random class
         FakeClientCommand (pBot->pEdict, command); // issue the command
      }
      else
      {
         pBot->BotChat.b_saytext_negative = TRUE; // bot disagrees
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
      }
   }

   // else has the bot been asked to disguise as a friend ?
   else if (pBot->bot_order == BOT_ORDER_DISGUISEFRIENDLY)
   {
      // only spies may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_SPY)
      {
         char command[32];
         int bot_class = TFC_CLASS_SPY;
         pBot->BotChat.b_saytext_affirmative = TRUE; // bot acknowledges
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
         while (bot_class == TFC_CLASS_SPY)
            bot_class = RANDOM_LONG (1, 9); // pick up a random player class
         sprintf (command, "disguise_friendly %d", bot_class); // build the command
         FakeClientCommand (pBot->pEdict, command); // issue the command
      }
      else
      {
         pBot->BotChat.b_saytext_negative = TRUE; // bot disagrees
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
      }
   }

   // else has the bot been asked to feign death ?
   else if (pBot->bot_order == BOT_ORDER_FEIGN)
   {
      // only spies may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_SPY)
      {
         pBot->BotChat.b_saytext_affirmative = TRUE; // bot acknowledges
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
         FakeClientCommand (pBot->pEdict, "sfeign"); 
      }
      else
      {
         pBot->BotChat.b_saytext_negative = TRUE; // bot disagrees
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
      }
   }

   // else has the bot been asked to stop feigning death ?
   else if (pBot->bot_order == BOT_ORDER_STOPFEIGN)
   {
      // only spies may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_SPY)
      {
         pBot->BotChat.b_saytext_affirmative = TRUE; // bot acknowledges
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
         FakeClientCommand (pBot->pEdict, "feign"); 
      }
      else
      {
         pBot->BotChat.b_saytext_negative = TRUE; // bot disagrees
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
      }
   }

   // else has the bot been asked to build a sentry gun ?
   else if (pBot->bot_order == BOT_ORDER_BUILDSENTRY)
   {
      // only engineers that have enough metal (> 130) may acknowledge
      if ((pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
          && (pBot->m_rgAmmo[weapon_defs[TF_WEAPON_SPANNER].iAmmo1] > 130))
      {
         pBot->BotChat.b_saytext_affirmative = TRUE; // bot acknowledges
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
         FakeClientCommand (pBot->pEdict, "build 2"); 
      }
      else
      {
         pBot->BotChat.b_saytext_negative = TRUE; // bot disagrees
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
      }
   }

   // else has the bot been asked to rotate his sentry gun by 180 degrees ?
   else if (pBot->bot_order == BOT_ORDER_ROTATESENTRY_180DEGREES)
   {
      // only engineers may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
      {
         pBot->BotChat.b_saytext_affirmative = TRUE; // bot acknowledges
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
         FakeClientCommand (pBot->pEdict, "rotatesentry180"); 
      }
      else
      {
         pBot->BotChat.b_saytext_negative = TRUE; // bot disagrees
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
      }
   }

   // else has the bot been asked to rotate his sentry gun by 45 degrees ?
   else if (pBot->bot_order == BOT_ORDER_ROTATESENTRY_45DEGREES)
   {
      // only engineers may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
      {
         pBot->BotChat.b_saytext_affirmative = TRUE; // bot acknowledges
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
         FakeClientCommand (pBot->pEdict, "rotatesentry"); 
      }
      else
      {
         pBot->BotChat.b_saytext_negative = TRUE; // bot disagrees
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
      }
   }

   // else has the bot been asked to destroy his sentry gun ?
   else if (pBot->bot_order == BOT_ORDER_DETONATESENTRY)
   {
      // only engineers may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
      {
         pBot->BotChat.b_saytext_affirmative = TRUE; // bot acknowledges
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
         FakeClientCommand (pBot->pEdict, "detsentry"); 
      }
      else
      {
         pBot->BotChat.b_saytext_negative = TRUE; // bot disagrees
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
      }
   }

   // else has the bot been asked to build a dispenser ?
   else if (pBot->bot_order == BOT_ORDER_BUILDDISPENSER)
   {
      // only engineers that have enough metal (> 100) may acknowledge
      if ((pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
          && (pBot->m_rgAmmo[weapon_defs[TF_WEAPON_SPANNER].iAmmo1] > 100))
      {
         pBot->BotChat.b_saytext_affirmative = TRUE; // bot acknowledges
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
         FakeClientCommand (pBot->pEdict, "build 1"); 
      }
      else
      {
         pBot->BotChat.b_saytext_negative = TRUE; // bot disagrees
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
      }
   }

   // else has the bot been asked to destroy his dispenser ?
   else if (pBot->bot_order == BOT_ORDER_BUILDDISPENSER)
   {
      // only engineers may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
      {
         pBot->BotChat.b_saytext_affirmative = TRUE; // bot acknowledges
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
         FakeClientCommand (pBot->pEdict, "detdispenser"); 
      }
      else
      {
         pBot->BotChat.b_saytext_negative = TRUE; // bot disagrees
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
      }
   }

   pBot->bot_order = BOT_ORDER_NOORDER; // reset bot order field
   return;
}


void UpdateBulletSounds (edict_t *pEdict)
{
   if (pEdict == NULL)
      return; // reliability check

   // check if this player is actually attacking something
   if (((pEdict->v.button & IN_ATTACK) == 0)
       || ((pEdict->v.button & IN_ATTACK2) == 0))
      return; // give up, this player is not firing

   // message indicates that the player is decreasing his ammo, was this ammo a bullet ?
   if ((strcmp (STRING (pEdict->v.weaponmodel), "models/p_9mmhandgun") == 0)
       || (strncmp (STRING (pEdict->v.weaponmodel), "models/p_crowbar", 16) == 0)
       || (strncmp (STRING (pEdict->v.weaponmodel), "models/p_egon", 13) == 0)
       || (strncmp (STRING (pEdict->v.weaponmodel), "models/p_glauncher", 18) == 0)
       || (strncmp (STRING (pEdict->v.weaponmodel), "models/p_grenade", 16) == 0)
       || (strncmp (STRING (pEdict->v.weaponmodel), "models/p_knife", 14) == 0)
       || (strncmp (STRING (pEdict->v.weaponmodel), "models/p_medkit", 15) == 0)
       || (strncmp (STRING (pEdict->v.weaponmodel), "models/p_mini", 13) == 0)
       || (strncmp (STRING (pEdict->v.weaponmodel), "models/p_rpg", 12) == 0)
       || (strncmp (STRING (pEdict->v.weaponmodel), "models/p_spanner", 16) == 0)
       || (strcmp (STRING (pEdict->v.weaponmodel), "models/p_srpg") == 0)
       || (strncmp (STRING (pEdict->v.weaponmodel), "models/p_umbrella", 17) == 0))
      return; // give up, player's weapon does not fire bullets

   // determine the impact point
   TraceResult tr;

   UTIL_MakeVectors (pEdict->v.v_angle); // build base vectors
   UTIL_TraceLine (GetGunPosition (pEdict), gpGlobals->v_forward * 9999, dont_ignore_monsters, ignore_glass, pEdict, &tr);

   // cycle through all bots and alert those who saw the bullet ricochet
   for (int index = 0; index < 32; index++)
   {
      // does this bot have no enemy AND ricochet was close to bot AND bot can see ricochet ?
      if ((bots[index].pEdict != pEdict) && (bots[index].pBotEnemy == NULL) && ((tr.vecEndPos - bots[index].pEdict->v.origin).Length () < 300)
          && BotCanSeeThis (&bots[index], tr.vecEndPos) && FInViewCone (tr.vecEndPos, bots[index].pEdict))
      {
         BotSetIdealYaw (&bots[index], UTIL_VecToAngles (bots[index].pEdict->v.origin - tr.vecEndPos).y); // face where it came from
         bots[index].f_reach_time = gpGlobals->time + 0.5; // don't reach point for half a second
      }
   }

   return;
}
