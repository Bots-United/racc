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
// DMC version
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
   edict_t *pPlayer, *pent;

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
      // is it a shootable button ?
      if (strcmp (STRING (pBot->BotEnemy.pEdict->v.classname), "func_button") == 0)
      {
         // has the bot just discovered it ?
         if (pBot->f_interact_time + 0.5 < *server.time)
            BotShootButton (pBot); // if so, shoot at the button
         else
         {
            memset (&pBot->BotEnemy, 0, sizeof (pBot->BotEnemy)); // else null out enemy structures
            memset (&pBot->LastSeenEnemy, 0, sizeof (pBot->LastSeenEnemy));
            pBot->f_find_item_time = *server.time + 5.0; // don't look for items for 5 seconds
         }

         return (NULL); // bot doesn't have any "real" enemy
      }

      // is the enemy dead ?
      if (!IsAlive (pBot->BotEnemy.pEdict))  // is the enemy dead?, assume bot killed it
      {
         if (RANDOM_LONG (1,100) <= (56 - 2 * player_count))
            pBot->BotChat.bot_saytext = BOT_SAYTEXT_LAUGH; // bot laughs (text)

         // don't complain about this enemy anymore but hail for the victory instead
         pBot->BotChat.bot_sayaudio &= ~(BOT_SAYAUDIO_ATTACKING | BOT_SAYAUDIO_TAKINGDAMAGE);
         pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_VICTORY; // bot laughs (audio)
         pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (0.5, 3.0);

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
         // don't look for closer enemies if bot's current enemy is not "human-like"
         if (pBot->BotEnemy.pEdict->v.flags & FL_CLIENT)
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

               if (server.is_teamplay && (GetTeam (pBot->pEdict) == GetTeam (pPlayer)))
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

         pBot->f_reload_time = *server.time + RANDOM_LONG (3.0, 5.0); // set next reload time

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
            pBot->f_pause_time = *server.time + RANDOM_FLOAT (2.0, 8.0 - pBot->pProfile->skill); // pause for a while
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
      if (server.is_teamplay && (GetTeam (pBot->pEdict) == GetTeam (pPlayer)))
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

         if (distance < nearestdistance)
         {
            nearestdistance = distance; // update nearest distance
            pNewEnemy = pPlayer; // bot found an enemy !

            pBot->BotEnemy.appearance_time = *server.time; // save when we first saw this enemy
         }
      }
   }

   // if still no enemy in sight, look for shootable buttons
   if (FNullEnt (pNewEnemy) && (pBot->f_find_item_time < *server.time))
   {
      // loop through all buttons...
      while ((pent = UTIL_FindEntityByString (pent, "classname", "func_button")) != NULL)
      {
         // is this button visible AND shootable ?
         if ((BotCanSeeOfEntity (pBot, pent) != g_vecZero) && (pent->v.health > 0))
         {
            float distance = (pent->v.origin - pBot->pEdict->v.origin).Length ();

            // is this the closest shootable button ?
            if (distance < nearestdistance)
            {
               nearestdistance = distance; // update nearest distance
               pNewEnemy = pent; // bot found a shootable button !
               pBot->f_interact_time = *server.time; // save when we first saw this button
               break; // don't look for anything else
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

   // is it time to reload ?
   if ((pBot->f_reload_time > 0) && (pBot->f_reload_time <= *server.time))
   {
      BotSwitchToBestWeapon (pBot); // switch to best gun;
      pBot->f_reload_time = -1;  // so we won't keep reloading
      if (RANDOM_LONG (1, 100) <= 80)
         pBot->pEdict->v.button |= IN_RELOAD;  // press reload button (but can forget...)
      pBot->f_shoot_time = *server.time; // reset next shoot time
   }

   return (pNewEnemy);
}


void BotSelectWeapon (bot_t *pBot, int weapon_slot)
{
   // in DMC, we need to issue weapon change commands by the weaponselect field of the
   // usercmd structure, as just typing the weapon names on the console doesn't work...

   static usercmd_t usercmd_botchangeweapon;

   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   usercmd_botchangeweapon.lerp_msec = 0;
   usercmd_botchangeweapon.msec = 0;
   usercmd_botchangeweapon.viewangles = pBot->pEdict->v.v_angle;
   usercmd_botchangeweapon.forwardmove = 0;
   usercmd_botchangeweapon.sidemove = 0;
   usercmd_botchangeweapon.upmove = 0;
   usercmd_botchangeweapon.lightlevel = 127;
   usercmd_botchangeweapon.buttons = 0;
   usercmd_botchangeweapon.impulse = 0;
   usercmd_botchangeweapon.weaponselect = weapon_slot; // fill in with wanted weapon slot number
   usercmd_botchangeweapon.impact_index = 0;
   usercmd_botchangeweapon.impact_position = g_vecZero;

   CmdStart (pBot->pEdict, &usercmd_botchangeweapon, 0); // issue the select item command
   CmdEnd (pBot->pEdict); // issue end of command notifier
}


void BotSwitchToBestWeapon (bot_t *pBot)
{
   int weapon_index;

   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   // select the best weapon to use
   for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
   {
      // is the bot NOT carrying this weapon ?
      if (!(pBot->pEdict->v.weapons & weapons[weapon_index].id))
         continue; // skip to next weapon

      // is the bot underwater AND does this weapon NOT work under water ?
      if ((pBot->pEdict->v.waterlevel == 3)
          && !(weapons[weapon_index].primary.can_use_underwater || weapons[weapon_index].secondary.can_use_underwater))
         continue; // skip to next weapon

      // see if there is enough primary ammo
      if ((weapons[weapon_index].primary.type_of_ammo == -1)
          || (*pBot->bot_weapons[weapon_index].primary_ammo >= weapons[weapon_index].primary.min_ammo))
         break; // at last, bot found the right weapon to kick your ass with

      continue; // weapon not usable, skip to next weapon
   }

   // is this one NOT bot's current weapon ?
   if ((weapons[weapon_index].id > 0) && (pBot->current_weapon->hardware->id != weapons[weapon_index].id))
      BotSelectWeapon (pBot, weapons[weapon_index].slot); // select this weapon
}


// specifing a weapon_choice allows you to choose the weapon the bot will
// use (assuming enough ammo exists for that weapon)

void BotFireWeapon (bot_t *pBot, Vector v_enemy, int weapon_choice)
{
   int weapon_index;
   float distance = v_enemy.Length (); // distance to enemy
   TraceResult tr;

   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   // select the best weapon to use
   for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
   {
      // was a weapon choice specified AND if so do they NOT match ?
      if ((weapon_choice != 0) && (weapon_choice != weapons[weapon_index].id))
         continue; // skip to next weapon

      // is the bot NOT carrying this weapon ?
      if (!(pBot->pEdict->v.weapons & weapons[weapon_index].id))
         continue; // skip to next weapon

      // is the bot underwater AND does this weapon NOT work under water ?
      if ((pBot->pEdict->v.waterlevel == 3)
          && !(weapons[weapon_index].primary.can_use_underwater || weapons[weapon_index].secondary.can_use_underwater))
         continue; // skip to next weapon

      // see if there is enough primary ammo
      // AND the bot is far enough away to use primary fire
      // AND the bot is close enough to the enemy to use primary fire
      if (((weapons[weapon_index].primary.type_of_ammo == -1)
           || (*pBot->bot_weapons[weapon_index].primary_ammo >= weapons[weapon_index].primary.min_ammo))
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
         // is the bot NOT carrying this weapon ?
         if (!(pBot->pEdict->v.weapons & weapons[weapon_index].id))
            continue; // skip to next weapon

         // is the bot underwater AND does this weapon NOT work under water ?
         if ((pBot->pEdict->v.waterlevel == 3)
             && !(weapons[weapon_index].primary.can_use_underwater || weapons[weapon_index].secondary.can_use_underwater))
            continue; // skip to next weapon

         // see if there is enough primary ammo
         if ((weapons[weapon_index].primary.type_of_ammo == -1)
             || (*pBot->bot_weapons[weapon_index].primary_ammo >= weapons[weapon_index].primary.min_ammo))
            break; // at last, bot found the right weapon to kick your ass with

         continue; // weapon not usable, skip to next weapon
      }
   }

   // is this one NOT bot's current weapon ?
   if ((weapons[weapon_index].id > 0) && (pBot->current_weapon->hardware->id != weapons[weapon_index].id))
      BotSelectWeapon (pBot, weapons[weapon_index].slot); // select this weapon

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

   // is it the rocket launcher ?
   if (weapons[weapon_index].id == DMC_WEAPON_ROCKETLAUNCHER)
   {
      // is it unsafe to fire a rocket here ?

      // check at gun level to take rocket blast radius in account
      UTIL_TraceHull (GetGunPosition (pBot->pEdict),
                      GetGunPosition (pBot->pEdict) + pBot->BotAim.v_forward * 150,
                      ignore_monsters, head_hull, pBot->pEdict, &tr);
      if (tr.flFraction < 1.0)
         return; // if hit something, then it is unsafe to fire here
   }

   // else if it is a normal weapon
   else if (weapons[weapon_index].id != DMC_WEAPON_AXE)
   {
      // compute the maximum angle deviation to enemy beyond which the bot should NOT fire
      float horizontal_maxangle = tanh ((pBot->BotEnemy.pEdict->v.absmax.x - pBot->BotEnemy.pEdict->v.absmin.x) / (2 * distance)) * (6 - pBot->pProfile->skill);
      float vertical_maxangle = tanh ((pBot->BotEnemy.pEdict->v.absmax.z - pBot->BotEnemy.pEdict->v.absmin.z) / (2 * distance)) * (6 - pBot->pProfile->skill);

      // is the bot's crosshair not on the enemy yet ?
      if (((fabs (pBot->BotAim.v_turn_speed.y) > horizontal_maxangle)
           || (fabs (pBot->BotAim.v_turn_speed.x) > vertical_maxangle))
          && (distance > 200 * (6 - pBot->pProfile->skill)))
         return; // then don't waste precious bullets
   }

   // if NOT in the case where it is a proximity weapon and bot is far from his enemy...
   if (!((weapons[weapon_index].id == DMC_WEAPON_AXE) && (distance > 50))
        || (RANDOM_LONG (1, 100) < 2))
      pBot->pEdict->v.button |= IN_ATTACK; // press the FIRE button

   // should we hold button down to fire ?
   if (weapons[weapon_index].primary.should_hold)
      pBot->f_shoot_time = *server.time; // set next frame to keep pressing the button

   // else set next time to shoot
   else
   {
      float min_delay = weapons[weapon_index].primary.min_delay[pBot->pProfile->skill - 1];
      float max_delay = weapons[weapon_index].primary.max_delay[pBot->pProfile->skill - 1];
      
      pBot->f_shoot_time = *server.time + RANDOM_FLOAT (min_delay, max_delay);
   }
}


void BotShootAtEnemy (bot_t *pBot)
{
   float f_distance;
   Vector v_enemy, v_target_angle;
   bool enemy_sees_us;

   if (!IsValidPlayer (pBot->pEdict) || FNullEnt (pBot->BotEnemy.pEdict))
      return; // reliability check

   // target some part of our enemy's body
   v_enemy = BotCanSeeOfEntity (pBot, pBot->BotEnemy.pEdict) - GetGunPosition (pBot->pEdict);
   f_distance = v_enemy.Length (); // how far away is the enemy scum ?

   // determine the current state of the enemy
   enemy_sees_us = IsInPlayerFOV (pBot->BotEnemy.pEdict, pBot->pEdict->v.origin);

   // move the aim cursor and compensate for the recoil
   BotSetIdealAngles (pBot, WrapAngles (UTIL_VecToAngles (v_enemy) + pBot->pEdict->v.punchangle / (6 - pBot->pProfile->skill)));

   // decide which behaviour the bot will have while firing
   if (f_distance > 200)
   {
      // if not fearful and won't fall OR proximity weapon, run to enemy
      if ((!pBot->b_is_fearful && !(pBot->BotBody.hit_state & OBSTACLE_FRONT_FALL))
          || (pBot->current_weapon->hardware->id == DMC_WEAPON_AXE))
      {
         pBot->BotMove.f_forward_time = *server.time + 60.0; // if not fearful and no sniper...
         pBot->BotMove.f_walk_time = 0.0; // ...run if enemy is far
      }

      // else keep the distance
      else
         pBot->BotMove.f_forward_time = 0; // try to stay at a distant range from our enemy

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

      // else if already strafing and not jumped for 0.7 s (and bot skilled enough), randomly jump
      else if ((pBot->BotMove.f_strafe_speed != 0) && (pBot->BotMove.f_jump_time + 0.7 < *server.time) && (pBot->pProfile->skill > 1))
         pBot->BotMove.f_jump_time = *server.time + RANDOM_FLOAT (0.1, (6.0 - pBot->pProfile->skill) / 4); //jump

      // else if fearful and not currently strafing
      else if ((pBot->b_is_fearful) && (pBot->BotMove.f_strafeleft_time + 0.6 < *server.time)
               && (pBot->BotMove.f_straferight_time + 0.6 < *server.time))
      {
         if ((pBot->BotEnemy.appearance_time + 5.0 < *server.time)
             || ((pBot->BotEnemy.appearance_time + 0.5 < *server.time)
                 && (pBot->current_weapon->hardware->id == DMC_WEAPON_AXE)))
            pBot->b_is_fearful = FALSE; // prevent bot from never attacking
      }
   }

   else if (f_distance > 20)
   {
      pBot->BotMove.f_forward_time = *server.time + 60.0; // walk if distance is closer
      pBot->BotMove.f_walk_time = *server.time + 0.2;
   }

   else
      pBot->BotMove.f_forward_time = 0; // don't move if close enough

   if (pBot->f_shoot_time > *server.time)
      return; // don't shoot if not time to yet

   BotFireWeapon (pBot, v_enemy, 0); // select the best weapon at this distance and fire
}


void BotShootButton (bot_t *pBot)
{
   Vector v_button;

   if (!IsValidPlayer (pBot->pEdict) || FNullEnt (pBot->BotEnemy.pEdict))
      return; // reliability check

   v_button = VecBModelOrigin (pBot->BotEnemy.pEdict) - GetGunPosition (pBot->pEdict);

   // move the aim cursor and compensate for the recoil
   BotSetIdealAngles (pBot, WrapAngles (UTIL_VecToAngles (v_button) + pBot->pEdict->v.punchangle / (6 - pBot->pProfile->skill)));

   // aim at the button and fire the shotgun
   BotFireWeapon (pBot, v_button, DMC_WEAPON_QUAKEGUN);
}
