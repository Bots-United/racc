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
// ASHEEP version
//
// bot_combat.cpp
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "bot_common.h"
#include "bot_specific.h"

extern bot_weapon_t weapon_defs[MAX_WEAPONS];
extern bool b_observer_mode;
extern bool is_team_play;
extern float pause_time[5][2];


typedef struct
{
   int   iId; // the weapon ID value
   char  weapon_name[64]; // name of the weapon when selecting it
   int   skill_level; // bot skill must be less than or equal to this value
   float primary_min_distance; // 0 = no minimum
   float primary_max_distance; // 9999 = no maximum
   float secondary_min_distance; // 0 = no minimum
   float secondary_max_distance; // 9999 = no maximum
   bool  can_use_underwater; // can use this weapon underwater
   int   primary_fire_percent; // times out of 100 to use primary fire
   int   min_primary_ammo; // minimum ammout of primary ammo needed to fire
   int   min_secondary_ammo; // minimum ammout of seconday ammo needed to fire
   bool  primary_fire_hold; // hold down primary fire button to use?
   bool  secondary_fire_hold; // hold down secondary fire button to use?
   bool  primary_fire_charge; // charge weapon using primary fire?
   bool  secondary_fire_charge; // charge weapon using secondary fire?
   float primary_charge_delay; // time to charge weapon
   float secondary_charge_delay; // time to charge weapon
} bot_weapon_select_t;

typedef struct
{
   int iId;
   float primary_base_delay;
   float primary_min_delay[5];
   float primary_max_delay[5];
   float secondary_base_delay;
   float secondary_min_delay[5];
   float secondary_max_delay[5];
} bot_fire_delay_t;


// weapons are stored in priority order, most desired weapon should be at
// the start of the array and least desired should be at the end

bot_weapon_select_t asheep_weapon_select[] = {
   {ASHEEP_WEAPON_M41A, "weapon_9mmm41a", 1, 0.0, 750.0, 300.0, 2000.0,
    FALSE, 90, 10, 1, TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {ASHEEP_WEAPON_MP5, "weapon_9mmAR", 1, 0.0, 750.0, 300.0, 2000.0,
    FALSE, 90, 10, 1, TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {ASHEEP_WEAPON_RPG, "weapon_rpg", 1, 150.0, 9999.0, 0.0, 0.0,
    TRUE, 80, 1, 1, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {ASHEEP_WEAPON_SHOTGUN, "weapon_shotgun", 1, 0.0, 150.0, 30.0, 150.0,
    FALSE, 30, 1, 2, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {ASHEEP_WEAPON_BERETTA, "weapon_beretta", 1, 0.0, 700.0, 0.0, 0.0,
    FALSE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {ASHEEP_WEAPON_PYTHON, "weapon_357", 1, 0.0, 700.0, 0.0, 0.0,
    FALSE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {ASHEEP_WEAPON_IRONBAR, "weapon_crowbar", 1, 0.0, 40.0, 0.0, 0.0,
    TRUE, 100, 0, 0, TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {ASHEEP_WEAPON_POOLSTICK, "weapon_poolstick", 1, 0.0, 40.0, 0.0, 0.0,
    TRUE, 100, 0, 0, TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {ASHEEP_WEAPON_CROSSBOW, "weapon_crossbow", 1, 100.0, 5000.0, 0.0, 0.0,
    TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {ASHEEP_WEAPON_HANDGRENADE, "weapon_handgrenade", 1, 250.0, 1000.0, 0.0, 0.0,
    TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {ASHEEP_WEAPON_SNARK, "weapon_snark", 1, 150.0, 500.0, 0.0, 0.0,
    FALSE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {ASHEEP_WEAPON_TOAD, "weapon_toad", 1, 150.0, 500.0, 0.0, 0.0,
    FALSE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {ASHEEP_WEAPON_EGON, "weapon_egon", 1, 0.0, 9999.0, 0.0, 0.0,
    FALSE, 100, 1, 0, TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {ASHEEP_WEAPON_GAUSS, "weapon_gauss", 1, 0.0, 9999.0, 0.0, 9999.0,
    FALSE, 70, 1, 10, FALSE, FALSE, FALSE, TRUE, 0.0, 1.4},
   {ASHEEP_WEAPON_HORNETGUN, "weapon_hornetgun", 1, 30.0, 1000.0, 30.0, 1000.0,
    TRUE, 50, 1, 4, FALSE, TRUE, FALSE, FALSE, 0.0, 0.0},
   {ASHEEP_WEAPON_GLOCK, "weapon_9mmhandgun", 1, 0.0, 1200.0, 0.0, 1200.0,
    TRUE, 20, 1, 2, FALSE, TRUE, FALSE, FALSE, 0.0, 0.0},
   /* terminator */
   {0, "", 0, 0.0, 0.0, 0.0, 0.0, TRUE, 0, 1, 1, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0}
};



// weapon firing delay based on skill (min and max delay for each weapon)
// THESE MUST MATCH THE SAME ORDER AS THE WEAPON SELECT ARRAY!!!

bot_fire_delay_t asheep_fire_delay[] = {
   {ASHEEP_WEAPON_M41A,
    0.1, {0.5, 0.4, 0.25, 0.1, 0.0}, {0.8, 0.65, 0.45, 0.3, 0.1},
    0.75, {1.4, 1.0, 0.7, 0.4, 0.0}, {2.0, 1.6, 1.0, 0.7, 0.3}},
   {ASHEEP_WEAPON_MP5,
    0.1, {0.5, 0.4, 0.2, 0.1, 0.0}, {0.8, 0.6, 0.4, 0.3, 0.1},
    1.0, {1.4, 1.0, 0.7, 0.4, 0.0}, {2.0, 1.6, 1.0, 0.7, 0.3}},
   {ASHEEP_WEAPON_RPG,
    0.7, {0.7, 0.5, 0.4, 0.3, 0.2}, {1.5, 1.0, 0.9, 0.7, 0.5},
    0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}},
   {ASHEEP_WEAPON_SHOTGUN,
    0.5, {0.4, 0.3, 0.2, 0.1, 0.0}, {1.5, 1.2, 0.8, 0.5, 0.2},
    1.0, {0.4, 0.3, 0.2, 0.1, 0.0}, {1.5, 1.2, 0.8, 0.5, 0.2}},
   {ASHEEP_WEAPON_BERETTA,
    0.3, {1.0, 0.6, 0.4, 0.2, 0.0}, {1.5, 1.0, 0.7, 0.4, 0.1},
    0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}},
   {ASHEEP_WEAPON_PYTHON,
    0.6, {1.5, 1.0, 0.4, 0.2, 0.0}, {2.2, 1.3, 0.8, 0.5, 0.2},
    0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}},
   {ASHEEP_WEAPON_IRONBAR,
    0.3, {0.6, 0.4, 0.3, 0.2, 0.0}, {1.0, 0.7, 0.5, 0.3, 0.1},
    0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}},
   {ASHEEP_WEAPON_POOLSTICK,
    0.3, {0.6, 0.4, 0.3, 0.2, 0.0}, {1.0, 0.7, 0.5, 0.3, 0.1},
    0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}},
   {ASHEEP_WEAPON_CROSSBOW,
    0.6, {1.0, 0.8, 0.5, 0.2, 0.0}, {1.3, 1.0, 0.7, 0.4, 0.25},
    0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}},
   {ASHEEP_WEAPON_HANDGRENADE,
    0.1, {2.0, 1.5, 1.0, 0.7, 0.5}, {3.0, 2.3, 1.7, 1.2, 1.0},
    0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}},
   {ASHEEP_WEAPON_SNARK,
    0.1, {0.6, 0.4, 0.2, 0.1, 0.0}, {1.0, 0.7, 0.5, 0.2, 0.1},
    0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}},
   {ASHEEP_WEAPON_TOAD,
    0.1, {0.6, 0.4, 0.2, 0.1, 0.0}, {1.0, 0.7, 0.5, 0.2, 0.1},
    0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}},
   {ASHEEP_WEAPON_EGON,
    0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0},
    0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}},
   {ASHEEP_WEAPON_GAUSS,
    0.2, {1.0, 0.5, 0.3, 0.2, 0.0}, {1.2, 0.8, 0.5, 0.3, 0.1},
    1.0, {1.2, 0.8, 0.5, 0.3, 0.2}, {2.0, 1.5, 1.0, 0.7, 0.5}},
   {ASHEEP_WEAPON_HORNETGUN,
    0.3, {1.0, 0.6, 0.4, 0.2, 0.0}, {1.5, 1.0, 0.7, 0.4, 0.1},
    0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}},
   {ASHEEP_WEAPON_GLOCK,
    0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1},
    0.2, {0.2, 0.1, 0.1, 0.0, 0.0}, {0.4, 0.2, 0.2, 0.1, 0.1}},
   /* terminator */
   {0, 0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0},
       0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}}
};




edict_t *BotCheckForEnemies (bot_t *pBot)
{
   Vector vecEnd;
   edict_t *pNewEnemy = NULL;
   float nearestdistance = 2500;
   int i;

   if (pBot->pEdict == NULL)
      return NULL; // reliability check

   // does the bot already have an enemy?
   if (pBot->pBotEnemy != NULL)
   {
      // is the enemy dead ? assume bot killed it
      if (!IsAlive (pBot->pBotEnemy))
      {
         // was this enemy a player or a bot (not a squark nor a toad) ?
         if ((pBot->pBotEnemy->v.flags & FL_CLIENT) || (pBot->pBotEnemy->v.flags & FL_FAKECLIENT))
         {
            // sometimes laugh at him
            if (RANDOM_LONG (1,100) <= (56 - 2 * gpGlobals->maxClients))
            {
               pBot->BotChat.b_saytext_kill = TRUE; // bot laughs
               pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (3.0, 5.0);
            }

            // if this enemy was close to us, spray a logo
            if (((pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length () < 200) && (RANDOM_LONG (1, 100) < 33))
               pBot->f_spraying_logo_time = gpGlobals->time + RANDOM_FLOAT (1.0, 2.0);

            pBot->pVictimEntity = pBot->pBotEnemy; // bot remembers his victim
         }

         pBot->b_enemy_hidden = FALSE; // have no enemy anymore
         pBot->pBotEnemy = NULL; // nulls out enemy pointer
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

   // search the world for players...
   for (i = 1; i <= gpGlobals->maxClients; i++)
   {
      edict_t *pPlayer = INDEXENT(i);

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
            if (((pPlayer->v.button & IN_ATTACK) == IN_ATTACK)
                || ((pPlayer->v.button & IN_ATTACK2) == IN_ATTACK2))
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
      pBot->f_reload_time = -1; // so we won't keep reloading
      if (RANDOM_LONG (1, 100) <= 80)
         pBot->pEdict->v.button |= IN_RELOAD; // press reload button (but can forget...)
      pBot->f_shoot_time = gpGlobals->time; // reset next shoot time
   }

   return (pNewEnemy);
}


void BotSelectItem (bot_t *pBot, char *item_name)
{
   if (pBot->pEdict == NULL)
      return; // reliability check

   // if bot is selecting a sniper gun, 2 out of 3 times switch to deadly sniper mode
   if (((strcmp (item_name, "weapon_crossbow") == 0)
        || (strcmp (item_name, "weapon_357") == 0))
       && (RANDOM_LONG (1, 100) < 66))
   {
      pBot->f_togglesniper_time = gpGlobals->time + 0.45; // toggle sniper mode in 0.45 second
      pBot->f_shoot_time = gpGlobals->time + 0.50; // don't shoot in the meantime
   }

   FakeClientCommand (pBot->pEdict, item_name); // issue the select item command
}


void BotSwitchToBestWeapon (bot_t *pBot)
{
   bot_weapon_select_t *pSelect = &asheep_weapon_select[0];
   int select_index = 0;
   bool primary_ammo_left, use_primary, use_secondary;

   if (pBot->pEdict == NULL)
      return; // reliability check

   // select the best weapon to use
   while (pSelect[select_index].iId)
   {
      // reset weapon usable fire state
      primary_ammo_left = FALSE;
      use_primary = FALSE;
      use_secondary = FALSE;

      // is the bot NOT carrying this weapon ?
      if (!(pBot->bot_weapons & (1 << pSelect[select_index].iId)))
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

      // is the bot camping AND does this weapon NOT fit for the job ?
      if ((pBot->f_camp_time > gpGlobals->time)
          && ((pSelect[select_index].iId == ASHEEP_WEAPON_IRONBAR)
              || (pSelect[select_index].iId == ASHEEP_WEAPON_GLOCK)
              || (pSelect[select_index].iId == ASHEEP_WEAPON_SHOTGUN)
              || (pSelect[select_index].iId == ASHEEP_WEAPON_HORNETGUN)
              || (pSelect[select_index].iId == ASHEEP_WEAPON_HANDGRENADE)
              || (pSelect[select_index].iId == ASHEEP_WEAPON_TRIPMINE)
              || (pSelect[select_index].iId == ASHEEP_WEAPON_SATCHEL)
              || (pSelect[select_index].iId == ASHEEP_WEAPON_SNARK)
              || (pSelect[select_index].iId == ASHEEP_WEAPON_TOAD)
              || (pSelect[select_index].iId == ASHEEP_WEAPON_POOLSTICK)))
      {
         select_index++; // skip to next weapon
         continue;
      }

      // is the bot already holding this weapon and there is still ammo in clip ?
      if ((pSelect[select_index].iId == pBot->current_weapon.iId)
          && ((pSelect[select_index].min_primary_ammo <= 0) || (pBot->current_weapon.iClip < 0)
              || (pBot->current_weapon.iClip >= pSelect[select_index].min_primary_ammo)))
         primary_ammo_left = TRUE;

      // does this weapon have clips left ?
      if (((weapon_defs[pSelect[select_index].iId].iAmmo1 == -1)
           || (pBot->m_rgAmmo[weapon_defs[pSelect[select_index].iId].iAmmo1]
               >= pSelect[select_index].min_primary_ammo)))
         primary_ammo_left = TRUE;

      // see if there is enough primary ammo
      if (primary_ammo_left)
         use_primary = TRUE;

      // see if there is enough secondary ammo OR no secondary ammo required
      if (((weapon_defs[pSelect[select_index].iId].iAmmo2 == -1)
           || (pBot->m_rgAmmo[weapon_defs[pSelect[select_index].iId].iAmmo2]
               >= pSelect[select_index].min_secondary_ammo)))
         use_secondary = TRUE;

      // see if there is enough ammo to fire the weapon...
      if (use_primary || use_secondary)
         break; // at last, bot found the right weapon to kick your ass with
      else
      {
         select_index++; // skip to next weapon
         continue;
      }
   }

   // is this one NOT bot's current weapon ?
   if ((pSelect[select_index].iId > 0) && (pBot->current_weapon.iId != pSelect[select_index].iId))
      BotSelectItem (pBot, pSelect[select_index].weapon_name); // select this weapon
}


// specifing a weapon_choice allows you to choose the weapon the bot will
// use (assuming enough ammo exists for that weapon)

void BotFireWeapon (Vector v_enemy, bot_t *pBot, int weapon_choice)
{
   bot_weapon_select_t *pSelect = &asheep_weapon_select[0];
   bot_fire_delay_t *pDelay = &asheep_fire_delay[0];
   int select_index = 0;
   bool primary_ammo_left, use_primary, use_secondary;
   int primary_percent = RANDOM_LONG (1, 100);
   float distance = v_enemy.Length (); // distance to enemy

   if (pBot->pEdict == NULL)
      return; // reliability check

   // are we charging the primary fire ?
   if (pBot->f_primary_charging > 0)
   {
      // is it time to fire the charged weapon ?
      if (pBot->f_primary_charging <= gpGlobals->time)
      {
         pBot->f_primary_charging = -1; // stop charging it (-1 means not charging)

         // identify the weapon we were charging
         while ((pSelect[select_index].iId) && (pSelect[select_index].iId != pBot->charging_weapon_id))
            select_index++;

         // set next time to shoot, as next frame will automatically release fire button
         float base_delay = pDelay[select_index].primary_base_delay;
         float min_delay = pDelay[select_index].primary_min_delay[pBot->bot_skill - 1];
         float max_delay = pDelay[select_index].primary_max_delay[pBot->bot_skill - 1];

         pBot->f_shoot_time = gpGlobals->time + base_delay + RANDOM_FLOAT (min_delay, max_delay);
         return;
      }
      else
      {
         pBot->pEdict->v.button |= IN_ATTACK; // press the FIRE button
         pBot->f_shoot_time = gpGlobals->time; // set next frame to keep pressing the button
         return;
      }
   }

   // are we charging the secondary fire ?
   if (pBot->f_secondary_charging > 0)
   {
      // is it time to fire the charging weapon ?
      if (pBot->f_secondary_charging <= gpGlobals->time)
      {
         pBot->f_secondary_charging = -1; // stop charging it (-1 means not charging)

         // identify the weapon we were charging
         while ((pSelect[select_index].iId) && (pSelect[select_index].iId != pBot->charging_weapon_id))
            select_index++;

         // set next time to shoot, as next frame will automatically release fire button
         float base_delay = pDelay[select_index].secondary_base_delay;
         float min_delay = pDelay[select_index].secondary_min_delay[pBot->bot_skill - 1];
         float max_delay = pDelay[select_index].secondary_max_delay[pBot->bot_skill - 1];

         pBot->f_shoot_time = gpGlobals->time + base_delay + RANDOM_FLOAT (min_delay, max_delay);
         return;
      }
      else
      {
         pBot->pEdict->v.button |= IN_ATTACK2; // press the FIRE2 button
         pBot->f_shoot_time = gpGlobals->time; // set next frame to keep pressing the button
         return;
      }
   }

   // we are not already charging a weapon, so select the best one to use
   while (pSelect[select_index].iId)
   {
      // reset weapon usable fire state
      primary_ammo_left = FALSE;
      use_primary = FALSE;
      use_secondary = FALSE;

      // was a weapon choice specified AND if so do they NOT match ?
      if ((weapon_choice != 0) && (weapon_choice != pSelect[select_index].iId))
      {
         select_index++; // skip to next weapon
         continue;
      }

      // is the bot NOT carrying this weapon ?
      if (!(pBot->bot_weapons & (1 << pSelect[select_index].iId)))
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

      // is the bot already holding this weapon and there is still ammo in clip ?
      if ((pSelect[select_index].iId == pBot->current_weapon.iId)
          && ((pSelect[select_index].min_primary_ammo <= 0) || (pBot->current_weapon.iClip < 0)
              || (pBot->current_weapon.iClip >= pSelect[select_index].min_primary_ammo)))
         primary_ammo_left = TRUE;

      // does this weapon have clips left ?
      if (((weapon_defs[pSelect[select_index].iId].iAmmo1 == -1)
           || (pBot->m_rgAmmo[weapon_defs[pSelect[select_index].iId].iAmmo1]
               >= pSelect[select_index].min_primary_ammo)))
         primary_ammo_left = TRUE;

      // see if there is enough primary ammo
      // AND the bot is far enough away to use primary fire
      // AND the bot is close enough to the enemy to use primary fire
      if (primary_ammo_left
          && (distance >= pSelect[select_index].primary_min_distance)
          && (distance <= pSelect[select_index].primary_max_distance))
         use_primary = TRUE;

      // see if there is enough secondary ammo
      // AND the bot is far enough away to use secondary fire
      // AND the bot is close enough to the enemy to use secondary fire
      if (((weapon_defs[pSelect[select_index].iId].iAmmo2 == -1)
           || (pBot->m_rgAmmo[weapon_defs[pSelect[select_index].iId].iAmmo2]
               >= pSelect[select_index].min_secondary_ammo))
          && (distance >= pSelect[select_index].secondary_min_distance)
          && (distance <= pSelect[select_index].secondary_max_distance))
         use_secondary = TRUE;

      // see if the bot can use both primary and secondary fire
      if (use_primary && use_secondary)
      {
         if (primary_percent <= pSelect[select_index].primary_fire_percent)
            use_secondary = FALSE; // choose first of both from use percents
         else
            use_primary = FALSE; // choose second of both from use percents
      }

      // see if there is enough ammo to fire the weapon...
      if (use_primary || use_secondary)
         break; // at last, bot found the right weapon to kick your ass with
      else
      {
         select_index++; // skip to next weapon
         continue;
      }
   }

   // if bot can't decide which weapon to choose, cycle again but don't check for distances
   if (pSelect[select_index].iId == 0)
   {
      select_index = 0; // reset select_index to the start of the array

      while (pSelect[select_index].iId)
      {
         // reset weapon usable fire state
         primary_ammo_left = FALSE;
         use_primary = FALSE;
         use_secondary = FALSE;

         // is the bot NOT carrying this weapon ?
         if (!(pBot->bot_weapons & (1 << pSelect[select_index].iId)))
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

         // is the bot already holding this weapon and there is still ammo in clip ?
         if ((pSelect[select_index].iId == pBot->current_weapon.iId)
             && ((pSelect[select_index].min_primary_ammo <= 0) || (pBot->current_weapon.iClip < 0)
                 || (pBot->current_weapon.iClip >= pSelect[select_index].min_primary_ammo)))
            primary_ammo_left = TRUE;

         // does this weapon have clips left ?
         if (((weapon_defs[pSelect[select_index].iId].iAmmo1 == -1)
              || (pBot->m_rgAmmo[weapon_defs[pSelect[select_index].iId].iAmmo1]
                  >= pSelect[select_index].min_primary_ammo)))
            primary_ammo_left = TRUE;

         // see if there is enough primary ammo
         if (primary_ammo_left)
            use_primary = TRUE;

         // see if there is enough secondary ammo
         if (((weapon_defs[pSelect[select_index].iId].iAmmo2 == -1)
              || (pBot->m_rgAmmo[weapon_defs[pSelect[select_index].iId].iAmmo2]
                  >= pSelect[select_index].min_secondary_ammo)))
            use_secondary = TRUE;

         // see if the bot can use both primary and secondary fire
         if (use_primary && use_secondary)
         {
            if (primary_percent <= pSelect[select_index].primary_fire_percent)
               use_secondary = FALSE; // choose first of both from use percents
            else
               use_primary = FALSE; // choose second of both from use percents
         }

         // see if there is enough ammo to fire the weapon...
         if (use_primary || use_secondary)
            break; // at last, bot found the right weapon to kick your ass with
         else
         {
            select_index++; // skip to next weapon
            continue;
         }
      }
   }

   // is this one NOT bot's current weapon ?
   if ((pSelect[select_index].iId > 0) && (pBot->current_weapon.iId != pSelect[select_index].iId))
      BotSelectItem (pBot, pSelect[select_index].weapon_name); // select this weapon

   // is bot just 'standing' on enemy ?
   if ((pBot->pEdict->v.origin.z - pBot->pBotEnemy->v.origin.z > 30)
       && ((pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length () < 90))
      pBot->BotMove.f_duck_time = gpGlobals->time + 0.1; // duck to hit him

   // is it the crossbow or the magnum ?
   if ((pSelect[select_index].iId == ASHEEP_WEAPON_CROSSBOW)
       || (pSelect[select_index].iId == ASHEEP_WEAPON_PYTHON))
   {
      pBot->BotMove.f_forward_time = 0; // don't move while sniping

      // are we moving too fast to use it ?
      if (pBot->pEdict->v.velocity.Length () > 50)
         return;  // don't press attack key until velocity is < 50

      UTIL_MakeVectors (pBot->pEdict->v.v_angle); // build base vectors in aim direction

      // have we aimed with insufficient accuracy to fire ?
      if (UTIL_AngleOfVectors (pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin, gpGlobals->v_forward)
          > (300 / (pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length ()))
         return; // don't shoot yet, it would be a monumental error...
   }

   // is it the RPG ?
   if (pSelect[select_index].iId == ASHEEP_WEAPON_RPG)
   {
      // is it unsafe to fire a rocket here ?
      TraceResult tr;

      UTIL_MakeVectors (pBot->pEdict->v.v_angle); // build base vectors in bot's view angle

      // check at head level to take rocket initial uplift deviation in account
      UTIL_TraceHull (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs,
                      pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs + gpGlobals->v_forward * 150,
                      ignore_monsters, head_hull, pBot->pEdict->v.pContainingEntity, &tr);
      if (tr.flFraction < 1.0)
         return; // if hit something, then it is unsafe to fire here

      // check at gun level to take rocket initial right deviation in account
      UTIL_TraceLine (GetGunPosition (pBot->pEdict),
                      GetGunPosition (pBot->pEdict) + gpGlobals->v_forward * 75 + gpGlobals->v_right * 32,
                      ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);
      if (tr.flFraction < 1.0)
         return; // if hit something, then it is unsafe to fire here
   }

   // should we use the primary attack ?
   if (use_primary)
   {
      // if using hand grenade, quickly adjust to correct pitch
      if ((pSelect[select_index].iId == ASHEEP_WEAPON_HANDGRENADE) && (distance < 1000))
      {
         float offset_angle = -sqrt (1000 - distance) + 45; // 1000 is the longest distance
         pBot->pEdict->v.idealpitch += offset_angle; // add the offset to v.idealpitch
         BotChangePitch (pBot, offset_angle); // turn towards idealpitch in 1 frame
      }

      // if NOT in the case where it is a proximity weapon and bot is far from his enemy...
      if (!(((pSelect[select_index].iId == ASHEEP_WEAPON_IRONBAR)
             || (pSelect[select_index].iId == ASHEEP_WEAPON_POOLSTICK))
            && ((pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length () > 50))
           || (RANDOM_LONG (1, 100) < 2))
         pBot->pEdict->v.button |= IN_ATTACK; // press the FIRE button

      // should we charge the primary fire ?
      if (pSelect[select_index].primary_fire_charge)
      {
         pBot->charging_weapon_id = pSelect[select_index].iId; // save charging weapon id
         pBot->f_primary_charging = gpGlobals->time + pSelect[select_index].primary_charge_delay;
         pBot->f_shoot_time = gpGlobals->time; // set next frame to keep pressing the button
      }

      // else this is a normal shoot
      else
      {
         // should we hold button down to fire ?
         if (pSelect[select_index].primary_fire_hold)
         {
            // if no need for ammo OR no need for clip OR still enough ammo in clip...
            if ((pSelect[select_index].min_primary_ammo <= 0) || (pBot->current_weapon.iClip < 0)
                || (pBot->current_weapon.iClip >= pSelect[select_index].min_primary_ammo))
               pBot->f_shoot_time = gpGlobals->time; // set next frame to keep pressing the button
            else
            {
               pBot->f_shoot_time = gpGlobals->time + 0.25; // delay a while and reload
               pBot->pEdict->v.button = IN_RELOAD; // press the RELOAD button
            }
         }

         // else set next time to shoot
         else
         {
            float base_delay = pDelay[select_index].primary_base_delay;
            float min_delay = pDelay[select_index].primary_min_delay[pBot->bot_skill - 1];
            float max_delay = pDelay[select_index].primary_max_delay[pBot->bot_skill - 1];

            pBot->f_shoot_time = gpGlobals->time + base_delay + RANDOM_FLOAT (min_delay, max_delay);
         }
      }
   }

   // else we MUST be using the secondary attack
   else
   {
      // if using hand grenade, quickly adjust to correct pitch
      if ((pSelect[select_index].iId == ASHEEP_WEAPON_MP5) && (distance < 2000))
      {
         float offset_angle = -sqrt (2000 - distance) + 45; // 2000 is the longest distance
         pBot->pEdict->v.idealpitch += offset_angle; // add the offset to v.idealpitch
         BotChangePitch (pBot, offset_angle); // turn towards idealpitch in 1 frame
      }

      // if NOT in the case where it is a proximity weapon and bot is far from his enemy...
      if (!(((pSelect[select_index].iId == ASHEEP_WEAPON_IRONBAR)
             || (pSelect[select_index].iId == ASHEEP_WEAPON_POOLSTICK))
            && ((pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length () > 50))
           || (RANDOM_LONG (1, 100) < 2))
         pBot->pEdict->v.button |= IN_ATTACK2; // press the FIRE2 button

      // should we charge the secondary fire ?
      if (pSelect[select_index].secondary_fire_charge)
      {
         pBot->charging_weapon_id = pSelect[select_index].iId; // save charging weapon id
         pBot->f_secondary_charging = gpGlobals->time + pSelect[select_index].secondary_charge_delay;
         pBot->f_shoot_time = gpGlobals->time; // set next frame to keep pressing the button
      }

      // else this is a normal shoot
      else
      {
         // should we hold button down to fire ?
         if (pSelect[select_index].secondary_fire_hold)
            pBot->f_shoot_time = gpGlobals->time; // set next frame to keep pressing the button

         // else set next time to shoot
         else
         {
            float base_delay = pDelay[select_index].secondary_base_delay;
            float min_delay = pDelay[select_index].secondary_min_delay[pBot->bot_skill - 1];
            float max_delay = pDelay[select_index].secondary_max_delay[pBot->bot_skill - 1];

            pBot->f_shoot_time = gpGlobals->time + base_delay + RANDOM_FLOAT (min_delay, max_delay);
         }
      }
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
      bool has_sniper_rifle = ((pBot->current_weapon.iId == ASHEEP_WEAPON_CROSSBOW)
                               || (pBot->current_weapon.iId == ASHEEP_WEAPON_PYTHON));

      // if not fearful, no sniper gun and won't fall OR proximity weapon, run to enemy
      if ((!pBot->b_is_fearful && !has_sniper_rifle && !BotCanFallForward (pBot, &tr))
          || (pBot->current_weapon.iId == ASHEEP_WEAPON_IRONBAR)
          || (pBot->current_weapon.iId == ASHEEP_WEAPON_POOLSTICK))
      {
         pBot->BotMove.f_forward_time = gpGlobals->time + 60.0; // if not fearful and no sniper...
         pBot->BotMove.b_is_walking = FALSE; // ...run if enemy is far
      }

      // else if not fearful OR quite far from enemy, keep the distance
      else if ((!pBot->b_is_fearful) || (f_distance > 500))
         pBot->BotMove.f_forward_time = 0; // try to stay at a distant range from our enemy

      // else bot may be fearful, or enemy too close, so step back
      else
         pBot->BotMove.f_backwards_time = gpGlobals->time + 0.5; // make some steps back

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

      // else if bot is already strafing AND bot has not jumped for 0.7 s
      // AND bot hasn't some sort of sniper gun AND bot is skilled enough, randomly jump
      else if ((pBot->BotMove.f_strafe_speed != 0) && (pBot->BotMove.f_jump_time + 0.7 < gpGlobals->time)
               && (!has_sniper_rifle) && (pBot->bot_skill > 2))
         pBot->BotMove.f_jump_time = gpGlobals->time + RANDOM_FLOAT (0.1, (6.0 - pBot->bot_skill) / 4); //jump

      // else if fearful and not currently strafing, duck
      else if (pBot->b_is_fearful && (pBot->BotMove.f_strafeleft_time + 0.6 < gpGlobals->time)
               && (pBot->BotMove.f_straferight_time + 0.6 < gpGlobals->time))
      {
         pBot->BotMove.f_duck_time = gpGlobals->time + 0.8; // duck and fire
         if ((pBot->f_see_enemy_time + 5.0 < gpGlobals->time)
             || ((pBot->f_see_enemy_time + 0.5 < gpGlobals->time)
                 && ((pBot->current_weapon.iId == ASHEEP_WEAPON_IRONBAR)
                     || (pBot->current_weapon.iId == ASHEEP_WEAPON_POOLSTICK))))
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

      // if bot is fearful and not currently strafing AND enemy is not ducking yet, duck
      if (pBot->b_is_fearful && (pBot->BotMove.f_strafeleft_time + 0.6 < gpGlobals->time)
          && (pBot->BotMove.f_straferight_time + 0.6 < gpGlobals->time)
          && (pBot->pBotEnemy->v.flDuckTime <= 0))
      {
         pBot->BotMove.f_duck_time = gpGlobals->time + 0.8; // duck and fire
         if ((pBot->f_see_enemy_time + 5.0 < gpGlobals->time)
             || ((pBot->f_see_enemy_time + 0.5 < gpGlobals->time)
                 && ((pBot->current_weapon.iId == ASHEEP_WEAPON_IRONBAR)
                     || (pBot->current_weapon.iId == ASHEEP_WEAPON_POOLSTICK))))
            pBot->b_is_fearful = FALSE; // prevent bot from never attacking
      }
   }

   else
      pBot->BotMove.f_forward_time = 0; // don't move if close enough

   // is it time to shoot yet ?
   if (pBot->f_shoot_time <= gpGlobals->time)
      BotFireWeapon (v_enemy, pBot, 0); // select the best weapon at this distance and fire

   // else is it time to toggle sniper mode ?
   else if ((pBot->f_togglesniper_time > 0) && (pBot->f_togglesniper_time < gpGlobals->time))
   {
      pBot->pEdict->v.button |= IN_ATTACK2; // press the secondary fire button to toggle mode
      pBot->f_togglesniper_time = 0; // reset toggle sniper mode time
   }
}
