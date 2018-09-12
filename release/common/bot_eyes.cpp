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
// bot_eyes.cpp
//

#include "racc.h"


extern entity_t *pListenserverEntity;
extern debug_level_t DebugLevel;


void BotSee (bot_t *pBot)
{
   // this is the function that makes the bot see. It fires bursts of TraceLines, incrementing
   // the trace angle each iteration, and store the results of the scan in an array, which
   // median index corresponds to the direction the bot is facing. This is a logic modeling
   // of the human eye. Unfortunately, the processing power required for a 3D scan is too high,
   // so here we just do a flat scan, either down a slope or at flat, but never up to the sky,
   // thus trying to avoid irrelevant data. The idea is not perfect, though. The list of
   // entities in sight is also requested from the engine, then each of them is replaced in
   // its context, i.e. a corresponding is made between it and the right eye scan angle.
   // And finally, the bot keeps an eye on the chat messages on its screen...

   float angle, distance = 0, maxdistance = 0;
   int fov_index = 0, entity_index = 0, entity_count = 0;
   vector v_viewangle;
   referential_t referential;
   TraceResult tr1, tr2, tr3;
   entity_t *pEntity = NULL;

   if (IsNull (pBot->pEntity))
      return; // reliability check

   if (pBot->BotEyes.f_sample_time > CurrentTime ())
      return; // return if not time to see yet

   if (pBot->BotEyes.f_blinded_time > CurrentTime ())
      return; // return if bot is blinded

   // scan 90 degrees of bot's field of view...
   for (angle = -MAX_PLAYER_FOV / 2; angle <= MAX_PLAYER_FOV / 2; angle += (angle * angle) / 1500 + 1.5)
   {
      v_viewangle = ViewAnglesOf (pBot->pEntity); // restore bot's current v_angle
      if (v_viewangle.x > 0)
         v_viewangle.x = 0; // scan at flat rather than up in the sky !
      v_viewangle.y = WrapAngle (v_viewangle.y + angle); // pan it from left to right
      referential = ReferentialOfAngles (v_viewangle); // build base vectors in that direction

      // trace line slightly under eyes level
      UTIL_TraceLine (EyeOriginOf (pBot->pEntity),
                      (EyeOriginOf (pBot->pEntity)) + (referential.v_forward * 10000),
                      ignore_monsters, pBot->pEntity, &tr1);

      if (DebugLevel.eyes > 0)
         DrawLine (pListenserverEntity,
                   EyeOriginOf (pBot->pEntity),
                   (EyeOriginOf (pBot->pEntity)) + ((referential.v_forward * 10000) * tr1.flFraction),
                   2, 255, 0, 0);

      distance = tr1.flFraction; // store distance to obstacle

      // if this plane is a slope that is smooth enough for bot to climb it or a slope that goes down...
      if ((AngleBetweenVectors ((vector) tr1.vecPlaneNormal, vector (0, 0, 1)) < 30) || (AngleBetweenVectors ((vector) tr1.vecPlaneNormal, vector (0, 0, -1)) < 30))
      {
         // trace line parallel to previous starting a bit lower to get a new hit point
         UTIL_TraceLine ((EyeOriginOf (pBot->pEntity)) + vector (0, 0, -5),
                         ((EyeOriginOf (pBot->pEntity)) + vector (0, 0, -5)) + (referential.v_forward * 10000),
                         ignore_monsters, pBot->pEntity, &tr2);

         // compute a normalized vector parallel to slope
         vector v_parallel = (tr1.vecEndPos - tr2.vecEndPos).Normalize ();

         // trace line parallel to slope so that the bot 'sees' up or down the slope
         UTIL_TraceLine ((EyeOriginOf (pBot->pEntity)) + ((referential.v_forward * 10000) * tr1.flFraction),
                         ((EyeOriginOf (pBot->pEntity)) + ((referential.v_forward * 10000) * tr1.flFraction)) + (v_parallel * 10000),
                         ignore_monsters, pBot->pEntity, &tr3);

         if (DebugLevel.eyes > 0)
            DrawLine (pListenserverEntity,
                      (EyeOriginOf (pBot->pEntity)) + (((referential.v_forward * 10000) * tr1.flFraction) * 0.99),
                      ((EyeOriginOf (pBot->pEntity)) + (((referential.v_forward * 10000) * tr1.flFraction) * 0.99)) + ((v_parallel * 10000) * tr3.flFraction),
                      2, 255, 0, 0);

         // add distance from this traceline to the first one so that bot 'sees' up or down the slope
         distance += tr3.flFraction;
      }

      /*--- THIS DOES NOT WORK ---
      // else if plane is vertical and not too high, check if plane is a stair
      else if ((tr1.vecPlaneNormal.z == 0) && ())
      {
         // trace line at eyes level less maximal stair height (18)
         UTIL_TraceLine ((EyeOriginOf (pBot->pEntity)) + vector (0, 0, -18),
                         ((EyeOriginOf (pBot->pEntity)) + vector (0, 0, -18)) + (referential.v_forward * 10000),
                         ignore_monsters, pBot->pEntity, &tr2);

         // if two both not same length and relatively close together, bot may have found a staircase
         if ((tr2.flFraction < tr1.flFraction)
             && (((tr2.flFraction - tr1.flFraction) * 10000 < 50)
                 && ((tr2.flFraction - tr1.flFraction) * 10000 > 10)))
         {
            // compute a normalized vector parallel to staircase slope
            vector v_parallel = (tr1.vecEndPos - tr2.vecEndPos).Normalize ();

            // trace line parallel to staircase slope so that the bot 'sees' up the staircase
            UTIL_TraceLine ((EyeOriginOf (pBot->pEntity)) + (((referential.v_forward * 10000) * tr1.flFraction) * 0.80),
                            ((EyeOriginOf (pBot->pEntity)) + (((referential.v_forward * 10000) * tr1.flFraction) * 0.80)) + (v_parallel * 10000),
                            ignore_monsters, pBot->pEntity, &tr3);

            if (DebugLevel.eyes > 0)
               DrawLine (pListenserverEntity,
                         (EyeOriginOf (pBot->pEntity)) + (((referential.v_forward * 10000) * tr1.flFraction) * 0.80),
                         ((EyeOriginOf (pBot->pEntity)) + (((referential.v_forward * 10000) * tr1.flFraction) * 0.80)) + ((v_parallel * 10000) * tr3.flFraction),
                         2, 255, 0, 0);

            // add distance from this traceline to the first one so that bot 'sees' up the staircase
            distance += tr3.flFraction;
         }
      }
      --- THIS DOES NOT WORK ---*/

      // store the results of the scan in bot's FOV array
      pBot->BotEyes.BotFOV[fov_index].scan_angles = v_viewangle;
      pBot->BotEyes.BotFOV[fov_index].distance = distance * 10000;
      pBot->BotEyes.BotFOV[fov_index].vecEndPos = (vector) tr1.vecEndPos;
      pBot->BotEyes.BotFOV[fov_index].Normal = (vector) tr1.vecPlaneNormal;
      pBot->BotEyes.BotFOV[fov_index].pHit = tr1.pHit;

      fov_index++; // increment FOV array index
   }

   // now ask the engine for the entity list

   // erase the previous array
   memset (pBot->BotEyes.pEntitiesInSight, NULL, sizeof (pBot->BotEyes.pEntitiesInSight));

   // cycle through all entities in game
   for (entity_index = 0; entity_index < MaxEntitiesOnServer (); entity_index++)
   {
      pEntity = EntityAtIndex (entity_index); // bind that entity number
      if (IsNull (pEntity))
         continue; // if unregistered, skip it

      if (BotCanSeeOfEntity (pBot, pEntity) == NULLVEC)
         continue; // if not in sight, discard this item

      pBot->BotEyes.pEntitiesInSight[entity_count] = pEntity; // bot can see this entity

      entity_count++;
      if (entity_count == BOT_EYE_SENSITIVITY)
         break; // if too much entities in sight, stop
   }

   // the human eye samples 30 times per second, bots should line up with that too...
   pBot->BotEyes.f_sample_time = CurrentTime () + 0.033; // next sampling in 33 ms

   return;
}


vector BotCanSeeOfEntity (bot_t *pBot, entity_t *pEntity)
{
   // this function returns the best vector the bot pointed to by pBot can see of entity pEntity,
   // and in case pEntity is definitely not visible from the bot's point of view, it returns a
   // NULL vector. Either the entity is a player, or a bounded entity, or a point-based entity.
   // if it's a player, checks are made on different points of the player's body, starting from
   // the most tactically important one (the head) to the less tactically important ones, in
   // order to determine if the player is visible and WHAT is visible of this player ; if it's
   // an entity which has a bounding box, we check whether either the center of it, or any of
   // its four corners on a flattened referential, are visible, and if none of these points are,
   // we assume that the entity isn't visible at all. Else if it's a point-based entity, all we
   // can check is the visibility of the center (origin) of this entity. In case some part of
   // pEntity is visible from the bot's point of view, the vector position of the best visible
   // point of pEntity is returned.

   static TraceResult tr;
   static vector v_source, v_targetorigin, v_targetmins, v_targetmaxs, v_targetside, v_targetangles;

   if (IsNull (pBot->pEntity) || IsNull (pEntity))
      return (NULLVEC); // reliability check

   if (IsInvisible (pEntity))
      return (NULLVEC); // if invisible (such as weapon having been picked up), bot sees nothing

   v_source = EyeOriginOf (pBot->pEntity); // compute source vector location

   // is the target entity a player ?
   if (IsAPlayer (pEntity))
   {
      // always look for the player's head first
      v_targetorigin = EyeOriginOf (pEntity); // compute target head vector location
      if (VectorIsInPlayerFOV (pBot->pEntity, v_targetorigin))
      {
         pfnTraceLine (v_source, v_targetorigin, 0x101, pEntity, &tr);
         if (tr.flFraction == 1.0)
            return (v_targetorigin); // head of target player is visible
      }

      // else look for the player's waist
      pfnGetBonePosition (pEntity, PLAYERBONE_PELVIS, v_targetside, v_targetangles);
      if (VectorIsInPlayerFOV (pBot->pEntity, v_targetside))
      {
         pfnTraceLine (v_source, v_targetside, 0x101, pEntity, &tr);
         if (tr.flFraction == 1.0)
            return (v_targetside); // body of target player is visible
      }

      // else look for the player's left arm
      pfnGetBonePosition (pEntity, PLAYERBONE_L_FOREARM, v_targetside, v_targetangles);
      if (VectorIsInPlayerFOV (pBot->pEntity, v_targetside))
      {
         pfnTraceLine (v_source, v_targetside, 0x101, pEntity, &tr);
         if (tr.flFraction == 1.0)
            return (v_targetside); // left arm of target player is visible
      }

      // else look for the player's right arm
      pfnGetBonePosition (pEntity, PLAYERBONE_R_FOREARM, v_targetside, v_targetangles);
      if (VectorIsInPlayerFOV (pBot->pEntity, v_targetside))
      {
         pfnTraceLine (v_source, v_targetside, 0x101, pEntity, &tr);
         if (tr.flFraction == 1.0)
            return (v_targetside); // right arm of target player is visible
      }

      // else look for the player's left foot
      pfnGetBonePosition (pEntity, PLAYERBONE_L_FOOT, v_targetside, v_targetangles);
      if (VectorIsInPlayerFOV (pBot->pEntity, v_targetside))
      {
         pfnTraceLine (v_source, v_targetside, 0x101, pEntity, &tr);
         if (tr.flFraction == 1.0)
            return (v_targetside); // left foot of target player is visible
      }

      // else look for the player's right foot
      pfnGetBonePosition (pEntity, PLAYERBONE_R_FOOT, v_targetside, v_targetangles);
      if (VectorIsInPlayerFOV (pBot->pEntity, v_targetside))
      {
         pfnTraceLine (v_source, v_targetside, 0x101, pEntity, &tr);
         if (tr.flFraction == 1.0)
            return (v_targetside); // right foot of target player is visible
      }

      return (NULLVEC); // none of these player's body parts are visible, assume player isn't
   }

   // else has the target entity got a bounding box ?
   else if (HasBoundingBox (pEntity))
   {
      // get the entity's bounding box limits
      GetEntityBoundingBox (pEntity, v_targetmins, v_targetmaxs);

      // do some checks for determining which part of the bounding box is visible
      v_targetorigin = OriginOf (pEntity); // compute target center vector location
      if (VectorIsInPlayerFOV (pBot->pEntity, v_targetorigin))
      {
         pfnTraceLine (v_source, v_targetorigin, 0x101, pEntity, &tr);
         if (tr.flFraction == 1.0)
            return (v_targetorigin); // center of entity is visible
      }

      v_targetside = vector (v_targetmins.x, v_targetmins.y, v_targetorigin.z);
      if (VectorIsInPlayerFOV (pBot->pEntity, v_targetside))
      {
         pfnTraceLine (v_source, v_targetside, 0x101, pEntity, &tr);
         if (tr.flFraction == 1.0)
            return (v_targetside); // this side of entity is visible
      }

      v_targetside = vector (v_targetmins.x, v_targetmaxs.y, v_targetorigin.z);
      if (VectorIsInPlayerFOV (pBot->pEntity, v_targetside))
      {
         pfnTraceLine (v_source, v_targetside, 0x101, pEntity, &tr);
         if (tr.flFraction == 1.0)
            return (v_targetside); // this side of entity is visible
      }

      v_targetside = vector (v_targetmaxs.x, v_targetmins.y, v_targetorigin.z);
      if (VectorIsInPlayerFOV (pBot->pEntity, v_targetside))
      {
         pfnTraceLine (v_source, v_targetside, 0x101, pEntity, &tr);
         if (tr.flFraction == 1.0)
            return (v_targetside); // this side of entity is visible
      }

      v_targetside = vector (v_targetmaxs.x, v_targetmaxs.y, v_targetorigin.z);
      if (VectorIsInPlayerFOV (pBot->pEntity, v_targetside))
      {
         pfnTraceLine (v_source, v_targetside, 0x101, pEntity, &tr);
         if (tr.flFraction == 1.0)
            return (v_targetside); // this side of entity is visible
      }

      return (NULLVEC); // neither the center nor any of the sides of pEntity is visible
   }

   // else it's a point-based entity, check for its visibility
   v_targetorigin = OriginOf (pEntity);
   if (VectorIsInPlayerFOV (pBot->pEntity, v_targetorigin))
   {
      pfnTraceLine (v_source, v_targetorigin, 0x101, pEntity, &tr);
      if (tr.flFraction == 1.0)
         return (v_targetorigin); // if visible, return the origin of entity the engine knows
   }

   return (NULLVEC); // else assume entity is not visible
}
