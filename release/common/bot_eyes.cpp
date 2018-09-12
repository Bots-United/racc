// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// bot_eyes.cpp
//

#include "racc.h"


void BotSee (player_t *pPlayer)
{
   // this is the function that makes the bot see. It fires bursts of TraceLines, incrementing
   // the trace angle each iteration, and store the results of the scan in an array, which
   // median index corresponds to the direction the bot is facing. This is a logic modeling
   // of the human eye. Unfortunately, the processing power required for a 3D scan is too high,
   // so here we just do a flat scan. The idea is not perfect, though. The list of potential
   // entities in sight is built from the engine's own list, then each of them is replaced in
   // its context, i.e. a corresponding is made between it and the right scan angle.
   // And finally, the bot keeps an eye on the chat messages on its screen...

   bot_eyes_t *pBotEyes;
   float angle, distance;
   int fov_index, entity_index, entity_count;
   Vector v_forward, v_groundslope, v_originalangle, v_viewangle;
   test_result_t tr1, tr2, tr3;
   edict_t *pEntity;

   if (DebugLevel.eyes_disabled)
      return; // cancel if we don't want the AI to see

   pBotEyes = &pPlayer->Bot.BotEyes; // quick access to bot eyes

   if (!pPlayer->is_alive)
      return; // cancel if bot is dead (saves tracelines)

   if (pBotEyes->blinded_time > server.time)
      return; // cancel if bot is blinded

   // ask the engine for the entity list

   // erase the previous array
   memset (pBotEyes->pEntitiesInSight, NULL, sizeof (pBotEyes->pEntitiesInSight));

   // cycle through all entities in game
   entity_count = 0;
   for (entity_index = 0; entity_index < server.max_entities; entity_index++)
   {
      pEntity = INDEXENT (entity_index + 1); // bind that entity number
      if (FNullEnt (pEntity))
         continue; // if unregistered, skip it

      if (BotCanSeeOfEntity (pPlayer, pEntity) == g_vecZero)
         continue; // if not in sight, discard this item

      pBotEyes->pEntitiesInSight[entity_count] = pEntity; // bot can see this entity
      entity_count++; // we know now one entity more

      if (entity_count == BOT_EYE_SENSITIVITY)
         break; // if too much entities in sight, stop
   }
   pBotEyes->entity_count = entity_count; // keep track of how many entity this bot sees

   if (pBotEyes->sample_time > server.time)
      return; // don't go further if not time to sample the field of view yet

   // figure out the ground slope
   v_forward = Vector (pPlayer->v_forward.x, pPlayer->v_forward.y, 0); // build base vector
   tr1 = PlayerTestLine (pPlayer,
                         pPlayer->v_origin,
                         pPlayer->v_origin + Vector (0, 0, -9999));
   tr2 = PlayerTestLine (pPlayer,
                         pPlayer->v_origin + (v_forward * 15),
                         pPlayer->v_origin + (v_forward * 15) + Vector (0, 0, -9999));
   v_groundslope = tr2.v_endposition - tr1.v_endposition;

   // now figure out a corresponding angle and clamp its X component (pitch) in bounds
   v_originalangle = VecToAngles (v_groundslope);
   v_originalangle.x = -v_originalangle.x * 0.75; // fakeclient aim bug fix + temperate pitch
   if (v_originalangle.x < -45)
      v_originalangle.x = -45;
   else if (v_originalangle.x > 45)
      v_originalangle.x = 45;

   // store the capture point
   pBotEyes->v_capture_point = pPlayer->v_eyeposition;

   // scan 100 degrees of bot's field of view from LEFT to RIGHT (yaw angles are inverted)...
   fov_index = 0;
   for (angle = 50; angle >= -50; angle -= (angle * angle) / 1500 + 1.5)
   {
      v_viewangle = v_originalangle; // restore bot's current v_angle
      v_viewangle.y = WrapAngle (v_viewangle.y + angle); // pan it from left to right
      BuildReferential (v_viewangle); // build base vectors in that direction

      // trace line slightly under eyes level
      tr1 = PlayerTestLine (pPlayer,
                            pPlayer->v_eyeposition,
                            pPlayer->v_eyeposition + (referential.v_forward * 10000));

      // if debug level is high, draw the field of view of this bot
      if (pPlayer->is_watched && (DebugLevel.eyes > 1))
         UTIL_DrawLine (pPlayer->v_eyeposition,
                        pPlayer->v_eyeposition + ((referential.v_forward * 10000) * tr1.fraction),
                        1, 255, 0, 0);

      distance = tr1.fraction * 10000; // store distance to obstacle

      // if this plane is a slope that is smooth enough for bot to climb it or a slope that goes down...
      if ((AngleOfVectors (tr1.v_normal, Vector (0, 0, 1)) < 30) || (AngleOfVectors (tr1.v_normal, Vector (0, 0, -1)) < 30))
      {
         // trace line parallel to previous starting a bit lower to get a new hit point
         tr2 = PlayerTestLine (pPlayer,
                               pPlayer->v_eyeposition + Vector (0, 0, -5),
                               (pPlayer->v_eyeposition + Vector (0, 0, -5)) + (referential.v_forward * 10000));

         // compute a normalized vector parallel to slope
         Vector v_parallel = (tr1.v_endposition - tr2.v_endposition).Normalize ();

         // trace line parallel to slope so that the bot 'sees' up or down the slope
         tr3 = PlayerTestLine (pPlayer,
                               pPlayer->v_eyeposition + (((referential.v_forward * 10000) * tr1.fraction) * 0.99),
                               (pPlayer->v_eyeposition + (((referential.v_forward * 10000) * tr1.fraction) * 0.99)) + (v_parallel * 10000));

         // if debug level is high, draw the field of view of this bot
         if (pPlayer->is_watched && (DebugLevel.eyes > 1))
            UTIL_DrawLine (pPlayer->v_eyeposition + (((referential.v_forward * 10000) * tr1.fraction) * 0.99),
                           (pPlayer->v_eyeposition + (((referential.v_forward * 10000) * tr1.fraction) * 0.99)) + ((v_parallel * 10000) * tr3.fraction),
                           1, 255, 0, 0);

         // add distance from this traceline to the first one so that bot 'sees' up or down the slope
         distance += tr3.fraction * 10000;
      }

/*    --- THIS DOES NOT WORK ---
      // else if plane is vertical and not too high, check if plane is a stair
      else if ((tr1.v_normal.z == 0) && (tr1.pHit->v.size.z < 18))
      {
         // trace line at eyes level less maximal stair height (18)
         tr2 = PlayerTestLine (pPlayer,
                               pPlayer->v_eyeposition + Vector (0, 0, -18),
                               (pPlayer->v_eyeposition + Vector (0, 0, -18)) + (referential.v_forward * 10000));

         // if two both not same length and relatively close together, bot may have found a staircase
         if ((tr2.fraction < tr1.fraction)
             && (((tr2.fraction - tr1.fraction) * 10000 < 50)
                 && ((tr2.fraction - tr1.fraction) * 10000 > 10)))
         {
            // compute a normalized vector parallel to staircase slope
            Vector v_parallel = (tr1.v_endposition - tr2.v_endposition).Normalize ();

            // trace line parallel to staircase slope so that the bot 'sees' up the staircase
            tr3 = PlayerTestLine (pPlayer,
                                  pPlayer->v_eyeposition + (((referential.v_forward * 10000) * tr1.fraction) * 0.80),
                                  (pPlayer->v_eyeposition + (((referential.v_forward * 10000) * tr1.fraction) * 0.80)) + (v_parallel * 10000));

            // if debug level is high, draw the field of view of this bot
            if (pPlayer->is_watched && (DebugLevel.eyes > 1))
               UTIL_DrawBeam (pListenserverPlayer,
                              (pPlayer->v_eyeposition + (((referential.v_forward * 10000) * tr1.fraction) * 0.80),
                              (pPlayer->v_eyeposition + (((referential.v_forward * 10000) * tr1.fraction) * 0.80)) + ((v_parallel * 10000) * tr3.fraction),
                              1, 5, 0, 255, 0, 0, 255, 0);

            // add distance from this traceline to the first one so that bot 'sees' up the staircase
            distance += tr3.fraction * 10000;
         }
      }
      --- THIS DOES NOT WORK ---
*/

      // store the results of the scan in bot's FOV array
      pBotEyes->BotFOV[fov_index].scan_angles = v_viewangle;
      pBotEyes->BotFOV[fov_index].distance = distance;
      pBotEyes->BotFOV[fov_index].vecEndPos = tr1.v_endposition;
      pBotEyes->BotFOV[fov_index].Normal = tr1.v_normal;
      pBotEyes->BotFOV[fov_index].pHit = tr1.pHit;

      fov_index++; // increment FOV array index
   }

   // we just can't match up with the human eye's refresh rate... bots will sample less often
   pBotEyes->sample_time = server.time + 0.20; // next sampling in 200 ms
   return;
}


edict_t *BotDoesSee (player_t *pPlayer, const char *model)
{
   // this function returns a pointer to an edict_t entity in the bot's ears if the bot is
   // currently seeing an entity whose relative path to the mesh model file is specified
   // by the "model" parameter. If the bot does not see such an entity, we return NULL.

   int index;
   int length;

   length = strlen (model); // get the length of the model string we want to search for

   // cycle through all the things the bot currently sees
   for (index = 0; index < pPlayer->Bot.BotEyes.entity_count; index++)
      if (strncmp (model, STRING (pPlayer->Bot.BotEyes.pEntitiesInSight[index]->v.model), length) == 0)
         return (pPlayer->Bot.BotEyes.pEntitiesInSight[index]); // entity model was found, bot sees it

   return (NULL); // bot does not see anything wearing that mesh model
}


Vector BotCanSeeOfEntity (player_t *pPlayer, edict_t *pEntity)
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

   static test_result_t tr;
   static Vector v_source, v_targetorigin, v_targetside, v_targetangles;

   if (FNullEnt (pPlayer->pEntity) || FNullEnt (pEntity))
      return (g_vecZero); // reliability check

   if (IsInvisible (pEntity))
      return (g_vecZero); // if entity is invisible, bot sees nothing

   v_source = pPlayer->v_eyeposition; // compute source vector location

   // is the target entity a player ?
   if (pEntity->v.flags & FL_CLIENT)
   {
      // always look for the player's head first
      v_targetorigin = GetGunPosition (pEntity); // compute target head vector location
      if (IsInFieldOfView (pPlayer->pEntity, v_targetorigin)
          && (TestVisibility (v_source, v_targetorigin, pEntity).fraction == 1.0))
         return (v_targetorigin); // head of target player is visible

      // else look for the player's waist, ask engine if bone number is known
      if (GameConfig.playerbones.pelvis != 0)
         GET_BONE_POSITION (pEntity, GameConfig.playerbones.pelvis, v_targetorigin, v_targetangles);
      else
         v_targetorigin = OriginOf (pEntity); // if bone number is unknown, take the entity origin
      if (IsInFieldOfView (pPlayer->pEntity, v_targetorigin)
          && (TestVisibility (v_source, v_targetorigin, pEntity).fraction == 1.0))
         return (v_targetorigin); // body of target player is visible

      // does the bot know this player bone ?
      if (GameConfig.playerbones.left_upperarm != 0)
      {
         // else look for the player's left arm
         GET_BONE_POSITION (pEntity, GameConfig.playerbones.left_upperarm, v_targetside, v_targetangles);
         if (IsInFieldOfView (pPlayer->pEntity, v_targetside)
             && (TestVisibility (v_source, v_targetside, pEntity).fraction == 1.0))
            return (v_targetside); // left arm of target player is visible
      }

      // does the bot know this player bone ?
      if (GameConfig.playerbones.right_upperarm != 0)
      {
         // else look for the player's right arm
         GET_BONE_POSITION (pEntity, GameConfig.playerbones.right_upperarm, v_targetside, v_targetangles);
         if (IsInFieldOfView (pPlayer->pEntity, v_targetside)
             && (TestVisibility (v_source, v_targetside, pEntity).fraction == 1.0))
            return (v_targetside); // right arm of target player is visible
      }

      // does the bot know this player bone ?
      if (GameConfig.playerbones.left_foot != 0)
      {
         // else look for the player's left foot
         GET_BONE_POSITION (pEntity, GameConfig.playerbones.left_foot, v_targetside, v_targetangles);
         if (IsInFieldOfView (pPlayer->pEntity, v_targetside)
             && (TestVisibility (v_source, v_targetside, pEntity).fraction == 1.0))
            return (v_targetside); // left foot of target player is visible
      }

      // does the bot know this player bone ?
      if (GameConfig.playerbones.right_foot != 0)
      {
         // else look for the player's right foot
         GET_BONE_POSITION (pEntity, GameConfig.playerbones.right_foot, v_targetside, v_targetangles);
         if (IsInFieldOfView (pPlayer->pEntity, v_targetside)
             && (TestVisibility (v_source, v_targetside, pEntity).fraction == 1.0))
            return (v_targetside); // right foot of target player is visible
      }

      return (g_vecZero); // none of these player's body parts are visible, assume player isn't
   }

   // else is the target entity a monster ?
   else if (pEntity->v.flags & FL_MONSTER)
   {
      // always look for the monster's head first
      v_targetorigin = GetGunPosition (pEntity); // compute target head vector location
      if (IsInFieldOfView (pPlayer->pEntity, v_targetorigin)
          && (TestVisibility (v_source, v_targetorigin, pEntity).fraction == 1.0))
         return (v_targetorigin); // head of target monster is visible

      // then look for the monster's body, use the entity origin for that
      v_targetorigin = OriginOf (pEntity); // compute target head vector location
      if (IsInFieldOfView (pPlayer->pEntity, v_targetorigin)
          && (TestVisibility (v_source, v_targetorigin, pEntity).fraction == 1.0))
         return (v_targetorigin); // head of target monster is visible

      return (g_vecZero); // neither the head nor the body of this monster are visible
   }

   // else has the target entity got a bounding box ?
   else if ((pEntity->v.absmin != g_vecZero) && (pEntity->v.absmax != g_vecZero))
   {
      // do some checks for determining which part of the bounding box is visible
      v_targetorigin = OriginOf (pEntity); // compute target center vector location
      if (IsInFieldOfView (pPlayer->pEntity, v_targetorigin))
      {
         tr = TestVisibility (v_source, v_targetorigin, pEntity);
         if ((tr.v_endposition.x > pEntity->v.absmin.x) && (tr.v_endposition.x < pEntity->v.absmax.x)
             && (tr.v_endposition.y > pEntity->v.absmin.y) && (tr.v_endposition.y < pEntity->v.absmax.y)
             && (tr.v_endposition.z > pEntity->v.absmin.z) && (tr.v_endposition.z < pEntity->v.absmax.z))
            return (tr.v_endposition); // entity is visible at hit point (center)
      }

      v_targetside = Vector (pEntity->v.absmin.x, pEntity->v.absmin.y, v_targetorigin.z);
      if (IsInFieldOfView (pPlayer->pEntity, v_targetside))
      {
         tr = TestVisibility (v_source, v_targetside, pEntity);
         if ((tr.v_endposition.x > pEntity->v.absmin.x) && (tr.v_endposition.x < pEntity->v.absmax.x)
             && (tr.v_endposition.y > pEntity->v.absmin.y) && (tr.v_endposition.y < pEntity->v.absmax.y)
             && (tr.v_endposition.z > pEntity->v.absmin.z) && (tr.v_endposition.z < pEntity->v.absmax.z))
            return (tr.v_endposition); // entity is visible at hit point (side 1)
      }

      v_targetside = Vector (pEntity->v.absmin.x, pEntity->v.absmax.y, v_targetorigin.z);
      if (IsInFieldOfView (pPlayer->pEntity, v_targetside))
      {
         tr = TestVisibility (v_source, v_targetside, pEntity);
         if ((tr.v_endposition.x > pEntity->v.absmin.x) && (tr.v_endposition.x < pEntity->v.absmax.x)
             && (tr.v_endposition.y > pEntity->v.absmin.y) && (tr.v_endposition.y < pEntity->v.absmax.y)
             && (tr.v_endposition.z > pEntity->v.absmin.z) && (tr.v_endposition.z < pEntity->v.absmax.z))
            return (tr.v_endposition); // entity is visible at hit point (side 2)
      }

      v_targetside = Vector (pEntity->v.absmax.x, pEntity->v.absmin.y, v_targetorigin.z);
      if (IsInFieldOfView (pPlayer->pEntity, v_targetside))
      {
         tr = TestVisibility (v_source, v_targetside, pEntity);
         if ((tr.v_endposition.x > pEntity->v.absmin.x) && (tr.v_endposition.x < pEntity->v.absmax.x)
             && (tr.v_endposition.y > pEntity->v.absmin.y) && (tr.v_endposition.y < pEntity->v.absmax.y)
             && (tr.v_endposition.z > pEntity->v.absmin.z) && (tr.v_endposition.z < pEntity->v.absmax.z))
            return (tr.v_endposition); // entity is visible at hit point (side 3)
      }

      v_targetside = Vector (pEntity->v.absmax.x, pEntity->v.absmax.y, v_targetorigin.z);
      if (IsInFieldOfView (pPlayer->pEntity, v_targetside))
      {
         tr = TestVisibility (v_source, v_targetside, pEntity);
         if ((tr.v_endposition.x > pEntity->v.absmin.x) && (tr.v_endposition.x < pEntity->v.absmax.x)
             && (tr.v_endposition.y > pEntity->v.absmin.y) && (tr.v_endposition.y < pEntity->v.absmax.y)
             && (tr.v_endposition.z > pEntity->v.absmin.z) && (tr.v_endposition.z < pEntity->v.absmax.z))
            return (tr.v_endposition); // entity is visible at hit point (side 4)
      }

      return (g_vecZero); // neither the center nor any of the sides of pEntity is visible
   }

   // else it's a point-based entity
   else
   {
      // check for its visibility
      v_targetorigin = OriginOf (pEntity);
      if (IsInFieldOfView (pPlayer->pEntity, v_targetorigin)
          && (TestVisibility (v_source, v_targetorigin, pEntity).fraction == 1.0))
         return (v_targetorigin); // if visible, return the origin of entity the engine knows

      return (g_vecZero); // this point-based entity is not visible
   }

   return (g_vecZero); // assume entity is not visible
}
