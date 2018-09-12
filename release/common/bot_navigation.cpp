// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// bot_navigation.cpp
//

// FIXME: some of the stuff in here is PRETTY OLD and comes straight from my first waypointless
// attempts. Actually the new bot navigation is not completely finished yet.

#include "racc.h"


void BotLookAt (player_t *pPlayer, Vector v_location)
{
   // this function is a helper to set the pPlayer bot's ideal angles so as to face the vector
   // location passed in with v_location. Instead of letting the bot look towards a certain
   // angle, using this function you can make it look at a particular location (no matter the
   // current angle it is facing). Like the one below, this function sets an IDEAL angle. It does
   // does NOT make the bot actually turn its head or point its gun ; this is the job of the
   // BotPointGun() function, which is called automatically at the end of each frame.

   BotSetIdealAngles (pPlayer, VecToAngles (v_location - pPlayer->v_eyeposition));
   return; // done
}


void BotSetIdealAngles (player_t *pPlayer, Vector ideal_angles)
{
   // this function sets the angles at which the bot wants to look. You don't have to
   // worry about changing the view angles of the bot directly, the bot turns to the
   // right direction by itself in a manner depending of the situation (e.g combat aim,
   // wandering aim...) with the BotPointGun() function. You can call this one as many
   // times you want each frame, as it is just about setting IDEAL angles. Note the use
   // of WrapAngle() to keep the angles in bounds and prevent overflows.

   // this function is part of the fakeclient aim bug fix. We need to revert the x component.

   pPlayer->Bot.BotHand.ideal_angles.x = -WrapAngle (ideal_angles.x); // st00pid engine bug !
   pPlayer->Bot.BotHand.ideal_angles.y = WrapAngle (ideal_angles.y);
   pPlayer->Bot.BotHand.ideal_angles.z = WrapAngle (ideal_angles.z);
   return; // done
}


void BotSetIdealPitch (player_t *pPlayer, float ideal_pitch)
{
   // this function is part of the fakeclient aim bug fix. We need to revert the x component.

   pPlayer->Bot.BotHand.ideal_angles.x = -WrapAngle (ideal_pitch); // st00pid engine bug !
   return; // done
}


void BotSetIdealYaw (player_t *pPlayer, float ideal_yaw)
{
   // this function is part of the fakeclient aim bug fix.

   pPlayer->Bot.BotHand.ideal_angles.y = WrapAngle (ideal_yaw);
   return; // done
}


void BotAddIdealPitch (player_t *pPlayer, float pitch_to_add)
{
   // this function is part of the fakeclient aim bug fix. We need to revert the x component.

   pPlayer->Bot.BotHand.ideal_angles.x = -WrapAngle (pPlayer->Bot.BotHand.ideal_angles.x + pitch_to_add);
   return; // done
}


void BotAddIdealYaw (player_t *pPlayer, float yaw_to_add)
{
   // this function is part of the fakeclient aim bug fix.

   pPlayer->Bot.BotHand.ideal_angles.y = WrapAngle (pPlayer->Bot.BotHand.ideal_angles.y + yaw_to_add);
   return; // done
}


void BotPointGun (player_t *pPlayer)
{
   // this function is called every frame for every bot. Its purpose is to make the bot
   // move its crosshair to the direction where it wants to look. There is some kind of
   // filtering for the view, to make it human-like.

   // OMG, .weirdest .engine .bug .ever here - Thanks to botman and all the people at botman's
   // forums for helping us together setting things straight once for all !!!

   static float speed, turn_skill, da_deadly_math; // speed : 0.1 - 1
   static Vector v_deviation;

   if (DebugLevel.hand_disabled)
      return; // return if we don't want the AI to act and use things

   v_deviation = WrapAngles (pPlayer->Bot.BotHand.ideal_angles - pPlayer->v_angle);
   turn_skill = pPlayer->Bot.pProfile->skill * 2;

   speed = 0.5 * sqrt (v_deviation.x * v_deviation.x + v_deviation.y * v_deviation.y); // pythagores
   if (speed < 0.5)
      speed = 0.5; // lower bound (don't want a too slow aiming)
   if (speed > 1.4)
      speed = 1.4; // upper bound (don't want superman-style aiming either)

   if (!FNullEnt (pPlayer->Bot.BotEnemy.pEdict))
      speed *= 2; // if bot is aiming at something, aim faster

   // thanks Tobias "Killaruna" Heimann and Johannes "@$3.1415rin" Lampel for this one
   speed = speed * 0.2 + (turn_skill - 1) / (20 / speed); // compute final speed factor
   da_deadly_math = exp (log (0.5 * speed) * 0.02 * server.msecval);

   pPlayer->Bot.BotHand.turn_speed.y = (pPlayer->Bot.BotHand.turn_speed.y * da_deadly_math
                                        + speed * v_deviation.y * (1 - da_deadly_math))
                                       * 0.02 * server.msecval;
   pPlayer->Bot.BotHand.turn_speed.x = (pPlayer->Bot.BotHand.turn_speed.x * da_deadly_math
                                        + speed * v_deviation.x * (1 - da_deadly_math))
                                       * 0.02 * server.msecval;

   // influence of y movement on x axis and vice versa (less influence than x on y since it's
   // easier and more natural for the bot to "move its mouse" horizontally than vertically)
   pPlayer->Bot.BotHand.turn_speed.x += pPlayer->Bot.BotHand.turn_speed.y / (1.5 * (1 + turn_skill));
   pPlayer->Bot.BotHand.turn_speed.y += pPlayer->Bot.BotHand.turn_speed.x / (1 + turn_skill);

   // don't allow the bot's aiming movement to get past the target point
   if ((fabs (v_deviation.x - pPlayer->Bot.BotHand.turn_speed.x) > 0)
       && (v_deviation.x * pPlayer->Bot.BotHand.turn_speed.x > 0))
      pPlayer->Bot.BotHand.turn_speed.x = v_deviation.x;
   if ((fabs (v_deviation.y - pPlayer->Bot.BotHand.turn_speed.y) > 0)
       && (v_deviation.y * pPlayer->Bot.BotHand.turn_speed.y > 0))
      pPlayer->Bot.BotHand.turn_speed.y = v_deviation.y;

   // move the aim cursor
   if (DebugLevel.is_inhumanturns)
      pPlayer->pEntity->v.v_angle = pPlayer->Bot.BotHand.ideal_angles;
   else
      pPlayer->pEntity->v.v_angle = WrapAngles (pPlayer->v_angle + pPlayer->Bot.BotHand.turn_speed); 

   // set the body angles to point the gun correctly
   pPlayer->pEntity->v.angles.x = -pPlayer->pEntity->v.v_angle.x / 3;
   pPlayer->pEntity->v.angles.y = pPlayer->pEntity->v.v_angle.y;
   pPlayer->pEntity->v.angles.z = 0;

   // if debug level is high, draw a line where the bot is looking and where it wants to look
   if (pPlayer->is_watched && (DebugLevel.hand > 1))
   {
      BuildReferential (pPlayer->Bot.BotHand.ideal_angles); // make base vectors for ideal angles
      UTIL_DrawLine (pPlayer->v_eyeposition,
                     pPlayer->v_eyeposition + referential.v_forward * 150,
                     1, 255, 0, 0); // ideal
      UTIL_DrawLine (pPlayer->v_eyeposition,
                     pPlayer->v_eyeposition + pPlayer->v_forward * 150,
                     1, 255, 255, 255); // current
   }

   return;
}


void BotUseHand (player_t *pPlayer)
{
   // the purpose of this function is to translate the data of the BotHand structure (timings
   // at which the bot has to perform some action - fire for 2 seconds, use a particular item for
   // 5 seconds, and so on) into the right input buttons to be passed to RunPlayerMove(), which
   // is the function that asks the engine to perform the actions of the fakeclient.

   bot_hand_t *BotHand;

   static unsigned long clear_mask = ~(INPUT_KEY_FIRE1 | INPUT_KEY_FIRE2 | INPUT_KEY_RELOAD
                                       | INPUT_KEY_DISPLAYSCORE | INPUT_KEY_USE
                                       | INPUT_KEY_SPRAY | INPUT_KEY_LIGHT);

   if (DebugLevel.hand_disabled)
      return; // return if we don't want the AI to do anything with its hands

   if (pPlayer->Bot.is_controlled)
      return; // if bot is bewitched, it doesn't "steer itself"

   BotHand = &pPlayer->Bot.BotHand; // quick access to bot legs

   // reset those of the bot's input keys that belong to the hand usage
   pPlayer->input_buttons &= clear_mask;

   // may the bot pull the primary trigger now ?
   if (BotHand->fire1_time > server.time)
      pPlayer->input_buttons |= INPUT_KEY_FIRE1; // fire the primary weapon rail

   // may the bot pull the secondary trigger now ?
   if (BotHand->fire2_time > server.time)
      pPlayer->input_buttons |= INPUT_KEY_FIRE2; // fire the secondary weapon rail

   // may the bot reload its weapon now ?
   if (BotHand->reload_time > server.time)
      pPlayer->input_buttons |= INPUT_KEY_RELOAD; // reload the gun

   // may the bot use something now ?
   if (BotHand->use_time > server.time)
      pPlayer->input_buttons |= INPUT_KEY_USE; // press the use key

   // may the bot display the scores now ?
   if (BotHand->displayscore_time > server.time)
      pPlayer->input_buttons |= INPUT_KEY_DISPLAYSCORE; // display the scores tab

   // may the bot spray something now ?
   if (BotHand->spray_time > server.time)
   {
      pPlayer->input_buttons |= INPUT_KEY_SPRAY; // spray logo
      BotHand->spray_time = server.time; // do this once since it's an instant command
   }

   // may the bot light something now ?
   if (BotHand->light_time > server.time)
   {
      pPlayer->input_buttons |= INPUT_KEY_LIGHT; // light the torch
      BotHand->light_time = server.time; // do this once since it's an instant command
   }

   return;
}


void BotMove (player_t *pPlayer)
{
   // the purpose of this function is to translate the data of the BotLegs structure (timings
   // at which the bot has to perform some movement - jump in 2 seconds, move forward for 5
   // seconds, and so on) into the right input buttons to be passed to RunPlayerMove(), which is
   // the function that asks the engine to perform the movement of the fakeclient. It also sets
   // the correct values for move_speed and strafe_speed which are parameters of RunPlayerMove().

   bot_legs_t *BotLegs;

   static unsigned long clear_mask = ~(INPUT_KEY_FORWARD | INPUT_KEY_BACKWARDS
                                       | INPUT_KEY_STRAFELEFT | INPUT_KEY_STRAFERIGHT
                                       | INPUT_KEY_JUMP | INPUT_KEY_DUCK | INPUT_KEY_WALK);

   if (DebugLevel.legs_disabled)
      return; // return if we don't want the AI to move

   if (pPlayer->Bot.is_controlled)
      return; // if bot is bewitched, it doesn't "steer itself"

   BotLegs = &pPlayer->Bot.BotLegs; // quick access to bot legs

   // reset those of the bot's input keys that belong to the legs movement
   pPlayer->input_buttons &= clear_mask;

   // may the bot jump now ?
   if ((BotLegs->jump_time < server.time) && (BotLegs->jump_time + 0.1 > server.time))
      pPlayer->input_buttons |= INPUT_KEY_JUMP; // jump

   // has the bot just jumped AND is bot skilled enough ?
   if ((BotLegs->jump_time + 0.1 < server.time) && (BotLegs->jump_time + 0.2 > server.time)
       && (pPlayer->Bot.pProfile->skill > 1))
      BotLegs->duck_time = server.time + 0.1; // duck while jumping

   // may the bot duck now ?
   if (BotLegs->duck_time > server.time)
      pPlayer->input_buttons |= INPUT_KEY_DUCK; // duck

   // may the bot safely strafe left now ?
   if ((BotLegs->strafeleft_time > server.time)
       && !(pPlayer->Bot.BotBody.hit_state & OBSTACLE_LEFT_FALL))
   {
      // only press this key when the opposite key has been released for enough time long
// FIXME: this fucks everything up, go figure why.
//      if (BotLegs->straferight_time + 0.2 < server.time)
      {
         pPlayer->input_buttons |= INPUT_KEY_STRAFELEFT;
      }
   }

   // else may the bot safely strafe right now ?
   else if ((BotLegs->straferight_time > server.time)
            && !(pPlayer->Bot.BotBody.hit_state & OBSTACLE_RIGHT_FALL))
   {
      // only press this key when the opposite key has been released for enough time long
//      if (BotLegs->strafeleft_time + 0.2 < server.time)
      {
         pPlayer->input_buttons |= INPUT_KEY_STRAFERIGHT;
      }
   }

   // may the bot move backwards now ?
   if ((BotLegs->backwards_time > server.time)
       || BotLegs->emergency_walkback)
   {
      // only press this key when the opposite key has been released for enough time long
//      if (BotLegs->forward_time + 0.2 < server.time)
      {
         pPlayer->input_buttons |= INPUT_KEY_BACKWARDS;
      }
   }

   // else may the bot move forward now ?
   else if (BotLegs->forward_time > server.time)
   {
      // only press this key when the opposite key has been released for enough time long
//      if (BotLegs->backwards_time + 0.2 < server.time)
      {
         pPlayer->input_buttons |= INPUT_KEY_FORWARD;
      }
   }

   // may the bot walk now ?
   if (BotLegs->walk_time > server.time)
      pPlayer->input_buttons |= INPUT_KEY_WALK;

   return;
}


void BotOnLadder (player_t *pPlayer)
{
   test_result_t tr;

   // if the bot is skilled enough, it will duck on ladders...
   if (pPlayer->Bot.pProfile->skill > 3)
      pPlayer->Bot.BotLegs.duck_time = server.time + 0.2;

   // check if bot JUST got on the ladder...
   if (pPlayer->Bot.end_use_transport_time + 1.0 < server.time)
      pPlayer->Bot.start_use_transport_time = server.time;

   // has the bot NOT identified the ladder yet ?
   if (FNullEnt (pPlayer->Bot.pTransportEntity))
   {
      pPlayer->Bot.transport_direction = TRANSPORT_UNKNOWN; // reset ladder direction
      BotFindTransportEntity (pPlayer, REACHABILITY_LADDER); // find the involved ladder
      return; // and return
   }

   if ((pPlayer->Bot.v_reach_point - pPlayer->v_origin).Length2D () < 60)
   {
      Vector bot_angles = VecToAngles (pPlayer->Bot.v_reach_point - pPlayer->v_origin);
      BotSetIdealYaw (pPlayer, bot_angles.y); // face the middle of the ladder...
   }

   // moves the bot up or down a ladder.  if the bot can't move
   // (i.e. get's stuck with someone else on ladder), the bot will
   // change directions and go the other way on the ladder.

   // is the bot currently going up ?
   if (pPlayer->Bot.transport_direction == TRANSPORT_FORWARD)
   {
      BotSetIdealPitch (pPlayer, 80); // look upwards

      // has the bot climbed the whole ladder ?
      if (pPlayer->v_origin.z > pPlayer->Bot.pTransportEntity->v.absmax.z)
      {
         BotSetIdealPitch (pPlayer, 0); // look at flat again

         // is the bot stuck ?
         if (pPlayer->Bot.is_stuck)
         {
            pPlayer->Bot.BotLegs.duck_time = server.time + 0.2; // duck

            // is there a floor on the left ?
            tr = PlayerTestLine (pPlayer,
                                 pPlayer->v_eyeposition,
                                 pPlayer->v_eyeposition + Vector (0, 0, -100) - Vector (pPlayer->v_right.x, pPlayer->v_right.y, 0) * 100);
            if ((tr.fraction < 1.0) && (AngleOfVectors (tr.v_normal, Vector (0, 0, 1)) < 45))
               pPlayer->Bot.BotLegs.strafeleft_time = server.time + 0.5; // strafe left to reach it

            // else is there a floor on the right ?
            tr = PlayerTestLine (pPlayer,
                                 pPlayer->v_eyeposition,
                                 pPlayer->v_eyeposition + Vector (0, 0, -100) + Vector (pPlayer->v_right.x, pPlayer->v_right.y, 0) * 100);
            if ((tr.fraction < 1.0) && (AngleOfVectors (tr.v_normal, Vector (0, 0, 1)) < 45))
               pPlayer->Bot.BotLegs.straferight_time = server.time + 0.5; // strafe right to reach it
         }
      }

      // else check if the bot is stuck...
      else if (pPlayer->Bot.is_stuck)
      {
         BotSetIdealPitch (pPlayer, -80); // look downwards (change directions)
         pPlayer->Bot.transport_direction = TRANSPORT_BACKWARDS;
      }
   }

   // else is the bot currently going down ?
   else if (pPlayer->Bot.transport_direction == TRANSPORT_BACKWARDS)
   {
      BotSetIdealPitch (pPlayer, -80); // look downwards

      // has the bot climbed the whole ladder ?
      if (fabs (pPlayer->pEntity->v.absmin.z - pPlayer->Bot.pTransportEntity->v.absmin.z) < 10)
      {
         BotSetIdealPitch (pPlayer, 0); // look at flat again

         // is the bot stuck ?
         if (pPlayer->Bot.is_stuck)
         {
            pPlayer->Bot.BotLegs.duck_time = server.time + 0.2; // duck

            // is there a floor on the left ?
            tr = PlayerTestLine (pPlayer,
                                 pPlayer->v_eyeposition,
                                 pPlayer->v_eyeposition + Vector (0, 0, -100) - Vector (pPlayer->v_right.x, pPlayer->v_right.y, 0) * 100);
            if ((tr.fraction < 1.0) && (AngleOfVectors (tr.v_normal, Vector (0, 0, 1)) < 45))
               pPlayer->Bot.BotLegs.strafeleft_time = server.time + 0.5; // strafe left to reach it

            // else is there a floor on the right ?
            tr = PlayerTestLine (pPlayer,
                                 pPlayer->v_eyeposition,
                                 pPlayer->v_eyeposition + Vector (0, 0, -100) + Vector (pPlayer->v_right.x, pPlayer->v_right.y, 0) * 100);
            if ((tr.fraction < 1.0) && (AngleOfVectors (tr.v_normal, Vector (0, 0, 1)) < 45))
               pPlayer->Bot.BotLegs.straferight_time = server.time + 0.5; // strafe right to reach it
         }
      }

      // else check if the bot is stuck...
      else if (pPlayer->Bot.is_stuck)
      {
         BotSetIdealPitch (pPlayer, 80); // look upwards (change directions)
         pPlayer->Bot.transport_direction = TRANSPORT_FORWARD;
      }
   }

   // else the bot hasn't picked a direction yet, try going up...
   else
   {
      BotSetIdealPitch (pPlayer, 80); // look upwards
      pPlayer->Bot.transport_direction = TRANSPORT_FORWARD;
   }

   pPlayer->Bot.end_use_transport_time = server.time;

   // check if bot has been on a ladder for more than 5 seconds...
   if ((pPlayer->Bot.start_use_transport_time > 0.0)
       && (pPlayer->Bot.start_use_transport_time + 5.0 < server.time))
   {
      pPlayer->Bot.BotLegs.jump_time = server.time; // jump to unstuck from ladder...
      pPlayer->Bot.finditem_time = server.time + 10.0; // don't look for items for 10 seconds
      pPlayer->Bot.start_use_transport_time = 0.0;  // reset start ladder use time
   }

   return;
}


void BotUnderWater (player_t *pPlayer)
{
   // handle movements under water. right now, just try to keep from drowning by swimming up
   // towards the surface and look to see if there is a surface the bot can jump up onto to
   // get out of the water. bots DON'T like water!

   test_result_t tr;

   BotSetIdealPitch (pPlayer, 60); // look upwards to swim up towards the surface

   // look from eye position straight forward (remember: the bot is looking
   // upwards at a 60 degree angle so TraceLine will go out and up...

   tr = PlayerTestLine (pPlayer,
                        pPlayer->v_eyeposition,
                        pPlayer->v_eyeposition + pPlayer->v_forward * 90);

   // did the trace NOT hit anything AND is the trace end point in open space ?
   if ((tr.fraction == 1.0) && (POINT_CONTENTS (tr.v_endposition) == CONTENTS_EMPTY))
   {
      // ok so far, we are at the surface of the water, continue...

      // trace from the previous end point straight down...
      tr = PlayerTestLine (pPlayer,
                           tr.v_endposition,
                           tr.v_endposition + Vector (0, 0, -90));

      // did the trace hit something AND is the trace end point NOT in open space ?
      if ((tr.fraction < 1.0) && (POINT_CONTENTS (tr.v_endposition) != CONTENTS_WATER))
         pPlayer->Bot.BotLegs.jump_time = server.time; // assume it's land, we can jump
   }

   return;
}


void BotUseLift (player_t *pPlayer)
{
   // check if lift has started moving...
   if ((pPlayer->environment == ENVIRONMENT_GROUND) && (pPlayer->v_velocity.z != 0)  && !pPlayer->Bot.is_lift_moving)
      pPlayer->Bot.is_lift_moving = TRUE;

   // else check if lift has stopped moving OR bot has waited too long for the lift to move...
   else if (((pPlayer->environment == ENVIRONMENT_GROUND) && (pPlayer->v_velocity.z == 0) && pPlayer->Bot.is_lift_moving)
            || ((pPlayer->Bot.interact_time + 2.0 < server.time) && !pPlayer->Bot.is_lift_moving))
   {
      pPlayer->Bot.is_interacting = FALSE; // clear use button flag
      pPlayer->Bot.reach_time = server.time; // get a new reach point as soon as now
      pPlayer->Bot.BotLegs.forward_time = server.time + 0.5; // run forward
   }
}


bool BotCanUseInteractives (player_t *pPlayer)
{
   edict_t *pent;
   edict_t *pButton;
   test_result_t tr;
   Vector v_entity_origin;
   bool found_button;

   // if bot has already found an interactive entity to toy with...
   if (!FNullEnt (pPlayer->Bot.pInteractiveEntity))
      return (TRUE); // return; bot has already something to do

   // if bot has just interacted with something in the past seconds...
   if (pPlayer->Bot.interact_time + 2.0 > server.time)
      return (FALSE); // return; too early for bot to check again

   pPlayer->Bot.pInteractiveEntity = NULL; // free bot's memory about interactive entities

   // check for interactive entities that are nearby
   pent = NULL;
   while ((pent = FindEntityInSphere (pent, pPlayer->v_eyeposition, 400)) != NULL)
   {
      v_entity_origin = OriginOf (pent); // get a quick access to this entity's origin

      // check if that entity is a door in his view cone
      if ((strcmp ("door_origin", STRING (pent->v.classname)) == 0)
          && IsInFieldOfView (pPlayer->pEntity, v_entity_origin))
      {
         // trace a line from bot's waist to door origin entity...
         tr = PlayerTestLine (pPlayer,
                              pPlayer->v_origin,
                              OriginOf (pent));

         // check if traced all the way up to the door, either it is closed OR open
         // AND bot has not used a door for at least 15 seconds
         if (((strncmp ("func_door", STRING (tr.pHit->v.classname), 9) == 0) || (tr.fraction == 1.0))
             && (pPlayer->Bot.interact_time + 15.0 < server.time))
         {
            found_button = FALSE;

            // check for buttons near this door
            pButton = NULL;
            while ((pButton = FindEntityInSphere (pButton, v_entity_origin, 300)) != NULL)
            {
               // if this button seems to control a door
               if ((strcmp ("func_button", STRING (pButton->v.classname)) == 0)
                   && (strncmp ("cam", STRING (pButton->v.target), 3) != 0)
                   && (strcmp ("Locked", STRING (pButton->v.targetname)) == 0))
                  found_button = TRUE; // remember it
            }

            // is there a button that controls this door nearby ?
            if (found_button)
               continue; // skip this door: bot has to press a button first

            pPlayer->Bot.pInteractiveEntity = pent; // save door entity
         }
      }

      // else check if that entity is a button in his view cone
      else if ((strcmp ("func_button", STRING (pent->v.classname)) == 0)
               && IsInFieldOfView (pPlayer->pEntity, v_entity_origin))
      {
         // trace a line from bot's waist to button origin entity...
         tr = PlayerTestLine (pPlayer,
                              pPlayer->v_origin,
                              v_entity_origin);

         // check if traced all the way up to the button
         // AND bot has not used a button for at least 10 seconds
         if ((strcmp ("func_button", STRING (tr.pHit->v.classname)) == 0)
             && (pPlayer->Bot.interact_time + 10.0 < server.time))
            pPlayer->Bot.pInteractiveEntity = pent; // save button entity
      }
   }

   // if at this point bot has remembered no entity in particular
   if (FNullEnt (pPlayer->Bot.pInteractiveEntity))
      return (FALSE); // bot didn't found anything togglable on his way

   return (TRUE); // seems like bot found something togglable on his way
}


void BotInteractWithWorld (player_t *pPlayer)
{
   Vector v_goal;
   float distance;

   pPlayer->Bot.is_interacting = FALSE; // reset any interaction flag
   pPlayer->Bot.interact_time = server.time; // save last interaction time

   pPlayer->Bot.BotLegs.forward_time = server.time + 0.5; // run forward to interactive entity

   // reliability check: has the bot goal been reset ?
   if (FNullEnt (pPlayer->Bot.pInteractiveEntity))
      return; // give up

   v_goal = OriginOf (pPlayer->Bot.pInteractiveEntity);
   distance = (v_goal - pPlayer->v_origin).Length ();

   // see how far our bot is from its goal
   if (distance < 100)
      pPlayer->Bot.BotLegs.walk_time = server.time + 0.2; // slow down while getting closer

   // is bot far enough from entity AND path to entity is blocked (entity is no more visible) ?
   if ((distance > 100) && (BotCanSeeOfEntity (pPlayer, pPlayer->Bot.pInteractiveEntity) == g_vecZero))
   {
      pPlayer->Bot.pInteractiveEntity = NULL; // reset interactive entity
      return; // give up: interactive entity is no more visible
   }

   // is the bot about to fall ?
   if ((pPlayer->Bot.fallcheck_time < server.time) && (pPlayer->Bot.BotBody.hit_state & OBSTACLE_FRONT_FALL))
   {
      pPlayer->Bot.pInteractiveEntity = NULL; // reset interactive entity
      BotTurnAtFall (pPlayer); // try to avoid falling
      return; // give up: bot can't reach interactive entity safely
   }

   // face the interactive entity
   BotLookAt (pPlayer, v_goal);

   // check for corners and turn there if needed
   BotCheckForCorners (pPlayer);

   // if bot is not moving fast enough AND bot is close to entity, bot may have reached its goal
   if ((pPlayer->v_velocity.Length2D () < 10) && (distance < 50))
   {
      pPlayer->Bot.BotHand.use_time = server.time + 0.2; // activate the entity in case it is needed
      pPlayer->Bot.is_interacting = TRUE; // set interaction flag
      pPlayer->Bot.is_lift_moving = FALSE; // set this in case bot would stand on a lift
      pPlayer->Bot.pInteractiveEntity = NULL; // reset interactive entity
      pPlayer->Bot.BotLegs.backwards_time = server.time + RandomFloat (0.1, 0.4); // step back
   }

   return;
}


void BotTurnAtFall (player_t *pPlayer)
{
   Vector Normal;
   float Y, Y1, Y2, D1, D2, Z;

   // Find the normal vector from the trace result.  The normal vector will
   // be a vector that is perpendicular to the surface from the TraceResult.
   // Don't revert Normal since the edge plane is seen by the 'other side'

   Normal = WrapAngles360 (VecToAngles (pPlayer->Bot.BotBody.v_fall_plane_normal));

   // Since the bot keeps it's view angle in -180 < x < 180 degrees format,
   // and since TraceResults are 0 < x < 360, we convert the bot's view
   // angle (yaw) to the same format as TraceResult.

   Y = WrapAngle360 (pPlayer->v_angle.y + 180);

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
      Z = WrapAngle (Y1 - RandomFloat (0.0, 10.0)); // avoid exact angle
   else
      Z = WrapAngle (Y2 + RandomFloat (0.0, 10.0)); // avoid exact angle

   BotSetIdealYaw (pPlayer, Z); // set the direction to head off into...
   pPlayer->Bot.reach_time = server.time + 1.0; // don't try to reach point for one sec
   pPlayer->Bot.finditem_time = server.time + 1.0; // don't try to reach items for one sec
   pPlayer->Bot.fallcheck_time = server.time + 2.0; // give bot time to turn
}


bool BotCantSeeForward (player_t *pPlayer)
{
   // use a TraceLine to determine if bot is facing a wall

   // trace from the bot's eyes straight forward and return whether we hit something or not
   return (PlayerTestLine (pPlayer,
                           pPlayer->v_eyeposition,
                           pPlayer->v_eyeposition + pPlayer->v_forward * 150).fraction < 1.0);
}


void BotTurnTowardsDirection (player_t *pPlayer, char direction)
{
   static short start_index, index, end_index, angles_count;
   static fov_line_t *peak[3]; // left, peak, right
   static float interesting_angles[BOT_FOV_WIDTH];

   // given the sound direction, figure out the portion of the FOV array to search
   if (direction & DIRECTION_LEFT)
   {
      start_index = 0; // left
      if (direction & DIRECTION_FRONT)
         end_index = BOT_FOV_WIDTH / 2; // left AND front
      else
         end_index = BOT_FOV_WIDTH / 4; // totally left
   }
   else if (direction & DIRECTION_RIGHT)
   {
      end_index = BOT_FOV_WIDTH; // right
      if (direction & DIRECTION_FRONT)
         start_index = BOT_FOV_WIDTH / 2; // right AND front
      else
         start_index = 3 * BOT_FOV_WIDTH / 4; // totally right
   }
   else
   {
      start_index = BOT_FOV_WIDTH / 4; // front
      end_index = 3 * BOT_FOV_WIDTH / 4;
   }

   angles_count = 0; // reset good watch angles candidates

   // scan the FOV graph array in bot's eyes...
   for (index = start_index; index < end_index; index++)
   {
      // fill the peak candidate array, handle start and end of FOV as well
      if (index > 0)
         peak[0] = &pPlayer->Bot.BotEyes.BotFOV[index - 1];
      else
         peak[0] = &pPlayer->Bot.BotEyes.BotFOV[index];
      peak[1] = &pPlayer->Bot.BotEyes.BotFOV[index];
      if (index < BOT_FOV_WIDTH - 1)
         peak[2] = &pPlayer->Bot.BotEyes.BotFOV[index + 1];
      else
         peak[2] = &pPlayer->Bot.BotEyes.BotFOV[index];

      // have we a discontinuousity in the distances (i.e an incoming path) or a door ?
      if ((peak[1]->distance > 200)
          && (((peak[1]->distance > peak[0]->distance) && (peak[1]->distance > peak[2]->distance)
               && ((peak[1]->distance > peak[0]->distance + 100) || (peak[1]->distance > peak[2]->distance + 100)))
              || (strncmp (STRING (peak[1]->pHit->v.classname), "func_door", 9) == 0)))
      {
         interesting_angles[angles_count] = peak[1]->scan_angles.y; // remember this angle
         angles_count++; // increment interesting angles count
      }
   }

   // okay, now we know which angles are candidates for determining a good watch angle
   if ((angles_count > 0) && (angles_count < 72))
      BotSetIdealYaw (pPlayer, interesting_angles[RandomLong (0, angles_count - 1)]); // choose one
   else
      BotSetIdealYaw (pPlayer, RandomFloat (-180, 180)); // if no candidates, choose a totally random angle

   pPlayer->Bot.reach_time = server.time + 1.0; // don't try to reach point for one second

   return;
}


void BotRandomTurn (player_t *pPlayer)
{
   short index, angles_count;
   fov_line_t peak[72]; // left, peak, right
   float interesting_angles[72], angle;
   test_result_t tr;

   if (pPlayer->Bot.randomturn_time > server.time)
      return; // cancel if not time to yet (already in the process of random turning)

   angle = WrapAngle (pPlayer->v_angle.y); // initialize scan angle to bot's view angle

   // scan 360 degrees around here in 72 samples, 5 degree step each...
   for (index = 0; index < 72; index++)
   {
      angle = WrapAngle (angle + 5); // pan the trace angle from left to right
      BuildReferential (Vector (0, angle, 0)); // build base vectors at flat in that direction

      // trace line at eyes level forward
      tr = PlayerTestLine (pPlayer,
                           pPlayer->v_eyeposition,
                           pPlayer->v_eyeposition + (referential.v_forward * 10000));

      // fill the peak candidate array
      peak[index].distance = tr.fraction * 10000;
      peak[index].scan_angles = Vector (0, angle, 0);
      peak[index].pHit = tr.pHit;
   }

   // now that the array is filled, cycle again and count the interesting angles
   angles_count = 0;
   for (index = 0; index < 72; index++)
   {
      // have we a discontinuousity in the distances (i.e an incoming path) or a door ?
      if ((((peak[index].distance > peak[index > 0 ? index : 0].distance)
            && (peak[index].distance > peak[index < 72 ? index : 71].distance)
            && ((peak[index].distance > peak[index > 0 ? index : 0].distance + 100)
                || (peak[index].distance > peak[index < 72 ? index : 71].distance + 100)))
           || (strncmp (STRING (peak[index].pHit->v.classname), "func_door", 9) == 0))
          && (peak[index].distance > 200))
      {
         interesting_angles[angles_count] = WrapAngle (peak[index].scan_angles.y - 5); // remember this angle
         angles_count++; // increment interesting angles count
      }
   }

   // okay, now we know which angles are candidates for determining a good watch angle
   if ((angles_count > 0) && (angles_count < 72))
      BotSetIdealYaw (pPlayer, interesting_angles[RandomLong (0, angles_count - 1)]); // choose one
   else
      BotSetIdealYaw (pPlayer, RandomFloat (-180, 180)); // if no candidate, choose a totally random angle

   pPlayer->Bot.reach_time = server.time + 1.0; // don't try to reach point for one second
   if (pPlayer->Bot.getreachpoint_time < server.time + 1.0)
      pPlayer->Bot.getreachpoint_time = server.time + 1.0; // don't try to pick a new one either

   pPlayer->Bot.randomturn_time = server.time + RandomFloat (1.0, 2.0); // give bot time to turn
   return;
}


void BotFindTransportEntity (player_t *pPlayer, int transport_type)
{
   edict_t *pTransportEntity = NULL;
   float nearest_distance = 9999;
   float distance;
   char transport_classname[32];

   // figure out the classname of the entity to search for
   if (transport_type == REACHABILITY_LADDER)
   {
      pPlayer->Bot.transport_type = REACHABILITY_LADDER; // it's a ladder
      strcpy (transport_classname, "func_ladder");
   }
   else if (transport_type == REACHABILITY_ELEVATOR)
   {
      pPlayer->Bot.transport_type = REACHABILITY_ELEVATOR; // it's an elevator
      strcpy (transport_classname, "func_door");
   }
   else if (transport_type == REACHABILITY_PLATFORM)
   {
      pPlayer->Bot.transport_type = REACHABILITY_PLATFORM; // it's a bobbing platform
      strcpy (transport_classname, "func_train");
   }
   else if (transport_type == REACHABILITY_CONVEYOR)
   {
      pPlayer->Bot.transport_type = REACHABILITY_CONVEYOR; // it's a conveyor ribbon
      strcpy (transport_classname, "func_conveyor");
   }
   else if (transport_type == REACHABILITY_TRAIN)
   {
      pPlayer->Bot.transport_type = REACHABILITY_TRAIN; // it's a train
      strcpy (transport_classname, "func_tracktrain");
   }
   else
      TerminateOnError ("BotFindTransportEntity(): not a transport entity type: %d\n", transport_type);

   // now cycle through all entities of that type and find the closest one...
   pTransportEntity = NULL;
   while ((pTransportEntity = FindEntityByString (pTransportEntity, "classname", transport_classname)) != NULL)
   {
      // get the distance from bot to entity
      distance = (OriginOf (pTransportEntity) - pPlayer->v_origin).Length ();

      if (distance > 500)
         continue; // discard entity if it's not within bot's search range

      // see if this transport entity is the closest we've found so far...
      if (distance < nearest_distance)
      {
         nearest_distance = distance; // update minimum distance

         pPlayer->Bot.pTransportEntity = pTransportEntity; // save the transport entity
         pPlayer->Bot.v_reach_point = OriginOf (pTransportEntity); // tell bot to reach entity
         pPlayer->Bot.is_picking_item = TRUE; // priority is just the same as pickup items
      }
   }

   return;
}


void BotCamp (player_t *pPlayer)
{
   bool b_can_see_position;

   b_can_see_position = BotCanSeeThis (pPlayer, pPlayer->Bot.v_place_to_keep);

   // first check: is the place not visible for more than 3 seconds OR enemy in sight ?
   if ((!b_can_see_position && (pPlayer->Bot.place_time + 3 < server.time))
       || !FNullEnt (pPlayer->Bot.BotEnemy.pEdict))
   {
      pPlayer->Bot.v_place_to_keep = g_vecZero; // forget this place
      return; // don't camp anymore as long as this enemy is alive
   }

   // else is the place visible ?
   else if (b_can_see_position)
      pPlayer->Bot.place_time = server.time; // reset place time

   // how far is our place ?
   if ((pPlayer->Bot.v_place_to_keep - pPlayer->v_origin).Length () > 50)
   {
      // run to the position where the bot should be
      BotReachPosition (pPlayer, pPlayer->Bot.v_place_to_keep);

      // is bot about to hit something it can jump up ?
      if ((pPlayer->Bot.BotBody.hit_state & OBSTACLE_FRONT_LOWWALL)
          && (pPlayer->Bot.BotLegs.jump_time + 2.0 < server.time))
         pPlayer->Bot.BotLegs.jump_time = server.time; // jump up and move forward

      // else is it about to hit something it can duck under ?
      else if (pPlayer->Bot.BotBody.hit_state & OBSTACLE_FRONT_LOWCEILING)
      {
         // if debug mode is enabled...
         if (pPlayer->is_watched && (DebugLevel.legs > 0))
         {
            AIConsole_printf (CHANNEL_LEGS, 4, "Ducks because REACHING CAMP SPOT\n");
            UTIL_DrawLine (pPlayer->v_origin, pPlayer->Bot.v_place_to_keep, 10, 255, 255, 255);
         }

         pPlayer->Bot.BotLegs.duck_time = server.time + RandomFloat (0.5, 1.5); // duck & go
      }
   }

   // else the bot is arrived where it wants to camp
   else
   {
      BotSetIdealPitch (pPlayer, 0); // look at flat

      // can the bot duck safely without losing visibility ?
      if (!(pPlayer->Bot.BotBody.hit_state & (OBSTACLE_FRONT_LOWWALL | OBSTACLE_FRONT_WALL)))
      {
         // if debug mode is enabled...
         if (pPlayer->is_watched && (DebugLevel.legs > 0))
            AIConsole_printf (CHANNEL_LEGS, 4, "Ducks because CAMPING\n");

         pPlayer->Bot.BotLegs.duck_time = server.time + 0.5; // duck to improve weapon accuracy
      }

      // if time to look around OR bot can't see forward
      if ((pPlayer->Bot.randomturn_time < server.time) || BotCantSeeForward (pPlayer))
      {
         BotRandomTurn (pPlayer); // randomly turnaround and repeat the same thing...
         pPlayer->Bot.randomturn_time = server.time + RandomFloat (0.5, 15.0); // in a few secs
      }
   }

   return;
}


void BotCheckForCorners (player_t *pPlayer)
{
   // this function is mostly a reflex action ; its purpose is to make the bot react motionally
   // by strafing and/or jumping to potentially threatening corners on his sides, where players
   // could ambush. We do this by firing TraceLines left and right at a certain distance ahead
   // of the bot, depending on its velocity. If a corner is found, the bot adjusts its movement
   // (strafes and jumps) in order to eventually surprise a potential ambusher.

   bot_legs_t *pBotLegs;
   test_result_t tr_near;
   test_result_t tr_far;
   float distance;
   bool can_turn_left = FALSE;
   bool can_turn_right = FALSE;
   bool can_go_ahead = FALSE;

   pBotLegs = &pPlayer->Bot.BotLegs; // quick access to bot's legs

   if (pPlayer->Bot.cornercheck_time > server.time)
      return; // cancel if not time to

   if (pBotLegs->forward_time < server.time)
      return; // don't check for corners if the bot isn't in movement

   pPlayer->Bot.cornercheck_time = server.time + 0.10; // next check in 100 ms unless specified

   distance = 0.6 * pPlayer->v_velocity.Length2D (); // distance of check depends on speed

   // make sure we are tracing inside the map...
   if (pPlayer->Bot.BotEyes.BotFOV[BOT_FOV_WIDTH / 2].distance > 7 * distance)
      can_go_ahead = TRUE; // there's still a long way ahead
   else if (pPlayer->Bot.BotEyes.BotFOV[BOT_FOV_WIDTH / 2].distance < distance)
      distance *= 0.99; // ensure left and right traces will have some margin on the sides

   // check for corners on the LEFT

   // do a trace from 100 units in front of the bot's eyes left...
   tr_near = PlayerTestLine (pPlayer,
                             pPlayer->v_origin + pPlayer->v_forward * distance * 0.75,
                             pPlayer->v_origin + pPlayer->v_forward * distance * 0.75 - pPlayer->v_right * 1000);

   // do a trace from 150 units in front of the bot's eyes left...
   tr_far = PlayerTestLine (pPlayer,
                            pPlayer->v_origin + pPlayer->v_forward * distance,
                            pPlayer->v_origin + pPlayer->v_forward * distance - pPlayer->v_right * 1000);

   // did the close trace hit something AND did the far trace hit something much more far ?
   if ((tr_near.fraction < 1.0) && (tr_far.fraction > 2 * tr_near.fraction))
      can_turn_left = TRUE; // there's a corner on the left

   // now check for corners on the RIGHT

   // do a trace from 100 units in front of the bot's eyes right...
   tr_near = PlayerTestLine (pPlayer,
                             pPlayer->v_origin + pPlayer->v_forward * distance * 0.75,
                             pPlayer->v_origin + pPlayer->v_forward * distance * 0.75 + pPlayer->v_right * 1000);

   // do a trace from 150 units in front of the bot's eyes right...
   tr_far = PlayerTestLine (pPlayer,
                            pPlayer->v_origin + pPlayer->v_forward * distance,
                            pPlayer->v_origin + pPlayer->v_forward * distance + pPlayer->v_right * 1000);

   // did the close trace hit something AND did the far trace hit something much more far ?
   if ((tr_near.fraction < 1.0) && (tr_far.fraction > tr_near.fraction * 2))
      can_turn_right = TRUE; // there's a corner on the right

// ***VERY*** OLD STUFF, wonder why that's still here (dated from first waypointless attempts)
//   // does the bot have a goal ?
//   if (pBot->v_goal != g_vecZero)
//   {
//      // which side is our goal ?
//      Vector v_goal_angle = WrapAngles (VecToAngles (pBot->v_goal - pPlayer->v_origin) - pPlayer->v_angle);
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
      if (RandomLong (1, 100) < 50)
         can_turn_right = FALSE; // bot decide to go LEFT
      else
         can_turn_left = FALSE; // bot decide to go RIGHT
   }

   // if bot can turn left AND go ahead...
   if (can_turn_left && can_go_ahead)
   {
      if (RandomLong (1, 100) < 50)
         can_go_ahead = FALSE; // bot decide to go LEFT
      else
         can_turn_left = FALSE; // bot decide to go AHEAD
   }

   // if bot can turn right AND go ahead...
   if (can_turn_right && can_go_ahead)
   {
      if (RandomLong (1, 100) < 50)
         can_go_ahead = FALSE; // bot decide to go RIGHT
      else
         can_turn_right = FALSE; // bot decide to go AHEAD
   }

   // given what the bot decided, make it jump or strafe or not
   if (can_turn_left)
   {
      BotSetIdealYaw (pPlayer, pPlayer->Bot.BotHand.ideal_angles.y + RandomLong (60, 80)); // turn there

      // decide if the bot will jump at this corner. If so, the bot will look down a bit more.
      if (RandomLong (1, 100) < 10 * pPlayer->Bot.pProfile->skill)
      {
         pBotLegs->jump_time = server.time; // randomly jump (based on skill)
         BotSetIdealPitch (pPlayer, pPlayer->Bot.BotHand.ideal_angles.x - RandomLong (5, 10)); // look down
      }
      else
         BotSetIdealPitch (pPlayer, pPlayer->Bot.BotHand.ideal_angles.x - RandomLong (0, 5)); // look a bit down

      pBotLegs->straferight_time = server.time + RandomFloat (0.3, 0.8); // strafe right

      pPlayer->Bot.reach_time = server.time + 0.5; // don't try to reach point for 0.5 second
      pPlayer->Bot.cornercheck_time += 3.0; // don't check for corners for 3 secs
   }
   else if (can_turn_right)
   {
      BotSetIdealYaw (pPlayer, pPlayer->Bot.BotHand.ideal_angles.y - RandomLong (60, 80)); // turn there

      // decide if the bot will jump at this corner. If so, the bot will look down a bit more.
      if (RandomLong (1, 100) < 10 * pPlayer->Bot.pProfile->skill)
      {
         pBotLegs->jump_time = server.time; // randomly jump (based on skill)
         BotSetIdealPitch (pPlayer, pPlayer->Bot.BotHand.ideal_angles.x - RandomLong (5, 10)); // look down
      }
      else
         BotSetIdealPitch (pPlayer, pPlayer->Bot.BotHand.ideal_angles.x - RandomLong (0, 5)); // look a bit down

      pBotLegs->strafeleft_time = server.time + RandomFloat (0.3, 0.8); // strafe left

      pPlayer->Bot.reach_time = server.time + 0.5; // don't try to reach point for 0.5 second
      pPlayer->Bot.cornercheck_time += 3.0; // don't check for corners for 3 secs
   }
   else if (can_go_ahead)
      pPlayer->Bot.cornercheck_time += 1.0; // don't check for corners for 1 sec

   return;
}


void BotWalkPath (player_t *pPlayer)
{
   // this function makes the bot pointed to by pBot follow recursively the series of navlinks
   // that made up the path it wants to follow, until it either reaches its end, or that it finds
   // out that the distance to the next navlink in the list is increasing instead of decreasing,
   // indicating that the path is failing (either the bot falled down a cliff or whatever) ; in
   // this case, the bot is told to start thinking of a new path immediately.

   pathmachine_t *pathmachine;
   bot_legs_t *BotMove;
   fov_line_t *vision_front;
   int path_index;
   Vector v_bot2previouslink, v_bot2currentlink, v_bot2nextlink;
   float previouslink_distance, currentlink_distance, nextlink_distance;
   float desired_distance;
   Vector v_pseudolink_origin;

   pathmachine = &pPlayer->Bot.BotBrain.PathMachine; // quick access to bot's pathmachine
   BotMove = &pPlayer->Bot.BotLegs; // quick access to bot's legs
   path_index = BotMove->path_index; // quick access to path index
   vision_front = &pPlayer->Bot.BotEyes.BotFOV[BOT_FOV_WIDTH / 2]; // quick access to bot's front vision

   // first off, has the bot reached its destination ?
   if (path_index >= pathmachine->path_count)
   {
      pPlayer->Bot.BotBrain.bot_task = BOT_TASK_IDLE; // bot doesn't need to follow a path anymore
      pathmachine->path_count = 0; // reset the path
      BotMove->path_index = 0;
      return;
   }

   // bot isn't arrived yet

   // bot needs to know where it's heading to, that is, it needs to know both the link it's
   // currently reaching, AND the link after this one, in order to know when to skip to it. So
   // get a quick access to current and next navlink (also check for end of path on next navlink)
   pPlayer->Bot.previous_navlink = pathmachine->path[(path_index - 1 >= 0 ? path_index - 1 : path_index)];
   pPlayer->Bot.current_navlink = pathmachine->path[path_index];
   pPlayer->Bot.next_navlink = pathmachine->path[(path_index + 1 < pathmachine->path_count ? path_index + 1 : path_index)];

   // get vector from bot to previous, current, and next link
   v_bot2previouslink = pPlayer->Bot.previous_navlink->v_origin - pPlayer->v_origin;
   v_bot2currentlink = pPlayer->Bot.current_navlink->v_origin - pPlayer->v_origin;
   v_bot2nextlink = pPlayer->Bot.next_navlink->v_origin - pPlayer->v_origin;

   previouslink_distance = v_bot2previouslink.Length (); // get distance from bot to next link
   currentlink_distance = v_bot2currentlink.Length (); // get distance from bot to next link
   nextlink_distance = v_bot2nextlink.Length (); // get distance from bot to link after next link

   desired_distance = GameConfig.bb_width * 1.5; // assume bot reached waypoint at 48 units distance

   // does the current navlink involve a ladder ?
   if (pPlayer->Bot.current_navlink->reachability & REACHABILITY_LADDER)
   {
      desired_distance = GameConfig.bb_width * 0.5; // ladder connections need precise placement

      // has the bot NOT identified the ladder yet ?
      if (FNullEnt (pPlayer->Bot.pTransportEntity))
         BotFindTransportEntity (pPlayer, REACHABILITY_LADDER); // if so, find the involved ladder
      
      // has the bot found it finally ?
      if (!FNullEnt (pPlayer->Bot.pTransportEntity))
      {
         // is the bot going UP or DOWN the ladder ?
         if (pPlayer->Bot.current_navlink->v_origin.z > pPlayer->v_origin.z)
         {
            pPlayer->Bot.transport_direction = TRANSPORT_FORWARD; // bot is going UP

            // recompute all the positions and distances by faking the link origin ON the ladder
            v_pseudolink_origin = OriginOf (pPlayer->Bot.pTransportEntity);
            v_pseudolink_origin.z = pPlayer->Bot.current_navlink->v_origin.z;
            v_bot2currentlink = v_pseudolink_origin - pPlayer->v_origin;
            currentlink_distance = v_bot2currentlink.Length ();
         }

         // else the bot is going down...
         else
         {
            pPlayer->Bot.transport_direction = TRANSPORT_BACKWARDS; // bot is going DOWN

            // recompute all the positions and distances by faking the link origin BEFORE the ladder
            v_pseudolink_origin = OriginOf (pPlayer->Bot.pTransportEntity);
            v_pseudolink_origin.z = pPlayer->Bot.current_navlink->v_origin.z;
            v_bot2currentlink = v_pseudolink_origin - pPlayer->v_origin;
            currentlink_distance = v_bot2currentlink.Length ();
         }
      }
   }

   // does the link involve a fall ?
   if (pPlayer->Bot.current_navlink->reachability & REACHABILITY_FALLEDGE)
   {
      desired_distance = GameConfig.bb_width; // falling into pits need more precise placement

      // a priori, nothing to do but to let ourselves fall down the pit...
   }

   // does the link involve an elevator ?
   if (pPlayer->Bot.current_navlink->reachability & REACHABILITY_ELEVATOR)
   {
      desired_distance = GameConfig.bb_width; // elevators need more precise placement

      // has the bot NOT identified the elevator yet ?
      if (FNullEnt (pPlayer->Bot.pTransportEntity))
         BotFindTransportEntity (pPlayer, REACHABILITY_ELEVATOR); // if so, find the involved elevator

      // TODO: check for a button
      // TODO: wait for the elevator to stop before getting on it
      // TODO: wait for the elevator to stop before leaving it
   }

   // does the link involve a bobbing platform ?
   if (pPlayer->Bot.current_navlink->reachability & REACHABILITY_PLATFORM)
   {
      desired_distance = GameConfig.bb_width * 0.5; // bobbing platforms need precise placement

      // has the bot NOT identified the platform yet ?
      if (FNullEnt (pPlayer->Bot.pTransportEntity))
         BotFindTransportEntity (pPlayer, REACHABILITY_PLATFORM); // if so, find the involved platform

      // TODO: check for a button
      // TODO: wait for the platform to be close enough to us before getting on it
      // TODO: wait to be close enough to destination before leaving the platform
   }

   // does the link involve a conveyor ribbon ?
   if (pPlayer->Bot.current_navlink->reachability & REACHABILITY_CONVEYOR)
   {
      desired_distance = GameConfig.bb_width * 0.5; // conveyors need precise placement

      // has the bot NOT identified the conveyor yet ?
      if (FNullEnt (pPlayer->Bot.pTransportEntity))
         BotFindTransportEntity (pPlayer, REACHABILITY_CONVEYOR); // if so, find the involved conveyor

      // TODO: check for a button
      // TODO: wait to be close enough to destination before leaving the conveyor
   }

   // does the link involve a train ?
   if (pPlayer->Bot.current_navlink->reachability & REACHABILITY_TRAIN)
   {
      desired_distance = GameConfig.bb_width * 0.5; // trains need precise placement

      // has the bot NOT identified the train yet ?
      if (FNullEnt (pPlayer->Bot.pTransportEntity))
         BotFindTransportEntity (pPlayer, REACHABILITY_TRAIN); // if so, find the involved train

      // TODO: check for the presence of the train
      // TODO: wait for the train to be close enough to us before getting on it
      // TODO: wait to be close enough to destination before leaving the train
   }

   // does the link involve a teleporter ?
   if (pPlayer->Bot.current_navlink->reachability & REACHABILITY_TELEPORTER)
   {
      desired_distance = GameConfig.bb_width * 0.5; // teleporters may need precise placement

      // a priori, nothing to do but to step inside the teleporter...
   }

   // does the link involve a jump ?
   if (pPlayer->Bot.current_navlink->reachability & REACHABILITY_JUMP)
   {
      desired_distance = GameConfig.bb_width * 0.375; // jumps need precise placement

      // nothing to do but to jump when we're close enough -- this is handled later
   }

   // does the link involve to crouch ?
   if (pPlayer->Bot.current_navlink->reachability & REACHABILITY_CROUCH)
   {
      pPlayer->Bot.BotLegs.duck_time = server.time + 0.2; // well, crouch then...
   }

   // is there a breakable in front of the bot ?
   if ((vision_front->distance < 50) && IsBreakable (vision_front->pHit))
   {
      // FIXME: make the breakable the bot's enemy

      BotSwitchToBestWeapon (pPlayer); // switch to best weapon for the job
      pPlayer->Bot.BotHand.fire1_time = server.time + 0.2; // shoot at stuff
   }

   // if time to, update the bot's distance to its next link
/*   if (BotMove->nextlink_distance_updatetime < server.time)
   {
      BotMove->nextlink_distance = currentlink_distance; // update distance
      BotMove->nextlink_distance_updatetime = server.time + 1.0; // next update in 1s
   }*/

   // is the path failing OR is not reachable anymore (normal reachabilities only) ?
   if (pPlayer->Bot.is_stuck)
   {
      if (pPlayer->is_watched)
         printf ("PATH FAILED!!! CAN'T WALK PATH!!!\n");
//         AIConsole_printf (CHANNEL_NAVIGATION, 3, "PATH FAILED!!! CAN'T WALK PATH!!!\n");

      if (pPlayer->Bot.next_navlink != pPlayer->Bot.current_navlink)
         RemoveNavLink (pPlayer->Bot.next_navlink->node_from, pPlayer->Bot.current_navlink);  // then destroy this link, it's a bad one...

      pathmachine->path_count = 0; // reset the path
      BotMove->path_index = 0;
//      BotMove->nextlink_distance_updatetime = 0;

      // assign the bot the PREVIOUS navlink as a reach point
      pPlayer->Bot.v_reach_point = pPlayer->Bot.previous_navlink->v_origin;
      BotMoveTowardsReachPoint (pPlayer); // make the bot walk back

      pPlayer->Bot.BotBrain.bot_task = BOT_TASK_FINDPATH; // tell the bot to figure out a new path
      return;
   }

   // if we are watching this bot, display the path it is following
   if (pPlayer->is_watched)
   {
      //UTIL_DrawPath (pListenserverPlayer, pathmachine);
      UTIL_DrawLine (pPlayer->v_origin, pPlayer->Bot.current_navlink->v_origin, 1, 255, 255, 255);
      UTIL_DrawWalkface (pPlayer->Bot.current_navlink->node_from->walkface, 1, 0, 0, 63);
      UTIL_DrawLine (pPlayer->Bot.current_navlink->v_origin, pPlayer->Bot.next_navlink->v_origin, 1, 0, 0, 255);
      UTIL_DrawWalkface (pPlayer->Bot.next_navlink->node_from->walkface, 1, 0, 0, 63);
   }

   // assign the bot this navlink as a reach point
   pPlayer->Bot.v_reach_point = pPlayer->Bot.current_navlink->v_origin;

   // walk the path, Neo.
   BotMoveTowardsReachPoint (pPlayer);

   // avoid teammates while we're at it
//   BotAvoidObstacles (pPlayer);
   BotAvoidTeammates (pPlayer);

   // can the bot look around while wandering ? (don't do so for ladders...)
   if (!(pPlayer->Bot.current_navlink->reachability & REACHABILITY_LADDER))
      BotLookAt (pPlayer, pPlayer->Bot.next_navlink->v_origin); // yes, look around
   else
      BotLookAt (pPlayer, pPlayer->Bot.current_navlink->v_origin); // bot should rather look at its destination

   // has the bot reached its current link OR has the bot bypassed it already ?
   if ((currentlink_distance < desired_distance)
       || ((pPlayer->Bot.current_navlink->reachability & (REACHABILITY_NORMAL | REACHABILITY_CROUCH))
           && (AngleOfVectors (v_bot2currentlink/*.Make2D ()*/, v_bot2nextlink/*.Make2D ()*/) > 60)))
   {
      BotMove->path_index++; // skip to the next one in the list
      pPlayer->Bot.pTransportEntity = NULL; // forget about any transport entity

      // was the link involving a jump ?
      if (pPlayer->Bot.current_navlink->reachability & REACHABILITY_JUMP)
      {
         // I would never make use of such expedients but it's just insane to make a bot jump
         // correctly without recording the connection velocity. Count Floyd was fucking right.
         pPlayer->pEntity->v.velocity = pPlayer->Bot.current_navlink->v_connectvelocity;
         pPlayer->Bot.BotLegs.jump_time = server.time; // jump. Now.
      }
   }

   return; // enough for this frame
}


void BotWander (player_t *pPlayer)
{
   // this function makes the bot wander around, doing random stuff, just to pass time. It uses
   // a waypointless navigation algorithm, based on the postulate that any human being has,
   // without any cognitive focus, a natural tendancy to orient its look towards the longest
   // direction coverable in his field of view. This postulate has already been applied in the
   // RACC preview and it has proven to work quite well for basic navigation without waypoints
   // (actually, it's the best waypointless algorithm ever ;))

   AIConsole_printf (CHANNEL_NAVIGATION, 3, "WAYPOINTLESS WANDER!\n");

   // if bot is using a button...
   if (pPlayer->Bot.is_interacting)
      BotUseLift (pPlayer); // bot may be on a lift

   // else if bot is underwater...
   else if (pPlayer->environment == ENVIRONMENT_WATER)
      BotUnderWater (pPlayer); // handle under water movement

   // else if bot is on a ladder...
   else if (pPlayer->environment == ENVIRONMENT_LADDER)
      BotOnLadder (pPlayer); // handle ladder movement

   // else if the bot JUST got off the ladder...
   else if (pPlayer->Bot.end_use_transport_time + 1.0 > server.time)
      pPlayer->Bot.transport_direction = TRANSPORT_UNKNOWN;

   // else if some door to open or some button to press...
   else if (!pPlayer->Bot.is_picking_item && BotCanUseInteractives (pPlayer))
      BotInteractWithWorld (pPlayer);

   // else let's just wander around
   else
   {
      // if time to get a new reach point and bot is not picking an item...
      if ((pPlayer->Bot.getreachpoint_time < server.time)
          && !pPlayer->Bot.is_picking_item)
         BotFindReachPoint (pPlayer); // get a new reach point

      // run to the location the bot wants to reach
      BotReachPosition (pPlayer, pPlayer->Bot.v_reach_point);

      // is bot about to hit something it can jump up ?
      if ((pPlayer->Bot.BotBody.hit_state & OBSTACLE_FRONT_LOWWALL)
          && (pPlayer->Bot.BotLegs.jump_time + 2.0 < server.time))
         pPlayer->Bot.BotLegs.jump_time = server.time; // jump up and move forward

      // else is it about to hit something it can duck under ?
      else if (pPlayer->Bot.BotBody.hit_state & OBSTACLE_FRONT_LOWCEILING)
      {
         // if debug mode is enabled...
         if (pPlayer->is_watched && (DebugLevel.legs > 0))
            AIConsole_printf (CHANNEL_LEGS, 4, "Ducks because WANDERING\n");

         pPlayer->Bot.BotLegs.duck_time = server.time + RandomFloat (0.5, 1.5); // duck & go
      }

      // is the bot about to fall ?
      if ((pPlayer->Bot.fallcheck_time < server.time)
          && (pPlayer->Bot.BotBody.hit_state & OBSTACLE_FRONT_FALL))
         BotTurnAtFall (pPlayer); // try to avoid falling

      // is the bot NOT picking up an item ?
      if (!pPlayer->Bot.is_picking_item)
         BotCheckForCorners (pPlayer); // check for corners on the sides

      // should the bot pause for a while here (every so often, based on skill) ?
      if (!pPlayer->Bot.is_picking_item && (pPlayer->Bot.rush_time < server.time)
          && (mission.start_time + 30.0 < server.time)
          && (pPlayer->Bot.nextpause_time < server.time)
          && !BotCantSeeForward (pPlayer))
      {
         pPlayer->Bot.pause_time = server.time + RandomFloat ((6 - pPlayer->Bot.pProfile->skill) / 2, 6 - pPlayer->Bot.pProfile->skill);
         pPlayer->Bot.nextpause_time = pPlayer->Bot.pause_time + pPlayer->Bot.pProfile->skill * RandomFloat (5, 15);
      }
   }

   // avoid any teammates and obstacles while we're at it
   BotAvoidObstacles (pPlayer);
   BotAvoidTeammates (pPlayer);

   return;
}


char BotEstimateDirection (player_t *pPlayer, Vector v_location)
{
   // this function returns the direction (defined as the DIRECTION_FRONT, DIRECTION_BACK,
   // DIRECTION_LEFT and DIRECTION_RIGHT) at which the vector location v_location is, relatively
   // to the bot itself, under the form of a bitmap (so that a location can be both backwards
   // and on the right, for example).

   float destination_angle;
   char direction;

   direction = 0; // reset direction bitmap first

   // determine which side of the bot is v_location
   destination_angle = WrapAngle ((VecToAngles (v_location - pPlayer->v_origin) - pPlayer->v_angle).y);

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


void BotMoveTowardsReachPoint (player_t *pPlayer)
{
   // this function makes the bot pBot press the right keys in order to move towards the spatial
   // location described by the v_position vector. The bot doesn't need to face the destination
   // vector, instead the function decides which are the optimal keys (forward, backwards, strafe
   // left or right) to press for the bot to move towards v_position.

   bot_body_t *pBotBody;
   bot_legs_t *pBotLegs;
   char direction;

   pBotBody = &pPlayer->Bot.BotBody; // quick access to bot body
   pBotLegs = &pPlayer->Bot.BotLegs; // quick access to bot legs

   // if debug level is high, show the user where the bot wants to go
   if (pPlayer->is_watched && (DebugLevel.legs > 1))
      UTIL_DrawLine (pPlayer->v_origin, pPlayer->Bot.v_reach_point, 1, 0, 255, 0);

   // determine which side of the bot is the bot's objective
   direction = BotEstimateDirection (pPlayer, pPlayer->Bot.v_reach_point);

   // given the angle, let the bot press the right keys (also allowing a combination of these)
   if (direction & DIRECTION_FRONT)
      pBotLegs->forward_time = server.time + 0.1; // go forward to position
   if (direction & DIRECTION_BACK)
      pBotLegs->backwards_time = server.time + 0.1; // go backwards to position
   if (direction & DIRECTION_LEFT)
      pBotLegs->strafeleft_time = server.time + 0.1; // strafe left to position
   if (direction & DIRECTION_RIGHT)
      pBotLegs->straferight_time = server.time + 0.1; // strafe right to position

   // is bot about to hit something it can jump up ?
   if (((pBotBody->hit_state & OBSTACLE_FRONT_LOWWALL)
        && (pBotLegs->forward_time > server.time))
       || ((pBotBody->hit_state & OBSTACLE_LEFT_LOWWALL)
           && (pBotLegs->strafe_speed < 0))
       || ((pBotBody->hit_state & OBSTACLE_RIGHT_LOWWALL)
           && (pBotLegs->strafe_speed > 0)))
   {
      // if debug mode is enabled...
      if (pPlayer->is_watched && (DebugLevel.legs > 0))
         AIConsole_printf (CHANNEL_LEGS, 4, "Jumps because MOVING TOWARDS POSITION\n");

      // has the bot not jumped for enough time long ?
      if (pBotLegs->jump_time + 1.0 < server.time)
         pBotLegs->jump_time = server.time; // jump up and move forward
   }

   // else is bot about to hit something it can duck under ?
   else if ((((pBotBody->hit_state & OBSTACLE_FRONT_LOWCEILING)
              && (pBotLegs->forward_time > server.time))
             || ((pBotBody->hit_state & OBSTACLE_LEFT_LOWCEILING)
                 && ((pBotLegs->strafeleft_time > server.time) || (pBotLegs->forward_time > server.time)))
             || ((pBotBody->hit_state & OBSTACLE_RIGHT_LOWCEILING)
                 && ((pBotLegs->straferight_time > server.time) || (pBotLegs->forward_time > server.time))))
            && (pPlayer->environment == ENVIRONMENT_GROUND))
   {
      // if debug mode is enabled...
      if (pPlayer->is_watched && (DebugLevel.legs > 0))
         AIConsole_printf (CHANNEL_LEGS, 4, "Ducks because MOVING TOWARDS POSITION\n");

      pBotLegs->duck_time = server.time + RandomFloat (0.5, 1.5); // duck & go
   }

   // is the bot stuck ?
   if (pPlayer->Bot.is_stuck)
      BotUnstuck (pPlayer); // try to unstuck our poor bot

   return; // bot should now be moving towards the desired location
}


bool BotReachPosition (player_t *pPlayer, Vector v_position)
{
   if (pPlayer->Bot.reach_time > server.time)
      return (FALSE); // cancel if not time to

   // if bot is not stuck AND the position is visible, look at destination
   if (!pPlayer->Bot.is_stuck && BotCanSeeThis (pPlayer, v_position))
      BotLookAt (pPlayer, v_position);

   // is the bot NOT paused yet ?
   if (pPlayer->Bot.pause_time < server.time)
      pPlayer->Bot.BotLegs.forward_time = server.time + 0.5; // then go forward to position
   else if (pPlayer->Bot.BotLegs.duck_time > server.time)
      pPlayer->Bot.BotLegs.duck_time = server.time + 0.2; // else keep ducking if needed

   // if position is getting closer, walk
   if ((v_position - pPlayer->v_origin).Length () < 40)
      pPlayer->Bot.BotLegs.walk_time = server.time + 0.2; // walk while getting closer

   // is the bot about to fall ?
   if ((pPlayer->Bot.fallcheck_time < server.time) && (pPlayer->Bot.BotBody.hit_state & OBSTACLE_FRONT_FALL))
   {
      BotTurnAtFall (pPlayer); // try to avoid falling
      return (TRUE); // still on the way, despite the obstacles
   }

   // if debug mode is enabled, print out what the bot intends to do
   if (pPlayer->is_watched && (DebugLevel.navigation > 0))
      AIConsole_printf (CHANNEL_NAVIGATION, 1, "Reaches position (remaining distance %f)\n", (v_position - pPlayer->v_origin).Length ());

   // is the bot stuck ?
   if (pPlayer->Bot.is_stuck)
      BotUnstuck (pPlayer); // try to unstuck our poor bot...

   return (TRUE);
}


void BotFindReachPoint (player_t *pPlayer)
{
   int fov_index, max_index;
   float maxdistance = 0;
   Vector v_vecEndPos_dropped;

   // cycle through the FOV data to get the longest distance
   for (fov_index = 0; fov_index < BOT_FOV_WIDTH; fov_index++)
   {
      if (pPlayer->Bot.BotEyes.BotFOV[fov_index].distance > maxdistance)
      {
         maxdistance = pPlayer->Bot.BotEyes.BotFOV[fov_index].distance; // found new reach point
         max_index = fov_index; // remember the FOV index it was on
      }
   }

   // check to see if we can drop this new reach point at human height
   v_vecEndPos_dropped = DropAtHumanHeight (pPlayer->Bot.BotEyes.BotFOV[max_index].vecEndPos);
   if (BotCanSeeThis (pPlayer, v_vecEndPos_dropped))
      pPlayer->Bot.v_reach_point = v_vecEndPos_dropped; // place this reach point on the ground
   else
      pPlayer->Bot.v_reach_point = pPlayer->Bot.BotEyes.BotFOV[max_index].vecEndPos; // let it as is

   // ok so far, bot has a direction to head off into...

   // is it not acceptable (i.e. WAY too close already) ?
   if (maxdistance < 150)
   {
      // then the bot has probably reached a dead end
      BotRandomTurn (pPlayer); // pick up a new direction to head up to

      // and randomly pause here
      if (RandomLong (1, 100) < 40)
         pPlayer->Bot.pause_time = server.time + RandomFloat (0.5, 6 - pPlayer->Bot.pProfile->skill); // pause

      return;
   }

   // if debug level is high, draw a line where the bot wants to go
   if (pPlayer->is_watched && (DebugLevel.navigation > 1))
      UTIL_DrawLine (pPlayer->v_eyeposition, pPlayer->Bot.v_reach_point, 1, 255, 255, 255);

   pPlayer->Bot.getreachpoint_time = server.time + 0.20; // next reach point in 200 ms
   return;
}


void BotUnstuck (player_t *pPlayer)
{
   // this function tries to make the bot whose player structure is pointed to by pPlayer perform
   // the necessary movement(s) to get out of a stuck situation, which happens when the bot wants
   // to move but is unable to because of some obstacle blocking him.

   test_result_t tr1, tr2, tr3;

   // if debug mode is enabled, tell us that this bot is stuck
   if (pPlayer->is_watched && (DebugLevel.navigation > 0))
      AIConsole_printf (CHANNEL_NAVIGATION, 0, "BOT STUCK!!!\n");

   // check if bot can jump onto something and has not jumped for quite a time
   if ((pPlayer->Bot.BotBody.hit_state & OBSTACLE_FRONT_LOWWALL) 
       && (pPlayer->Bot.BotLegs.jump_time + 3.0 < server.time))
   {
      // if debug mode is enabled...
      if (pPlayer->is_watched && (DebugLevel.legs > 0))
         AIConsole_printf (CHANNEL_LEGS, 4, "Jumps because STUCK\n");

      pPlayer->Bot.BotLegs.jump_time = server.time; // jump up and move forward
   }

   // else check if bot can duck under something
   else if (pPlayer->Bot.BotBody.hit_state & OBSTACLE_FRONT_LOWCEILING)
   {
      // if debug mode is enabled...
      if (pPlayer->is_watched && (DebugLevel.legs > 0))
         AIConsole_printf (CHANNEL_LEGS, 4, "Ducks because STUCK\n");

      pPlayer->Bot.BotLegs.duck_time = server.time + RandomFloat (0.5, 1.5); // duck & move forward
   }

   // else check if it won't hurt if the bot steps backwards...
   else if (pPlayer->Bot.BotLegs.backwards_time + 3.0 < server.time)
   {
      // if debug mode is enabled...
      if (pPlayer->is_watched && (DebugLevel.legs > 0))
         AIConsole_printf (CHANNEL_LEGS, 4, "Steps back because STUCK!!\n");

      pPlayer->Bot.BotLegs.backwards_time = server.time + RandomFloat (0.5, 1.0); // step back
   }

   // let's see if the bot has reached a dead-end...
   tr1 = PlayerTestLine (pPlayer,
                         pPlayer->v_eyeposition,
                         pPlayer->v_eyeposition + pPlayer->v_forward * 40);
   tr2 = PlayerTestLine (pPlayer,
                         pPlayer->v_eyeposition,
                         pPlayer->v_eyeposition + pPlayer->v_right * 60);
   tr3 = PlayerTestLine (pPlayer,
                         pPlayer->v_eyeposition,
                         pPlayer->v_eyeposition - pPlayer->v_right * 60);

   // has the bot reached a dead-end ?
   if ((tr1.fraction < 1.0) && (tr2.fraction < 1.0) && (tr3.fraction < 1.0))
   {
      if (RandomLong (1, 100) < 50)
      {
         // if debug mode is enabled...
         if (pPlayer->is_watched && (DebugLevel.legs > 0))
            AIConsole_printf (CHANNEL_LEGS, 4, "Camps because STUCK AT DEAD END\n");

         BotCanCampNearHere (pPlayer, pPlayer->v_origin); // then just camp here on occasion
      }
      else
      {
         // if debug mode is enabled...
         if (pPlayer->is_watched && (DebugLevel.legs > 0))
            AIConsole_printf (CHANNEL_LEGS, 4, "Random turns because STUCK AT DEAD END\n");

         BotRandomTurn (pPlayer); // else pick a new direction
      }
   }

   // can't figure out what to do, try to jump first...
   else if (pPlayer->Bot.BotLegs.jump_time + 3.0 < server.time)
   {
      // if debug mode is enabled...
      if (pPlayer->is_watched && (DebugLevel.legs > 0))
         AIConsole_printf (CHANNEL_LEGS, 4, "Jumps because REALLY STUCK!!\n");

      pPlayer->Bot.BotLegs.jump_time = server.time; // jump up and move forward
   }

   // else duck
   else if (pPlayer->Bot.BotLegs.duck_time + 3.0 < server.time)
   {
      // if debug mode is enabled...
      if (pPlayer->is_watched && (DebugLevel.legs > 0))
         AIConsole_printf (CHANNEL_LEGS, 4, "Ducks because REALLY STUCK!!\n");

      pPlayer->Bot.BotLegs.duck_time = server.time + RandomFloat (0.5, 1.0); // duck & move forward
   }

   // else is the bot trying to get to an item?...
   else if (pPlayer->Bot.is_picking_item)
   {
      // if debug mode is enabled...
      if (pPlayer->is_watched && (DebugLevel.legs > 0))
         AIConsole_printf (CHANNEL_LEGS, 4, "Don't pick items because REALLY STUCK!!\n");

      pPlayer->Bot.is_picking_item = FALSE; // give up trying to reach that item
      pPlayer->Bot.finditem_time = server.time + 10.0; // don't look for items
   }

   // else our destination is REALLY unreachable, try to turnaround to unstuck
   else
   {
      // if debug mode is enabled...
      if (pPlayer->is_watched && (DebugLevel.legs > 0))
         AIConsole_printf (CHANNEL_LEGS, 4, "Random turns because REALLY REALLY STUCK!!!\n");

      BotRandomTurn (pPlayer); // randomly turnaround
   }

   pPlayer->Bot.is_stuck = FALSE; // after this, the bot SHOULD be unstuck (well, hopefully!)
   return;
}


void BotAvoidTeammates (player_t *pPlayer)
{
   // this function is mostly a reflex action ; its purpose is to make the bot react motionally
   // by strafing to any teammate around. The bot cycles through all players and determines if a
   // player is visible and nearby, and if so, it performs the necessary movement adjustments in
   // order to avoid colliding into him.

   int index;
   player_t *pOtherPlayer;
   bot_body_t *pBotBody;
   bot_legs_t *pBotLegs;
   float teammate_angle;
   float teammate_distance;

   pBotBody = &pPlayer->Bot.BotBody; // quick access to bot body
   pBotLegs = &pPlayer->Bot.BotLegs; // quick access to bot legs

   if (pBotLegs->avoid_teammates_time > server.time)
      return; // cancel if not time to

   pBotLegs->avoid_teammates_time = server.time + 0.2; // think about it again in 200 milliseconds

   // search the world for players...
   for (index = 0; index < server.max_clients; index++)
   {
      pOtherPlayer = &players[index]; // quick access to player

      if (!IsValidPlayer (pOtherPlayer) || (pOtherPlayer == pPlayer))
         continue; // skip invalid players and skip self (i.e. this bot)

      if (DebugLevel.is_observer && !pOtherPlayer->is_racc_bot)
         continue; // skip real players in observer mode

      if (pOtherPlayer->pEntity == pPlayer->Bot.BotEnemy.pEdict)
         continue; // don't mind about enemies...

      // see if bot can see the teammate...
      if (IsInFieldOfView (pPlayer->pEntity, pOtherPlayer->v_eyeposition) && BotCanSeeThis (pPlayer, pOtherPlayer->v_eyeposition))
      {
         // bot found a visible teammate
         teammate_angle = AngleOfVectors (pOtherPlayer->v_velocity, pPlayer->v_velocity);
         teammate_distance = (pOtherPlayer->v_origin - pPlayer->v_origin).Length ();

         // is that teammate near us OR coming in front of us and within a certain distance ?
         if ((teammate_distance < 100)
             || ((teammate_distance < 300) && (fabs (teammate_angle) > 160)
                 && (fabs (WrapAngle (pOtherPlayer->v_angle.y - pPlayer->v_angle.y)) > 165)))
         {
            // if we are moving full speed AND there's room forward OR teammate is very close...
            if ((teammate_distance < 70)
                || (!(pBotBody->hit_state & OBSTACLE_FRONT_WALL)
                    && (pPlayer->v_velocity.Length2D () > 50)))
            {
               // is the player coming on the left AND is it safe for the bot to strafe right ?
               if ((teammate_angle > 0) && !(pBotBody->hit_state & OBSTACLE_RIGHT_FALL))
                  pBotLegs->straferight_time = server.time + 0.1; // strafe right to avoid him

               // else is the player coming on the right AND is it safe for the bot to strafe left ?
               else if ((teammate_angle < 0) && !(pBotBody->hit_state & OBSTACLE_LEFT_FALL))
                  pBotLegs->strafeleft_time = server.time + 0.1; // strafe left to avoid him
            }
         }
      }
   }

   return; // enough avoiding teammates
}


void BotAvoidObstacles (player_t *pPlayer)
{
   // this function is mostly a reflex action ; its purpose is to make the bot react motionally
   // by strafing to any obstacle around, i.e crates, walls, etc. The bot uses its sensitive
   // information in order to determine if it needs to get further from a wall, or whatever
   // obstacle. If a wall or corner obstacle is found, the bot adjusts its movement (strafes)
   // in order to avoid it.

   bot_eyes_t *pBotEyes;
   bot_body_t *pBotBody;
   bot_legs_t *pBotLegs;
   fov_line_t *vision_left;
   fov_line_t *vision_right;
   test_result_t tr_left;
   test_result_t tr_right;

   pBotEyes = &pPlayer->Bot.BotEyes; // quick access to bot eyes
   pBotBody = &pPlayer->Bot.BotBody; // quick access to bot body
   pBotLegs = &pPlayer->Bot.BotLegs; // quick access to bot legs

   if (pBotLegs->avoid_obstacles_time > server.time)
      return; // cancel if not time to

   pBotLegs->avoid_obstacles_time = server.time + 0.2; // think about it again in 200 milliseconds

   // determine if bot needs to strafe to avoid walls and corners

   // analyze the bot's field of view ahead of the bot on the LEFT and on the RIGHT
   vision_left = &pBotEyes->BotFOV[BOT_FOV_WIDTH / 2 - 13];
   vision_right = &pBotEyes->BotFOV[BOT_FOV_WIDTH / 2 + 12];

   // are both sides traces stopped at close range ?
   if ((vision_left->distance < 50) && (vision_right->distance < 50))
   {
      // is there something on which the bot can jump up ?
      if ((pBotBody->hit_state & OBSTACLE_FRONT_LOWWALL)
          && (pBotLegs->forward_speed > pPlayer->pEntity->v.maxspeed / 2))
      {
         if (pPlayer->is_watched && (DebugLevel.legs > 1))
            printf ("JUMPING to AVOID OBSTACLE (<50 ft)\n");

         pBotLegs->jump_time = server.time; // jump over the obstacle
      }

      // else did the right trace go further than the left trace ?
      else if ((vision_left->distance < vision_right->distance)
               && !(pBotBody->hit_state & OBSTACLE_RIGHT_WALL))
      {
         if (pPlayer->is_watched && (DebugLevel.legs > 1))
            printf ("STRAFING RIGHT to AVOID OBSTACLE (<50 ft)\n");

         pBotLegs->straferight_time = server.time + 0.1; // strafe right
      }

      // else the left trace went further
      else if (!(pBotBody->hit_state & OBSTACLE_LEFT_WALL))
      {
         if (pPlayer->is_watched && (DebugLevel.legs > 1))
            printf ("STRAFING LEFT to AVOID OBSTACLE (<50 ft)\n");

         pBotLegs->strafeleft_time = server.time + 0.1; // strafe left
      }

      // else there are walls on both sides
      else
      {
         if (pPlayer->is_watched && (DebugLevel.legs > 1))
            printf ("walking BACKWARDS to AVOID OBSTACLE (<50 ft)\n");

         pBotLegs->backwards_time = server.time + 0.3; // so step back a bit

         // was bot walking a path ?
         if ((pPlayer->Bot.BotBrain.bot_task == BOT_TASK_WALKPATH)
             && (pPlayer->Bot.BotBrain.PathMachine.path_count > 0))
         {
            pPlayer->Bot.BotBrain.bot_task = BOT_TASK_IDLE; // have bot figure out another path

            // is the bot completely obstructed here ?
            if ((pBotBody->hit_state & (OBSTACLE_LEFT_WALL | OBSTACLE_FRONT_WALL | OBSTACLE_RIGHT_WALL))
                && (pPlayer->Bot.next_navlink != pPlayer->Bot.current_navlink))
               RemoveNavLink (pPlayer->Bot.next_navlink->node_from, pPlayer->Bot.current_navlink);  // then destroy this link, it's a bad one...
         }
      }

      return; // enough avoiding already
   }

   // there is no immediate obstruction, check further...

   // analyze the bot's field of view ahead of the bot on the LEFT and on the RIGHT
   vision_left = &pBotEyes->BotFOV[1];
   vision_right = &pBotEyes->BotFOV[BOT_FOV_WIDTH - 2];

   // is there a wall on the sides ?
   if ((vision_left->distance < 45) || (vision_right->distance < 45))
   {
      // is the wall rather on the left than on the right ?
      if (vision_left->distance < vision_right->distance)
      {
         if (pPlayer->is_watched && (DebugLevel.legs > 1))
            printf ("STRAFING RIGHT to AVOID OBSTACLE (LEFT WALL)\n");

         pBotLegs->straferight_time = server.time + 0.1; // strafe to avoid it
      }
      else
      {
         if (pPlayer->is_watched && (DebugLevel.legs > 1))
            printf ("STRAFING LEFT to AVOID OBSTACLE (RIGHT WALL)\n");

         pBotLegs->strafeleft_time = server.time + 0.1; // strafe to avoid it
      }

      return; // enough avoiding for now
   }

   // no obstruction on the sides, now check further...

   // analyze the bot's field of view ahead of the bot on the LEFT and on the RIGHT
   tr_left = PlayerTestLine (pPlayer,
                             pBotEyes->v_capture_point + (vision_left->vecEndPos - pBotEyes->v_capture_point).Normalize () * 45,
                             pPlayer->Bot.v_reach_point);
   tr_right = PlayerTestLine (pPlayer,
                              pBotEyes->v_capture_point + (vision_right->vecEndPos - pBotEyes->v_capture_point).Normalize () * 45,
                              pPlayer->Bot.v_reach_point);

   // did the left trace hit something at a close range AND the right trace hit nothing ?
   if ((tr_left.fraction < 0.3) && (tr_right.fraction == 1.0))
   {
      if (pPlayer->is_watched && (DebugLevel.legs > 1))
         printf ("STRAFING RIGHT to AVOID OBSTACLE (anticipation)\n");

      pBotLegs->straferight_time = server.time + 0.1; // there's a corner, strafe to avoid it
   }

   // did the right trace hit something at a close range AND the left trace hit nothing ?
   if ((tr_right.fraction < 0.3) && (tr_left.fraction == 1.0))
   {
      if (pPlayer->is_watched && (DebugLevel.legs > 1))
         printf ("STRAFING LEFT to AVOID OBSTACLE (anticipation)\n");

      pBotLegs->strafeleft_time = server.time + 0.1; // there's a corner, strafe to avoid it
   }

   return; // definitely no obstacle
}


bool BotCanSeeThis (player_t *pPlayer, Vector v_destination)
{
   test_result_t tr1, tr2;

   // don't look through water
   if ((POINT_CONTENTS (pPlayer->v_eyeposition) == CONTENTS_WATER)
       != (POINT_CONTENTS (v_destination) == CONTENTS_WATER))
      return (FALSE);

   // look from bot's left and right eyes
   tr1 = PlayerTestLine (pPlayer,
                         pPlayer->v_eyeposition - pPlayer->v_right * 16,
                         v_destination);
   tr2 = PlayerTestLine (pPlayer,
                         pPlayer->v_eyeposition + pPlayer->v_right * 16,
                         v_destination);

   if ((tr1.fraction == 1.0) && (tr2.fraction == 1.0))
      return (TRUE); // line of sight is excellent

   else if ((tr1.fraction == 1.0) && (tr2.fraction < 1.0))
      return (TRUE); // line of sight is valid, though bot might want to strafe left to see better

   else if ((tr1.fraction < 1.0) && (tr2.fraction == 1.0))
      return (TRUE); // line of sight is valid, though bot might want to strafe right to see better

   return (FALSE); // line of sight is not established
}


bool BotCanCampNearHere (player_t *pPlayer, Vector v_here)
{
   float distance = 0, prev_distance = 0, prev_prev_distance = 0, angle, interesting_angles[72];
   int index, angles_count = 0;
   Vector v_prev_hitpoint = v_here;
   test_result_t tr;
   player_t *pOtherPlayer;

   if (pPlayer->environment != ENVIRONMENT_GROUND)
      return (FALSE); // don't even think about it if bot is jumping

   angle = WrapAngle (pPlayer->v_angle.y); // initialize scan angle to bot's view angle

   // cycle through all players to find if a teammate is already camping near here
   for (index = 0; index < server.max_clients; index++)
   {
      pOtherPlayer = &players[index]; // quick access to player

      if (!IsValidPlayer (pOtherPlayer) || !pOtherPlayer->is_alive || (pOtherPlayer == pPlayer))
         continue; // skip invalid and dead players and skip self (i.e. this bot)

      if (DebugLevel.is_observer && !pOtherPlayer->is_racc_bot)
         continue; // skip real players if in observer mode

      if ((pOtherPlayer->input_buttons & INPUT_KEY_DUCK)
          && ((pOtherPlayer->v_origin - v_here).Length () < 1000))
         return (FALSE); // give up if another player is already camping near here
   }

   // scan 360 degrees around here in 72 samples...
   for (index = 0; index < 72; index++)
   {
      angle = WrapAngle (angle + 5); // pan the trace angle from left to right
      BuildReferential (Vector (0, angle, 0)); // build base vectors in that direction

      // trace line at waist level
      tr = PlayerTestLine (pPlayer,
                           v_here,
                           v_here + (referential.v_forward * 10000));

      // if debug level is high, draw the field of view of this bot
      if (pPlayer->is_watched && (DebugLevel.eyes > 1))
         UTIL_DrawLine (v_here, v_here + (referential.v_forward * 10000), 1, 255, 0, 0);

      if (prev_distance > 0)
         prev_prev_distance = prev_distance; // rotate the previous distances
      else
         prev_prev_distance = tr.fraction * 10000; // handle start of scan
      if (distance > 0)
         prev_distance = distance; // rotate the previous distances
      else
         prev_distance = tr.fraction * 10000; // handle start of scan
      distance = tr.fraction * 10000; // store distance to obstacle

      // have we a peak (meaning a safe corner) ?
      if ((prev_distance > prev_prev_distance) && (prev_distance > distance) && (prev_distance > 80)
          && BotCanSeeThis (pPlayer, v_prev_hitpoint) && IsAtHumanHeight (v_prev_hitpoint)
          && PlayerCanReach (pPlayer, v_prev_hitpoint))
      {
         interesting_angles[angles_count] = WrapAngle (angle - 5); // remember this angle
         angles_count++; // increment interesting angles count
      }

      v_prev_hitpoint = tr.v_endposition; // rotate the remembered hit point
   }

   // okay, now we know which angles are candidates for determining a good camp point

   if ((angles_count <= 1) || (angles_count >= 72))
      return (FALSE); // give up if none found, bot can't camp near here

   angle = WrapAngle (interesting_angles[RandomLong (0, angles_count - 1)]); // choose one
   BuildReferential (Vector (0, angle, 0)); // build base vectors in that direction

   // trace line slightly under eyes level
   tr = PlayerTestLine (pPlayer,
                        v_here,
                        v_here + (referential.v_forward * 10000));

   // assign bot this camp point
   pPlayer->Bot.v_place_to_keep = v_here + referential.v_forward * ((10000 * tr.fraction) - 40);
   pPlayer->Bot.place_time = server.time; // remember when we last saw the place to keep
   pPlayer->Bot.reload_time = server.time + RandomLong (1.5, 3.0); // switch to best weapon for the job

   return (TRUE); // bot found a camp spot next to v_here
}


void BotNavLoadBrain (player_t *pPlayer)
{
   // this function sets up the navigation nodes in the bot's memory. Either by loading them
   // from disk, or by inferring brand new ones based on the map's contents. They will anyhow
   // be saved back to disk when the bot will leave the server. Note the use of the MFILE file
   // loading library that loads the file completely in memory and reads it from here instead
   // of reading it from the disk.

   MFILE *mfp;
   FILE *fp;
   bot_brain_t *brain;
   char nav_filename[256];
   navnode_t *node;
   int recorded_walkfaces_count, face_index, link_index, array_index;
   char cookie[32];
   char section_name_length;
   bool valid_brain = FALSE;
   bool found_section = FALSE;
   bool valid_section = FALSE;
   char *section_before, *section_after;
   int section_before_size, section_after_start, section_after_size;

   brain = &pPlayer->Bot.BotBrain; // quick access to bot brain

   // build the file name
   sprintf (nav_filename, "%s/knowledge/%s/%s.nav", GameConfig.racc_basedir, GameConfig.mod_name, NormalizeChars (pPlayer->connection_name));

   // get the section name length
   section_name_length = strlen (server.map_name) + 1;

   // first make sure the brain space is empty
   if (brain->PathMemory != NULL)
      free (brain->PathMemory);

   // allocate enough memory for the navigation nodes
   brain->PathMemory = (navnode_t *) malloc (map.walkfaces_count * sizeof (navnode_t));
   if (brain->PathMemory == NULL)
      TerminateOnError ("BotNavLoadBrain(): Unable to allocate enough memory to infer a new nav brain to %s\n", pPlayer->connection_name); // bomb out on error

   // initialize the nav brain memory space to zero
   memset (brain->PathMemory, 0, sizeof (*brain->PathMemory));

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
      if (mfseekAtSection (mfp, server.map_name) == 0)
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
      ServerConsole_printf ("RACC: bot %s's nav brain damaged!\n", pPlayer->connection_name);

      if (server.developer_level > 1)
         ServerConsole_printf ("RACC: inferring a new nav brain to %s\n", pPlayer->connection_name);

      // create the new brain (i.e, save a void one in the brain file)
      fp = fopen (nav_filename, "wb");
      if (fp == NULL)
         TerminateOnError ("BotNavLoadBrain(): Unable to operate on %s's nav brain !\n", pPlayer->connection_name);

      fwrite ("RACCNAV", sizeof ("RACCNAV"), 1, fp); // write identification tag
      fwrite ("[likelevels]", sizeof ("[likelevels]"), 1, fp); // write likelevels section tag
      fwrite (&default_likelevels, sizeof (float), RACC_MAX_LIKELEVELS, fp); // write all default likelevels
      fwrite ("[section]", sizeof ("[section]"), 1, fp); // write map section tag
      fwrite (server.map_name, section_name_length, 1, fp); // write map section name (map name)
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
      ServerConsole_printf ("RACC: bot %s is discovering a new map!\n", pPlayer->connection_name);

      if (server.developer_level > 1)
         ServerConsole_printf ("RACC: adding new section to %s's nav brain\n", pPlayer->connection_name);

      // open the brain for appending
      fp = fopen (nav_filename, "ab");
      if (fp == NULL)
         TerminateOnError ("BotNavLoadBrain(): Unable to operate on %s's nav brain !\n", pPlayer->connection_name);

      fwrite ("[section]", sizeof ("[section]"), 1, fp); // section tag
      fwrite (server.map_name, section_name_length, 1, fp); // section name
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
      ServerConsole_printf ("RACC: damaged section in bot %s's nav brain!\n", pPlayer->connection_name);

      if (server.developer_level > 1)
         ServerConsole_printf ("RACC: flushing section in %s's nav brain\n", pPlayer->connection_name);

      // open the brain for surgery
      mfp = mfopen (nav_filename, "rb");
      if (mfp == NULL)
         TerminateOnError ("BotNavLoadBrain(): Unable to operate on %s's nav brain !\n", pPlayer->connection_name);

      // locate at start of the damaged section
      if (mfseekAtSection (mfp, server.map_name) != 0)
         TerminateOnError ("BotNavLoadBrain(): Unable to locate before damaged section in %s's nav brain !\n", pPlayer->connection_name);

      section_before_size = mftell (mfp); // get the size of what's before the damaged section
      if (section_before_size == 0)
         TerminateOnError ("BotNavLoadBrain(): Unable to read before damaged section in %s's nav brain !\n", pPlayer->connection_name);
      section_before = (char *) malloc (section_before_size); // allocate memory for what's before
      if (section_before == NULL)
         TerminateOnError ("BotNavLoadBrain(): malloc() failure for reading before damaged section in %s's nav brain !\n", pPlayer->connection_name);
      mfseek (mfp, 0, SEEK_SET); // rewind at start of file
      mfread (section_before, section_before_size, 1, mfp); // and read what's before damaged section

      // now locate after the damaged section
      if (mfseekAfterSection (mfp, server.map_name) != 0)
      {
         if (section_before != NULL)
            free (section_before); // free the memory we mallocated() for what's before damaged section
         TerminateOnError ("BotNavLoadBrain(): Unable to locate after damaged section in %s's nav brain !\n", pPlayer->connection_name);
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
            TerminateOnError ("BotNavLoadBrain(): malloc() failure for reading after damaged section in %s's nav brain !\n", pPlayer->connection_name);
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
         TerminateOnError ("BotNavLoadBrain(): Unable to operate on %s's nav brain !\n", pPlayer->connection_name);
      }

      fwrite (section_before, section_before_size, 1, fp); // write what's before
      fwrite ("[section]", sizeof ("[section]"), 1, fp); // section tag
      fwrite (server.map_name, section_name_length, 1, fp); // section name
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
      ServerConsole_printf ("RACC: restoring nav brain to %s\n", pPlayer->connection_name);

   // now that we ensured about its validity, we can safely load the brain
   mfp = mfopen (nav_filename, "rb"); // open the brain file again
   mfseek (mfp, sizeof ("RACCNAV"), SEEK_CUR); // skip the "RACCNAV" tag
   mfseek (mfp, sizeof ("[likelevels]"), SEEK_CUR); // skip the "[likelevels]" tag

   // first read the global reachability likelevels
   mfread (&brain->likelevel, sizeof (likelevel_t), 1, mfp); // read ALL likelevels

   // now read the map section navmesh data
   mfseekAtSection (mfp, server.map_name); // seek at start of section
   mfseek (mfp, sizeof ("[section]"), SEEK_CUR); // skip the [section] tag
   mfseek (mfp, section_name_length, SEEK_CUR); // skip the section name
   mfread (&recorded_walkfaces_count, sizeof (long), 1, mfp); // load the walkfaces count

   // file is okay, cycle through all navigation nodes...
   for (face_index = 0; face_index < map.walkfaces_count; face_index++)
   {
      node = &brain->PathMemory[face_index]; // quick access to node

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

         node->links[link_index].node_from = (navnode_t *) ((unsigned long) brain->PathMemory + array_index * sizeof (navnode_t));

         // test this pointer against access violation (pointers are plain evil)
         if ((node->links[link_index].node_from < &brain->PathMemory[0]) || (node->links[link_index].node_from > &brain->PathMemory[map.walkfaces_count - 1]))
            TerminateOnError ("BotNavLoadBrain(): bad node pointer %d (range %d - %d), index %d/%d\n", node->links[link_index].node_from, &brain->PathMemory[0], &brain->PathMemory[map.walkfaces_count - 1], link_index, node->links_count);

         // read the reachability type for this link (normal, ladder, elevator...)
         mfread (&node->links[link_index].reachability, sizeof (short), 1, mfp);

         // read the vector origin for this link
         mfread (&node->links[link_index].v_origin, sizeof (Vector), 1, mfp);

         // read the connection velocity for this link
         mfread (&node->links[link_index].v_connectvelocity, sizeof (Vector), 1, mfp);
      }
   }

   mfclose (mfp); // everything is loaded, close the file
   return; // no error, return FALSE
}


void BotNavSaveBrain (player_t *pPlayer)
{
   // this function saves the navigation nodes in the bot's memory in a file to disk. The
   // authentication being made by comparison of the recorded number of walkfaces, this data
   // is written in the file at the very end of the function, to ensure an error in the save
   // process won't let a badly authenticated file on disk.

   MFILE *mfp;
   FILE *fp;
   bot_brain_t *brain;
   char nav_filename[256];
   char section_name_length;
   navnode_t *node;
   int face_index, link_index, array_index;
   char *section_before = NULL, *section_after = NULL;
   int section_before_size, section_after_start, section_after_size;

   brain = &pPlayer->Bot.BotBrain; // quick access to bot brain

   // build the file name
   sprintf (nav_filename, "%s/knowledge/%s/%s.nav", GameConfig.racc_basedir, GameConfig.mod_name, NormalizeChars (pPlayer->connection_name));

   // get the section name
   section_name_length = strlen (server.map_name) + 1;

   // open the brain for updating (let's read what's before first)
   mfp = mfopen (nav_filename, "rb");
   if (mfp == NULL)
      TerminateOnError ("BotNavLoadBrain(): Unable to operate on %s's nav brain !\n", pPlayer->connection_name);

   // locate at start of the section to update
   if (mfseekAtSection (mfp, server.map_name) != 0)
      TerminateOnError ("BotNavSaveBrain(): Unable to locate before section to update in %s's nav brain !\n", pPlayer->connection_name);

   section_before_size = mftell (mfp); // get the size of what's before the section to update
   if (section_before_size == 0)
      TerminateOnError ("BotNavSaveBrain(): Unable to read before section to update in %s's nav brain !\n", pPlayer->connection_name);
   section_before = (char *) malloc (section_before_size + 1); // allocate memory for what's before
   if (section_before == NULL)
      TerminateOnError ("BotNavSaveBrain(): malloc() failure for reading before section to update in %s's nav brain !\n", pPlayer->connection_name);
   mfseek (mfp, 0, SEEK_SET); // rewind at start of file
   mfread (section_before, section_before_size, 1, mfp); // and read what's before section to update

   // now locate after the section to update
   if (mfseekAfterSection (mfp, server.map_name) != 0)
   {
      if (section_before != NULL)
         free (section_before); // free the memory we mallocated() for what's before section to update
      TerminateOnError ("BotNavSaveBrain(): Unable to locate after section to update in %s's nav brain !\n", pPlayer->connection_name);
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
         TerminateOnError ("BotNavSaveBrain(): malloc() failure for reading after section to update in %s's nav brain !\n", pPlayer->connection_name);
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
      TerminateOnError ("BotNavSaveBrain(): Unable to operate on %s's nav brain !\n", pPlayer->connection_name);
   }

   fwrite (section_before, section_before_size, 1, fp); // write what's before
   fwrite ("[section]", sizeof ("[section]"), 1, fp); // section tag
   fwrite (server.map_name, section_name_length, 1, fp); // section name
   fwrite ("\0\0\0\0", sizeof (long), 1, fp); // fill the field with zeroes (temporarily)

   // for each navigation node...
   for (face_index = 0; face_index < map.walkfaces_count; face_index++)
   {
      node = &brain->PathMemory[face_index]; // quick access to node

      // write the number of links this node has
      fwrite (&node->links_count, sizeof (char), 1, fp);

      // for each link of this node...
      for (link_index = 0; link_index < node->links_count; link_index++)
      {
         // translate the pointer address into an array relative index
         array_index = ((unsigned long) node->links[link_index].node_from - (unsigned long) brain->PathMemory) / sizeof (navnode_t);
         if ((array_index < 0) || (array_index >= map.walkfaces_count))
            TerminateOnError ("BotNavSaveBrain(): bad node array index %d (max %d), index %d/%d\n", array_index, map.walkfaces_count - 1, link_index, node->links_count);
         fwrite (&array_index, sizeof (long), 1, fp); // write the walkface index of the link

         // write the reachability type for this link (normal, ladder, elevator...)
         fwrite (&node->links[link_index].reachability, sizeof (short), 1, fp);

         // write the vector origin for this link
         fwrite (&node->links[link_index].v_origin, sizeof (Vector), 1, fp);

         // write the connection velocity for this link
         fwrite (&node->links[link_index].v_connectvelocity, sizeof (Vector), 1, fp);
      }
   }

   if (section_after_size > 0)
      fwrite (section_after, section_after_size, 1, fp); // and write what's after, if needed

   // now that the map specific data sections have been dumped, we can write the likelevels
   fseek (fp, 0, SEEK_SET); // rewind at start of file
   fseek (fp, sizeof ("RACCNAV"), SEEK_CUR); // skip the "RACCNAV" tag
   fseek (fp, sizeof ("[likelevels]"), SEEK_CUR); // skip the "[likelevels]" tag
   fwrite (&brain->likelevel, sizeof (likelevel_t), 1, fp); // write ALL likelevels

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
