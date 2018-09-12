// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// CSTRIKE version
//
// bot_start.cpp
//

#include "racc.h"


void BotStartGame (player_t *pPlayer)
{
   hud_t *bot_hud;
   profile_t *bot_profile;

   if (!IsValidPlayer (pPlayer))
      return; // reliability check

   bot_hud = &pPlayer->Bot.BotEyes.BotHUD; // quick access to bot HUD
   bot_profile = pPlayer->Bot.pProfile; // quick access to bot profile

   // handle Counter-Strike team selection stuff here...
   if (bot_hud->menu_state == MENU_CSTRIKE_TEAMSELECT_MAINMENU)
   {
      bot_hud->menu_state = MENU_CSTRIKE_IDLE; // switch back to idle

      // select the team the bot wishes to join...
      if ((bot_profile->team < 1) || (bot_profile->team > 2))
      {
         // none specified, is bot forced to choose a team ?
         if ((server.bot_forced_team[0] == 'T') || (server.bot_forced_team[0] == 't'))
            bot_profile->team = 1; // join the terrorist team
         else if ((server.bot_forced_team[0] == 'C') || (server.bot_forced_team[0] == 'c'))
            bot_profile->team = 2; // join the counter-terrorist team

         if ((bot_profile->team < 1) || (bot_profile->team > 2))
            bot_profile->team = 5; // else use auto team assignment
      }

      FakeClientCommand (pPlayer->pEntity, "menuselect %d", bot_profile->team);
      return;
   }

   else if ((bot_hud->menu_state == MENU_CSTRIKE_TEAMSELECT_COUNTERMENU)
            || (bot_hud->menu_state == MENU_CSTRIKE_TEAMSELECT_TERRMENU))
   {
      bot_hud->menu_state = MENU_CSTRIKE_IDLE; // switch back to idle

      // select the class the bot wishes to use...
      if ((bot_profile->subclass < 1) || (bot_profile->subclass > 4))
         bot_profile->subclass = 5;

      FakeClientCommand (pPlayer->pEntity, "menuselect %d", bot_profile->subclass);
      pPlayer->Bot.not_started = FALSE; // bot has now joined the game
      return;
   }
}


void BotBuyStuff (player_t *pPlayer)
{
   // this function is a state machine

   static int *weapon_rates = NULL;
   char weapon_index;
   char best_index;

   // first check and see if the weapon_rates has been sized correctly
   if (weapon_rates == NULL)
      weapon_rates = (int *) malloc (weapon_count * sizeof (int));

   // have we NOT succeeded mallocating it ?
   if (weapon_rates == NULL)
      TerminateOnError ("BotBuyStuff(): malloc() failure for weapon rates array on %d bytes (out of memory ?)\n", weapon_count * sizeof (int));

   // alright, zero the array out
   memset (weapon_rates, 0, weapon_count * sizeof (int));

   // is bot a VIP ?
   if (strcmp ("vip", pPlayer->model) == 0)
   {
      pPlayer->Bot.buy_time = 0; // don't buy anything if bot is VIP (VIPs don't buy stuff)
      return;
   }

   // pause the bot for a while
   pPlayer->Bot.pause_time = server.time + 1.2;

   // state 0: if armor is damaged and bot has some money, buy some armor
   if (pPlayer->Bot.buy_state == 0)
   {
      if ((pPlayer->pEntity->v.armorvalue < 80) && (pPlayer->money > 1000))
      {
         // if bot is rich, buy kevlar + helmet, else buy a single kevlar
         if (pPlayer->money > 2500)
            FakeClientCommand (pPlayer->pEntity, BUY_KEVLARHELMET);
         else
            FakeClientCommand (pPlayer->pEntity, BUY_KEVLAR);

         pPlayer->Bot.buy_time = server.time + RandomFloat (0.3, 0.5); // delay next buy
      }
      pPlayer->Bot.buy_state++;
      return;
   }

   // state 1: buy a weapon
   else if (pPlayer->Bot.buy_state == 1)
   {
      // cycle through all weapons and rate them...
      for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
         weapon_rates[weapon_index] = BotRateWeapon (pPlayer, &weapons[weapon_index]);

      // now find the best one which is affordable (but always keep 800 euro in reserve)...
      best_index = 0;
      for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
         if ((weapon_rates[weapon_index] > weapon_rates[best_index])
             && (weapons[weapon_index].price < pPlayer->money - 800))
            best_index = weapon_index; // remember the best index

      // can the bot afford this weapon ?
      if (weapons[best_index].price < pPlayer->money - 800)
      {
         FakeClientCommand (pPlayer->pEntity, weapons[best_index].buy_command); // just buy it
         AIConsole_printf (CHANNEL_COGNITION, 3, "BUYING %s!!\n", weapons[best_index].classname);
      }

      pPlayer->Bot.buy_time = server.time + RandomFloat (0.3, 0.5); // delay next buy
      pPlayer->Bot.buy_state++;
      return;
   }

   // states 2, 3, 4: if bot has still some money, buy more ammo
   else if ((pPlayer->Bot.buy_state == 2) || (pPlayer->Bot.buy_state == 3) || (pPlayer->Bot.buy_state == 4))
   {
      if (pPlayer->money > 300)
      {
         if (PlayerHasWeaponOfClass (pPlayer, WEAPON_CLASS_PRIMARY))
            FakeClientCommand (pPlayer->pEntity, BUY_PRIMARYAMMO); // buy some primary ammo
         else if (PlayerHasWeaponOfClass (pPlayer, WEAPON_CLASS_SECONDARY))
            FakeClientCommand (pPlayer->pEntity, BUY_SECONDARYAMMO); // buy some secondary ammo
         pPlayer->Bot.buy_time = server.time + 0.3; // delay next buy
      }

      pPlayer->Bot.buy_state++;
      return;
   }

   // state 5: buy another weapon
   else if (pPlayer->Bot.buy_state == 5)
   {
      // cycle through all weapons and rate them...
      for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
         weapon_rates[weapon_index] = BotRateWeapon (pPlayer, &weapons[weapon_index]);

      // now find the best one which is affordable (but always keep 800 euro in reserve)...
      best_index = 0;
      for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
         if ((weapon_rates[weapon_index] > weapon_rates[best_index])
             && (weapons[weapon_index].price < pPlayer->money - 800))
            best_index = weapon_index; // remember the best index

      // can the bot afford this weapon ?
      if (weapons[best_index].price < pPlayer->money - 800)
      {
         FakeClientCommand (pPlayer->pEntity, weapons[best_index].buy_command); // just buy it
         AIConsole_printf (CHANNEL_COGNITION, 3, "BUYING %s!!\n", weapons[best_index].classname);
      }

      pPlayer->Bot.buy_time = server.time + RandomFloat (0.3, 0.5); // delay next buy
      pPlayer->Bot.buy_state++;
      return;
   }

   // states 6 and 7: if bot has still some money, buy more ammo
   else if ((pPlayer->Bot.buy_state == 6) || (pPlayer->Bot.buy_state == 7))
   {
      if (pPlayer->money > 300)
      {
         if (PlayerHasWeaponOfClass (pPlayer, WEAPON_CLASS_PRIMARY))
            FakeClientCommand (pPlayer->pEntity, BUY_PRIMARYAMMO); // buy some primary ammo
         else if (PlayerHasWeaponOfClass (pPlayer, WEAPON_CLASS_SECONDARY))
            FakeClientCommand (pPlayer->pEntity, BUY_SECONDARYAMMO); // buy some secondary ammo
         pPlayer->Bot.buy_time = server.time + 0.3; // delay next buy
      }

      pPlayer->Bot.buy_state++;
      return;
   }

   // state 8: if bot has still some money, buy yet another weapon
   else if (pPlayer->Bot.buy_state == 8)
   {
      // cycle through all weapons and rate them...
      for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
         weapon_rates[weapon_index] = BotRateWeapon (pPlayer, &weapons[weapon_index]);

      // now find the best one which is affordable (but always keep 800 euro in reserve)...
      best_index = 0;
      for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
         if ((weapon_rates[weapon_index] > weapon_rates[best_index])
             && (weapons[weapon_index].price < pPlayer->money - 800))
            best_index = weapon_index; // remember the best index

      // can the bot afford this weapon ?
      if (weapons[best_index].price < pPlayer->money - 800)
      {
         FakeClientCommand (pPlayer->pEntity, weapons[best_index].buy_command); // just buy it
         AIConsole_printf (CHANNEL_COGNITION, 3, "BUYING %s!!\n", weapons[best_index].classname);
      }

      pPlayer->Bot.buy_time = server.time + RandomFloat (0.3, 0.5); // delay next buy
      pPlayer->Bot.buy_state++;
      return;
   }

   // state 9: if bot is counter-terrorist and we're on a bomb map, randomly buy the defuse kit
   else if (pPlayer->Bot.buy_state == 9)
   {
      if ((GetTeam (pPlayer) == CSTRIKE_COUNTER_TERRORIST) && (mission.bomb != BOMB_NONE)
          && (pPlayer->Bot.BotEyes.BotHUD.icons_state[HUD_ICON_DEFUSER] == HUD_ICON_OFF)
          && (pPlayer->money > 200) && RandomLong (1, 100) < 33)
      {
         FakeClientCommand (pPlayer->pEntity, BUY_DEFUSEKIT); // to buy the defuse kit
         pPlayer->Bot.buy_time = server.time + RandomFloat (0.3, 0.5); // delay next buy
      }

      pPlayer->Bot.buy_state++;
      return;
   }

   // state 10: switch to best weapon
   else if (pPlayer->Bot.buy_state == 10)
   {
      BotSwitchToBestWeapon (pPlayer);
      pPlayer->Bot.buy_time = 0; // don't buy anything more after that
      pPlayer->Bot.buy_state = 0; // reset buy state

      if (RandomLong (1, 100) <= (56 - 2 * player_count))
         pPlayer->Bot.BotChat.bot_sayaudio = BOT_SAYAUDIO_FIRSTSPAWN; // bot says 'go go go' or something like that
      else if (RandomLong (1, 100) < 50)
      {
         if (RandomLong (1, 100) < 34)
            FakeClientCommand (pPlayer->pEntity, RADIOMSG_STICKTOGETHER);
         else if (RandomLong (1, 100) < 50)
            FakeClientCommand (pPlayer->pEntity, RADIOMSG_GOGOGO);
         else
            FakeClientCommand (pPlayer->pEntity, RADIOMSG_STORMTHEFRONT);
      }

      return;
   }
}
