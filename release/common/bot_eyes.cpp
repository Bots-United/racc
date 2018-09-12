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
// bot_eyes.cpp
//

#include "racc.h"


void BotSee (bot_t *pBot)
{
   // this is the function that makes the bot see. It fires bursts of TraceLines, incrementing
   // the trace angle each iteration, and store the results of the scan in an array, which
   // median index corresponds to the direction the bot is facing. This is a logic modeling
   // of the human eye. Unfortunately, the processing power required for a 3D scan is too high,
   // so here we just do a flat scan. The idea is not perfect, though. The list of potential
   // entities in sight is built from the engine's own list, then each of them is replaced in
   // its context, i.e. a corresponding is made between it and the right scan angle.
   // And finally, the bot keeps an eye on the chat messages on its screen...

   float angle, distance = 0;
   int fov_index = 0, entity_index = 0, entity_count = 0;
   Vector v_forward, v_groundslope, v_originalangle, v_viewangle;
   TraceResult tr1, tr2, tr3;
//   edict_t *pEntity;

   if (DebugLevel.eyes_disabled)
      return; // return if we don't want the AI to see

   if (pBot->BotEyes.blinded_time > *server.time)
      return; // return if bot is blinded

   // ask the engine for the entity list

   // erase the previous array
   memset (pBot->BotEyes.pEntitiesInSight, NULL, sizeof (pBot->BotEyes.pEntitiesInSight));

   // cycle through all entities in game
/*   for (entity_index = 0; entity_index < gpGlobals->maxEntities; entity_index++)
   {
      pEntity = INDEXENT (entity_index); // bind that entity number
      if (FNullEnt (pEntity))
         continue; // if unregistered, skip it

      if (BotCanSeeOfEntity (pBot, pEntity) == g_vecZero)
         continue; // if not in sight, discard this item

      pBot->BotEyes.pEntitiesInSight[entity_count] = pEntity; // bot can see this entity

      entity_count++;
      if (entity_count == BOT_EYE_SENSITIVITY)
         break; // if too much entities in sight, stop
   }*/

   if (pBot->pEdict->v.deadflag == DEAD_DEAD)
      return; // don't go further if bot is dead (saves tracelines)

   if (pBot->BotEyes.sample_time > *server.time)
      return; // don't go further if not time to sample the field of view yet

   // figure out the ground slope
   v_forward = Vector (pBot->BotAim.v_forward.x, pBot->BotAim.v_forward.y, 0); // build base vector
   UTIL_TraceLine (pBot->pEdict->v.origin,
                   pBot->pEdict->v.origin + Vector (0, 0, -9999),
                   ignore_monsters, pBot->pEdict, &tr1);
   UTIL_TraceLine (pBot->pEdict->v.origin + (v_forward * 15),
                   pBot->pEdict->v.origin + (v_forward * 15) + Vector (0, 0, -9999),
                   ignore_monsters, pBot->pEdict, &tr2);
   v_groundslope = tr2.vecEndPos - tr1.vecEndPos;

   // now figure out a corresponding angle and clamp its X component (pitch) in bounds
   v_originalangle = UTIL_VecToAngles (v_groundslope);
   v_originalangle.x = -v_originalangle.x * 0.75; // fakeclient aim bug fix + temperate pitch
   if (v_originalangle.x < -45)
      v_originalangle.x = -45;
   else if (v_originalangle.x > 45)
      v_originalangle.x = 45;

   // scan 100 degrees of bot's field of view...
   for (angle = -50; angle <= 50; angle += (angle * angle) / 1500 + 1.5)
   {
      v_viewangle = v_originalangle; // restore bot's current v_angle
      v_viewangle.y = WrapAngle (v_viewangle.y + angle); // pan it from left to right
      MAKE_VECTORS (v_viewangle); // build base vectors in that direction

      // trace line slightly under eyes level
      UTIL_TraceLine (pBot->BotAim.v_eyeposition,
                      pBot->BotAim.v_eyeposition + (gpGlobals->v_forward * 10000),
                      ignore_monsters, pBot->pEdict, &tr1);

      // if debug mode is enabled, draw the field of view of this bot
      if ((DebugLevel.eyes > 1) && IsValidPlayer (pListenserverEntity) && ((pBot->pEdict->v.origin - pListenserverEntity->v.origin).Length () <= 100))
         UTIL_DrawLine (pListenserverEntity,
                        pBot->BotAim.v_eyeposition,
                        pBot->BotAim.v_eyeposition + ((gpGlobals->v_forward * 10000) * tr1.flFraction),
                        1, 255, 0, 0);

      distance = tr1.flFraction * 10000; // store distance to obstacle

      // if this plane is a slope that is smooth enough for bot to climb it or a slope that goes down...
      if ((AngleOfVectors (tr1.vecPlaneNormal, Vector (0, 0, 1)) < 30) || (AngleOfVectors (tr1.vecPlaneNormal, Vector (0, 0, -1)) < 30))
      {
         // trace line parallel to previous starting a bit lower to get a new hit point
         UTIL_TraceLine (pBot->BotAim.v_eyeposition + Vector (0, 0, -5),
                         (pBot->BotAim.v_eyeposition + Vector (0, 0, -5)) + (gpGlobals->v_forward * 10000),
                         ignore_monsters, pBot->pEdict, &tr2);

         // compute a normalized vector parallel to slope
         Vector v_parallel = (tr1.vecEndPos - tr2.vecEndPos).Normalize ();

         // trace line parallel to slope so that the bot 'sees' up or down the slope
         UTIL_TraceLine (pBot->BotAim.v_eyeposition + (((gpGlobals->v_forward * 10000) * tr1.flFraction) * 0.99),
                         (pBot->BotAim.v_eyeposition + (((gpGlobals->v_forward * 10000) * tr1.flFraction) * 0.99)) + (v_parallel * 10000),
                         ignore_monsters, pBot->pEdict, &tr3);

         // if debug mode is enabled, draw the field of view of this bot
         if ((DebugLevel.eyes > 0) && IsValidPlayer (pListenserverEntity) && ((pBot->pEdict->v.origin - pListenserverEntity->v.origin).Length () <= 100))
            UTIL_DrawLine (pListenserverEntity,
                           pBot->BotAim.v_eyeposition + (((gpGlobals->v_forward * 10000) * tr1.flFraction) * 0.99),
                           (pBot->BotAim.v_eyeposition + (((gpGlobals->v_forward * 10000) * tr1.flFraction) * 0.99)) + ((v_parallel * 10000) * tr3.flFraction),
                           1, 255, 0, 0);

         // add distance from this traceline to the first one so that bot 'sees' up or down the slope
         distance += tr3.flFraction * 10000;
      }

/*    --- THIS DOES NOT WORK ---
      // else if plane is vertical and not too high, check if plane is a stair
      else if ((tr1.vecPlaneNormal.z == 0) && (tr1.pHit->v.size.z < 18))
      {
         // trace line at eyes level less maximal stair height (18)
         UTIL_TraceLine (pBot->BotAim.v_eyeposition + Vector (0, 0, -18),
                         (pBot->BotAim.v_eyeposition + Vector (0, 0, -18)) + (gpGlobals->v_forward * 10000),
                         ignore_monsters, pBot->pEdict, &tr2);

         // if two both not same length and relatively close together, bot may have found a staircase
         if ((tr2.flFraction < tr1.flFraction)
             && (((tr2.flFraction - tr1.flFraction) * 10000 < 50)
                 && ((tr2.flFraction - tr1.flFraction) * 10000 > 10)))
         {
            // compute a normalized vector parallel to staircase slope
            Vector v_parallel = (tr1.vecEndPos - tr2.vecEndPos).Normalize ();

            // trace line parallel to staircase slope so that the bot 'sees' up the staircase
            UTIL_TraceLine (pBot->BotAim.v_eyeposition + (((gpGlobals->v_forward * 10000) * tr1.flFraction) * 0.80),
                            (pBot->BotAim.v_eyeposition + (((gpGlobals->v_forward * 10000) * tr1.flFraction) * 0.80)) + (v_parallel * 10000),
                            ignore_monsters, pBot->pEdict, &tr3);

            // if debug mode is enabled, draw the field of view of this bot
            if ((DebugLevel.eyes > 0) && IsValidPlayer (pListenserverEntity) && ((pBot->pEdict->v.origin - pListenserverEntity->v.origin).Length () <= 100))
               UTIL_DrawBeam (pListenserverEntity,
                              (pBot->BotAim.v_eyeposition + (((gpGlobals->v_forward * 10000) * tr1.flFraction) * 0.80),
                              (pBot->BotAim.v_eyeposition + (((gpGlobals->v_forward * 10000) * tr1.flFraction) * 0.80)) + ((v_parallel * 10000) * tr3.flFraction),
                              1, 5, 0, 255, 0, 0, 255, 0);

            // add distance from this traceline to the first one so that bot 'sees' up the staircase
            distance += tr3.flFraction * 10000;
         }
      }
      --- THIS DOES NOT WORK ---
*/

      // store the results of the scan in bot's FOV array
      pBot->BotEyes.BotFOV[fov_index].scan_angles = v_viewangle;
      pBot->BotEyes.BotFOV[fov_index].distance = distance;
      pBot->BotEyes.BotFOV[fov_index].vecEndPos = tr1.vecEndPos;
      pBot->BotEyes.BotFOV[fov_index].Normal = tr1.vecPlaneNormal;
      pBot->BotEyes.BotFOV[fov_index].pHit = tr1.pHit;

      fov_index++; // increment FOV array index
   }

   // we just can't match up the human eye's refresh rate... bots will sample less often
   pBot->BotEyes.sample_time = *server.time + 0.20; // next sampling in 200 ms
   return;
}


Vector BotCanSeeOfEntity (bot_t *pBot, edict_t *pEntity)
{
   // this function returns the best vector the bot pointed to by pBot can see of entity pEntity,
   // and in case pEntity is definitely not visible from the bot's point of view, it returns a
   // null vector. Either the entity is a player, or a bounded entity, or a point-based entity.
   // if it's a player, checks are made on different points of the player's body, starting from
   // the most tactically important one (the head) to the less tactically important ones, in
   // order to determine if the player is visible and WHAT is visible of this player ; if it's
   // an entity which has a bounding box, we check whether either the center of it, or any of
   // its four corners on a flattened referential, are visible, and if none of these points are,
   // we assume that the entity isn't visible at all. Else if it's a point-based entity, all we
   // can check is the visibility of the center (origin) of this entity. In case some part of
   // pEntity is visible from the bot's point of view, the vector position of the best visible
   // point of pEntity is returned.

   // note: 0x101 = (ignore_monsters, ignore_glass)

   static TraceResult tr;
   static Vector v_source, v_targetorigin, v_targetside, v_targetangles;

   if (FNullEnt (pBot->pEdict) || FNullEnt (pEntity))
      return (g_vecZero); // reliability check

   v_source = pBot->BotAim.v_eyeposition; // compute source vector location

   // is the target entity a player ?
   if (pEntity->v.flags & FL_CLIENT)
   {
      if ((pEntity->v.effects & EF_NODRAW) || (pEntity->v.model == NULL))
         return (g_vecZero); // if invisible (such as spectator), bot sees nothing

      // always look for the player's head first
      v_targetorigin = pEntity->v.origin + pEntity->v.view_ofs; // compute target head vector location
      if (IsInPlayerFOV (pBot->pEdict, v_targetorigin))
      {
         TRACE_LINE (v_source, v_targetorigin, 0x101, pEntity, &tr);
         if (tr.flFraction == 1.0)
            return (v_targetorigin); // head of target player is visible
      }

      // else look for the player's waist, ask engine if bone number is known
      if (playerbones.pelvis != 0)
         GET_BONE_POSITION (pEntity, playerbones.pelvis, v_targetorigin, v_targetangles);
      else
         v_targetorigin = pEntity->v.origin; // if bone number is unknown, take the entity origin
      if (IsInPlayerFOV (pBot->pEdict, v_targetorigin))
      {
         TRACE_LINE (v_source, v_targetorigin, 0x101, pEntity, &tr);
         if (tr.flFraction == 1.0)
            return (v_targetorigin); // body of target player is visible
      }

      // does the bot know this player bone ?
      if (playerbones.left_upperarm != 0)
      {
         // else look for the player's left arm
         GET_BONE_POSITION (pEntity, playerbones.left_upperarm, v_targetside, v_targetangles);
         if (IsInPlayerFOV (pBot->pEdict, v_targetside))
         {
            TRACE_LINE (v_source, v_targetside, 0x101, pEntity, &tr);
            if (tr.flFraction == 1.0)
               return (v_targetside); // left arm of target player is visible
         }
      }

      // does the bot know this player bone ?
      if (playerbones.right_upperarm != 0)
      {
         // else look for the player's right arm
         GET_BONE_POSITION (pEntity, playerbones.right_upperarm, v_targetside, v_targetangles);
         if (IsInPlayerFOV (pBot->pEdict, v_targetside))
         {
            TRACE_LINE (v_source, v_targetside, 0x101, pEntity, &tr);
            if (tr.flFraction == 1.0)
               return (v_targetside); // right arm of target player is visible
         }
      }

      // does the bot know this player bone ?
      if (playerbones.left_foot != 0)
      {
         // else look for the player's left foot
         GET_BONE_POSITION (pEntity, playerbones.left_foot, v_targetside, v_targetangles);
         if (IsInPlayerFOV (pBot->pEdict, v_targetside))
         {
            TRACE_LINE (v_source, v_targetside, 0x101, pEntity, &tr);
            if (tr.flFraction == 1.0)
               return (v_targetside); // left foot of target player is visible
         }
      }

      // does the bot know this player bone ?
      if (playerbones.right_foot != 0)
      {
         // else look for the player's right foot
         GET_BONE_POSITION (pEntity, playerbones.right_foot, v_targetside, v_targetangles);
         if (IsInPlayerFOV (pBot->pEdict, v_targetside))
         {
            TRACE_LINE (v_source, v_targetside, 0x101, pEntity, &tr);
            if (tr.flFraction == 1.0)
               return (v_targetside); // right foot of target player is visible
         }
      }

      return (g_vecZero); // none of these player's body parts are visible, assume player isn't
   }

   // else is the target entity a monster ?
   else if (pEntity->v.flags & FL_MONSTER)
   {
      if ((pEntity->v.effects & EF_NODRAW) || (pEntity->v.model == NULL))
         return (g_vecZero); // if invisible (invisible monsters, why not ?), bot sees nothing

      // always look for the monster's head first
      v_targetorigin = pEntity->v.origin + pEntity->v.view_ofs; // compute target head vector location
      if (IsInPlayerFOV (pBot->pEdict, v_targetorigin))
      {
         TRACE_LINE (v_source, v_targetorigin, 0x101, pEntity, &tr);
         if (tr.flFraction == 1.0)
            return (v_targetorigin); // head of target monster is visible
      }

      // then look for the monster's body, use the entity origin for that
      v_targetorigin = pEntity->v.origin; // compute target head vector location
      if (IsInPlayerFOV (pBot->pEdict, v_targetorigin))
      {
         TRACE_LINE (v_source, v_targetorigin, 0x101, pEntity, &tr);
         if (tr.flFraction == 1.0)
            return (v_targetorigin); // head of target monster is visible
      }

      return (g_vecZero); // neither the head nor the body of this monster are visible
   }

   // else has the target entity got a bounding box ?
   else if ((pEntity->v.absmin != g_vecZero) && (pEntity->v.absmax != g_vecZero))
   {
      // do some checks for determining which part of the bounding box is visible
      v_targetorigin = VecBModelOrigin (pEntity); // compute target center vector location
      if (IsInPlayerFOV (pBot->pEdict, v_targetorigin))
      {
         TRACE_LINE (v_source, v_targetorigin, 0x101, pEntity, &tr);
         if ((tr.vecEndPos.x > pEntity->v.absmin.x) && (tr.vecEndPos.x < pEntity->v.absmax.x)
             && (tr.vecEndPos.y > pEntity->v.absmin.y) && (tr.vecEndPos.y < pEntity->v.absmax.y)
             && (tr.vecEndPos.z > pEntity->v.absmin.z) && (tr.vecEndPos.z < pEntity->v.absmax.z))
            return (tr.vecEndPos); // entity is visible at hit point (center)
      }

      v_targetside = Vector (pEntity->v.absmin.x, pEntity->v.absmin.y, v_targetorigin.z);
      if (IsInPlayerFOV (pBot->pEdict, v_targetside))
      {
         TRACE_LINE (v_source, v_targetside, 0x101, pEntity, &tr);
         if ((tr.vecEndPos.x > pEntity->v.absmin.x) && (tr.vecEndPos.x < pEntity->v.absmax.x)
             && (tr.vecEndPos.y > pEntity->v.absmin.y) && (tr.vecEndPos.y < pEntity->v.absmax.y)
             && (tr.vecEndPos.z > pEntity->v.absmin.z) && (tr.vecEndPos.z < pEntity->v.absmax.z))
            return (tr.vecEndPos); // entity is visible at hit point (side 1)
      }

      v_targetside = Vector (pEntity->v.absmin.x, pEntity->v.absmax.y, v_targetorigin.z);
      if (IsInPlayerFOV (pBot->pEdict, v_targetside))
      {
         TRACE_LINE (v_source, v_targetside, 0x101, pEntity, &tr);
         if ((tr.vecEndPos.x > pEntity->v.absmin.x) && (tr.vecEndPos.x < pEntity->v.absmax.x)
             && (tr.vecEndPos.y > pEntity->v.absmin.y) && (tr.vecEndPos.y < pEntity->v.absmax.y)
             && (tr.vecEndPos.z > pEntity->v.absmin.z) && (tr.vecEndPos.z < pEntity->v.absmax.z))
            return (tr.vecEndPos); // entity is visible at hit point (side 2)
      }

      v_targetside = Vector (pEntity->v.absmax.x, pEntity->v.absmin.y, v_targetorigin.z);
      if (IsInPlayerFOV (pBot->pEdict, v_targetside))
      {
         TRACE_LINE (v_source, v_targetside, 0x101, pEntity, &tr);
         if ((tr.vecEndPos.x > pEntity->v.absmin.x) && (tr.vecEndPos.x < pEntity->v.absmax.x)
             && (tr.vecEndPos.y > pEntity->v.absmin.y) && (tr.vecEndPos.y < pEntity->v.absmax.y)
             && (tr.vecEndPos.z > pEntity->v.absmin.z) && (tr.vecEndPos.z < pEntity->v.absmax.z))
            return (tr.vecEndPos); // entity is visible at hit point (side 3)
      }

      v_targetside = Vector (pEntity->v.absmax.x, pEntity->v.absmax.y, v_targetorigin.z);
      if (IsInPlayerFOV (pBot->pEdict, v_targetside))
      {
         TRACE_LINE (v_source, v_targetside, 0x101, pEntity, &tr);
         if ((tr.vecEndPos.x > pEntity->v.absmin.x) && (tr.vecEndPos.x < pEntity->v.absmax.x)
             && (tr.vecEndPos.y > pEntity->v.absmin.y) && (tr.vecEndPos.y < pEntity->v.absmax.y)
             && (tr.vecEndPos.z > pEntity->v.absmin.z) && (tr.vecEndPos.z < pEntity->v.absmax.z))
            return (tr.vecEndPos); // entity is visible at hit point (side 4)
      }

      return (g_vecZero); // neither the center nor any of the sides of pEntity is visible
   }

   // else it's a point-based entity
   else
   {
      if ((pEntity->v.effects & EF_NODRAW) || (pEntity->v.model == NULL))
         return (g_vecZero); // if invisible (such as weapon having been picked up), bot sees nothing

      // else it's a point-based entity, check for its visibility
      v_targetorigin = pEntity->v.origin;
      if (IsInPlayerFOV (pBot->pEdict, v_targetorigin))
      {
         TRACE_LINE (v_source, v_targetorigin, 0x101, pEntity, &tr);
         if (tr.flFraction == 1.0)
            return (v_targetorigin); // if visible, return the origin of entity the engine knows
      }

      return (g_vecZero); // this point-based entity is not visible
   }

   return (g_vecZero); // assume entity is not visible
}
