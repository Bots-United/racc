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
// TFC version
//
// bot.cpp
//

#include "racc.h"


// this is the LINK_ENTITY_TO_CLASS function that creates a player (bot)
extern "C" EXPORT void player (entvars_t *pev);


void BotCreate (void)
{
   // this is where the show begins, i.e. the function that creates a bot. How it works :
   // I check which profiles are not currently in use by other bots. Third step, is to ask
   // the engine to create the fakeclient and give it a player entity pointer. And once
   // ClientPutInServer() has been called, ladies and gentlemen, please welcome our new bot.

   bot_t *pBot;
   edict_t *pBotEdict;
   profile_t *pBotProfile;
   char ip_address[32];
   char reject_reason[128];
   bool profiles_used[RACC_MAX_PROFILES];
   long index, bot_index, profile_index, used_count;

   // reset used profiles flags array
   memset (profiles_used, 0, sizeof (profiles_used));

   // cycle through all bot slots
   for (bot_index = 0; bot_index < *server.max_clients; bot_index++)
   {
      // is this bot active ?
      if (bots[bot_index].is_active && IsValidPlayer (bots[bot_index].pEdict))
      {
         // cycle through all the bot profiles we know
         for (profile_index = 0; profile_index < profile_count; profile_index++)
         {
            // does the bot have the same profile as this one ?
            if (&profiles[profile_index] == bots[bot_index].pProfile)
            {
               profiles_used[profile_index] = TRUE; // this profile is used, so flag it
               used_count++; // increment the used profiles counter
            }
         }
      }
   }

   // if all the profiles are used, that's there aren't enough living bots to join
   if (used_count == profile_count)
   {
      ServerConsole_printf ("RACC: not enough people in cybernetic population!\n"); // tell why
      server.max_bots = bot_count; // max out the bots to the current number
      return; // ...and cancel bot creation
   }

   // pick up a profile that isn't used
   do
      profile_index = RANDOM_LONG (0, profile_count - 1); // pick up one randomly until not used
   while (profiles_used[profile_index]);

   // okay, now we have a valid profile for our new bot
   pBotProfile = &profiles[profile_index];

   pBotEdict = (*g_engfuncs.pfnCreateFakeClient) (pBotProfile->name); // create the fake client
   if (FNullEnt (pBotEdict))
      return; // cancel if unable to create fake client

   // link his entity to an useful pointer
   pBot = &bots[ENTINDEX (pBotEdict) - 1];
   pBot->pEdict = pBotEdict;
   pBot->pProfile = pBotProfile;

   if (pBot->pEdict->pvPrivateData != NULL)
      FREE_PRIVATE (pBot->pEdict); // free our predecessor's private data
   pBot->pEdict->pvPrivateData = NULL; // fools the private data pointer 
   pBot->pEdict->v.frags = 0; // reset his frag count 

   // initialize his weapons database pointers
   memset (&pBot->bot_weapons, 0, sizeof (pBot->bot_weapons));
   for (index = 0; index < MAX_WEAPONS; index++)
   {
      pBot->bot_weapons[index].hardware = &weapons[index];
      pBot->bot_weapons[index].primary_ammo = &pBot->bot_ammos[weapons[index].primary.type_of_ammo];
      pBot->bot_weapons[index].secondary_ammo = &pBot->bot_ammos[weapons[index].secondary.type_of_ammo];
   }
   pBot->current_weapon = &pBot->bot_weapons[0]; // set current weapon pointer to failsafe value

   // create the player entity by calling MOD's player() function
   player (&pBot->pEdict->v);

   // set the standard skin in the infobuffer (skins will be attributed by class later)
   (*g_engfuncs.pfnSetClientKeyValue) (ENTINDEX (pBot->pEdict), (*g_engfuncs.pfnGetInfoKeyBuffer) (pBot->pEdict), "model", "gordon");

   // let him connect to the server under its own name
   sprintf (ip_address, "127.0.0.%d", ENTINDEX (pBot->pEdict) + 100); // build it an unique address
   ClientConnect (pBot->pEdict, pBot->pProfile->name, ip_address, reject_reason);

   // print a notification message on the dedicated server console if in developer mode
   if (server.is_dedicated && (server.developer_level > 0))
   {
      if (server.developer_level > 1)
      {
         ServerConsole_printf ("Server requiring authentication\n");
         ServerConsole_printf ("Client %s connected\n", STRING (pBot->pEdict->v.netname));
         ServerConsole_printf ("Adr: %s:27005\n", ip_address);
      }
      ServerConsole_printf ("Verifying and uploading resources...\n");
      ServerConsole_printf ("Custom resources total 0 bytes\n");
      ServerConsole_printf ("  Decals:  0 bytes\n");
      ServerConsole_printf ("----------------------\n");
      ServerConsole_printf ("Resources to request: 0 bytes\n");
   }

   // let him actually join the game
   pBot->pEdict->v.flags |= FL_THIRDPARTYBOT; // let ClientPutInServer() know it's a bot connecting
   ClientPutInServer (pBot->pEdict);

   // create his illumination entity (thanks to Tom Simpson from FoxBot for the engine bug fix)
   pBot->pIllumination = pfnCreateNamedEntity (MAKE_STRING ("info_target"));
   Spawn (pBot->pIllumination); // spawn it
   pBot->pIllumination->v.movetype = MOVETYPE_NOCLIP; // set its movement to no clipping
   pBot->pIllumination->v.nextthink = *server.time; // needed to make it think
   pBot->pIllumination->v.classname = MAKE_STRING ("entity_botlightvalue"); // sets its name
   SET_MODEL (pBot->pIllumination, "models/mechgibs.mdl"); // sets it a model

   // initialize all the variables for this bot...

   BotReset (pBot); // reset our bot

   pBot->is_active = TRUE; // set his 'is active' flag

   // if internet mode is on...
   if (server.is_internetmode)
   {
      pBot->time_to_live = *server.time + RANDOM_LONG (300, 3600); // set him a TTL
      pBot->quit_game_time = pBot->time_to_live + RANDOM_FLOAT (3.0, 7.0); // disconnect time
   }
   else
   {
      pBot->time_to_live = -1; // don't set him a TTL (time to live)
      pBot->quit_game_time = -1; // so never quit
   }

   // say hello here
   if (RANDOM_LONG (1, 100) <= (86 - 2 * player_count))
      pBot->BotChat.bot_saytext = BOT_SAYTEXT_HELLO;

   pBot->f_bot_alone_timer = *server.time + RANDOM_LONG (30, 120); // set an idle delay
   pBot->b_not_started = TRUE; // tells bot to go and select team and class
}


void BotReset (bot_t *pBot)
{
   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   v_pathdebug_from = pBot->pEdict->v.origin;

   // reset bot's input channels
   memset (&pBot->BotEyes, 0, sizeof (pBot->BotEyes));
   memset (&pBot->BotEars, 0, sizeof (pBot->BotEars));
   memset (&pBot->BotBody, 0, sizeof (pBot->BotBody));

   BotSetIdealAngles (pBot, Vector (RANDOM_FLOAT (-15, 15), RANDOM_FLOAT (-180, 180), 0));
   pBot->pEdict->v.angles.x = -pBot->pEdict->v.v_angle.x / 3;
   pBot->pEdict->v.angles.y = pBot->pEdict->v.v_angle.y;
   pBot->pEdict->v.angles.z = 0;
   pBot->BotMove.f_max_speed = CVAR_GET_FLOAT ("sv_maxspeed");

   pBot->f_find_item_time = 0.0;

   pBot->pBotLadder = NULL;
   pBot->ladder_direction = LADDER_UNKNOWN;
   pBot->f_start_use_ladder_time = 0.0;
   pBot->f_end_use_ladder_time = 0.0;

   if (RANDOM_LONG (1, 100) < 33)
      pBot->b_is_fearful = TRUE;
   else
      pBot->b_is_fearful = FALSE;
   pBot->BotMove.b_emergency_walkback = FALSE;
   pBot->BotMove.f_walk_time = 0.0;
   pBot->BotMove.f_forward_time = 0.0;
   pBot->BotMove.f_backwards_time = 0.0;
   pBot->BotMove.f_jump_time = 0.0;
   pBot->BotMove.f_duck_time = 0.0;
   pBot->BotMove.f_strafeleft_time = 0.0;
   pBot->BotMove.f_straferight_time = 0.0;

   memset (&pBot->BotEnemy, 0, sizeof (pBot->BotEnemy));
   memset (&pBot->LastSeenEnemy, 0, sizeof (pBot->LastSeenEnemy));
   pBot->f_reload_time = -1;
   pBot->f_throwgrenade_time = 0;
   pBot->pBotUser = NULL;
   pBot->f_bot_use_time = 0.0;
   pBot->f_randomturn_time = *server.time;
   pBot->BotChat.bot_saytext = 0;
   pBot->b_help_asked = FALSE;
   pBot->BotChat.f_saytext_time = 0.0;
   pBot->BotChat.f_sayaudio_time = 0.0;
   pBot->BotChat.bot_sayaudio = BOT_SAYAUDIO_NOTHING;

   pBot->f_shoot_time = *server.time;

   pBot->f_rush_time = *server.time + RANDOM_FLOAT (15.0, 45.0);
   pBot->f_pause_time = 0.0;
   pBot->f_sound_update_time = 0.0;

   pBot->f_find_goal_time = 0;
   pBot->v_goal = g_vecZero;
   pBot->v_place_to_keep = g_vecZero;
   pBot->f_place_time = 0;
   pBot->f_camp_time = 0;
   pBot->f_reach_time = 0;
   pBot->v_reach_point = g_vecZero;
   pBot->f_turncorner_time = *server.time + 5.0;

   pBot->bot_task = BOT_TASK_IDLE;
   pBot->b_interact = FALSE;
   pBot->f_interact_time = 0;
   pBot->b_lift_moving = FALSE;
   pBot->f_spraying_logo_time = 0;
   pBot->b_logo_sprayed = FALSE;

   memset (&pBot->bot_ammos, 0, sizeof (pBot->bot_ammos));
}


bool BotCheckForSpecialZones (bot_t *pBot)
{
   edict_t *pSpecialZone = NULL;

   if (!IsValidPlayer (pBot->pEdict))
      return (FALSE); // reliability check

   if (DebugLevel.is_dontfindmode)
      return (FALSE); // don't process if botdontfind is set

   // is there a special zone near here?
   while ((pSpecialZone = UTIL_FindEntityInSphere (pSpecialZone, pBot->pEdict->v.origin, 1000)) != NULL)
   {
      // check for a visible dropped flag
      if ((strcmp ("item_tfgoal", STRING (pSpecialZone->v.classname)) == 0)
          && (BotCanSeeThis (pBot, pSpecialZone->v.origin))
          && (IsInPlayerFOV (pBot->pEdict, pSpecialZone->v.origin))
          && ENT_IS_ON_FLOOR (pSpecialZone))
      {
         // is it our flag ?
         if (GetTeam (pBot->pEdict) == GetTeam (pSpecialZone))
         {
            BotReachPosition (pBot, pSpecialZone->v.origin); // go pick it up
            return (TRUE); // bot is concerned by this special zone
         }
      }

      // check for a visible flag target
      else if ((strcmp ("info_tfgoal", STRING (pSpecialZone->v.classname)) == 0)
               && (BotCanSeeOfEntity (pBot, pSpecialZone) != g_vecZero))
      {
         // both teams will head to the flag site. Bots will so "cruise" around
         // the flag, permanently looking for enemies.

         // let's run to that item if previous goal was not visible for more than 30s
         if (pBot->f_reach_time + 30 < *server.time)
         {
            BotReachPosition (pBot, VecBModelOrigin (pSpecialZone));
            return (TRUE); // bot is concerned by this special zone
         }
      }
   }

   return (FALSE); // bot found nothing interesting
}


bool BotCheckForGrenades (bot_t *pBot)
{
   edict_t *pGrenade = NULL;

   if (!IsValidPlayer (pBot->pEdict))
      return (FALSE); // reliability check

   if (DebugLevel.is_dontfindmode)
      return (FALSE); // don't process if botdontfind is set

   // is there an armed grenade near here?
   while ((pGrenade = UTIL_FindEntityInSphere (pGrenade, pBot->pEdict->v.origin, 300)) != NULL)
   {
      // check if entity is an armed grenade
      if ((strstr (STRING (pGrenade->v.classname), "grenade") != NULL)
          && (BotCanSeeThis (pBot, pGrenade->v.origin))
          && (IsInPlayerFOV (pBot->pEdict, pGrenade->v.origin)))
      {
         float grenade_angle = UTIL_VecToAngles (pGrenade->v.origin - pBot->pEdict->v.origin).y;
         BotSetIdealYaw (pBot, grenade_angle); // face the grenade...

         // ... and run away !!
         pBot->BotMove.f_backwards_time = *server.time + 0.5; // until the grenade explodes

         // reliability check: the v.owner entvars_t slot of pGrenade may be unregistered...
         if (pGrenade->v.owner != NULL)
         {
            // check if this grenade is our enemies'...
            int grenade_team = GetTeam (pGrenade->v.owner);
            int bot_team = GetTeam (pBot->pEdict);

            // if so, yell
            if ((bot_team != grenade_team) && !(team_allies[bot_team] & (1 << grenade_team)))
            {
               pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_SEEGRENADE; // bot says 'danger'
               pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (0.7, 1.5);
            }
         }

         return (TRUE); // bot is concerned by this grenade
      }
   }

   return (FALSE); // bot found nothing interesting
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

   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   if (DebugLevel.is_dontfindmode)
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

         // check if entity is outside field of view
         if (!IsInPlayerFOV (pBot->pEdict, entity_origin))
            continue; // skip this item if bot can't "see" it

         // check if entity is a ladder (ladders are a special case)
         if (strcmp ("func_ladder", STRING (pent->v.classname)) == 0)
         {
            // force ladder origin to same z coordinate as bot since
            // the VecBModelOrigin is the center of the ladder.  For
            // LONG ladders, the center MAY be hundreds of units above
            // the bot.  Fake an origin at the same level as the bot...

            entity_origin.z = pBot->pEdict->v.origin.z;

            // trace a line from bot's eyes to func_ladder entity...
            UTIL_TraceLine (GetGunPosition (pBot->pEdict), entity_origin, ignore_monsters, pBot->pEdict, &tr);

            // check if traced all the way up to the entity (didn't hit wall)
            if (tr.flFraction >= 1.0)
            {
               // always use the ladder if haven't used a ladder in at least 5 seconds...
               if (pBot->f_end_use_ladder_time + 5.0 < *server.time)
                  can_pickup = TRUE;
            }
         }
         else
         {
            // trace a line from bot's eyes to entity
            UTIL_TraceLine (GetGunPosition (pBot->pEdict), entity_origin, ignore_monsters, pBot->pEdict, &tr);
            
            // check if traced NEARLY all the way up to the entity
            if ((tr.flFraction > 0.8) && (tr.flFraction < 1.0))
            {
               // check if entity is stuff that can be blown up by a some plastic...
               if ((strcmp ("func_wall_toggle", STRING (pent->v.classname)) == 0)
                        && (pBot->pEdict->v.playerclass == TFC_CLASS_DEMOMAN))
               {
                  // if close enough, drop plastic
                  if ((entity_origin - pBot->pEdict->v.origin).Length () < 80)
                     FakeClientCommand (pBot->pEdict, "detstart 5");
                  
                  can_pickup = TRUE;
               }
            }

            // else if traced all the way up to the entity (didn't hit wall)
            else if (strcmp (STRING (pent->v.classname), STRING (tr.pHit->v.classname)) == 0)
            {
               // find distance to item for later use...
               float distance = (entity_origin - pBot->pEdict->v.origin).Length ();

               // check if entity is a breakable...
               if ((strcmp ("func_breakable", STRING (pent->v.classname)) == 0)
                   && (pent->v.takedamage != DAMAGE_NO) && (pent->v.health > 0)
                   && !(pent->v.flags & FL_WORLDBRUSH)
                   && (fabs (entity_origin.z - pBot->pEdict->v.origin.z) < 60))
               {
                  // check if close enough...
                  if (distance < 50)
                  {
                     if ((pBot->current_weapon->hardware->id != TF_WEAPON_SPANNER)
                         || (pBot->current_weapon->hardware->id != TF_WEAPON_KNIFE)
                         || (pBot->current_weapon->hardware->id != TF_WEAPON_AXE))
                     {
                        // select a proximity weapon
                        if (pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
                           FakeClientCommand (pBot->pEdict, "tf_weapon_spanner");
                        else if (pBot->pEdict->v.playerclass == TFC_CLASS_SPY)
                           FakeClientCommand (pBot->pEdict, "tf_weapon_knife");
                        else
                           FakeClientCommand (pBot->pEdict, "tf_weapon_axe");
                     }
                     else
                     {
                        // point the weapon at the breakable and strike it
                        BotSetIdealAngles (pBot, UTIL_VecToAngles (entity_origin - GetGunPosition (pBot->pEdict)));
                        pBot->pEdict->v.button |= IN_ATTACK; // strike the breakable
                        pBot->f_reload_time = *server.time + RANDOM_LONG (1.5, 3.0); // set next time to reload
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

         // check if entity is outside field of view
         if (!IsInPlayerFOV (pBot->pEdict, entity_origin))
            continue;  // skip this item if bot can't "see" it

         // check if line of sight to object is not blocked (i.e. visible)
         if (BotCanSeeThis (pBot, entity_origin))
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
               if (!(pBot->BotBody.hit_state & OBSTACLE_LEFT_FALL))
                  pBot->BotMove.f_strafeleft_time = *server.time + RANDOM_FLOAT (0.5, 2.0);
               else if (!(pBot->BotBody.hit_state & OBSTACLE_RIGHT_FALL))
                  pBot->BotMove.f_straferight_time = *server.time + RANDOM_FLOAT (0.5, 2.0);
               else
                  pBot->BotMove.f_backwards_time = *server.time + RANDOM_FLOAT (0.5, 2.0);
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

   if (!FNullEnt (pPickupEntity))
   {
      pBot->b_is_picking_item = TRUE; // set bot picking item flag
      pBot->v_reach_point = pickup_origin; // save the location of item bot is trying to get
   }
   else
      pBot->b_is_picking_item = FALSE; // reset picking item flag
}


void BotPreThink (bot_t *pBot)
{
   // this is the first step of the bot Think() trilogy. In nature, every behaviour that can be
   // associated to intelligence is resulting of three invariable steps :
   // 1 - sensing the environment and the character's state
   // 2 - working out the changes to commit onto the environment and the character's state
   // 3 - performing these changes, and experiencing this action, looping back to step 1.
   // Here we deal with the first step, sensing, which is a correlation of three "input vectors"
   // (actually there are more of them but these three together are sufficient to be said
   // symptomatic of the human behaviour), respectively in order of importance the sight, the
   // hearing, and the touch feeling. Since FPS players experience this last one mainly by proxy,
   // the game simulating it by visual and auditive means, the touch feeling of the bot will be
   // a little more efficient than those of the players, giving them a slight advantage in this
   // particular domain. But since we're very far from emulating the two other vectors (vision
   // and hearing) as accurately as their human equivalents, it's just fair that way :)

   pBot->pEdict->v.flags |= FL_THIRDPARTYBOT; // set the third party bot flag

   // compute the bot's eye position and its aim vector
   pBot->BotAim.v_eyeposition = pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs;
   pfnAngleVectors (pBot->pEdict->v.v_angle, (float *) &pBot->BotAim.v_forward, (float *) &pBot->BotAim.v_right, (float *) &pBot->BotAim.v_up);

   BotSee (pBot); // make bot see
   BotHear (pBot); // make bot hear
   BotTouch (pBot); // make bot touch

   pBot->pEdict->v.button = 0; // reset buttons pressed
   pBot->pEdict->v.impulse = 0; // reset impulse buttons pressed
   pBot->BotMove.f_move_speed = 0; // reset move_speed
   pBot->BotMove.f_strafe_speed = 0; // reset strafe_speed
   pBot->BotMove.b_emergency_walkback = FALSE;

   return;
}


void BotThink (bot_t *pBot)
{
   // this is the second step of the bot Think() trilogy. In nature, every behaviour that can be
   // associated to intelligence is resulting of three invariable steps :
   // 1 - sensing the environment and the character's state
   // 2 - working out the changes to commit onto the environment and the character's state
   // 3 - performing these changes, and experiencing this action, looping back to step 1.
   // Here we deal with the second step, thinking, where we involve all the cognitive stuff and
   // case-based reasoning that conditions the AI's behaviour.
   // Note: so far, this function is just a large bunch of if/else statments.

   // if the bot hasn't selected stuff to start the game yet, go do that...
   if (pBot->b_not_started)
   {
      BotStartGame (pBot);
      return;
   }

   // is the bot controlled by the player ?
   if (pBot->is_controlled)
   {
      BotIsBewitched (pBot); // OMG, this is witchery !
      return; // let the player steer this bot
   }

   // if the bot is stuck, try to unstuck it
   if (pBot->b_is_stuck && !IsFlying (pBot->pEdict))
      BotUnstuck (pBot); // try to unstuck our poor bot...

   // think (or continue thinking) of a path if necessary
   BotRunPathMachine (pBot);

   // is it time for the bot to leave the game ? (depending on his time to live)
   if ((pBot->time_to_live > 0) && (pBot->time_to_live <= *server.time))
   {
      pBot->time_to_live = *server.time + 6.0; // don't say it twice (bad hack)
      pBot->BotMove.f_forward_time = 0.0; // stop the bot while he is leaving
      BotSetIdealAngles (pBot, pBot->pEdict->v.v_angle); // don't make it move its crosshair
      if (RANDOM_LONG (1, 100) <= (66 - 2 * player_count))
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_BYE; // say goodbye
      return;
   }

   // if the bot is dead, press fire to respawn...
   if (!IsAlive (pBot->pEdict))
   {
      BotReset (pBot); // reset our bot

      // was the bot killed by another player ?
      if (IsValidPlayer (pBot->pKillerEntity) && (RANDOM_LONG (1, 100) <= (56 - 2 * player_count)))
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_WHINE;

      if (RANDOM_LONG (1, 100) <= 50)
         pBot->pEdict->v.button = IN_ATTACK;

      return;
   }

   // should the bot complain of being alone for a long time ?
   if ((pBot->f_bot_alone_timer > 0) && (pBot->f_bot_alone_timer <= *server.time))
   {
      pBot->f_bot_alone_timer = *server.time + RANDOM_LONG (30, 120); // sets new delay

      if (RANDOM_LONG (1, 100) <= (66 - 2 * player_count))
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_ALONE; // complain

      // bot must be tired of hunting, so let him camp for a while around here...
      if (!((pBot->current_weapon->hardware->id == TF_WEAPON_MEDIKIT)
            || (pBot->current_weapon->hardware->id == TF_WEAPON_SPANNER)
            || (pBot->current_weapon->hardware->id == TF_WEAPON_AXE)
            || (pBot->current_weapon->hardware->id == TF_WEAPON_GL)
            || (pBot->current_weapon->hardware->id == TF_WEAPON_FLAMETHROWER)
            || (pBot->current_weapon->hardware->id == TF_WEAPON_IC)
            || (pBot->current_weapon->hardware->id == TF_WEAPON_TRANQ)
            || (pBot->current_weapon->hardware->id == TF_WEAPON_PL)
            || (pBot->current_weapon->hardware->id == TF_WEAPON_KNIFE)
            || (pBot->current_weapon->hardware->id == TF_WEAPON_GRENADE)))
         BotCanCampNearHere (pBot, pBot->pEdict->v.origin);

      // yeah, i'm lazy.

      // if bot is an engineer and has enough metal, drop a sentry gun here to waste some time
      if ((pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
          && (*pBot->bot_weapons[TF_WEAPON_SPANNER].primary_ammo > 130))
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
   if (!pBot->b_help_asked && !FNullEnt (pBot->BotEnemy.pEdict) && (pBot->pEdict->v.health <= 20))
      if (RANDOM_LONG (1, 100) <= (91 - 2 * player_count))
      {
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEEDBACKUP; // yell
         pBot->b_help_asked = TRUE; // don't do it twice
      }

   // is it time to call for medic ?
   if (pBot->b_help_asked && (pBot->pEdict->v.health <= 20)
       && (pBot->f_reload_time > 0) && (pBot->f_reload_time < *server.time))
   {
      pBot->f_reload_time = 0;
      pBot->BotMove.f_walk_time = *server.time + 10.0;
	   FakeClientCommand (pBot->pEdict, "saveme"); // call for medic
   }

   // has the bot been ordered something ?
   if ((pBot->BotEars.bot_order != BOT_ORDER_NOORDER) && (pBot->BotEars.f_order_time + 1.0 < *server.time))
      BotAnswerToOrder (pBot); // answer to this order

   // is the bot blinded ?
   if (pBot->BotEyes.blinded_time > *server.time)
   {
      pBot->BotMove.f_duck_time = *server.time + RANDOM_FLOAT (0.5, 1.0); // duck when blinded

      // pick up a random strafe direction
      if (RANDOM_LONG (1, 100) < 50)
         pBot->BotMove.f_strafeleft_time = *server.time + 0.1;
      else
         pBot->BotMove.f_straferight_time = *server.time + 0.1;

      if (RANDOM_LONG (0, 100) < 50)
         pBot->BotMove.b_emergency_walkback = TRUE;

      return;
   }

   // see how much we have turned
   if (fabs (pBot->BotAim.v_turn_speed.y) < 2.0)
      pBot->b_is_walking_straight = TRUE;
   else
      pBot->b_is_walking_straight = FALSE;

   if ((fabs (pBot->BotAim.v_turn_speed.x) > 10) || (fabs (pBot->BotAim.v_turn_speed.y) > 10))
      pBot->BotMove.f_walk_time = *server.time + 0.2; // slow down if turning a lot

   // let's look for enemies...
   pBot->BotEnemy.pEdict = BotCheckForEnemies (pBot);

   // avoid walls, corners and teammates
   if (pBot->f_avoid_time < *server.time)
      BotAvoidObstacles (pBot);

   // are there armed grenades near us ?
   if (BotCheckForGrenades (pBot))
      pBot->BotMove.b_emergency_walkback = TRUE;

   // does an enemy exist ?
   if (!FNullEnt (pBot->BotEnemy.pEdict))
   {
      pBot->LastSeenEnemy = pBot->BotEnemy; // remember the last seen enemy
      pBot->f_pause_time = 0; // dont't pause
      BotShootAtEnemy (pBot); // shoot at the enemy

      return; // the bot has something to do
   }

   // else has the enemy suddently disappeared ?
   else if (!FNullEnt (pBot->LastSeenEnemy.pEdict))
   {
      // did the enemy just went out of FOV ?
      if (!IsInPlayerFOV (pBot->pEdict, pBot->LastSeenEnemy.pEdict->v.origin)
          && BotCanSeeThis (pBot, pBot->LastSeenEnemy.pEdict->v.origin))
      {
         // OMG, this enemy is circle-strafing us out !!!
         BotSetIdealAngles (pBot, UTIL_VecToAngles (pBot->LastSeenEnemy.pEdict->v.origin - GetGunPosition (pBot->pEdict)));
         pBot->BotMove.b_emergency_walkback = TRUE; // walk back to get the enemy back in field
      }

      // else has the enemy just gone hiding ?
      else if (IsInPlayerFOV (pBot->pEdict, pBot->LastSeenEnemy.pEdict->v.origin)
               && !BotCanSeeThis (pBot, pBot->LastSeenEnemy.pEdict->v.origin))
      {
         pBot->LastSeenEnemy.is_hiding = TRUE; // bot remembers this enemy is hiding

         //BotShootAtHiddenEnemy (pBot); // shoot at the hidden enemy

         // if bot is an engineer and has enough metal, try to drop a sentry gun here
         if ((pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
             && (*pBot->bot_weapons[TF_WEAPON_SPANNER].primary_ammo > 130))
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

         // if bot is waiting for enemy to strike back, don't move
         if (pBot->f_pause_time > *server.time)
         {
            pBot->BotMove.f_forward_time = 0; // don't move while pausing
            pBot->LastSeenEnemy.disappearance_time = *server.time; // set lost enemy time to now
         }

         // else rush after that coward one
         else if ((pBot->LastSeenEnemy.v_targetpoint - pBot->pEdict->v.origin).Length () > 50)
         {
            // if bot is unable to chase it, then just wander around
            if (!BotReachPosition (pBot, pBot->LastSeenEnemy.v_targetpoint))
               memset (&pBot->LastSeenEnemy, 0, sizeof (pBot->LastSeenEnemy)); // here we are, seems that bot really lost enemy
         }

         else
            memset (&pBot->LastSeenEnemy, 0, sizeof (pBot->LastSeenEnemy)); // here we are, seems that bot really lost enemy
      }

      return; // the bot has something to do
   }

   // else look for special zones
   else if (BotCheckForSpecialZones (pBot))
   {
      // is bot about to hit something it can jump up ?
      if ((pBot->BotBody.hit_state & OBSTACLE_FRONT_LOWWALL) && (pBot->BotMove.f_jump_time + 2.0 < *server.time))
         pBot->BotMove.f_jump_time = *server.time; // jump up and move forward

      // else is it about to hit something it can duck under ?
      else if (pBot->BotBody.hit_state & OBSTACLE_FRONT_LOWCEILING)
         pBot->BotMove.f_duck_time = *server.time + RANDOM_FLOAT (0.5, 1.5); // duck & go

      // if bot is about to fall...
      if (pBot->f_fallcheck_time < *server.time)
      {
         if (pBot->BotBody.hit_state & OBSTACLE_FRONT_FALL)
            BotTurnAtFall (pBot); // try to avoid falling
      }

      return;
   }

   // is bot keeping a place ?
   if (pBot->v_place_to_keep != g_vecZero)
      BotStayInPosition (pBot);

   // else is bot being "used" ?
   else if (pBot->pBotUser != NULL)
      BotFollowUser (pBot);

   // else may the bot spray a logo (don't spray if bot has an enemy) ?
   else if ((pBot->f_spraying_logo_time > *server.time) && FNullEnt (pBot->BotEnemy.pEdict))
   {
      pBot->BotMove.f_forward_time = 0; // don't move
      BotSetIdealPitch (pBot, -50); // look down at 45 degree angle
      pBot->f_reach_time = *server.time + 0.5; // don't reach point for half a second

      // is the bot finally looking down enough to spray its logo ?
      if (!pBot->b_logo_sprayed && (pBot->pEdict->v.v_angle.x > 45))
      {
         pBot->pEdict->v.impulse = 201; // spray logo when finished looking down
         pBot->BotMove.f_backwards_time = *server.time + RANDOM_FLOAT (0.5, 1.0); // move back
         pBot->b_logo_sprayed = TRUE; // remember this is done
         pBot->BotEyes.sample_time = *server.time; // open eyes again

         return;
      }
   }

   // else if nothing special to do...
   else
   {
      if (pBot->f_find_item_time < *server.time)
         BotCheckForItems (pBot); // if time to, see if there are any visible items
      else
         pBot->b_is_picking_item = FALSE;

      BotWander (pBot); // then just wander around
   }

   if (pBot->f_pause_time > *server.time) // is the bot "paused"?
      pBot->BotMove.f_forward_time = 0; // don't move while pausing

   return;
}


void BotPostThink (bot_t *pBot)
{
   // this is the last step of the bot Think() trilogy. In nature, every behaviour that can be
   // associated to intelligence is resulting of three invariable steps :
   // 1 - sensing the environment and the character's state
   // 2 - working out the changes to commit onto the environment and the character's state
   // 3 - performing these changes, and experiencing this action, looping back to step 1.
   // Here we deal with the third step, action. This is what I call the 'motile' part of the AI
   // (by opposition to the 'sensitive' part which is the first step of the trilogy). Motile
   // intelligence is the result of three "output vectors" - as formerly, more than three in fact,
   // but these three can be said to be representative of the human behaviour. These are: the
   // ability to communicate (chat), the ability to walk standing (move) and the ability to use
   // the hands as tools (point gun). We do all these actions here, provided the AI character
   // is "alive" in the game ; if not, it has, like a player, only the ability to chat. Once all
   // these three sub-steps of the motile part of the thinking cycle have been made, all we have
   // to do is to ask the engine to move the bot's player entity until the next frame, according
   // to our computations. This is done by calling the engine function RunPlayerMove().

   // handle bot speaking stuff
   BotChat (pBot);

   // is the bot alive in the game ?
   if (IsAlive (pBot->pEdict))
   {
      // handle bot moving stuff
      BotMove (pBot);
      BotPointGun (pBot);

      if (pBot->pEdict->v.velocity.z > MAX_SAFEFALL_SPEED)
         pBot->BotBody.fall_time = *server.time; // save bot fall time
      pBot->v_prev_position = pBot->pEdict->v.origin; // save previous position (for checking if stuck)
   }

   // ask the engine to do the fakeclient movement on server
   pfnRunPlayerMove (pBot->pEdict, WrapAngles (pBot->pEdict->v.v_angle), pBot->BotMove.f_move_speed, pBot->BotMove.f_strafe_speed, 0, pBot->pEdict->v.button, pBot->pEdict->v.impulse, server.msecval);

   // if this fakeclient has an illumination entity (RedFox's engine bug fix, thanks m8t :))
   if (!FNullEnt (pBot->pIllumination))
   {
      SET_ORIGIN (pBot->pIllumination, pBot->pEdict->v.origin); // make his light entity follow him
      if (pBot->pIllumination->v.nextthink + 0.1 < *server.time)
         pBot->pIllumination->v.nextthink = *server.time + 0.2; // make it think at 3 Hertz
   }

   return; // finished bot's motile part of the thinking cycle
}


void BotAnswerToOrder (bot_t *pBot)
{
   if (!IsValidPlayer (pBot->pEdict) || FNullEnt (pBot->BotEars.pAskingEntity))
      return; // reliability check

   // has the bot been asked to follow someone ?
   if (pBot->BotEars.bot_order == BOT_ORDER_FOLLOW)
   {
      // does the bot want to follow the caller ?
      if (FNullEnt (pBot->BotEnemy.pEdict)
          && ((RANDOM_LONG (1, 100) < 80) && pBot->b_is_fearful)
              || ((RANDOM_LONG (1, 100) < 40) && !pBot->b_is_fearful))
      {
         pBot->v_place_to_keep = g_vecZero; // don't stay in position anymore
         pBot->pBotUser = pBot->BotEars.pAskingEntity; // mark this client as using the bot
         pBot->v_lastseenuser_position = pBot->BotEars.pAskingEntity->v.origin; // remember last seen user position
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_FOLLOWOK; // bot acknowledges
      }
      else
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot refuses
   }

   // else has the bot been asked to check in ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_REPORT)
   {
      // does the bot want to answer the caller ?
      if (RANDOM_LONG (1, 100) < 66)
      {
         // does the bot have no enemy ?
         if (FNullEnt (pBot->BotEnemy.pEdict))
         {
            pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_REPORTING; // set him for reporting
            pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (1.0, 2.0);
         }
         else
         {
            pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_ATTACKING; // bot yells attack (audio)
            pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (1.0, 2.0);
         }
      }
   }

   // else has the bot been asked to keep a position ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_STAY)
   {
      // does the bot wants to obey the caller ?
      if (FNullEnt (pBot->BotEnemy.pEdict)
          && ((RANDOM_LONG (1, 100) < 80) && pBot->b_is_fearful)
              || ((RANDOM_LONG (1, 100) < 40) && !pBot->b_is_fearful))
      {
         pBot->v_place_to_keep = pBot->pEdict->v.origin; // position to stay in
         pBot->f_place_time = *server.time; // remember when we last saw the place to keep
         pBot->pBotUser = NULL; // free the user client slot
         pBot->v_lastseenuser_position = g_vecZero; // forget last seen user position
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_HOLDPOSITIONOK; // bot acknowledges
      }
      else
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot refuses
   }

   // else has the bot been asked to rush on his own ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_GO)
   {
      pBot->v_place_to_keep = g_vecZero; // don't stay in position anymore
      pBot->pBotUser = NULL; // free the user client slot
      pBot->v_lastseenuser_position = g_vecZero; // forget last seen user position
      if (!pBot->b_is_fearful)
         pBot->f_rush_time = *server.time + RANDOM_FLOAT (15.0, 45.0); // rush if not fearful

      // does the bot want to answer the caller ?
      if (RANDOM_LONG (1, 100) < 66)
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_AFFIRMATIVE; // bot acknowledges
   }

   // else has the bot been asked to set the plastic's timer to five seconds ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_DETONATEPLASTIC_5SECONDS)
   {
      // only demomans may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_DEMOMAN)
      {
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_AFFIRMATIVE; // bot acknowledges
         pBot->f_pause_time = *server.time + 5.0; // pause while planting plastic
         FakeClientCommand (pBot->pEdict, "detstart 5");
      }
      else
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot disagrees
   }

   // else has the bot been asked to set the plastic's timer to five seconds ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_DETONATEPLASTIC_20SECONDS)
   {
      // only demomans may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_DEMOMAN)
      {
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_AFFIRMATIVE; // bot acknowledges
         pBot->f_pause_time = *server.time + 5.0; // pause while planting plastic
         FakeClientCommand (pBot->pEdict, "detstart 20");
      }
      else
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot disagrees
   }

   // else has the bot been asked to disarm the plastic ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_DISARMPLASTIC)
   {
      // only demomans may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_DEMOMAN)
      {
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_AFFIRMATIVE; // bot acknowledges
         pBot->f_pause_time = *server.time + 5.0; // pause while defusing plastic
         FakeClientCommand (pBot->pEdict, "detstop");
      }
      else
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot disagrees
   }

   // else has the bot been asked to disguise as an enemy ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_DISGUISEENEMY)
   {
      // only spies may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_SPY)
      {
         char command[32];
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_AFFIRMATIVE; // bot acknowledges
         sprintf (command, "disguise_enemy %d", RANDOM_LONG (1, 9)); // pickup random class
         FakeClientCommand (pBot->pEdict, command); // issue the command
      }
      else
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot disagrees
   }

   // else has the bot been asked to disguise as a friend ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_DISGUISEFRIENDLY)
   {
      // only spies may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_SPY)
      {
         char command[32];
         int bot_class = TFC_CLASS_SPY;
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_AFFIRMATIVE; // bot acknowledges
         while (bot_class == TFC_CLASS_SPY)
            bot_class = RANDOM_LONG (1, 9); // pick up a random player class
         sprintf (command, "disguise_friendly %d", bot_class); // build the command
         FakeClientCommand (pBot->pEdict, command); // issue the command
      }
      else
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot disagrees
   }

   // else has the bot been asked to feign death ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_FEIGN)
   {
      // only spies may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_SPY)
      {
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_AFFIRMATIVE; // bot acknowledges
         FakeClientCommand (pBot->pEdict, "sfeign"); 
      }
      else
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot disagrees
   }

   // else has the bot been asked to stop feigning death ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_STOPFEIGN)
   {
      // only spies may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_SPY)
      {
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_AFFIRMATIVE; // bot acknowledges
         FakeClientCommand (pBot->pEdict, "feign"); 
      }
      else
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot disagrees
   }

   // else has the bot been asked to build a sentry gun ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_BUILDSENTRY)
   {
      // only engineers that have enough metal (> 130) may acknowledge
      if ((pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
          && (*pBot->bot_weapons[TF_WEAPON_SPANNER].primary_ammo > 130))
      {
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_AFFIRMATIVE; // bot acknowledges
         FakeClientCommand (pBot->pEdict, "build 2"); 
      }
      else
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot disagrees
   }

   // else has the bot been asked to rotate his sentry gun by 180 degrees ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_ROTATESENTRY_180DEGREES)
   {
      // only engineers may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
      {
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_AFFIRMATIVE; // bot acknowledges
         FakeClientCommand (pBot->pEdict, "rotatesentry180"); 
      }
      else
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot disagrees
   }

   // else has the bot been asked to rotate his sentry gun by 45 degrees ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_ROTATESENTRY_45DEGREES)
   {
      // only engineers may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
      {
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_AFFIRMATIVE; // bot acknowledges
         FakeClientCommand (pBot->pEdict, "rotatesentry"); 
      }
      else
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot disagrees
   }

   // else has the bot been asked to destroy his sentry gun ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_DETONATESENTRY)
   {
      // only engineers may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
      {
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_AFFIRMATIVE; // bot acknowledges
         FakeClientCommand (pBot->pEdict, "detsentry"); 
      }
      else
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot disagrees
   }

   // else has the bot been asked to build a dispenser ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_BUILDDISPENSER)
   {
      // only engineers that have enough metal (> 100) may acknowledge
      if ((pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
          && (*pBot->bot_weapons[TF_WEAPON_SPANNER].primary_ammo > 100))
      {
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_AFFIRMATIVE; // bot acknowledges
         FakeClientCommand (pBot->pEdict, "build 1"); 
      }
      else
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot disagrees
   }

   // else has the bot been asked to destroy his dispenser ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_BUILDDISPENSER)
   {
      // only engineers may acknowledge
      if (pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
      {
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_AFFIRMATIVE; // bot acknowledges
         FakeClientCommand (pBot->pEdict, "detdispenser"); 
      }
      else
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot disagrees
   }

   pBot->BotEars.bot_order = BOT_ORDER_NOORDER; // reset bot order field
   return;
}


void BotReactToSound (bot_t *pBot, noise_t *sound)
{
   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   // is it a bunker alarm sound ?
   if (strcmp ("ambience/siren.wav", sound->file_path) == 0)
   {
      // was the bot camping ?
      if (pBot->f_camp_time > *server.time)
      {
         pBot->v_place_to_keep = g_vecZero; // forget our camp spot...
         pBot->f_camp_time = *server.time; // ...get up...
         pBot->f_rush_time = *server.time + 60.0; // ...and run away !!
      }
   }

   // else is it a player movement sound ?
   else if (strncmp ("player/pl_", sound->file_path, 10) == 0)
   {
      // are there teammates around ?
      //if (bot is in squad OR teammates around)
         // don't process this sound

      // given the direction the bot thinks the sound is, let the bot have a look there
      if (sound->direction & DIRECTION_LEFT)
         BotAddIdealYaw (pBot, RANDOM_FLOAT (45, 135));
      else if (sound->direction & DIRECTION_RIGHT)
         BotAddIdealYaw (pBot, -RANDOM_FLOAT (45, 135));
      else if (sound->direction & DIRECTION_BACK)
         BotAddIdealYaw (pBot, RANDOM_FLOAT (135, 225));
   }

   return; // finished
}


void PlayClientSoundsForBots (edict_t *pPlayer)
{
   // this function determines if the player pPlayer is walking or running, or climbing a ladder,
   // or landing on the ground, and so if he's likely to emit some client sound or not. Since
   // these types of sounds are predicted on the client side only, and bots have no client DLL,
   // we have to simulate their emitting in order for the bots to hear them. So in case a player
   // is moving, we bring his footstep sounds to the ears of the bots around. This sound is based
   // on the texture the player is walking on. Using TraceTexture(), we ask the engine for that
   // texture, then look up in the step sounds database in order to determine which footstep
   // sound is related to that texture. The ladder check then assumes that a player moving
   // vertically, not on the ground, having a ladder in his immediate surroundings is climbing
   // it, and the ladder sound is emitted periodically the same way footstep sounds are emitted.
   // Then, the landing check looks for non-null value of the player's punch angles (screen
   // tilting) while this player's damage inflictor be either null, or the world. If the test
   // success, a landing sound is emitted as well.
   // thanks to Tom Simpson from FoxBot for the water sounds handling

   edict_t *pGroundEntity = NULL;
   const char *texture_name;
   char texture_type;
   char sound_path[256];
   int player_index;
   float player_velocity, volume;

   if (DebugLevel.is_observer && !(pPlayer->v.flags & FL_THIRDPARTYBOT))
      return; // skip real players if in observer mode

   player_index = ENTINDEX (pPlayer) - 1; // get the player index
   player_velocity = pPlayer->v.velocity.Length (); // get the player velocity

   // does the server allow footstep sounds AND this player is actually moving
   // AND is player on the ground AND is it time for him to make a footstep sound
   // OR has that player just landed on the ground after a jump ?
   if ((server.does_footsteps && IsOnFloor (pPlayer) && (player_velocity > 0)
        && (players[player_index].step_sound_time < *server.time))
       || ((FNullEnt (pPlayer->v.dmg_inflictor) || (pPlayer->v.dmg_inflictor == pWorldEntity))
           && (pPlayer->v.punchangle != g_vecZero) && !(players[player_index].prev_v.flags & (FL_ONGROUND | FL_PARTIALGROUND))))
   {
      // is this player sloshing in water ?
      if (pPlayer->v.waterlevel > 0)
      {
         sprintf (sound_path, "player/pl_slosh%d.wav", RANDOM_LONG (1, 4)); // build a slosh sound path

         // bring slosh sound from this player to the bots' ears
         DispatchSound (sound_path, pPlayer->v.origin + Vector (0, 0, -18), 0.9, ATTN_NORM);
         players[player_index].step_sound_time = *server.time + 0.300; // next slosh in 300 milliseconds
      }

      // else this player is definitely not in water, does he move fast enough to make sounds ?
      else if (player_velocity > MAX_WALK_SPEED)
      {
         // get the entity under the player's feet
         if (!FNullEnt (pPlayer->v.groundentity))
            pGroundEntity = pPlayer->v.groundentity; // this player is standing over something
         else
            pGroundEntity = pWorldEntity; // this player is standing over the world itself

         // ask the engine for the texture name on pGroundEntity under the player's feet
         texture_name = TRACE_TEXTURE (pGroundEntity, pPlayer->v.origin, Vector (0, 0, -9999));

         // if the engine found the texture, ask the game DLL for the texture type
         if (texture_name != NULL)
            texture_type = PM_FindTextureType ((char *) texture_name); // ask for texture type

         // given the type of texture under player's feet, prepare a sound file for being played
         switch (texture_type)
         {
            default:
            case CHAR_TEX_CONCRETE:
               sprintf (sound_path, "player/pl_step%d.wav", RANDOM_LONG (1, 4)); // 4 step sounds
               volume = 0.9;
               break;
            case CHAR_TEX_METAL:
               sprintf (sound_path, "player/pl_metal%d.wav", RANDOM_LONG (1, 4)); // 4 metal sounds
               volume = 0.9;
               break;
            case CHAR_TEX_DIRT:
               sprintf (sound_path, "player/pl_dirt%d.wav", RANDOM_LONG (1, 4)); // 4 dirt sounds
               volume = 0.9;
               break;
            case CHAR_TEX_VENT:
               sprintf (sound_path, "player/pl_duct%d.wav", RANDOM_LONG (1, 4)); // 4 duct sounds
               volume = 0.5;
               break;
            case CHAR_TEX_GRATE:
               sprintf (sound_path, "player/pl_grate%d.wav", RANDOM_LONG (1, 4)); // 4 grate sounds
               volume = 0.9;
               break;
            case CHAR_TEX_TILE:
               sprintf (sound_path, "player/pl_tile%d.wav", RANDOM_LONG (1, 5)); // 5 tile sounds
               volume = 0.8;
               break;
            case CHAR_TEX_SLOSH:
               sprintf (sound_path, "player/pl_slosh%d.wav", RANDOM_LONG (1, 4)); // 4 slosh sounds
               volume = 0.9;
               break;
            case CHAR_TEX_WOOD:
               sprintf (sound_path, "debris/wood%d.wav", RANDOM_LONG (1, 3)); // 3 wood sounds
               volume = 0.9;
               break;
            case CHAR_TEX_GLASS:
            case CHAR_TEX_COMPUTER:
               sprintf (sound_path, "debris/glass%d.wav", RANDOM_LONG (1, 4)); // 4 glass sounds
               volume = 0.8;
               break;
         }

         // did we hit a breakable ?
         if (!FNullEnt (pPlayer->v.groundentity)
             && (strcmp ("func_breakable", STRING (pPlayer->v.groundentity->v.classname)) == 0))
            volume /= 1.5; // drop volume, the object will already play a damaged sound

         // bring footstep sound from this player's feet to the bots' ears
         DispatchSound (sound_path, pPlayer->v.origin + Vector (0, 0, -18), volume, ATTN_NORM);
         players[player_index].step_sound_time = *server.time + 0.3; // next step in 300 milliseconds
      }
   }

   // is this player completely in water AND it's time to play a wade sound
   // AND this player is pressing the jump key for swimming up ?
   if ((players[player_index].step_sound_time < *server.time)
       && (pPlayer->v.waterlevel == 2) && (pPlayer->v.button & IN_JUMP))
   {
      sprintf (sound_path, "player/pl_wade%d.wav", RANDOM_LONG (1, 4)); // build a wade sound path

      // bring wade sound from this player to the bots' ears
      DispatchSound (sound_path, pPlayer->v.origin + Vector (0, 0, -18), 0.9, ATTN_NORM);
      players[player_index].step_sound_time = *server.time + 0.5; // next wade in 500 milliseconds
   }

   // now let's see if this player is on a ladder, for that we consider that he's not on the
   // ground, he's actually got a velocity (especially vertical), and that he's got a
   // func_ladder entity right in front of him. Is that player moving anormally NOT on ground ?
   if ((pPlayer->v.velocity.z != 0) && IsFlying (pPlayer) && !IsOnFloor (pPlayer))
   {
      pGroundEntity = NULL; // first ensure the pointer at which to start the search is NULL

      // cycle through all ladders...
      while ((pGroundEntity = UTIL_FindEntityByString (pGroundEntity, "classname", "func_ladder")) != NULL)
      {
         // is this ladder at the same height as the player AND the player is next to it (in
         // which case, assume he's climbing it), AND it's time for him to emit ladder sound ?
         if ((pGroundEntity->v.absmin.z < pPlayer->v.origin.z) && (pGroundEntity->v.absmax.z > pPlayer->v.origin.z)
             && (((pGroundEntity->v.absmin + pGroundEntity->v.absmax) / 2 - pPlayer->v.origin).Length2D () < 40)
             && (players[player_index].step_sound_time < *server.time))
         {
            volume = 0.8; // default volume for ladder sounds (empirical)

            // now build a random sound path amongst the 4 different ladder sounds
            sprintf (sound_path, "player/pl_ladder%d.wav", RANDOM_LONG (1, 4));

            // is the player ducking ?
            if (pPlayer->v.button & IN_DUCK)
               volume /= 1.5; // drop volume, the player is trying to climb silently

            // bring ladder sound from this player's feet to the bots' ears
            DispatchSound (sound_path, pPlayer->v.origin + Vector (0, 0, -18), volume, ATTN_NORM);
            players[player_index].step_sound_time = *server.time + 0.500; // next in 500 milliseconds
         }
      }
   }

   return;
}
