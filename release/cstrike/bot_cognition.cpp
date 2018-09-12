// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// bot_cognition.cpp
//

#include "racc.h"


void BotFindGoal (player_t *pPlayer)
{
   // this function makes the bot thinks about what it wants to do, and puts it in the right
   // state of mind for accomplishing the required actions. Depending on the team/class and/or
   // mission objectives for the player character controlled by this bot, we cycle through all
   // the potential goals and find the best one for us.

   bot_brain_t *pBotBrain;
   edict_t *pEntity;
   float distance, min_distance;
   noise_t *bomb_noise;

   pBotBrain = &pPlayer->Bot.BotBrain; // quick access to bot brain

   if (DebugLevel.is_dontfindmode)
      return; // don't process if botdontfind is set

   // has the bot already a goal (or was this goal not reset yet) ?
   if (pBotBrain->bot_goal != BOT_GOAL_NONE)
      return; // cancel, bot already knows what to do

   if (pPlayer->is_watched && (DebugLevel.cognition > 0))
      printf ("BOT %s REEVALUATING GOAL!!!\n", pPlayer->connection_name);

   pBotBrain->bot_goal = BOT_GOAL_NONE; // reset bot goal

   // check if bot is VIP
   if (strcmp ("vip", pPlayer->model) == 0)
   {
      min_distance = 9999.0; // reset min_distance

      // loop through all VIP safety zones
      pEntity = NULL;
      while ((pEntity = FindEntityByString (pEntity, "classname", "func_vip_safetyzone")) != NULL)
      {
         distance = (OriginOf (pEntity) - pPlayer->v_origin).Length ();

         // randomly choose the nearest safety zone
         if (((distance < min_distance) && (RandomLong (1, 100) < 66))
             || (RandomLong (1, 100) < 33))
         {
            pBotBrain->bot_goal |= BOT_GOAL_REACHSAFETYZONE; // VIP's goal is to reach safety
            min_distance = distance; // update the minimum distance

            pPlayer->Bot.v_goal = ReachableOriginOf (pEntity); // remember this entity
         }
      }

      return;
   }

   // else check if bot is terrorist
   else if (GetTeam (pPlayer) == CSTRIKE_TERRORIST)
   {
      // several things to check when bot is a terrorist. Let's check them all in priority order,
      // the lesser priority things first (so that we end up with the higher priority stuff)...

      min_distance = 9999.0; // reset min_distance

      // loop through all escape zones
      pEntity = NULL;
      while ((pEntity = FindEntityByString (pEntity, "classname", "func_escapezone")) != NULL)
      {
         distance = (OriginOf (pEntity) - pPlayer->v_origin).Length ();

         // randomly choose the nearest goal
         if (((distance < min_distance) && (RandomLong (1, 100) < 66))
             || (RandomLong (1, 100) < 33))
         {
            pBotBrain->bot_goal |= BOT_GOAL_REACHSAFETYZONE; // on this map, goal is to reach safety
            min_distance = distance; // update the minimum distance
            pPlayer->Bot.v_goal = ReachableOriginOf (pEntity); // remember this entity
         }
      }

      // is there a bomb on this map ?
      if (mission.bomb != BOMB_NONE)
      {
         // was the bomb dropped somewhere ?
         if (mission.bomb == BOMB_DROPPED)
         {
            // loop through all dropped stuff
            pEntity = NULL;
            while ((pEntity = FindEntityByString (pEntity, "classname", "weaponbox")) != NULL)
            {
               if ((strcmp ("backpack.mdl", STRING (pEntity->v.model) + 9) != 0)
                   || (EnvironmentOf (pEntity) != ENVIRONMENT_GROUND))
                  continue; // discard any dropped stuff that is NOT a dropped bomb

               pBotBrain->bot_goal |= BOT_GOAL_PICKBOMB; // bot must find the bomb and pick it up
               pPlayer->Bot.v_goal = OriginOf (pEntity); // remember this entity
            }
         }

         // else does someone carry the bomb on this team ?
         else if (mission.bomb == BOMB_CARRIED)
         {
            min_distance = 9999.0; // reset min_distance

            // loop through all bomb zones
            pEntity = NULL;
            while ((pEntity = FindEntityByString (pEntity, "classname", "func_bomb_target")) != NULL)
            {
               distance = (OriginOf (pEntity) - pPlayer->v_origin).Length ();

               // randomly choose the nearest one
               if (((distance < min_distance) && (RandomLong (1, 100) < 66))
                   || (RandomLong (1, 100) < 33))
               {
                  pBotBrain->bot_goal |= BOT_GOAL_PLANTBOMB; // bot's goal is to blow stuff up, haha!
                  min_distance = distance; // update the minimum distance
                  pPlayer->Bot.v_goal = ReachableOriginOf (pEntity); // remember this entity
               }
            }

            // loop through all bomb spots
            pEntity = NULL;
            while ((pEntity = FindEntityByString (pEntity, "classname", "info_bomb_target")) != NULL)
            {
               distance = (OriginOf (pEntity) - pPlayer->v_origin).Length ();

               // randomly choose the nearest goal if bomb not planted yet
               if (((distance < min_distance) && (RandomLong (1, 100) < 66))
                   || (RandomLong (1, 100) < 33))
               {
                  pBotBrain->bot_goal |= BOT_GOAL_PLANTBOMB; // bot's goal is to blow stuff up, haha!
                  min_distance = distance; // update the minimum distance
                  pPlayer->Bot.v_goal = ReachableOriginOf (pEntity); // remember this entity
               }
            }
         }

         // else was the bomb already planted ?
         else if (mission.bomb == BOMB_PLANTED)
         {
            // figure out where is that bomb the bot sees on its radar, and once this is known,
            // head up there in order to secure the site

            // figure out where's the bomb first
            // FIXME: make the bot actually ESTIMATE the position on radar instead of knowing it.
            pEntity = NULL;
            while ((pEntity = FindEntityByString (pEntity, "classname", "grenade")) != NULL)
            {
               // does this grenade look like a planted C4 ?
               if (strcmp ("models/w_c4.mdl", STRING (pEntity->v.model)) == 0)
               {
                  pBotBrain->bot_goal |= BOT_GOAL_PROTECTSITE; // bot's goal is to defend the bomb
                  pPlayer->Bot.v_goal = OriginOf (pEntity); // remember the location of the site
                  break; // no need to search further once the bomb is found
               }
            }
         }

         // end of bomb-related goals

         // TODO: hostage related goals for Ts
      }

      return;
   }

   // else check if bot is counter-terrorist
   else if (GetTeam (pPlayer) == CSTRIKE_COUNTER_TERRORIST)
   {
      // several things to check when bot is a CT. Let's check them all in priority order,
      // the lesser priority things first (so that we end up with the higher priority stuff)...

      min_distance = 9999.0; // reset min_distance

      // loop through all escape zones
      pEntity = NULL;
      while ((pEntity = FindEntityByString (pEntity, "classname", "func_escapezone")) != NULL)
      {
         distance = (OriginOf (pEntity) - pPlayer->v_origin).Length ();

         // randomly choose the nearest goal
         if (((distance < min_distance) && (RandomLong (1, 100) < 66))
             || (RandomLong (1, 100) < 33))
         {
            pBotBrain->bot_goal |= BOT_GOAL_PROTECTSITE; // bot must protect the escape zones
            min_distance = distance; // update the minimum distance
            pPlayer->Bot.v_goal = ReachableOriginOf (pEntity); // remember this entity
         }
      }

      // is there a bomb on this map ?
      if (mission.bomb != BOMB_NONE)
      {
         // was the bomb dropped somewhere ?
         if (mission.bomb == BOMB_DROPPED)
         {
            // loop through all dropped stuff
            pEntity = NULL;
            while ((pEntity = FindEntityByString (pEntity, "classname", "weaponbox")) != NULL)
            {
               if ((strcmp ("backpack.mdl", STRING (pEntity->v.model) + 9) != 0)
                   || (EnvironmentOf (pEntity) != ENVIRONMENT_GROUND))
                  continue; // discard any dropped stuff that is NOT a dropped bomb

               pBotBrain->bot_goal |= BOT_GOAL_PROTECTSITE; // bot must prevent Ts from picking bomb
               pPlayer->Bot.v_goal = OriginOf (pEntity); // remember this entity
            }
         }

         // else does someone carry the bomb on the opposite team ?
         else if (mission.bomb == BOMB_CARRIED)
         {
            min_distance = 9999.0; // reset min_distance

            // loop through all bomb zones
            pEntity = NULL;
            while ((pEntity = FindEntityByString (pEntity, "classname", "func_bomb_target")) != NULL)
            {
               distance = (OriginOf (pEntity) - pPlayer->v_origin).Length ();

               // randomly choose the nearest one
               if (((distance < min_distance) && (RandomLong (1, 100) < 66))
                   || (RandomLong (1, 100) < 33))
               {
                  pBotBrain->bot_goal |= BOT_GOAL_PROTECTSITE; // bot must prevent Ts from planting bomb
                  min_distance = distance; // update the minimum distance
                  pPlayer->Bot.v_goal = ReachableOriginOf (pEntity); // remember this entity
               }
            }

            // loop through all bomb spots
            pEntity = NULL;
            while ((pEntity = FindEntityByString (pEntity, "classname", "info_bomb_target")) != NULL)
            {
               distance = (OriginOf (pEntity) - pPlayer->v_origin).Length ();

               // randomly choose the nearest goal if bomb not planted yet
               if (((distance < min_distance) && (RandomLong (1, 100) < 66))
                   || (RandomLong (1, 100) < 33))
               {
                  pBotBrain->bot_goal |= BOT_GOAL_PROTECTSITE; // bot must prevent Ts from planting bomb
                  min_distance = distance; // update the minimum distance
                  pPlayer->Bot.v_goal = ReachableOriginOf (pEntity); // remember this entity
               }
            }
         }

         // else has someone planted the bomb on the opposite team ?
         else if (mission.bomb == BOMB_PLANTED)
         {
            min_distance = 9999.0; // reset min_distance

            // check if the bot already hears the bomb
            bomb_noise = BotDoesHear (pPlayer, "weapons/c4_beep");

            // loop through all bomb zones
            pEntity = NULL;
            while ((pEntity = FindEntityByString (pEntity, "classname", "func_bomb_target")) != NULL)
            {
               distance = (OriginOf (pEntity) - pPlayer->v_origin).Length ();

               // randomly choose the nearest one UNLESS bot already hears the bomb around here
               if (((distance < min_distance)
                    && ((bomb_noise != NULL) || (RandomLong (1, 100) < 66)))
                   || (RandomLong (1, 100) < 33))
               {
                  pBotBrain->bot_goal |= BOT_GOAL_DEFUSEBOMB; // bot must prevent the bomb to blow
                  min_distance = distance; // update the minimum distance
                  pPlayer->Bot.v_goal = ReachableOriginOf (pEntity); // remember this entity
               }
            }

            // loop through all bomb spots
            pEntity = NULL;
            while ((pEntity = FindEntityByString (pEntity, "classname", "info_bomb_target")) != NULL)
            {
               distance = (OriginOf (pEntity) - pPlayer->v_origin).Length ();

               // randomly choose the nearest goal UNLESS bot already hears the bomb around here
               if (((distance < min_distance)
                    && ((bomb_noise != NULL) || (RandomLong (1, 100) < 66)))
                   || (RandomLong (1, 100) < 33))
               {
                  pBotBrain->bot_goal |= BOT_GOAL_DEFUSEBOMB; // bot must prevent the bomb to blow
                  min_distance = distance; // update the minimum distance
                  pPlayer->Bot.v_goal = ReachableOriginOf (pEntity); // remember this entity
               }
            }
         }

         // end of bomb-related goals

         // TODO: hostage related goals for CTs
      }

      return;
   }

   return;
}


void BotAnalyzeGoal (player_t *pPlayer)
{
   // this function makes the bot decides what to do in order to reach the goal it assigned
   // itself, from figuring out a path to it to taking the appropriate actions when it has
   // reached the destination. NOTE: it does NOT make the bot ACT, instead it makes it decide
   // HOW to act. Since the bot may have several goals at once, all the checks must be made
   // successively. Hence the order in which we do things is important : we check from the
   // least priority goal to the most important one.

   static bot_brain_t *pBotBrain;

   pBotBrain = &pPlayer->Bot.BotBrain; // quick access to bot brain

   if (pBotBrain->bot_goal == BOT_GOAL_NONE)
      return; // don't do anything if bot has no goal

   // is our bot's goal to find the bomb and pick it up ?
   if (pBotBrain->bot_goal & BOT_GOAL_PICKBOMB)
   {
      // bot already knows where is the bomb (v_goal), so first off it needs to get there, then
      // look for the bomb and pick it up.

      // is bot NOT finding a path yet NOR walking it NOR already picking up the bomb ?
      if ((pBotBrain->bot_task != BOT_TASK_FINDPATH)
          && (pBotBrain->bot_task != BOT_TASK_WALKPATH)
          && (pBotBrain->bot_task != BOT_TASK_PICKBOMB))
      {
         // is the bot just arrived where the bomb has been dropped ?
         if ((pPlayer->Bot.v_goal - pPlayer->v_origin).Length () < 200)
            pBotBrain->bot_task = BOT_TASK_PICKBOMB; // have the bot have a look around to find that bomb

         // grmbl, this lazy bot is doing nothing.
         else
            pBotBrain->bot_task = BOT_TASK_FINDPATH; // bot must reach the spot where the bomb is
      }
   }

   // is our bot's goal to plant the bomb ?
   if (pBotBrain->bot_goal & BOT_GOAL_PLANTBOMB)
   {
      // bot already knows where the bomb site is (v_goal), so first off it needs to get there,
      // then look for a place to plant the bomb and plant it.

      // is bot NOT finding a path yet NOR walking it NOR already planting the bomb ?
      if ((pBotBrain->bot_task != BOT_TASK_FINDPATH)
          && (pBotBrain->bot_task != BOT_TASK_WALKPATH)
          && (pBotBrain->bot_task != BOT_TASK_PLANTBOMB))
      {
         // is the bot just arrived at the bomb site ?
         if ((pPlayer->Bot.v_goal - pPlayer->v_origin).Length () < 200)
         {
            // does the bot have a bomb in its inventory ?
            if (pPlayer->Bot.BotEyes.BotHUD.icons_state[HUD_ICON_BOMB] != HUD_ICON_OFF)
               pBotBrain->bot_task = BOT_TASK_PLANTBOMB; // have the bot find a safe spot to plant the bomb
            else
               pBotBrain->bot_task = BOT_TASK_CAMP; // have the bot camp around here
         }

         // grmbl, this lazy bot is doing nothing.
         else
            pBotBrain->bot_task = BOT_TASK_FINDPATH; // bot must reach the bomb site
      }
   }

   // is our bot's goal to defuse the bomb ?
   if (pBotBrain->bot_goal & BOT_GOAL_DEFUSEBOMB)
   {
      // bot already knows where the bomb site is (v_goal), so first off it needs to get there,
      // then look for a place where the bomb can be and defuse it.

      // is bot NOT finding a path yet NOR walking it NOR already defusing the bomb ?
      if ((pBotBrain->bot_task != BOT_TASK_FINDPATH)
          && (pBotBrain->bot_task != BOT_TASK_WALKPATH)
          && (pBotBrain->bot_task != BOT_TASK_DEFUSEBOMB)
          && (pBotBrain->bot_task != BOT_TASK_USECHARGER))
      {
         // is the bot just arrived at the bomb site ?
         if ((pPlayer->Bot.v_goal - pPlayer->v_origin).Length () < 200)
         {
            pBotBrain->bot_task = BOT_TASK_DEFUSEBOMB; // have a look around to find that bomb

            // should the bot listen to the bomb ?
            if (pBotBrain->listen_time + 1 < server.time)
               pBotBrain->listen_time = server.time + 2.0; // have the bot listen carefully
         }

         // grmbl, this lazy bot is doing nothing.
         else
            pBotBrain->bot_task = BOT_TASK_FINDPATH; // bot must reach the bomb site
      }
   }

   // is our bot's goal to protect a site ?
   if (pBotBrain->bot_goal & BOT_GOAL_PROTECTSITE)
   {
      // bot already knows where the site to protect is (v_goal), so first off it needs to get
      // there, then look for a safe place where to ambush in.

      // is bot NOT finding a path yet NOR walking it NOR already camping ?
      if ((pBotBrain->bot_task != BOT_TASK_FINDPATH)
          && (pBotBrain->bot_task != BOT_TASK_WALKPATH)
          && (pBotBrain->bot_task != BOT_TASK_CAMP))
      {
         // is the bot just arrived at the site to protect ?
         if ((pPlayer->Bot.v_goal - pPlayer->v_origin).Length () < 200)
            pBotBrain->bot_task = BOT_TASK_CAMP; // have the bot camp around here

         // grmbl, this lazy bot is doing nothing.
         else
            pBotBrain->bot_task = BOT_TASK_FINDPATH; // bot must reach the site to protect
      }
   }

   // is our bot's goal to reach a rescue zone ?
   if (pBotBrain->bot_goal & BOT_GOAL_REACHSAFETYZONE)
   {
      // bot already knows where the site to reach is (v_goal), so first off it needs to get
      // there, and then... uhhh... well... well then, he has won =)

      // is bot NOT finding a path yet NOR walking it ?
      if ((pBotBrain->bot_task != BOT_TASK_FINDPATH)
          && (pBotBrain->bot_task != BOT_TASK_WALKPATH))
         pBotBrain->bot_task = BOT_TASK_FINDPATH; // bot must reach the safety zone
   }

   // is our bot's goal to assassinate the VIP ?
   if (pBotBrain->bot_goal & BOT_GOAL_ASSASSINATEVIP)
   {
      // assassinating a VIP is equivalent to guarding the site where the VIP is supposed to
      // reach safety. On occasion can a bot decide to ambush on a particular spot of the map,
      // but this is highly unreliable because there are always several paths from the VIP start
      // point to its goal. Our bot already knows where the safety zone is (v_goal), so first
      // off it needs to get there, and then ambush nearby.

      // is bot NOT finding a path yet NOR walking it NOR already camping around the site ?
      if ((pBotBrain->bot_task != BOT_TASK_FINDPATH)
          && (pBotBrain->bot_task != BOT_TASK_WALKPATH)
          && (pBotBrain->bot_task != BOT_TASK_CAMP))
      {
         // is the bot just arrived at the site to protect ?
         if ((pPlayer->Bot.v_goal - pPlayer->v_origin).Length () < 200)
            pBotBrain->bot_task = BOT_TASK_CAMP; // have the bot camp around here

         // grmbl, this lazy bot is doing nothing.
         else
            pBotBrain->bot_task = BOT_TASK_FINDPATH; // bot must reach the site to protect
      }
   }

   // is our bot's goal to rescue a hostage ?
   if (pBotBrain->bot_goal & BOT_GOAL_RESCUEHOSTAGE)
   {
      // TODO: hostage stuff here
   }

   return;
}


void BotExecuteTask (player_t *pPlayer)
{
   // this function makes the bot execute a precise, concrete action, as part of the greater plan
   // defined by the bot's goal. Tasks are atomic sub-goals and not two tasks can be executed at
   // the same time.

   bot_eyes_t *pBotEyes;
   bot_ears_t *pBotEars;
   bot_legs_t *pBotLegs;
   bot_chat_t *pBotChat;
   bot_brain_t *pBotBrain;
   Vector bot_angles;
   noise_t *bomb_sound;
   edict_t *bomb_entity;
   navnode_t *current_navnode;

   pBotEyes = &pPlayer->Bot.BotEyes; // quick access to bot eyes
   pBotEars = &pPlayer->Bot.BotEars; // quick access to bot ears
   pBotLegs = &pPlayer->Bot.BotLegs; // quick access to bot legs
   pBotChat = &pPlayer->Bot.BotChat; // quick access to bot chat
   pBotBrain = &pPlayer->Bot.BotBrain; // quick access to bot brain

   // if debug level is high and we're spectating this bot, draw a beam to its goal point
   if (pPlayer->is_watched && (DebugLevel.cognition > 0))
      UTIL_DrawLine (pPlayer->v_origin, pPlayer->Bot.v_goal, 1, 0, 0, 255);

   // is the bot supposed to find a path ?
   if (pBotBrain->bot_task == BOT_TASK_FINDPATH)
   {
      // attempt to find a path, keep trying until we're in the right mindset for it
      if (BotFindPathTo (pPlayer, pPlayer->Bot.v_goal, TRUE))
         pBotBrain->bot_task = BOT_TASK_WALKPATH; // walk the path as soon as we're thinking about it
      else
         BotWander (pPlayer); // start wandering around in the meantime

      return;
   }

   // else is the bot supposed to walk a path ?
   else if (pBotBrain->bot_task == BOT_TASK_WALKPATH)
   {
      // has the bot finished thinking about the path ?
      if (pBotBrain->PathMachine.finished)
      {
         // has the pathmachine NOT finished its job yet ?
         if (pBotBrain->PathMachine.busy)
            BotWander (pPlayer); // start wandering around in the meantime
         else if (pBotBrain->PathMachine.path_count > 0)
            BotWalkPath (pPlayer); // as soon as the bot found a path, start walking it
         else
            pBotBrain->bot_task = BOT_TASK_FINDPATH; // damnit, no path found, try again ASAP
      }

      return;
   }

   // else is the bot supposed to pick up a bomb on the ground ?
   else if (pBotBrain->bot_task == BOT_TASK_PICKBOMB)
   {
      // is the bomb ALREADY in sight ?
      if (!FNullEnt (pPlayer->Bot.pInteractiveEntity)
          && (strncmp ("models/backpack", STRING (pPlayer->Bot.pInteractiveEntity->v.model), 15) == 0))
      {
         if (pPlayer->is_watched && (DebugLevel.cognition > 0))
         {
            AIConsole_printf (CHANNEL_COGNITION, 2, "SEEING BOMB! (dist %.1f)\n",
                              (OriginOf (pPlayer->Bot.pInteractiveEntity) - pPlayer->v_origin).Length ());

            if (DebugLevel.cognition > 1)
               UTIL_DrawLine (pPlayer->v_origin, pPlayer->Bot.pInteractiveEntity->v.origin, 2, 0, 0, 255);
         }

         // assign the bot the bomb's origin as a reach point
         pPlayer->Bot.v_reach_point = OriginOf (pPlayer->Bot.pInteractiveEntity);

         // let's run there...
         BotMoveTowardsReachPoint (pPlayer); // walk the path, Neo.
         BotAvoidObstacles (pPlayer); // avoid any obstacles while we're at it
         BotAvoidTeammates (pPlayer); // avoid teammates, too

         return;
      }

      // else we haven't seen the bomb yet, but we are searching
      else
      {
         if (pPlayer->is_watched && (DebugLevel.cognition > 0))
            AIConsole_printf (CHANNEL_COGNITION, 2, "SEARCHING FOR BOMB!\n");

         // check if bot SEES the bomb
         bomb_entity = BotDoesSee (pPlayer, "models/backpack");

         // does the bot see it ?
         if (!FNullEnt (bomb_entity))
         {
            pPlayer->Bot.pInteractiveEntity = bomb_entity;
            return; // bot sees the bomb, keep track of it and return
         }

         pBotLegs->forward_time = server.time + 0.2; // keep moving

         // if the bot knows the ground where it is AND has finished observing...
         if ((pPlayer->pFaceAtFeet != NULL)
             && (fabs (pPlayer->v_angle.x - pPlayer->Bot.BotHand.ideal_angles.x) < 2)
             && (fabs (pPlayer->v_angle.y - pPlayer->Bot.BotHand.ideal_angles.y) < 2))
         {
            // get a quick access to the current navnode and have the bot look at one link
            current_navnode = &pBotBrain->PathMemory[WalkfaceIndexOf (pPlayer->pFaceAtFeet)];
            BotLookAt (pPlayer, current_navnode->links[RandomLong (0, current_navnode->links_count)].v_origin);
         }

         return;
      }
   }

   // else is the bot supposed to plant a bomb ?
   else if (pBotBrain->bot_task == BOT_TASK_PLANTBOMB)
   {
      // does the bomb icon NOT blink anymore OR has the bot a new enemy ?
      if ((pBotEyes->BotHUD.icons_state[HUD_ICON_BOMB] != HUD_ICON_BLINKING)
          || !FNullEnt (pPlayer->Bot.BotEnemy.pEdict))
         pBotBrain->bot_task = BOT_TASK_FINDPATH; // then forget about planting

      // is the bot's current weapon NOT the C4 ?
      if (!PlayerHoldsWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_PLACE))
      {
         printf ("selecting bomb\n");
         BotSelectWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_PLACE); // take the C4 in hand
      }
      else
      {
         pPlayer->Bot.pause_time = server.time + 0.5; // pause the bot

         // make the bot look somewhat down as it's planting the bomb
         BotSetIdealAngles (pPlayer, WrapAngles (Vector (-45 / pPlayer->Bot.pProfile->skill, pPlayer->v_angle.y, 0)));

         if (pPlayer->Bot.pProfile->skill > 1)
            pBotLegs->duck_time = server.time + 0.5; // if bot is skilled enough, duck

         pPlayer->Bot.BotHand.fire1_time = server.time + 0.2; // press key to plant the bomb
      }

      return;
   }

   // else is the bot supposed to defuse a bomb ?
   else if (pBotBrain->bot_task == BOT_TASK_DEFUSEBOMB)
   {
      // is the bomb ALREADY in sight ?
      if (!FNullEnt (pPlayer->Bot.pInteractiveEntity)
          && (strncmp ("models/w_c4", STRING (pPlayer->Bot.pInteractiveEntity->v.model), 11) == 0))
      {
         if (pPlayer->is_watched && (DebugLevel.cognition > 0))
         {
            AIConsole_printf (CHANNEL_COGNITION, 2, "SEEING BOMB! (dist %.1f)\n",
                              (OriginOf (pPlayer->Bot.pInteractiveEntity) - pPlayer->v_origin).Length ());

            if (DebugLevel.cognition > 1)
               UTIL_DrawLine (pPlayer->v_origin, pPlayer->Bot.pInteractiveEntity->v.origin, 2, 0, 0, 255);
         }

         // assign the bot the bomb's origin as a reach point
         pPlayer->Bot.v_reach_point = OriginOf (pPlayer->Bot.pInteractiveEntity);

         // if bot is close to the bomb, defuse it...
         if ((pPlayer->Bot.v_reach_point - pPlayer->v_origin).Length () < 50)
         {
            pBotBrain->bot_task = BOT_TASK_USECHARGER; // the bot needs to USE the bomb to defuse it
            FakeClientCommand (pPlayer->pEntity, RADIOMSG_NEEDBACKUP); // bot speaks, "i need backup!"
         }

         // else if getting close, either duck or slow down
         else if ((pPlayer->Bot.v_reach_point - pPlayer->v_origin).Length () < 100)
         {
            BotLookAt (pPlayer, pPlayer->Bot.v_reach_point); // look at bomb
            pBotLegs->forward_time = server.time + 0.5; // go ahead...
            BotAvoidObstacles (pPlayer); // avoid any obstacles while we're at it
            BotAvoidTeammates (pPlayer); // avoid teammates, too

            // is the bomb lower than the bot ?
            if (pPlayer->Bot.v_reach_point.z < pPlayer->v_origin.z)
               pBotLegs->duck_time = server.time + 0.5; // if so, duck above the bomb
            else
               pBotLegs->walk_time = server.time + 0.2; // else walk as the bot comes closer
         }

         // else let's run there...
         else
         {
            BotMoveTowardsReachPoint (pPlayer); // walk the path, Neo.
            BotAvoidObstacles (pPlayer); // avoid any obstacles while we're at it
            BotAvoidTeammates (pPlayer); // avoid teammates, too
         }

         return;
      }

      // else is the bomb ALREADY being heard ?
      else if ((pPlayer->Bot.LastSuspiciousSound.file_path != NULL)
               && (strncmp ("weapons/c4_beep", pPlayer->Bot.LastSuspiciousSound.file_path, 15) == 0))
      {
         if (pPlayer->is_watched && (DebugLevel.cognition > 0))
            AIConsole_printf (CHANNEL_COGNITION, 2, "HEARING BOMB!\n");

         // check if bot SEES the bomb
         bomb_entity = BotDoesSee (pPlayer, "models/w_c4");

         // does the bot see it ?
         if (!FNullEnt (bomb_entity))
         {
            pPlayer->Bot.pInteractiveEntity = bomb_entity;
            return; // bot sees the bomb, keep track of it and return
         }

         // else check where this sound comes from...
         if (pPlayer->Bot.LastSuspiciousSound.direction & DIRECTION_FRONT)
            pBotLegs->forward_time = server.time + 0.2; // check in front of the bot
         if (pPlayer->Bot.LastSuspiciousSound.direction & DIRECTION_LEFT)
            pBotLegs->strafeleft_time = server.time + 0.2; // check on the left
         if (pPlayer->Bot.LastSuspiciousSound.direction & DIRECTION_RIGHT)
            pBotLegs->straferight_time = server.time + 0.2; // check on the right
         if (pPlayer->Bot.LastSuspiciousSound.direction & DIRECTION_BACK)
            pBotLegs->backwards_time = server.time + 0.2; // check behind the bot

         // if the bot knows the ground where it is AND has finished observing...
         if ((pPlayer->pFaceAtFeet != NULL)
             && (fabs (pPlayer->v_angle.x - pPlayer->Bot.BotHand.ideal_angles.x) < 2)
             && (fabs (pPlayer->v_angle.y - pPlayer->Bot.BotHand.ideal_angles.y) < 2))
         {
            // get a quick access to the current navnode and have the bot look at one link
            current_navnode = &pBotBrain->PathMemory[WalkfaceIndexOf (pPlayer->pFaceAtFeet)];
            BotLookAt (pPlayer, current_navnode->links[RandomLong (0, current_navnode->links_count)].v_origin);
         }

         return;
      }

      // else we haven't seen NOR heard the bomb yet, but we are searching
      else
      {
         if (pPlayer->is_watched && (DebugLevel.cognition > 0))
            AIConsole_printf (CHANNEL_COGNITION, 2, "LISTENING THE WORLD!\n");

         // check if bot SEES the bomb
         bomb_entity = BotDoesSee (pPlayer, "models/w_c4");

         // does the bot see it ?
         if (!FNullEnt (bomb_entity))
         {
            pPlayer->Bot.pInteractiveEntity = bomb_entity;
            return; // bot sees the bomb, keep track of it and return
         }

         // now check if bot HEARS the bomb
         bomb_sound = BotDoesHear (pPlayer, "weapons/c4_beep");

         // does the bot hear it ?
         if (bomb_sound != NULL)
         {
            memcpy (&pPlayer->Bot.LastSuspiciousSound, bomb_sound, sizeof (*bomb_sound));
            return; // bot hears the bomb, keep track of it and return
         }

         // has the bot listened for enough time long ?
         if (pBotBrain->listen_time < server.time)
         {
            // it's the wrong site !!!
            FakeClientCommand (pPlayer->pEntity, RADIOMSG_SECTORCLEAR); // send "sector clear" radio message
            pBotChat->bot_sayaudio = BOT_SAYAUDIO_REPORTINGIN; // bot reports in

            // FIXME: tell the bots to MARK this bombsite as bad not to choose it later

            pBotBrain->bot_goal = BOT_GOAL_NONE; // have the bot re-evaluate his goal
            pBotBrain->bot_task = BOT_TASK_IDLE;
         }

         // the last bomb tick may have bipped just one second ago and the next tick is
         // not sounding yet, so we need to make the bot listen for a few seconds...

         return; // so just continue hearing
      }
   }

   // else is the bot supposed to use something that "charges" ?
   else if (pBotBrain->bot_task == BOT_TASK_USECHARGER)
   {
      // does the bot hear an error sound ?
      if (BotDoesHear (pPlayer, "items/suitchargeno1") != NULL)
      {
         pBotBrain->bot_task = BOT_TASK_IDLE; // stop charging when bot hears error sound
         return;
      }

      // the bot is NOT hearing an error sound, let's face the thing to charge...
      bot_angles = VecToAngles (OriginOf (pPlayer->Bot.pInteractiveEntity) - pPlayer->v_eyeposition);
      BotSetIdealAngles (pPlayer, Vector (bot_angles.x / 2 + bot_angles.x / (2 * pPlayer->Bot.pProfile->skill), bot_angles.y, bot_angles.z)); // look at bomb

      // is the charger under the bot ?
      if (OriginOf (pPlayer->Bot.pInteractiveEntity).z < pPlayer->v_origin.z)
         pPlayer->Bot.BotLegs.duck_time = server.time + 0.5; // if so, let the bot duck

      // now keep pressing the USE button as long as the bot hears a charging sound, or the bot
      // sees a progress bar on the screen ; if not, randomly press it to trigger the charge
      if (pPlayer->Bot.BotEyes.BotHUD.has_progress_bar
          || (BotDoesHear (pPlayer, "items/suitcharge1") != NULL)
          || (RandomLong (1, 100) < 95))
         pPlayer->Bot.BotHand.use_time = server.time + 0.2; // keep pressing the button once the progress bar appeared

      return;
   }

   // else is the bot supposed to camp ?
   else if (pBotBrain->bot_task == BOT_TASK_CAMP)
   {
      pPlayer->Bot.v_place_to_keep = pPlayer->v_origin;
      BotCamp (pPlayer);

      return;
   }

   // else is the bot supposed to avoid the bomb explosion ?
   else if (pBotBrain->bot_task == BOT_TASK_AVOIDEXPLOSION)
   {

      return;
   }

   return;
}
