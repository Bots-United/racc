// RACC - AI development project for first-person shooter games derivated from Valve's Half-Life
// (http://www.racc-ai.com/)
//
// The game to engine interfacing code is based on the work done by Jeffrey 'Botman' Broome
// (http://planethalflife.com/botman/)
//
// This project is partially based on the work done by Eric Bieschke in his BSDbot
// (http://gamershomepage.com/csbot/)
//
// This project is partially based on the work done by Brendan 'Spyro' McCarthy in his ODD Bot
// (http://oddbot.hlfusion.com/)
//
// This project is partially based on the work done by Alistair 'eLiTe' Stewart in his TEAMbot
// (http://www.planethalflife.com/teambot/)
//
// This project is partially based on the work done by Johannes '@$3.1415rin' Lampel in his JoeBot
// (http://www.joebot.net/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// bot_legs.cpp
//

#include "racc.h"


extern entity_t *pListenserverEntity;
extern debug_level_t DebugLevel;


void BotMove (bot_t *pBot)
{
   // the purpose of this function is to translate the data of the BotLegs structure (timings
   // at which the bot has to perform some movement - jump in 2 seconds, move forward for 5
   // seconds, and so on) into the right input buttons to be passed to RunPlayerMove(), which is
   // the function that asks the engine to perform the movement of the fakeclient. It also sets
   // the correct values for move_speed and strafe_speed which are parameters of RunPlayerMove().

   TraceResult tr;

   if (IsNull (pBot->pEntity))
      return; // reliability check

   if (!IsAlive (pBot->pEntity))
      return; // don't try to move anyhow anymore if bot is dead :)

   // has the bot just jumped AND is bot skilled enough for doing a duck-jump ?
   if ((pBot->BotLegs.input_buttons.f_jump_time > CurrentTime ()) && (pBot->BotLegs.input_buttons.f_jump_time < CurrentTime () + 0.1)
       && (pBot->pPersonality->skill > 1))
      pBot->BotLegs.input_buttons.f_duck_time = CurrentTime () + 0.2; // duck while jumping

   // may the bot safely strafe left now ?
   if ((pBot->BotLegs.input_buttons.f_strafeleft_time > CurrentTime ()) && !(pBot->BotBody.hit_state & OBSTACLE_LEFT_FALL))
      pBot->BotLegs.movement_speed.y = -pBot->BotLegs.f_max_speed; // strafe left

   // else may the bot safely strafe right now ?
   else if ((pBot->BotLegs.input_buttons.f_straferight_time > CurrentTime ()) && !(pBot->BotBody.hit_state & OBSTACLE_RIGHT_FALL))
      pBot->BotLegs.movement_speed.y = pBot->BotLegs.f_max_speed; // strafe right

   // may the bot move backwards now ?
   if ((pBot->BotLegs.input_buttons.f_backwards_time > CurrentTime ()) || pBot->BotLegs.b_emergency_walkback)
      pBot->BotLegs.movement_speed.x = -pBot->BotLegs.f_max_speed; // move backwards

   // else may the bot run forward now ?
   else if (pBot->BotLegs.input_buttons.f_forward_time > CurrentTime ())
      pBot->BotLegs.movement_speed.x = pBot->BotLegs.f_max_speed; // run forward

   // may the bot walk now ?
   if ((pBot->BotLegs.input_buttons.f_walk_time > CurrentTime ()) && !pBot->BotLegs.b_emergency_walkback)
      pBot->BotLegs.movement_speed.x = pBot->BotLegs.f_max_speed / 2; // walk forward

   return;
}


void BotAvoidObstacles (bot_t *pBot)
{
   // this function is mostly a reflex action ; its purpose is to make the bot react motionally
   // to any obstacle immediately around, i.e players, walls, etc. First the bot cycles through
   // all players and determines if a player is visible and nearby, and if so, it performs the
   // necessary movement adjustments in order to avoid colliding into him. Then, it uses its
   // sensitive information in order to determine if it also needs to get further from a wall,
   // or whatever obstacle. If a wall or corner obstacle is found, the bot adjusts its movement
   // (strafes) in order to avoid it.

   TraceResult tr, tr2;
   int player_index;
   referential_t referential;

   if (IsNull (pBot->pEntity))
      return; // reliability check

   // search the world for players...
   for (player_index = 0; player_index < MaxClientsOnServer (); player_index++)
   {
      entity_t *pPlayer = PlayerAtIndex (player_index);

      if (IsNull (pPlayer) || (pPlayer == pBot->pEntity))
         continue; // skip invalid players and skip self (i.e. this bot)

      if (DebugLevel.is_observer && !IsABot (pPlayer))
         continue; // skip real players in observer mode

      if (GetTeam (pPlayer) != GetTeam (pBot->pEntity))
         continue; // don't mind about enemies...

      // see if bot can see the teammate...
      if (BotCanSeeOfEntity (pBot, pPlayer) != NULLVEC)
      {
         // bot found a visible teammate
         vector v_teammate_angle = WrapAngles (AnglesOfVector (OriginOf (pPlayer) - OriginOf (pBot->pEntity)) - ViewAnglesOf (pBot->pEntity));
         float f_teammate_distance = (OriginOf (pPlayer) - OriginOf (pBot->pEntity)).Length ();

         // is that teammate near us OR coming in front of us and within a certain distance ?
         if ((f_teammate_distance < 100)
             || ((f_teammate_distance < 300) && (v_teammate_angle.y < 15)
                 && (abs (WrapAngle (ViewAnglesOf (pPlayer).y - ViewAnglesOf (pBot->pEntity).y)) > 165)))
         {
            // if we are moving full speed AND there's room forward OR teammate is very close...
            if (((HorizontalVelocityOf (pBot->pEntity) > 10) && (pBot->BotBody.hit_state == OBSTACLE_NONE))
                || (f_teammate_distance < 70))
            {
               if (v_teammate_angle.y > 0)
                  pBot->BotLegs.input_buttons.f_straferight_time = CurrentTime () + 0.2; // strafe right to avoid him
               else
                  pBot->BotLegs.input_buttons.f_strafeleft_time = CurrentTime () + 0.2; // strafe left to avoid him

               pBot->f_reach_time = CurrentTime () + 0.5; // delay reaching point
            }
         }
      }
   }

   // determine if bot need to strafe to avoid walls and corners
   referential = ReferentialOfAngles (ViewAnglesOf (pBot->pEntity)); // build base vectors

   // do a trace on the left side of the bot some steps forward
   UTIL_TraceLine (OriginOf (pBot->pEntity) - referential.v_right * 16,
                   OriginOf (pBot->pEntity) - referential.v_right * 16 + referential.v_forward * 40,
                   ignore_monsters, pBot->pEntity, &tr);

   // do a trace on the right side of the bot some steps forward
   UTIL_TraceLine (OriginOf (pBot->pEntity) + referential.v_right * 16,
                   OriginOf (pBot->pEntity) + referential.v_right * 16 + referential.v_forward * 40,
                   ignore_monsters, pBot->pEntity, &tr2);

   // did the right trace hit something further than the left trace ?
   if (tr.flFraction < tr2.flFraction)
      pBot->BotLegs.input_buttons.f_straferight_time = CurrentTime () + 0.2; // there's an obstruction, strafe to avoid it

   // else did the left trace hit something further than the right trace ?
   else if (tr.flFraction > tr2.flFraction)
      pBot->BotLegs.input_buttons.f_strafeleft_time = CurrentTime () + 0.2; // there's an obstruction, strafe to avoid it

   // else there is no immediate obstruction, check further...
   else
   {
      // make sure we trace inside the map (check on the left)
      UTIL_TraceLine (OriginOf (pBot->pEntity),
                      OriginOf (pBot->pEntity) - referential.v_right * 30 + referential.v_forward * 30,
                      ignore_monsters, pBot->pEntity, &tr);

      // make sure we trace inside the map (check on the right)
      UTIL_TraceLine (OriginOf (pBot->pEntity),
                      OriginOf (pBot->pEntity) + referential.v_right * 30 + referential.v_forward * 30,
                      ignore_monsters, pBot->pEntity, &tr2);

      // if there is a wall on the left
      if (tr.flFraction < 1.0)
         pBot->BotLegs.input_buttons.f_straferight_time = CurrentTime () + 0.2; // strafe to avoid it

      // else if there is a wall on the right
      else if (tr2.flFraction < 1.0)
         pBot->BotLegs.input_buttons.f_strafeleft_time = CurrentTime () + 0.2; // strafe to avoid it

      // else no side obstruction, check further...
      else
      {
         // do a trace from 30 units on the left of the bot to destination
         UTIL_TraceLine (OriginOf (pBot->pEntity) - referential.v_right * 30 + referential.v_forward * 30,
                         pBot->v_reach_point,
                         ignore_monsters, pBot->pEntity, &tr);

         // do a trace from 30 units on the right of the bot to destination
         UTIL_TraceLine (OriginOf (pBot->pEntity) + referential.v_right * 30 + referential.v_forward * 30,
                         pBot->v_reach_point,
                         ignore_monsters, pBot->pEntity, &tr2);

         // did the left trace hit something at a close range AND the right trace hit nothing ?
         if ((tr.flFraction < 0.3) && (tr2.flFraction == 1.0))
            pBot->BotLegs.input_buttons.f_straferight_time = CurrentTime () + 0.2; // there's a corner, strafe to avoid it

         // did the right trace hit something at a close range AND the left trace hit nothing ?
         if ((tr2.flFraction < 0.3) && (tr.flFraction == 1.0))
            pBot->BotLegs.input_buttons.f_strafeleft_time = CurrentTime () + 0.2; // there's a corner, strafe to avoid it
      }
   }

   pBot->f_avoid_time = CurrentTime () + 0.2; // next check in 200 ms
   return;
}


void BotSetIdealAngles (bot_t *pBot, vector v_ideal_angles)
{
   // this function sets the angles at which the bot wants to look. You don't have to
   // worry about changing the view angles of the bot directly, the bot turns to the
   // right direction by itself in a manner depending of the situation (e.g combat aim,
   // wandering aim...) with the BotPointGun() function. You can call this one as many
   // times you want each frame, as it is just about setting IDEAL angles. Note the use
   // of WrapAngle() to keep the angles in bounds and prevent overflows.

   if (IsNull (pBot->pEntity))
      return; // reliability check

   pBot->BotLegs.ideal_angles = WrapAngles (v_ideal_angles);
}


void BotPointGun (bot_t *pBot)
{
   // this function is called every frame for every bot, after the bot has finished its
   // sensing and thinking cycle, in the BotPostThink (). Its purpose is to make the bot
   // move its crosshair to the direction where it wants to look. There is some kind of
   // filtering for the view, to make it human-like.

   float frame_duration, speed; // speed : 0.1 - 1
   vector v_deviation;

   if (IsNull (pBot->pEntity))
      return; // reliability check

   // if debug mode is enabled, display the bot's angles
   if (DebugLevel.legs > 0)
   {
      DrawLine (pListenserverEntity, EyeOriginOf (pBot->pEntity), EyeOriginOf (pBot->pEntity) + ReferentialOfAngles (ViewAnglesOf (pBot->pEntity)).v_forward * 100, 2, 0, 255, 0);
      DrawLine (pListenserverEntity, EyeOriginOf (pBot->pEntity), EyeOriginOf (pBot->pEntity) + ReferentialOfAngles (pBot->BotLegs.ideal_angles).v_forward * 100, 2, 255, 0, 0);
      DrawLine (pListenserverEntity, OriginOf (pBot->pEntity), OriginOf (pBot->pEntity) + ReferentialOfAngles (AnglesOf (pBot->pEntity)).v_forward * 100, 2, 0, 0, 255);
   }

   frame_duration = FrameDuration (); // get the duration of a video frame on this server
   v_deviation = WrapAngles (pBot->BotLegs.ideal_angles - ViewAnglesOf (pBot->pEntity));

   // let the bot know if it is walking straight ahead or not
   if (v_deviation.y <= 1.0)
      pBot->BotLegs.is_walking_straight = TRUE;
   else
      pBot->BotLegs.is_walking_straight = FALSE;

   // if bot is aiming at something, or turning at a corner, or blinded, aim fast else take our time...
   if (!IsNull (pBot->pBotEnemy))
      speed = 0.7 + (pBot->pPersonality->skill - 1) / 10; // fast aim
   else if (pBot->f_turncorner_time > CurrentTime ())
      speed = 0.8 + (pBot->pPersonality->skill - 1) / 5; // fast and inaccurate aim
   else if (pBot->BotEyes.f_blinded_time > CurrentTime ())
      speed = 0.1 + (pBot->pPersonality->skill - 1) / 10; // slow and inaccurate aim
   else
      speed = 0.2 + (pBot->pPersonality->skill - 1) / 20; // slow aim

   // thanks Tobias "Killaruna" Heimann and Johannes "@$3.1415rin" Lampel for this one
   pBot->BotLegs.aim_speed.x = (pBot->BotLegs.aim_speed.x * exp (log (speed / 2) * frame_duration * 20)
                                + speed * v_deviation.x * (1 - exp (log (speed / 2) * frame_duration * 20)))
                               * frame_duration * 20;
   pBot->BotLegs.aim_speed.y = (pBot->BotLegs.aim_speed.y * exp (log (speed / 2) * frame_duration * 20)
                                + speed * v_deviation.y * (1 - exp (log (speed / 2) * frame_duration * 20)))
                               * frame_duration * 20;

   // influence of y movement on x axis, based on skill (less influence than x on y since it's
   // easier and more natural for the bot to "move its mouse" horizontally than vertically)
   if (pBot->BotLegs.aim_speed.x > 0)
      pBot->BotLegs.aim_speed.x += pBot->BotLegs.aim_speed.y / (1.5 * (1 + pBot->pPersonality->skill));
   else
      pBot->BotLegs.aim_speed.x -= pBot->BotLegs.aim_speed.y / (1.5 * (1 + pBot->pPersonality->skill));

   // influence of x movement on y axis, based on skill
   if (pBot->BotLegs.aim_speed.y > 0)
      pBot->BotLegs.aim_speed.y += pBot->BotLegs.aim_speed.x / (1 + pBot->pPersonality->skill);
   else
      pBot->BotLegs.aim_speed.y -= pBot->BotLegs.aim_speed.x / (1 + pBot->pPersonality->skill);

   // update the bot's player entity angles
   ChangePlayerAngles (pBot->pEntity, ViewAnglesOf (pBot->pEntity) + pBot->BotLegs.aim_speed);

   return;
}
