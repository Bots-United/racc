// RACC - AI development project for first-person shooter games derivated from Valve's Half-Life
// (http://www.racc-ai.com/)
//
// The game to engine interfacing code is based on the work done by Jeffrey 'botman' Broome
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


void BotTouch (bot_t *pBot)
{
   // this function makes the bot aware of its immediate surroundings, building an integer bitmap
   // representing the layout of the terrain right around the bot, and realizing if its movement
   // doesn't perform as expected (check if stuck), which is a feedback from the motile AI.

   if (DebugLevel.body_disabled)
      return; // return if we don't want the AI to feel

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
   Vector v_zoffset;

   // is bot ducking or under water ?
   if ((pBot->BotMove.f_duck_time > *server.time) || (pBot->pEdict->v.waterlevel == 2))
      v_zoffset = Vector (0, 0, -1); // if so, offset one unit down from origin
   else
      v_zoffset = Vector (0, 0, -19); // else offset 19 units down as bot is standing

   pBot->BotBody.hit_state = OBSTACLE_NONE; // reset the hit state bitmap first

   // check on the left

   // do a trace 18 units higher than the max stair height left...
   UTIL_TraceLine (pBot->pEdict->v.origin + v_zoffset + Vector (0, 0, 18),
                   pBot->pEdict->v.origin + v_zoffset + Vector (0, 0, 18) - (pBot->BotAim.v_right * 90),
                   ignore_monsters, pBot->pEdict, &tr1);

   // do a trace from the eyes position left...
   UTIL_TraceLine (pBot->BotAim.v_eyeposition,
                   pBot->BotAim.v_eyeposition - (pBot->BotAim.v_right * 90),
                   ignore_monsters, pBot->pEdict, &tr2);

   // do a trace one unit lower than the max stair height left...
   UTIL_TraceLine (pBot->pEdict->v.origin + v_zoffset,
                   pBot->pEdict->v.origin + v_zoffset - (pBot->BotAim.v_right * 90),
                   ignore_monsters, pBot->pEdict, &tr3);

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
      UTIL_TraceLine (pBot->BotAim.v_eyeposition - (pBot->BotAim.v_right * 90),
                      pBot->BotAim.v_eyeposition - (pBot->BotAim.v_right * 90) + Vector (0, 0, -250),
                      ignore_monsters, pBot->pEdict, &tr4);

      // did the trace hit nothing OR some water ?
      if ((tr4.flFraction == 1.0) || (POINT_CONTENTS (tr4.vecEndPos) == CONTENTS_WATER))
         pBot->BotBody.hit_state |= OBSTACLE_LEFT_FALL; // bot can fall on the left
   }

   // check in front

   // do a trace 18 units higher than the max stair height forward...
   UTIL_TraceLine (pBot->pEdict->v.origin + v_zoffset + Vector (0, 0, 18),
                   pBot->pEdict->v.origin + v_zoffset + Vector (0, 0, 18) + (pBot->BotAim.v_forward * 90),
                   ignore_monsters, pBot->pEdict, &tr1);

   // do a trace from the eyes position forward...
   UTIL_TraceLine (pBot->BotAim.v_eyeposition,
                   pBot->BotAim.v_eyeposition + (pBot->BotAim.v_forward * 90),
                   ignore_monsters, pBot->pEdict, &tr2);

   // do a trace one unit lower than the max stair height forward...
   UTIL_TraceLine (pBot->pEdict->v.origin + v_zoffset,
                   pBot->pEdict->v.origin + v_zoffset + (pBot->BotAim.v_forward * 90),
                   ignore_monsters, pBot->pEdict, &tr3);

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
      UTIL_TraceLine (pBot->BotAim.v_eyeposition + (pBot->BotAim.v_forward * 90),
                      pBot->BotAim.v_eyeposition + (pBot->BotAim.v_forward * 90) + Vector (0, 0, -250),
                      ignore_monsters, pBot->pEdict, &tr4);

      // did the trace hit nothing OR some water ?
      if ((tr4.flFraction == 1.0) || (POINT_CONTENTS (tr4.vecEndPos) == CONTENTS_WATER))
      {
         pBot->BotBody.hit_state |= OBSTACLE_FRONT_FALL; // bot can fall in front
         pBot->f_reach_time = *server.time + 1.5; // reflex : don't reach point for 1.5 second

         // trace backwards in front of the bot 17 units down to find the edge plane
         UTIL_TraceLine (pBot->BotAim.v_eyeposition + Vector (0, 0, -80) + pBot->BotAim.v_forward * 90,
                         pBot->BotAim.v_eyeposition + Vector (0, 0, -80) + pBot->BotAim.v_forward * 90 - pBot->BotAim.v_forward * 300,
                         ignore_monsters, pBot->pEdict, &tr5);

         // did the trace hit something ?
         if (tr5.flFraction < 1.0)
            pBot->BotBody.v_fall_plane_normal = tr5.vecPlaneNormal; // if so, then we found the edge plane
         else
         {
            // Houston, we have a problem. The bot is about to fall but we did NOT found the
            // edge plane. Make it jump as a reflex to reach the opposite side (if any)

            pBot->BotMove.f_forward_time = *server.time + 0.2; // run forward
            pBot->BotMove.f_walk_time = 0;
            pBot->BotMove.f_jump_time = *server.time + 0.3; // banzaiii...
         }
      }
      else
         pBot->BotBody.v_fall_plane_normal = g_vecZero; // else no fall, so reset the edge plane
   }

   // check on the right

   // do a trace 18 units higher than the max stair height right...
   UTIL_TraceLine (pBot->pEdict->v.origin + v_zoffset + Vector (0, 0, 18),
                   pBot->pEdict->v.origin + v_zoffset + Vector (0, 0, 18) + (pBot->BotAim.v_right * 90),
                   ignore_monsters, pBot->pEdict, &tr1);

   // do a trace from the eyes position right...
   UTIL_TraceLine (pBot->BotAim.v_eyeposition,
                   pBot->BotAim.v_eyeposition + (pBot->BotAim.v_right * 90),
                   ignore_monsters, pBot->pEdict, &tr2);

   // do a trace one unit lower than the max stair height right...
   UTIL_TraceLine (pBot->pEdict->v.origin + v_zoffset,
                   pBot->pEdict->v.origin + v_zoffset + (pBot->BotAim.v_right * 90),
                   ignore_monsters, pBot->pEdict, &tr3);

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
      UTIL_TraceLine (pBot->BotAim.v_eyeposition + (pBot->BotAim.v_right * 90),
                      pBot->BotAim.v_eyeposition + (pBot->BotAim.v_right * 90) + Vector (0, 0, -250),
                      ignore_monsters, pBot->pEdict, &tr4);

      // did the trace hit nothing OR some water ?
      if ((tr4.flFraction == 1.0) || (POINT_CONTENTS (tr4.vecEndPos) == CONTENTS_WATER))
         pBot->BotBody.hit_state |= OBSTACLE_RIGHT_FALL; // bot can fall on the right
   }

   if ((DebugLevel.body > 0) && IsValidPlayer (pListenserverEntity) && ((pBot->pEdict->v.origin - pListenserverEntity->v.origin).Length () <= 100))
      printf ("%s senses: 0%s%s%s%s%s%s%s%s%s%s%s%s\n", STRING (pBot->pEdict->v.netname),
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

   // if bot is controlled by the player...
   if (pBot->is_controlled)
   {
      pBot->b_is_stuck = FALSE; // then it can't be stuck
      return;
   }

   // if bot was previously known as stuck and something new happened...
   if (pBot->b_is_stuck
       && ((pBot->f_pause_time > *server.time)
           || (pBot->f_camp_time > *server.time)
           || (pBot->BotEyes.blinded_time > *server.time)))
      pBot->b_is_stuck = FALSE; // this bot can no longer be considered as "stuck"

   if (pBot->f_check_stuck_time > *server.time)
      return; // cancel if can't tell yet

   // if bot definitely doesn't move as fast as it would like to...
   if ((pBot->f_pause_time < *server.time)
       && (pBot->f_camp_time < *server.time)
       && (pBot->BotEyes.blinded_time < *server.time)
       && ((pBot->BotMove.f_move_speed != 0) || (pBot->BotMove.f_strafe_speed != 0))
       && ((pBot->pEdict->v.velocity == g_vecZero)
           || ((pBot->pEdict->v.origin - pBot->v_prev_position).Length () < (1 / (float) server.msecval) / 10)))
      pBot->b_is_stuck = TRUE; // set stuck flag and assume bot is stuck
   else
      pBot->b_is_stuck = FALSE; // else consider bot is not stuck

   pBot->f_check_stuck_time = *server.time + 0.5; // next check in 0.5 second
   return;
}
