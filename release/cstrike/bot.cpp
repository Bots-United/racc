// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// CSTRIKE version
//
// bot.cpp
//

#include "racc.h"


void BotCreate (void)
{
   // this is where the show begins, i.e. the function that creates a bot. How it works :
   // I check which profiles are not currently in use by other bots. Next step, is to ask
   // the engine to create the fakeclient and give it a player entity pointer. And once
   // ClientPutInServer() has been called, ladies and gentlemen, please welcome our new bot.

   player_t *pPlayer;
   bool *usedprofiles; // mallocated
   int usedprofile_count;
   long index, profile_index;

   // start by sizing the used profiles flags array correctly and resetting it
   usedprofiles = (bool *) malloc (profile_count * sizeof (bool));
   if (usedprofiles == NULL)
      TerminateOnError ("BotCreate(): malloc failure on %d bytes (out of memory ?)\n", profile_count * sizeof (bool));
   memset (usedprofiles, 0, sizeof (usedprofiles));

   // cycle through all bot slots
   usedprofile_count = 0;
   for (index = 0; index < server.max_clients; index++)
   {
      pPlayer = &players[index]; // quick access to player

      if (!IsValidPlayer (pPlayer) || !pPlayer->is_racc_bot)
         continue; // skip invalid players and real clients

      // cycle through all the bot profiles we know
      for (profile_index = 0; profile_index < profile_count; profile_index++)
      {
         // does the bot have the same profile as this one ?
         if (&profiles[profile_index] == pPlayer->Bot.pProfile)
         {
            usedprofiles[profile_index] = TRUE; // this profile is used, so flag it
            usedprofile_count++; // increment the used profiles counter
         }
      }
   }

   // if all the profiles are used, that's there aren't enough living bots to join
   if (usedprofile_count == profile_count)
   {
      ServerConsole_printf ("RACC: not enough people in cybernetic population!\n"); // tell why
      server.max_bots = bot_count; // max out the bots to the current number

      if (usedprofiles != NULL)
         free (usedprofiles); // free the used profiles flags array
      usedprofiles = NULL;

      return; // ...and cancel bot creation
   }

   // pick up a profile that isn't used
   do
      profile_index = RandomLong (0, profile_count - 1); // pick up one randomly until not used
   while (usedprofiles[profile_index]); // keep searching until one is found

   // found one !

   if (usedprofiles != NULL)
      free (usedprofiles); // free the used profiles flags array
   usedprofiles = NULL;

   // okay, now we have a valid profile for our new bot
   pPlayer = CreateFakeClient (&profiles[profile_index]); // ask the engine to create a client

   // did the bot client creation NOT succeed ?
   if (pPlayer == NULL)
   {
      ServerConsole_printf ("RACC: game refuses to create bot client!\n"); // go complain
      return; // and cancel bot creation
   }

   // remember this player is a RACC bot
   pPlayer->is_racc_bot = TRUE;

   // initialize all the variables for this bot...

   BotReset (pPlayer); // reset our bot

   pPlayer->Bot.BotEyes.BotHUD.menu_state = MENU_CSTRIKE_IDLE; // not selecting team yet

   // if internet mode is on...
   if (server.is_internetmode)
   {
      pPlayer->Bot.time_to_live = server.time + RandomLong (300, 3600); // set him a TTL
      pPlayer->Bot.quit_game_time = pPlayer->Bot.time_to_live + RandomFloat (3.0, 7.0); // disconnect time
   }
   else
   {
      pPlayer->Bot.time_to_live = 0; // else don't set him a TTL (time to live)
      pPlayer->Bot.quit_game_time = 0; // so never quit
   }

   // say hello here
   if (RandomLong (1, 100) < (86 - 2 * player_count))
      pPlayer->Bot.BotChat.bot_saytext = BOT_SAYTEXT_HELLO;

   pPlayer->Bot.bot_alone_time = server.time + RandomLong (30, 120); // set an idle delay
   pPlayer->Bot.not_started = TRUE; // not started yet
}


void BotReset (player_t *pPlayer)
{
   // reset bot's input channels
   memset (&pPlayer->Bot.BotEyes, 0, sizeof (pPlayer->Bot.BotEyes));
   memset (&pPlayer->Bot.BotEars, 0, sizeof (pPlayer->Bot.BotEars));
   memset (&pPlayer->Bot.BotBody, 0, sizeof (pPlayer->Bot.BotBody));

   // reset bot's output channels
   memset (&pPlayer->Bot.BotLegs, 0, sizeof (pPlayer->Bot.BotLegs));
   memset (&pPlayer->Bot.BotHand, 0, sizeof (pPlayer->Bot.BotHand));
   memset (&pPlayer->Bot.BotChat, 0, sizeof (pPlayer->Bot.BotChat));

   // clean up the bot's pathmachine
   if (pPlayer->Bot.BotBrain.PathMachine.open != NULL)
      memset (pPlayer->Bot.BotBrain.PathMachine.open, 0, map.walkfaces_count * sizeof (navnode_t *));
   pPlayer->Bot.BotBrain.PathMachine.open_count = 0;
   pPlayer->Bot.BotBrain.PathMachine.finished = FALSE;
   pPlayer->Bot.BotBrain.PathMachine.busy = FALSE;

   // initialize some variables to default values
   BotSetIdealAngles (pPlayer, Vector (RandomFloat (-15, 15), RandomFloat (-180, 180), 0));

   pPlayer->Bot.finditem_time = 0.0;

   pPlayer->Bot.pTransportEntity = NULL;
   pPlayer->Bot.transport_type = 0;
   pPlayer->Bot.transport_direction = TRANSPORT_UNKNOWN;
   pPlayer->Bot.start_use_transport_time = 0.0;
   pPlayer->Bot.end_use_transport_time = 0.0;

   if (RandomLong (1, 100) < 33)
      pPlayer->Bot.is_fearful = TRUE;
   else
      pPlayer->Bot.is_fearful = FALSE;

   pPlayer->Bot.reload_time = -1;
   pPlayer->Bot.randomturn_time = 0.0;

   pPlayer->Bot.shoot_time = server.time;

   pPlayer->Bot.buy_state = 0;
   pPlayer->Bot.buy_time = server.time + RandomFloat (2.0, 5.0);
   pPlayer->Bot.rush_time = server.time + RandomFloat (15.0, 45.0);
   pPlayer->Bot.pause_time = pPlayer->Bot.buy_time;
   pPlayer->Bot.checkfootsteps_time = 0;

   pPlayer->Bot.v_place_to_keep = g_vecZero;
   pPlayer->Bot.place_time = 0;
   pPlayer->Bot.reach_time = 0;
   pPlayer->Bot.v_reach_point = g_vecZero;
   pPlayer->Bot.cornercheck_time = server.time + RandomFloat (10.0, 20.0);

   pPlayer->Bot.BotBrain.bot_goal = BOT_GOAL_NONE;
   pPlayer->Bot.BotBrain.bot_task = BOT_TASK_IDLE;
   pPlayer->Bot.v_goal = g_vecZero;

   pPlayer->Bot.is_interacting = FALSE;
   pPlayer->Bot.interact_time = 0;
   pPlayer->Bot.is_lift_moving = FALSE;
   pPlayer->Bot.spraylogo_time = 0;
   pPlayer->Bot.has_sprayed_logo = FALSE;

   memset (&pPlayer->Bot.BotEnemy, 0, sizeof (pPlayer->Bot.BotEnemy));
   memset (&pPlayer->Bot.LastSeenEnemy, 0, sizeof (pPlayer->Bot.LastSeenEnemy));
   memset (&pPlayer->Bot.LastSuspiciousSound, 0, sizeof (pPlayer->Bot.LastSuspiciousSound));
   memset (&pPlayer->Bot.bot_ammos, 0, sizeof (pPlayer->Bot.bot_ammos));
}


bool BotCheckForSpecialZones (player_t *pPlayer)
{
   edict_t *pSpecialZone;
   Vector v_zone_origin;
   player_t *pOtherPlayer;
   int index;

   if (!IsValidPlayer (pPlayer))
      return (FALSE); // reliability check

   if (DebugLevel.is_dontfindmode)
      return (FALSE); // don't process if botdontfind is set

   // is there a special zone near here?
   pSpecialZone = NULL;
   while ((pSpecialZone = FindEntityInSphere (pSpecialZone, pPlayer->v_origin, 1000)) != NULL)
   {
      if (BotCanSeeOfEntity (pPlayer, pSpecialZone) == g_vecZero)
         continue; // discard entity if bot can't see it

      v_zone_origin = OriginOf (pSpecialZone);

      // check for a visible safety zone
      if (strcmp ("func_vip_safetyzone", STRING (pSpecialZone->v.classname)) == 0)
      {
         // is bot a VIP ?
         if (strcmp ("vip", pPlayer->model) == 0)
         {
            pPlayer->Bot.BotLegs.forward_time = server.time + 0.5; // let our bot go...

            // is bot NOT already reaching this safety zone ?
            if (pPlayer->Bot.v_reach_point != v_zone_origin)
            {
               pPlayer->Bot.v_reach_point = v_zone_origin; // reach the safety zone
               FakeClientCommand (pPlayer->pEntity, RADIOMSG_COVERME); // bot speaks, "cover me!"
            }

            return (TRUE); // bot is concerned by this special zone
         }

         // else don't mind it if bot is camping
         else if (pPlayer->Bot.BotBrain.bot_goal & BOT_GOAL_PROTECTSITE)
            return (FALSE); // bot is not concerned by this special zone

         // else check if bot is a 'normal' counter-terrorist
         else if (GetTeam (pPlayer) == CSTRIKE_COUNTER_TERRORIST)
         {
            // cycle through all bot slots
            for (index = 0; index < server.max_clients; index++)
            {
               pOtherPlayer = &players[index]; // quick access to player

               if (!IsValidPlayer (pOtherPlayer) || (pOtherPlayer == pPlayer)
                   || !pOtherPlayer->is_alive || !pOtherPlayer->is_racc_bot)
                  continue; // skip invalid, dead players, real clients and self

               // is this one VIP AND visible AND not seeing safety zone ?
               if ((strcmp ("vip", pOtherPlayer->model) == 0)
                   && !BotCanSeeThis (pOtherPlayer, v_zone_origin)
                   && (BotCanSeeOfEntity (pPlayer, pOtherPlayer->pEntity) != g_vecZero))
               {
                  pOtherPlayer->Bot.v_place_to_keep = g_vecZero; // reset any v_place_to_keep

                  // let's make him head off toward us...
                  BotSetIdealYaw (pOtherPlayer, VecToAngles (pPlayer->v_origin - pOtherPlayer->v_origin).y);
                  pOtherPlayer->Bot.reach_time = server.time + 0.5; // make him ignore his reach points for a while

                  FakeClientCommand (pOtherPlayer->pEntity, RADIOMSG_AFFIRMATIVE); // make bot agree
                  pOtherPlayer->Bot.BotChat.bot_sayaudio = BOT_SAYAUDIO_AFFIRMATIVE;
                  return (FALSE); // normal CT is NOT directly concerned by this special zone
               }
            }

            return (FALSE); // normal CT can't see the VIP and is NOT concerned by this zone
         }
      }

      // check for a visible escape zone
      else if (strcmp ("func_escapezone", STRING (pSpecialZone->v.classname)) == 0)
      {
         // is bot a terrorist?
         if (GetTeam (pPlayer) == CSTRIKE_TERRORIST)
         {
            pPlayer->Bot.BotLegs.forward_time = server.time + 0.5; // let our bot go...

            // is bot NOT already reaching the escape zone ?
            if (pPlayer->Bot.v_reach_point != v_zone_origin)
            {
               pPlayer->Bot.v_reach_point = v_zone_origin; // reach the escape zone
               FakeClientCommand (pPlayer->pEntity, RADIOMSG_FALLBACK); // bot speaks, "fallback team!"
            }

            // cycle through all bot slots to find teammates
            for (index = 0; index < server.max_clients; index++)
            {
               pOtherPlayer = &players[index]; // quick access to player

               if (!IsValidPlayer (pOtherPlayer) || (pOtherPlayer == pPlayer)
                   || !pOtherPlayer->is_alive || !pOtherPlayer->is_racc_bot)
                  continue; // skip invalid, dead players, real clients and self

               // is this one terrorist AND visible AND not seeing escape zone ?
               if ((GetTeam (pOtherPlayer) == CSTRIKE_TERRORIST)
                   && !BotCanSeeThis (pOtherPlayer, v_zone_origin)
                   && (BotCanSeeOfEntity (pPlayer, pOtherPlayer->pEntity) != g_vecZero))
               {
                  pOtherPlayer->Bot.v_place_to_keep = g_vecZero; // reset any v_place_to_keep

                  // let's make him head off toward us...
                  BotSetIdealYaw (pOtherPlayer, VecToAngles (pPlayer->v_origin - pOtherPlayer->v_origin).y);
                  pOtherPlayer->Bot.reach_time = server.time + 0.5; // make him ignore his reach points for a while

                  FakeClientCommand (pOtherPlayer->pEntity, RADIOMSG_AFFIRMATIVE); // bot agrees

                  pOtherPlayer->Bot.BotChat.bot_sayaudio = BOT_SAYAUDIO_AFFIRMATIVE;
               }
            }

            return (TRUE); // bot is concerned by this special zone
         }
      }

      // check for a visible dropped bomb
      else if ((strcmp (STRING (pSpecialZone->v.model), "models/w_backpack.mdl") == 0)
               && (EnvironmentOf (pSpecialZone) == ENVIRONMENT_GROUND))
      {
         // both terrorists and counter-terrorists will head to the bomb. Because
         // counter-terrorists won't be able to pick it up, they will so
         // "cruise" around the bomb spot, permanently looking for enemies.

         // is bot a terrorist ?
         if (GetTeam (pPlayer) == CSTRIKE_TERRORIST)
            return (BotReachPosition (pPlayer, v_zone_origin)); // if bot is a T, go pick it up

         // else don't mind it if bot is camping
         else if (pPlayer->Bot.BotBrain.bot_goal & BOT_GOAL_PROTECTSITE)
            return (FALSE); // bot is not concerned by this special zone

         // else bot must be a counter-terrorist, can he camp near here ?
         else if (PlayerHasWeaponOfClass (pPlayer, WEAPON_CLASS_PRIMARY)
                  || PlayerHasWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_MEDIUMDAMAGE))
         {
            if (BotCanCampNearHere (pPlayer, v_zone_origin)
                && (RandomLong (1, 100) <= (91 - 2 * player_count)))
               FakeClientCommand (pPlayer->pEntity, RADIOMSG_INPOSITION); // let the bot camp here
            return (FALSE); // normal CTs are NOT directly concerned by this special zone
         }
      }

      // check for a visible planted bomb
      else if (strcmp (STRING (pSpecialZone->v.model), "models/w_c4.mdl") == 0)
      {
         // is bot a counter-terrorist ?
         if (GetTeam (pPlayer) == CSTRIKE_COUNTER_TERRORIST)
         {
            pPlayer->Bot.BotBrain.bot_goal |= BOT_GOAL_DEFUSEBOMB;
            pPlayer->Bot.v_goal = OriginOf (pSpecialZone);
            pPlayer->Bot.BotBrain.bot_task = BOT_TASK_FINDPATH; // find path to bomb
            return (TRUE); // bot is concerned by this special zone
         }

         // else don't mind it if bot is camping
         else if (pPlayer->Bot.BotBrain.bot_goal & BOT_GOAL_PROTECTSITE)
            return (FALSE); // bot is not concerned by this special zone

         // else bot must be a terrorist
         else
         {
            if (BotCanCampNearHere (pPlayer, v_zone_origin)
                && (RandomLong (1, 100) <= (91 - 2 * player_count)))
               FakeClientCommand (pPlayer->pEntity, RADIOMSG_INPOSITION); // let the bot camp here
            return (FALSE); // campers are NOT directly concerned by this special zone
         }
      }

      // check for a visible bomb target
      else if ((strcmp ("func_bomb_target", STRING (pSpecialZone->v.classname)) == 0)
               || (strcmp ("info_bomb_target", STRING (pSpecialZone->v.classname)) == 0))
      {
         // is bot a terrorist ?
         if (GetTeam (pPlayer) == CSTRIKE_TERRORIST)
         {
            // does the bot have the C4 ?
            if (pPlayer->Bot.BotEyes.BotHUD.icons_state[HUD_ICON_BOMB] != HUD_ICON_OFF)
            {
               BotPlantBomb (pPlayer, v_zone_origin); // plant bomb
               return (TRUE); // bot is concerned by this special zone
            }

            // if not, bot must be a 'normal' terrorist
            else
            {
               // cycle through all bot slots
               for (index = 0; index < server.max_clients; index++)
               {
                  pOtherPlayer = &players[index]; // quick access to player

                  if (!IsValidPlayer (pOtherPlayer) || (pOtherPlayer == pPlayer)
                      || !pOtherPlayer->is_alive || !pOtherPlayer->is_racc_bot)
                     continue; // skip invalid, dead players, real clients and self

                  // does this one have C4 AND visible AND not seeing bomb site ?
                  if ((pOtherPlayer->Bot.BotEyes.BotHUD.icons_state[HUD_ICON_BOMB] != HUD_ICON_OFF)
                      && !BotCanSeeThis (pOtherPlayer, v_zone_origin)
                      && (BotCanSeeOfEntity (pPlayer, pOtherPlayer->pEntity) != g_vecZero))
                  {
                     pOtherPlayer->Bot.v_place_to_keep = g_vecZero; // reset any v_place_to_keep

                     // let's make him head off toward us...
                     BotSetIdealYaw (pOtherPlayer, VecToAngles (pPlayer->v_origin - pOtherPlayer->v_origin).y);
                     pOtherPlayer->Bot.reach_time = server.time + 0.5; // make him ignore his reach points for a while

                     FakeClientCommand (pOtherPlayer->pEntity, RADIOMSG_YOUTAKETHEPOINT); // bot speaks, "you take the point!"
                     pOtherPlayer->Bot.BotChat.bot_sayaudio = BOT_SAYAUDIO_AFFIRMATIVE;
                  }
               }

               return (FALSE); // bot is NOT concerned by this special zone
            }
         }

         // else don't mind it if bot is camping already
         else if (pPlayer->Bot.BotBrain.bot_goal & BOT_GOAL_PROTECTSITE)
            return (FALSE); // bot is not concerned by this special zone

         // else bot must be a counter-terrorist, if bomb not planted yet can the bot camp near here ?
         else if ((mission.bomb != BOMB_PLANTED)
                  && (PlayerHasWeaponOfClass (pPlayer, WEAPON_CLASS_PRIMARY)
                      || PlayerHasWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_MEDIUMDAMAGE)))
         {
            if (BotCanCampNearHere (pPlayer, v_zone_origin)
                && (RandomLong (1, 100) <= (91 - 2 * player_count)))
               FakeClientCommand (pPlayer->pEntity, RADIOMSG_INPOSITION); // let the bot camp here
            return (FALSE); // campers are NOT directly concerned by this special zone
         }
      }

      // check for a visible hostage rescue zone
      else if (strcmp ("func_hostage_rescue", STRING (pSpecialZone->v.classname)) == 0)
      {
         // if bot is a terrorist, is it not already camping ?
         if ((GetTeam (pPlayer) == CSTRIKE_TERRORIST)
             && (pPlayer->Bot.BotBrain.bot_task != BOT_TASK_CAMP))
         {
            if (BotCanCampNearHere (pPlayer, v_zone_origin)
                && (RandomLong (1, 100) <= (91 - 2 * player_count)))
               FakeClientCommand (pPlayer->pEntity, RADIOMSG_INPOSITION); // let the bot camp here
            return (FALSE); // campers are NOT directly concerned by this special zone
         }
      }

      // check for a visible hostage
      else if (strcmp ("hostage_entity", STRING (pSpecialZone->v.classname)) == 0)
      {
         // is bot a terrorist ?
         if (GetTeam (pPlayer) == CSTRIKE_TERRORIST)
         {
            // check if the hostage is moving AND bot has no enemy yet
            if ((pSpecialZone->v.velocity != g_vecZero) && FNullEnt (pPlayer->Bot.BotEnemy.pEdict))
            {
               pPlayer->Bot.BotEnemy.pEdict = pSpecialZone; // alert, hostage flees away, shoot him!
               pPlayer->Bot.BotEnemy.appearance_time = server.time;
            }

            // else is the bot not already camping ?
            if (pPlayer->Bot.BotBrain.bot_task != BOT_TASK_CAMP)
               if (BotCanCampNearHere (pPlayer, v_zone_origin)
                   && (RandomLong (1, 100) <= (91 - 2 * player_count)))
                  FakeClientCommand (pPlayer->pEntity, RADIOMSG_INPOSITION); // let the bot camp here

            return (FALSE); // terrorists are NOT directly concerned by this special zone
         }

         // else bot must be a counter-terrorist
         else
         {
            // TODO: implement hostage usage
            return (FALSE);
         }
      }
   }

   return (FALSE); // bot found nothing interesting
}


bool BotCheckForGrenades (player_t *pPlayer)
{
   edict_t *pGrenade;
   Vector v_distance;
   float f_distance;
   float grenade_angle;

   if (DebugLevel.is_dontfindmode)
      return (FALSE); // don't process if botdontfind is set

   // is there an armed grenade near here?
   pGrenade = NULL;
   while ((pGrenade = FindEntityInSphere (pGrenade, pPlayer->v_origin, 300)) != NULL)
   {
      if (strcmp ("grenade", STRING (pGrenade->v.classname)) == 0)
         continue; // discard entity if it is NOT an armed grenade

      if (BotCanSeeOfEntity (pPlayer, pGrenade) == g_vecZero)
         continue; // discard entity if bot can't see it

      // check if this grenade is NOT a smoke grenade neither the C4 (not to confuse w/ bomb)
      if ((strcmp (STRING (pGrenade->v.model), "models/w_smokegrenade.mdl") != 0)
          && (strcmp (STRING (pGrenade->v.model), "models/w_c4.mdl") != 0))
      {
         // get the grenade's distance from the bot and its angle relatively to the bot
         v_distance = OriginOf (pGrenade) - pPlayer->v_origin;
         f_distance = v_distance.Length ();
         grenade_angle = VecToAngles (v_distance).y;

         BotSetIdealYaw (pPlayer, grenade_angle); // face the grenade...

         // ... and run away !!
         pPlayer->Bot.BotLegs.backwards_time = server.time + 0.5; // until the grenade explodes

         // is it a flashbang ?
         if (strcmp (STRING (pGrenade->v.model), "models/w_flashbang.mdl") == 0)
         {
            // strafe to (hopefully) take cover
            if (RandomLong (1, 100) < 50)
               pPlayer->Bot.BotLegs.strafeleft_time = server.time + RandomFloat (0.5, 2.0);
            else
               pPlayer->Bot.BotLegs.straferight_time = server.time + RandomFloat (0.5, 2.0);
         }

         // is this grenade going down AND is it quite close to the bot ?
         if ((pGrenade->v.velocity.z < 0) && (f_distance < 200))
            pPlayer->Bot.BotChat.bot_sayaudio = BOT_SAYAUDIO_SEEGRENADE; // bot says 'danger'

         return (TRUE); // bot is concerned by this grenade
      }
   }

   return (FALSE); // bot found nothing interesting
}


void BotCheckForItems (player_t *pPlayer)
{
   // this function makes the bot check if there's some interesting item to pick up nearby, and
   // in case yes, head up there to pick it up

   edict_t *pent;
   edict_t *pPickupEntity;
   Vector pickup_origin;
   Vector entity_origin;
   float min_distance = 501;
   bool can_pickup;

   if (DebugLevel.is_dontfindmode)
      return; // don't process if botdontfind is set

   pPlayer->Bot.is_picking_item = FALSE;
   pPickupEntity = NULL;

   // loop through all entities in game
   pent = NULL;
   while ((pent = FindEntityInSphere (pent, pPlayer->v_origin, 500)) != NULL)
   {
      if (BotCanSeeOfEntity (pPlayer, pent) == g_vecZero)
         continue; // discard entity if bot can't see it

      can_pickup = FALSE; // assume can't use it until known otherwise
      entity_origin = OriginOf (pent);

      // find distance to item for later use...
      float distance = (entity_origin - pPlayer->v_origin).Length ();

      // check if entity is a breakable...
      if (IsBreakable (pent) && (fabs (entity_origin.z - pPlayer->v_origin.z) < 60))
      {
         // check if close enough...
         if (distance < 50)
         {
            if (!PlayerHoldsWeaponOfRange (pPlayer, WEAPONRAIL_RANGE_MELEE))
               FakeClientCommand (pPlayer->pEntity, "weapon_knife"); // select a proximity weapon
            else
            {
               // point the weapon at the breakable and strike it
               BotLookAt (pPlayer, entity_origin);
               pPlayer->Bot.BotHand.fire1_time = server.time + 0.2; // strike the breakable
               pPlayer->Bot.reload_time = server.time + RandomLong (1.5, 3.0); // set next time to reload
            }
         }

         can_pickup = TRUE;
      }

      // check if entity is some interesting weapon...
      if (strcmp ("weaponbox", STRING (pent->v.classname)) == 0)
      {
         // check if the item is really interesting for our bot
         if (BotItemIsInteresting (pPlayer, pent))
         {
            if ((entity_origin - pPlayer->v_origin).Length () < 60)
               BotDiscardItem (pPlayer, pent); // discard our current stuff

            can_pickup = TRUE;
         }
         else
            can_pickup = FALSE;
      }

      // else check if entity is an abandoned defuse kit...
      if (strcmp ("item_thighpack", STRING (pent->v.classname)) == 0)
      {
         if ((pPlayer->Bot.BotEyes.BotHUD.icons_state[HUD_ICON_DEFUSER] == HUD_ICON_OFF)
             && (GetTeam (pPlayer) == CSTRIKE_COUNTER_TERRORIST))
            can_pickup = TRUE; // if bot has no defuse kit AND is a CT, go pick that one up
         else
            can_pickup = FALSE; // this item is not interesting
      }

      // did the bot find something it can pick up ?
      if (can_pickup)
      {
         float distance = (entity_origin - pPlayer->v_origin).Length ();

         // see if it's the closest item so far...
         if (distance < min_distance)
         {
            min_distance = distance; // update the minimum distance
            pPickupEntity = pent; // remember this entity
            pickup_origin = entity_origin; // remember location of entity
         }
      }
   }

   // now did the bot find around here an interesting entity it can pick up ?
   if (pPickupEntity != NULL)
   {
      pPlayer->Bot.is_picking_item = TRUE; // set bot picking item flag
      pPlayer->Bot.v_reach_point = pickup_origin; // save the location of item bot is trying to get
   }
   else
      pPlayer->Bot.is_picking_item = FALSE; // reset picking item flag
}


void BotSense (player_t *pPlayer)
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

////////////////////////////////////////////////////////////////////////////////////////////////
////  CUT AND PASTE THIS CODE WHERE YOU WANT THE BOT TO BREAK INTO THE DEBUGGER ON COMMAND  ////
////                                                                                        ////
if (DebugLevel.is_broke) _asm int 3; // x86 code only                                       ////
////////////////////////////////////////////////////////////////////////////////////////////////

   BotSee (pPlayer); // make bot see
   BotHear (pPlayer); // make bot hear
   BotTouch (pPlayer); // make bot touch

   return;
}


void BotThink (player_t *pPlayer)
{
   // this is the second step of the bot Think() trilogy. In nature, every behaviour that can be
   // associated to intelligence is resulting of three invariable steps :
   // 1 - sensing the environment and the character's state
   // 2 - working out the changes to commit onto the environment and the character's state
   // 3 - performing these changes, and experiencing this action, looping back to step 1.
   // Here we deal with the second step, thinking, where we involve all the cognitive stuff and
   // case-based reasoning that conditions the AI's behaviour.
   // Note: so far, this function is just a large bunch of if/else statments.

   pPlayer->Bot.BotLegs.emergency_walkback = FALSE;

   if (mission.finished)
      return; // if the mission is currently finishing, don't think.

   // if the bot hasn't selected its team and class yet...
   if (pPlayer->Bot.not_started)
   {
      if (pPlayer->Bot.BotEyes.BotHUD.menu_state != MENU_CSTRIKE_IDLE)
         BotStartGame (pPlayer); // browse the team and class selection menus when they appear
      return;
   }

   // is the bot controlled by the player ?
   if (pPlayer->Bot.is_controlled)
   {
      pPlayer->input_buttons = pListenserverPlayer->input_buttons; // duplicate the bot controls
      return; // and let the listen server player steer this bot
   }

   // is it time for the bot to leave the game ? (depending on his time to live)
   if ((pPlayer->Bot.time_to_live > 0) && (pPlayer->Bot.time_to_live < server.time))
   {
      pPlayer->Bot.time_to_live = server.time + 6.0; // don't say it twice (bad hack)
      BotSetIdealAngles (pPlayer, pPlayer->v_angle); // don't make it move its crosshair
      if (RandomLong (1, 100) <= (66 - 2 * player_count))
         pPlayer->Bot.BotChat.bot_saytext = BOT_SAYTEXT_BYE; // say goodbye
      return;
   }

   // if the bot is dead, wait for respawn...
   if (!pPlayer->is_alive)
   {
      // if not reset yet...
      if (pPlayer->Bot.BotEyes.sample_time > 0)
         BotReset (pPlayer); // reset our bot for next round

      // was the bot killed by another player AND has it not complained yet (on random) ?
      if (IsValidPlayer (&players[pPlayer->Bot.killer_index])
          && (pPlayer->Bot.BotChat.saytext_time < server.time)
          && (RandomLong (1, 100) <= (56 - 2 * player_count)))
         pPlayer->Bot.BotChat.bot_saytext = BOT_SAYTEXT_WHINE;

      return;
   }

   // cognitive part of the AI

   BotFindGoal (pPlayer); // find a goal, figure out what to do
   BotAnalyzeGoal (pPlayer); // once we know what to do, let's figure out HOW to do it
   BotExecuteTask (pPlayer); // now we have a precise, concrete task to do right now, just do it

   // think (or continue thinking) of a path if necessary
   BotRunPathMachine (pPlayer);

   // OLD STUFF BELOW /////////////////////////////////////////////////////////

/*   // should the bot complain of being alone for a long time ?
   if ((pPlayer->Bot.bot_alone_time > 0) && (pPlayer->Bot.bot_alone_time < server.time))
   {
      pPlayer->Bot.bot_alone_time = server.time + RandomLong (30, 120); // sets new delay

      if (RandomLong (1, 100) <= (66 - 2 * player_count))
      {
         pPlayer->Bot.BotChat.bot_saytext = BOT_SAYTEXT_ALONE; // complain

         // once out of three times send a radio message
         if (RandomLong (1, 100) < 34)
            if (RandomLong (1, 100) < 50)
               FakeClientCommand (pPlayer->pEntity, RADIOMSG_SECTORCLEAR);
            else
               FakeClientCommand (pPlayer->pEntity, RADIOMSG_REPORTINGIN);
      }
   }

   // should the bot yell for backup ?
   if (!pPlayer->Bot.already_asked_help && !FNullEnt (pPlayer->Bot.BotEnemy.pEdict) && (pPlayer->pEntity->v.health <= 20))
      if (RandomLong (1, 100) <= (91 - 2 * player_count))
      {
         pPlayer->Bot.BotChat.bot_saytext = BOT_SAYTEXT_NEEDBACKUP; // yell
         FakeClientCommand (pPlayer->pEntity, RADIOMSG_NEEDBACKUP); // send a radio message
         pPlayer->Bot.already_asked_help = TRUE; // don't do it twice
      }

   // is the bot planting the bomb ?
   if (pBotBrain->bot_task == BOT_TASK_PLANTBOMB)
   {
      if (pPlayer->Bot.BotEyes.BotHUD.icons_state[HUD_ICON_BOMB] == HUD_ICON_ON)
         pBotBrain->bot_task = BOT_TASK_IDLE; // stop planting if bomb icon doesn't blink anymore

      BotCheckForEnemies (pPlayer); // let the bot check for enemies

      if (!FNullEnt (pPlayer->Bot.BotEnemy.pEdict))
         pBotBrain->bot_task = BOT_TASK_IDLE; // stop planting if enemy found

      if (pPlayer->Bot.BotEyes.BotHUD.icons_state[HUD_ICON_BOMB] == HUD_ICON_OFF)
      {
         pBotBrain->bot_task = BOT_TASK_IDLE; // finished planting when C4 no more in hand
         if (BotHasPrimary (pPlayer))
            if (BotCanCampNearHere (pPlayer, pPlayer->v_origin)
                && (RandomLong (1, 100) <= (91 - 2 * player_count)))
               FakeClientCommand (pPlayer->pEntity, RADIOMSG_INPOSITION); // let the bot camp here
      }

      else if (pPlayer->Bot.current_weapon->hardware->id != CSTRIKE_WEAPON_C4)
         FakeClientCommand (pPlayer->pEntity, "weapon_c4"); // take the C4

      pBot->f_pause_time = server.time + 0.5; // pause the bot
      BotSetIdealAngles (pPlayer->Bot.WrapAngles (Vector (-45 / pBot->pProfile->skill, pPlayer->v_angle.y, 0))); // look down at 45 degree angle
      if (pPlayer->Bot.pProfile->skill > 2)
         pPlayer->Bot.BotLegs.f_duck_time = server.time + 0.5; // if bot is skilled enough, duck
      pPlayer->input_buttons.f_fire1_time = server.time + 0.2; // plant the bomb
      return;
   }

   // else is the bot defusing bomb ?
   else if (pBotBrain->bot_task == BOT_TASK_DEFUSEBOMB)
   {
      if (mission.bomb != BOMB_PLANTED)
         pBotBrain->bot_task = BOT_TASK_IDLE; // finished defusing when "bomb defused" message received
      Vector bot_angles = VecToAngles (pPlayer->Bot.v_goal - pPlayer->v_eyeposition);
      BotSetIdealAngles (pPlayer, Vector (bot_angles.x / 2 + bot_angles.x / (2 * pPlayer->Bot.pProfile->skill), bot_angles.y, bot_angles.z)); // look at bomb
      pPlayer->Bot.f_pause_time = server.time + 0.5; // pause the bot
      if (pPlayer->Bot.v_goal.z < pPlayer->v_origin.z)
         pPlayer->Bot.BotLegs.f_duck_time = server.time + 0.5; // if bomb is under the bot, let the bot duck
      if (pPlayer->Bot.BotEyes.BotHUD.has_progress_bar || (RandomLong (1, 100) < 95))
         pPlayer->input_buttons.f_use_time = server.time + 0.2; // keep pressing the button once the progress bar appeared
      return;
   }*/

/*   // has the bot been ordered something ?
   if ((pPlayer->Bot.BotEars.bot_order != BOT_ORDER_NOORDER)
       && (pPlayer->Bot.BotEars.order_time + 1.0 < server.time))
      BotAnswerToOrder (pPlayer); // answer to this order*/

   // is the bot alive and should the bot buy stuff now ?
   if (pPlayer->is_alive && (pPlayer->Bot.buy_time > 0) && (pPlayer->Bot.buy_time < server.time))
   {
      BotBuyStuff (pPlayer); // buy stuff
      return;
   }

   // is the bot blinded (e.g. affected by a flashbang) ?
   if (pPlayer->Bot.BotEyes.blinded_time > server.time)
   {
      pPlayer->Bot.BotLegs.duck_time = server.time + RandomFloat (0.5, 1.0); // duck when blinded

      // pick up a random strafe direction
      if (RandomLong (1, 100) < 50)
         pPlayer->Bot.BotLegs.strafeleft_time = server.time + 0.1;
      else
         pPlayer->Bot.BotLegs.straferight_time = server.time + 0.1;

      if (RandomLong (0, 100) < 50)
         pPlayer->Bot.BotLegs.emergency_walkback = TRUE;

      return;
   }

   // see how much we have turned
   if (fabs (pPlayer->Bot.BotHand.turn_speed.y) < 2.0)
      pPlayer->Bot.is_walking_straight = TRUE;
   else
      pPlayer->Bot.is_walking_straight = FALSE;

/*   if ((fabs (pPlayer->Bot.BotHand.v_turn_speed.x) > 10)
       || (fabs (pPlayer->Bot.BotHand.v_turn_speed.y) > 10))
      pPlayer->Bot.BotLegs.f_walk_time = server.time + 0.2; // slow down if turning a lot

   // let's look for enemies...
   BotCheckForEnemies (pPlayer);

   // avoid walls, corners and teammates
   BotAvoidObstacles (pPlayer);
   BotAvoidTeammates (pPlayer);

   // are there armed grenades near us ?
   if (BotCheckForGrenades (pPlayer))
      pPlayer->Bot.BotLegs.b_emergency_walkback = TRUE;

   // does an enemy exist ?
   if (!FNullEnt (pPlayer->Bot.BotEnemy.pEdict))
   {
      pPlayer->Bot.LastSeenEnemy = pPlayer->Bot.BotEnemy; // remember the last seen enemy
      pPlayer->Bot.f_pause_time = 0; // dont't pause
      BotShootAtEnemy (pPlayer); // shoot at the enemy

      return; // the bot has something to do
   }

   // else has the enemy suddently disappeared ?
   else if (!FNullEnt (pPlayer->Bot.LastSeenEnemy.pEdict))
   {
      // did the enemy just went out of FOV ?
      if (!IsInFieldOfView (pPlayer->pEntity, OriginOf (pPlayer->Bot.LastSeenEnemy.pEdict))
          && BotCanSeeThis (pPlayer, OriginOf (pPlayer->Bot.LastSeenEnemy.pEdict)))
      {
         // OMG, this enemy is circle-strafing us out !!!
         BotLookAt (pPlayer, OriginOf (pPlayer->Bot.LastSeenEnemy.pEdict));
         pPlayer->Bot.BotLegs.b_emergency_walkback = TRUE; // walk back to get the enemy back in field
      }

      // else has the enemy just gone hiding ?
      else if (IsInFieldOfView (pPlayer->pEntity, OriginOf (pPlayer->Bot.LastSeenEnemy.pEdict))
               && !BotCanSeeThis (pPlayer, OriginOf (pPlayer->Bot.LastSeenEnemy.pEdict)))
      {
         pPlayer->Bot.LastSeenEnemy.is_hiding = TRUE; // bot remembers this enemy is hiding

         //BotShootAtHiddenEnemy (pPlayer); // shoot at the hidden enemy

         // if bot is waiting for enemy to strike back, don't move
         if (pPlayer->Bot.f_pause_time > server.time)
            pPlayer->Bot.LastSeenEnemy.disappearance_time = server.time; // set lost enemy time to now

         // else rush after that coward one
         else if ((pPlayer->Bot.LastSeenEnemy.v_targetpoint - pPlayer->v_origin).Length () > 50)
         {
            // if bot is unable to chase it, then just wander around
            if (!BotReachPosition (pPlayer, pPlayer->Bot.LastSeenEnemy.v_targetpoint))
               memset (&pPlayer->Bot.LastSeenEnemy, 0, sizeof (pPlayer->Bot.LastSeenEnemy)); // here we are, seems that bot really lost enemy
         }

         else
            memset (&pPlayer->Bot.LastSeenEnemy, 0, sizeof (pPlayer->Bot.LastSeenEnemy)); // here we are, seems that bot really lost enemy
      }

      // any case else, the enemy apparently vanished into thin air !!!
      else
         memset (&pPlayer->Bot.LastSeenEnemy, 0, sizeof (pPlayer->Bot.LastSeenEnemy)); // all we can do is forget it

      return; // the bot has something to do
   }
*/
/*   // else look for special zones
   else if (BotCheckForSpecialZones (pPlayer))
   {
      // is bot about to hit something it can jump up ?
      if ((pPlayer->Bot.BotBody.hit_state & OBSTACLE_FRONT_LOWWALL)
          && (pPlayer->Bot.BotLegs.f_jump_time + 2.0 < server.time))
         pPlayer->Bot.BotLegs.f_jump_time = server.time; // jump up and move forward

      // else is it about to hit something it can duck under ?
      else if (pPlayer->Bot.BotBody.hit_state & OBSTACLE_FRONT_LOWCEILING)
         pPlayer->Bot.BotLegs.f_duck_time = server.time + RandomFloat (0.5, 1.5); // duck & go

      // if bot is about to fall...
      if (pPlayer->Bot.f_fallcheck_time < server.time)
      {
         if (pPlayer->Bot.BotBody.hit_state & OBSTACLE_FRONT_FALL)
            BotTurnAtFall (pPlayer); // try to avoid falling
      }

      return;
   }*/

/*   // is bot keeping a place ?
   if (pPlayer->Bot.v_place_to_keep != g_vecZero)
      BotCamp (pPlayer);

   // else is bot being "used" ?
   else if (pPlayer->Bot.pBotUser != NULL)
      BotFollowUser (pPlayer);

   // else may the bot spray a logo (don't spray if bot has an enemy) ?
   else if ((pPlayer->Bot.f_spraying_logo_time > server.time)
                && FNullEnt (pPlayer->Bot.BotEnemy.pEdict))
   {
      BotSetIdealPitch (pPlayer, -50); // look down at 45 degree angle
      pPlayer->Bot.reach_time = server.time + 0.5; // don't reach point for half a second

      // is the bot finally looking down enough to spray its logo ?
      if (!pPlayer->Bot.has_sprayed_logo && (pPlayer->v_angle.x > 45))
      {
         pPlayer->input_buttons |= INPUT_KEY_SPRAY; // spray logo when finished looking down
         pPlayer->Bot.BotLegs.f_backwards_time = server.time + RandomFloat (0.5, 1.0); // move back
         pPlayer->Bot.has_sprayed_logo = TRUE; // remember this is done
         pPlayer->Bot.BotEyes.sample_time = server.time; // open eyes again

         return;
      }
   }
*/
/*   // else if nothing special to do...
   else if (pBotBrain->bot_task == BOT_TASK_WANDER)
   {
      if (pPlayer->Bot.finditem_time < server.time)
         BotCheckForItems (pPlayer); // if time to, see if there are any visible items
      else
         pPlayer->Bot.b_is_picking_item = FALSE;

      BotWander (pPlayer); // let this bot wander around
   }*/

   return;
}


void BotAct (player_t *pPlayer)
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
   BotChat (pPlayer);

   // is the bot alive in the game ?
   if (pPlayer->is_alive)
   {
      // handle bot moving stuff
      if (mission.start_time < server.time)
         BotMove (pPlayer); // don't allow bots to move when freeze time is not elapsed yet
      BotPointGun (pPlayer);
      BotUseHand (pPlayer);
   }

   // is this bot being spectated by the listen server client ?
   if (pPlayer->is_watched && DebugLevel.aiconsole)
      DisplayAIConsole (pPlayer); // display debug AI console for this bot

   // ask the engine to do the fakeclient movement on server
   MoveFakeClient (pPlayer);

   // if bot is allowed to quit AND it's time to quit
   if (server.is_internetmode && (pPlayer->Bot.quit_game_time > 0) && (pPlayer->Bot.quit_game_time < server.time))
      sprintf (server.server_command, "kick \"%s\"\n", pPlayer->connection_name); // disconnect bot

   return; // finished bot's motile part of the thinking cycle
}


bool BotItemIsInteresting (player_t *pPlayer, edict_t *pItem)
{
   // this function returns TRUE if the considered item is interesting for the bot to pick it
   // up, FALSE otherwise.

   bool item_is_primary;
   bool item_is_secondary;
   bool bot_has_primary;
   bool bot_has_secondary;
   bool bot_holds_primary;
   bool bot_holds_secondary;

   // if it's a bomb and bot is a terrorist, this is some interesting stuff indeed!
   if ((pPlayer->Bot.pProfile->team == CSTRIKE_TERRORIST)
       && (strcmp ("backpack.mdl", STRING (pItem->v.model) + 9) == 0))
      return (TRUE); // this item is really interesting

   // see what's that
   item_is_primary = ItemIsWeaponOfClass (pItem, WEAPON_CLASS_PRIMARY);
   item_is_secondary = ItemIsWeaponOfClass (pItem, WEAPON_CLASS_SECONDARY);

   // see which weapons we have
   bot_has_primary = PlayerHasWeaponOfClass (pPlayer, WEAPON_CLASS_PRIMARY);
   bot_has_secondary = PlayerHasWeaponOfClass (pPlayer, WEAPON_CLASS_SECONDARY);
   bot_holds_primary = PlayerHoldsWeaponOfClass (pPlayer, WEAPON_CLASS_PRIMARY);
   bot_holds_secondary = PlayerHoldsWeaponOfClass (pPlayer, WEAPON_CLASS_SECONDARY);

   // if bot has no primary weapon or little ammo or no ammo left and this is a primary weapon...
   if (item_is_primary
       && (!bot_has_primary
           || (bot_holds_primary && (*pPlayer->Bot.current_weapon->primary_ammo < 8))
           || (bot_has_primary && !bot_holds_primary)))
      return (TRUE); // this item is really interesting

   // if bot has no secondary weapon or little ammo and this is a secondary weapon...
   if (item_is_secondary
       && (!bot_has_secondary
           || (bot_holds_secondary && (*pPlayer->Bot.current_weapon->primary_ammo < 8))))
      return (TRUE); // this item is really interesting

   return (FALSE); // all other stuff may not be interesting
}


void BotDiscardItem (player_t *pPlayer, edict_t *pItem)
{
   if (FNullEnt (pItem))
      return; // reliability check

   // if bot is wanting to pick up a primary weapon and needs to discard one to do so...
   if (ItemIsWeaponOfClass (pItem, WEAPON_CLASS_PRIMARY)
       && PlayerHasWeaponOfClass (pPlayer, WEAPON_CLASS_PRIMARY))
   {
      // is the bot NOT currently holding his primary weapon in hand ?
      if (!PlayerHoldsWeaponOfClass (pPlayer, WEAPON_CLASS_PRIMARY))
         BotSelectWeaponOfClass (pPlayer, WEAPON_CLASS_PRIMARY); // then select it
      else
         FakeClientCommand (pPlayer->pEntity, "drop"); // discard primary weapon
   }

   // else if the bot wants to pick up a secondary weapon...
   else if (ItemIsWeaponOfClass (pItem, WEAPON_CLASS_SECONDARY))
   {
      // is the bot NOT currently holding his secondary weapon in hand ?
      if (!PlayerHoldsWeaponOfClass (pPlayer, WEAPON_CLASS_SECONDARY))
         BotSelectWeaponOfClass (pPlayer, WEAPON_CLASS_SECONDARY); // then select it
      else
         FakeClientCommand (pPlayer->pEntity, "drop"); // discard secondary weapon
   }

   pPlayer->Bot.finditem_time = server.time + 3.0; // delay looking for items
}


void BotAnswerToOrder (player_t *pPlayer)
{
   if (FNullEnt (pPlayer->Bot.BotEars.pAskingEntity))
      return; // reliability check

   // has the bot been asked to follow someone ?
   if (pPlayer->Bot.BotEars.bot_order == BOT_ORDER_FOLLOW)
   {
      // does the bot want to follow the caller ?
      if (FNullEnt (pPlayer->Bot.BotEnemy.pEdict)
          && ((RandomLong (1, 100) < 80) && pPlayer->Bot.is_fearful)
              || ((RandomLong (1, 100) < 40) && !pPlayer->Bot.is_fearful))
      {
         // TODO: stuff to make bot follow caller

         pPlayer->Bot.BotChat.bot_saytext = BOT_SAYTEXT_FOLLOWOK; // bot acknowledges
         FakeClientCommand (pPlayer->pEntity, RADIOMSG_AFFIRMATIVE); // send a radio message
      }
      else
      {
         pPlayer->Bot.BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot refuses
         FakeClientCommand (pPlayer->pEntity, RADIOMSG_NEGATIVE); // send a radio message
      }
   }

   // else has the bot been asked to check in ?
   else if (pPlayer->Bot.BotEars.bot_order == BOT_ORDER_REPORT)
   {
      // does the bot want to answer the caller ?
      if (RandomLong (1, 100) < 66)
      {
         // does the bot have no enemy ?
         if (FNullEnt (pPlayer->Bot.BotEnemy.pEdict))
         {
            pPlayer->Bot.BotChat.bot_sayaudio = BOT_SAYAUDIO_REPORTINGIN; // set him for reporting
            if (RandomLong (1, 100) < 50)
               FakeClientCommand (pPlayer->pEntity, RADIOMSG_SECTORCLEAR); // send "sector clear" radio message
            else
               FakeClientCommand (pPlayer->pEntity, RADIOMSG_REPORTINGIN); // send "checking in" radio message
         }
         else
         {
            pPlayer->Bot.BotChat.bot_sayaudio = BOT_SAYAUDIO_ATTACKING; // bot yells attack (audio)
            FakeClientCommand (pPlayer->pEntity, RADIOMSG_TAKINGFIRE); // send "taking fire" radio message
         }
      }
   }

   // else has the bot been asked to keep a position ?
   else if (pPlayer->Bot.BotEars.bot_order == BOT_ORDER_STAY)
   {
      // does the bot wants to obey the caller ?
      if (FNullEnt (pPlayer->Bot.BotEnemy.pEdict)
          && ((RandomLong (1, 100) < 80) && pPlayer->Bot.is_fearful)
              || ((RandomLong (1, 100) < 40) && !pPlayer->Bot.is_fearful)
          && BotCanCampNearHere (pPlayer, pPlayer->v_origin))
      {
         pPlayer->Bot.BotChat.bot_saytext = BOT_SAYTEXT_HOLDPOSITIONOK; // bot acknowledges
         FakeClientCommand (pPlayer->pEntity, RADIOMSG_INPOSITION); // send a radio message
      }
      else
      {
         pPlayer->Bot.BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot refuses
         FakeClientCommand (pPlayer->pEntity, RADIOMSG_NEGATIVE); // send a radio message
      }
   }

   // else has the bot been asked to rush on his own ?
   else if (pPlayer->Bot.BotEars.bot_order == BOT_ORDER_GO)
   {
      pPlayer->Bot.v_place_to_keep = g_vecZero; // don't stay in position anymore

      if (!pPlayer->Bot.is_fearful)
         pPlayer->Bot.rush_time = server.time + RandomFloat (15.0, 45.0); // rush if not fearful

      // does the bot want to answer the caller ?
      if (RandomLong (1, 100) < 50)
         if (RandomLong (1, 100) < 66)
            pPlayer->Bot.BotChat.bot_saytext = BOT_SAYTEXT_AFFIRMATIVE; // bot acknowledges
         else
            FakeClientCommand (pPlayer->pEntity, RADIOMSG_AFFIRMATIVE); // send a radio message
   }

   pPlayer->Bot.BotEars.bot_order = BOT_ORDER_NOORDER; // reset bot order field
   return;
}


void BotReactToSound (player_t *pPlayer, noise_t *sound)
{
   if (DebugLevel.ears_disabled)
      return; // return if we don't want the AI to hear

   // is it a C4 close to explode sound ?
   if (strcmp ("weapons/c4_beep4", sound->file_path) == 0)
   {
      // was the bot camping ?
      if (pPlayer->Bot.BotBrain.bot_goal & BOT_GOAL_PROTECTSITE)
      {
         // FIXME: pick up a spot far away at random in the map and have the bot run there
         //pPlayer->Bot.v_goal = ;
         pPlayer->Bot.BotBrain.bot_task = BOT_TASK_FINDPATH; // ...get up...
         pPlayer->Bot.rush_time = server.time + 60.0; // ...and run away !!
         if (RandomLong (1, 100) <= (91 - 2 * player_count))
            FakeClientCommand (pPlayer->pEntity, RADIOMSG_FALLBACK); // bot says, "fallback team !"
      }
   }

   // else is the bot stopped or camping AND has the bot no enemy AND is it a player movement sound ?
   else if (((pPlayer->Bot.pause_time > server.time)
             || (pPlayer->Bot.BotBrain.bot_task == BOT_TASK_CAMP))
            && (pPlayer->Bot.checkfootsteps_time < server.time)
            && (FNullEnt (pPlayer->Bot.BotEnemy.pEdict)
            && (strncmp ("player/pl_", sound->file_path, 10) == 0)))
   {
      // are there teammates around ?
      //if (bot is in squad OR teammates around)
         // don't process this sound

      // given the direction the bot thinks the sound is, let the bot have a look there
      BotTurnTowardsDirection (pPlayer, sound->direction);

      pPlayer->Bot.checkfootsteps_time = server.time + 5.0;
   }

   /*if (strcmp ((char *) p, "#Follow_me") == 0) // 'Follow Me' radio command
   {
      // check if bot can see the caller
      if (BotGetIdealAimVector (&bots[bot_index], players[sender_index - 1].pEntity)) != g_vecZero)
      {
         bots[bot_index].BotEars.bot_order = BOT_ORDER_FOLLOW; // let the bot know he has been ordered something
         bots[bot_index].BotEars.pAskingEntity = players[sender_index - 1].pEntity; // remember asker
         bots[bot_index].BotEars.order_time = server.time; // remember when the order came
      }
   }
   else if (strcmp ((char *) p, "#Hold_this_position") == 0) // 'Hold This Position' radio command
   {
      // check if bot can see the caller
      if (BotGetIdealAimVector (&bots[bot_index], players[sender_index - 1].pEntity) != g_vecZero)
      {
         bots[bot_index].BotEars.bot_order = BOT_ORDER_STAY; // let the bot know he has been ordered something
         bots[bot_index].BotEars.pAskingEntity = players[sender_index - 1].pEntity; // remember asker
         bots[bot_index].BotEars.order_time = server.time; // remember when the order came
      }
   }
   else if ((strcmp ((char *) p, "#Go_go_go") == 0) // 'Go Go Go' radio command
            || (strcmp ((char *) p, "#Storm_the_front") == 0)) // 'Storm The Front' radio command
   {
      bots[bot_index].BotEars.bot_order = BOT_ORDER_GO; // let the bot know he has been ordered something
      bots[bot_index].BotEars.pAskingEntity = players[sender_index - 1].pEntity; // remember asker
      bots[bot_index].BotEars.order_time = server.time; // remember when the order came
   }
   else if (strcmp ((char *) p, "#Report_in_team") == 0) // 'Report In' radio command
   {
      bots[bot_index].BotEars.bot_order = BOT_ORDER_REPORT; // let the bot know he has been ordered something
      bots[bot_index].BotEars.pAskingEntity = players[sender_index - 1].pEntity; // remember asker
      bots[bot_index].BotEars.order_time = server.time; // remember when the order came
   }*/

   return; // finished
}


void PlayClientSoundsForBots (player_t *pPlayer)
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
   const char *texture_name, *player_weapon;
   char texture_type;
   int sound_index;
   char sound_path[256];
   float player_velocity, volume;

   if (DebugLevel.is_observer && !pPlayer->is_racc_bot)
      return; // skip real players if in observer mode

   player_velocity = pPlayer->v_velocity.Length (); // get the player velocity
   player_weapon = pPlayer->weapon_model + 9; // get player's weapon, skip 'models/p_'

   // does the server allow footstep sounds AND this player is actually moving
   // AND is player on the ground AND is it time for him to make a footstep sound
   // OR has that player just landed on the ground after a jump ?
   if ((server.does_footsteps && (player_velocity > 0)
        && ((pPlayer->environment == ENVIRONMENT_GROUND) || (pPlayer->environment == ENVIRONMENT_SLOSHING))
        && (pPlayer->step_sound_time < server.time))
       || pPlayer->has_just_fallen)
   {
      // is this player sloshing in water ?
      if (pPlayer->environment == ENVIRONMENT_SLOSHING)
      {
         sprintf (sound_path, "player/pl_slosh%d.wav", RandomLong (1, 4)); // build a slosh sound path

         // bring slosh sound from this player to the bots' ears
         DispatchSound (sound_path, pPlayer->v_origin + Vector (0, 0, -18), 0.9, ATTN_NORM);
         pPlayer->step_sound_time = server.time + 0.300; // next slosh in 300 milliseconds
      }

      // else this player is definitely not in water, does he move fast enough to make sounds ?
      else if (player_velocity > GameConfig.max_walk_speed)
      {
         // get the entity under the player's feet
         if (!FNullEnt (pPlayer->pEntity->v.groundentity))
            pGroundEntity = pPlayer->pEntity->v.groundentity; // this player is standing over something
         else
            pGroundEntity = pWorldEntity; // this player is standing over the world itself

         // ask the engine for the texture name on pGroundEntity under the player's feet
         texture_name = TRACE_TEXTURE (pGroundEntity, pPlayer->v_origin, Vector (0, 0, -9999));

         // if the engine found the texture, ask the game DLL for the texture type
         if (texture_name != NULL)
            texture_type = MDLL_PM_FindTextureType ((char *) texture_name); // ask for texture type

         // given the type of texture under player's feet, prepare a sound file for being played

         // loop through all the footstep sounds the bot knows until we find the right one
         for (sound_index = 0; sound_index < ricochetsound_count; sound_index++)
         {
            // is it this texture type the bullet just hit OR have we reached the default sound ?
            if ((texture_type == footstepsounds[sound_index].texture_type)
                || (footstepsounds[sound_index].texture_type == '*'))
               break; // then no need to search further
         }

         // have a copy of this footstep sound
         sprintf (sound_path, footstepsounds[sound_index].file_path);
         volume = footstepsounds[sound_index].volume;

         // did we hit a breakable ?
         if (!FNullEnt (pPlayer->pEntity->v.groundentity)
             && (strcmp ("func_breakable", STRING (pPlayer->pEntity->v.groundentity->v.classname)) == 0))
            volume /= 1.5; // drop volume, the object will already play a damaged sound

         // bring footstep sound from this player's feet to the bots' ears
         DispatchSound (sound_path, pPlayer->v_origin + Vector (0, 0, -18), volume, ATTN_NORM);
         pPlayer->step_sound_time = server.time + 0.3; // next step in 300 milliseconds
      }
   }

   // is this player completely in water AND it's time to play a wade sound
   // AND this player is pressing the jump key for swimming up ?
   if ((pPlayer->step_sound_time < server.time)
       && (pPlayer->environment == ENVIRONMENT_SLOSHING) && (pPlayer->input_buttons & INPUT_KEY_JUMP))
   {
      sprintf (sound_path, "player/pl_wade%d.wav", RandomLong (1, 4)); // build a wade sound path

      // bring wade sound from this player to the bots' ears
      DispatchSound (sound_path, pPlayer->v_origin + Vector (0, 0, -18), 0.9, ATTN_NORM);
      pPlayer->step_sound_time = server.time + 0.5; // next wade in 500 milliseconds
   }

   // now let's see if this player is on a ladder, for that we consider that he's not on the
   // ground, he's actually got a velocity (especially vertical), and that he's got a
   // func_ladder entity right in front of him. Is that player moving anormally NOT on ground ?
   if ((pPlayer->environment == ENVIRONMENT_LADDER) && (pPlayer->v_velocity.z != 0))
   {
      // cycle through all ladders...
      pGroundEntity = NULL;
      while ((pGroundEntity = FindEntityByString (pGroundEntity, "classname", "func_ladder")) != NULL)
      {
         // is this ladder at the same height as the player AND the player is next to it (in
         // which case, assume he's climbing it), AND it's time for him to emit ladder sound ?
         if ((pGroundEntity->v.absmin.z < pPlayer->v_origin.z)
             && (pGroundEntity->v.absmax.z > pPlayer->v_origin.z)
             && (((pGroundEntity->v.absmin + pGroundEntity->v.absmax) / 2 - pPlayer->v_origin).Length2D () < 40)
             && (pPlayer->step_sound_time < server.time))
         {
            volume = 0.8; // default volume for ladder sounds (empirical)

            // now build a random sound path amongst the 4 different ladder sounds
            sprintf (sound_path, "player/pl_ladder%d.wav", RandomLong (1, 4));

            // is the player ducking ?
            if (pPlayer->input_buttons & INPUT_KEY_DUCK)
               volume /= 1.5; // drop volume, the player is trying to climb silently

            // bring ladder sound from this player's feet to the bots' ears
            DispatchSound (sound_path, pPlayer->v_origin + Vector (0, 0, -18), volume, ATTN_NORM);
            pPlayer->step_sound_time = server.time + 0.500; // next in 500 milliseconds
         }
      }
   }

   // now let's see if this player is pulling the pin of a grenade...
   if ((pPlayer->input_buttons & INPUT_KEY_FIRE1) && !(pPlayer->prev.input_buttons & INPUT_KEY_FIRE1)
       && ((strcmp (player_weapon, "flashbang.mdl") == 0)
           || (strcmp (player_weapon, "hegrenade.mdl") == 0)
           || (strcmp (player_weapon, "smokegrenade.mdl") == 0)))
      DispatchSound ("weapons/pinpull.wav", pPlayer->v_eyeposition, 1.0, ATTN_NORM);

   return;
}


int GetTeam (player_t *pPlayer)
{
   // this function returns an integer '#define' describing the team to which belongs the player
   // structure pointed to by pPlayer. In case the team is not found, we return -1. The idea
   // of checking the 3rd character of the player model is courtesy of Wei "Whistler" Mingzhi.

   char discriminative = pPlayer->model[2];

   if ((discriminative == 'r') // "teRror" (Phoenix Connektion)
       || (discriminative == 'e') // "leEt" (L337 Krew) & "guErilla" (Guerilla Warfare)
       || (discriminative == 'c')) // "arCtic" (Arctic Avenger)
      return (CSTRIKE_TERRORIST);
   else if ((discriminative == 'b') // "urBan" (Seal Team 6)
            || (discriminative == 'g') // "gsG9" (German GSG-9) & "giGn" (French GIGN)
            || (discriminative == 's') // "saS" (UK SAS)
            || (discriminative == 'p')) // "viP" (VIP)
      return (CSTRIKE_COUNTER_TERRORIST);

   return (-1); // return -1 if team is unknown
}
