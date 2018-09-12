// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// bot_body.cpp
//

#include "racc.h"


void BotTouch (player_t *pPlayer)
{
   // this function makes the bot aware of its immediate surroundings, building an integer bitmap
   // representing the layout of the terrain right around the bot, and realizing if its movement
   // doesn't perform as expected (check if stuck), which is a feedback from the motile AI.

   if (DebugLevel.body_disabled)
      return; // return if we don't want the AI to feel

   BotCheckForObstacles (pPlayer); // identify any obstacle in front of us
   BotCheckIfStuck (pPlayer); // realize if we are stuck or not
}


void BotCheckForObstacles (player_t *pPlayer)
{
   // this function fills an integer bitmap describing the presence and quality of any
   // obstacle right around the bot. Useful for low walls over which the bot has to jump,
   // or for determining if the bot should duck to pass a low ceiling. This function is
   // called every frame, systematically, in BotSense(), so that the bot knows, when
   // it starts thinking, the quality of the terrain in front of it. First it checks if
   // it is about to hit something when walking forward, and if so, it checks if the bot's
   // look hits a wall when looking straight horizontally. If so, then the bot might be
   // able to duck over something to pass by ; if not, then the bot might be able to
   // jump over the obstacle ; we do the appropriate checks.

   // NOTE: player's origin is 37 units above the ground (standing height)
   //       player is 36 units high when ducking
   //       max stair height is 18 units
   //       max jump height is 45 units
   //       max jump height is 63 units when doing a duck-jump

   // NOTE(bis): TraceHull() fails when too close to obstacle but TraceLine doesn't.
   //            TraceLine() fails on func_illusionaries but TraceHull doesn't.
   //            Because of this strange behaviour we need to combine both:
   //            IF either TraceLine OR TraceHull fails, there is an obstacle.

   test_result_t trl1, trl2, trl3, trl4, trl5;
   Vector v_feet;
   float angle_diff_in_radians, check_distance;
   float left_check_dst, front_check_dst, right_check_dst;
   Vector v_left, v_front, v_right;
   bot_body_t *pBotBody;

   pBotBody = &pPlayer->Bot.BotBody; // quick access to bot body

   pBotBody->prev_hit_state = pBotBody->hit_state; // rotate the hit state
   pBotBody->hit_state = OBSTACLE_NONE; // first off, reset the hit state bitmap...
   pBotBody->v_fall_plane_normal = g_vecZero; // ...and the fall edge plane

   // given the bot's velocity, bot will check closer or further forward (range 32-120 units)
   check_distance = 0.4 * pPlayer->v_velocity.Length2D ();
   angle_diff_in_radians = (VecToAngles (pPlayer->v_velocity).y - pPlayer->v_angle.y) * MATH_PI / 180;

   // we gotta transpose this distance to both sides of the bot (and also forward)

   // left distance
   left_check_dst = check_distance * sin (angle_diff_in_radians);
   if (left_check_dst < GameConfig.bb_width)
      left_check_dst = GameConfig.bb_width; // bound it to 32 units minimum
   else if (left_check_dst > 120)
      left_check_dst = 120; // and 120 units maximum

   // forward distance
   front_check_dst = check_distance * cos (angle_diff_in_radians);
   if (front_check_dst < GameConfig.bb_depth)
      front_check_dst = GameConfig.bb_depth; // bound it to 32 units minimum
   else if (front_check_dst > 120)
      front_check_dst = 120; // and 120 units maximum

   // right distance
   right_check_dst = check_distance * -sin (angle_diff_in_radians);
   if (right_check_dst < GameConfig.bb_width)
      right_check_dst = GameConfig.bb_width; // bound it to 32 units minimum
   else if (right_check_dst > 120)
      right_check_dst = 120; // and 120 units maximum

   // and save them away
   pBotBody->left_check_dst = left_check_dst;
   pBotBody->front_check_dst = front_check_dst;
   pBotBody->right_check_dst = right_check_dst;

   // now build the unary vectors
   v_left = -pPlayer->v_right;
   v_left.z = 0;
   v_left = v_left.Normalize ();
   v_front = pPlayer->v_forward;
   v_front.z = 0;
   v_front = v_front.Normalize ();
   v_right = -v_left;

   // get the bot's feet position
   v_feet = pPlayer->v_origin; // take the origin...
   v_feet.z = pPlayer->pEntity->v.absmin.z; // and lower it at the bottom of the bounding box

   // if body debug level is high, display this bot's bounding box as well as the trace rays
   if (pPlayer->is_watched && (DebugLevel.body > 2))
      UTIL_DrawBox (pPlayer->pEntity->v.absmin, pPlayer->pEntity->v.absmax, 1, 0, 255, 0);

   ///////////////////////
   // check on the left //
   ///////////////////////

   // do a trace 17 units above max stair height left...
   trl1 = PlayerTestLine (pPlayer,
                          v_feet + Vector (0, 0, 18 + 17),
                          v_feet + Vector (0, 0, 18 + 17) + (v_left * 200));
   if (pPlayer->is_watched && (DebugLevel.body > 1))
      UTIL_DrawLine (v_feet + Vector (0, 0, 18 + 17),
                     v_feet + Vector (0, 0, 18 + 17) + (v_left * left_check_dst),
                     1, 0, 255, 0);

   // do a trace one unit above max jump height left...
   trl2 = PlayerTestLine (pPlayer,
                          v_feet + Vector (0, 0, 64),
                          v_feet + Vector (0, 0, 64) + (v_left * 200));
   if (pPlayer->is_watched && (DebugLevel.body > 1))
      UTIL_DrawLine (v_feet + Vector (0, 0, 64),
                     v_feet + Vector (0, 0, 64) + (v_left * left_check_dst),
                     1, 0, 255, 0);

   // do a trace one unit lower than max stair height left...
   trl3 = PlayerTestLine (pPlayer,
                          v_feet + Vector (0, 0, 17),
                          v_feet + Vector (0, 0, 17) + (v_left * 200));
   if (pPlayer->is_watched && (DebugLevel.body > 1))
      UTIL_DrawLine (v_feet + Vector (0, 0, 17),
                     v_feet + Vector (0, 0, 17) + (v_left * left_check_dst),
                     1, 0, 255, 0);

   // is there something in the way at feet level that is not a slope
   // AND nothing in the way at eye level ?
   if ((200 * trl3.fraction < left_check_dst) && (trl3.v_normal.z < 0.5)
       && (200 * trl2.fraction >= left_check_dst))
   {
      // if bot is standing on the ground, OR obstacle WAS jumpable when bot was last on ground
      if (pPlayer->environment == ENVIRONMENT_GROUND)
         pBotBody->hit_state |= OBSTACLE_LEFT_LOWWALL; // then it's a low wall
      else if (pBotBody->prev_hit_state & OBSTACLE_LEFT_LOWWALL)
         pBotBody->hit_state |= OBSTACLE_LEFT_LOWWALL; // then it's a low wall
      else
         pBotBody->hit_state |= OBSTACLE_LEFT_WALL; // else it's still a wall
   }

   // is there something in the way at eye level AND nothing in the way at knee level
   // AND the bot is standing on the ground ?
   if ((200 * trl1.fraction < left_check_dst) && (200 * trl3.fraction >= left_check_dst)
       && (pPlayer->environment == ENVIRONMENT_GROUND))
      pBotBody->hit_state |= OBSTACLE_LEFT_LOWCEILING; // bot can duck under this obstacle

   // is there something in the way at eye level AND something in the way at knee level
   // OR something in the way at eye level that is an unclimbable slope ?
   if (((200 * trl2.fraction < left_check_dst) && (200 * trl1.fraction < left_check_dst))
       || ((200 * trl2.fraction >= 60) && (trl2.v_normal.z > 0) && (trl2.v_normal.z < 0.5)))
      pBotBody->hit_state |= OBSTACLE_LEFT_WALL; // bot will definitely hit something

   // if the area is clear on the left at head level, trace down to check for a possible fall
   if (200 * trl2.fraction >= left_check_dst)
   {
      trl4 = PlayerTestHull (pPlayer,
                             v_feet + Vector (0, 0, 64) + (v_left * left_check_dst),
                             v_feet + Vector (0, 0, 64) + (v_left * left_check_dst) + Vector (0, 0, -250),
                             TRUE);
      if (pPlayer->is_watched && (DebugLevel.body > 1))
         UTIL_DrawLine (v_feet + Vector (0, 0, 64) + (v_left * left_check_dst),
                        v_feet + Vector (0, 0, 64) + (v_left * left_check_dst) + (Vector (0, 0, -250) * trl4.fraction),
                        1, 0, 255, 0);

      // did the trace hit nothing OR some water ?
      if ((trl4.fraction == 1.0) || (POINT_CONTENTS (trl4.v_endposition) == CONTENTS_WATER))
         pBotBody->hit_state |= OBSTACLE_LEFT_FALL; // bot can fall on the left
   }

   ////////////////////
   // check in front //
   ////////////////////

   // do a trace 17 units above max stair height forward...
   trl1 = PlayerTestLine (pPlayer,
                          v_feet + Vector (0, 0, 18 + 17),
                          v_feet + Vector (0, 0, 18 + 17) + (v_front * 200));
   if (pPlayer->is_watched && (DebugLevel.body > 1))
      UTIL_DrawLine (v_feet + Vector (0, 0, 18 + 17),
                     v_feet + Vector (0, 0, 18 + 17) + (v_front * front_check_dst),
                     1, 0, 255, 0);

   // do a trace one unit above max jump height forward...
   trl2 = PlayerTestLine (pPlayer,
                          v_feet + Vector (0, 0, 64),
                          v_feet + Vector (0, 0, 64) + (v_front * 200));
   if (pPlayer->is_watched && (DebugLevel.body > 1))
      UTIL_DrawLine (v_feet + Vector (0, 0, 64),
                     v_feet + Vector (0, 0, 64) + (v_front * front_check_dst),
                     1, 0, 255, 0);

   // do a trace one unit lower than max stair height forward...
   trl3 = PlayerTestLine (pPlayer,
                          v_feet + Vector (0, 0, 17),
                          v_feet + Vector (0, 0, 17) + (v_front * 200));
   if (pPlayer->is_watched && (DebugLevel.body > 1))
      UTIL_DrawLine (v_feet + Vector (0, 0, 17),
                     v_feet + Vector (0, 0, 17) + (v_front * front_check_dst),
                     1, 0, 255, 0);

   // is there something in the way at feet level that is not a slope
   // AND nothing in the way at eye level ?
   if ((200 * trl3.fraction < front_check_dst) && (trl3.v_normal.z < 0.5)
       && (200 * trl2.fraction >= front_check_dst))
   {
      // if bot is standing on the ground, OR obstacle WAS jumpable when bot was last on ground
      if (pPlayer->environment == ENVIRONMENT_GROUND)
         pBotBody->hit_state |= OBSTACLE_FRONT_LOWWALL; // then it's a low wall
      else if (pBotBody->prev_hit_state & OBSTACLE_FRONT_LOWWALL)
         pBotBody->hit_state |= OBSTACLE_FRONT_LOWWALL; // then it's a low wall
      else
         pBotBody->hit_state |= OBSTACLE_FRONT_WALL; // else it's still a wall
   }

   // is there something in the way at eye level AND nothing in the way at knee level
   // AND the bot is standing on the ground ?
   if ((200 * trl1.fraction < front_check_dst) && (200 * trl3.fraction >= front_check_dst)
       && (pPlayer->environment == ENVIRONMENT_GROUND))
      pBotBody->hit_state |= OBSTACLE_FRONT_LOWCEILING; // bot can duck under this obstacle

   // is there something in the way at eye level AND something in the way at knee level ?
   // OR something in the way at eye level that is an unclimbable slope ?
   if (((200 * trl2.fraction < front_check_dst) && (trl1.fraction * 200 < front_check_dst))
       || ((200 * trl2.fraction >= 60) && (trl2.v_normal.z > 0) && (trl2.v_normal.z < 0.5)))
      pBotBody->hit_state |= OBSTACLE_FRONT_WALL; // bot will definitely hit something

   // if the area is clear in front at head level, trace down to check for a possible fall
   if (200 * trl2.fraction >= front_check_dst)
   {
      trl4 = PlayerTestHull (pPlayer,
                             v_feet + Vector (0, 0, 64) + (v_front * front_check_dst),
                             v_feet + Vector (0, 0, 64) + (v_front * front_check_dst) + Vector (0, 0, -250),
                             TRUE);
      if (pPlayer->is_watched && (DebugLevel.body > 1))
         UTIL_DrawLine (v_feet + Vector (0, 0, 64) + (v_front * front_check_dst),
                        v_feet + Vector (0, 0, 64) + (v_front * front_check_dst) + (Vector (0, 0, -250) * trl4.fraction),
                        1, 0, 255, 0);

      // did the trace hit nothing OR some water ?
      if ((trl4.fraction == 1.0) || (POINT_CONTENTS (trl4.v_endposition) == CONTENTS_WATER))
      {
         pBotBody->hit_state |= OBSTACLE_FRONT_FALL; // bot can fall in front
         pPlayer->Bot.reach_time = server.time + 1.5; // reflex : don't reach point for 1.5 second

         // trace backwards in front of the bot 17 units down to find the edge plane
         trl5 = PlayerTestLine (pPlayer,
                                v_feet + Vector (0, 0, -10) + (v_front * front_check_dst),
                                v_feet + Vector (0, 0, -10) + (v_front * front_check_dst) + (-v_front * front_check_dst * 2));

         // did the trace hit something ?
         if (trl5.fraction < 1.0)
            pBotBody->v_fall_plane_normal = trl5.v_normal; // if so, then we found the edge plane
         else
         {
            // Houston, we have a problem. The bot is about to fall but we did NOT found the
            // edge plane. Make it jump as a reflex to reach the opposite side (if any)

            pPlayer->Bot.BotLegs.forward_time = server.time + 0.5; // run forward
            pPlayer->Bot.BotLegs.jump_time = server.time + 0.3; // banzaiii...
         }
      }
   }

   ////////////////////////
   // check on the right //
   ////////////////////////

   // do a trace 17 units above max stair height right...
   trl1 = PlayerTestLine (pPlayer,
                          v_feet + Vector (0, 0, 18 + 17),
                          v_feet + Vector (0, 0, 18 + 17) + (v_right * 200));
   if (pPlayer->is_watched && (DebugLevel.body > 1))
      UTIL_DrawLine (v_feet + Vector (0, 0, 18 + 17),
                     v_feet + Vector (0, 0, 18 + 17) + (v_right * right_check_dst),
                     1, 0, 255, 0);

   // do a trace one unit above max jump height right...
   trl2 = PlayerTestLine (pPlayer,
                          v_feet + Vector (0, 0, 64),
                          v_feet + Vector (0, 0, 64) + (v_right * 200));
   if (pPlayer->is_watched && (DebugLevel.body > 1))
      UTIL_DrawLine (v_feet + Vector (0, 0, 64),
                     v_feet + Vector (0, 0, 64) + (v_right * right_check_dst),
                     1, 0, 255, 0);

   // do a trace one unit lower than max stair height right...
   trl3 = PlayerTestLine (pPlayer,
                          v_feet + Vector (0, 0, 17),
                          v_feet + Vector (0, 0, 17) + (v_right * 200));
   if (pPlayer->is_watched && (DebugLevel.body > 1))
      UTIL_DrawLine (v_feet + Vector (0, 0, 17),
                     v_feet + Vector (0, 0, 17) + (v_right * right_check_dst),
                     1, 0, 255, 0);

   // is there something in the way at feet level that is not a slope
   // AND nothing in the way at eye level ?
   if ((200 * trl3.fraction < right_check_dst) && (trl3.v_normal.z < 0.5)
       && (200 * trl2.fraction >= right_check_dst))
   {
      // if bot is standing on the ground, OR obstacle WAS jumpable when bot was last on ground
      if (pPlayer->environment == ENVIRONMENT_GROUND)
         pBotBody->hit_state |= OBSTACLE_RIGHT_LOWWALL; // then it's a low wall
      else if (pBotBody->prev_hit_state & OBSTACLE_RIGHT_LOWWALL)
         pBotBody->hit_state |= OBSTACLE_RIGHT_LOWWALL; // then it's a low wall
      else
         pBotBody->hit_state |= OBSTACLE_RIGHT_WALL; // else it's still a wall
   }

   // is there something in the way at eye level AND nothing in the way at knee level
   // AND the bot is standing on the ground ?
   if ((200 * trl1.fraction < right_check_dst) && (200 * trl3.fraction >= right_check_dst)
       && (pPlayer->environment == ENVIRONMENT_GROUND))
      pBotBody->hit_state |= OBSTACLE_RIGHT_LOWCEILING; // bot can duck under this obstacle

   // is there something in the way at eye level AND something in the way at knee level ?
   // OR something in the way at eye level that is an unclimbable slope ?
   if (((200 * trl2.fraction < right_check_dst) && (200 * trl1.fraction < right_check_dst))
       || ((200 * trl2.fraction >= 60) && (trl2.v_normal.z > 0) && (trl2.v_normal.z < 0.5)))
      pBotBody->hit_state |= OBSTACLE_RIGHT_WALL; // bot will definitely hit something

   // if the area is clear on the right at head level, trace down to check for a possible fall
   if (200 * trl2.fraction >= right_check_dst)
   {
      trl4 = PlayerTestHull (pPlayer,
                             v_feet + Vector (0, 0, 64) + (v_right * right_check_dst),
                             v_feet + Vector (0, 0, 64) + (v_right * right_check_dst) + Vector (0, 0, -250),
                             TRUE);
      if (pPlayer->is_watched && (DebugLevel.body > 1))
         UTIL_DrawLine (v_feet + Vector (0, 0, 64) + (v_right * right_check_dst),
                        v_feet + Vector (0, 0, 64) + (v_right * right_check_dst) + (Vector (0, 0, -250) * trl4.fraction),
                        1, 0, 255, 0);

      // did the trace hit nothing OR some water ?
      if ((trl4.fraction == 1.0) || (POINT_CONTENTS (trl4.v_endposition) == CONTENTS_WATER))
         pBotBody->hit_state |= OBSTACLE_RIGHT_FALL; // bot can fall on the right
   }

   // and finally, see if we are falling or not
   if (pPlayer->v_velocity.z > -GameConfig.max_safefall_speed)
      pBotBody->fall_time = server.time; // save bot fall time

   return; // finished, surrounding obstacles bitmap is filled
}


void BotCheckIfStuck (player_t *pPlayer)
{
   // this function checks if the bot doesn't move as fast as it would like to. It is called
   // every frame to do a periodic check ; then, by comparing the bot's current position and
   // the previously recorded one, and regarding to the contents of the BotLegs structure,
   // this function can tell if the bot is stuck or not.

   pPlayer->Bot.average_velocity = pPlayer->Bot.average_velocity + pPlayer->v_velocity.Length ();
   pPlayer->Bot.average_velocity_frames_count++;

   // if bot is controlled by the player...
   if (pPlayer->Bot.is_controlled)
   {
      pPlayer->Bot.is_stuck = FALSE; // then it can't be stuck
      return;
   }

   // if bot was previously known as stuck and something new happened...
   if (pPlayer->Bot.is_stuck
       && ((pPlayer->Bot.pause_time > server.time)
           || (pPlayer->Bot.BotBrain.bot_task == BOT_TASK_CAMP)
           || (pPlayer->Bot.BotEyes.blinded_time > server.time)))
      pPlayer->Bot.is_stuck = FALSE; // this bot can no longer be considered as "stuck"

   if (pPlayer->Bot.check_stuck_time > server.time)
      return; // cancel if can't tell yet

   // now compute the real average
   pPlayer->Bot.average_velocity = pPlayer->Bot.average_velocity / pPlayer->Bot.average_velocity_frames_count;

   // if bot definitely doesn't move as fast as it would like to...
   if ((pPlayer->Bot.pause_time < server.time)
       && (pPlayer->Bot.BotBrain.bot_task != BOT_TASK_CAMP)
       && (pPlayer->Bot.BotEyes.blinded_time < server.time)
       && ((pPlayer->Bot.BotLegs.forward_speed != 0)
           || (pPlayer->Bot.BotLegs.strafe_speed != 0))
       && (pPlayer->Bot.average_velocity < 2))
   {
      pPlayer->Bot.is_stuck = TRUE; // set stuck flag and assume bot is stuck

      if (pPlayer->is_watched)
         printf ("ALERT, %s IS STUCK!!!!\n", pPlayer->connection_name);
//      SERVER_COMMAND ("pause\n");
   }
   else
      pPlayer->Bot.is_stuck = FALSE; // else consider bot is not stuck

   pPlayer->Bot.average_velocity = 0; // reset average velocity
   pPlayer->Bot.average_velocity_frames_count = 0; // reset frames count
   pPlayer->Bot.check_stuck_time = server.time + 0.5; // next check in 0.5 second
   return;
}
