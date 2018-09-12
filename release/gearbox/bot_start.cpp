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
// GEARBOX version
//
// bot_start.cpp
//

#include "racc.h"


void BotStartGame (bot_t *pBot)
{
   char command[32];

   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   // is it Opposing Force Capture The Flag ?
   if (is_opfor_ctf)
   {
      // handle Opposing Force CTF stuff here...
      if (pBot->BotEyes.BotHUD.menu_state == MENU_GEARBOX_TEAMSELECT_TEAMMENU)
      {
         pBot->BotEyes.BotHUD.menu_state = MENU_GEARBOX_IDLE; // switch back to idle

         // select the team the bot wishes to join...
         if ((pBot->pProfile->team < 1) || (pBot->pProfile->team > 2))
         {
            // none specified, is bot forced to choose a team ?
            if ((server.bot_forced_team[0] == 'B') || (server.bot_forced_team[0] == 'b')
                || (server.bot_forced_team[0] == 'S') || (server.bot_forced_team[0] == 's'))
               pBot->pProfile->team = 1; // join the Black Mesa (scientists) team
            else if ((server.bot_forced_team[0] == 'G') || (server.bot_forced_team[0] == 'g')
                     || (server.bot_forced_team[0] == 'M') || (server.bot_forced_team[0] == 'm'))
               pBot->pProfile->team = 2; // join the Grunts (military) team

            if ((pBot->pProfile->team < 1) || (pBot->pProfile->team > 2))
               pBot->pProfile->team = 3; // else use auto team assignment
         }

         sprintf (command, "jointeam %d", pBot->pProfile->team);
         FakeClientCommand (pBot->pEdict, command);
         return;
      }

      else if (pBot->BotEyes.BotHUD.menu_state == MENU_GEARBOX_TEAMSELECT_CLASSMENU)
      {
         pBot->BotEyes.BotHUD.menu_state = MENU_GEARBOX_IDLE; // switch back to idle

         // select the class the bot wishes to use...
         if ((pBot->pProfile->subclass < 0) || (pBot->pProfile->subclass > 6))
            pBot->pProfile->subclass = 7;

         sprintf (command, "selectchar %d", pBot->pProfile->subclass);
         FakeClientCommand (pBot->pEdict, command);
         pBot->b_not_started = FALSE; // bot has now joined the game (doesn't need to be started)
         return;
      }
   }

   // else if not CTF, don't need to do anything to start game...
   else
      pBot->b_not_started = FALSE; // bot has already joined the game
}

