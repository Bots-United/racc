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
// bot_start.cpp
//

#include "racc.h"

extern debug_level_t DebugLevel;


void BotStartGame (bot_t *pBot)
{
   char command[32];

   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   // handle Counter-Strike team selection stuff here...
   if (pBot->BotEyes.BotHUD.menu_state == MENU_CS_TEAMSELECT_MAINMENU)
   {
      pBot->BotEyes.BotHUD.menu_state = MENU_CS_IDLE; // switch back to idle

      // select the team the bot wishes to join...
      if ((pBot->pProfile->team < 1) || (pBot->pProfile->team > 2))
      {
         // none specified, is bot forced to choose a team ?
         if ((server.bot_forced_team[0] == 'T') || (server.bot_forced_team[0] == 't'))
            pBot->pProfile->team = 1; // join the terrorist team
         else if ((server.bot_forced_team[0] == 'C') || (server.bot_forced_team[0] == 'c'))
            pBot->pProfile->team = 2; // join the counter-terrorist team

         if ((pBot->pProfile->team < 1) || (pBot->pProfile->team > 2))
            pBot->pProfile->team = 5; // else use auto team assignment
      }

      sprintf (command, "menuselect %d", pBot->pProfile->team);
      FakeClientCommand (pBot->pEdict, command);
      return;
   }

   else if ((pBot->BotEyes.BotHUD.menu_state == MENU_CS_TEAMSELECT_COUNTERMENU)
            || (pBot->BotEyes.BotHUD.menu_state == MENU_CS_TEAMSELECT_TERRMENU))
   {
      pBot->BotEyes.BotHUD.menu_state = MENU_CS_IDLE; // switch back to idle

      // select the class the bot wishes to use...
      if ((pBot->pProfile->subclass < 1) || (pBot->pProfile->subclass > 4))
         pBot->pProfile->subclass = 5;

      sprintf (command, "menuselect %d", pBot->pProfile->subclass);
      FakeClientCommand (pBot->pEdict, command);
      pBot->b_not_started = FALSE; // bot has now joined the game
      return;
   }
}
