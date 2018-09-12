// RACC - AI development project for first-person shooter games derivated from Valve's Half-Life
// (http://www.racc-ai.com/)
//
// This project is based on the work done by Jeffrey 'Botman' Broome in his HPB bot
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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "usercmd.h"
#include "bot_common.h"
#include "bot_specific.h"

extern bot_weapon_t weapon_defs[MAX_WEAPONS];
extern bool b_observer_mode;
extern bool is_team_play;
extern float pause_time[5][2];

// these dll.cpp functions are used in BotSelectItem()
void CmdStart (const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed);
void CmdEnd (const edict_t *player);


typedef struct
{
   int   iId; // the weapon ID value
   int   skill_level; // bot skill must at least equal this value for bot to use this weapon
   float primary_min_distance; // 0 = no minimum
   float primary_max_distance; // 9999 = no maximum
   bool  can_use_underwater; // can use this weapon underwater
   int   min_primary_ammo; // minimum ammout of primary ammo needed to fire
   bool  primary_fire_hold; // hold down primary fire button to use?
} bot_weapon_select_t;

typedef struct
{
   int iId;
   float primary_base_delay;
   float primary_min_delay[5];
   float primary_max_delay[5];
} bot_fire_delay_t;


// weapons are stored in priority order, most desired weapon should be at
// the start of the array and least desired should be at the end
bot_weapon_select_t dmc_weapon_select[] = {
   {DMC_WEAPON_SUPERNAILGUN, 1, 0.0, 1500.0, TRUE, 1, TRUE},
   {DMC_WEAPON_ROCKETLAUNCHER, 2, 180.0, 9999.0, TRUE, 1, TRUE},
   {DMC_WEAPON_SUPERSHOTGUN, 1, 0.0, 750.0, TRUE, 1, TRUE},
   {DMC_WEAPON_GRENADELAUNCHER, 2, 150.0, 1000.0, TRUE, 1, TRUE},
   {DMC_WEAPON_NAILGUN, 1, 0.0, 750.0, TRUE, 1, TRUE},
   {DMC_WEAPON_AXE, 2, 0.0, 50.0, TRUE, 0, TRUE},
   {DMC_WEAPON_LIGHTNING, 1, 0.0, 9999.0, FALSE, 1, TRUE},
   {DMC_WEAPON_QUAKEGUN, 1, 50.0, 9999.0, TRUE, 0, TRUE},
   /* terminator */
   {0, 0, 0.0, 0.0, TRUE, 1, FALSE}
};


// weapon firing delay based on skill (min and max delay for each weapon)
// THESE MUST MATCH THE SAME ORDER AS THE WEAPON SELECT ARRAY!!!
bot_fire_delay_t dmc_fire_delay[] = {
   {DMC_WEAPON_SUPERNAILGUN, 0.1, {0.5, 0.4, 0.25, 0.1, 0.0}, {0.8, 0.65, 0.45, 0.3, 0.1}},
   {DMC_WEAPON_ROCKETLAUNCHER, 0.7, {0.7, 0.5, 0.4, 0.3, 0.2}, {1.5, 1.0, 0.9, 0.7, 0.5}},
   {DMC_WEAPON_SUPERSHOTGUN, 0.4, {0.8, 0.6, 0.4, 0.2, 0.0}, {2.0, 1.2, 0.8, 0.5, 0.25}},
   {DMC_WEAPON_GRENADELAUNCHER, 0.75, {1.0, 0.8, 0.5, 0.2, 0.0}, {1.3, 1.0, 0.7, 0.4, 0.25}},
   {DMC_WEAPON_NAILGUN, 0.1, {0.5, 0.4, 0.25, 0.1, 0.0}, {0.8, 0.65, 0.45, 0.3, 0.1}},
   {DMC_WEAPON_AXE, 0.1, {0.3, 0.2, 0.1, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {DMC_WEAPON_LIGHTNING, 0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}},
   {DMC_WEAPON_QUAKEGUN, 0.1, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   /* terminator */
   {0, 0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}},
};



edict_t *BotCheckForEnemies (bot_t *pBot)
{
   Vector vecEnd;
   edict_t *pNewEnemy = NULL;
   edict_t *pent = NULL;
   float nearestdistance = 2500;
   int i;

   if (pBot->pEdict == NULL)
      return NULL; // reliability check

   if (pBot->pBotEnemy != NULL)  // does the bot already have an enemy?
   {
      // is it a shootable button ?
      if (strcmp (STRING (pBot->pBotEnemy->v.classname), "func_button") == 0)
      {
         // has the bot just discovered it ?
         if (pBot->f_interact_time + 0.5 < gpGlobals->time)
            BotShootButton (pBot); // if so, shoot at the button
         else
         {
            pBot->pBotEnemy = NULL; // else nulls out enemy pointer
            pBot->f_find_item_time = gpGlobals->time + 5.0; // don't look for items for 5 seconds
         }

         return NULL; // bot doesn't have any "real" enemy
      }

      // if the enemy is dead?
      if (!IsAlive (pBot->pBotEnemy))  // is the enemy dead?, assume bot killed it
      {
         if (RANDOM_LONG (1,100) <= (56 - 2 * gpGlobals->maxClients))
         {
            pBot->BotChat.b_saytext_kill = TRUE; // bot laughs
            pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (3.0, 5.0);
         }

         // if this enemy was close to us, spray a logo
         if (((pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length () < 200) && (RANDOM_LONG (1, 100) < 33))
            pBot->f_spraying_logo_time = gpGlobals->time + RANDOM_FLOAT (1.0, 2.0);

         pBot->b_enemy_hidden = FALSE; // have no enemy anymore
         pBot->pVictimEntity = pBot->pBotEnemy; // bot remembers his victim
         pBot->pBotEnemy = NULL; // and nulls out enemy pointer
      }

      // else if enemy is still visible and in field of view, look for a greater threat, else keep it
      else if (FInViewCone (pBot->pBotEnemy->v.origin, pBot->pEdict)
               && (BotGetIdealAimVector (pBot, pBot->pBotEnemy) != Vector (0, 0, 0)))
      {
         // don't look for closer enemies if bot's current enemy is not "human-like"
         if ((pBot->pBotEnemy->v.flags & FL_CLIENT) || (pBot->pBotEnemy->v.flags & FL_FAKECLIENT))
         {
            // search the world for players...
            for (i = 1; i <= gpGlobals->maxClients; i++)
            {
               edict_t *pPlayer = INDEXENT (i);

               if (!pPlayer || pPlayer->free || (pPlayer == pBot->pEdict))
                  continue; // skip invalid players and skip self (i.e. this bot)

               if (!IsAlive (pPlayer))
                  continue; // skip this player if not alive (i.e. dead or dying)

               if (b_observer_mode && !(pPlayer->v.flags & FL_FAKECLIENT))
                  continue; // skip this player if real client and we are in observer mode

               if (is_team_play && (GetTeam (pBot->pEdict) == GetTeam (pPlayer)))
                  continue; // skip this player if it is a teammate

               // see if bot can see the player...
               if (FInViewCone (pPlayer->v.origin, pBot->pEdict) && (BotGetIdealAimVector (pBot, pPlayer) != Vector (0, 0, 0)))
               {
                  float distance = (pPlayer->v.origin - pBot->pEdict->v.origin).Length ();

                  if (distance < nearestdistance)
                  {
                     nearestdistance = distance; // update nearest distance
                     pBot->pBotEnemy = pPlayer; // bot found a greater threat !
                  }
               }
            }
         }

         // face the enemy
         Vector bot_angles = UTIL_VecToAngles (pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin);
         BotSetIdealYaw (pBot, bot_angles.y);

         pBot->f_reload_time = gpGlobals->time + RANDOM_LONG (3.0, 5.0); // set next reload time
         pBot->v_lastseenenemy_position = pBot->pBotEnemy->v.origin; // save enemy position

         return (pBot->pBotEnemy);
      }

      // else bot lost his enemy
      else
      {
         pBot->f_lost_enemy_time = gpGlobals->time; // save lost enemy time
         pBot->BotMove.f_strafeleft_time = 0; // stop strafing
         pBot->BotMove.f_straferight_time = 0;
         if (RANDOM_LONG (1, 100) < 33)
            pBot->f_pause_time = gpGlobals->time + RANDOM_FLOAT (2.0, 7.0); // pause for a while
      }
   }

   // no enemy yet, let's search the world for players...
   for (i = 1; i <= gpGlobals->maxClients; i++)
   {
      edict_t *pPlayer = INDEXENT (i);

      if (!pPlayer || pPlayer->free || (pPlayer == pBot->pEdict))
         continue; // skip invalid players and skip self (i.e. this bot)

      if (!IsAlive (pPlayer))
         continue; // skip this player if not alive (i.e. dead or dying)

      if (b_observer_mode && !(pPlayer->v.flags & FL_FAKECLIENT))
         continue; // skip real players in observater mode

      // if it is a teammate, check if this teammate is firing in some direction
      if (is_team_play && (GetTeam (pBot->pEdict) == GetTeam (pPlayer)))
      {
         // check if this teammate is visible and relatively close
         if ((BotGetIdealAimVector (pBot, pPlayer) != Vector (0, 0, 0))
             && ((pPlayer->v.origin - pBot->pEdict->v.origin).Length () < 750))
         {
            // if this teammate is attacking something
            if ((pPlayer->v.button & IN_ATTACK) == IN_ATTACK)
            {
               // if this teammate is far, come to him
               if ((pPlayer->v.origin - pBot->pEdict->v.origin).Length () > 200)
                  pBot->v_reach_point = pPlayer->v.origin; // "i'm coming, mate !!"

               // else if we are close to him...
               else
               {
                  BotSetIdealYaw (pBot, pPlayer->v.v_angle.y); // look where he is looking
                  pBot->f_reach_time = gpGlobals->time + 0.5; // let the bot turn
               }
            }
         }

         continue; // don't target your teammates...
      }

      vecEnd = pPlayer->v.origin + pPlayer->v.view_ofs;

      // see if bot can see the player...
      if (FInViewCone (vecEnd, pBot->pEdict) && (BotGetIdealAimVector (pBot, pPlayer) != Vector (0, 0, 0)))
      {
         float distance = (pPlayer->v.origin - pBot->pEdict->v.origin).Length ();

         if (distance < nearestdistance)
         {
            nearestdistance = distance; // update nearest distance
            pNewEnemy = pPlayer; // bot found an enemy !

            pBot->f_see_enemy_time = gpGlobals->time; // save when we first saw this enemy
            pBot->f_aim_adjust_time = gpGlobals->time; // start adjusting aim now
         }
      }
   }

   // if still no enemy in sight, look for shootable buttons
   if (!pNewEnemy && (pBot->f_find_item_time < gpGlobals->time))
   {
      // loop through all buttons...
      while ((pent = UTIL_FindEntityByClassname (pent, "func_button")) != NULL)
      {
         // is this button visible AND shootable ?
         if (FInViewCone (vecEnd, pBot->pEdict) && BotCanSeeThis (pBot, vecEnd) && (pent->v.health > 0))
         {
            float distance = (pent->v.origin - pBot->pEdict->v.origin).Length ();

            // is this the closest shootable button ?
            if (distance < nearestdistance)
            {
               nearestdistance = distance; // update nearest distance
               pNewEnemy = pent; // bot found a shootable button !
               pBot->f_interact_time = gpGlobals->time; // save when we first saw this button
               pBot->f_aim_adjust_time = gpGlobals->time; // start adjusting aim now
               break; // don't look for anything else
            }
         }
      }
   }

   if (pNewEnemy)
   {
      // face the enemy
      Vector bot_angles = UTIL_VecToAngles (pNewEnemy->v.origin - pBot->pEdict->v.origin);
      BotSetIdealYaw (pBot, bot_angles.y);

      // set next reload time
      pBot->f_reload_time = gpGlobals->time + RANDOM_FLOAT (3.0, 5.0);
   }

   // is it time to reload ?
   if ((pBot->f_reload_time > 0) && (pBot->f_reload_time <= gpGlobals->time))
   {
      BotSwitchToBestWeapon (pBot); // switch to best gun;
      pBot->f_reload_time = -1;  // so we won't keep reloading
      if (RANDOM_LONG (1, 100) <= 80)
         pBot->pEdict->v.button |= IN_RELOAD;  // press reload button (but can forget...)
      pBot->f_shoot_time = gpGlobals->time; // reset next shoot time
   }

   return (pNewEnemy);
}


void BotSelectItem (bot_t *pBot, int item_id)
{
   // in DMC, we need to issue weapon change commands by the weaponselect field of the
   // usercmd structure, as just typing the weapon names on the console don't work...

   usercmd_t usercmd_botchangeweapon;

   if (pBot->pEdict == NULL)
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
   usercmd_botchangeweapon.weaponselect = item_id; // store the wanted weapon's ID
   usercmd_botchangeweapon.impact_index = 0;
   usercmd_botchangeweapon.impact_position = Vector (0, 0, 0);

   pBot->pEdict->v.flags |= FL_FAKECLIENT; // there's a check in CmdStart...
   CmdStart (pBot->pEdict, &usercmd_botchangeweapon, 0); // issue the select item command
   CmdEnd (pBot->pEdict); // issue end of command notifier
}


void BotSwitchToBestWeapon (bot_t *pBot)
{
   bot_weapon_select_t *pSelect = &dmc_weapon_select[0];
   int select_index = 0;
   int weapon_index;

   if (pBot->pEdict == NULL)
      return; // reliability check

   // select the best weapon to use
   while (pSelect[select_index].iId)
   {
      // is the bot NOT carrying this weapon ?
      if (!(pBot->bot_weapons & pSelect[select_index].iId))
      {
         select_index++; // skip to next weapon
         continue;
      }   

      // is the bot NOT skilled enough to use this weapon ?
      if (pBot->bot_skill < pSelect[select_index].skill_level)
      {
         select_index++; // skip to next weapon
         continue;
      }

      // is the bot underwater AND does this weapon NOT work under water ?
      if ((pBot->pEdict->v.waterlevel == 3) && !(pSelect[select_index].can_use_underwater))
      {
         select_index++; // skip to next weapon
         continue;
      }

      weapon_index = 0;
      int value = pSelect[select_index].iId;
      while (value > 0)
      {
         weapon_index++; // find out array position from the weapon value (bitshifted)
         value = value >> 1;
      }

      // see if there is enough primary ammo
      if ((weapon_defs[weapon_index].iAmmo1 == -1)
          || (pBot->m_rgAmmo[weapon_defs[weapon_index].iAmmo1] >= pSelect[select_index].min_primary_ammo))
         break; // at last, bot found the right weapon to kick your ass with
      else
         select_index++; // skip to next weapon
   }

   // is this one NOT bot's current weapon ?
   if ((pSelect[select_index].iId > 0) && (pBot->current_weapon.iId != pSelect[select_index].iId))
      BotSelectItem (pBot, weapon_index); // select this weapon
}


// specifing a weapon_choice allows you to choose the weapon the bot will
// use (assuming enough ammo exists for that weapon)

void BotFireWeapon (Vector v_enemy, bot_t *pBot, int weapon_choice)
{
   bot_weapon_select_t *pSelect = &dmc_weapon_select[0];
   bot_fire_delay_t *pDelay = &dmc_fire_delay[0];
   int select_index = 0;
   int weapon_index;
   float distance = v_enemy.Length (); // distance to enemy

   if (pBot->pEdict == NULL)
      return; // reliability check

   // select the best weapon to use
   while (pSelect[select_index].iId)
   {
      // was a weapon choice specified AND if so do they NOT match ?
      if ((weapon_choice != 0) && (weapon_choice != pSelect[select_index].iId))
      {
         select_index++; // skip to next weapon
         continue;
      }

      // is the bot NOT carrying this weapon ?
      if (!(pBot->bot_weapons & pSelect[select_index].iId))
      {
         select_index++; // skip to next weapon
         continue;
      }   

      // is the bot NOT skilled enough to use this weapon ?
      if (pBot->bot_skill < pSelect[select_index].skill_level)
      {
         select_index++; // skip to next weapon
         continue;
      }

      // is the bot underwater AND does this weapon NOT work under water ?
      if ((pBot->pEdict->v.waterlevel == 3) && !(pSelect[select_index].can_use_underwater))
      {
         select_index++; // skip to next weapon
         continue;
      }

      weapon_index = 0;
      int value = pSelect[select_index].iId;
      while (value > 0)
      {
         weapon_index++; // find out array position from the weapon value (bitshifted)
         value = value >> 1;
      }

      // see if there is enough primary ammo
      // AND the bot is far enough away to use primary fire
      // AND the bot is close enough to the enemy to use primary fire
      if (((weapon_defs[weapon_index].iAmmo1 == -1)
           || (pBot->m_rgAmmo[weapon_defs[weapon_index].iAmmo1] >= pSelect[select_index].min_primary_ammo))
          && (distance >= pSelect[select_index].primary_min_distance)
          && (distance <= pSelect[select_index].primary_max_distance))
         break; // at last, bot found the right weapon to kick your ass with
      else
         select_index++; // skip to next weapon
   }

   // if bot can't decide which weapon to choose, cycle again but don't check for distances
   if (pSelect[select_index].iId == 0)
   {
      select_index = 0; // reset select_index to the start of the array

      while (pSelect[select_index].iId)
      {
         // is the bot NOT carrying this weapon ?
         if (!(pBot->bot_weapons & pSelect[select_index].iId))
         {
            select_index++; // skip to next weapon
            continue;
         }   

         // is the bot underwater AND does this weapon NOT work under water ?
         if ((pBot->pEdict->v.waterlevel == 3) && !(pSelect[select_index].can_use_underwater))
         {
            select_index++; // skip to next weapon
            continue;
         }

         weapon_index = 0;
         int value = pSelect[select_index].iId;
         while (value > 0)
         {
            weapon_index++; // find out array position from the weapon value (bitshifted)
            value = value >> 1;
         }

         // see if there is enough primary ammo
         if ((weapon_defs[weapon_index].iAmmo1 == -1)
             || (pBot->m_rgAmmo[weapon_defs[weapon_index].iAmmo2Max] >= pSelect[select_index].min_primary_ammo))
            break; // at last, bot found the right weapon to kick your ass with
         else
            select_index++; // skip to next weapon
      }
   }

   // is this one NOT bot's current weapon ?
   // is this one NOT bot's current weapon ?
   if ((pSelect[select_index].iId > 0) && (pBot->current_weapon.iId != pSelect[select_index].iId))
      BotSelectItem (pBot, weapon_index); // select this weapon

   // is bot just 'standing' on enemy ?
   if ((pBot->pEdict->v.origin.z - pBot->pBotEnemy->v.origin.z > 30)
       && ((pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length () < 90))
      pBot->BotMove.b_emergency_walkback = TRUE; // walk back to get down

   // is it the rocket launcher ?
   if (pSelect[select_index].iId == DMC_WEAPON_ROCKETLAUNCHER)
   {
      // is it unsafe to fire a rocket here ?
      TraceResult tr;

      UTIL_MakeVectors (pBot->pEdict->v.v_angle); // build base vectors in bot's view angle

      // check at gun level to take rocket blast radius in account
      UTIL_TraceHull (GetGunPosition (pBot->pEdict),
                      GetGunPosition (pBot->pEdict) + gpGlobals->v_forward * 150,
                      ignore_monsters, head_hull, pBot->pEdict->v.pContainingEntity, &tr);
      if (tr.flFraction < 1.0)
         return; // if hit something, then it is unsafe to fire here
   }

   // if using grenadelauncher, quickly adjust to correct pitch
   if ((pSelect[select_index].iId == DMC_WEAPON_GRENADELAUNCHER) && (distance < 1000))
   {
      float offset_angle = -sqrt (1000 - distance) + 45; // 1000 is the longest distance
      pBot->pEdict->v.idealpitch += offset_angle; // add the offset to v.idealpitch
      BotChangePitch (pBot, offset_angle); // turn towards idealpitch in 1 frame
   }

   // if NOT in the case where it is a proximity weapon and bot is far from his enemy...
   if (!((pSelect[select_index].iId == DMC_WEAPON_AXE)
         && ((pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length () > 50))
        || (RANDOM_LONG (1, 100) < 2))
      pBot->pEdict->v.button |= IN_ATTACK; // press the FIRE button

   // should we hold button down to fire ?
   if (pSelect[select_index].primary_fire_hold)
      pBot->f_shoot_time = gpGlobals->time; // set next frame to keep pressing the button

   // else set next time to shoot
   else
   {
      float base_delay = pDelay[select_index].primary_base_delay;
      float min_delay = pDelay[select_index].primary_min_delay[pBot->bot_skill - 1];
      float max_delay = pDelay[select_index].primary_max_delay[pBot->bot_skill - 1];
      
      pBot->f_shoot_time = gpGlobals->time + base_delay + RANDOM_FLOAT (min_delay, max_delay);
   }
}


void BotShootAtEnemy (bot_t *pBot)
{
   float f_distance;
   Vector v_enemy, v_target_angle;
   TraceResult tr;

   if ((pBot->pEdict == NULL) || (pBot->pBotEnemy == NULL))
      return; // reliability check

   // target some part of our enemy's body
   v_enemy = BotGetIdealAimVector (pBot, pBot->pBotEnemy) - GetGunPosition (pBot->pEdict);

   // add imprecision offset resulting from bot & player movement (based on bot's skill)
   v_enemy = v_enemy + (((pBot->pBotEnemy->v.velocity - pBot->pEdict->v.velocity) / (pBot->bot_skill * 2)) * RANDOM_FLOAT (0, 1));

   // move the aim cursor
   BotPointGun (pBot, UTIL_VecToAngles (v_enemy));

   f_distance = v_enemy.Length (); // how far away is the enemy scum ?

   // decide which behaviour the bot will have while firing
   if (f_distance > 200)
   {
      // if not fearful and won't fall OR proximity weapon, run to enemy
      if ((!pBot->b_is_fearful && !BotCanFallForward (pBot, &tr))
          || (pBot->current_weapon.iId == DMC_WEAPON_AXE))
      {
         pBot->BotMove.f_forward_time = gpGlobals->time + 60.0; // if not fearful and no sniper...
         pBot->BotMove.b_is_walking = FALSE; // ...run if enemy is far
      }

      // else keep the distance
      else
         pBot->BotMove.f_forward_time = 0; // try to stay at a distant range from our enemy

      // if not strafing for [max pause time] seconds, pick up a strafe direction
      if ((pBot->BotMove.f_strafeleft_time + pause_time[pBot->bot_skill - 1][1] < gpGlobals->time)
          && (pBot->BotMove.f_straferight_time + pause_time[pBot->bot_skill - 1][1] < gpGlobals->time)
          && (pBot->bot_skill > 2))
      {
         if (RANDOM_LONG (1, 100) < 50)
            pBot->BotMove.f_strafeleft_time = gpGlobals->time + RANDOM_FLOAT (0.5, 2.0);
         else
            pBot->BotMove.f_straferight_time = gpGlobals->time + RANDOM_FLOAT (0.5, 2.0);
      }

      // else if already strafing and not jumped for 0.7 s (and bot skilled enough), randomly jump
      else if ((pBot->BotMove.f_strafe_speed != 0) && (pBot->BotMove.f_jump_time + 0.7 < gpGlobals->time) && (pBot->bot_skill > 1))
         pBot->BotMove.f_jump_time = gpGlobals->time + RANDOM_FLOAT (0.1, (6.0 - pBot->bot_skill) / 4); //jump

      // else if fearful and not currently strafing
      else if ((pBot->b_is_fearful) && (pBot->BotMove.f_strafeleft_time + 0.6 < gpGlobals->time)
               && (pBot->BotMove.f_straferight_time + 0.6 < gpGlobals->time))
      {
         if ((pBot->f_see_enemy_time + 5.0 < gpGlobals->time)
             || ((pBot->f_see_enemy_time + 0.5 < gpGlobals->time)
                 && (pBot->current_weapon.iId == DMC_WEAPON_AXE)))
            pBot->b_is_fearful = FALSE; // prevent bot from never attacking
      }

      // if bot should move and is not actually moving, try to unstuck it
      if (((pBot->BotMove.f_move_speed != 0) || (pBot->BotMove.f_strafe_speed != 0)) && BotIsStuck (pBot))
         BotUnstuck (pBot);
   }

   else if (f_distance > 20)
   {
      pBot->BotMove.f_forward_time = gpGlobals->time + 60.0; // walk if distance is closer
      pBot->BotMove.b_is_walking = TRUE;
   }

   else
      pBot->BotMove.f_forward_time = 0; // don't move if close enough

   // is it time to shoot yet?
   if (pBot->f_shoot_time <= gpGlobals->time)
      BotFireWeapon (v_enemy, pBot, 0); // select the best weapon at this distance and fire
}


void BotShootButton (bot_t *pBot)
{
   Vector v_button;

   if ((pBot->pEdict == NULL) || (pBot->pBotEnemy == NULL))
      return; // reliability check

   v_button = VecBModelOrigin (pBot->pBotEnemy) - GetGunPosition (pBot->pEdict);

   // move the aim cursor
   BotPointGun (pBot, UTIL_VecToAngles (v_button));

   // aim at the button and fire the shotgun
   BotFireWeapon (v_button, pBot, DMC_WEAPON_QUAKEGUN);
}
