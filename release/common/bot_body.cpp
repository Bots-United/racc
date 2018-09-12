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
// bot_body.cpp
//

#include "racc.h"


extern debug_level_t DebugLevel;


void BotTouch (bot_t *pBot)
{
   // this function makes the bot aware of its immediate surroundings, building an integer bitmap
   // representing the layout of the terrain right around the bot, and realizing if its movement
   // doesn't perform as expected (check if stuck), which is a feedback from the motile AI.

   if (IsNull (pBot->pEntity))
      return; // reliability check

   BotCheckForObstaclesAtFeet (pBot); // identify any obstacle in front of us
   BotCheckIfStuck (pBot); // realize if we are stuck or not
}


void BotCheckForObstaclesAtFeet (bot_t *pBot)
{
   // this function returns an integer bitmap describing the presence and quality of any
   // obstacle right around the bot. Useful for low walls over which the bot has to jump,
   // or for determining if the bot should duck to pass a low ceiling. This function is
   // called every frame, systematically, in BotPreThink(), so that the bot knows, when
   // it starts thinking, the quality of the terrain in front of it. First it checks if
   // it is about to hit something when walking forward, and if so, it checks if the bot's
   // look hits a wall when looking straight horizontally. If so, then the bot might be
   // able to duck over something to pass by ; if not, then the bot might be able to
   // jump over the obstacle ; we do the appropriate checks.

   TraceResult tr1, tr2, tr3, tr4, tr5;
   vector v_zoffset;
   referential_t referential;

   if (IsNull (pBot->pEntity))
      return; // reliability check

   // is bot ducking or under water ?
   if ((pBot->BotLegs.input_buttons.f_duck_time > CurrentTime ()) || (WaterLevelOf (pBot->pEntity) == WATERLEVEL_COMPLETELY))
      v_zoffset = vector (0, 0, -1); // if so, offset one unit down from origin
   else
      v_zoffset = vector (0, 0, -19); // else offset 19 units down as bot is standing

   pBot->BotBody.hit_state = OBSTACLE_NONE; // reset the hit state bitmap first
   referential = ReferentialOfAngles (ViewAnglesOf (pBot->pEntity)); // build base vectors

   // check on the left

   // do a trace 18 units higher than the max stair height left...
   UTIL_TraceLine (OriginOf (pBot->pEntity) + v_zoffset + vector (0, 0, 18),
                   OriginOf (pBot->pEntity) + v_zoffset + vector (0, 0, 18) - (referential.v_right * 90),
                   ignore_monsters, pBot->pEntity, &tr1);

   // do a trace from the eyes position left...
   UTIL_TraceLine (EyeOriginOf (pBot->pEntity),
                   EyeOriginOf (pBot->pEntity) - (referential.v_right * 90),
                   ignore_monsters, pBot->pEntity, &tr2);

   // do a trace one unit lower than the max stair height left...
   UTIL_TraceLine (OriginOf (pBot->pEntity) + v_zoffset,
                   OriginOf (pBot->pEntity) + v_zoffset - (referential.v_right * 90),
                   ignore_monsters, pBot->pEntity, &tr3);

   // is there something in the way at feet level that is not a slope AND nothing in the way at eye level ?
   if ((tr3.flFraction < 1.0) && (tr3.vecPlaneNormal.z < 0.5) && (tr2.flFraction == 1.0))
      pBot->BotBody.hit_state |= OBSTACLE_LEFT_LOWWALL; // bot can jump over this obstacle

   // is there something in the way at eye level AND nothing in the way at knee level ?
   if ((tr2.flFraction < 1.0) && (tr1.flFraction == 1.0))
      pBot->BotBody.hit_state |= OBSTACLE_LEFT_LOWCEILING; // bot can duck under this obstacle

   // is there something in the way at eye level AND something in the way at knee level ?
   if ((tr2.flFraction < 1.0) && (tr1.flFraction < 1.0))
      pBot->BotBody.hit_state |= OBSTACLE_LEFT_WALL; // bot will definitely hit something

   // if the area is clear on the left at head level, trace down to check for a possible fall
   if (tr2.flFraction == 1.0)
   {
      UTIL_TraceLine (EyeOriginOf (pBot->pEntity) - (referential.v_right * 90),
                      EyeOriginOf (pBot->pEntity) - (referential.v_right * 90) + vector (0, 0, -250),
                      ignore_monsters, pBot->pEntity, &tr4);

      // did the trace hit nothing OR some water ?
      if ((tr4.flFraction == 1.0) || (ContentsOf ((vector) tr4.vecEndPos) == MATTER_WATER))
         pBot->BotBody.hit_state |= OBSTACLE_LEFT_FALL; // bot can fall on the left
   }

   // check in front

   // do a trace 18 units higher than the max stair height forward...
   UTIL_TraceLine (OriginOf (pBot->pEntity) + v_zoffset + vector (0, 0, 18),
                   OriginOf (pBot->pEntity) + v_zoffset + vector (0, 0, 18) + (referential.v_forward * 90),
                   ignore_monsters, pBot->pEntity, &tr1);

   // do a trace from the eyes position forward...
   UTIL_TraceLine (EyeOriginOf (pBot->pEntity),
                   EyeOriginOf (pBot->pEntity) + (referential.v_forward * 90),
                   ignore_monsters, pBot->pEntity, &tr2);

   // do a trace one unit lower than the max stair height forward...
   UTIL_TraceLine (OriginOf (pBot->pEntity) + v_zoffset,
                   OriginOf (pBot->pEntity) + v_zoffset + (referential.v_forward * 90),
                   ignore_monsters, pBot->pEntity, &tr3);

   // is there something in the way at feet level that is not a slope AND nothing in the way at eye level ?
   if ((tr3.flFraction < 1.0) && (tr3.vecPlaneNormal.z < 0.5) && (tr2.flFraction == 1.0))
      pBot->BotBody.hit_state |= OBSTACLE_FRONT_LOWWALL; // bot can jump over this obstacle

   // is there something in the way at eye level AND nothing in the way at knee level ?
   if ((tr2.flFraction < 1.0) && (tr1.flFraction == 1.0))
      pBot->BotBody.hit_state |= OBSTACLE_FRONT_LOWCEILING; // bot can duck under this obstacle

   // is there something in the way at eye level AND something in the way at knee level ?
   if ((tr2.flFraction < 1.0) && (tr1.flFraction < 1.0))
      pBot->BotBody.hit_state |= OBSTACLE_FRONT_WALL; // bot will definitely hit something

   // if the area is clear in front at head level, trace down to check for a possible fall
   if (tr2.flFraction == 1.0)
   {
      UTIL_TraceLine (EyeOriginOf (pBot->pEntity) + (referential.v_forward * 90),
                      EyeOriginOf (pBot->pEntity) + (referential.v_forward * 90) + vector (0, 0, -250),
                      ignore_monsters, pBot->pEntity, &tr4);

      // did the trace hit nothing OR some water ?
      if ((tr4.flFraction == 1.0) || (ContentsOf ((vector) tr4.vecEndPos) == MATTER_WATER))
      {
         pBot->BotBody.hit_state |= OBSTACLE_FRONT_FALL; // bot can fall in front
         pBot->f_reach_time = CurrentTime () + 1.5; // reflex : don't reach point for 1.5 second

         // trace backwards in front of the bot 17 units down to find the edge plane
         UTIL_TraceLine (EyeOriginOf (pBot->pEntity) + vector (0, 0, -80) + referential.v_forward * 90,
                         EyeOriginOf (pBot->pEntity) + vector (0, 0, -80) + referential.v_forward * 90 - referential.v_forward * 300,
                         ignore_monsters, pBot->pEntity, &tr5);

         // did the trace hit something ?
         if (tr5.flFraction < 1.0)
            pBot->BotBody.v_fall_plane_normal = (vector) tr5.vecPlaneNormal; // if so, then we found the edge plane
         else
         {
            // Houston, we have a problem. The bot is about to fall but we did NOT found the
            // edge plane. Make it jump as a reflex to reach the opposite side (if any)

            pBot->BotLegs.input_buttons.f_forward_time = CurrentTime () + 60.0; // run forward
            pBot->BotLegs.input_buttons.f_walk_time = 0;
            pBot->BotLegs.input_buttons.f_jump_time = CurrentTime () + 0.3; // banzaiii...
         }
      }
      else
         pBot->BotBody.v_fall_plane_normal = NULLVEC; // else no fall, so reset the edge plane
   }

   // check on the right

   // do a trace 18 units higher than the max stair height right...
   UTIL_TraceLine (OriginOf (pBot->pEntity) + v_zoffset + vector (0, 0, 18),
                   OriginOf (pBot->pEntity) + v_zoffset + vector (0, 0, 18) + (referential.v_right * 90),
                   ignore_monsters, pBot->pEntity, &tr1);

   // do a trace from the eyes position right...
   UTIL_TraceLine (EyeOriginOf (pBot->pEntity),
                   EyeOriginOf (pBot->pEntity) + (referential.v_right * 90),
                   ignore_monsters, pBot->pEntity, &tr2);

   // do a trace one unit lower than the max stair height right...
   UTIL_TraceLine (OriginOf (pBot->pEntity) + v_zoffset,
                   OriginOf (pBot->pEntity) + v_zoffset + (referential.v_right * 90),
                   ignore_monsters, pBot->pEntity, &tr3);

   // is there something in the way at feet level that is not a slope AND nothing in the way at eye level ?
   if ((tr3.flFraction < 1.0) && (tr3.vecPlaneNormal.z < 0.5) && (tr2.flFraction == 1.0))
      pBot->BotBody.hit_state |= OBSTACLE_RIGHT_LOWWALL; // bot can jump over this obstacle

   // is there something in the way at eye level AND nothing in the way at knee level ?
   if ((tr2.flFraction < 1.0) && (tr1.flFraction == 1.0))
      pBot->BotBody.hit_state |= OBSTACLE_RIGHT_LOWCEILING; // bot can duck under this obstacle

   // is there something in the way at eye level AND something in the way at knee level ?
   if ((tr2.flFraction < 1.0) && (tr1.flFraction < 1.0))
      pBot->BotBody.hit_state |= OBSTACLE_RIGHT_WALL; // bot will definitely hit something

   // if the area is clear on the right at head level, trace down to check for a possible fall
   if (tr2.flFraction == 1.0)
   {
      UTIL_TraceLine (EyeOriginOf (pBot->pEntity) + (referential.v_right * 90),
                      EyeOriginOf (pBot->pEntity) + (referential.v_right * 90) + vector (0, 0, -250),
                      ignore_monsters, pBot->pEntity, &tr4);

      // did the trace hit nothing OR some water ?
      if ((tr4.flFraction == 1.0) || (ContentsOf ((vector) tr4.vecEndPos) == MATTER_WATER))
         pBot->BotBody.hit_state |= OBSTACLE_RIGHT_FALL; // bot can fall on the right
   }

   if (DebugLevel.body > 0)
      printf ("%s senses: 0%s%s%s%s%s%s%s%s%s%s%s%s\n", NetnameOf (pBot->pEntity),
              (pBot->BotBody.hit_state & OBSTACLE_LEFT_LOWWALL ? " | L_LOWWALL" : ""),
              (pBot->BotBody.hit_state & OBSTACLE_LEFT_LOWCEILING ? " | L_LOWCEIL" : ""),
              (pBot->BotBody.hit_state & OBSTACLE_LEFT_WALL ? " | L_WALL" : ""),
              (pBot->BotBody.hit_state & OBSTACLE_LEFT_FALL ? " | L_FALL" : ""),
              (pBot->BotBody.hit_state & OBSTACLE_FRONT_LOWWALL ? " | F_LOWWALL" : ""),
              (pBot->BotBody.hit_state & OBSTACLE_FRONT_LOWCEILING ? " | F_LOWCEIL" : ""),
              (pBot->BotBody.hit_state & OBSTACLE_FRONT_WALL ? " | F_WALL" : ""),
              (pBot->BotBody.hit_state & OBSTACLE_FRONT_FALL ? " | F_FALL" : ""),
              (pBot->BotBody.hit_state & OBSTACLE_RIGHT_LOWWALL ? " | R_LOWWALL" : ""),
              (pBot->BotBody.hit_state & OBSTACLE_RIGHT_LOWCEILING ? " | R_LOWCEIL" : ""),
              (pBot->BotBody.hit_state & OBSTACLE_RIGHT_WALL ? " | R_WALL" : ""),
              (pBot->BotBody.hit_state & OBSTACLE_RIGHT_FALL ? " | R_FALL" : ""));

   return;
}


void BotCheckIfStuck (bot_t *pBot)
{
   // this function checks if the bot doesn't move as fast as it would like to. It is called
   // every frame to do a periodic check ; then, by comparing the bot's current position and
   // the previously recorded one, and regarding to the contents of the BotLegs structure,
   // this function can tell if the bot is stuck or not.

   int index;

   if (IsNull (pBot->pEntity))
      return; // reliability check

   if (pBot->f_check_stuck_time > CurrentTime ())
      return; // cancel if not time to check yet

   // has bot not moved as fast he wanted ?
   if (((OriginOf (pBot->pEntity) - pBot->v_prev_position).Length () < 5) && (pBot->BotLegs.movement_speed.Length () > 0))
   {
      pBot->b_is_stuck = TRUE; // set stuck flag

      // if debug mode is enabled, tell the developer that our bot is getting stuck
      if (DebugLevel.body > 0)
         printf ("ALERT - BOT \"%s\" IS STUCK\n", NetnameOf (pBot->pEntity));
   }

   // 2nd check: is there no visibility in here ?
   for (index = 0; index < 52; index++)
      if (pBot->BotEyes.BotFOV[index].distance > 80)
         break; // break when visibility found

   // no visibility ?
   if (index == 52)
   {
      pBot->b_is_stuck = TRUE; // set stuck flag

      // if debug mode is enabled, tell the developer that our bot is getting stuck
      if (DebugLevel.body > 0)
         printf ("ALERT - BOT \"%s\" IS STUCK\n", NetnameOf (pBot->pEntity));
   }

   pBot->v_prev_position = OriginOf (pBot->pEntity); // save current position
   pBot->f_check_stuck_time = CurrentTime () + 1.0; // check again in one second
   return;
}
