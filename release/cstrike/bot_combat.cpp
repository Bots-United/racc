// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// CSTRIKE version
//
// bot_combat.cpp
//

// FIXME: most if not all of this stuff comes straight from the old RACC. The combat code
// should rely on the navigation, which MUST be finished and IRONED OUT BEFOREHAND.

#include "racc.h"


void BotCheckForEnemies (player_t *pPlayer)
{
   int index;
   player_t *pOtherPlayer;
   edict_t *pMonster;
   bool otherplayer_is_vip;
   float enemy_distance;
   float nearest_distance = 9999;

   // does the bot already have an enemy ?
   if (!FNullEnt (pPlayer->Bot.BotEnemy.pEdict))
   {
      // is the enemy dead ?
      if (!IsAlive (pPlayer->Bot.BotEnemy.pEdict))
      {
         // hail for the victory
         pPlayer->Bot.BotChat.bot_sayaudio = BOT_SAYAUDIO_VICTORY; // bot laughs (audio)

         if (RandomLong(1,100) <= (56 - 2 * player_count))
            pPlayer->Bot.BotChat.bot_saytext = BOT_SAYTEXT_LAUGH; // bot laughs (text)

         // once out of two times send a radio message
         if (RandomLong (1, 100) < 50)
            FakeClientCommand (pPlayer->pEntity, RADIOMSG_ENEMYDOWN);

         // if this enemy was close to us, spray a logo
         if (((OriginOf (pPlayer->Bot.BotEnemy.pEdict) - pPlayer->v_origin).Length () < 200)
             && (RandomLong (1, 100) < 33))
            pPlayer->Bot.spraylogo_time = server.time + RandomFloat (1.0, 2.0);

         // was the victim a player ?
         if (pPlayer->Bot.BotEnemy.pEdict->v.flags & FL_CLIENT)
            pPlayer->Bot.victim_index = ENTINDEX (pPlayer->Bot.BotEnemy.pEdict) - 1; // remember victim

         // have no enemy anymore, null out enemy structures
         memset (&pPlayer->Bot.BotEnemy, 0, sizeof (pPlayer->Bot.BotEnemy));
         memset (&pPlayer->Bot.LastSeenEnemy, 0, sizeof (pPlayer->Bot.LastSeenEnemy));

         pPlayer->Bot.BotEyes.sample_time = server.time; // open eyes again
         pPlayer->Bot.reach_time = server.time; // and get a new reach point
      }

      // else is enemy still visible and in field of view ?
      else if (BotCanSeeOfEntity (pPlayer, pPlayer->Bot.BotEnemy.pEdict) != g_vecZero)
      {
         // see if this enemy is a VIP player (greatest enemy there is)
         // to do this, ensure this enemy is a client -hence having a slot in the client array-
         // then lookup in this array to see if its player model is typical from the VIP
         otherplayer_is_vip = ((pPlayer->Bot.BotEnemy.pEdict->v.flags & FL_CLIENT)
                               && (strcmp ("vip", players[ENTINDEX (pPlayer->Bot.BotEnemy.pEdict) - 1].model) == 0));

         // is our current enemy NOT the greater threat there is, already ?
         if (!otherplayer_is_vip)
         {
            // cycle through all players...
            for (index = 0; index < server.max_clients; index++)
            {
               pOtherPlayer = &players[index]; // quick access to player

               if (!IsValidPlayer (pOtherPlayer) || !pOtherPlayer->is_alive || (pOtherPlayer == pPlayer))
                  continue; // skip invalid and dead players and skip self (i.e. this bot)

               if (DebugLevel.is_observer && !pOtherPlayer->is_racc_bot)
                  continue; // skip this player if real client and we are in observer mode

               if (GetTeam (pOtherPlayer) == GetTeam (pPlayer))
                  continue; // skip this player if it is a teammate

               // see if bot can see the player...
               if (BotCanSeeOfEntity (pPlayer, pOtherPlayer->pEntity) != g_vecZero)
               {
                  float distance = (pOtherPlayer->v_origin - pPlayer->v_origin).Length ();

                  if (distance < nearest_distance)
                  {
                     nearest_distance = distance; // update nearest distance
                     pPlayer->Bot.BotEnemy.pEdict = pOtherPlayer->pEntity; // bot found a greater threat !
                  }
               }
            }
         }

         // yell about this enemy
         pPlayer->Bot.BotChat.bot_sayaudio = BOT_SAYAUDIO_ATTACKING; // bot yells attack (audio)

         // set next reload time
         pPlayer->Bot.reload_time = server.time + RandomLong (1.0, 3.0);

         return;
      }

      // else, looks like bot lost his enemy
      else
      {
         pPlayer->Bot.BotEnemy.disappearance_time = server.time; // save lost enemy time
         pPlayer->Bot.BotLegs.strafeleft_time = 0; // stop strafing
         pPlayer->Bot.BotLegs.straferight_time = 0;

         // pause for a while on occasion to let the bot recover from the fight
         if (RandomLong (1, 100) < (pPlayer->Bot.is_fearful ? 66 : 33))
         {
            pPlayer->Bot.pause_time = server.time + RandomFloat (1.0, 7.0 - pPlayer->Bot.pProfile->skill); // pause for a while
            pPlayer->Bot.BotEyes.sample_time = pPlayer->Bot.pause_time; // open eyes after that date
            pPlayer->Bot.reach_time = pPlayer->Bot.pause_time; // and get a new reach point then
         }
      }
   }

   // looks like the bot has no enemy...

   if (DebugLevel.is_peacemode)
   {
      memset (&pPlayer->Bot.BotEnemy, 0, sizeof (pPlayer->Bot.BotEnemy));
      memset (&pPlayer->Bot.LastSeenEnemy, 0, sizeof (pPlayer->Bot.LastSeenEnemy));
      return; // if peacemode is set, bot won't look for new enemies
   }

   // are there monsters in sight ?
   pMonster = NULL;
   while ((pMonster = FindEntityInSphere (pMonster, pPlayer->v_eyeposition, 800)) != NULL)
   {
      if (!(pMonster->v.flags & FL_MONSTER))
         continue; // skip anything that is NOT a monster

      if (!IsAlive (pMonster))
         continue; // skip monster if already dead

      if (BotCanSeeOfEntity (pPlayer, pMonster) == g_vecZero)
         continue; // skip monster if bot can't see it

      // this monster is visible, see how far it is from the bot
      enemy_distance = (OriginOf (pMonster) - pPlayer->v_origin).Length ();

      // is this monster the closest to us so far ?
      if (enemy_distance < nearest_distance)
      {
         pPlayer->Bot.BotEnemy.pEdict = pMonster; // bot found a monster to shoot at !
         pPlayer->Bot.BotEnemy.appearance_time = server.time; // save when we first saw this enemy
      }
   }

   // are there players in sight ?
   for (index = 0; index < server.max_clients; index++)
   {
      pOtherPlayer = &players[index]; // quick access to player

      if (!IsValidPlayer (pOtherPlayer) || !pOtherPlayer->is_alive || (pOtherPlayer == pPlayer))
         continue; // skip invalid and dead players and skip self (i.e. this bot)

      if (DebugLevel.is_observer && !pOtherPlayer->is_racc_bot)
         continue; // skip real players in observer mode

      if (BotCanSeeOfEntity (pPlayer, pOtherPlayer->pEntity) == g_vecZero)
         continue; // skip this player if bot can't see him

      // this player is visible, see how far it is from the bot
      enemy_distance = (pOtherPlayer->v_origin - pPlayer->v_origin).Length ();

      // is this player a teammate ? if so, check if he's firing in some direction
      if (GetTeam (pOtherPlayer) == GetTeam (pPlayer))
      {
         // if this teammate is attacking something AND it is relatively close...
         if ((pOtherPlayer->input_buttons & INPUT_KEY_FIRE1) && (enemy_distance < 750))
         {
            // if this teammate is far, come to him
            if (enemy_distance > 200)
               pPlayer->Bot.v_reach_point = pOtherPlayer->v_origin; // "i'm coming, mate !!"

            // else if we are close to him...
            else
            {
               BotSetIdealYaw (pPlayer, pOtherPlayer->v_angle.y); // look where he is looking
               pPlayer->Bot.reach_time = server.time + 0.5; // let the bot turn
            }
         }

         continue; // don't target your teammates...
      }

      // now this player is an enemy

      // see if this enemy the greater enemy there is
      otherplayer_is_vip = (strcmp ("vip", pOtherPlayer->model) == 0);

      // is he the greater enemy OR is this enemy the closest to us so far ?
      if (otherplayer_is_vip || (enemy_distance < nearest_distance))
      {
         pPlayer->Bot.BotEnemy.pEdict = pOtherPlayer->pEntity; // bot found an enemy !
         pPlayer->Bot.BotEnemy.appearance_time = server.time; // save when we first saw this enemy

         // was this player the big badass ?
         if (otherplayer_is_vip)
            break; // if so, then don't look any further
      }
   }

   // and finally, is something shooting at us ?
   if (!FNullEnt (pPlayer->pEntity->v.dmg_inflictor)
       && (pPlayer->pEntity->v.dmg_inflictor->v.flags & (FL_CLIENT | FL_MONSTER)))
   {
      // now this dirty bastard supercedes all other enemies! SHOOT HIM !!!
      pPlayer->Bot.BotEnemy.pEdict = pPlayer->pEntity->v.dmg_inflictor; // bot found an enemy
      pPlayer->Bot.BotEnemy.appearance_time = server.time; // save when we first saw this enemy
   }

   // has the bot found a new enemy at last ?
   if (!FNullEnt (pPlayer->Bot.BotEnemy.pEdict))
   {
      // if no enemy alert for about 20 seconds in team, send a radio message
      if (f_team_radiotime[GetTeam (pPlayer)] + 20 < server.time)
      {
         FakeClientCommand (pPlayer->pEntity, RADIOMSG_ENEMYSPOTTED); // bot says 'enemy spotted'
         f_team_radiotime[GetTeam (pPlayer)] = server.time; // update team radio time
         pPlayer->Bot.BotChat.bot_sayaudio = BOT_SAYAUDIO_ALERT; // bot says 'alert'
      }

      // notify squad and prepare to reload as soon as it'll be needed
//      BotNotifySquad (pPlayer, MESSAGE_SQUAD_NEWENEMY);
      pPlayer->Bot.reload_time = server.time + RandomFloat (3.0, 5.0); // set reload time
   }

   // else bot has found no enemy, does the bot need to zoom out ?
   else if ((pPlayer->pEntity->v.fov < 90)
            && PlayerHoldsWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_SCOPED | WEAPONRAIL_PROPERTY_SNIPER))
   {
      // let the bot zoom out
      if (pPlayer->input_buttons & INPUT_KEY_FIRE2)
         pPlayer->Bot.BotHand.fire2_time = 0; // by alternatively releasing...
      else
         pPlayer->Bot.BotHand.fire2_time = server.time + 0.2; // ...and pressing the zoom key
   }

   // is it time to do a weapon checkup ?
   if ((pPlayer->Bot.reload_time > 0) && (pPlayer->Bot.reload_time <= server.time))
   {
      BotSwitchToBestWeapon (pPlayer); // switch to best gun
      pPlayer->Bot.reload_time = -1; // so we won't keep reloading
      pPlayer->Bot.BotHand.reload_time = server.time + 0.2; // press reload button
   }

   return;
}


void BotSwitchToBestWeapon (player_t *pPlayer)
{
//   static weapon_t *suitable_weapons[MAX_BOT_WEAPONS];
//   int suitable_weapons_count = 0;
   bool ammo_left;
   int weapon_index;

/*   // cycle through all weapons the bot has and see the ones that are suitable
   for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
   {
      // is the bot NOT carrying this weapon ?
      if (!(pBot->pEdict->v.weapons & (1 << pBot->bot_weapons[weapon_index].hardware->id)))
         continue; // skip to next weapon

      // has this weapon no ammo left ?
      if ((*pBot->bot_weapons[weapon_index].primary_ammo < weapons[weapon_index].primary.min_ammo)
          && (*pBot->bot_weapons[weapon_index].secondary_ammo < weapons[weapon_index].secondary.min_ammo))
         continue;

      // is the bot underwater AND does this weapon NOT work under water ?
      if ((pPlayer->environment == ENVIRONMENT_WATER)
          && !(weapons[weapon_index].primary.can_use_underwater && weapons[weapon_index].secondary.can_use_underwater))
         continue; // skip to next weapon

   }*/

   // select the best weapon to use
   for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
   {
      // reset weapon usable state
      ammo_left = FALSE;

      // is the bot NOT carrying this weapon ?
      if (!(pPlayer->pEntity->v.weapons & (1 << weapons[weapon_index].id)))
         continue; // skip to next weapon

      // is the bot underwater AND does this weapon NOT work under water ?
      if ((pPlayer->environment == ENVIRONMENT_WATER)
          && !((weapons[weapon_index].primary.properties & WEAPONRAIL_PROPERTY_WATERPROOF)
               || (weapons[weapon_index].secondary.properties & WEAPONRAIL_PROPERTY_WATERPROOF)))
         continue; // skip to next weapon

      // is the bot camping AND does this weapon NOT fit for the job ?
      if ((pPlayer->Bot.BotBrain.bot_task == BOT_TASK_CAMP)
          && (PlayerHoldsWeaponOfClass (pPlayer, WEAPON_CLASS_GRENADE)
              || PlayerHoldsWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_BUCKSHOT)
              || PlayerHoldsWeaponOfRange (pPlayer, WEAPONRAIL_RANGE_MELEE)))
         continue; // skip to next weapon

      // is it the knife AND is the bot still suspicious about enemies around ?
      if ((weapons[weapon_index].primary.range <= WEAPONRAIL_RANGE_MELEE)
          && (weapons[weapon_index].secondary.range <= WEAPONRAIL_RANGE_MELEE)
          && (pPlayer->Bot.bot_alone_time > server.time))
         continue; // skip to next weapon

      // is the bot already holding this weapon and there is still ammo in clip ?
      if ((weapons[weapon_index].id == pPlayer->Bot.current_weapon->hardware->id)
          && ((weapons[weapon_index].primary.min_ammo <= 0)
              || (pPlayer->Bot.current_weapon->clip_ammo < 0)
              || (pPlayer->Bot.current_weapon->clip_ammo >= weapons[weapon_index].primary.min_ammo)))
         ammo_left = TRUE;

      // does this weapon have clips left ?
      if (((weapons[weapon_index].primary.type_of_ammo == -1)
           || (*pPlayer->Bot.bot_weapons[weapon_index].primary_ammo >= weapons[weapon_index].primary.min_ammo)))
         ammo_left = TRUE;

      // see if there is enough ammo OR no ammo required
      if (ammo_left)
         break; // at last, bot found the right weapon to kick your ass with

      continue; // weapon not usable, skip to next weapon
   }

   // is this one NOT bot's current weapon ?
   if ((weapons[weapon_index].id > 0) && (pPlayer->Bot.current_weapon->hardware->id != weapons[weapon_index].id))
      FakeClientCommand (pPlayer->pEntity, weapons[weapon_index].classname); // select this weapon
}


void BotFireWeapon (player_t *pPlayer)
{
   bool ammo_left;
   int weapon_index;
   Vector v_distance;
   float f_distance;

   if (FNullEnt (pPlayer->Bot.BotEnemy.pEdict))
      return; // reliability check: cancel if bot has no enemy

   // get distance from bot to enemy
   v_distance = OriginOf (pPlayer->Bot.BotEnemy.pEdict) - pPlayer->v_origin;
   f_distance = v_distance.Length ();

   // are we burst-firing ?
   if (pPlayer->Bot.current_weapon->primary_charging_time > 0)
   {
      // are we firing with a sniper rifle that can fire bursts ?
      if (PlayerHoldsWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_SNIPER | WEAPONRAIL_PROPERTY_AUTOMATIC))
         pPlayer->Bot.BotLegs.forward_time = 0; // don't move while using sniper rifle

      // is it time to stop the burst ?
      if (pPlayer->Bot.current_weapon->primary_charging_time < server.time)
      {
         pPlayer->Bot.current_weapon->primary_charging_time = 0; // stop firing it

         // set next time to shoot, as next frame will automatically release fire button
         float min_delay = pPlayer->Bot.current_weapon->hardware->primary.min_delay[pPlayer->Bot.pProfile->skill - 1];
         float max_delay = pPlayer->Bot.current_weapon->hardware->primary.max_delay[pPlayer->Bot.pProfile->skill - 1];

         pPlayer->Bot.shoot_time = server.time + RandomFloat (min_delay, max_delay) / 2;
      }
      else
      {
         pPlayer->Bot.BotHand.fire1_time = server.time + 0.2; // press the FIRE button
         pPlayer->Bot.shoot_time = server.time; // set next frame to keep pressing the button
      }

      return; // keep burst-firing
   }

   // we are not already charging a weapon, so select the best one to use
   for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
   {
      // reset weapon usable state
      ammo_left = FALSE;

      // is the bot NOT carrying this weapon ?
      if (!(pPlayer->pEntity->v.weapons & (1 << weapons[weapon_index].id)))
         continue; // skip to next weapon

      // is the bot underwater AND does this weapon NOT work under water ?
      if ((pPlayer->environment == ENVIRONMENT_WATER)
          && !((weapons[weapon_index].primary.properties & WEAPONRAIL_PROPERTY_WATERPROOF)
               || (weapons[weapon_index].secondary.properties & WEAPONRAIL_PROPERTY_WATERPROOF)))
         continue; // skip to next weapon

      // is the bot already holding this weapon and there is still ammo in clip ?
      if ((weapons[weapon_index].id == pPlayer->Bot.current_weapon->hardware->id)
          && ((weapons[weapon_index].primary.min_ammo <= 0)
              || (pPlayer->Bot.current_weapon->clip_ammo < 0)
              || (pPlayer->Bot.current_weapon->clip_ammo >= weapons[weapon_index].primary.min_ammo)))
         ammo_left = TRUE;

      // does this weapon have clips left ?
      if (((weapons[weapon_index].primary.type_of_ammo == -1)
           || (*pPlayer->Bot.bot_weapons[weapon_index].primary_ammo >= weapons[weapon_index].primary.min_ammo)))
         ammo_left = TRUE;

      // see if there is enough ammo
      // AND the bot is far enough away to fire
      // AND the bot is close enough to the enemy to fire
      if (ammo_left
/*          && (f_distance >= weapons[weapon_index].primary.min_range)
          && (f_distance <= weapons[weapon_index].primary.max_range)*/)
         break; // at last, bot found the right weapon to kick your ass with

      continue; // weapon not usable, skip to next weapon
   }

   // if bot can't decide which weapon to choose, cycle again but don't check for distances
   if (weapon_index == weapon_count)
   {
      for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
      {
         // reset weapon usable state
         ammo_left = FALSE;

         // is the bot NOT carrying this weapon ?
         if (!(pPlayer->pEntity->v.weapons & (1 << weapons[weapon_index].id)))
            continue; // skip to next weapon

         // is the bot underwater AND does this weapon NOT work under water ?
         if ((pPlayer->environment == ENVIRONMENT_WATER)
             && !((weapons[weapon_index].primary.properties & WEAPONRAIL_PROPERTY_WATERPROOF)
                  || (weapons[weapon_index].secondary.properties & WEAPONRAIL_PROPERTY_WATERPROOF)))
            continue; // skip to next weapon

         // is the bot already holding this weapon and there is still ammo in clip ?
         if ((weapons[weapon_index].id == pPlayer->Bot.current_weapon->hardware->id)
             && ((weapons[weapon_index].primary.min_ammo <= 0)
                 || (pPlayer->Bot.current_weapon->clip_ammo < 0)
                 || (pPlayer->Bot.current_weapon->clip_ammo >= weapons[weapon_index].primary.min_ammo)))
            ammo_left = TRUE;

         // does this weapon have clips left ?
         if (((weapons[weapon_index].primary.type_of_ammo == -1)
              || (*pPlayer->Bot.bot_weapons[weapon_index].primary_ammo >= weapons[weapon_index].primary.min_ammo)))
            ammo_left = TRUE;

         // see if there is enough ammo
         if (ammo_left)
            break; // bot finally found a weapon to use

         continue; // weapon not usable, skip to next weapon
      }
   }

   // is this one NOT bot's current weapon ?
   if ((weapons[weapon_index].id > 0) && (pPlayer->Bot.current_weapon->hardware->id != weapons[weapon_index].id))
      FakeClientCommand (pPlayer->pEntity, weapons[weapon_index].classname); // select this weapon

   // is bot just 'standing' on enemy ?
   if ((pPlayer->pEntity->v.absmin.z == pPlayer->Bot.BotEnemy.pEdict->v.absmax.z)
       && (v_distance.Length2D () < 46))
      pPlayer->Bot.BotLegs.duck_time = server.time + 0.1; // duck to hit him

   // is this weapon a sniper rifle ?
   if ((weapons[weapon_index].primary.properties & WEAPONRAIL_PROPERTY_SNIPER)
       || (weapons[weapon_index].secondary.properties & WEAPONRAIL_PROPERTY_SNIPER))
   {
      pPlayer->Bot.BotLegs.forward_time = 0; // don't move while using sniper rifle

      // are we moving too fast to use it ?
      if (pPlayer->v_velocity.Length () > 75)
         return; // don't press attack key until velocity is < 75

      // have we aimed with insufficient accuracy to fire AND is the bot not too nervous ?
      if (!PlayerAimIsOver (pPlayer, pPlayer->Bot.BotEnemy.pEdict)
          && (RandomLong (1, 100) < 94 + pPlayer->Bot.pProfile->skill))
         return; // don't shoot yet, it would be a monumental error...

      pPlayer->Bot.BotHand.fire1_time = server.time + 0.2; // we will likely hit the enemy, FIRE !!
   }

   // else is this weapon a grenade ?
   else if (weapons[weapon_index].weapon_class == WEAPON_CLASS_GRENADE)
   {
      pPlayer->Bot.BotChat.bot_sayaudio = BOT_SAYAUDIO_THROWGRENADE; // bot says 'throwgrenade'
      pPlayer->Bot.BotHand.fire1_time = server.time + 0.2; // throw the grenade !!
   }

   // else is it a melee weapon ?
   else if ((weapons[weapon_index].primary.range <= WEAPONRAIL_RANGE_MELEE)
            && (weapons[weapon_index].secondary.range <= WEAPONRAIL_RANGE_MELEE))
   {
      // is bot close enough to its enemy OR is bot nervous enough ?
      if ((f_distance < 60) || (RandomLong (1, 100) < 2))
      {
         // does the enemy NOT see us OR is bot "creative" enough to use secondary attack ?
         if (!IsInFieldOfView (pPlayer->Bot.BotEnemy.pEdict, pPlayer->v_origin)
             || (RandomLong (1, 100) < pPlayer->Bot.pProfile->skill * 10))
            pPlayer->Bot.BotHand.fire2_time = server.time + 0.2; // if so, use the deadly secondary melee attack
         else
            pPlayer->Bot.BotHand.fire1_time = server.time + 0.2; // else use primary melee attack
      }
   }

   // else this is definitely a normal weapon
   else
   {
      // compute the maximum angle deviation to enemy beyond which the bot should NOT fire
      float horizontal_maxangle = tanh ((pPlayer->Bot.BotEnemy.pEdict->v.absmax.x - pPlayer->Bot.BotEnemy.pEdict->v.absmin.x) / (2 * f_distance)) * (6 - pPlayer->Bot.pProfile->skill);
      float vertical_maxangle = tanh ((pPlayer->Bot.BotEnemy.pEdict->v.absmax.z - pPlayer->Bot.BotEnemy.pEdict->v.absmin.z) / (2 * f_distance)) * (6 - pPlayer->Bot.pProfile->skill);

      // is the bot's crosshair not on the enemy yet ?
      if (((fabs (pPlayer->Bot.BotHand.turn_speed.y) > horizontal_maxangle)
           || (fabs (pPlayer->Bot.BotHand.turn_speed.x) > vertical_maxangle))
          && (f_distance > 200 * (6 - pPlayer->Bot.pProfile->skill)))
         return; // then don't waste precious bullets

      pPlayer->Bot.BotHand.fire1_time = server.time + 0.2; // else we will likely hit the enemy, FIRE !!
   }

   // should we use burst-fire ?
   if ((pPlayer->Bot.pProfile->skill > 2) && (weapons[weapon_index].primary.charge_delay > 0)
       && (f_distance > 200 * (6 - pPlayer->Bot.pProfile->skill)))
   {
      float burst_delay = weapons[weapon_index].primary.charge_delay + ((6 - pPlayer->Bot.pProfile->skill) / 10);

      pPlayer->Bot.current_weapon->primary_charging_time = server.time + burst_delay; // set the delay of burst
      pPlayer->Bot.shoot_time = server.time; // set next frame to keep pressing the button
   }

   // else this was a normal shoot
   else
   {
      // should we hold button down to fire ?
      if (weapons[weapon_index].primary.properties & WEAPONRAIL_PROPERTY_AUTOMATIC)
      {
         // if no need for ammo OR no need for clip OR still enough ammo in clip...
         if ((weapons[weapon_index].primary.min_ammo <= 0)
             || (pPlayer->Bot.current_weapon->clip_ammo < 0)
             || (pPlayer->Bot.current_weapon->clip_ammo >= weapons[weapon_index].primary.min_ammo))
            pPlayer->Bot.shoot_time = server.time; // set next frame to keep pressing the button
         else
         {
            pPlayer->Bot.shoot_time = server.time + 0.25; // delay a while and reload
            pPlayer->Bot.BotHand.reload_time = server.time + 0.2; // press the RELOAD button
         }
      }

      // else set next time to shoot
      else
      {
         float min_delay = weapons[weapon_index].primary.min_delay[pPlayer->Bot.pProfile->skill - 1];
         float max_delay = weapons[weapon_index].primary.max_delay[pPlayer->Bot.pProfile->skill - 1];

         pPlayer->Bot.shoot_time = server.time + RandomFloat (min_delay, max_delay);
      }
   }

   return; // finished
}


void BotShootAtEnemy (player_t *pPlayer)
{
   Vector v_distance;
   float f_distance;
   int enemy_index;
   bool has_proximity_weapon;
   bool has_sniper_rifle;
   bool has_zoomable_rifle;
   bool has_grenade;
   bool enemy_sees_us;
   bool enemy_ducking;
   Vector v_target_angle;

   if (FNullEnt (pPlayer->Bot.BotEnemy.pEdict))
      return; // reliability check: don't shoot at an unexistent enemy

   enemy_index = ENTINDEX (pPlayer->Bot.BotEnemy.pEdict) - 1; // get enemy index

   // target some part of our enemy's body
   v_distance = BotCanSeeOfEntity (pPlayer, pPlayer->Bot.BotEnemy.pEdict) - pPlayer->v_eyeposition;
   f_distance = v_distance.Length (); // how far away is that scum ?

   // determine what type of weapon the bot is holding in its hands
   has_proximity_weapon = PlayerHoldsWeaponOfRange (pPlayer, WEAPONRAIL_RANGE_MELEE);
   has_sniper_rifle = PlayerHoldsWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_SNIPER);
   has_zoomable_rifle = PlayerHoldsWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_SCOPED);
   has_grenade = PlayerHoldsWeaponOfClass (pPlayer, WEAPON_CLASS_GRENADE);

   // determine the current state of the enemy
   enemy_sees_us = IsInFieldOfView (pPlayer->Bot.BotEnemy.pEdict, pPlayer->v_origin);
   enemy_ducking = ((enemy_index >= 0) && (enemy_index < server.max_clients)
                    && (players[enemy_index].input_buttons & INPUT_KEY_DUCK));

   // move the aim cursor and compensate for the recoil, good or bad based on skill
   BotSetIdealAngles (pPlayer, VecToAngles (v_distance)
                               + pPlayer->pEntity->v.punchangle / (6 - pPlayer->Bot.pProfile->skill));

   // decide which behaviour the bot will have while firing

   if (f_distance > 200)
   {
      // has the bot a proximity weapon OR is the bot not fearful, no sniper gun and won't fall...
      if (has_proximity_weapon
          || (!pPlayer->Bot.is_fearful && !has_sniper_rifle && !(pPlayer->Bot.BotBody.hit_state & OBSTACLE_FRONT_FALL)))
         pPlayer->Bot.BotLegs.forward_time = server.time + 0.5; // run to enemy

      // else bot is sniper or path to enemy is blocked, so if not fearful OR quite far from
      // enemy, try to keep the distance
      else if (!pPlayer->Bot.is_fearful || (f_distance > 500))
         pPlayer->Bot.BotLegs.forward_time = 0; // try to stay at a distant range from our enemy

      // else if enemy sees us, bot may be fearful, or enemy too close, so step back
      else if (enemy_sees_us)
         pPlayer->Bot.BotLegs.backwards_time = server.time + 0.5; // make some steps back

      // if the bot's enemy sees it AND bot is skilled enough to strafe AND is not strafing
      // for a few seconds already, pick up a random strafe direction
      if (enemy_sees_us && (pPlayer->Bot.pProfile->skill > 2)
          && (pPlayer->Bot.BotLegs.strafeleft_time + 6 - pPlayer->Bot.pProfile->skill < server.time)
          && (pPlayer->Bot.BotLegs.straferight_time + 6 - pPlayer->Bot.pProfile->skill < server.time))
      {
         if (RandomLong (1, 100) < 50)
            pPlayer->Bot.BotLegs.strafeleft_time = server.time + RandomFloat (0.5, 2.0);
         else
            pPlayer->Bot.BotLegs.straferight_time = server.time + RandomFloat (0.5, 2.0);
      }

      // else if bot is already strafing AND bot has not jumped for 0.7 s
      // AND bot hasn't some sort of sniper gun AND bot is skilled enough, randomly jump
      else if ((pPlayer->Bot.BotLegs.strafe_speed != 0)
               && (pPlayer->Bot.BotLegs.jump_time + 0.7 < server.time)
               && !has_sniper_rifle && (pPlayer->Bot.pProfile->skill > 2))
         pPlayer->Bot.BotLegs.jump_time = server.time + RandomFloat (0.1, (6.0 - pPlayer->Bot.pProfile->skill) / 4); //jump

      // else if enemy sees us, bot skilled enough and not currently strafing, duck
      else if (enemy_sees_us && (pPlayer->Bot.pProfile->skill > 2)
               && (pPlayer->Bot.BotLegs.strafeleft_time + 0.6 < server.time)
               && (pPlayer->Bot.BotLegs.straferight_time + 0.6 < server.time))
      {
         pPlayer->Bot.BotLegs.duck_time = server.time + 0.8; // duck and fire
         if ((pPlayer->Bot.BotEnemy.appearance_time + 5.0 < server.time)
             || ((pPlayer->Bot.BotEnemy.appearance_time + 0.5 < server.time)
                 && (has_proximity_weapon || has_grenade)))
            pPlayer->Bot.is_fearful = FALSE; // prevent bot from never attacking
      }

      // if enemy does NOT see us and bot hasn't any sort of proximity weapon, duck
      if (!enemy_sees_us && !has_proximity_weapon)
         pPlayer->Bot.BotLegs.duck_time = server.time + 0.2; // duck and fire
   }

   else if (f_distance > 20)
   {
      pPlayer->Bot.BotLegs.forward_time = server.time + 0.5; // walk if distance is closer

      // if enemy doesn't see us, walk
      if (!enemy_sees_us)
         pPlayer->Bot.BotLegs.walk_time = server.time + 0.2;

      // if bot is fearful and not currently strafing AND enemy is not ducking yet, duck
      if (pPlayer->Bot.is_fearful && (pPlayer->Bot.BotLegs.strafeleft_time + 0.6 < server.time)
          && (pPlayer->Bot.BotLegs.straferight_time + 0.6 < server.time)
          && !enemy_ducking)
      {
         pPlayer->Bot.BotLegs.duck_time = server.time + 0.8; // duck and fire
         if ((pPlayer->Bot.BotEnemy.appearance_time + 5.0 < server.time)
             || ((pPlayer->Bot.BotEnemy.appearance_time + 0.5 < server.time)
                 && (has_proximity_weapon || has_grenade)))
            pPlayer->Bot.is_fearful = FALSE; // prevent bot from never attacking
      }
   }

   else
      pPlayer->Bot.BotLegs.forward_time = 0; // don't move if close enough

   // now it's time to pull the trigger of that gun

   if (pPlayer->Bot.shoot_time > server.time)
      return; // don't shoot if not time to yet

   // is the bot holding a sniper rifle ?
   if (has_sniper_rifle)
   {
      // should the bot switch to the long-range zoom ?
      if ((f_distance > 600) && (pPlayer->pEntity->v.fov >= 40))
      {
         // then let the bot zoom in
         if (pPlayer->input_buttons & INPUT_KEY_FIRE2)
            pPlayer->Bot.BotHand.fire2_time = 0; // by alternatively releasing...
         else
            pPlayer->Bot.BotHand.fire2_time = server.time + 0.2; // ...and pressing the zoom key

         return; // don't shoot right now, bot is zooming in
      }

      // else should the bot switch to the close-range zoom ?
      else if ((f_distance > 150) && (pPlayer->pEntity->v.fov >= 90))
      {
         // then let the bot zoom in
         if (pPlayer->input_buttons & INPUT_KEY_FIRE2)
            pPlayer->Bot.BotHand.fire2_time = 0; // by alternatively releasing...
         else
            pPlayer->Bot.BotHand.fire2_time = server.time + 0.2; // ...and pressing the zoom key

         return; // don't shoot right now, bot is zooming in
      }

      // else should the bot restore the normal view ?
      else if ((f_distance <= 150) && (pPlayer->pEntity->v.fov < 90))
      {
         // then let the bot zoom out
         if (pPlayer->input_buttons & INPUT_KEY_FIRE2)
            pPlayer->Bot.BotHand.fire2_time = 0; // by alternatively releasing...
         else
            pPlayer->Bot.BotHand.fire2_time = server.time + 0.2; // ...and pressing the zoom key

         return; // don't shoot right now, bot is zooming out
      }
   }

   // else is the bot holding a zoomable rifle ?
   else if (has_zoomable_rifle)
   {
      // should the bot switch to zoomed mode ?
      if ((f_distance > 400) && (pPlayer->pEntity->v.fov >= 90))
      {
         // then let the bot zoom in
         if (pPlayer->input_buttons & INPUT_KEY_FIRE2)
            pPlayer->Bot.BotHand.fire2_time = 0; // by alternatively releasing...
         else
            pPlayer->Bot.BotHand.fire2_time = server.time + 0.2; // ...and pressing the zoom key

         return; // don't shoot right now, bot is zooming in
      }

      // else should the bot restore the normal view ?
      else if ((f_distance <= 400) && (pPlayer->pEntity->v.fov < 90))
      {
         // then let the bot zoom out
         if (pPlayer->input_buttons & INPUT_KEY_FIRE2)
            pPlayer->Bot.BotHand.fire2_time = 0; // by alternatively releasing...
         else
            pPlayer->Bot.BotHand.fire2_time = server.time + 0.2; // ...and pressing the zoom key

         return; // don't shoot right now, bot is zooming out
      }
   }

   BotFireWeapon (pPlayer); // select the best weapon at this distance and fire
}


void BotPlantBomb (player_t *pPlayer, Vector v_target)
{
   // if bomb icon is blinking, plant the bomb...
   if (pPlayer->Bot.BotEyes.BotHUD.icons_state[HUD_ICON_BOMB] == HUD_ICON_BLINKING)
   {
      pPlayer->Bot.BotBrain.bot_task = BOT_TASK_PLANTBOMB; // make the bot remember he is planting a bomb
      FakeClientCommand (pPlayer->pEntity, RADIOMSG_GONNABLOW); // bot speaks, "get outta here!"
   }

   // else let's run there...
   else
      BotReachPosition (pPlayer, v_target);
}


void BotDefuseBomb (player_t *pPlayer, Vector v_target)
{
   // if close to bomb, defuse it...
   if ((v_target - pPlayer->v_origin).Length () < 40)
   {
      pPlayer->Bot.BotBrain.bot_task = BOT_TASK_DEFUSEBOMB; // make the bot remember he is defusing a bomb
      pPlayer->Bot.v_goal = v_target; // make the bot remember the bomb location
      FakeClientCommand (pPlayer->pEntity, RADIOMSG_NEEDBACKUP); // bot speaks, "i need backup!"
   }

   // else if getting close, either duck or slow down
   else if ((v_target - pPlayer->v_origin).Length () < 100)
   {
      pPlayer->Bot.BotLegs.forward_time = server.time + 0.5; // go ahead...

      // is the bomb lower than the bot ?
      if (v_target.z < pPlayer->v_origin.z)
         pPlayer->Bot.BotLegs.duck_time = server.time + 0.5; // if so, duck above the bomb
      else
         pPlayer->Bot.BotLegs.walk_time = server.time + 0.2; // else walk as the bot comes closer

      BotLookAt (pPlayer, v_target); // look at bomb
   }

   // else let's run there...
   else
      BotReachPosition (pPlayer, v_target);
}
