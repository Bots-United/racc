// RACC - AI development project for first-person shooter games derived from Valve's Half-Life
// (http://www.racc-ai.com/)
//
// The game to engine interfacing code is based on the work done by Jeffrey 'botman' Broome
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
// CSTRIKE version
//
// bot_combat.cpp
//

// FIXME: most if not all of this stuff comes straight from the old RACC. The combat code should
// rely on the navigation, which MUST be finished and IRONED OUT BEFOREHAND. Don't laugh.

#include "racc.h"


edict_t *BotCheckForEnemies (bot_t *pBot)
{
   edict_t *pNewEnemy = NULL;
   float nearestdistance = 2500;
   int index;
   edict_t *pPlayer;

   if (!IsValidPlayer (pBot->pEdict))
      return (NULL); // reliability check

   if (DebugLevel.is_peacemode)
   {
      memset (&pBot->BotEnemy, 0, sizeof (pBot->BotEnemy));
      memset (&pBot->LastSeenEnemy, 0, sizeof (pBot->LastSeenEnemy));
      return (NULL); // if peacemode is set, bot won't look for enemies
   }

   // does the bot already have an enemy ?
   if (!FNullEnt (pBot->BotEnemy.pEdict))
   {
      // is the enemy dead ?
      if (!IsAlive (pBot->BotEnemy.pEdict))
      {
         if (RANDOM_LONG(1,100) <= (56 - 2 * player_count))
            pBot->BotChat.bot_saytext = BOT_SAYTEXT_LAUGH; // bot laughs (text)

         // don't complain about this enemy anymore but hail for the victory instead
         pBot->BotChat.bot_sayaudio &= ~(BOT_SAYAUDIO_ATTACKING | BOT_SAYAUDIO_TAKINGDAMAGE);
         pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_VICTORY; // bot laughs (audio)
         pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (0.5, 3.0);

         // once out of two times send a radio message
         if (RANDOM_LONG (1, 100) < 50)
            FakeClientCommand (pBot->pEdict, RADIOMSG_ENEMYDOWN);

         // if this enemy was close to us, spray a logo
         if (((pBot->BotEnemy.pEdict->v.origin - pBot->pEdict->v.origin).Length () < 200) && (RANDOM_LONG (1, 100) < 33))
            pBot->f_spraying_logo_time = *server.time + RANDOM_FLOAT (1.0, 2.0);

         pBot->pVictimEntity = pBot->BotEnemy.pEdict; // bot remembers his victim

         // have no enemy anymore, null out enemy structures
         memset (&pBot->BotEnemy, 0, sizeof (pBot->BotEnemy));
         memset (&pBot->LastSeenEnemy, 0, sizeof (pBot->LastSeenEnemy));

         pBot->BotEyes.sample_time = *server.time; // open eyes again
         pBot->f_reach_time = *server.time; // and get a new reach point
      }

      // else if enemy is still visible and in field of view, look for a greater threat, else keep it
      else if (BotCanSeeOfEntity (pBot, pBot->BotEnemy.pEdict) != g_vecZero)
      {
         // if the bot's current enemy is NOT the VIP, try to evaluate a greater threat
         if (!PlayerIsVIP (pBot->BotEnemy.pEdict))
         {
            // search the world for players...
            for (index = 0; index < *server.max_clients; index++)
            {
               pPlayer = players[index].pEntity; // quick access to player

               if (!IsValidPlayer (pPlayer) || (pPlayer == pBot->pEdict))
                  continue; // skip invalid players and skip self (i.e. this bot)

               if (!players[index].is_alive)
                  continue; // skip this player if not alive (i.e. dead or dying)

               if (DebugLevel.is_observer && !(pPlayer->v.flags & FL_THIRDPARTYBOT))
                  continue; // skip this player if real client and we are in observer mode

               if (GetTeam (pBot->pEdict) == GetTeam (pPlayer))
                  continue; // skip this player if it is a teammate

               // see if bot can see the player...
               if (BotCanSeeOfEntity (pBot, pPlayer) != g_vecZero)
               {
                  float distance = (pPlayer->v.origin - pBot->pEdict->v.origin).Length ();

                  if (distance < nearestdistance)
                  {
                     nearestdistance = distance; // update nearest distance
                     pBot->BotEnemy.pEdict = pPlayer; // bot found a greater threat !
                  }
               }
            }
         }

         // yell about this enemy, but have pauses between each new yell
         if (pBot->BotChat.f_sayaudio_time < *server.time)
         {
            pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_ATTACKING; // bot yells attack (audio)
            pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (1.0, 4.5);
         }

         // set next time to reload
         pBot->f_reload_time = *server.time + RANDOM_LONG (1.0, 3.0);

         return (pBot->BotEnemy.pEdict);
      }

      // else bot lost his enemy
      else
      {
         pBot->BotEnemy.disappearance_time = *server.time; // save lost enemy time
         pBot->BotMove.f_strafeleft_time = 0; // stop strafing
         pBot->BotMove.f_straferight_time = 0;

         // pause for a while on occasion to let the bot recover from the fight
         if (RANDOM_LONG (1, 100) < (pBot->b_is_fearful ? 66 : 33))
         {
            pBot->f_pause_time = *server.time + RANDOM_FLOAT (1.0, 7.0 - pBot->pProfile->skill); // pause for a while
            pBot->BotEyes.sample_time = pBot->f_pause_time; // open eyes after that date
            pBot->f_reach_time = pBot->f_pause_time; // and get a new reach point then
         }
      }
   }

   // looks like the bot has no enemy yet...

   // let's search the world for players...
   for (index = 0; index < *server.max_clients; index++)
   {
      pPlayer = players[index].pEntity; // quick access to player

      if (!IsValidPlayer (pPlayer) || (pPlayer == pBot->pEdict))
         continue; // skip invalid players and skip self (i.e. this bot)

      if (!players[index].is_alive)
         continue; // skip this player if not alive (i.e. dead or dying)

      if (DebugLevel.is_observer && !(pPlayer->v.flags & FL_THIRDPARTYBOT))
         continue; // skip real players in observer mode

      // if it is a teammate, check if this teammate is firing in some direction
      if (GetTeam (pBot->pEdict) == GetTeam (pPlayer))
      {
         // check if this teammate is visible and relatively close
         if (((pPlayer->v.origin - pBot->pEdict->v.origin).Length () < 750)
             && (BotCanSeeOfEntity (pBot, pPlayer) != g_vecZero))
         {
            // if this teammate is attacking something...
            if (pPlayer->v.button & IN_ATTACK)
            {
               // if this teammate is far, come to him
               if ((pPlayer->v.origin - pBot->pEdict->v.origin).Length () > 200)
                  pBot->v_reach_point = pPlayer->v.origin; // "i'm coming, mate !!"

               // else if we are close to him...
               else
               {
                  BotSetIdealYaw (pBot, pPlayer->v.v_angle.y); // look where he is looking
                  pBot->f_reach_time = *server.time + 0.5; // let the bot turn
               }
            }
         }

         continue; // don't target your teammates...
      }

      // see if bot can see the player...
      if (BotCanSeeOfEntity (pBot, pPlayer) != g_vecZero)
      {
         float distance = (pPlayer->v.origin - pBot->pEdict->v.origin).Length ();
         bool player_is_vip = PlayerIsVIP (pPlayer);

         // target the nearest enemy if no VIP found
         if ((distance < nearestdistance) || player_is_vip)
         {
            if (player_is_vip)
               nearestdistance = 0; // if the VIP has been found, keep it
            else
               nearestdistance = distance; // else normal proximity selection rules apply

            pNewEnemy = pPlayer; // bot found an enemy !

            pBot->BotEnemy.appearance_time = *server.time; // save when we first saw this enemy

            // if no enemy alert for about 30 seconds in team, send a radio message
            if (f_team_radiotime[GetTeam (pBot->pEdict)] + 30 < *server.time)
            {
               FakeClientCommand (pBot->pEdict, RADIOMSG_ENEMYSPOTTED); // bot says 'enemy spotted'
               f_team_radiotime[GetTeam (pBot->pEdict)] = *server.time + RANDOM_FLOAT (5.0, 15.0);
               pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_ALERT; // bot says 'alert'
               pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (0.5, 3.0);
            }
         }
      }
   }

   // has the bot found a new enemy ?
   if (!FNullEnt (pNewEnemy))
   {
      //BotNotifySquad (pBot, MESSAGE_SQUAD_NEWENEMY);
      pBot->f_reload_time = *server.time + RANDOM_FLOAT (3.0, 5.0); // set next reload time
   }

   // else bot has found no enemy, so let's zoom out if needed
   else if (((pBot->current_weapon->hardware->id == CS_WEAPON_SCOUT)
             || (pBot->current_weapon->hardware->id == CS_WEAPON_AWP)
             || (pBot->current_weapon->hardware->id == CS_WEAPON_G3SG1)
             || (pBot->current_weapon->hardware->id == CS_WEAPON_SG550)
             || (pBot->current_weapon->hardware->id == CS_WEAPON_AUG)
             || (pBot->current_weapon->hardware->id == CS_WEAPON_SG552))
            && (pBot->pEdict->v.fov < 90))
   {
      // let the bot zoom out
      if (pBot->pEdict->v.button & IN_ATTACK2)
         pBot->pEdict->v.button &= ~IN_ATTACK2;
      else
         pBot->pEdict->v.button |= IN_ATTACK2;
   }

   // is it time to reload ?
   if ((pBot->f_reload_time > 0) && (pBot->f_reload_time <= *server.time))
   {
      BotSwitchToBestWeapon (pBot); // switch to best gun
      pBot->f_reload_time = -1; // so we won't keep reloading
      pBot->pEdict->v.button |= IN_RELOAD; // press reload button
   }

   return (pNewEnemy);
}


void BotSwitchToBestWeapon (bot_t *pBot)
{
//   static weapon_t *suitable_weapons[MAX_BOT_WEAPONS];
//   int suitable_weapons_count = 0;
   bool ammo_left;
   int weapon_index;

   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

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
      if ((pBot->pEdict->v.waterlevel == 3)
          && !(weapons[weapon_index].primary.can_use_underwater && weapons[weapon_index].secondary.can_use_underwater))
         continue; // skip to next weapon

   }*/

   // select the best weapon to use
   for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
   {
      // reset weapon usable state
      ammo_left = FALSE;

      // is the bot NOT carrying this weapon ?
      if (!(pBot->pEdict->v.weapons & (1 << weapons[weapon_index].id)))
         continue; // skip to next weapon

      // is the bot underwater AND does this weapon NOT work under water ?
      if ((pBot->pEdict->v.waterlevel == 3)
          && !(weapons[weapon_index].primary.can_use_underwater || weapons[weapon_index].secondary.can_use_underwater))
         continue; // skip to next weapon

      // is the bot camping AND does this weapon NOT fit for the job ?
      if ((pBot->f_camp_time > *server.time)
          && ((weapons[weapon_index].id == CS_WEAPON_FLASHBANG)
              || (weapons[weapon_index].id == CS_WEAPON_HEGRENADE)
              || (weapons[weapon_index].id == CS_WEAPON_SMOKEGRENADE)
              || (weapons[weapon_index].id == CS_WEAPON_M3)
              || (weapons[weapon_index].id == CS_WEAPON_XM1014)
              || (weapons[weapon_index].id == CS_WEAPON_KNIFE)))
         continue; // skip to next weapon

      // is it the knife AND is the bot still suspicious about enemies around ?
      if ((weapons[weapon_index].id == CS_WEAPON_KNIFE) && (pBot->f_bot_alone_timer > 0) && (pBot->f_bot_alone_timer > *server.time))
         continue; // skip to next weapon

      // is the bot already holding this weapon and there is still ammo in clip ?
      if ((weapons[weapon_index].id == pBot->current_weapon->hardware->id)
          && ((weapons[weapon_index].primary.min_ammo <= 0) || (pBot->current_weapon->clip_ammo < 0)
              || (pBot->current_weapon->clip_ammo >= weapons[weapon_index].primary.min_ammo)))
         ammo_left = TRUE;

      // does this weapon have clips left ?
      if (((weapons[weapon_index].primary.type_of_ammo == -1)
           || (*pBot->bot_weapons[weapon_index].primary_ammo
               >= weapons[weapon_index].primary.min_ammo)))
         ammo_left = TRUE;

      // see if there is enough ammo OR no ammo required
      if (ammo_left)
         break; // at last, bot found the right weapon to kick your ass with

      continue; // weapon not usable, skip to next weapon
   }

   // is this one NOT bot's current weapon ?
   if ((weapons[weapon_index].id > 0) && (pBot->current_weapon->hardware->id != weapons[weapon_index].id))
      FakeClientCommand (pBot->pEdict, weapons[weapon_index].classname); // select this weapon
}


// specifing a weapon_choice allows you to choose the weapon the bot will
// use (assuming enough ammo exists for that weapon)

void BotFireWeapon (bot_t *pBot, Vector v_enemy, int weapon_choice)
{
   bool ammo_left;
   int weapon_index;
   float distance = v_enemy.Length (); // distance to enemy
   TraceResult tr;

   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   // are we burst-firing ?
   if (pBot->current_weapon->primary_charging_time > 0)
   {
      // are we firing with a sniper rifle that can fire bursts ?
      if ((pBot->current_weapon->hardware->id == CS_WEAPON_G3SG1)
          || (pBot->current_weapon->hardware->id == CS_WEAPON_SG550))
         pBot->BotMove.f_forward_time = 0; // don't move while using sniper rifle

      // is it time to stop the burst ?
      if (pBot->current_weapon->primary_charging_time < *server.time)
      {
         pBot->current_weapon->primary_charging_time = 0; // stop firing it

         // set next time to shoot, as next frame will automatically release fire button
         float min_delay = pBot->current_weapon->hardware->primary.min_delay[pBot->pProfile->skill - 1];
         float max_delay = pBot->current_weapon->hardware->primary.max_delay[pBot->pProfile->skill - 1];

         pBot->f_shoot_time = *server.time + RANDOM_FLOAT (min_delay, max_delay) / 2;
         return;
      }
      else
      {
         pBot->pEdict->v.button |= IN_ATTACK; // press the FIRE button
         pBot->f_shoot_time = *server.time; // set next frame to keep pressing the button
         return;
      }
   }

   // we are not already charging a weapon, so select the best one to use
   for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
   {
      // reset weapon usable state
      ammo_left = FALSE;

      // was a weapon choice specified AND if so do they NOT match ?
      if ((weapon_choice != 0) && (weapon_choice != weapons[weapon_index].id))
         continue; // skip to next weapon

      // is the bot NOT carrying this weapon ?
      if (!(pBot->pEdict->v.weapons & (1 << weapons[weapon_index].id)))
         continue; // skip to next weapon

      // is the bot underwater AND does this weapon NOT work under water ?
      if ((pBot->pEdict->v.waterlevel == 3)
          && !(weapons[weapon_index].primary.can_use_underwater || weapons[weapon_index].secondary.can_use_underwater))
         continue; // skip to next weapon

      // is the bot already holding this weapon and there is still ammo in clip ?
      if ((weapons[weapon_index].id == pBot->current_weapon->hardware->id)
          && ((weapons[weapon_index].primary.min_ammo <= 0) || (pBot->current_weapon->clip_ammo < 0)
              || (pBot->current_weapon->clip_ammo >= weapons[weapon_index].primary.min_ammo)))
         ammo_left = TRUE;

      // does this weapon have clips left ?
      if (((weapons[weapon_index].primary.type_of_ammo == -1)
           || (*pBot->bot_weapons[weapon_index].primary_ammo
               >= weapons[weapon_index].primary.min_ammo)))
         ammo_left = TRUE;

      // see if there is enough ammo
      // AND the bot is far enough away to fire
      // AND the bot is close enough to the enemy to fire
      if (ammo_left
          && (distance >= weapons[weapon_index].primary.min_range)
          && (distance <= weapons[weapon_index].primary.max_range))
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
         if (!(pBot->pEdict->v.weapons & (1 << weapons[weapon_index].id)))
            continue; // skip to next weapon

         // is the bot underwater AND does this weapon NOT work under water ?
         if ((pBot->pEdict->v.waterlevel == 3)
             && !(weapons[weapon_index].primary.can_use_underwater || weapons[weapon_index].secondary.can_use_underwater))
            continue; // skip to next weapon

         // is the bot already holding this weapon and there is still ammo in clip ?
         if ((weapons[weapon_index].id == pBot->current_weapon->hardware->id)
             && ((weapons[weapon_index].primary.min_ammo <= 0) || (pBot->current_weapon->clip_ammo < 0)
                 || (pBot->current_weapon->clip_ammo >= weapons[weapon_index].primary.min_ammo)))
            ammo_left = TRUE;

         // does this weapon have clips left ?
         if (((weapons[weapon_index].primary.type_of_ammo == -1)
              || (*pBot->bot_weapons[weapon_index].primary_ammo
                  >= weapons[weapon_index].primary.min_ammo)))
            ammo_left = TRUE;

         // see if there is enough ammo
         if (ammo_left)
            break; // bot finally found a weapon to use

         continue; // weapon not usable, skip to next weapon
      }
   }

   // is this one NOT bot's current weapon ?
   if ((weapons[weapon_index].id > 0) && (pBot->current_weapon->hardware->id != weapons[weapon_index].id))
      FakeClientCommand (pBot->pEdict, weapons[weapon_index].classname); // select this weapon

   // is the bot firing at anything but an enemy ?
   if (FNullEnt (pBot->BotEnemy.pEdict))
   {
      float min_delay = weapons[weapon_index].primary.min_delay[pBot->pProfile->skill - 1];
      float max_delay = weapons[weapon_index].primary.max_delay[pBot->pProfile->skill - 1];

      pBot->pEdict->v.button |= IN_ATTACK; // shoot, and set next time to shoot
      pBot->f_shoot_time = *server.time + RANDOM_FLOAT (min_delay, max_delay);
      return;
   }

   // else the bot really has a big bad evil enemy...

   // is bot just 'standing' on enemy ?
   if ((pBot->pEdict->v.absmin.z == pBot->BotEnemy.pEdict->v.absmax.z)
       && ((pBot->BotEnemy.pEdict->v.origin - pBot->pEdict->v.origin).Length2D () < 46))
      pBot->BotMove.f_duck_time = *server.time + 0.1; // duck to hit him

   // let's see if the bot's crosshair is on the enemy
   UTIL_TraceLine (GetGunPosition (pBot->pEdict), GetGunPosition (pBot->pEdict) + pBot->BotAim.v_forward * 10000,
                   dont_ignore_monsters, ignore_glass, pBot->pEdict, &tr);

   // is this weapon a sniper rifle ?
   if ((weapons[weapon_index].id == CS_WEAPON_SCOUT) || (weapons[weapon_index].id == CS_WEAPON_AWP)
       || (weapons[weapon_index].id == CS_WEAPON_G3SG1) || (weapons[weapon_index].id == CS_WEAPON_SG550))
   {
      pBot->BotMove.f_forward_time = 0; // don't move while using sniper rifle

      // are we moving too fast to use it ?
      if (pBot->pEdict->v.velocity.Length () > 75)
         return; // don't press attack key until velocity is < 75

      // have we aimed with insufficient accuracy to fire ?
      if (!(tr.pHit->v.flags & (FL_CLIENT | FL_MONSTER)) && (RANDOM_LONG (1, 100) < 94 + pBot->pProfile->skill))
         return; // don't shoot yet, it would be a monumental error...

      pBot->pEdict->v.button |= IN_ATTACK; // we will likely hit the enemy, FIRE !!
   }

   // else is this weapon a grenade ?
   else if ((weapons[weapon_index].id == CS_WEAPON_HEGRENADE)
            || (weapons[weapon_index].id == CS_WEAPON_FLASHBANG)
            || (weapons[weapon_index].id == CS_WEAPON_SMOKEGRENADE))
   {
      pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_THROWGRENADE; // bot says 'throwgrenade'
      pBot->BotChat.f_sayaudio_time = *server.time; + RANDOM_FLOAT (0.2, 0.8);

      pBot->pEdict->v.button |= IN_ATTACK; // throw the grenade !!
   }

   // else is it a melee weapon ?
   else if (weapons[weapon_index].id == CS_WEAPON_KNIFE)
   {
      // is bot close enough to its enemy ?
      if ((distance < 60) || (RANDOM_LONG (1, 100) < 2))
      {
         // does the enemy NOT see us ?
         if (!IsInPlayerFOV (pBot->BotEnemy.pEdict, pBot->pEdict->v.origin) || (RANDOM_LONG (1, 100) < pBot->pProfile->skill * 10))
            pBot->pEdict->v.button |= IN_ATTACK2; // if so, use the deadly secondary melee attack
         else
            pBot->pEdict->v.button |= IN_ATTACK; // else use primary melee attack
      }
   }

   // else this is definitely a normal weapon
   else
   {
      // compute the maximum angle deviation to enemy beyond which the bot should NOT fire
      float horizontal_maxangle = tanh ((pBot->BotEnemy.pEdict->v.absmax.x - pBot->BotEnemy.pEdict->v.absmin.x) / (2 * distance)) * (6 - pBot->pProfile->skill);
      float vertical_maxangle = tanh ((pBot->BotEnemy.pEdict->v.absmax.z - pBot->BotEnemy.pEdict->v.absmin.z) / (2 * distance)) * (6 - pBot->pProfile->skill);

      // is the bot's crosshair not on the enemy yet ?
      if (((fabs (pBot->BotAim.v_turn_speed.y) > horizontal_maxangle)
           || (fabs (pBot->BotAim.v_turn_speed.x) > vertical_maxangle))
          && (distance > 200 * (6 - pBot->pProfile->skill)))
         return; // then don't waste precious bullets

      pBot->pEdict->v.button |= IN_ATTACK; // else we will likely hit the enemy, FIRE !!
   }

   // should we use burst-fire ?
   if ((pBot->pProfile->skill > 2) && (weapons[weapon_index].primary.should_charge)
       && (distance > 200 * (6 - pBot->pProfile->skill)))
   {
      float burst_delay = weapons[weapon_index].primary.charge_delay + ((6 - pBot->pProfile->skill) / 10);

      pBot->current_weapon->primary_charging_time = *server.time + burst_delay; // set the delay of burst
      pBot->f_shoot_time = *server.time; // set next frame to keep pressing the button
   }

   // else this was a normal shoot
   else
   {
      // should we hold button down to fire ?
      if (weapons[weapon_index].primary.should_hold)
      {
         // if no need for ammo OR no need for clip OR still enough ammo in clip...
         if ((weapons[weapon_index].primary.min_ammo <= 0) || (pBot->current_weapon->clip_ammo < 0)
             || (pBot->current_weapon->clip_ammo >= weapons[weapon_index].primary.min_ammo))
            pBot->f_shoot_time = *server.time; // set next frame to keep pressing the button
         else
         {
            pBot->f_shoot_time = *server.time + 0.25; // delay a while and reload
            pBot->pEdict->v.button = IN_RELOAD; // press the RELOAD button
         }
      }

      // else set next time to shoot
      else
      {
         float min_delay = weapons[weapon_index].primary.min_delay[pBot->pProfile->skill - 1];
         float max_delay = weapons[weapon_index].primary.max_delay[pBot->pProfile->skill - 1];

         pBot->f_shoot_time = *server.time + RANDOM_FLOAT (min_delay, max_delay);
      }
   }

   return; // finished
}


void BotShootAtEnemy (bot_t *pBot)
{
   float f_distance;
   Vector v_enemy, v_target_angle;
   bool has_proximity_weapon;
   bool has_sniper_rifle;
   bool has_zoomable_rifle;
   bool has_grenade;
   bool enemy_sees_us;
   bool enemy_ducking;

   if (!IsValidPlayer (pBot->pEdict) || FNullEnt (pBot->BotEnemy.pEdict))
      return; // reliability check

   // target some part of our enemy's body
   v_enemy = BotCanSeeOfEntity (pBot, pBot->BotEnemy.pEdict) - GetGunPosition (pBot->pEdict);
   f_distance = v_enemy.Length (); // how far away is that scum ?

   // determine what type of weapon the bot is holding in its hands
   has_proximity_weapon = (pBot->current_weapon->hardware->id == CS_WEAPON_KNIFE);
   has_sniper_rifle = ((pBot->current_weapon->hardware->id == CS_WEAPON_SCOUT)
                       || (pBot->current_weapon->hardware->id == CS_WEAPON_AWP)
                       || (pBot->current_weapon->hardware->id == CS_WEAPON_G3SG1)
                       || (pBot->current_weapon->hardware->id == CS_WEAPON_SG550));
   has_zoomable_rifle = ((pBot->current_weapon->hardware->id == CS_WEAPON_AUG)
                         || (pBot->current_weapon->hardware->id == CS_WEAPON_SG552));
   has_grenade = ((pBot->current_weapon->hardware->id == CS_WEAPON_HEGRENADE)
                  || (pBot->current_weapon->hardware->id == CS_WEAPON_SMOKEGRENADE)
                  || (pBot->current_weapon->hardware->id == CS_WEAPON_FLASHBANG));

   // determine the current state of the enemy
   enemy_sees_us = IsInPlayerFOV (pBot->BotEnemy.pEdict, pBot->pEdict->v.origin);
   enemy_ducking = ((pBot->BotEnemy.pEdict->v.button & IN_DUCK) == IN_DUCK);

   // move the aim cursor and compensate for the recoil
   BotSetIdealAngles (pBot, WrapAngles (UTIL_VecToAngles (v_enemy) + pBot->pEdict->v.punchangle / (6 - pBot->pProfile->skill)));

   // decide which behaviour the bot will have while firing
   if (f_distance > 200)
   {
      // if not fearful, no sniper gun and won't fall OR proximity weapon...
      if ((!pBot->b_is_fearful && !has_sniper_rifle && !(pBot->BotBody.hit_state & OBSTACLE_FRONT_FALL))
          || has_proximity_weapon)
      {
         pBot->BotMove.f_forward_time = *server.time + 60.0; // run to enemy
         pBot->BotMove.f_walk_time = 0.0;
      }

      // else bot is sniper or path to enemy is blocked, so if not fearful OR quite far from
      // enemy, try to keep the distance
      else if (!pBot->b_is_fearful || (f_distance > 500))
         pBot->BotMove.f_forward_time = 0; // try to stay at a distant range from our enemy

      // else if enemy sees us, bot may be fearful, or enemy too close, so step back
      else if (enemy_sees_us)
         pBot->BotMove.f_backwards_time = *server.time + 0.5; // make some steps back

      // if the bot's enemy sees it AND bot is skilled enough to strafe AND is not strafing
      // for a few seconds already, pick up a random strafe direction
      if (enemy_sees_us && (pBot->pProfile->skill > 2)
          && (pBot->BotMove.f_strafeleft_time + 6 - pBot->pProfile->skill < *server.time)
          && (pBot->BotMove.f_straferight_time + 6 - pBot->pProfile->skill < *server.time))
      {
         if (RANDOM_LONG (1, 100) < 50)
            pBot->BotMove.f_strafeleft_time = *server.time + RANDOM_FLOAT (0.5, 2.0);
         else
            pBot->BotMove.f_straferight_time = *server.time + RANDOM_FLOAT (0.5, 2.0);
      }

      // else if bot is already strafing AND bot has not jumped for 0.7 s
      // AND bot hasn't some sort of sniper gun AND bot is skilled enough, randomly jump
      else if ((pBot->BotMove.f_strafe_speed != 0) && (pBot->BotMove.f_jump_time + 0.7 < *server.time)
               && !has_sniper_rifle && (pBot->pProfile->skill > 2))
         pBot->BotMove.f_jump_time = *server.time + RANDOM_FLOAT (0.1, (6.0 - pBot->pProfile->skill) / 4); //jump

      // else if enemy sees us, bot skilled enough and not currently strafing, duck
      else if (enemy_sees_us && (pBot->pProfile->skill > 2) && (pBot->BotMove.f_strafeleft_time + 0.6 < *server.time)
               && (pBot->BotMove.f_straferight_time + 0.6 < *server.time))
      {
         pBot->BotMove.f_duck_time = *server.time + 0.8; // duck and fire
         if ((pBot->BotEnemy.appearance_time + 5.0 < *server.time)
             || ((pBot->BotEnemy.appearance_time + 0.5 < *server.time)
                 && (has_proximity_weapon || has_grenade)))
            pBot->b_is_fearful = FALSE; // prevent bot from never attacking
      }

      // if enemy does NOT see us and bot hasn't any sort of proximity weapon, duck
      if (!enemy_sees_us && !has_proximity_weapon)
         pBot->BotMove.f_duck_time = *server.time + 0.2; // duck and fire
   }

   else if (f_distance > 20)
   {
      pBot->BotMove.f_forward_time = *server.time + 60.0; // walk if distance is closer

      // if enemy doesn't see us, walk
      if (!enemy_sees_us)
         pBot->BotMove.f_walk_time = *server.time + 0.2;

      // if bot is fearful and not currently strafing AND enemy is not ducking yet, duck
      if (pBot->b_is_fearful && (pBot->BotMove.f_strafeleft_time + 0.6 < *server.time)
          && (pBot->BotMove.f_straferight_time + 0.6 < *server.time)
          && !enemy_ducking)
      {
         pBot->BotMove.f_duck_time = *server.time + 0.8; // duck and fire
         if ((pBot->BotEnemy.appearance_time + 5.0 < *server.time)
             || ((pBot->BotEnemy.appearance_time + 0.5 < *server.time)
                 && (has_proximity_weapon || has_grenade)))
            pBot->b_is_fearful = FALSE; // prevent bot from never attacking
      }
   }

   else
      pBot->BotMove.f_forward_time = 0; // don't move if close enough

   if (pBot->f_shoot_time > *server.time)
      return; // don't shoot if not time to yet

   // is the bot holding a sniper rifle ?
   if (has_sniper_rifle)
   {
      // should the bot switch to the long-range zoom ?
      if ((f_distance > 600) && (pBot->pEdict->v.fov >= 40))
      {
         // then let the bot zoom in
         if (pBot->pEdict->v.button & IN_ATTACK2)
            pBot->pEdict->v.button &= ~IN_ATTACK2;
         else
            pBot->pEdict->v.button |= IN_ATTACK2;

         return; // don't shoot right now, bot is zooming in
      }

      // else should the bot switch to the close-range zoom ?
      else if ((f_distance > 150) && (pBot->pEdict->v.fov >= 90))
      {
         // then let the bot zoom in
         if (pBot->pEdict->v.button & IN_ATTACK2)
            pBot->pEdict->v.button &= ~IN_ATTACK2;
         else
            pBot->pEdict->v.button |= IN_ATTACK2;

         return; // don't shoot right now, bot is zooming in
      }

      // else should the bot restore the normal view ?
      else if ((f_distance <= 150) && (pBot->pEdict->v.fov < 90))
      {
         // then let the bot zoom out
         if (pBot->pEdict->v.button & IN_ATTACK2)
            pBot->pEdict->v.button &= ~IN_ATTACK2;
         else
            pBot->pEdict->v.button |= IN_ATTACK2;

         return; // don't shoot right now, bot is zooming out
      }
   }

   // else is the bot holding a zoomable rifle ?
   else if (has_zoomable_rifle)
   {
      // should the bot switch to zoomed mode ?
      if ((f_distance > 400) && (pBot->pEdict->v.fov >= 90))
      {
         // then let the bot zoom in
         if (pBot->pEdict->v.button & IN_ATTACK2)
            pBot->pEdict->v.button &= ~IN_ATTACK2;
         else
            pBot->pEdict->v.button |= IN_ATTACK2;

         return; // don't shoot right now, bot is zooming in
      }

      // else should the bot restore the normal view ?
      else if ((f_distance <= 400) && (pBot->pEdict->v.fov < 90))
      {
         // then let the bot zoom out
         if (pBot->pEdict->v.button & IN_ATTACK2)
            pBot->pEdict->v.button &= ~IN_ATTACK2;
         else
            pBot->pEdict->v.button |= IN_ATTACK2;

         return; // don't shoot right now, bot is zooming out
      }
   }

   BotFireWeapon (pBot, v_enemy, 0); // select the best weapon at this distance and fire
}


void BotPlantBomb (bot_t *pBot, Vector v_target)
{
   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   // if bomb icon is blinking, plant the bomb...
   if (pBot->b_can_plant)
   {
      pBot->bot_task |= BOT_TASK_PLANTING; // make the bot remember he is planting a bomb
      FakeClientCommand (pBot->pEdict, RADIOMSG_GONNABLOW); // bot speaks, "get outta here!"
   }

   // else let's run there...
   else
      BotReachPosition (pBot, v_target);
}


void BotDefuseBomb (bot_t *pBot, edict_t *pBomb)
{
   if (!IsValidPlayer (pBot->pEdict) || (pBomb == NULL))
      return; // reliability check

   // if close to bomb, defuse it...
   if ((pBomb->v.origin - pBot->pEdict->v.origin).Length () < 40)
   {
      pBot->bot_task |= BOT_TASK_DEFUSING; // make the bot remember he is defusing a bomb
      pBot->v_goal = pBomb->v.origin; // make the bot remember the bomb location
      FakeClientCommand (pBot->pEdict, RADIOMSG_NEEDBACKUP); // bot speaks, "i need backup!"
   }

   // else if getting close, either duck or slow down
   else if ((pBomb->v.origin - pBot->pEdict->v.origin).Length () < 100)
   {
      pBot->BotMove.f_forward_time = *server.time + 60.0; // go ahead...
      if (pBomb->v.origin.z < pBot->pEdict->v.origin.z)
      {
         pBot->BotMove.f_duck_time = *server.time + 0.5; // ...but either duck...
         pBot->BotMove.f_walk_time = 0.0;
      }
      else
         pBot->BotMove.f_walk_time = *server.time + 0.2; // ...or walk
      BotSetIdealAngles (pBot, UTIL_VecToAngles (pBomb->v.origin - GetGunPosition (pBot->pEdict))); // look at bomb
   }

   // else let's run there...
   else
      BotReachPosition (pBot, pBomb->v.origin);
}
