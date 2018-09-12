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
// bot_navigation.cpp
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "bot_common.h"

extern edict_t *listenserver_edict;
extern float pause_frequency[5];
extern float pause_time[5][2];
extern bool b_observer_mode;
extern bool b_debug_nav;
extern bool b_doors_saved;


void BotMove (bot_t *pBot)
{
   TraceResult tr;

   if (pBot->pEdict == NULL)
      return; // reliability check

   // may the bot jump now ?
   if ((pBot->BotMove.f_jump_time < gpGlobals->time) && (pBot->BotMove.f_jump_time + 0.1 > gpGlobals->time))
      pBot->pEdict->v.button |= IN_JUMP; // jump

   // has the bot just jumped AND is bot skilled enough ?
   if ((pBot->BotMove.f_jump_time + 0.1 < gpGlobals->time) && (pBot->BotMove.f_jump_time + 0.2 > gpGlobals->time) && (pBot->bot_skill > 1))
      pBot->BotMove.f_duck_time = gpGlobals->time + 0.2; // duck while jumping

   // may the bot duck now ?
   if (pBot->BotMove.f_duck_time > gpGlobals->time)
      pBot->pEdict->v.button |= IN_DUCK; // duck

   // may the bot safely strafe left now ?
   if ((pBot->BotMove.f_strafeleft_time > gpGlobals->time) && !BotCanFallOnTheLeft (pBot))
      pBot->BotMove.f_strafe_speed = -pBot->BotMove.f_max_speed; // strafe left

   // else may the bot safely strafe right now ?
   else if ((pBot->BotMove.f_straferight_time > gpGlobals->time) && !BotCanFallOnTheRight (pBot))
      pBot->BotMove.f_strafe_speed = pBot->BotMove.f_max_speed; // strafe right

   // may the bot move backwards now ?
   if ((pBot->BotMove.f_backwards_time > gpGlobals->time) || (pBot->BotMove.b_emergency_walkback))
      pBot->BotMove.f_move_speed = -pBot->BotMove.f_max_speed; // move backwards

   // else may the bot run forward now ?
   else if ((pBot->BotMove.f_forward_time > gpGlobals->time) && !pBot->BotMove.b_is_walking)
      pBot->BotMove.f_move_speed = pBot->BotMove.f_max_speed; // run forward

   // else may the bot walk forward now ?
   else if ((pBot->BotMove.f_forward_time > gpGlobals->time) && pBot->BotMove.b_is_walking)
      pBot->BotMove.f_move_speed = pBot->BotMove.f_max_speed / 2; // walk forward
}


float BotChangePitch (bot_t *pBot, float speed)
{
   float ideal;
   float current;
   float diff;

   if (pBot->pEdict == NULL)
      return 0; // reliability check

   // check for wrap around of angles...
   pBot->pEdict->v.v_angle = UTIL_WrapAngles (pBot->pEdict->v.v_angle);
   pBot->pEdict->v.idealpitch = UTIL_WrapAngle (pBot->pEdict->v.idealpitch);

   // turn from the current v_angle pitch to the idealpitch by selecting
   // the quickest way to turn to face that direction

   current = pBot->pEdict->v.v_angle.x;
   ideal = pBot->pEdict->v.idealpitch;

   // find the difference in the current and ideal angle
   diff = abs (current - ideal);

   // check if the bot is already facing that direction...
   if (diff <= 1)
      return diff; // return number of degrees turned

   // check if difference is less than the max degrees per turn
   if (diff < speed)
      speed = diff; // just need to turn a little bit (less than max)

   // here we have four cases, both angle positive, one positive and
   // the other negative, one negative and the other positive, or
   // both negative.  handle each case separately...

   if ((current >= 0) && (ideal >= 0)) // both positive
   {
      if (current > ideal)
         current -= speed;
      else
         current += speed;
   }
   else if ((current >= 0) && (ideal < 0))
   {
      if (current - 180 > ideal)
         current += speed;
      else
         current -= speed;
   }
   else if ((current < 0) && (ideal >= 0))
   {
      if (current + 180 > ideal)
         current += speed;
      else
         current -= speed;
   }
   else // current and ideal both negative
   {
      if (current > ideal)
         current -= speed;
      else
         current += speed;
   }

   pBot->pEdict->v.v_angle.x = UTIL_WrapAngle (current);
   return speed; // return number of degrees turned
}


float BotChangeYaw (bot_t *pBot, float speed)
{
   float ideal;
   float current;
   float diff;

   if (pBot->pEdict == NULL)
      return 0; // reliability check

   // check for wrap around of angles...
   pBot->pEdict->v.v_angle = UTIL_WrapAngles (pBot->pEdict->v.v_angle);
   pBot->pEdict->v.ideal_yaw = UTIL_WrapAngle (pBot->pEdict->v.ideal_yaw);

   // turn from the current v_angle yaw to the ideal yaw by selecting
   // the quickest way to turn to face that direction

   current = pBot->pEdict->v.v_angle.y;
   ideal = pBot->pEdict->v.ideal_yaw;

   // find the difference in the current and ideal angle
   diff = abs (current - ideal);

   // check if the bot is already facing that direction...
   if (diff <= 1)
      return diff; // return number of degrees turned

   // check if difference is less than the max degrees per turn
   if (diff < speed)
      speed = diff; // just need to turn a little bit (less than max)

   // here we have four cases, both angle positive, one positive and
   // the other negative, one negative and the other positive, or
   // both negative.  handle each case separately...

   if ((current >= 0) && (ideal >= 0)) // both positive
   {
      if (current > ideal)
         current -= speed;
      else
         current += speed;
   }
   else if ((current >= 0) && (ideal < 0))
   {
      if (current - 180 > ideal)
         current += speed;
      else
         current -= speed;
   }
   else if ((current < 0) && (ideal >= 0))
   {
      if (current + 180 > ideal)
         current += speed;
      else
         current -= speed;
   }
   else // current and ideal both negative
   {
      if (current > ideal)
         current -= speed;
      else
         current += speed;
   }

   pBot->pEdict->v.v_angle.y = UTIL_WrapAngle (current);
   return speed; // return number of degrees turned
}


void BotOnLadder (bot_t *pBot)
{
   Vector v_src, v_dest, view_angles;
   TraceResult tr;
   float angle = 0.0;
   bool done = FALSE;

   if (pBot->pEdict == NULL)
      return; // reliability check

   // check if bot JUST got on the ladder...
   if (pBot->f_end_use_ladder_time + 1.0 < gpGlobals->time)
      pBot->f_start_use_ladder_time = gpGlobals->time;

   BotFindLadder (pBot); // square up the bot on the ladder...
   if ((pBot->v_reach_point - pBot->pEdict->v.origin).Length2D () < 60)
   {
      Vector bot_angles = UTIL_VecToAngles (pBot->v_reach_point - pBot->pEdict->v.origin);
      BotSetIdealYaw (pBot, bot_angles.y); // face the middle of the ladder...
   }

   // moves the bot up or down a ladder.  if the bot can't move
   // (i.e. get's stuck with someone else on ladder), the bot will
   // change directions and go the other way on the ladder.

   if (pBot->ladder_dir == LADDER_UP) // is the bot currently going up?
   {
      pBot->pEdict->v.v_angle.x = -80; // look upwards

      // check if the bot is stuck...
      if (BotIsStuck (pBot))
      {
         pBot->pEdict->v.v_angle.x = 80; // look downwards (change directions)
         pBot->ladder_dir = LADDER_DOWN;
      }
   }
   else if (pBot->ladder_dir == LADDER_DOWN) // is the bot currently going down?
   {
      pBot->pEdict->v.v_angle.x = 80; // look downwards

      // check if the bot is stuck...
      if (BotIsStuck (pBot))
      {
         pBot->pEdict->v.v_angle.x = -80; // look upwards (change directions)
         pBot->ladder_dir = LADDER_UP;
      }
   }
   else  // the bot hasn't picked a direction yet, try going up...
   {
      pBot->pEdict->v.v_angle.x = -80; // look upwards
      pBot->ladder_dir = LADDER_UP;
   }

   pBot->pEdict->v.button |= IN_FORWARD; // needed to climb ladders
   pBot->f_end_use_ladder_time = gpGlobals->time;

   // check if bot has been on a ladder for more than 5 seconds...
   if ((pBot->f_start_use_ladder_time > 0.0) && (pBot->f_start_use_ladder_time + 5.0 < gpGlobals->time))
   {
      pBot->BotMove.f_jump_time = gpGlobals->time; // jump to unstuck from ladder...
      pBot->f_find_item_time = gpGlobals->time + 10.0; // don't look for items for 10 seconds
      pBot->f_start_use_ladder_time = 0.0;  // reset start ladder use time
   }
}


void BotFollowOnLadder (bot_t *pBot)
{
   Vector v_src, v_dest, view_angles;
   TraceResult tr;
   float angle = 0.0;
   bool done = FALSE;

   if ((pBot->pEdict == NULL) || (pBot->pBotUser == NULL))
      return; // reliability check

   // check if bot JUST got on the ladder...
   if (pBot->f_end_use_ladder_time + 1.0 < gpGlobals->time)
      pBot->f_start_use_ladder_time = gpGlobals->time;

   BotFindLadder (pBot); // square up the bot on the ladder...
   if ((pBot->v_reach_point - pBot->pEdict->v.origin).Length2D () < 60)
   {
      Vector bot_angles = UTIL_VecToAngles (pBot->v_reach_point - pBot->pEdict->v.origin);
      BotSetIdealYaw (pBot, bot_angles.y); // face the middle of the ladder...
   }

   // moves the bot up or down a ladder according to the direction choosen by bot's user
   if (pBot->pBotUser->v.origin.z > pBot->pEdict->v.origin.z)
      pBot->pEdict->v.v_angle.x = -80; // user goes upstairs, look upwards
   else
      pBot->pEdict->v.v_angle.x = 80; // user goes downstairs, look downwards

   pBot->pEdict->v.button |= IN_FORWARD; // needed to climb ladders
   pBot->f_end_use_ladder_time = gpGlobals->time;
}


void BotUnderWater (bot_t *pBot)
{
   // handle movements under water. right now, just try to keep from
   // drowning by swimming up towards the surface and look to see if
   // there is a surface the bot can jump up onto to get out of the
   // water. bots DON'T like water!

   Vector v_src, v_forward;
   TraceResult tr;
   int contents;

   if (pBot->pEdict == NULL)
      return; // reliability check

   pBot->pEdict->v.button |= IN_FORWARD; // move forward (in the direction the bot is looking)
   pBot->pEdict->v.v_angle.x = -60; // look upwards to swim up towards the surface

   // look from eye position straight forward (remember: the bot is looking
   // upwards at a 60 degree angle so TraceLine will go out and up...

   UTIL_MakeVectors (pBot->pEdict->v.v_angle); // build base vectors
   UTIL_TraceLine (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs,
                   pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs + gpGlobals->v_forward * 90,
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // check if the trace didn't hit anything (i.e. nothing in the way)...
   if (tr.flFraction >= 1.0)
   {
      // find out what the contents is of the end of the trace...
      contents = UTIL_PointContents (tr.vecEndPos);

      // check if the trace endpoint is in open space...
      if (contents == CONTENTS_EMPTY)
      {
         // ok so far, we are at the surface of the water, continue...

         v_src = tr.vecEndPos;
         v_forward = tr.vecEndPos;
         v_forward.z -= 90;

         // trace from the previous end point straight down...
         UTIL_TraceLine (v_src, v_forward, ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

         // check if the trace hit something...
         if (tr.flFraction < 1.0)
         {
            contents = UTIL_PointContents (tr.vecEndPos);

            // if contents isn't water then assume it's land, jump
            if (contents != CONTENTS_WATER)
               pBot->BotMove.f_jump_time = gpGlobals->time;
         }
      }
   }
}


void BotUseLift (bot_t *pBot)
{
   if (pBot->pEdict == NULL)
      return; // reliability check

   // check if lift has started moving...
   if ((pBot->pEdict->v.velocity.z != 0) && ENT_IS_ON_FLOOR (pBot->pEdict) && !pBot->b_lift_moving)
   {
      pBot->b_lift_moving = TRUE;
      pBot->BotMove.f_forward_time = 0; // don't move while using elevator
   }

   // else check if lift has stopped moving OR bot has waited too long for the lift to move...
   else if (((pBot->pEdict->v.velocity.z == 0) && ENT_IS_ON_FLOOR (pBot->pEdict) && pBot->b_lift_moving)
            || ((pBot->f_interact_time + 2.0 < gpGlobals->time) && !pBot->b_lift_moving))
   {
      pBot->b_interact = FALSE; // clear use button flag
      pBot->f_reach_time = gpGlobals->time; // get a new reach point as soon as now
      pBot->BotMove.f_forward_time = gpGlobals->time + 60.0; // run forward
      pBot->BotMove.b_is_walking = FALSE;
   }
}


bool BotCanUseInteractives (bot_t *pBot)
{
   edict_t *pent = NULL, *pButton = NULL;
   TraceResult tr;

   if (pBot->pEdict == NULL)
      return FALSE; // reliability check

   // if not done yet...
   if (!b_doors_saved)
   {
      SaveDoorsOrigins (); // store doors origins for bots
      b_doors_saved = TRUE; // set doors saved flag
   }

   // if bot has already found an interactive entity to toy with...
   if (pBot->v_interactive_entity != Vector (0, 0, 0))
      return TRUE; // return; bot has already something to do

   // if bot has just interacted with something in the past seconds...
   if (pBot->f_interact_time + 2.0 > gpGlobals->time)
      return FALSE; // return; too early for bot to check again

   pBot->v_interactive_entity = Vector (0, 0, 0); // free bot's memory about interactive entities
   UTIL_MakeVectors (pBot->pEdict->v.v_angle); // build base vectors

   // check for interactive entities that are nearby
   while ((pent = UTIL_FindEntityInSphere (pent, pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs, 400)) != NULL)
   {
      // check if that entity is a door in his view cone
      if ((strcmp ("door_origin", STRING (pent->v.classname)) == 0) && FInViewCone (pent->v.origin, pBot->pEdict))
      {
         // trace a line from bot's waist to door origin entity...
         UTIL_TraceLine (pBot->pEdict->v.origin, pent->v.origin, dont_ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

         // check if traced all the way up to the door, either it is closed OR open
         // AND bot has not used a door for at least 15 seconds
         if (((strncmp ("func_door", STRING (tr.pHit->v.classname), 9) == 0) || (tr.flFraction == 1.0))
             && (pBot->f_interact_time + 15.0 < gpGlobals->time))
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
      else if ((strcmp ("func_button", STRING (pent->v.classname)) == 0) && FInViewCone (VecBModelOrigin (pent), pBot->pEdict))
      {
         // trace a line from bot's waist to button origin entity...
         UTIL_TraceLine (pBot->pEdict->v.origin, VecBModelOrigin (pent), dont_ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

         // check if traced all the way up to the button
         // AND bot has not used a button for at least 10 seconds
         if ((strcmp ("func_button", STRING (tr.pHit->v.classname)) == 0)
             && (pBot->f_interact_time + 10.0 < gpGlobals->time))
            pBot->v_interactive_entity = VecBModelOrigin (pent); // save button location
      }
   }

   // if at this point bot has remembered no entity in particular
   if (pBot->v_interactive_entity == Vector (0, 0, 0))
      return FALSE; // bot didn't found anything togglable on his way
   else
      return TRUE; // seems like bot found something togglable on his way
}


void BotInteractWithWorld (bot_t *pBot)
{
   TraceResult tr;

   if (pBot->pEdict == NULL)
      return; // reliability check

   pBot->b_interact = FALSE; // reset any interaction flag
   pBot->f_interact_time = gpGlobals->time; // save last interaction time

   pBot->BotMove.f_forward_time = gpGlobals->time + 60.0; // run forward to interactive entity

   // reliability check: has the bot goal been resetted ?
   if (pBot->v_interactive_entity == Vector (0, 0, 0))
      return; // give up

   // see how far our bot is from its goal
   if ((pBot->v_interactive_entity - pBot->pEdict->v.origin).Length () > 100)
      pBot->BotMove.b_is_walking = FALSE; // if bot is rather far, run to position
   else
      pBot->BotMove.b_is_walking = TRUE; // else slow down while getting close

   // trace a line from bot's eyes to interactive entity origin...
   UTIL_TraceLine (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs, pBot->v_interactive_entity, dont_ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // is bot far enough from entity AND path to entity is blocked (entity is no more visible)
   // OR no more in field of fiew OR bot is close enough to assume entity has already been used ?
   if ((((pBot->v_interactive_entity - pBot->pEdict->v.origin).Length () > 100) && (tr.flFraction < 0.80))
       || !FInViewCone (pBot->v_interactive_entity, pBot->pEdict)
       || ((pBot->v_interactive_entity - pBot->pEdict->v.origin).Length () < 10))
   {
      pBot->v_interactive_entity = Vector (0, 0, 0); // reset interactive entity
      return; // give up: interactive entity is no more visible
   }

   // is the bot about to fall ? (TraceResult gets returned)
   if (BotCanFallForward (pBot, &tr) && (pBot->f_fallcheck_time < gpGlobals->time))
   {
      pBot->v_interactive_entity = Vector (0, 0, 0); // reset interactive entity
      BotTurnAtFall (pBot, &tr); // try to avoid falling
      return; // give up: bot can't reach interactive entity safely
   }

   // face the interactive entity
   BotSetIdealYaw (pBot, UTIL_VecToAngles (pBot->v_interactive_entity - pBot->pEdict->v.origin).y);
   pBot->pEdict->v.angles.x = 0;
   pBot->pEdict->v.angles.y = UTIL_WrapAngle (pBot->pEdict->v.v_angle.y); // make body face eyes' same way 
   pBot->pEdict->v.angles.z = 0;

   // if bot should check for corners on his sides...
   if (pBot->f_turncorner_time < gpGlobals->time)
      BotCheckForCorners (pBot); // check for corners and turn there if needed

   // if bot is not moving fast enough AND bot is close to entity, bot may have hit its goal
   if ((pBot->pEdict->v.velocity.Length2D () < 10.0)
      && ((pBot->v_interactive_entity - pBot->pEdict->v.origin).Length () < 50))
   {
      pBot->pEdict->v.button |= IN_USE; // activate the entity in case it is needed
      pBot->b_interact = TRUE; // set interaction flag
      pBot->b_lift_moving = FALSE; // set this in case bot would stand on a lift
      pBot->v_interactive_entity = Vector (0, 0, 0); // reset interactive entity
      pBot->BotMove.f_backwards_time = gpGlobals->time + RANDOM_FLOAT (0.1, 0.4); // step back
   }

   // if the bot is stuck...
   if (BotIsStuck (pBot))
      BotUnstuck (pBot); // try to unstuck our poor bot
}


bool BotCantMoveForward (bot_t *pBot, TraceResult *tr)
{
   TraceResult tr2;
   bool bot_will_touch = FALSE;
   Vector v_zoffset;

   if (pBot->pEdict == NULL)
      return FALSE; // reliability check

   // is bot ducking or under water ?
   if ((pBot->BotMove.f_duck_time > gpGlobals->time) || (pBot->pEdict->v.waterlevel == 3))
      v_zoffset = Vector (0, 0, -1); // if so, offset one unit down from origin
   else
      v_zoffset = Vector (0, 0, -19); // else offset 19 units down as bot is standing

   UTIL_MakeVectors (pBot->pEdict->v.v_angle); // build base vectors

   // do a trace one unit lower than the max stair height forward...
   UTIL_TraceLine (pBot->pEdict->v.origin + v_zoffset,
                   pBot->pEdict->v.origin + v_zoffset + gpGlobals->v_forward * 90,
                   ignore_monsters, pBot->pEdict, tr);

   // do a trace 18 units higher than the previous trace forward...
   UTIL_TraceLine (pBot->pEdict->v.origin + v_zoffset + Vector (0, 0, 18),
                   pBot->pEdict->v.origin + v_zoffset + Vector (0, 0, 18) + gpGlobals->v_forward * 90,
                   ignore_monsters, pBot->pEdict, &tr2);

   // is there an obstacle in the way AND this obstacle is a wall, a steep slope or a low wall ?
   if ((tr->flFraction < 1.0)
       && ((tr->flFraction == tr2.flFraction)
           || ((tr->vecPlaneNormal.z > 0) && (tr->vecPlaneNormal.z < 0.5))
           || ((tr->vecPlaneNormal.z == 0) && (tr2.flFraction == 1.0))))
      bot_will_touch = TRUE;

   // now do a trace from the eyes position forward...
   UTIL_TraceLine (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs,
                   pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs + gpGlobals->v_forward * 40,
                   ignore_monsters, pBot->pEdict, tr);

   // is there something in the way ?
   if (tr->flFraction < 1.0)
      return TRUE; // bot will hit something
   else if (bot_will_touch)
   {
      if (!BotCanJumpUp (pBot))
         return TRUE; // bot will hit something
      else if (pBot->BotMove.f_jump_time + 0.5 < gpGlobals->time)
         pBot->BotMove.f_jump_time = gpGlobals->time; // bot can jump over this wall
   }

   return FALSE; // bot can move forward, return false
}


void BotTurnAtWall (bot_t *pBot, TraceResult *tr)
{
   Vector v_src, Normal;
   float Y, Y1, Y2, D1, D2, Z;
   bool cant_turn_on_left = FALSE, cant_turn_on_right = FALSE;

   if (pBot->pEdict == NULL)
      return; // reliability check

   // Find the normal vector from the trace result.  The normal vector will
   // be a vector that is perpendicular to the surface from the TraceResult.

   Normal = UTIL_EngineWrapAngles (UTIL_VecToAngles (tr->vecPlaneNormal));

   // Since the bot keeps it's view angle in -180 < x < 180 degrees format,
   // and since TraceResults are 0 < x < 360, we convert the bot's view
   // angle (yaw) to the same format as TraceResult.

   Y = UTIL_EngineWrapAngle (pBot->pEdict->v.v_angle.y + 180);

   // Turn the normal vector around 180 degrees (i.e. make it point towards
   // the wall not away from it.  That makes finding the angles that the
   // bot needs to turn a little easier.

   Normal.y = UTIL_EngineWrapAngle (Normal.y - 180);

   // Let's now check if bot would fall on either side of the wall, or if there
   // is another wall in that direction (if bot is stuck in a corner, for example)
   // Use some TraceLines to determine if bot is going to fall from an edge or
   // hit a wall.

   UTIL_MakeVectors (Normal);

   // check on the left

   // first check for a wall to be certain to trace inside the map
   if (!BotCheckForWall (pBot, -gpGlobals->v_right * 80))
   {
      // next trace left down to check for an effective fall
      UTIL_TraceLine (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs - gpGlobals->v_right * 80,
                      pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs - gpGlobals->v_right * 80 + Vector (0, 0, -300),
                      ignore_monsters, pBot->pEdict->v.pContainingEntity, tr);

      // did the trace hit nothing ?
      if (tr->flFraction == 1.0)
         cant_turn_on_left = TRUE; // there is a fall
   }
   else
      cant_turn_on_left = TRUE; // there is a wall

   // now check on the right

   // first check for a wall to be certain to trace inside the map
   if (!BotCheckForWall (pBot, gpGlobals->v_right * 80))
   {
      // next trace right down to check for an effective fall
      UTIL_TraceLine (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs + gpGlobals->v_right * 80,
                      pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs + gpGlobals->v_right * 80 + Vector (0, 0, -300),
                      ignore_monsters, pBot->pEdict->v.pContainingEntity, tr);

      // did the trace hit nothing ?
      if (tr->flFraction == 1.0)
         cant_turn_on_right = TRUE; // there is a fall
   }
   else
      cant_turn_on_right = TRUE; // there is a wall

   // Here we compare the bots view angle (Y) to the Normal - 91 degrees (Y1)
   // and the Normal + 91 degrees (Y2).  These two angles (Y1 & Y2) represent
   // angles that are parallel to the wall surface, but heading in opposite
   // directions.  We want the bot to choose the one that will require the
   // least amount of turning (saves time) and have the bot head off in that
   // direction.

   Y1 = UTIL_EngineWrapAngle (Normal.y - 90);
   Y2 = UTIL_EngineWrapAngle (Normal.y + 90);

   // D1 and D2 are the difference (in degrees) between the bot's current
   // angle and Y1 or Y2 (respectively).

   D1 = abs (Y - Y1);
   D2 = abs (Y - Y2);

   // If difference 1 (D1) is more than difference 2 (D2) then the bot will
   // have to turn LESS if it heads in direction Y1 otherwise, head in
   // direction Y2.  I know this seems backwards, but try some sample angles
   // out on some graph paper and go through these equations using a
   // calculator, you'll see what I mean.
   // If there is a fall on either side, choose the other side. Else turn back.

   if ((cant_turn_on_left) && (cant_turn_on_right))
      Z = UTIL_WrapAngle (Y + RANDOM_FLOAT (-10.0, 10.0)); // no way, bot MUST turn back
   else if (cant_turn_on_left)
      Z = UTIL_WrapAngle (Y1 - RANDOM_FLOAT (0.0, 10.0)); // bot MUST turn right
   else if (cant_turn_on_right)
      Z = UTIL_WrapAngle (Y2 + RANDOM_FLOAT (0.0, 10.0)); // bot MUST turn left
   else if (D1 > D2)
      Z = UTIL_WrapAngle (Y1 - RANDOM_FLOAT (0.0, 10.0)); // bot turns right
   else
      Z = UTIL_WrapAngle (Y2 + RANDOM_FLOAT (0.0, 10.0)); // bot turns left

   BotSetIdealYaw (pBot, Z); // set the direction to head off into...
   pBot->f_reach_time = gpGlobals->time + 0.5; // don't try to reach point for 0.5 sec
   pBot->f_find_item_time = gpGlobals->time + 1.0; // don't try to reach items for one sec
}


bool BotCanFallOnTheLeft (bot_t *pBot)
{
   TraceResult tr;

   if (pBot->pEdict == NULL)
      return FALSE; // reliability check

   // use some TraceLines to determine if bot is going to fall from an edge
   UTIL_MakeVectors (pBot->pEdict->v.v_angle); // build base vectors

   // first check for a wall to be certain to trace inside the map
   if (BotCheckForWall (pBot, -gpGlobals->v_right * 80))
      return FALSE; // there is a wall, bot can't fall

   // next trace down to check for an effective fall
   UTIL_TraceLine (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs - (gpGlobals->v_right * 80),
                   pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs - (gpGlobals->v_right * 80) + Vector (0, 0, -250),
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // did the trace hit something strong (a floor) or some water ?
   if ((tr.flFraction < 1.0) || (UTIL_PointContents (tr.vecEndPos) == CONTENTS_WATER))
      return FALSE; // there is a floor or some water down, bot can't fall

   return TRUE;
}


bool BotCanFallOnTheRight (bot_t *pBot)
{
   TraceResult tr;

   if (pBot->pEdict == NULL)
      return FALSE; // reliability check

   // use some TraceLines to determine if bot is going to fall from an edge
   UTIL_MakeVectors (pBot->pEdict->v.v_angle); // build base vectors

   // first check for a wall to be certain to trace inside the map
   if (BotCheckForWall (pBot, gpGlobals->v_right * 80))
      return FALSE; // there is a wall, bot can't fall

   // next trace down to check for an effective fall
   UTIL_TraceLine (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs + (gpGlobals->v_right * 80),
                   pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs + (gpGlobals->v_right * 80) + Vector (0, 0, -250),
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // did the trace hit something strong (a floor) or some water ?
   if ((tr.flFraction < 1.0) || (UTIL_PointContents (tr.vecEndPos) == CONTENTS_WATER))
      return FALSE; // there is a floor or some water down, bot can't fall

   return TRUE;
}


bool BotCanFallForward (bot_t *pBot, TraceResult *tr)
{
   if (pBot->pEdict == NULL)
      return FALSE; // reliability check

   // use some TraceLines to determine if bot is going to fall from an edge
   UTIL_MakeVectors (pBot->pEdict->v.v_angle); // build base vectors

   // first check for a wall to be certain to trace inside the map
   if (BotCheckForWall (pBot, gpGlobals->v_forward * 160))
      return FALSE; // there is a wall, bot can't fall

   // next trace down 80 units ahead to check for an effective fall
   UTIL_TraceLine (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs + (gpGlobals->v_forward * 80),
                   pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs + (gpGlobals->v_forward * 80) + Vector (0, 0, -250),
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, tr);

   // did the trace hit something strong (a floor) or some water ?
   if ((tr->flFraction < 1.0) || (UTIL_PointContents (tr->vecEndPos) == CONTENTS_WATER))
   {
      // 2nd chance, trace again but less close to bot
      UTIL_TraceLine (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs + (gpGlobals->v_forward * 160),
                      pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs + (gpGlobals->v_forward * 160) + Vector (0, 0, -250),
                      ignore_monsters, pBot->pEdict->v.pContainingEntity, tr);

      // did the trace hit something strong (a floor) or some water ?
      if ((tr->flFraction < 1.0) || (UTIL_PointContents (tr->vecEndPos) == CONTENTS_WATER))
         return FALSE; // there is a floor or some water down, bot can't fall
   }

   // else bot is getting VERY close to fall
   else
      pBot->f_reach_time = gpGlobals->time + 0.5; // don't reach point for half a second

   // then trace backwards 160 units in front of the bot 17 units down to find edge plane
   UTIL_TraceLine (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs + Vector (0, 0, -80) + gpGlobals->v_forward * 160,
                   pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs + Vector (0, 0, -80) + gpGlobals->v_forward * 160 - gpGlobals->v_forward * 300,
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, tr);

   // did the trace hit something ?
   if (tr->flFraction < 1.0)
      return TRUE; // if so, then we found the edge plane

   // we did NOT found the edge plane, can't avoid bot to fall, so make him jump to give
   // him a better chance to reach the opposite side of the edge (if any...)

   pBot->BotMove.f_forward_time = gpGlobals->time + 60.0; // run forward
   pBot->BotMove.b_is_walking = FALSE;
   pBot->BotMove.f_jump_time = gpGlobals->time; // jump (banzaii...)
   return FALSE;
}


void BotTurnAtFall (bot_t *pBot, TraceResult *tr)
{
   Vector Normal;
   float Y, Y1, Y2, D1, D2, Z;

   if (pBot->pEdict == NULL)
      return; // reliability check

   // Find the normal vector from the trace result.  The normal vector will
   // be a vector that is perpendicular to the surface from the TraceResult.
   // Don't revert Normal since the edge plane is seen by the 'other side'

   Normal = UTIL_EngineWrapAngles (UTIL_VecToAngles (tr->vecPlaneNormal));

   // Since the bot keeps it's view angle in -180 < x < 180 degrees format,
   // and since TraceResults are 0 < x < 360, we convert the bot's view
   // angle (yaw) to the same format as TraceResult.

   Y = UTIL_EngineWrapAngle (pBot->pEdict->v.v_angle.y + 180);

   // Here we compare the bots view angle (Y) to the Normal - 90 degrees (Y1)
   // and the Normal + 90 degrees (Y2).  These two angles (Y1 & Y2) represent
   // angles that are parallel to the edge surface, but heading in opposite
   // directions.  We want the bot to choose the one that will require the
   // least amount of turning (saves time) and have the bot head off in that
   // direction.

   Y1 = UTIL_EngineWrapAngle (Normal.y - 90);
   Y2 = UTIL_EngineWrapAngle (Normal.y + 90);

   // D1 and D2 are the difference (in degrees) between the bot's current
   // angle and Y1 or Y2 (respectively).

   D1 = abs (Y - Y1);
   D2 = abs (Y - Y2);

   // If difference 1 (D1) is more than difference 2 (D2) then the bot will
   // have to turn LESS if it heads in direction Y1 otherwise, head in
   // direction Y2.  I know this seems backwards, but try some sample angles
   // out on some graph paper and go through these equations using a
   // calculator, you'll see what I mean.

   if (D1 > D2)
      Z = UTIL_WrapAngle (Y1 - RANDOM_FLOAT (0.0, 10.0)); // avoid exact angle
   else
      Z = UTIL_WrapAngle (Y2 + RANDOM_FLOAT (0.0, 10.0)); // avoid exact angle

   BotSetIdealYaw (pBot, Z); // set the direction to head off into...
   pBot->f_reach_time = gpGlobals->time + 1.0; // don't try to reach point for one sec
   pBot->f_find_item_time = gpGlobals->time + 1.0; // don't try to reach items for one sec
   pBot->f_fallcheck_time = gpGlobals->time + 2.0; // give bot time to turn
}


bool BotCantSeeForward (bot_t *pBot)
{
   // use a TraceLine to determine if bot is facing a wall

   Vector v_src, v_forward;

   if (pBot->pEdict == NULL)
      return FALSE; // reliability check

   UTIL_MakeVectors (pBot->pEdict->v.v_angle);

   // do a trace from the bot's eyes forward...

   v_src = pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs; // EyePosition()
   v_forward = v_src + gpGlobals->v_forward * 150; // distance 150

   // trace from the bot's eyes straight forward...
   TraceResult tr;
   UTIL_TraceLine (v_src, v_forward, ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // check if the trace hit something...
   if (tr.flFraction < 1.0)
      return TRUE; // bot can't see forward

   return FALSE; // bot can see forward, return false
}


bool BotCanJumpUp (bot_t *pBot)
{
   // What I do here is trace 3 lines straight out, one unit higher than
   // the highest normal jumping distance.  I trace once at the center of
   // the body, once at the right side, and once at the left side.  If all
   // three of these TraceLines don't hit an obstruction then I know the
   // area to jump to is clear.  I then need to trace from head level,
   // above where the bot will jump to, downward to see if there is anything
   // blocking the jump.  There could be a narrow opening that the body
   // will not fit into.  These horizontal and vertical TraceLines seem
   // to catch most of the problems with falsely trying to jump on something
   // that the bot can not get onto.

   TraceResult tr;
   Vector v_jump, v_source, v_dest;

   if (pBot->pEdict == NULL)
      return FALSE; // reliability check

   // convert current view angle to vectors for TraceLine math...

   v_jump = UTIL_WrapAngles (pBot->pEdict->v.v_angle);
   v_jump.x = 0; // reset pitch to 0 (level horizontally)
   v_jump.z = 0; // reset roll to 0 (straight up and down)

   UTIL_MakeVectors (v_jump); // build base vectors

   // trace a line forward at maximum jump height (64)...
   UTIL_TraceLine (pBot->pEdict->v.origin + Vector (0, 0, -36 + 64),
                   pBot->pEdict->v.origin + Vector (0, 0, -36 + 64) + gpGlobals->v_forward * 24,
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // if trace hit something, return FALSE
   if (tr.flFraction < 1.0)
      return FALSE;

   // now trace a line forward at same height to one side of the bot...
   UTIL_TraceLine (pBot->pEdict->v.origin + gpGlobals->v_right * 16 + Vector (0, 0, -36 + 64),
                   pBot->pEdict->v.origin + gpGlobals->v_right * 16 + Vector (0, 0, -36 + 64) + gpGlobals->v_forward * 24,
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // if trace hit something, return FALSE
   if (tr.flFraction < 1.0)
      return FALSE;

   // now trace a line forward at same height on the other side of the bot...
   UTIL_TraceLine (pBot->pEdict->v.origin - gpGlobals->v_right * 16 + Vector (0, 0, -36 + 64),
                   pBot->pEdict->v.origin - gpGlobals->v_right * 16 + Vector (0, 0, -36 + 64) + gpGlobals->v_forward * 24,
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // if trace hit something, return FALSE
   if (tr.flFraction < 1.0)
      return FALSE;

   // now trace from max jump height upwards to check for obstructions...

   // trace a line straight up starting at max jump height...
   UTIL_TraceLine (pBot->pEdict->v.origin + Vector (0, 0, -36 + 64) + gpGlobals->v_forward * 24,
                   pBot->pEdict->v.origin + Vector (0, 0, -36 + 64) + gpGlobals->v_forward * 24 + Vector (0, 0, 36),
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // if trace hit something, return FALSE
   if (tr.flFraction < 1.0)
      return FALSE;

   // now trace a line straight up starting at max jump height to one side of the bot...
   UTIL_TraceLine (pBot->pEdict->v.origin + gpGlobals->v_right * 16 + Vector (0, 0, -36 + 64) + gpGlobals->v_forward * 24,
                   pBot->pEdict->v.origin + gpGlobals->v_right * 16 + Vector (0, 0, -36 + 64) + gpGlobals->v_forward * 24 + Vector (0, 0, 36),
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // if trace hit something, return FALSE
   if (tr.flFraction < 1.0)
      return FALSE;

   // now trace a line straight up starting at max jump height on the other side of the bot...
   UTIL_TraceLine (pBot->pEdict->v.origin - gpGlobals->v_right * 16 + Vector (0, 0, -36 + 64) + gpGlobals->v_forward * 24,
                   pBot->pEdict->v.origin - gpGlobals->v_right * 16 + Vector (0, 0, -36 + 64) + gpGlobals->v_forward * 24 + Vector (0, 0, 36),
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // if trace hit something, return FALSE
   if (tr.flFraction < 1.0)
      return FALSE;

   return TRUE;
}


bool BotCanDuckUnder (bot_t *pBot)
{
   // What I do here is trace 3 lines straight out, one unit higher than
   // the ducking height.  I trace once at the center of the body, once
   // at the right side, and once at the left side.  If all three of these
   // TraceLines don't hit an obstruction then I know the area to duck to
   // is clear.  I then need to trace from the ground up, 72 units, to make
   // sure that there is something blocking the TraceLine.  Then we know
   // we can duck under it.

   TraceResult tr;
   Vector v_duck, v_source, v_dest;

   if (pBot->pEdict == NULL)
      return FALSE; // reliability check

   // convert current view angle to vectors for TraceLine math...

   v_duck = UTIL_WrapAngles (pBot->pEdict->v.v_angle);
   v_duck.x = 0; // reset pitch to 0 (level horizontally)
   v_duck.z = 0; // reset roll to 0 (straight up and down)

   UTIL_MakeVectors (v_duck); // build base vectors

   // trace a line forward one unit above the ground...
   UTIL_TraceLine (pBot->pEdict->v.origin + Vector (0, 0, -35),
                   pBot->pEdict->v.origin + Vector (0, 0, -35) + gpGlobals->v_forward * 24,
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // if trace hit something, return FALSE
   if (tr.flFraction < 1.0)
      return FALSE;

   // trace a line forward one unit above the ground to one side of the bot...
   UTIL_TraceLine (pBot->pEdict->v.origin + Vector (0, 0, -35) + gpGlobals->v_right * 16,
                   pBot->pEdict->v.origin + Vector (0, 0, -35) + gpGlobals->v_right * 16 + gpGlobals->v_forward * 24,
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // if trace hit something, return FALSE
   if (tr.flFraction < 1.0)
      return FALSE;

   // trace a line forward one unit above the ground on the other side of the bot...
   UTIL_TraceLine (pBot->pEdict->v.origin + Vector (0, 0, -35) - gpGlobals->v_right * 16,
                   pBot->pEdict->v.origin + Vector (0, 0, -35) - gpGlobals->v_right * 16 + gpGlobals->v_forward * 24,
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // if trace hit something, return FALSE
   if (tr.flFraction < 1.0)
      return FALSE;

   // now trace from the ground up to check for object to duck under...

   // trace a line straight up in the air...
   UTIL_TraceLine (pBot->pEdict->v.origin + Vector (0, 0, -35) + gpGlobals->v_forward * 24,
                   pBot->pEdict->v.origin + Vector (0, 0, -35) + gpGlobals->v_forward * 24 + Vector (0, 0, 100),
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // if trace hit something lower than max duck height, return FALSE
   if (tr.flFraction < 0.36)
      return FALSE;

   // now check same height to one side of the bot...
   UTIL_TraceLine (pBot->pEdict->v.origin + Vector (0, 0, -35) + gpGlobals->v_right * 16 + gpGlobals->v_forward * 24,
                   pBot->pEdict->v.origin + Vector (0, 0, -35) + gpGlobals->v_right * 16 + gpGlobals->v_forward * 24 + Vector (0, 0, 100),
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // if trace hit something lower than max duck height, return FALSE
   if (tr.flFraction < 0.36)
      return FALSE;

   // now check same height on the other side of the bot...
   UTIL_TraceLine (pBot->pEdict->v.origin + Vector (0, 0, -35) - gpGlobals->v_right * 16 + gpGlobals->v_forward * 24,
                   pBot->pEdict->v.origin + Vector (0, 0, -35) - gpGlobals->v_right * 16 + gpGlobals->v_forward * 24 + Vector (0, 0, 100),
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // if trace hit something lower than max duck height, return FALSE
   if (tr.flFraction < 0.36)
      return FALSE;

   return TRUE;
}


void BotRandomTurn (bot_t *pBot)
{
   float distance = 0, prev_distance = 0, prev_prev_distance = 0, interesting_angles[72];
   int index, angles_count = 0;
   float angle;
   edict_t *pPrevHit;
   TraceResult tr;

   if (pBot->pEdict == NULL)
      return; // reliability check

   angle = UTIL_WrapAngle (pBot->pEdict->v.v_angle.y); // initialize scan angle to bot's view angle
   pPrevHit = pBot->pEdict; // initialize previous hit entity to the bot himself

   // scan 360 degrees around here in 72 samples...
   for (index = 0; index < 72; index++)
   {
      angle = UTIL_WrapAngle (angle + 5); // pan the trace angle from left to right
      UTIL_MakeVectors (Vector (0, angle, 0)); // build base vectors in that direction

      // trace line slightly under eyes level
      UTIL_TraceLine (pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2),
                      (pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2)) + (gpGlobals->v_forward * 10000),
                      ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

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
         interesting_angles[angles_count] = UTIL_WrapAngle (angle - 5); // remember this angle
         angles_count++; // increment interesting angles count
      }

      pPrevHit = tr.pHit; // save previous hit entity
   }

   // okay, now we know which angles are candidates for determining a good watch angle
   if ((angles_count > 0) && (angles_count < 72))
   {
      angle = UTIL_WrapAngle (interesting_angles[RANDOM_LONG (0, angles_count - 1)]); // choose one
      pBot->BotMove.f_forward_time = 0; // don't move while turning
      pBot->f_reach_time = gpGlobals->time + 1.0; // don't try to reach point for one second
   }

   // aargh, can't even figure out where to face !! fall back to totally randomness
   else
   {
      angle = RANDOM_FLOAT (-180, 180); // choose a random angle
      pBot->BotMove.f_forward_time = 0; // don't move while turning
      pBot->f_reach_time = gpGlobals->time + 1.0; // don't try to reach point for one second
   }

   BotSetIdealYaw (pBot, angle); // face the choosen angle
   return;
}


void BotFollowUser (bot_t *pBot)
{
   if ((pBot->pEdict == NULL) || (pBot->pBotUser == NULL))
      return; // reliability check

   // first check: is the user dead or not visible for more than 6 seconds ?
   if (!IsAlive (pBot->pBotUser)
       || (!BotCanSeeThis (pBot, pBot->pBotUser->v.origin)
           && (pBot->f_bot_use_time + 6 < gpGlobals->time)))
   {
      pBot->BotChat.b_saytext_cant = TRUE; // bot says can't follow the user
      pBot->f_bot_saytext_time = gpGlobals->time;
   }

   // is the user really visible ?
   if (BotGetIdealAimVector (pBot, pBot->pBotUser) != Vector (0, 0, 0))
   {
      pBot->f_bot_use_time = gpGlobals->time; // reset last visible user time
      pBot->v_lastseenuser_position = pBot->pBotUser->v.origin; // remember last seen user position
   }

   // where is/was our user ?
   Vector v_user = pBot->v_lastseenuser_position - pBot->pEdict->v.origin;

   // is the bot far from the user OR the user is climbing up a ladder ?
   if ((v_user.Length () > 120) || (pBot->pBotUser->v.movetype == MOVETYPE_FLY))
   {
      TraceResult tr;

      // is the user climbing up a ladder OR the bot is climbing up a ladder?
      if ((pBot->pBotUser->v.movetype == MOVETYPE_FLY) || (pBot->pEdict->v.movetype == MOVETYPE_FLY))
      {
         // is bot NOT on ladder yet ?
         if (pBot->pEdict->v.movetype != MOVETYPE_FLY)
         {
            BotFindLadder (pBot); // find where this ladder is
            pBot->ladder_dir = LADDER_UNKNOWN;
         }
         // else the bot IS on the ladder
         else
         {
            // check if bot JUST got on the ladder...
            if ((pBot->f_end_use_ladder_time + 1.0) < gpGlobals->time)
               pBot->f_start_use_ladder_time = gpGlobals->time;

            // go handle the ladder movement
            BotFollowOnLadder (pBot);

            pBot->f_end_use_ladder_time = gpGlobals->time;
         }

         pBot->BotMove.f_forward_time = gpGlobals->time + 60.0; // reach the ladder / climb up the ladder
         pBot->BotMove.b_is_walking = FALSE;
      }

      // else the user is NOT on a ladder, run to the position where our user was
      else if (pBot->f_reach_time < gpGlobals->time)
         BotReachPosition (pBot, pBot->v_lastseenuser_position);

      // if bot is about to hit something...
      if (BotCantMoveForward (pBot, &tr))
      {
         // can the bot jump up ?
         if (BotCanJumpUp (pBot))
            pBot->BotMove.f_jump_time = gpGlobals->time; // jump up and move forward

         // else can the bot duck under something ?
         else if (BotCanDuckUnder (pBot))
            pBot->BotMove.f_duck_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5); // duck & go
      }

      // it will soon be time to check around for possible enemies...
      pBot->f_randomturn_time = gpGlobals->time + 2.0;
   }
   else if (v_user.Length () > 50) // bot is in place
   {
      // if no enemy AND time to look around OR bot can't see forward
      if (((pBot->pBotEnemy == NULL) && (pBot->f_randomturn_time < gpGlobals->time))
          || (BotCantSeeForward (pBot) && (pBot->f_randomturn_time + 0.2 < gpGlobals->time)))
      {
         BotRandomTurn (pBot); // randomly turnaround
         pBot->f_randomturn_time = gpGlobals->time + RANDOM_FLOAT (0.5, 15.0);
      }

      pBot->BotMove.f_forward_time = 0; // don't move if close enough
   }
   else if (pBot->pBotEnemy == NULL)
   {
      // bot is too close to the user (don't check if bot has an enemy)
      BotSetIdealYaw (pBot, UTIL_VecToAngles (pBot->pBotUser->v.origin - pBot->pEdict->v.origin).y); // face the user
      pBot->BotMove.f_backwards_time = gpGlobals->time + 0.06; // make a step backwards
      pBot->f_randomturn_time = gpGlobals->time + RANDOM_FLOAT (0.5, 2.0);
   }
}


void BotFindLadder (bot_t *pBot)
{
   edict_t *pent = NULL;
   TraceResult tr;

   if (pBot->pEdict == NULL)
      return; // reliability check

   while ((pent = UTIL_FindEntityByClassname (pent, "func_ladder")) != NULL)
   {
      // see if this "func_ladder" entity is within bot's search range
      if ((VecBModelOrigin (pent) - pBot->pEdict->v.origin).Length () < 500)
      {
         // force ladder origin to same z coordinate as bot since the VecBModelOrigin is the
         // center of the ladder. For long ladders, the center may be hundreds of units above
         // the bot. Fake an origin at the same level as the bot...
         Vector ladder_origin = VecBModelOrigin (pent);
         ladder_origin.z = pBot->pEdict->v.origin.z;

         // check if ladder is outside field of view (+/- 60 degrees)
         if (BotAngleToLocation (pBot, ladder_origin - pBot->pEdict->v.origin) > 60)
            continue; // skip this item if bot can't "see" it

         // trace a line from bot's eyes to func_ladder entity...
         UTIL_TraceLine (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs,
                         ladder_origin,
                         ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

         // check if traced all the way up to the ladder (didn't hit wall)
         if (tr.flFraction == 1.0)
         {
            pBot->v_reach_point = ladder_origin; // save the ladder bot is trying to reach
            pBot->b_is_picking_item = TRUE; // ladders priority is just the same as pickup items
            return;
         }
      }
   }
}


void BotStayInPosition (bot_t *pBot)
{
   bool b_can_see_position;
   TraceResult tr;

   if (pBot->pEdict == NULL)
      return; // reliability check

   b_can_see_position = BotCanSeeThis (pBot, pBot->v_place_to_keep);

   // first check: is the place not visible for more than 3 seconds OR bot is camping and enemy in sight ?
   if ((!b_can_see_position && (pBot->f_place_time + 3 < gpGlobals->time))
       || ((pBot->f_camp_time > gpGlobals->time) && (pBot->pBotEnemy != NULL)))
   {
      pBot->v_place_to_keep = Vector (0, 0, 0); // forget this place
      pBot->f_camp_time = gpGlobals->time; // don't camp anymore as long as this enemy is alive
      return;
   }

   // else is the place visible ?
   else if (b_can_see_position)
      pBot->f_place_time = gpGlobals->time; // reset place time

   // how far is our place ?
   if ((pBot->v_place_to_keep - pBot->pEdict->v.origin).Length () > 50)
   {

      // if time to, run to the position where the bot should be
      if (pBot->f_reach_time < gpGlobals->time)
         BotReachPosition (pBot, pBot->v_place_to_keep);

      // if bot is about to hit something...
      if (BotCantMoveForward (pBot, &tr))
      {
         // can the bot jump up ?
         if (BotCanJumpUp (pBot))
            pBot->BotMove.f_jump_time = gpGlobals->time; // jump up and move forward

         // else can the bot duck under something ?
         else if (BotCanDuckUnder (pBot))
            pBot->BotMove.f_duck_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5); // duck & go
      }

      // it will soon be time to check around for possible enemies...
      pBot->f_randomturn_time = gpGlobals->time + 0.1;
   }
   else
   {
      pBot->BotMove.f_forward_time = 0; // don't move if close enough

      // is bot camping ?
      if (pBot->f_camp_time > gpGlobals->time)
      {
         pBot->BotMove.f_duck_time = gpGlobals->time + 0.5; // duck if bot is camping
         pBot->f_avoid_time = gpGlobals->time + 0.5; // don't avoid walls for a while too
      }

      // else has the bot just stopped camping ?
      else if (pBot->f_camp_time + 0.5 > gpGlobals->time)
      {
         pBot->v_place_to_keep = Vector (0, 0, 0); // free the slot
         pBot->f_camp_time = gpGlobals->time; // reset camping state
      }

      // if no enemy AND time to look around OR bot can't see forward
      if (((pBot->pBotEnemy == NULL) && (pBot->f_randomturn_time < gpGlobals->time))
          || (BotCantSeeForward (pBot) && (pBot->f_randomturn_time + 0.2 < gpGlobals->time)))
      {
         BotRandomTurn (pBot); // randomly turnaround
         pBot->f_randomturn_time = gpGlobals->time + RANDOM_FLOAT (0.5, 15.0);
      }

      pBot->pEdict->v.angles.x = 0;
   }
}


bool BotCheckForWall (bot_t *pBot, Vector v_direction)
{
   TraceResult tr;

   if (pBot->pEdict == NULL)
      return FALSE; // reliability check

   // check for a wall in v_direction
   UTIL_MakeVectors (pBot->pEdict->v.v_angle); // build base vectors
   UTIL_TraceLine (pBot->pEdict->v.origin,
                   pBot->pEdict->v.origin + v_direction,
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   if (tr.flFraction < 1.0)
      return TRUE; // if the trace hit something, then there is a wall in v_direction

   return FALSE; // else trace hit nothing, there is no wall
}


void BotCheckForCorners (bot_t *pBot)
{
   TraceResult tr, tr2;
   float distance = 100;
   bool can_turn_left = FALSE, can_turn_right = FALSE, can_go_ahead = FALSE;

   if (pBot->pEdict == NULL)
      return; // reliability check

   UTIL_MakeVectors (pBot->pEdict->v.v_angle); // build base vectors

   // make sure we are tracing inside the map...
   UTIL_TraceLine (pBot->pEdict->v.origin,
                   pBot->pEdict->v.origin + gpGlobals->v_forward * distance * 7,
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);
   if (tr.flFraction == 1.0)
      can_go_ahead = TRUE; // there's still a long way ahead
   else if (tr.flFraction * 7 < 1.0)
      distance = distance * tr.flFraction * 7 * 0.99;

   // do a trace from 75 units in front of the bot's eyes left...
   UTIL_TraceLine (pBot->pEdict->v.origin + gpGlobals->v_forward * distance * 0.75,
                   pBot->pEdict->v.origin + gpGlobals->v_forward * distance * 0.75 - gpGlobals->v_right * 1000,
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // do a trace from 100 units in front of the bot's eyes left...
   UTIL_TraceLine (pBot->pEdict->v.origin + gpGlobals->v_forward * distance,
                   pBot->pEdict->v.origin + gpGlobals->v_forward * distance - gpGlobals->v_right * 1000,
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr2);

   // did the close trace hit something AND did the far trace hit something much more far ?
   if ((tr.flFraction < 1.0) && (tr2.flFraction > tr.flFraction * 2))
      can_turn_left = TRUE; // there's a corner on the left

   // do a trace from 75 units in front of the bot's eyes right...
   UTIL_TraceLine (pBot->pEdict->v.origin + gpGlobals->v_forward * distance * 0.75,
                   pBot->pEdict->v.origin + gpGlobals->v_forward * distance * 0.75 + gpGlobals->v_right * 1000,
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // do a trace from 100 units in front of the bot's eyes right...
   UTIL_TraceLine (pBot->pEdict->v.origin + gpGlobals->v_forward * distance,
                   pBot->pEdict->v.origin + gpGlobals->v_forward * distance + gpGlobals->v_right * 1000,
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr2);

   // did the close trace hit something AND did the far trace hit something much more far ?
   if ((tr.flFraction < 1.0) && (tr2.flFraction > tr.flFraction * 2))
      can_turn_right = TRUE; // there's a corner on the right

//   // does the bot have a goal ?
//   if (pBot->v_goal != Vector (0, 0, 0))
//   {
//      // which side is our goal ?
//      Vector v_goal_angle = UTIL_WrapAngles (UTIL_VecToAngles (pBot->v_goal - pBot->pEdict->v.origin) - pBot->pEdict->v.v_angle);
//
//      if ((abs (v_goal_angle.x) < 45) && (v_goal_angle.y > 45))
//      {
//         can_go_ahead = FALSE; // bot doesn't want to go further this way
//         can_turn_right = FALSE; // bot won't turn right either
//      }
//      else  if ((abs (v_goal_angle.x) < 45) && (v_goal_angle.y < -45))
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
      BotAddIdealYaw (pBot, RANDOM_LONG (60, 80)); // turn there
      pBot->BotMove.f_straferight_time = gpGlobals->time + RANDOM_FLOAT (0.3, 0.8); // strafe
      pBot->f_reach_time = gpGlobals->time + 0.5; // don't try to reach point for 0.5 second
      pBot->f_turncorner_time = gpGlobals->time + 3.0; // don't check for corners for 3 secs
      return;
   }
   else if (can_turn_right)
   {
      BotAddIdealYaw (pBot, -RANDOM_LONG (60, 80)); // turn there
      pBot->BotMove.f_strafeleft_time = gpGlobals->time + RANDOM_FLOAT (0.3, 0.8); // strafe
      pBot->f_reach_time = gpGlobals->time + 0.5; // don't try to reach point for 0.5 second
      pBot->f_turncorner_time = gpGlobals->time + 3.0; // don't check for corners for 3 secs
      return;
   }
   else if (can_go_ahead)
   {
      pBot->f_turncorner_time = gpGlobals->time + 1.0; // don't check for corners for 1 sec
      return;
   }
   else
      pBot->f_turncorner_time = gpGlobals->time + 0.10; // found nothing, next check in 100 ms

   return;
}


void BotWander (bot_t *pBot)
{
   if (pBot->pEdict == NULL)
      return; // reliability check

   pBot->pEdict->v.v_angle.z = 0; // reset roll to 0 (straight up and down)
   pBot->pEdict->v.angles.x = 0;
   pBot->pEdict->v.angles.y = UTIL_WrapAngle (pBot->pEdict->v.v_angle.y); // make body face eye's direction
   pBot->pEdict->v.angles.z = 0;

   pBot->BotMove.f_forward_time = gpGlobals->time + 60.0; // let our bot go...

   // if bot is using a button...
   if (pBot->b_interact)
      BotUseLift (pBot); // bot may be on a lift

   // else if bot is underwater...
   else if (pBot->pEdict->v.waterlevel == 3)
      BotUnderWater (pBot); // handle under water movement

   // else if bot is on a ladder...
   else if (pBot->pEdict->v.movetype == MOVETYPE_FLY)
      BotOnLadder (pBot); // handle ladder movement

   // else if the bot JUST got off the ladder...
   else if (pBot->f_end_use_ladder_time + 1.0 > gpGlobals->time)
      pBot->ladder_dir = LADDER_UNKNOWN;

   // else if some door to open or some button to press...
   else if (BotCanUseInteractives (pBot) && !pBot->b_is_picking_item)
      BotInteractWithWorld (pBot);

   // else let's just wander around
   else
   {
      TraceResult tr;

      pBot->BotMove.b_is_walking = FALSE; // let our bot go...

      // if time to get a new reach point and bot is not picking an item...
      if ((pBot->f_samplefov_time < gpGlobals->time) && (pBot->f_reach_time < gpGlobals->time)
          && !pBot->b_is_picking_item)
         BotSampleFOV (pBot); // get a new reach point

      // if still time to reach it AND it is visible...
      if ((pBot->f_reach_time < gpGlobals->time) && FInViewCone (pBot->v_reach_point, pBot->pEdict))
         BotReachPosition (pBot, pBot->v_reach_point);

      // if bot is about to hit a wall...
      if (BotCantMoveForward (pBot, &tr))
         if ((pBot->v_reach_point - pBot->pEdict->v.origin).Length () > 40)
            BotTurnAtWall (pBot, &tr); // turn at wall

      // if bot is about to fall...
      if (pBot->f_fallcheck_time < gpGlobals->time)
         if (BotCanFallForward (pBot, &tr))
            BotTurnAtFall (pBot, &tr); // try to avoid falling

      // if bot should check for corners on his sides and bot is not picking an item...
      if ((pBot->f_turncorner_time < gpGlobals->time) && !pBot->b_is_picking_item)
         BotCheckForCorners (pBot);

      // if the bot should pause for a while here...
      if ((RANDOM_LONG (1, 1000) <= pause_frequency[pBot->bot_skill - 1]) && (pBot->pBotUser == NULL)
          && (pBot->f_buy_time + 20.0 < gpGlobals->time) && !pBot->b_is_picking_item
          && !BotCantSeeForward (pBot) && (pBot->f_rush_time < gpGlobals->time))
         pBot->f_pause_time = gpGlobals->time + RANDOM_FLOAT (pause_time[pBot->bot_skill - 1][0], pause_time[pBot->bot_skill - 1][1]);

      // if the bot is stuck and not paused...
      if (BotIsStuck (pBot) && (pBot->f_pause_time < gpGlobals->time))
         BotUnstuck (pBot); // try to unstuck our poor bot
   }
}


void BotReachPosition (bot_t *pBot, Vector v_position)
{
   TraceResult tr;

   if (pBot->pEdict == NULL)
      return; // reliability check

   // first check: do we mind if the position is not visible ?
   if (!BotCanSeeThis (pBot, v_position))
      return; // if so, give up: bot might see it again in a few secs while wandering

   // look at destination
   BotPointGun (pBot, UTIL_VecToAngles (v_position - GetGunPosition (pBot->pEdict)));

   pBot->BotMove.f_forward_time = gpGlobals->time + 60.0; // go forward to position

   if ((v_position - pBot->pEdict->v.origin).Length () > 40)
      pBot->BotMove.b_is_walking = FALSE; // if still far, run
   else
      pBot->BotMove.b_is_walking = TRUE; // else walk while getting closer

   // is the bot about to fall ? (TraceResult gets returned)
   if (BotCanFallForward (pBot, &tr) && (pBot->f_fallcheck_time < gpGlobals->time))
   {
      // if bot is not following anyone
      if (pBot->f_bot_use_time + 5.0 < gpGlobals->time)
         BotTurnAtFall (pBot, &tr); // try to avoid falling
      else
         pBot->BotMove.f_jump_time = gpGlobals->time; // jump to follow user

      return; // give up: bot might see his destination again in a few secs while wandering
   }

   // check if the bot is stuck, not paused and NOT on a ladder since handled elsewhere
   if (BotIsStuck (pBot) && (pBot->pEdict->v.movetype != MOVETYPE_FLY) && (pBot->f_pause_time < gpGlobals->time))
      BotUnstuck (pBot); // try to unstuck our poor bot
}


void BotSampleFOV (bot_t *pBot)
{
   float angle, distance = 0, maxdistance = 0;
   Vector v_viewangle;
   TraceResult tr, tr2, tr3;

   if (pBot->pEdict == NULL)
      return; // reliability check

   // scan 100 degrees of bot's field of view...
   for (angle = -50; angle <= 50; angle += (angle * angle) / 1500 + 1.5)
   {
      v_viewangle = UTIL_WrapAngles (pBot->pEdict->v.v_angle); // restore bot's current v_angle
      v_viewangle.y = UTIL_WrapAngle (v_viewangle.y + angle); // pan it from left to right
      UTIL_MakeVectors (v_viewangle); // build base vectors in that direction

      // trace line slightly under eyes level
      UTIL_TraceLine (pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2),
                      (pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2)) + (gpGlobals->v_forward * 10000),
                      ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

      if (b_debug_nav)
         UTIL_DrawBeam (listenserver_edict,
                        pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2),
                        (pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2)) + ((gpGlobals->v_forward * 10000) * tr.flFraction),
                        1, 5, 0, 255, 0, 0, 255, 0);

      distance = tr.flFraction; // store distance to obstacle

      // if this plane is a slope that is smooth enough for bot to climb it or a slope that goes down...
      if ((UTIL_AngleOfVectors (tr.vecPlaneNormal, Vector (0, 0, 1)) < 30) || (UTIL_AngleOfVectors (tr.vecPlaneNormal, Vector (0, 0, -1)) < 30))
      {
         // trace line parallel to previous starting a bit lower to get a new hit point
         UTIL_TraceLine ((pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2)) + Vector (0, 0, -5),
                         ((pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2)) + Vector (0, 0, -5)) + (gpGlobals->v_forward * 10000),
                         ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr2);

         // compute a normalized vector parallel to slope
         Vector v_parallel = (tr.vecEndPos - tr2.vecEndPos).Normalize ();

         // trace line parallel to slope so that the bot 'sees' up or down the slope
         UTIL_TraceLine ((pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2)) + (((gpGlobals->v_forward * 10000) * tr.flFraction) * 0.99),
                         ((pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2)) + (((gpGlobals->v_forward * 10000) * tr.flFraction) * 0.99)) + (v_parallel * 10000),
                         ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr3);

         if (b_debug_nav)
            UTIL_DrawBeam (listenserver_edict,
                           (pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2)) + (((gpGlobals->v_forward * 10000) * tr.flFraction) * 0.99),
                           ((pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2)) + (((gpGlobals->v_forward * 10000) * tr.flFraction) * 0.99)) + ((v_parallel * 10000) * tr3.flFraction),
                           1, 5, 0, 255, 0, 0, 255, 0);

         // add distance from this traceline to the first one so that bot 'sees' up or down the slope
         distance += tr3.flFraction;
      }

/*    --- THIS DOES NOT WORK ---
      // else if plane is vertical and not too high, check if plane is a stair
      else if ((tr.vecPlaneNormal.z == 0) && (tr.pHit->v.size.z < 18))
      {
         // trace line at eyes level less maximal stair height (18)
         UTIL_TraceLine ((pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2)) + Vector (0, 0, -18),
                         ((pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2)) + Vector (0, 0, -18)) + (gpGlobals->v_forward * 10000),
                         ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr2);

         // if two both not same length and relatively close together, bot may have found a staircase
         if ((tr2.flFraction < tr.flFraction)
             && (((tr2.flFraction - tr.flFraction) * 10000 < 50)
                 && ((tr2.flFraction - tr.flFraction) * 10000 > 10)))
         {
            // compute a normalized vector parallel to staircase slope
            Vector v_parallel = (tr.vecEndPos - tr2.vecEndPos).Normalize ();

            // trace line parallel to staircase slope so that the bot 'sees' up the staircase
            UTIL_TraceLine ((pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2)) + (((gpGlobals->v_forward * 10000) * tr.flFraction) * 0.80),
                            ((pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2)) + (((gpGlobals->v_forward * 10000) * tr.flFraction) * 0.80)) + (v_parallel * 10000),
                            ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr3);

            if (b_debug_nav)
               UTIL_DrawBeam (listenserver_edict,
                              (pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2)) + (((gpGlobals->v_forward * 10000) * tr.flFraction) * 0.80),
                              ((pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2)) + (((gpGlobals->v_forward * 10000) * tr.flFraction) * 0.80)) + ((v_parallel * 10000) * tr3.flFraction),
                              1, 5, 0, 255, 0, 0, 255, 0);

            // add distance from this traceline to the first one so that bot 'sees' up the staircase
            distance += tr3.flFraction;
         }
      }
      --- THIS DOES NOT WORK ---
*/

      if (distance > maxdistance)
      {
         maxdistance = distance; // the reach point will be in the longest direction

         Vector v_vecEndPos_dropped = DropAtHumanHeight (tr.vecEndPos); // drop the reach point
         if (FVisible (v_vecEndPos_dropped, pBot->pEdict))
            pBot->v_reach_point = v_vecEndPos_dropped; // place this reach point on the ground
         else
            pBot->v_reach_point = tr.vecEndPos; // let this reach point as is
      }
   }

   // ok so far, bot has a direction to head off into...
   if (b_debug_nav)
      UTIL_DrawBeam (listenserver_edict,
                     pBot->pEdict->v.origin + (pBot->pEdict->v.view_ofs / 2),
                     pBot->v_reach_point,
                     1, 10, 0, 255, 255, 255, 255, 0);

   pBot->f_samplefov_time = gpGlobals->time + 0.20; // next sampling in 200 ms
   return;
}


bool BotIsStuck (bot_t *pBot)
{
   if (pBot->pEdict == NULL)
      return FALSE; // reliability check

   // this check is made in two states
   if (((pBot->pEdict->v.origin - pBot->v_prev_position).Length () < 3) && ((pBot->BotMove.f_move_speed > 0) || (pBot->BotMove.f_strafe_speed > 0)))
   {
      if (!pBot->b_is_stuck)
      {
         pBot->b_is_stuck = TRUE; // set stuck flag for next check
         pBot->f_check_stuck_time = gpGlobals->time + 0.5; // check in half a second
         return FALSE; // starting check...
      }
      else if (pBot->f_check_stuck_time > gpGlobals->time)
         return FALSE; // still checking...
      else
      {
         pBot->b_is_stuck = FALSE; // reset stuck flag for next check
         return TRUE; // bot definitely doesn't move as fast as he would like to
      }
   }
   else
   {
      pBot->b_is_stuck = FALSE;
      return FALSE; // bot is definitely NOT stuck
   }
}


void BotUnstuck (bot_t *pBot)
{
   if (pBot->pEdict == NULL)
      return; // reliability check

   // check if bot can jump onto something and has not jumped for quite a time
   if (BotCanJumpUp (pBot) && (pBot->BotMove.f_jump_time + 3.0 < gpGlobals->time))
      pBot->BotMove.f_jump_time = gpGlobals->time; // jump up and move forward

   // else check if bot can duck under something
   else if (BotCanDuckUnder (pBot))
      pBot->BotMove.f_duck_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5); // duck & move forward

   // else can't figure out whether to jump or duck, try to jump first...
   else if (pBot->BotMove.f_jump_time + 3.0 < gpGlobals->time)
      pBot->BotMove.f_jump_time = gpGlobals->time; // jump up and move forward

   // else duck
   else if (pBot->BotMove.f_duck_time + 3.0 < gpGlobals->time)
      pBot->BotMove.f_duck_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.0); // duck & move forward

   // else is the bot trying to get to an item?...
   else if (pBot->b_is_picking_item)
   {
      pBot->b_is_picking_item = FALSE; // give up trying to reach that item
      pBot->f_find_item_time = gpGlobals->time + 10.0; // don't look for items
   }

   // else our destination is REALLY unreachable, try to turnaround to unstuck
   else
      BotRandomTurn (pBot); // randomly turnaround
}


void BotAvoidObstacles (bot_t *pBot)
{
   TraceResult tr, tr2;
   int player_index;

   if (pBot->pEdict == NULL)
      return; // reliability check

   // search the world for players...
   for (player_index = 1; player_index <= gpGlobals->maxClients; player_index++)
   {
      edict_t *pPlayer = INDEXENT (player_index);

      if (!pPlayer || pPlayer->free || (pPlayer == pBot->pEdict))
         continue; // skip invalid players and skip self (i.e. this bot)

      if (b_observer_mode && !(pPlayer->v.flags & FL_FAKECLIENT))
         continue; // skip real players in observer mode

      if (GetTeam (pPlayer) != GetTeam (pBot->pEdict))
         continue; // don't mind about enemies...

      Vector vecEnd = pPlayer->v.origin + pPlayer->v.view_ofs;

      // see if bot can see the teammate...
      if (FInViewCone (vecEnd, pBot->pEdict) && BotCanSeeThis (pBot, vecEnd))
      {
         // bot found a visible teammate
         Vector v_teammate_angle = UTIL_WrapAngles (UTIL_VecToAngles (vecEnd - pBot->pEdict->v.origin) - pBot->pEdict->v.v_angle);
         float f_teammate_distance = (pPlayer->v.origin - pBot->pEdict->v.origin).Length ();

         // is that teammate near us OR coming in front of us and within a certain distance ?
         if ((f_teammate_distance < 100)
             || ((f_teammate_distance < 300) && (v_teammate_angle.y < 15)
                 && (abs (UTIL_WrapAngle (pPlayer->v.v_angle.y - pBot->pEdict->v.v_angle.y)) > 165)))
         {
            // if we are moving full speed AND there's room forward OR teammate is very close...
            if (((pBot->pEdict->v.velocity.Length2D () > 10) && !BotCantMoveForward (pBot, &tr))
                || (f_teammate_distance < 70))
            {
               if (v_teammate_angle.y > 0)
               {
                  pBot->BotMove.f_strafeleft_time = 0;
                  pBot->BotMove.f_straferight_time = gpGlobals->time + 0.1; // strafe right to avoid him
               }
               else
               {
                  pBot->BotMove.f_strafeleft_time = gpGlobals->time + 0.1; // strafe left to avoid him
                  pBot->BotMove.f_straferight_time = 0;
               }

               pBot->f_reach_time = gpGlobals->time + 0.5; // delay reaching point
            }
         }
      }
   }

   // determine if bot need to strafe to avoid walls and corners
   UTIL_MakeVectors (pBot->pEdict->v.v_angle); // build base vectors

   // do a trace on the left side of the bot some steps forward
   UTIL_TraceLine (pBot->pEdict->v.origin - gpGlobals->v_right * 16,
                   pBot->pEdict->v.origin - gpGlobals->v_right * 16 + gpGlobals->v_forward * 40,
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // do a trace on the right side of the bot some steps forward
   UTIL_TraceLine (pBot->pEdict->v.origin + gpGlobals->v_right * 16,
                   pBot->pEdict->v.origin + gpGlobals->v_right * 16 + gpGlobals->v_forward * 40,
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr2);

   // did the right trace hit something further than the left trace ?
   if (tr.flFraction < tr2.flFraction)
   {
      pBot->BotMove.f_strafeleft_time = 0;
      pBot->BotMove.f_straferight_time = gpGlobals->time + 0.1; // there's an obstruction, strafe to avoid it
   }

   // else did the left trace hit something further than the right trace ?
   else if (tr.flFraction > tr2.flFraction)
   {
      pBot->BotMove.f_strafeleft_time = gpGlobals->time + 0.1; // there's an obstruction, strafe to avoid it
      pBot->BotMove.f_straferight_time = 0;
   }

   // else there is no immediate obstruction, check further...
   else
   {
      // make sure we trace inside the map (check on the left)
      UTIL_TraceLine (pBot->pEdict->v.origin,
                      pBot->pEdict->v.origin - gpGlobals->v_right * 30 + gpGlobals->v_forward * 30,
                      ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

      // make sure we trace inside the map (check on the right)
      UTIL_TraceLine (pBot->pEdict->v.origin,
                      pBot->pEdict->v.origin + gpGlobals->v_right * 30 + gpGlobals->v_forward * 30,
                      ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr2);

      // if there is a wall on the left
      if (tr.flFraction < 1.0)
      {
         pBot->BotMove.f_strafeleft_time = 0;
         pBot->BotMove.f_straferight_time = gpGlobals->time + 0.1; // strafe to avoid it
      }

      // else if there is a wall on the right
      else if (tr2.flFraction < 1.0)
      {
         pBot->BotMove.f_strafeleft_time = gpGlobals->time + 0.1; // strafe to avoid it
         pBot->BotMove.f_straferight_time = 0;
      }

      // else no side obstruction, check further...
      else
      {
         // do a trace from 30 units on the left of the bot to destination
         UTIL_TraceLine (pBot->pEdict->v.origin - gpGlobals->v_right * 30 + gpGlobals->v_forward * 30,
                         pBot->v_reach_point,
                         ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

         // do a trace from 30 units on the right of the bot to destination
         UTIL_TraceLine (pBot->pEdict->v.origin + gpGlobals->v_right * 30 + gpGlobals->v_forward * 30,
                         pBot->v_reach_point,
                         ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr2);

         // did the left trace hit something at a close range AND the right trace hit nothing ?
         if ((tr.flFraction < 0.3) && (tr2.flFraction == 1.0))
         {
            pBot->BotMove.f_strafeleft_time = 0;
            pBot->BotMove.f_straferight_time = gpGlobals->time + 0.1; // there's a corner, strafe to avoid it
         }

         // did the right trace hit something at a close range AND the left trace hit nothing ?
         if ((tr2.flFraction < 0.3) && (tr.flFraction == 1.0))
         {
            pBot->BotMove.f_strafeleft_time = gpGlobals->time + 0.1; // there's a corner, strafe to avoid it
            pBot->BotMove.f_straferight_time = 0;
         }
      }
   }

   pBot->f_avoid_time = gpGlobals->time + 0.2; // next check in 200 ms
   return;
}


bool BotCanSeeThis (bot_t *pBot, Vector v_destination)
{
   TraceResult tr, tr2;

   if (pBot->pEdict == NULL)
      return FALSE; // reliability check

   // don't look through water
   if ((UTIL_PointContents (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs) == CONTENTS_WATER)
       != (UTIL_PointContents (v_destination) == CONTENTS_WATER))
      return FALSE;

   // look from bot's left and right eyes
   UTIL_MakeVectors (pBot->pEdict->v.v_angle); // build base vectors
   UTIL_TraceLine (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs - gpGlobals->v_right * 16,
                   v_destination, ignore_monsters, ignore_glass, pBot->pEdict->v.pContainingEntity, &tr);
   UTIL_TraceLine (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs + gpGlobals->v_right * 16,
                   v_destination, ignore_monsters, ignore_glass, pBot->pEdict->v.pContainingEntity, &tr2);

   if ((tr.flFraction == 1.0) && (tr2.flFraction == 1.0))
      return TRUE; // line of sight is excellent

   else if ((tr.flFraction == 1.0) && (tr2.flFraction < 1.0))
      return TRUE; // line of sight is valid, though bot might want to strafe left to see better

   else if ((tr.flFraction < 1.0) && (tr2.flFraction == 1.0))
      return TRUE; // line of sight is valid, though bot might want to strafe right to see better

   return FALSE; // line of sight is not established
}


bool BotCanSeeThisBModel (bot_t *pBot, edict_t *pBModel)
{
   TraceResult tr, tr2;

   if (pBot->pEdict == NULL)
      return FALSE; // reliability check

   // don't look through water
   if ((UTIL_PointContents (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs) == CONTENTS_WATER)
       != (UTIL_PointContents (VecBModelOrigin (pBModel)) == CONTENTS_WATER))
      return FALSE;

   // look from bot's left and right eyes
   UTIL_MakeVectors (pBot->pEdict->v.v_angle); // build base vectors
   UTIL_TraceLine (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs - gpGlobals->v_right * 16,
                   VecBModelOrigin (pBModel), dont_ignore_monsters, ignore_glass, pBot->pEdict->v.pContainingEntity, &tr);
   UTIL_TraceLine (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs + gpGlobals->v_right * 16,
                   VecBModelOrigin (pBModel), dont_ignore_monsters, ignore_glass, pBot->pEdict->v.pContainingEntity, &tr2);

   // are the two hit points inside the BModel's boundaries ?
   if ((tr.vecEndPos.x > pBModel->v.absmin.x) && (tr.vecEndPos.x < pBModel->v.absmax.x)
       && (tr.vecEndPos.y > pBModel->v.absmin.y) && (tr.vecEndPos.y < pBModel->v.absmax.y)
       && (tr.vecEndPos.z > pBModel->v.absmin.z) && (tr.vecEndPos.z < pBModel->v.absmax.z)
       && (tr2.vecEndPos.x > pBModel->v.absmin.x) && (tr2.vecEndPos.x < pBModel->v.absmax.x)
       && (tr2.vecEndPos.y > pBModel->v.absmin.y) && (tr2.vecEndPos.y < pBModel->v.absmax.y)
       && (tr2.vecEndPos.z > pBModel->v.absmin.z) && (tr2.vecEndPos.z < pBModel->v.absmax.z))
      return TRUE; // line of sight is excellent

   // else is the left one inside the BModel's boundaries ?
   else if ((tr.vecEndPos.x > pBModel->v.absmin.x) && (tr.vecEndPos.x < pBModel->v.absmax.x)
            && (tr.vecEndPos.y > pBModel->v.absmin.y) && (tr.vecEndPos.y < pBModel->v.absmax.y)
            && (tr.vecEndPos.z > pBModel->v.absmin.z) && (tr.vecEndPos.z < pBModel->v.absmax.z))
      return TRUE; // line of sight is valid, though bot might want to strafe left to see better

   // else is the right one inside the BModel's boundaries ?
   else if ((tr2.vecEndPos.x > pBModel->v.absmin.x) && (tr2.vecEndPos.x < pBModel->v.absmax.x)
            && (tr2.vecEndPos.y > pBModel->v.absmin.y) && (tr2.vecEndPos.y < pBModel->v.absmax.y)
            && (tr2.vecEndPos.z > pBModel->v.absmin.z) && (tr2.vecEndPos.z < pBModel->v.absmax.z))
      return TRUE; // line of sight is valid, though bot might want to strafe right to see better

   return FALSE; // line of sight is not established
}


Vector BotGetIdealAimVector (bot_t *pBot, edict_t *pPlayer)
{
   TraceResult tr;

   if ((pBot->pEdict == NULL) || (pPlayer == NULL))
      return (Vector (0, 0, 0)); // reliability check

   UTIL_MakeVectors (pBot->pEdict->v.v_angle); // build base vectors in pEdict v_angle
   Vector vecLookerHead = pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs; // look from eyes position
   Vector vecPlayerHead = pPlayer->v.origin + pPlayer->v.view_ofs; // look at player's head
   Vector vecPlayerWaist = pPlayer->v.origin; // look at player's waist
   Vector vecPlayerTopHead = pPlayer->v.origin + pPlayer->v.view_ofs + Vector (0, 0, 3); // top of head
   Vector vecPlayerLArm = pPlayer->v.origin + gpGlobals->v_right * 12; // look at his left arm
   Vector vecPlayerRArm = pPlayer->v.origin - gpGlobals->v_right * 12; // look at his right arm
   Vector vecPlayerFeet; // look at player's feet

   if ((pPlayer->v.button & IN_DUCK) == IN_DUCK)
      vecPlayerFeet = pPlayer->v.origin + Vector (0, 0, -12); // player ducking, feet are higher
   else
      vecPlayerFeet = pPlayer->v.origin + Vector (0, 0, -28); // feet normal position

   // don't look through water
   if (((UTIL_PointContents (vecPlayerHead) == CONTENTS_WATER)
        && (UTIL_PointContents (vecPlayerFeet) == CONTENTS_WATER))
       != (UTIL_PointContents (vecLookerHead) == CONTENTS_WATER))
      return (Vector (0, 0, 0));

   // only target non human-like enemies at their origin
   if (!((pPlayer->v.flags & FL_CLIENT) || (pPlayer->v.flags & FL_FAKECLIENT)))
   {
      UTIL_TraceLine (vecLookerHead, vecPlayerWaist, ignore_monsters, ignore_glass, pBot->pEdict->v.pContainingEntity, &tr);

      if (tr.flFraction == 1.0)
         return (vecPlayerWaist); // line established to entity's origin
      else
         return (Vector (0, 0, 0)); // line of sight is not valid, bot can't see entity.
   }

   // first check at head level
   UTIL_TraceLine (vecLookerHead, vecPlayerHead, dont_ignore_monsters, ignore_glass, pBot->pEdict->v.pContainingEntity, &tr);
   if ((tr.flFraction > 0.5) && (strcmp (STRING (tr.pHit->v.classname), "player") == 0))
      return (vecPlayerHead); // line established to player's head

   // second check at waist level
   UTIL_TraceLine (vecLookerHead, vecPlayerWaist, dont_ignore_monsters, ignore_glass, pBot->pEdict->v.pContainingEntity, &tr);
   if ((tr.flFraction > 0.5) && (strcmp (STRING (tr.pHit->v.classname), "player") == 0))
      return (vecPlayerWaist); // line established to player's waist

   // third check at top of head level
   UTIL_TraceLine (vecLookerHead, vecPlayerTopHead, dont_ignore_monsters, ignore_glass, pBot->pEdict->v.pContainingEntity, &tr);
   if ((tr.flFraction > 0.5) && (strcmp (STRING (tr.pHit->v.classname), "player") == 0))
      return (vecPlayerTopHead); // line established to player's top of head

   // fourth check at left arm level
   UTIL_TraceLine (vecLookerHead, vecPlayerLArm, dont_ignore_monsters, ignore_glass, pBot->pEdict->v.pContainingEntity, &tr);
   if ((tr.flFraction > 0.5) && (strcmp (STRING (tr.pHit->v.classname), "player") == 0))
   {
      pBot->BotMove.f_straferight_time = gpGlobals->time + 0.2; // strafe right
      return (vecPlayerLArm); // line of sight is valid, but bot wants to strafe right to see better
   }

   // fifth check at right arm level
   UTIL_TraceLine (vecLookerHead, vecPlayerRArm, dont_ignore_monsters, ignore_glass, pBot->pEdict->v.pContainingEntity, &tr);
   if ((tr.flFraction > 0.5) && (strcmp (STRING (tr.pHit->v.classname), "player") == 0))
   {
      pBot->BotMove.f_strafeleft_time = gpGlobals->time + 0.2; // strafe left
      return (vecPlayerRArm); // line of sight is valid, but bot wants to strafe left to see better
   }

   // last check at feet level
   UTIL_TraceLine (vecLookerHead, vecPlayerFeet, dont_ignore_monsters, ignore_glass, pBot->pEdict->v.pContainingEntity, &tr);
   if ((tr.flFraction > 0.5) && (strcmp (STRING (tr.pHit->v.classname), "player") == 0))
      return (vecPlayerFeet); // line established to player's feet

   return (Vector (0, 0, 0)); // line of sight is not valid, bot can't see pPlayer.
}


void BotPointGun (bot_t *pBot, Vector v_target_angles)
{
   if (pBot->pEdict == NULL)
      return; // reliability check

   // offset target angle at some time interval
   if (pBot->f_aim_adjust_time < gpGlobals->time)
   {
      Vector v_deviation = UTIL_WrapAngles (v_target_angles - pBot->pEdict->v.v_angle);

      // if bot is turning at a corner
      if (pBot->f_turncorner_time > gpGlobals->time)
         v_deviation.y = v_deviation.y * 0.50; // offset yaw at 50 percent of the angle

      // else if bot is aiming at something
      else if (pBot->pBotEnemy != NULL)
         v_deviation.y = v_deviation.y * 0.25; // offset yaw at 25 percent of the angle

      // else bot must be just aimlessly wandering around
      else
         v_deviation.y = v_deviation.y * 0.10; // offset yaw at 10 percent of the angle

      if (v_deviation.x < 0)
         v_deviation.x = v_deviation.x + v_deviation.y / 10; // influence on x angle
      else
         v_deviation.x = v_deviation.x - v_deviation.y / 10; // influence on x angle

      // move the aim cursor
      BotSetViewAngles (pBot, pBot->pEdict->v.v_angle + v_deviation);

      // set the body angles to point the gun correctly
      pBot->pEdict->v.angles.x = UTIL_WrapAngle (pBot->pEdict->v.v_angle.x / 3);
      pBot->pEdict->v.angles.y = UTIL_WrapAngle (pBot->pEdict->v.v_angle.y);
      pBot->pEdict->v.angles.z = 0;

      // adjust the view angles to aim correctly (MUST be after body v.angles stuff)
      pBot->pEdict->v.v_angle.x = UTIL_WrapAngle (-pBot->pEdict->v.v_angle.x);
      BotSetIdealYaw (pBot, pBot->pEdict->v.v_angle.y); // make body face eye's direction

      // next aim adjustment in fraction second (based on skill)
      pBot->f_aim_adjust_time = gpGlobals->time + (6 - pBot->bot_skill) / 20;
   }
}


bool BotCanCampNearHere (bot_t *pBot, Vector v_here)
{
   float distance = 0, prev_distance = 0, prev_prev_distance = 0, angle, interesting_angles[72];
   int index, angles_count = 0;
   Vector v_prev_hitpoint = v_here;
   TraceResult tr;

   if (pBot->pEdict == NULL)
      return FALSE; // reliability check

   if (!ENT_IS_ON_FLOOR (pBot->pEdict))
      return FALSE; // don't even think about it if bot is jumping

   angle = UTIL_WrapAngle (pBot->pEdict->v.v_angle.y); // initialize scan angle to bot's view angle

   // cycle through all players to find if a teammate is already camping near here
   for (index = 1; index <= gpGlobals->maxClients; index++)
   {
      edict_t *pPlayer = INDEXENT (index);

      if (!pPlayer || pPlayer->free || (pPlayer == pBot->pEdict))
         continue; // skip invalid players and skip self (i.e. this bot)

      if (!IsAlive (pPlayer))
         continue; // skip this player if not alive (i.e. dead or dying)

      if (b_observer_mode && !(pPlayer->v.flags & FL_FAKECLIENT))
         continue; // skip real players if in observer mode

      if (((pPlayer->v.button & IN_DUCK) == IN_DUCK) && ((pPlayer->v.origin - v_here).Length () < 1000))
         return FALSE; // give up if another player is already camping near here
   }

   // scan 360 degrees around here in 72 samples...
   for (index = 0; index < 72; index++)
   {
      angle = UTIL_WrapAngle (angle + 5); // pan the trace angle from left to right
      UTIL_MakeVectors (Vector (0, angle, 0)); // build base vectors in that direction

      // trace line slightly under eyes level
      UTIL_TraceLine (v_here + (pBot->pEdict->v.view_ofs / 2),
                      (v_here + (pBot->pEdict->v.view_ofs / 2)) + (gpGlobals->v_forward * 10000),
                      ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

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
         interesting_angles[angles_count] = UTIL_WrapAngle (angle - 5); // remember this angle
         angles_count++; // increment interesting angles count
      }

      v_prev_hitpoint = tr.vecEndPos; // rotate the remembered hit point
   }

   // okay, now we know which angles are candidates for determining a good camp point

   if ((angles_count <= 1) || (angles_count >= 72))
      return FALSE; // give up if none found

   angle = UTIL_WrapAngle (interesting_angles[RANDOM_LONG (0, angles_count - 1)]); // choose one
   UTIL_MakeVectors (Vector (0, angle, 0)); // build base vectors in that direction

   // trace line slightly under eyes level
   UTIL_TraceLine (v_here + (pBot->pEdict->v.view_ofs / 2),
                   (v_here + (pBot->pEdict->v.view_ofs / 2)) + (gpGlobals->v_forward * 10000),
                   ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

   // assign bot this camp point
   pBot->v_place_to_keep = (v_here + (pBot->pEdict->v.view_ofs / 2)) + (gpGlobals->v_forward * ((10000 * tr.flFraction) - 40));
   pBot->f_place_time = gpGlobals->time; // remember when we last saw the place to keep
   pBot->f_camp_time = gpGlobals->time + (6 - pBot->bot_skill) * RANDOM_FLOAT (10, 20); // make him remember he is camping
   pBot->f_reload_time = gpGlobals->time + RANDOM_LONG (1.5, 3.0); // switch to best weapon for the job

   return TRUE; // bot found a camp spot next to v_here
}
