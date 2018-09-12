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
// bot_navigation.cpp
//

// FIXME: most of the stuff in here is PRETTY OLD and comes straight from my first waypointless
// attempts. Actually the new bot navigation is not written yet, I need a working pathfinder.

#include "racc.h"


void BotSetIdealAngles (bot_t *pBot, Vector ideal_angles)
{
   // this function sets the angles at which the bot wants to look. You don't have to
   // worry about changing the view angles of the bot directly, the bot turns to the
   // right direction by itself in a manner depending of the situation (e.g combat aim,
   // wandering aim...) with the BotPointGun() function. You can call this one as many
   // times you want each frame, as it is just about setting IDEAL angles. Note the use
   // of WrapAngle() to keep the angles in bounds and prevent overflows.

   // this function is part of the fakeclient aim bug fix. We need to revert the x component.

   pBot->BotAim.v_ideal_angles.x = -WrapAngle (ideal_angles.x); // st00pid engine bug !
   pBot->BotAim.v_ideal_angles.y = WrapAngle (ideal_angles.y);
   pBot->BotAim.v_ideal_angles.z = WrapAngle (ideal_angles.z);
   return; // done
}


void BotSetIdealPitch (bot_t *pBot, float ideal_pitch)
{
   // this function is part of the fakeclient aim bug fix. We need to revert the x component.

   pBot->BotAim.v_ideal_angles.x = -WrapAngle (ideal_pitch); // st00pid engine bug !
   return; // done
}


void BotSetIdealYaw (bot_t *pBot, float ideal_yaw)
{
   // this function is part of the fakeclient aim bug fix.

   pBot->BotAim.v_ideal_angles.y = WrapAngle (ideal_yaw);
   return; // done
}


void BotAddIdealPitch (bot_t *pBot, float pitch_to_add)
{
   // this function is part of the fakeclient aim bug fix. We need to revert the x component.

   pBot->BotAim.v_ideal_angles.x = -WrapAngle (pBot->BotAim.v_ideal_angles.x + pitch_to_add);
   return; // done
}


void BotAddIdealYaw (bot_t *pBot, float yaw_to_add)
{
   // this function is part of the fakeclient aim bug fix.

   pBot->BotAim.v_ideal_angles.y = WrapAngle (pBot->BotAim.v_ideal_angles.y + yaw_to_add);
   return; // done
}


void BotPointGun (bot_t *pBot)
{
   // this function is called every frame for every bot. Its purpose is to make the bot
   // move its crosshair to the direction where it wants to look. There is some kind of
   // filtering for the view, to make it human-like.

   // OMG, .weirdest .engine .bug .ever here - Thanks to botman and all the people at botman's
   // forums for helping us together setting things straight once for all !!!

   float speed, turn_skill, da_deadly_math; // speed : 0.1 - 1
   Vector v_deviation;

   if (DebugLevel.hand_disabled)
      return; // return if we don't want the AI to act and use things

   // ensure BotAim plugin compatibility
   pBot->pEdict->v.idealpitch = pBot->BotAim.v_ideal_angles.x;
   pBot->pEdict->v.ideal_yaw = pBot->BotAim.v_ideal_angles.y;

   v_deviation = WrapAngles (pBot->BotAim.v_ideal_angles - pBot->pEdict->v.v_angle);
   turn_skill = pBot->pProfile->skill * 2;

   // if bot is aiming at something, aim fast, else take our time...
   if (!FNullEnt (pBot->BotEnemy.pEdict))
      speed = 0.7 + (turn_skill - 1) / 10; // fast aim
   else
      speed = 0.2 + (turn_skill - 1) / 20; // slow aim

   // thanks Tobias "Killaruna" Heimann and Johannes "@$3.1415rin" Lampel for this one
   da_deadly_math = exp (log (speed / 2) * server.msecval / 50);

   pBot->BotAim.v_turn_speed.y = (pBot->BotAim.v_turn_speed.y * da_deadly_math
                                  + speed * v_deviation.y * (1 - da_deadly_math))
                                 * server.msecval / 50;
   pBot->BotAim.v_turn_speed.x = (pBot->BotAim.v_turn_speed.x * da_deadly_math
                                  + speed * v_deviation.x * (1 - da_deadly_math))
                                 * server.msecval / 50;

   // influence of y movement on x axis and vice versa (less influence than x on y since it's
   // easier and more natural for the bot to "move its mouse" horizontally than vertically)
   pBot->BotAim.v_turn_speed.x += pBot->BotAim.v_turn_speed.y / (1.5 * (1 + turn_skill));
   pBot->BotAim.v_turn_speed.y += pBot->BotAim.v_turn_speed.x / (1 + turn_skill);

   // move the aim cursor
   if (DebugLevel.is_inhumanturns)
      pBot->pEdict->v.v_angle = pBot->BotAim.v_ideal_angles;
   else
      pBot->pEdict->v.v_angle = WrapAngles (pBot->pEdict->v.v_angle + pBot->BotAim.v_turn_speed); 

   // set the body angles to point the gun correctly
   pBot->pEdict->v.angles.x = -pBot->pEdict->v.v_angle.x / 3;
   pBot->pEdict->v.angles.y = pBot->pEdict->v.v_angle.y;
   pBot->pEdict->v.angles.z = 0;

   // if debug mode is enabled, draw a line where the bot is looking and where it wants to look
   if ((DebugLevel.legs > 0) && IsValidPlayer (pListenserverEntity) && ((pBot->pEdict->v.origin - pListenserverEntity->v.origin).Length () <= 100))
   {
      printf ("BOT %s v_angle(%.1f,%.1f) ideal(%.1f,%.1f) speed(%.2f,%.2f)\n", STRING (pBot->pEdict->v.netname),
              pBot->pEdict->v.v_angle.x, pBot->pEdict->v.v_angle.y,
              pBot->BotAim.v_ideal_angles.x, pBot->BotAim.v_ideal_angles.y,
              pBot->BotAim.v_turn_speed.x, pBot->BotAim.v_turn_speed.y);
      MAKE_VECTORS (pBot->BotAim.v_ideal_angles); // make base vectors for ideal angles
      UTIL_DrawLine (pListenserverEntity, pBot->BotAim.v_eyeposition,
                     pBot->BotAim.v_eyeposition + gpGlobals->v_forward * 150,
                     1, 255, 0, 0); // ideal
      UTIL_DrawLine (pListenserverEntity, pBot->BotAim.v_eyeposition,
                     pBot->BotAim.v_eyeposition + pBot->BotAim.v_forward * 150,
                     1, 255, 255, 255); // current
   }

   return;
}


void BotMove (bot_t *pBot)
{
   // the purpose of this function is to translate the data of the BotMove structure (timings
   // at which the bot has to perform some movement - jump in 2 seconds, move forward for 5
   // seconds, and so on) into the right input buttons to be passed to RunPlayerMove(), which is
   // the function that asks the engine to perform the movement of the fakeclient. It also sets
   // the correct values for move_speed and strafe_speed which are parameters of RunPlayerMove().

   TraceResult tr;

   if (DebugLevel.legs_disabled)
      return; // return if we don't want the AI to move

   if (pBot->is_controlled)
      return; // if bot is bewitched, it doesn't "steer itself"

   // is the bot paused ?
   if (pBot->f_pause_time > *server.time)
   {
      pBot->BotMove.f_move_speed = 0;
      pBot->BotMove.f_strafe_speed = 0;

      if (pBot->BotMove.f_duck_time > *server.time)
         pBot->pEdict->v.button |= IN_DUCK;

      return; // don't move the bot if it should be paused
   }

   // may the bot jump now ?
   if ((pBot->BotMove.f_jump_time < *server.time) && (pBot->BotMove.f_jump_time + 0.1 > *server.time))
      pBot->pEdict->v.button |= IN_JUMP; // jump

   // has the bot just jumped AND is bot skilled enough ?
   if ((pBot->BotMove.f_jump_time + 0.1 < *server.time) && (pBot->BotMove.f_jump_time + 0.2 > *server.time) && (pBot->pProfile->skill > 1))
      pBot->BotMove.f_duck_time = *server.time + 0.2; // duck while jumping

   // may the bot duck now ?
   if (pBot->BotMove.f_duck_time > *server.time)
      pBot->pEdict->v.button |= IN_DUCK; // duck

   // may the bot safely strafe left now ?
   if ((pBot->BotMove.f_strafeleft_time > *server.time) && !(pBot->BotBody.hit_state & OBSTACLE_LEFT_FALL))
   {
      pBot->pEdict->v.button |= IN_MOVELEFT;
      pBot->BotMove.f_strafe_speed = -pBot->BotMove.f_max_speed; // strafe left
   }

   // else may the bot safely strafe right now ?
   else if ((pBot->BotMove.f_straferight_time > *server.time) && !(pBot->BotBody.hit_state & OBSTACLE_RIGHT_FALL))
   {
      pBot->pEdict->v.button |= IN_MOVERIGHT;
      pBot->BotMove.f_strafe_speed = pBot->BotMove.f_max_speed; // strafe right
   }

   // may the bot move backwards now ?
   if ((pBot->BotMove.f_backwards_time > *server.time) || (pBot->BotMove.b_emergency_walkback))
   {
      pBot->pEdict->v.button |= IN_BACK;
      pBot->BotMove.f_move_speed = -pBot->BotMove.f_max_speed; // move backwards
   }

   // else may the bot move forward now ?
   else if (pBot->BotMove.f_forward_time > *server.time)
   {
      pBot->pEdict->v.button |= IN_FORWARD;
      pBot->BotMove.f_move_speed = pBot->BotMove.f_max_speed; // run forward
   }

   // may the bot walk now ?
   if (pBot->BotMove.f_walk_time > *server.time)
   {
      pBot->pEdict->v.button |= IN_RUN;
      pBot->BotMove.f_move_speed /= 2; // forward walk
      pBot->BotMove.f_strafe_speed /= 2; // side walk
   }

   return;
}


void BotOnLadder (bot_t *pBot)
{
   TraceResult tr;

   if (FNullEnt (pBot->pEdict))
      return; // reliability check

   // if the bot is skilled enough, it will duck on ladders...
   if (pBot->pProfile->skill > 3)
      pBot->BotMove.f_duck_time = *server.time + 0.2;

   // check if bot JUST got on the ladder...
   if (pBot->f_end_use_ladder_time + 1.0 < *server.time)
      pBot->f_start_use_ladder_time = *server.time;

   BotFindLadder (pBot); // square up the bot on the ladder...

   // if the bot somehow "lost" its ladder, then just give up
   if (FNullEnt (pBot->pBotLadder))
   {
      pBot->ladder_direction = LADDER_UNKNOWN; // reset ladder direction
      return; // and return
   }

   if ((pBot->v_reach_point - pBot->pEdict->v.origin).Length2D () < 60)
   {
      Vector bot_angles = UTIL_VecToAngles (pBot->v_reach_point - pBot->pEdict->v.origin);
      BotSetIdealYaw (pBot, bot_angles.y); // face the middle of the ladder...
   }

   // moves the bot up or down a ladder.  if the bot can't move
   // (i.e. get's stuck with someone else on ladder), the bot will
   // change directions and go the other way on the ladder.

   // is the bot currently going up ?
   if (pBot->ladder_direction == LADDER_UP)
   {
      BotSetIdealPitch (pBot, 80); // look upwards

      // has the bot climbed the whole ladder ?
      if (pBot->pEdict->v.origin.z > pBot->pBotLadder->v.absmax.z)
      {
         BotSetIdealPitch (pBot, 0); // look at flat again

         // is the bot stuck ?
         if (pBot->b_is_stuck)
         {
            pBot->BotMove.f_duck_time = *server.time + 0.2; // duck

            // is there a floor on the left ?
            UTIL_TraceLine (pBot->BotAim.v_eyeposition,
                            pBot->BotAim.v_eyeposition + Vector (0, 0, -100) - Vector (pBot->BotAim.v_right.x, pBot->BotAim.v_right.y, 0) * 100,
                            dont_ignore_monsters, pBot->pEdict, &tr);
            if ((tr.flFraction < 1.0) && (AngleOfVectors (tr.vecPlaneNormal, Vector (0, 0, 1)) < 45))
               pBot->BotMove.f_strafeleft_time = *server.time + 0.5; // strafe left to reach it

            // else is there a floor on the right ?
            UTIL_TraceLine (pBot->BotAim.v_eyeposition,
                            pBot->BotAim.v_eyeposition + Vector (0, 0, -100) + Vector (pBot->BotAim.v_right.x, pBot->BotAim.v_right.y, 0) * 100,
                            dont_ignore_monsters, pBot->pEdict, &tr);
            if ((tr.flFraction < 1.0) && (AngleOfVectors (tr.vecPlaneNormal, Vector (0, 0, 1)) < 45))
               pBot->BotMove.f_straferight_time = *server.time + 0.5; // strafe right to reach it
         }
      }

      // else check if the bot is stuck...
      else if (pBot->b_is_stuck)
      {
         BotSetIdealPitch (pBot, -80); // look downwards (change directions)
         pBot->ladder_direction = LADDER_DOWN;
      }
   }

   // else is the bot currently going down ?
   else if (pBot->ladder_direction == LADDER_DOWN)
   {
      BotSetIdealPitch (pBot, -80); // look downwards

      // has the bot climbed the whole ladder ?
      if (fabs (pBot->pEdict->v.absmin.z - pBot->pBotLadder->v.absmin.z) < 10)
      {
         BotSetIdealPitch (pBot, 0); // look at flat again

         // is the bot stuck ?
         if (pBot->b_is_stuck)
         {
            pBot->BotMove.f_duck_time = *server.time + 0.2; // duck

            // is there a floor on the left ?
            UTIL_TraceLine (pBot->BotAim.v_eyeposition,
                            pBot->BotAim.v_eyeposition + Vector (0, 0, -100) - Vector (pBot->BotAim.v_right.x, pBot->BotAim.v_right.y, 0) * 100,
                            dont_ignore_monsters, pBot->pEdict, &tr);
            if ((tr.flFraction < 1.0) && (AngleOfVectors (tr.vecPlaneNormal, Vector (0, 0, 1)) < 45))
               pBot->BotMove.f_strafeleft_time = *server.time + 0.5; // strafe left to reach it

            // else is there a floor on the right ?
            UTIL_TraceLine (pBot->BotAim.v_eyeposition,
                            pBot->BotAim.v_eyeposition + Vector (0, 0, -100) + Vector (pBot->BotAim.v_right.x, pBot->BotAim.v_right.y, 0) * 100,
                            dont_ignore_monsters, pBot->pEdict, &tr);
            if ((tr.flFraction < 1.0) && (AngleOfVectors (tr.vecPlaneNormal, Vector (0, 0, 1)) < 45))
               pBot->BotMove.f_straferight_time = *server.time + 0.5; // strafe right to reach it
         }
      }

      // else check if the bot is stuck...
      else if (pBot->b_is_stuck)
      {
         BotSetIdealPitch (pBot, 80); // look upwards (change directions)
         pBot->ladder_direction = LADDER_UP;
      }
   }

   // else the bot hasn't picked a direction yet, try going up...
   else
   {
      BotSetIdealPitch (pBot, 80); // look upwards
      pBot->ladder_direction = LADDER_UP;
   }

   pBot->f_end_use_ladder_time = *server.time;

   // check if bot has been on a ladder for more than 5 seconds...
   if ((pBot->f_start_use_ladder_time > 0.0) && (pBot->f_start_use_ladder_time + 5.0 < *server.time))
   {
      pBot->BotMove.f_jump_time = *server.time; // jump to unstuck from ladder...
      pBot->f_find_item_time = *server.time + 10.0; // don't look for items for 10 seconds
      pBot->f_start_use_ladder_time = 0.0;  // reset start ladder use time
   }
}


void BotFollowOnLadder (bot_t *pBot)
{
   Vector bot_angles;

   if (FNullEnt (pBot->pEdict) || FNullEnt (pBot->pBotUser) || FNullEnt (pBot->pBotLadder))
      return; // reliability check

   // check if bot JUST got on the ladder...
   if (pBot->f_end_use_ladder_time + 1.0 < *server.time)
      pBot->f_start_use_ladder_time = *server.time;

   bot_angles = UTIL_VecToAngles (VecBModelOrigin (pBot->pBotLadder) - pBot->pEdict->v.origin);
   BotSetIdealYaw (pBot, bot_angles.y); // face the middle of the ladder...

   // moves the bot up or down a ladder according to the direction chosen by bot's user
   if (pBot->pBotUser->v.origin.z > pBot->pEdict->v.origin.z)
   {
      BotSetIdealPitch (pBot, 80); // user goes upstairs, look upwards
      pBot->ladder_direction = LADDER_UP;
   }
   else
   {
      BotSetIdealPitch (pBot, -80); // user goes downstairs, look downwards
      pBot->ladder_direction = LADDER_DOWN;
   }

   pBot->f_end_use_ladder_time = *server.time;
}


void BotUnderWater (bot_t *pBot)
{
   // handle movements under water. right now, just try to keep from drowning by swimming up
   // towards the surface and look to see if there is a surface the bot can jump up onto to
   // get out of the water. bots DON'T like water!

   Vector v_src, v_forward;
   TraceResult tr;
   int contents;

   if (FNullEnt (pBot->pEdict))
      return; // reliability check

   BotSetIdealPitch (pBot, 60); // look upwards to swim up towards the surface

   // look from eye position straight forward (remember: the bot is looking
   // upwards at a 60 degree angle so TraceLine will go out and up...

   UTIL_TraceLine (pBot->BotAim.v_eyeposition,
                   pBot->BotAim.v_eyeposition + pBot->BotAim.v_forward * 90,
                   ignore_monsters, pBot->pEdict, &tr);

   // check if the trace didn't hit anything (i.e. nothing in the way)...
   if (tr.flFraction >= 1.0)
   {
      // find out what the contents is of the end of the trace...
      contents = POINT_CONTENTS (tr.vecEndPos);

      // check if the trace endpoint is in open space...
      if (contents == CONTENTS_EMPTY)
      {
         // ok so far, we are at the surface of the water, continue...

         v_src = tr.vecEndPos;
         v_forward = tr.vecEndPos;
         v_forward.z -= 90;

         // trace from the previous end point straight down...
         UTIL_TraceLine (v_src, v_forward, ignore_monsters, pBot->pEdict, &tr);

         // check if the trace hit something...
         if (tr.flFraction < 1.0)
         {
            contents = POINT_CONTENTS (tr.vecEndPos);

            // if contents isn't water then assume it's land, jump
            if (contents != CONTENTS_WATER)
               pBot->BotMove.f_jump_time = *server.time;
         }
      }
   }
}


void BotUseLift (bot_t *pBot)
{
   if (FNullEnt (pBot->pEdict))
      return; // reliability check

   // check if lift has started moving...
   if ((pBot->pEdict->v.velocity.z != 0) && ENT_IS_ON_FLOOR (pBot->pEdict) && !pBot->b_lift_moving)
   {
      pBot->b_lift_moving = TRUE;
      pBot->BotMove.f_forward_time = 0; // don't move while using elevator
   }

   // else check if lift has stopped moving OR bot has waited too long for the lift to move...
   else if (((pBot->pEdict->v.velocity.z == 0) && ENT_IS_ON_FLOOR (pBot->pEdict) && pBot->b_lift_moving)
            || ((pBot->f_interact_time + 2.0 < *server.time) && !pBot->b_lift_moving))
   {
      pBot->b_interact = FALSE; // clear use button flag
      pBot->f_reach_time = *server.time; // get a new reach point as soon as now
      pBot->BotMove.f_forward_time = *server.time + 60.0; // run forward
      pBot->BotMove.f_walk_time = 0.0;
   }
}


bool BotCanUseInteractives (bot_t *pBot)
{
   edict_t *pent = NULL, *pButton = NULL;
   TraceResult tr;

   if (FNullEnt (pBot->pEdict))
      return (FALSE); // reliability check

   // if bot has already found an interactive entity to toy with...
   if (pBot->v_interactive_entity != g_vecZero)
      return (TRUE); // return; bot has already something to do

   // if bot has just interacted with something in the past seconds...
   if (pBot->f_interact_time + 2.0 > *server.time)
      return (FALSE); // return; too early for bot to check again

   pBot->v_interactive_entity = g_vecZero; // free bot's memory about interactive entities

   // check for interactive entities that are nearby
   while ((pent = UTIL_FindEntityInSphere (pent, pBot->BotAim.v_eyeposition, 400)) != NULL)
   {
      // check if that entity is a door in his view cone
      if ((strcmp ("door_origin", STRING (pent->v.classname)) == 0) && IsInPlayerFOV (pBot->pEdict, pent->v.origin))
      {
         // trace a line from bot's waist to door origin entity...
         UTIL_TraceLine (pBot->pEdict->v.origin, pent->v.origin, dont_ignore_monsters, pBot->pEdict, &tr);

         // check if traced all the way up to the door, either it is closed OR open
         // AND bot has not used a door for at least 15 seconds
         if (((strncmp ("func_door", STRING (tr.pHit->v.classname), 9) == 0) || (tr.flFraction == 1.0))
             && (pBot->f_interact_time + 15.0 < *server.time))
         {
            bool found_button = FALSE;

            // check for buttons near this door
            while ((pButton = UTIL_FindEntityInSphere (pButton, pent->v.origin, 300)) != NULL)
            {
               // if this button seems to control a door
               if ((strcmp ("func_button", STRING (pButton->v.classname)) == 0)
                   && (strcmp ("Locked", STRING (pButton->v.targetname)) == 0))
                  found_button = TRUE; // remember it
            }

            // is there a button that controls this door nearby ?
            if (found_button)
               continue; // skip this door: bot has to press a button first

            pBot->v_interactive_entity = pent->v.origin; // save door location
         }
      }

      // else check if that entity is a button in his view cone
      else if ((strcmp ("func_button", STRING (pent->v.classname)) == 0) && IsInPlayerFOV (pBot->pEdict, VecBModelOrigin (pent)))
      {
         // trace a line from bot's waist to button origin entity...
         UTIL_TraceLine (pBot->pEdict->v.origin, VecBModelOrigin (pent), dont_ignore_monsters, pBot->pEdict, &tr);

         // check if traced all the way up to the button
         // AND bot has not used a button for at least 10 seconds
         if ((strcmp ("func_button", STRING (tr.pHit->v.classname)) == 0)
             && (pBot->f_interact_time + 10.0 < *server.time))
            pBot->v_interactive_entity = VecBModelOrigin (pent); // save button location
      }
   }

   // if at this point bot has remembered no entity in particular
   if (pBot->v_interactive_entity == g_vecZero)
      return (FALSE); // bot didn't found anything togglable on his way
   else
      return (TRUE); // seems like bot found something togglable on his way
}


void BotInteractWithWorld (bot_t *pBot)
{
   TraceResult tr;

   if (FNullEnt (pBot->pEdict))
      return; // reliability check

   pBot->b_interact = FALSE; // reset any interaction flag
   pBot->f_interact_time = *server.time; // save last interaction time

   pBot->BotMove.f_forward_time = *server.time + 60.0; // run forward to interactive entity

   // reliability check: has the bot goal been resetted ?
   if (pBot->v_interactive_entity == g_vecZero)
      return; // give up

   // see how far our bot is from its goal
   if ((pBot->v_interactive_entity - pBot->pEdict->v.origin).Length () > 100)
      pBot->BotMove.f_walk_time = 0.0; // if bot is rather far, run to position
   else
      pBot->BotMove.f_walk_time = *server.time + 0.2; // else slow down while getting close

   // trace a line from bot's eyes to interactive entity origin...
   UTIL_TraceLine (pBot->BotAim.v_eyeposition, pBot->v_interactive_entity, dont_ignore_monsters, pBot->pEdict, &tr);

   // is bot far enough from entity AND path to entity is blocked (entity is no more visible)
   // OR no more in field of fiew OR bot is close enough to assume entity has already been used ?
   if ((((pBot->v_interactive_entity - pBot->pEdict->v.origin).Length () > 100) && (tr.flFraction < 0.80))
       || !IsInPlayerFOV (pBot->pEdict, pBot->v_interactive_entity)
       || ((pBot->v_interactive_entity - pBot->pEdict->v.origin).Length () < 10))
   {
      pBot->v_interactive_entity = g_vecZero; // reset interactive entity
      return; // give up: interactive entity is no more visible
   }

   // is the bot about to fall ?
   if ((pBot->f_fallcheck_time < *server.time) && (pBot->BotBody.hit_state & OBSTACLE_FRONT_FALL))
   {
      pBot->v_interactive_entity = g_vecZero; // reset interactive entity
      BotTurnAtFall (pBot); // try to avoid falling
      return; // give up: bot can't reach interactive entity safely
   }

   // face the interactive entity
   BotSetIdealAngles (pBot, UTIL_VecToAngles (pBot->v_interactive_entity - GetGunPosition (pBot->pEdict)));

   // if bot should check for corners on his sides...
   if (pBot->f_turncorner_time < *server.time)
      BotCheckForCorners (pBot); // check for corners and turn there if needed

   // if bot is not moving fast enough AND bot is close to entity, bot may have hit its goal
   if ((pBot->pEdict->v.velocity.Length2D () < 10.0)
      && ((pBot->v_interactive_entity - pBot->pEdict->v.origin).Length () < 50))
   {
      pBot->pEdict->v.button |= IN_USE; // activate the entity in case it is needed
      pBot->b_interact = TRUE; // set interaction flag
      pBot->b_lift_moving = FALSE; // set this in case bot would stand on a lift
      pBot->v_interactive_entity = g_vecZero; // reset interactive entity
      pBot->BotMove.f_backwards_time = *server.time + RANDOM_FLOAT (0.1, 0.4); // step back
   }
}


void BotTurnAtFall (bot_t *pBot)
{
   Vector Normal;
   float Y, Y1, Y2, D1, D2, Z;

   if (FNullEnt (pBot->pEdict))
      return; // reliability check

   // Find the normal vector from the trace result.  The normal vector will
   // be a vector that is perpendicular to the surface from the TraceResult.
   // Don't revert Normal since the edge plane is seen by the 'other side'

   Normal = WrapAngles360 (UTIL_VecToAngles (pBot->BotBody.v_fall_plane_normal));

   // Since the bot keeps it's view angle in -180 < x < 180 degrees format,
   // and since TraceResults are 0 < x < 360, we convert the bot's view
   // angle (yaw) to the same format as TraceResult.

   Y = WrapAngle360 (pBot->pEdict->v.v_angle.y + 180);

   // Here we compare the bots view angle (Y) to the Normal - 90 degrees (Y1)
   // and the Normal + 90 degrees (Y2).  These two angles (Y1 & Y2) represent
   // angles that are parallel to the edge surface, but heading in opposite
   // directions.  We want the bot to choose the one that will require the
   // least amount of turning (saves time) and have the bot head off in that
   // direction.

   Y1 = WrapAngle360 (Normal.y - 90);
   Y2 = WrapAngle360 (Normal.y + 90);

   // D1 and D2 are the difference (in degrees) between the bot's current
   // angle and Y1 or Y2 (respectively).

   D1 = fabs (Y - Y1);
   D2 = fabs (Y - Y2);

   // If difference 1 (D1) is more than difference 2 (D2) then the bot will
   // have to turn LESS if it heads in direction Y1 otherwise, head in
   // direction Y2.  I know this seems backwards, but try some sample angles
   // out on some graph paper and go through these equations using a
   // calculator, you'll see what I mean.

   if (D1 > D2)
      Z = WrapAngle (Y1 - RANDOM_FLOAT (0.0, 10.0)); // avoid exact angle
   else
      Z = WrapAngle (Y2 + RANDOM_FLOAT (0.0, 10.0)); // avoid exact angle

   BotSetIdealYaw (pBot, Z); // set the direction to head off into...
   pBot->f_reach_time = *server.time + 1.0; // don't try to reach point for one sec
   pBot->f_find_item_time = *server.time + 1.0; // don't try to reach items for one sec
   pBot->f_fallcheck_time = *server.time + 2.0; // give bot time to turn
}


bool BotCantSeeForward (bot_t *pBot)
{
   // use a TraceLine to determine if bot is facing a wall

   TraceResult tr;

   if (FNullEnt (pBot->pEdict))
      return (FALSE); // reliability check

   // trace from the bot's eyes straight forward...
   UTIL_TraceLine (pBot->BotAim.v_eyeposition,
                   pBot->BotAim.v_eyeposition + pBot->BotAim.v_forward * 150,
                   ignore_monsters, pBot->pEdict, &tr);

   // check if the trace hit something...
   if (tr.flFraction < 1.0)
      return (TRUE); // bot can't see forward

   return (FALSE); // bot can see forward, return FALSE
}


bool BotCanJumpUp (bot_t *pBot)
{
   // this function returns TRUE if the bot can jump up (presumably over an obstacle) without
   // hitting something, FALSE otherwise. One TraceHull is involved.

   Vector v_origin, v_forward, v_up;
   TraceResult tr;

   if (FNullEnt (pBot->pEdict))
      return (FALSE); // reliability check

   // bot simply can't jump nowhere if it isn't moving...
   if (pBot->pEdict->v.velocity == g_vecZero)
      return (FALSE);

   // flatten current view angle vectors for TraceLine math...
   v_origin = pBot->pEdict->v.origin;
   v_forward = Vector (pBot->BotAim.v_forward.x, pBot->BotAim.v_forward.y, 0);
   v_up = Vector (0, 0, 1);

   // trace a hull forward at maximum jump height (64 - duck hull center height)...
   UTIL_TraceHull (v_origin + v_up * 28, v_origin + v_up * 28 + v_forward * 24, ignore_monsters, head_hull, pBot->pEdict, &tr);

   return (tr.flFraction == 1.0); // if trace hit nothing, bot can safely jump up
}


bool BotCanDuckUnder (bot_t *pBot)
{
   // this function returns TRUE if the bot can duck and move forward (presumably crouching under
   // some obstacle) without hitting something, FALSE otherwise. One TraceHull is involved.

   Vector v_origin, v_forward, v_up;
   TraceResult tr;

   if (FNullEnt (pBot->pEdict))
      return (FALSE); // reliability check

   // flatten current view angle vectors for TraceLine math...
   v_origin = Vector (pBot->pEdict->v.origin.x, pBot->pEdict->v.origin.y, pBot->pEdict->v.absmin.z);
   v_forward = Vector (pBot->BotAim.v_forward.x, pBot->BotAim.v_forward.y, 0);
   v_up = Vector (0, 0, 1);

   // trace a hull forward one unit above the ground...
   UTIL_TraceHull (v_origin + v_up, v_origin + v_up + v_forward * 24, ignore_monsters, head_hull, pBot->pEdict, &tr);

   return (tr.flFraction == 1.0); // if trace hit nothing, bot can safely crouch here
}


void BotRandomTurn (bot_t *pBot)
{
   float distance = 0, prev_distance = 0, prev_prev_distance = 0, interesting_angles[72];
   int index, angles_count = 0;
   float angle;
   edict_t *pPrevHit;
   TraceResult tr;

   if (FNullEnt (pBot->pEdict))
      return; // reliability check

   angle = WrapAngle (pBot->pEdict->v.v_angle.y); // initialize scan angle to bot's view angle
   pPrevHit = pBot->pEdict; // initialize previous hit entity to the bot himself

   // scan 360 degrees around here in 72 samples...
   for (index = 0; index < 72; index++)
   {
      angle = WrapAngle (angle + 5); // pan the trace angle from left to right
      MAKE_VECTORS (Vector (0, angle, 0)); // build base vectors at flat in that direction

      // trace line at eyes level forward
      UTIL_TraceLine (pBot->BotAim.v_eyeposition,
                      pBot->BotAim.v_eyeposition + (gpGlobals->v_forward * 10000),
                      ignore_monsters, pBot->pEdict, &tr);

      if (prev_distance > 0)
         prev_prev_distance = prev_distance; // rotate the previous distances
      else
         prev_prev_distance = tr.flFraction * 10000; // handle start of scan
      if (distance > 0)
         prev_distance = distance; // rotate the previous distances
      else
         prev_distance = tr.flFraction * 10000; // handle start of scan
      distance = tr.flFraction * 10000; // store distance to obstacle

      // have we a discontinuousity in the distances (i.e an incoming path) or a door ?
      if ((((prev_distance > prev_prev_distance) && (prev_distance > distance)
            && ((prev_distance > prev_prev_distance + 100) || (prev_distance > distance + 100)))
           || (strncmp (STRING (pPrevHit->v.classname), "func_door", 9) == 0))
          && (prev_distance > 200))
      {
         interesting_angles[angles_count] = WrapAngle (angle - 5); // remember this angle
         angles_count++; // increment interesting angles count
      }

      pPrevHit = tr.pHit; // save previous hit entity
   }

   // okay, now we know which angles are candidates for determining a good watch angle
   if ((angles_count > 0) && (angles_count < 72))
   {
      angle = WrapAngle (interesting_angles[RANDOM_LONG (0, angles_count - 1)]); // choose one
      pBot->BotMove.f_forward_time = 0; // don't move while turning
      pBot->f_reach_time = *server.time + 1.0; // don't try to reach point for one second
   }

   // aargh, can't even figure out where to face !! fall back to totally randomness
   else
   {
      angle = RANDOM_FLOAT (-180, 180); // choose a random angle
      pBot->BotMove.f_forward_time = 0; // don't move while turning
      pBot->f_reach_time = *server.time + 1.0; // don't try to reach point for one second
   }

   BotSetIdealYaw (pBot, angle); // face the choosen angle
   return;
}


void BotFollowUser (bot_t *pBot)
{
   if (FNullEnt (pBot->pEdict) || FNullEnt (pBot->pBotUser))
      return; // reliability check

   // first check: is the user dead or not visible for more than 6 seconds ?
   if (!IsAlive (pBot->pBotUser)
       || (!BotCanSeeThis (pBot, pBot->pBotUser->v.origin)
           && (pBot->f_bot_use_time + 6 < *server.time)))
      pBot->BotChat.bot_saytext = BOT_SAYTEXT_CANTFOLLOW; // bot says can't follow the user

   // is the user really visible ?
   if (BotCanSeeOfEntity (pBot, pBot->pBotUser) != g_vecZero)
   {
      pBot->f_bot_use_time = *server.time; // reset last visible user time
      pBot->v_lastseenuser_position = pBot->pBotUser->v.origin; // remember last seen user position
   }

   // where is/was our user ?
   Vector v_user = pBot->v_lastseenuser_position - pBot->pEdict->v.origin;

   // is the bot far from the user OR the user is climbing up a ladder ?
   if ((v_user.Length () > 120) || IsFlying (pBot->pBotUser))
   {
      TraceResult tr;

      // is the user climbing up a ladder OR the bot is climbing up a ladder?
      if (IsFlying (pBot->pBotUser) || IsFlying (pBot->pEdict))
      {
         // is bot NOT on ladder yet ?
         if (!IsFlying (pBot->pEdict))
         {
            BotFindLadder (pBot); // find where this ladder is and reach it
            pBot->ladder_direction = LADDER_UNKNOWN;
         }

         // else the bot IS on the ladder
         else
         {
            // check if bot JUST got on the ladder...
            if ((pBot->f_end_use_ladder_time + 1.0) < *server.time)
               pBot->f_start_use_ladder_time = *server.time;

            // go handle the ladder movement
            BotFollowOnLadder (pBot);
         }

         pBot->BotMove.f_forward_time = *server.time + 60.0; // reach the ladder / climb up the ladder
         pBot->BotMove.f_walk_time = 0.0;
      }

      // else the user is NOT on a ladder, run to the position where our user was
      else if (pBot->f_reach_time < *server.time)
         BotReachPosition (pBot, pBot->v_lastseenuser_position);

      // is bot about to hit something it can jump up ?
      if ((pBot->BotBody.hit_state & OBSTACLE_FRONT_LOWWALL) && (pBot->BotMove.f_jump_time + 2.0 < *server.time))
         pBot->BotMove.f_jump_time = *server.time; // jump up and move forward

      // else is it about to hit something it can duck under ?
      else if (pBot->BotBody.hit_state & OBSTACLE_FRONT_LOWCEILING)
         pBot->BotMove.f_duck_time = *server.time + RANDOM_FLOAT (0.5, 1.5); // duck & go

      // it will soon be time to check around for possible enemies...
      pBot->f_randomturn_time = *server.time + 2.0;
   }
   else if (v_user.Length () > 50) // bot is in place
   {
      // if no enemy AND time to look around OR bot can't see forward
      if ((FNullEnt (pBot->BotEnemy.pEdict) && (pBot->f_randomturn_time < *server.time))
          || (BotCantSeeForward (pBot) && (pBot->f_randomturn_time + 0.2 < *server.time)))
      {
         BotRandomTurn (pBot); // randomly turnaround
         pBot->f_randomturn_time = *server.time + RANDOM_FLOAT (0.5, 15.0);
      }

      pBot->BotMove.f_forward_time = 0; // don't move if close enough
   }
   else if (FNullEnt (pBot->BotEnemy.pEdict))
   {
      // bot is too close to the user (don't check if bot has an enemy)
      BotSetIdealYaw (pBot, UTIL_VecToAngles (pBot->pBotUser->v.origin - pBot->pEdict->v.origin).y); // face the user
      pBot->BotMove.f_backwards_time = *server.time + 0.06; // make a step backwards
      pBot->f_randomturn_time = *server.time + RANDOM_FLOAT (0.5, 2.0);
   }
}


void BotFindLadder (bot_t *pBot)
{
   edict_t *pLadder;
   TraceResult tr;

   if (FNullEnt (pBot->pEdict))
      return; // reliability check

   pLadder = NULL; // first ensure the pointer at which to start the search is NULL

   // cycle through all ladders...
   while ((pLadder = UTIL_FindEntityByString (pLadder, "classname", "func_ladder")) != NULL)
   {
      // see if this "func_ladder" entity is within bot's search range
      if ((VecBModelOrigin (pLadder) - pBot->pEdict->v.origin).Length () < 500)
      {
         // force ladder origin to same z coordinate as bot since the VecBModelOrigin is the
         // center of the ladder. For long ladders, the center may be hundreds of units above
         // the bot. Fake an origin at the same level as the bot...
         Vector ladder_origin = VecBModelOrigin (pLadder);
         ladder_origin.z = pBot->pEdict->v.origin.z;

         // check if ladder is outside field of view
         if (!IsInPlayerFOV (pBot->pEdict, ladder_origin))
            continue; // skip this item if bot can't "see" it

         // trace a line from bot's eyes to func_ladder entity...
         UTIL_TraceLine (pBot->BotAim.v_eyeposition,
                         ladder_origin,
                         ignore_monsters, pBot->pEdict, &tr);

         // check if traced all the way up to the ladder (didn't hit wall)
         if (tr.flFraction == 1.0)
         {
            pBot->pBotLadder = pLadder; // save the ladder bot is trying to reach
            pBot->v_reach_point = ladder_origin; // tell the bot to reach this ladder
            pBot->b_is_picking_item = TRUE; // ladders priority is just the same as pickup items
            return;
         }
      }
   }

   return; // no ladder found
}


void BotStayInPosition (bot_t *pBot)
{
   bool b_can_see_position;
   TraceResult tr;

   if (FNullEnt (pBot->pEdict))
      return; // reliability check

   b_can_see_position = BotCanSeeThis (pBot, pBot->v_place_to_keep);

   // first check: is the place not visible for more than 3 seconds OR bot is camping and enemy in sight ?
   if ((!b_can_see_position && (pBot->f_place_time + 3 < *server.time))
       || ((pBot->f_camp_time > *server.time) && !FNullEnt (pBot->BotEnemy.pEdict)))
   {
      pBot->v_place_to_keep = g_vecZero; // forget this place
      pBot->f_camp_time = *server.time; // don't camp anymore as long as this enemy is alive
      return;
   }

   // else is the place visible ?
   else if (b_can_see_position)
      pBot->f_place_time = *server.time; // reset place time

   // how far is our place ?
   if ((pBot->v_place_to_keep - pBot->pEdict->v.origin).Length () > 50)
   {
      // if time to, run to the position where the bot should be
      if (pBot->f_reach_time < *server.time)
         BotReachPosition (pBot, pBot->v_place_to_keep);

      // is bot about to hit something it can jump up ?
      if ((pBot->BotBody.hit_state & OBSTACLE_FRONT_LOWWALL) && (pBot->BotMove.f_jump_time + 2.0 < *server.time))
         pBot->BotMove.f_jump_time = *server.time; // jump up and move forward

      // else is it about to hit something it can duck under ?
      else if (pBot->BotBody.hit_state & OBSTACLE_FRONT_LOWCEILING)
         pBot->BotMove.f_duck_time = *server.time + RANDOM_FLOAT (0.5, 1.5); // duck & go

      // it will soon be time to check around for possible enemies...
      pBot->f_randomturn_time = *server.time + 0.1;
   }
   else
   {
      pBot->BotMove.f_forward_time = 0; // don't move if close enough
      BotSetIdealPitch (pBot, 0); // look at flat

      // is bot camping ?
      if (pBot->f_camp_time > *server.time)
      {
         pBot->BotMove.f_duck_time = *server.time + 0.5; // duck if bot is camping
         pBot->f_avoid_time = *server.time + 0.5; // don't avoid walls for a while too
      }

      // else has the bot just stopped camping ?
      else if (pBot->f_camp_time + 0.5 > *server.time)
      {
         pBot->v_place_to_keep = g_vecZero; // free the slot
         pBot->f_camp_time = *server.time; // reset camping state
      }

      // if no enemy AND time to look around OR bot can't see forward
      if ((FNullEnt (pBot->BotEnemy.pEdict) && (pBot->f_randomturn_time < *server.time))
          || (BotCantSeeForward (pBot) && (pBot->f_randomturn_time + 0.2 < *server.time)))
      {
         BotRandomTurn (pBot); // randomly turnaround
         pBot->f_randomturn_time = *server.time + RANDOM_FLOAT (0.5, 15.0);
      }
   }
}


bool BotCheckForWall (bot_t *pBot, Vector v_direction)
{
   TraceResult tr;

   if (FNullEnt (pBot->pEdict))
      return (FALSE); // reliability check

   // check for a wall in v_direction
   UTIL_TraceLine (pBot->pEdict->v.origin,
                   pBot->pEdict->v.origin + v_direction,
                   ignore_monsters, pBot->pEdict, &tr);

   if (tr.flFraction < 1.0)
      return (TRUE); // if the trace hit something, then there is a wall in v_direction

   return (FALSE); // else trace hit nothing, there is no wall
}


void BotCheckForCorners (bot_t *pBot)
{
   TraceResult tr, tr2;
   float distance = 150;
   bool can_turn_left = FALSE, can_turn_right = FALSE, can_go_ahead = FALSE;

   if (FNullEnt (pBot->pEdict))
      return; // reliability check

   if (pBot->BotMove.f_move_speed == 0)
      return; // don't check for corners if the bot isn't in movement

   // make sure we are tracing inside the map...
   UTIL_TraceLine (pBot->pEdict->v.origin,
                   pBot->pEdict->v.origin + pBot->BotAim.v_forward * distance * 7,
                   ignore_monsters, pBot->pEdict, &tr);
   if (tr.flFraction == 1.0)
      can_go_ahead = TRUE; // there's still a long way ahead
   else if (tr.flFraction * 7 < 1.0)
      distance = distance * tr.flFraction * 7 * 0.99;

   // do a trace from 100 units in front of the bot's eyes left...
   UTIL_TraceLine (pBot->pEdict->v.origin + pBot->BotAim.v_forward * distance * 0.75,
                   pBot->pEdict->v.origin + pBot->BotAim.v_forward * distance * 0.75 - pBot->BotAim.v_right * 1000,
                   ignore_monsters, pBot->pEdict, &tr);

   // do a trace from 150 units in front of the bot's eyes left...
   UTIL_TraceLine (pBot->pEdict->v.origin + pBot->BotAim.v_forward * distance,
                   pBot->pEdict->v.origin + pBot->BotAim.v_forward * distance - pBot->BotAim.v_right * 1000,
                   ignore_monsters, pBot->pEdict, &tr2);

   // did the close trace hit something AND did the far trace hit something much more far ?
   if ((tr.flFraction < 1.0) && (tr2.flFraction > tr.flFraction * 2))
      can_turn_left = TRUE; // there's a corner on the left

   // do a trace from 100 units in front of the bot's eyes right...
   UTIL_TraceLine (pBot->pEdict->v.origin + pBot->BotAim.v_forward * distance * 0.75,
                   pBot->pEdict->v.origin + pBot->BotAim.v_forward * distance * 0.75 + pBot->BotAim.v_right * 1000,
                   ignore_monsters, pBot->pEdict, &tr);

   // do a trace from 150 units in front of the bot's eyes right...
   UTIL_TraceLine (pBot->pEdict->v.origin + pBot->BotAim.v_forward * distance,
                   pBot->pEdict->v.origin + pBot->BotAim.v_forward * distance + pBot->BotAim.v_right * 1000,
                   ignore_monsters, pBot->pEdict, &tr2);

   // did the close trace hit something AND did the far trace hit something much more far ?
   if ((tr.flFraction < 1.0) && (tr2.flFraction > tr.flFraction * 2))
      can_turn_right = TRUE; // there's a corner on the right

// ***VERY*** OLD STUFF, wonder why that's still here (dated from first waypointless attempts)
//   // does the bot have a goal ?
//   if (pBot->v_goal != g_vecZero)
//   {
//      // which side is our goal ?
//      Vector v_goal_angle = WrapAngles (UTIL_VecToAngles (pBot->v_goal - pBot->pEdict->v.origin) - pBot->pEdict->v.v_angle);
//
//      if ((fabs (v_goal_angle.x) < 45) && (v_goal_angle.y > 45))
//      {
//         can_go_ahead = FALSE; // bot doesn't want to go further this way
//         can_turn_right = FALSE; // bot won't turn right either
//      }
//      else  if ((fabs (v_goal_angle.x) < 45) && (v_goal_angle.y < -45))
//      {
//         can_go_ahead = FALSE; // bot doesn't want to go further this way
//         can_turn_left = FALSE; // bot won't turn left either
//      }
//      else if ((v_goal_angle.y > -45) && (v_goal_angle.y < 45))
//      {
//         can_turn_left = FALSE; // bot won't turn right
//         can_turn_right = FALSE; // bot won't turn left either
//      }
//   }

   // if bot can only go ahead...
   if (can_go_ahead && !can_turn_left && !can_turn_right)
      can_go_ahead = FALSE; // that's we're not near a corner, and choice is irrelevant

   // if bot can turn both sides...
   if (can_turn_left && can_turn_right)
   {
      if (RANDOM_LONG (1, 100) < 50)
         can_turn_right = FALSE; // bot decide to go LEFT
      else
         can_turn_left = FALSE; // bot decide to go RIGHT
   }

   // if bot can turn left AND go ahead...
   if (can_turn_left && can_go_ahead)
   {
      if (RANDOM_LONG (1, 100) < 50)
         can_go_ahead = FALSE; // bot decide to go LEFT
      else
         can_turn_left = FALSE; // bot decide to go AHEAD
   }

   // if bot can turn right AND go ahead...
   if (can_turn_right && can_go_ahead)
   {
      if (RANDOM_LONG (1, 100) < 50)
         can_go_ahead = FALSE; // bot decide to go RIGHT
      else
         can_turn_right = FALSE; // bot decide to go AHEAD
   }

   if (can_turn_left)
   {
      BotSetIdealYaw (pBot, pBot->BotAim.v_ideal_angles.y + RANDOM_LONG (60, 80)); // turn there
      BotSetIdealPitch (pBot, pBot->BotAim.v_ideal_angles.x - RANDOM_LONG (0, 5)); // look a bit down
      pBot->BotMove.f_straferight_time = *server.time + RANDOM_FLOAT (0.3, 0.8); // strafe
      pBot->f_reach_time = *server.time + 0.5; // don't try to reach point for 0.5 second
      pBot->f_turncorner_time = *server.time + 3.0; // don't check for corners for 3 secs
      return;
   }
   else if (can_turn_right)
   {
      BotSetIdealYaw (pBot, pBot->BotAim.v_ideal_angles.y - RANDOM_LONG (60, 80)); // turn there
      BotSetIdealPitch (pBot, pBot->BotAim.v_ideal_angles.x - RANDOM_LONG (0, 5)); // look a bit down
      pBot->BotMove.f_strafeleft_time = *server.time + RANDOM_FLOAT (0.3, 0.8); // strafe
      pBot->f_reach_time = *server.time + 0.5; // don't try to reach point for 0.5 second
      pBot->f_turncorner_time = *server.time + 3.0; // don't check for corners for 3 secs
      return;
   }
   else if (can_go_ahead)
   {
      pBot->f_turncorner_time = *server.time + 1.0; // don't check for corners for 1 sec
      return;
   }
   else
      pBot->f_turncorner_time = *server.time + 0.10; // found nothing, next check in 100 ms

   return;
}


void BotWander (bot_t *pBot)
{
   if (FNullEnt (pBot->pEdict))
      return; // reliability check

   pBot->BotMove.f_forward_time = *server.time + 60.0; // let our bot go...

   // if bot is using a button...
   if (pBot->b_interact)
      BotUseLift (pBot); // bot may be on a lift

   // else if bot is underwater...
   else if (pBot->pEdict->v.waterlevel == 3)
      BotUnderWater (pBot); // handle under water movement

   // else if bot is on a ladder...
   else if (IsFlying (pBot->pEdict))
      BotOnLadder (pBot); // handle ladder movement

   // else if the bot JUST got off the ladder...
   else if (pBot->f_end_use_ladder_time + 1.0 > *server.time)
      pBot->ladder_direction = LADDER_UNKNOWN;

   // else if some door to open or some button to press...
   else if (!pBot->b_is_picking_item && BotCanUseInteractives (pBot))
      BotInteractWithWorld (pBot);

   // else let's just wander around
   else
   {
      pBot->BotMove.f_walk_time = 0.0; // let our bot go...

      // if time to get a new reach point and bot is not picking an item...
      if ((pBot->f_getreachpoint_time < *server.time) && (pBot->f_reach_time < *server.time)
          && !pBot->b_is_picking_item)
         BotGetReachPoint (pBot); // get a new reach point

      // if still time to reach it...
      if (pBot->f_reach_time < *server.time)
         BotReachPosition (pBot, pBot->v_reach_point);

      // is bot about to hit something it can jump up ?
      if ((pBot->BotBody.hit_state & OBSTACLE_FRONT_LOWWALL) && (pBot->BotMove.f_jump_time + 2.0 < *server.time))
         pBot->BotMove.f_jump_time = *server.time; // jump up and move forward

      // else is it about to hit something it can duck under ?
      else if (pBot->BotBody.hit_state & OBSTACLE_FRONT_LOWCEILING)
         pBot->BotMove.f_duck_time = *server.time + RANDOM_FLOAT (0.5, 1.5); // duck & go

      // if bot is about to fall...
      if ((pBot->f_fallcheck_time < *server.time) && (pBot->BotBody.hit_state & OBSTACLE_FRONT_FALL))
         BotTurnAtFall (pBot); // try to avoid falling

      // if bot should check for corners on his sides and bot is not picking an item...
      if ((pBot->f_turncorner_time < *server.time) && !pBot->b_is_picking_item)
         BotCheckForCorners (pBot);

      // if the bot should pause for a while here (every so often, based on skill)...
      if ((RANDOM_LONG (1, 1000) <= (5 - pBot->pProfile->skill) * 2) && FNullEnt (pBot->pBotUser)
          && (pBot->f_buy_time + 20.0 < *server.time) && !pBot->b_is_picking_item
          && !BotCantSeeForward (pBot) && (pBot->f_rush_time < *server.time))
         pBot->f_pause_time = *server.time + RANDOM_FLOAT ((6 - pBot->pProfile->skill) / 2, 6 - pBot->pProfile->skill);
   }
}


char BotEstimateDirection (bot_t *pBot, Vector v_location)
{
   // this function returns the direction (defined as the DIRECTION_FRONT, DIRECTION_BACK,
   // DIRECTION_LEFT and DIRECTION_RIGHT) at which the vector location v_location is, relatively
   // to the bot itself, under the form of a bitmap (so that a location can be both backwards
   // and on the right, for example).

   float destination_angle;
   char direction;

   if (!IsValidPlayer (pBot->pEdict))
      return (DIRECTION_NONE); // reliability check

   direction = 0; // reset direction bitmap first

   // determine which side of the bot is v_location
   destination_angle = WrapAngle ((UTIL_VecToAngles (v_location - pBot->pEdict->v.origin) - pBot->pEdict->v.v_angle).y);

   // rotate clockwise around the bot and fill the direction bitmap
   if ((destination_angle >= -180) && (destination_angle < -112.5))
      direction |= DIRECTION_BACK; // location is in the bot's back
   if ((destination_angle >= -157.5) && (destination_angle < -22.5))
      direction |= DIRECTION_RIGHT; // location is on the right side of the bot
   if ((destination_angle >= -67.5) && (destination_angle < 67.5))
      direction |= DIRECTION_FRONT; // location is in front of the bot
   if ((destination_angle >= 22.5) && (destination_angle < 157.5))
      direction |= DIRECTION_LEFT; // location is on the left side of the bot
   if ((destination_angle >= 112.5) && (destination_angle < 180))
      direction |= DIRECTION_BACK; // location is in the bot's back

   return (direction); // and return the relative direction where bot thinks v_location is
}


void BotMoveTowardsPosition (bot_t *pBot, Vector v_position)
{
   // this function makes the bot pBot press the right keys in order to move towards the spatial
   // location described by the v_position vector. If a sufficiently close distance has been
   // met, the bot assumes it reached the requested position ; in such a case, the function
   // doesn't make the bot press any key anymore.

   char direction;

   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   if ((v_position - pBot->pEdict->v.origin).Length () < 100)
      return; // if the bot is really close to its position, assume it reached it

   // if debug mode is enabled, show the user where the bot wants to go
   if ((DebugLevel.legs > 0) && IsValidPlayer (pListenserverEntity) && ((pBot->pEdict->v.origin - pListenserverEntity->v.origin).Length () <= 100))
      UTIL_DrawLine (pListenserverEntity, pBot->pEdict->v.origin, v_position, 1, 0, 255, 0);

   // determine which side of the bot is the bot's objective
   direction = BotEstimateDirection (pBot, v_position);

   // given the angle, let the bot press the right keys (also allowing a combination of these)
   if (direction & DIRECTION_FRONT)
      pBot->BotMove.f_forward_time = *server.time + 0.2; // go forward to position
   if (direction & DIRECTION_BACK)
      pBot->BotMove.f_backwards_time = *server.time + 0.2; // go backwards to position
   if (direction & DIRECTION_LEFT)
      pBot->BotMove.f_strafeleft_time = *server.time + 0.2; // strafe left to position
   if (direction & DIRECTION_RIGHT)
      pBot->BotMove.f_straferight_time = *server.time + 0.2; // strafe right to position

   // if developer mode is high, print the angle to destination and the bot's keys
   if ((DebugLevel.legs > 1) && IsValidPlayer (pListenserverEntity) && ((pBot->pEdict->v.origin - pListenserverEntity->v.origin).Length () <= 100))
      printf ("(Bot %s keys: %s%s%s%s)\n",
              STRING (pBot->pEdict->v.netname),
              (pBot->BotMove.f_forward_time > *server.time ? "+FORWARD" : ""),
              (pBot->BotMove.f_backwards_time > *server.time ? "+BACK" : ""),
              (pBot->BotMove.f_strafeleft_time > *server.time ? "+STRAFE_L" : ""),
              (pBot->BotMove.f_straferight_time > *server.time ? "+STRAFE_R" : ""));

   // is bot about to hit something it can jump up ?
   if (((pBot->BotBody.hit_state & OBSTACLE_FRONT_LOWWALL) && (pBot->BotMove.f_forward_time > *server.time))
       || ((pBot->BotBody.hit_state & OBSTACLE_LEFT_LOWWALL) && (pBot->BotMove.f_strafeleft_time > *server.time))
       || ((pBot->BotBody.hit_state & OBSTACLE_RIGHT_LOWWALL) && (pBot->BotMove.f_straferight_time > *server.time)))
   {
      // has the bot not jumped for enough time long ?
      if (pBot->BotMove.f_jump_time + 1.2 < *server.time)
         pBot->BotMove.f_jump_time = *server.time + 0.3; // jump up and move forward
   }

   // else is bot about to hit something it can duck under ?
   else if (((pBot->BotBody.hit_state & OBSTACLE_FRONT_LOWCEILING) && (pBot->BotMove.f_forward_time > *server.time))
            || ((pBot->BotBody.hit_state & OBSTACLE_LEFT_LOWCEILING) && (pBot->BotMove.f_strafeleft_time > *server.time))
            || ((pBot->BotBody.hit_state & OBSTACLE_RIGHT_LOWCEILING) && (pBot->BotMove.f_straferight_time > *server.time)))
      pBot->BotMove.f_duck_time = *server.time + RANDOM_FLOAT (0.5, 1.5); // duck & go

   // make this bot correct the decided trajectory in order to avoid obstacles if needed
   BotAvoidObstacles (pBot);

   // if the bot is stuck...
   if (pBot->b_is_stuck)
      BotUnstuck (pBot); // try to unstuck our poor bot
}


bool BotReachPosition (bot_t *pBot, Vector v_position)
{
   TraceResult tr;

   if (!IsValidPlayer (pBot->pEdict))
      return (FALSE); // reliability check

   // do we mind if the position is not visible ?
   if (!BotCanSeeThis (pBot, v_position))
      return (FALSE); // if so, give up: bot might see it again in a few secs while wandering

   // look at destination and go forward to position
   BotSetIdealAngles (pBot, UTIL_VecToAngles (v_position - GetGunPosition (pBot->pEdict)));
   pBot->BotMove.f_forward_time = *server.time + 60.0;

   if ((v_position - pBot->pEdict->v.origin).Length () > 40)
      pBot->BotMove.f_walk_time = 0.0; // if still far, run
   else
      pBot->BotMove.f_walk_time = *server.time + 0.2; // else walk while getting closer

   // is the bot about to fall ? (TraceResult gets returned)
   if ((pBot->f_fallcheck_time < *server.time) && (pBot->BotBody.hit_state & OBSTACLE_FRONT_FALL))
   {
      // if bot is not following anyone
      if (pBot->f_bot_use_time + 5.0 < *server.time)
         BotTurnAtFall (pBot); // try to avoid falling
      else
         pBot->BotMove.f_jump_time = *server.time; // jump to follow user

      return (TRUE); // still on the way, despite the obstacles
   }

   // if debug mode is enabled, print out what the bot intends to do
   if ((DebugLevel.navigation > 0) && IsValidPlayer (pListenserverEntity) && ((pBot->pEdict->v.origin - pListenserverEntity->v.origin).Length () <= 100))
      printf ("Bot %s reaches position (remaining distance %f)\n", STRING (pBot->pEdict->v.netname), (v_position - pBot->pEdict->v.origin).Length ());

   return (TRUE);
}


void BotGetReachPoint (bot_t *pBot)
{
   int fov_index, max_index;
   float maxdistance = 0;
   Vector v_vecEndPos_dropped;

   if (FNullEnt (pBot->pEdict))
      return; // reliability check

   // cycle through the FOV data to get the longest distance
   for (fov_index = 0; fov_index < 52; fov_index++)
   {
      if (pBot->BotEyes.BotFOV[fov_index].distance > maxdistance)
      {
         maxdistance = pBot->BotEyes.BotFOV[fov_index].distance; // found new reach point
         max_index = fov_index; // remember the FOV index it was on
      }
   }

   // check to see if we can drop this new reach point at human height
   v_vecEndPos_dropped = DropAtHumanHeight (pBot->BotEyes.BotFOV[max_index].vecEndPos);
   if (FVisible (v_vecEndPos_dropped, pBot->pEdict))
      pBot->v_reach_point = v_vecEndPos_dropped; // place this reach point on the ground
   else
      pBot->v_reach_point = pBot->BotEyes.BotFOV[max_index].vecEndPos; // let it as is

   // ok so far, bot has a direction to head off into...

   // is it not acceptable (i.e. WAY too close already) ?
   if (maxdistance < 150)
   {
      // then the bot has probably reached a dead end
      if (RANDOM_LONG (1, 100) < 50)
         pBot->f_pause_time = *server.time + RANDOM_FLOAT (0.5, 6 - pBot->pProfile->skill); // pause

      BotRandomTurn (pBot); // pick up a new direction...
      pBot->f_getreachpoint_time = *server.time + 0.20; // ...in 200 ms
      return;
   }

   BotSetIdealAngles (pBot, UTIL_VecToAngles (pBot->v_reach_point - GetGunPosition (pBot->pEdict)));

   // if debug mode is enabled, draw a line where the bot wants to go
   if ((DebugLevel.navigation > 0) && IsValidPlayer (pListenserverEntity) && ((pBot->pEdict->v.origin - pListenserverEntity->v.origin).Length () <= 100))
      UTIL_DrawLine (pListenserverEntity, pBot->BotAim.v_eyeposition, pBot->v_reach_point, 1, 255, 255, 255);

   pBot->f_getreachpoint_time = *server.time + 0.20; // next reach point in 200 ms
   return;
}


void BotUnstuck (bot_t *pBot)
{
   TraceResult tr1, tr2, tr3;

   if (FNullEnt (pBot->pEdict))
      return; // reliability check

   // if debug mode is enabled, tell us that this bot is stuck
   if ((DebugLevel.navigation > 0) && IsValidPlayer (pListenserverEntity) && ((pBot->pEdict->v.origin - pListenserverEntity->v.origin).Length () <= 100))
      printf ("BOT %s IS STUCK!!!\n", STRING (pBot->pEdict->v.netname));

   pBot->f_reach_time = *server.time + 0.5; // don't reach point for half a second

   // check if bot can jump onto something and has not jumped for quite a time
   if ((pBot->BotMove.f_jump_time + 3.0 < *server.time) && BotCanJumpUp (pBot))
      pBot->BotMove.f_jump_time = *server.time; // jump up and move forward

   // else check if bot can duck under something
   else if (BotCanDuckUnder (pBot))
      pBot->BotMove.f_duck_time = *server.time + RANDOM_FLOAT (0.5, 1.5); // duck & move forward

   // let's see if the bot has reached a dead-end...
   UTIL_TraceLine (pBot->BotAim.v_eyeposition,
                   pBot->BotAim.v_eyeposition + pBot->BotAim.v_forward * 40,
                   ignore_monsters, dont_ignore_glass, pBot->pEdict, &tr1);
   UTIL_TraceLine (pBot->BotAim.v_eyeposition,
                   pBot->BotAim.v_eyeposition + pBot->BotAim.v_right * 60,
                   ignore_monsters, dont_ignore_glass, pBot->pEdict, &tr2);
   UTIL_TraceLine (pBot->BotAim.v_eyeposition,
                   pBot->BotAim.v_eyeposition - pBot->BotAim.v_right * 60,
                   ignore_monsters, dont_ignore_glass, pBot->pEdict, &tr3);

   // has the bot reached a dead-end ?
   if ((tr1.flFraction < 1.0) && (tr2.flFraction < 1.0) && (tr3.flFraction < 1.0))
   {
      if (RANDOM_LONG (1, 100) < 50)
         BotCanCampNearHere (pBot, pBot->pEdict->v.origin); // then just camp here on occasion
      else
         BotRandomTurn (pBot); // else pick a new direction
   }

   // can't figure out what to do, try to jump first...
   else if (pBot->BotMove.f_jump_time + 3.0 < *server.time)
      pBot->BotMove.f_jump_time = *server.time; // jump up and move forward

   // else duck
   else if (pBot->BotMove.f_duck_time + 3.0 < *server.time)
      pBot->BotMove.f_duck_time = *server.time + RANDOM_FLOAT (0.5, 1.0); // duck & move forward

   // else is the bot trying to get to an item?...
   else if (pBot->b_is_picking_item)
   {
      pBot->b_is_picking_item = FALSE; // give up trying to reach that item
      pBot->f_find_item_time = *server.time + 10.0; // don't look for items
   }

   // else our destination is REALLY unreachable, try to turnaround to unstuck
   else
      BotRandomTurn (pBot); // randomly turnaround

   pBot->b_is_stuck = FALSE; // consider bot should unstuck now
   return;
}


void BotAvoidObstacles (bot_t *pBot)
{
   // this function is mostly a reflex action ; its purpose is to make the bot react motionally
   // by strafing to any obstacle around, i.e players, walls, etc. First the bot cycles through
   // all players and determines if a player is visible and nearby, and if so, it performs the
   // necessary movement adjustments in order to avoid colliding into him. Then, it uses its
   // sensitive information in order to determine if it also needs to get further from a wall,
   // or whatever obstacle. If a wall or corner obstacle is found, the bot adjusts its movement
   // (strafes) in order to avoid it.

   TraceResult tr, tr2;
   int index;
   edict_t *pPlayer;

   if (FNullEnt (pBot->pEdict))
      return; // reliability check

   // search the world for players...
   for (index = 0; index < *server.max_clients; index++)
   {
      pPlayer = players[index].pEntity; // quick access to player

      if (FNullEnt (pPlayer) || pPlayer->free || (pPlayer == pBot->pEdict))
         continue; // skip invalid players and skip self (i.e. this bot)

      if (DebugLevel.is_observer && !(pPlayer->v.flags & FL_THIRDPARTYBOT))
         continue; // skip real players in observer mode

      if (GetTeam (pPlayer) != GetTeam (pBot->pEdict))
         continue; // don't mind about enemies...

      Vector vecEnd = pPlayer->v.origin + pPlayer->v.view_ofs;

      // see if bot can see the teammate...
      if (IsInPlayerFOV (pBot->pEdict, vecEnd) && BotCanSeeThis (pBot, vecEnd))
      {
         // bot found a visible teammate
         Vector v_teammate_angle = WrapAngles (UTIL_VecToAngles (vecEnd - pBot->pEdict->v.origin) - pBot->pEdict->v.v_angle);
         float f_teammate_distance = (pPlayer->v.origin - pBot->pEdict->v.origin).Length ();

         // is that teammate near us OR coming in front of us and within a certain distance ?
         if ((f_teammate_distance < 100)
             || ((f_teammate_distance < 300) && (v_teammate_angle.y < 15)
                 && (fabs (WrapAngle (pPlayer->v.v_angle.y - pBot->pEdict->v.v_angle.y)) > 165)))
         {
            // if we are moving full speed AND there's room forward OR teammate is very close...
            if (((pBot->pEdict->v.velocity.Length2D () > 10) && !(pBot->BotBody.hit_state & OBSTACLE_FRONT_WALL))
                || (f_teammate_distance < 70))
            {
               if (v_teammate_angle.y > 0)
               {
                  pBot->BotMove.f_strafeleft_time = 0;
                  pBot->BotMove.f_straferight_time = *server.time + 0.1; // strafe right to avoid him
               }
               else
               {
                  pBot->BotMove.f_strafeleft_time = *server.time + 0.1; // strafe left to avoid him
                  pBot->BotMove.f_straferight_time = 0;
               }

               pBot->f_reach_time = *server.time + 0.5; // delay reaching point
            }
         }
      }
   }

   // determine if bot need to strafe to avoid walls and corners

   // do a trace on the left side of the bot some steps forward
   UTIL_TraceLine (pBot->pEdict->v.origin - pBot->BotAim.v_right * 16,
                   pBot->pEdict->v.origin - pBot->BotAim.v_right * 16 + pBot->BotAim.v_forward * 40,
                   ignore_monsters, pBot->pEdict, &tr);

   // do a trace on the right side of the bot some steps forward
   UTIL_TraceLine (pBot->pEdict->v.origin + pBot->BotAim.v_right * 16,
                   pBot->pEdict->v.origin + pBot->BotAim.v_right * 16 + pBot->BotAim.v_forward * 40,
                   ignore_monsters, pBot->pEdict, &tr2);

   // did the right trace hit something further than the left trace ?
   if (tr.flFraction < tr2.flFraction)
   {
      pBot->BotMove.f_strafeleft_time = 0;
      pBot->BotMove.f_straferight_time = *server.time + 0.1; // there's an obstruction, strafe to avoid it
   }

   // else did the left trace hit something further than the right trace ?
   else if (tr.flFraction > tr2.flFraction)
   {
      pBot->BotMove.f_strafeleft_time = *server.time + 0.1; // there's an obstruction, strafe to avoid it
      pBot->BotMove.f_straferight_time = 0;
   }

   // else there is no immediate obstruction, check further...
   else
   {
      // make sure we trace inside the map (check on the left)
      UTIL_TraceLine (pBot->pEdict->v.origin,
                      pBot->pEdict->v.origin - pBot->BotAim.v_right * 30 + pBot->BotAim.v_forward * 30,
                      ignore_monsters, pBot->pEdict, &tr);

      // make sure we trace inside the map (check on the right)
      UTIL_TraceLine (pBot->pEdict->v.origin,
                      pBot->pEdict->v.origin + pBot->BotAim.v_right * 30 + pBot->BotAim.v_forward * 30,
                      ignore_monsters, pBot->pEdict, &tr2);

      // if there is a wall on the left
      if (tr.flFraction < 1.0)
      {
         pBot->BotMove.f_strafeleft_time = 0;
         pBot->BotMove.f_straferight_time = *server.time + 0.1; // strafe to avoid it
      }

      // else if there is a wall on the right
      else if (tr2.flFraction < 1.0)
      {
         pBot->BotMove.f_strafeleft_time = *server.time + 0.1; // strafe to avoid it
         pBot->BotMove.f_straferight_time = 0;
      }

      // else no side obstruction, check further...
      else
      {
         // do a trace from 30 units on the left of the bot to destination
         UTIL_TraceLine (pBot->pEdict->v.origin - pBot->BotAim.v_right * 30 + pBot->BotAim.v_forward * 30,
                         pBot->v_reach_point,
                         ignore_monsters, pBot->pEdict, &tr);

         // do a trace from 30 units on the right of the bot to destination
         UTIL_TraceLine (pBot->pEdict->v.origin + pBot->BotAim.v_right * 30 + pBot->BotAim.v_forward * 30,
                         pBot->v_reach_point,
                         ignore_monsters, pBot->pEdict, &tr2);

         // did the left trace hit something at a close range AND the right trace hit nothing ?
         if ((tr.flFraction < 0.3) && (tr2.flFraction == 1.0))
         {
            pBot->BotMove.f_strafeleft_time = 0;
            pBot->BotMove.f_straferight_time = *server.time + 0.1; // there's a corner, strafe to avoid it
         }

         // did the right trace hit something at a close range AND the left trace hit nothing ?
         if ((tr2.flFraction < 0.3) && (tr.flFraction == 1.0))
         {
            pBot->BotMove.f_strafeleft_time = *server.time + 0.1; // there's a corner, strafe to avoid it
            pBot->BotMove.f_straferight_time = 0;
         }
      }
   }

   pBot->f_avoid_time = *server.time + 0.2; // next check in 200 ms
   return;
}


bool BotCanSeeThis (bot_t *pBot, Vector v_destination)
{
   TraceResult tr, tr2;

   if (FNullEnt (pBot->pEdict))
      return (FALSE); // reliability check

   // don't look through water
   if ((POINT_CONTENTS (pBot->BotAim.v_eyeposition) == CONTENTS_WATER)
       != (POINT_CONTENTS (v_destination) == CONTENTS_WATER))
      return (FALSE);

   // look from bot's left and right eyes
   UTIL_TraceLine (pBot->BotAim.v_eyeposition - pBot->BotAim.v_right * 16,
                   v_destination, ignore_monsters, ignore_glass, pBot->pEdict, &tr);
   UTIL_TraceLine (pBot->BotAim.v_eyeposition + pBot->BotAim.v_right * 16,
                   v_destination, ignore_monsters, ignore_glass, pBot->pEdict, &tr2);

   if ((tr.flFraction == 1.0) && (tr2.flFraction == 1.0))
      return (TRUE); // line of sight is excellent

   else if ((tr.flFraction == 1.0) && (tr2.flFraction < 1.0))
      return (TRUE); // line of sight is valid, though bot might want to strafe left to see better

   else if ((tr.flFraction < 1.0) && (tr2.flFraction == 1.0))
      return (TRUE); // line of sight is valid, though bot might want to strafe right to see better

   return (FALSE); // line of sight is not established
}


bool BotCanCampNearHere (bot_t *pBot, Vector v_here)
{
   float distance = 0, prev_distance = 0, prev_prev_distance = 0, angle, interesting_angles[72];
   int index, angles_count = 0;
   Vector v_prev_hitpoint = v_here;
   TraceResult tr;
   edict_t *pPlayer;

   if (FNullEnt (pBot->pEdict))
      return (FALSE); // reliability check

   if (!ENT_IS_ON_FLOOR (pBot->pEdict))
      return (FALSE); // don't even think about it if bot is jumping

   angle = WrapAngle (pBot->pEdict->v.v_angle.y); // initialize scan angle to bot's view angle

   // cycle through all players to find if a teammate is already camping near here
   for (index = 0; index < *server.max_clients; index++)
   {
      pPlayer = players[index].pEntity; // quick access to player

      if (FNullEnt (pPlayer) || pPlayer->free || (pPlayer == pBot->pEdict))
         continue; // skip invalid players and skip self (i.e. this bot)

      if (!players[index].is_alive)
         continue; // skip this player if not alive (i.e. dead or dying)

      if (DebugLevel.is_observer && !(pPlayer->v.flags & FL_THIRDPARTYBOT))
         continue; // skip real players if in observer mode

      if (((pPlayer->v.button & IN_DUCK) == IN_DUCK) && ((pPlayer->v.origin - v_here).Length () < 1000))
         return (FALSE); // give up if another player is already camping near here
   }

   // scan 360 degrees around here in 72 samples...
   for (index = 0; index < 72; index++)
   {
      angle = WrapAngle (angle + 5); // pan the trace angle from left to right
      MAKE_VECTORS (Vector (0, angle, 0)); // build base vectors in that direction

      // trace line at waist level
      UTIL_TraceLine (v_here,
                      v_here + (gpGlobals->v_forward * 10000),
                      ignore_monsters, pBot->pEdict, &tr);

      // if debug mode is enabled, draw the field of view of this bot
      if ((DebugLevel.navigation > 0) && IsValidPlayer (pListenserverEntity) && ((pBot->pEdict->v.origin - pListenserverEntity->v.origin).Length () <= 100))
         UTIL_DrawLine (pListenserverEntity, v_here, v_here + (gpGlobals->v_forward * 10000), 1, 255, 0, 0);

      if (prev_distance > 0)
         prev_prev_distance = prev_distance; // rotate the previous distances
      else
         prev_prev_distance = tr.flFraction * 10000; // handle start of scan
      if (distance > 0)
         prev_distance = distance; // rotate the previous distances
      else
         prev_distance = tr.flFraction * 10000; // handle start of scan
      distance = tr.flFraction * 10000; // store distance to obstacle

      // have we a peak (meaning a safe corner) ?
      if ((prev_distance > prev_prev_distance) && (prev_distance > distance) && (prev_distance > 80)
          && BotCanSeeThis (pBot, v_prev_hitpoint) && IsAtHumanHeight (v_prev_hitpoint)
          && IsReachable (v_prev_hitpoint, pBot->pEdict))
      {
         interesting_angles[angles_count] = WrapAngle (angle - 5); // remember this angle
         angles_count++; // increment interesting angles count
      }

      v_prev_hitpoint = tr.vecEndPos; // rotate the remembered hit point
   }

   // okay, now we know which angles are candidates for determining a good camp point

   if ((angles_count <= 1) || (angles_count >= 72))
      return (FALSE); // give up if none found, bot can't camp near here

   angle = WrapAngle (interesting_angles[RANDOM_LONG (0, angles_count - 1)]); // choose one
   MAKE_VECTORS (Vector (0, angle, 0)); // build base vectors in that direction

   // trace line slightly under eyes level
   UTIL_TraceLine (v_here,
                   v_here + (gpGlobals->v_forward * 10000),
                   ignore_monsters, pBot->pEdict, &tr);

   // assign bot this camp point
   pBot->v_place_to_keep = (v_here + (pBot->pEdict->v.view_ofs / 2)) + (gpGlobals->v_forward * ((10000 * tr.flFraction) - 40));
   pBot->f_place_time = *server.time; // remember when we last saw the place to keep
   pBot->f_camp_time = *server.time + (6 - pBot->pProfile->skill) * RANDOM_FLOAT (10, 20); // make him remember he is camping
   pBot->f_reload_time = *server.time + RANDOM_LONG (1.5, 3.0); // switch to best weapon for the job

   return (TRUE); // bot found a camp spot next to v_here
}


void BotIsBewitched (bot_t *pBot)
{
   if (!IsValidPlayer (pBot->pEdict) || !IsValidPlayer (pListenserverEntity))
      return; // bot can't be bewitched if the listen server client is not around

   // duplicate the bot's controls on the player's one
   pBot->pEdict->v.button = pListenserverEntity->v.button;
   pBot->pEdict->v.impulse = pListenserverEntity->v.impulse;

   // forward - backwards
   if (pListenserverEntity->v.button & IN_FORWARD)
      pBot->BotMove.f_move_speed = pBot->BotMove.f_max_speed;
   else if (pListenserverEntity->v.button & IN_BACK)
      pBot->BotMove.f_move_speed = -pBot->BotMove.f_max_speed;

   // strafing left - right
   if (pListenserverEntity->v.button & IN_MOVERIGHT)
      pBot->BotMove.f_strafe_speed = pBot->BotMove.f_max_speed;
   else if (pListenserverEntity->v.button & IN_MOVELEFT)
      pBot->BotMove.f_strafe_speed = -pBot->BotMove.f_max_speed;

   // keypad turning
   if (pListenserverEntity->v.button & IN_RIGHT)
      BotSetIdealYaw (pBot, pBot->BotAim.v_ideal_angles.y + 1);
   else if (pListenserverEntity->v.button & IN_LEFT)
      BotSetIdealYaw (pBot, pBot->BotAim.v_ideal_angles.y - 1);

   // running - walking
   if (pListenserverEntity->v.button & IN_RUN)
   {
      pBot->BotMove.f_move_speed /= 2;
      pBot->BotMove.f_strafe_speed /= 2;
   }

   return; // then let the player steer this bot
}


void BotNavLoadBrain (bot_t *pBot)
{
   // this function sets up the navigation nodes in the bot's memory. Either by loading them
   // from disk, or by inferring brand new ones based on the map's contents. They will anyhow
   // be saved back to disk when the bot will leave the server. Note the use of the MFILE file
   // loading library that loads the file completely in memory and reads it from here instead
   // of reading it from the disk.

   MFILE *mfp;
   FILE *fp;
   char nav_filename[256];
   navnode_t *node;
   int recorded_walkfaces_count, face_index, link_index, array_index;
   char cookie[32];
   char section_name[32];
   char section_name_length;
   bool valid_brain = FALSE;
   bool found_section = FALSE;
   bool valid_section = FALSE;
   char *section_before, *section_after;
   int section_before_size, section_after_start, section_after_size;
   int default_likelevel = 1;

   if (FNullEnt (pBot->pEdict))
      return; // reliability check

   // build the file name
   sprintf (nav_filename, "racc/knowledge/%s/%s.nav", server.mod_name, NormalizeChars (STRING (pBot->pEdict->v.netname)));

   // get the section name
   sprintf (section_name, STRING (gpGlobals->mapname));
   section_name_length = strlen (section_name) + 1;

   // first make sure the brain space is empty
   if (pBot->BotBrain.PathMemory != NULL)
      free (pBot->BotBrain.PathMemory);

   // allocate enough memory for the navigation nodes
   pBot->BotBrain.PathMemory = (navnode_t *) malloc (map.walkfaces_count * sizeof (navnode_t));
   if (pBot->BotBrain.PathMemory == NULL)
      TerminateOnError ("BotNavLoadBrain(): Unable to allocate enough memory to infer a new nav brain to %s\n", STRING (pBot->pEdict->v.netname)); // bomb out on error

   // initialize the nav brain memory space to zero
   memset (pBot->BotBrain.PathMemory, 0, sizeof (pBot->BotBrain.PathMemory));

   // START OF BRAIN CHECKS

   // first prepare the brain ; i.e. check if the file exists, try to open it
   mfp = mfopen (nav_filename, "rb");
   if (mfp != NULL)
   {
      mfseek (mfp, 0, SEEK_SET); // seek at start of file
      mfread (cookie, sizeof ("RACCNAV"), 1, mfp); // read the brain signature

      if (strcmp (cookie, "RACCNAV") == 0)
         valid_brain = TRUE; // this brain file looks valid

      // does the section that corresponds to the current map exists ?
      if (mfseekAtSection (mfp, section_name) == 0)
      {
         found_section = TRUE; // this section exists, it has been found
         mfseek (mfp, sizeof ("[section]"), SEEK_CUR); // skip the [section] tag
         mfseek (mfp, section_name_length, SEEK_CUR); // skip the section name
         mfread (&recorded_walkfaces_count, sizeof (long), 1, mfp); // read the recorded # of walkfaces

         // check whether the brain file signature is valid AND the number of walkfaces matches
         if (recorded_walkfaces_count == map.walkfaces_count)
            valid_section = TRUE; // this section definitely looks valid, we can load it
      }

      mfclose (mfp); // close the brain (we read all the authentication info we need)
   }

   // END OF BRAIN CHECKS

   // START OF BRAIN SURGERY

   // if that brain was NOT explicitly reported as valid, we must fix it
   if (!valid_brain)
   {
      // there is a problem with the brain, infer a brand new one
      ServerConsole_printf ("RACC: bot %s's nav brain damaged!\n", STRING (pBot->pEdict->v.netname));

      if (server.developer_level > 1)
         ServerConsole_printf ("RACC: inferring a new nav brain to %s\n", STRING (pBot->pEdict->v.netname));

      // create the new brain (i.e, save a void one in the brain file)
      fp = fopen (nav_filename, "wb");
      if (fp == NULL)
         TerminateOnError ("BotNavLoadBrain(): Unable to operate on %s's nav brain !\n", STRING (pBot->pEdict->v.netname));

      fwrite ("RACCNAV", sizeof ("RACCNAV"), 1, fp); // write identification tag
      fwrite ("[likelevels]", sizeof ("[likelevels]"), 1, fp); // write likelevels section tag
      for (array_index = 0; array_index < 7; array_index++)
         fwrite (&default_likelevel, sizeof (long), 1, fp); // write 6 default likelevels
      fwrite ("[section]", sizeof ("[section]"), 1, fp); // write map section tag
      fwrite (section_name, section_name_length, 1, fp); // write map section name (map name)
      fwrite (&map.walkfaces_count, sizeof (long), 1, fp);
      for (face_index = 0; face_index < map.walkfaces_count; face_index++)
         fputc (0, fp); // assume each navnode has no link so far, so write zeroes everywhere
      fclose (fp); // everything is saved, close the file

      // and validate the newly fixed brain
      valid_brain = TRUE;
      found_section = TRUE;
      valid_section = TRUE;
   }

   // if that section was NOT found, we must add it
   if (!found_section)
   {
      // there is a problem with the brain, append a new section
      ServerConsole_printf ("RACC: bot %s is discovering a new map!\n", STRING (pBot->pEdict->v.netname));

      if (server.developer_level > 1)
         ServerConsole_printf ("RACC: adding new section to %s's nav brain\n", STRING (pBot->pEdict->v.netname));

      // open the brain for appending
      fp = fopen (nav_filename, "ab");
      if (fp == NULL)
         TerminateOnError ("BotNavLoadBrain(): Unable to operate on %s's nav brain !\n", STRING (pBot->pEdict->v.netname));

      fwrite ("[section]", sizeof ("[section]"), 1, fp); // section tag
      fwrite (section_name, section_name_length, 1, fp); // section name
      fwrite (&map.walkfaces_count, sizeof (long), 1, fp);
      for (face_index = 0; face_index < map.walkfaces_count; face_index++)
         fputc (0, fp); // assume each navnode has no link so far, so write zeroes everywhere
      fclose (fp); // everything is saved, close the file

      // and validate the newly fixed brain
      valid_brain = TRUE;
      found_section = TRUE;
      valid_section = TRUE;
   }

   // if that section was NOT explicitly reported as valid, we must fix it
   if (!valid_section)
   {
      // there is a problem with this map's section in the bot's brain, flush it
      ServerConsole_printf ("RACC: damaged section in bot %s's nav brain!\n", STRING (pBot->pEdict->v.netname));

      if (server.developer_level > 1)
         ServerConsole_printf ("RACC: flushing section in %s's nav brain\n", STRING (pBot->pEdict->v.netname));

      // open the brain for surgery
      mfp = mfopen (nav_filename, "rb");
      if (mfp == NULL)
         TerminateOnError ("BotNavLoadBrain(): Unable to operate on %s's nav brain !\n", STRING (pBot->pEdict->v.netname));

      // locate at start of the damaged section
      if (mfseekAtSection (mfp, section_name) != 0)
         TerminateOnError ("BotNavLoadBrain(): Unable to locate before damaged section in %s's nav brain !\n", STRING (pBot->pEdict->v.netname));

      section_before_size = mftell (mfp); // get the size of what's before the damaged section
      if (section_before_size == 0)
         TerminateOnError ("BotNavLoadBrain(): Unable to read before damaged section in %s's nav brain !\n", STRING (pBot->pEdict->v.netname));
      section_before = (char *) malloc (section_before_size); // allocate memory for what's before
      if (section_before == NULL)
         TerminateOnError ("BotNavLoadBrain(): malloc() failure for reading before damaged section in %s's nav brain !\n", STRING (pBot->pEdict->v.netname));
      mfseek (mfp, 0, SEEK_SET); // rewind at start of file
      mfread (section_before, section_before_size, 1, mfp); // and read what's before damaged section

      // now locate after the damaged section
      if (mfseekAfterSection (mfp, section_name) != 0)
      {
         if (section_before != NULL)
            free (section_before); // free the memory we mallocated() for what's before damaged section
         TerminateOnError ("BotNavLoadBrain(): Unable to locate after damaged section in %s's nav brain !\n", STRING (pBot->pEdict->v.netname));
      }
      section_after_start = mftell (mfp); // get the start of what's after the damaged section
      mfseek (mfp, 0, SEEK_END);
      section_after_size = mftell (mfp) - section_after_start; // get the size of it
      if (section_after_size > 0)
      {
         section_after = (char *) malloc (section_after_size); // allocate memory for what's after
         if (section_after == NULL)
         {
            if (section_before != NULL)
               free (section_before); // free the memory we mallocated() for what's before damaged section
            TerminateOnError ("BotNavLoadBrain(): malloc() failure for reading after damaged section in %s's nav brain !\n", STRING (pBot->pEdict->v.netname));
         }
         mfseek (mfp, section_after_start, SEEK_SET); // rewind at where what's after starts
         mfread (section_after, section_after_size, 1, mfp); // and read what's after damaged section
      }

      mfclose (mfp); // everything we wanted to know from the damaged brain is read, close the file

      // and finally fix the brain
      fp = fopen (nav_filename, "wb");
      if (fp == NULL)
      {
         if (section_before != NULL)
            free (section_before); // free the memory we mallocated() for what's before damaged section
         if (section_after != NULL)
            free (section_after); // free the memory we mallocated() for what's before damaged section
         TerminateOnError ("BotNavLoadBrain(): Unable to operate on %s's nav brain !\n", STRING (pBot->pEdict->v.netname));
      }

      fwrite (section_before, section_before_size, 1, fp); // write what's before
      fwrite ("[section]", sizeof ("[section]"), 1, fp); // section tag
      fwrite (section_name, section_name_length, 1, fp); // section name
      fwrite (&map.walkfaces_count, sizeof (long), 1, fp);
      for (face_index = 0; face_index < map.walkfaces_count; face_index++)
         fputc (0, fp); // assume each navnode has no link so far, so write zeroes everywhere
      if (section_after_size > 0)
         fwrite (section_after, section_after_size, 1, fp); // and write what's after, if needed
      fclose (fp); // everything is saved, close the file

      if (section_before != NULL)
         free (section_before); // free the memory we mallocated() for what's before damaged section
      if (section_after != NULL)
         free (section_after); // free the memory we mallocated() for what's before damaged section

      // and validate the newly fixed brain
      valid_brain = TRUE;
      found_section = TRUE;
      valid_section = TRUE;
   }

   // END OF BRAIN SURGERY

   // phew, voila! nav brain file is finally 100% certified valid...

   if (server.developer_level > 1)
      ServerConsole_printf ("RACC: restoring nav brain to %s\n", STRING (pBot->pEdict->v.netname));

   // now that we ensured about its validity, we can safely load the brain
   mfp = mfopen (nav_filename, "rb"); // open the brain file again
   mfseek (mfp, sizeof ("RACCNAV"), SEEK_CUR); // skip the "RACCNAV" tag
   mfseek (mfp, sizeof ("[likelevels]"), SEEK_CUR); // skip the "[likelevels]" tag

   // first read the global reachability likelevels
   mfread (&pBot->BotBrain.likelevel_ladder, sizeof (long), 1, mfp); // read ladder likelevel
   mfread (&pBot->BotBrain.likelevel_falledge, sizeof (long), 1, mfp); // read falledge likelevel
   mfread (&pBot->BotBrain.likelevel_elevator, sizeof (long), 1, mfp); // read elevator likelevel
   mfread (&pBot->BotBrain.likelevel_platform, sizeof (long), 1, mfp); // read platform likelevel
   mfread (&pBot->BotBrain.likelevel_conveyor, sizeof (long), 1, mfp); // read conveyor likelevel
   mfread (&pBot->BotBrain.likelevel_train, sizeof (long), 1, mfp); // read train likelevel
   mfread (&pBot->BotBrain.likelevel_longjump, sizeof (long), 1, mfp); // read longjump likelevel

   // now read the map section navmesh data
   mfseekAtSection (mfp, section_name); // seek at start of section
   mfseek (mfp, sizeof ("[section]"), SEEK_CUR); // skip the [section] tag
   mfseek (mfp, section_name_length, SEEK_CUR); // skip the section name
   mfread (&recorded_walkfaces_count, sizeof (long), 1, mfp); // load the walkfaces count

   // file is okay, cycle through all navigation nodes...
   for (face_index = 0; face_index < map.walkfaces_count; face_index++)
   {
      node = &pBot->BotBrain.PathMemory[face_index]; // quick access to node

      // link a pointer to the walkface it is for
      node->walkface = &map.walkfaces[face_index];

      // read the number of links this node has
      mfread (&node->links_count, sizeof (char), 1, mfp);

      // translate each face array index into a pointer
      for (link_index = 0; link_index < node->links_count; link_index++)
      {
         mfread (&array_index, sizeof (long), 1, mfp); // get the index in the walkface array it points to

         // test this index against overflow
         if ((array_index < 0) || (array_index >= map.walkfaces_count))
            TerminateOnError ("BotNavLoadBrain(): bad face array index %d (max %d), index %d/%d\n", array_index, map.walkfaces_count - 1, link_index, node->links_count);

         node->links[link_index].node_from = (navnode_t *) ((unsigned long) pBot->BotBrain.PathMemory + array_index * sizeof (navnode_t));

         // test this pointer against access violation (pointers are plain evil)
         if ((node->links[link_index].node_from < &pBot->BotBrain.PathMemory[0]) || (node->links[link_index].node_from > &pBot->BotBrain.PathMemory[map.walkfaces_count - 1]))
            TerminateOnError ("BotNavLoadBrain(): bad node pointer %d (range %d - %d), index %d/%d\n", node->links[link_index].node_from, &pBot->BotBrain.PathMemory[0], &pBot->BotBrain.PathMemory[map.walkfaces_count - 1], link_index, node->links_count);

         // read the reachability type for this link (normal, ladder, elevator...)
         mfread (&node->links[link_index].reachability, sizeof (char), 1, mfp);

         // read the vector origin for this link
         mfread (&node->links[link_index].v_origin, sizeof (Vector), 1, mfp);
      }
   }

   mfclose (mfp); // everything is loaded, close the file
   return; // no error, return FALSE
}


void BotNavSaveBrain (bot_t *pBot)
{
   // this function saves the navigation nodes in the bot's memory in a file to disk. The
   // authentication being made by comparison of the recorded number of walkfaces, this data
   // is written in the file at the very end of the function, to ensure an error in the save
   // process won't let a badly authenticated file on disk.

   MFILE *mfp;
   FILE *fp;
   char nav_filename[256];
   char section_name[32];
   char section_name_length;
   navnode_t *node;
   int face_index, link_index, array_index;
   char *section_before = NULL, *section_after = NULL;
   int section_before_size, section_after_start, section_after_size;

   if (FNullEnt (pBot->pEdict))
      return; // reliability check

   // build the file name
   sprintf (nav_filename, "racc/knowledge/%s/%s.nav", server.mod_name, NormalizeChars (STRING (pBot->pEdict->v.netname)));

   // get the section name
   sprintf (section_name, STRING (gpGlobals->mapname));
   section_name_length = strlen (section_name) + 1;

   // open the brain for updating (let's read what's before first)
   mfp = mfopen (nav_filename, "rb");
   if (mfp == NULL)
      TerminateOnError ("BotNavLoadBrain(): Unable to operate on %s's nav brain !\n", STRING (pBot->pEdict->v.netname));

   // locate at start of the section to update
   if (mfseekAtSection (mfp, section_name) != 0)
      TerminateOnError ("BotNavSaveBrain(): Unable to locate before section to update in %s's nav brain !\n", STRING (pBot->pEdict->v.netname));

   section_before_size = mftell (mfp); // get the size of what's before the section to update
   if (section_before_size == 0)
      TerminateOnError ("BotNavSaveBrain(): Unable to read before section to update in %s's nav brain !\n", STRING (pBot->pEdict->v.netname));
   section_before = (char *) malloc (section_before_size + 1); // allocate memory for what's before
   if (section_before == NULL)
      TerminateOnError ("BotNavSaveBrain(): malloc() failure for reading before section to update in %s's nav brain !\n", STRING (pBot->pEdict->v.netname));
   mfseek (mfp, 0, SEEK_SET); // rewind at start of file
   mfread (section_before, section_before_size, 1, mfp); // and read what's before section to update

   // now locate after the section to update
   if (mfseekAfterSection (mfp, section_name) != 0)
   {
      if (section_before != NULL)
         free (section_before); // free the memory we mallocated() for what's before section to update
      TerminateOnError ("BotNavSaveBrain(): Unable to locate after section to update in %s's nav brain !\n", STRING (pBot->pEdict->v.netname));
   }
   section_after_start = mftell (mfp); // get the start of what's after the section to update
   mfseek (mfp, 0, SEEK_END);
   section_after_size = mftell (mfp) - section_after_start; // get the size of it
   if (section_after_size > 0)
   {
      section_after = (char *) malloc (section_after_size + 1); // allocate memory for what's after
      if (section_after == NULL)
      {
         if (section_before != NULL)
            free (section_before); // free the memory we mallocated() for what's before section to update
         TerminateOnError ("BotNavSaveBrain(): malloc() failure for reading after section to update in %s's nav brain !\n", STRING (pBot->pEdict->v.netname));
      }
      mfseek (mfp, section_after_start, SEEK_SET); // rewind at where what's after starts
      mfread (section_after, section_after_size, 1, mfp); // and read what's after section to update
   }

   mfclose (mfp); // everything we wanted to know from the previous brain is read, close the file

   // and finally update the brain
   fp = fopen (nav_filename, "wb");
   if (fp == NULL)
   {
      if (section_before != NULL)
         free (section_before); // free the memory we mallocated() for what's before section to update
      if (section_after != NULL)
         free (section_after); // free the memory we mallocated() for what's before section to update
      TerminateOnError ("BotNavSaveBrain(): Unable to operate on %s's nav brain !\n", STRING (pBot->pEdict->v.netname));
   }

   fwrite (section_before, section_before_size, 1, fp); // write what's before
   fwrite ("[section]", sizeof ("[section]"), 1, fp); // section tag
   fwrite (section_name, section_name_length, 1, fp); // section name
   fwrite ("\0\0\0\0", sizeof (long), 1, fp); // fill the field with zeroes (temporarily)

   // for each navigation node...
   for (face_index = 0; face_index < map.walkfaces_count; face_index++)
   {
      node = &pBot->BotBrain.PathMemory[face_index]; // quick access to node

      // write the number of links this node has
      fwrite (&node->links_count, sizeof (char), 1, fp);

      // for each link of this node...
      for (link_index = 0; link_index < node->links_count; link_index++)
      {
         // translate the pointer address into an array relative index
         array_index = ((unsigned long) node->links[link_index].node_from - (unsigned long) pBot->BotBrain.PathMemory) / sizeof (navnode_t);
         if ((array_index < 0) || (array_index >= map.walkfaces_count))
            TerminateOnError ("BotNavSaveBrain(): bad node array index %d (max %d), index %d/%d\n", array_index, map.walkfaces_count - 1, link_index, node->links_count);
         fwrite (&array_index, sizeof (long), 1, fp); // write the walkface index of the link

         // write the reachability type for this link (normal, ladder, elevator...)
         fwrite (&node->links[link_index].reachability, sizeof (char), 1, fp);

         // read the vector origin for this link
         fwrite (&node->links[link_index].v_origin, sizeof (Vector), 1, fp);
      }
   }

   if (section_after_size > 0)
      fwrite (section_after, section_after_size, 1, fp); // and write what's after, if needed

   // now that the map specific data sections have been dumped, we can write the likelevels
   fseek (fp, 0, SEEK_SET); // rewind at start of file
   fseek (fp, sizeof ("RACCNAV"), SEEK_CUR); // skip the "RACCNAV" tag
   fseek (fp, sizeof ("[likelevels]"), SEEK_CUR); // skip the "[likelevels]" tag
   fwrite (&pBot->BotBrain.likelevel_ladder, sizeof (long), 1, fp); // write ladder likelevel
   fwrite (&pBot->BotBrain.likelevel_falledge, sizeof (long), 1, fp); // write falledge likelevel
   fwrite (&pBot->BotBrain.likelevel_elevator, sizeof (long), 1, fp); // write elevator likelevel
   fwrite (&pBot->BotBrain.likelevel_platform, sizeof (long), 1, fp); // write platform likelevel
   fwrite (&pBot->BotBrain.likelevel_conveyor, sizeof (long), 1, fp); // write conveyor likelevel
   fwrite (&pBot->BotBrain.likelevel_train, sizeof (long), 1, fp); // write train likelevel
   fwrite (&pBot->BotBrain.likelevel_longjump, sizeof (long), 1, fp); // write train likelevel

   // now we're ready to write the authentication tag
   fseek (fp, section_before_size, SEEK_SET); // seek back at start of section
   fseek (fp, sizeof ("[section]"), SEEK_CUR); // skip the [section] tag
   fseek (fp, section_name_length, SEEK_CUR); // skip the section name
   fwrite (&map.walkfaces_count, sizeof (long), 1, fp); // write the # of walkfaces on map

   fclose (fp); // everything is saved, close the file

   if (section_before != NULL)
      free (section_before); // free the memory we mallocated() for what's before section to update
   if (section_after != NULL)
      free (section_after); // free the memory we mallocated() for what's before section to update

   return; // and return
}
