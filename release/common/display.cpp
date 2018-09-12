// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// display.cpp
//

#include "racc.h"


void UTIL_DrawDots (Vector start, Vector end)
{
   // this function draws a dotted line visible from the client side of the listen server client,
   // from the vector location start to the vector location end, which is supposed to last 30
   // seconds. This duration is hardcoded.

   if (!IsValidPlayer (pListenserverPlayer))
      return; // reliability check

   // send this client a packet telling him to draw a dotted line using the specified parameters
   MESSAGE_BEGIN (MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, pListenserverPlayer->pEntity);
   WRITE_BYTE (TE_SHOWLINE);
   WRITE_COORD (start.x);
   WRITE_COORD (start.y);
   WRITE_COORD (start.z);
   WRITE_COORD (end.x);
   WRITE_COORD (end.y);
   WRITE_COORD (end.z);
   MESSAGE_END ();

   return; // finished
}


void UTIL_DrawLine (Vector start, Vector end, int life, unsigned char red, unsigned char green, unsigned char blue)
{
   // this function draws a line visible from the client side of the listen server player,
   // from the vector location start to the vector location end, which is supposed to last
   // life tenths seconds, and having the color defined by RGB.

   if (!IsValidPlayer (pListenserverPlayer))
      return; // reliability check

   // send this client a packet telling him to draw a beam using the specified parameters
   MESSAGE_BEGIN (MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, pListenserverPlayer->pEntity);
   WRITE_BYTE (TE_BEAMPOINTS);
   WRITE_COORD (start.x);
   WRITE_COORD (start.y);
   WRITE_COORD (start.z);
   WRITE_COORD (end.x);
   WRITE_COORD (end.y);
   WRITE_COORD (end.z);
   WRITE_SHORT (beam_model);
   WRITE_BYTE (1); // framestart
   WRITE_BYTE (10); // framerate
   WRITE_BYTE (life); // life in 0.1's
   WRITE_BYTE (10); // width
   WRITE_BYTE (0); // noise
   WRITE_BYTE (red); // red component of RGB color
   WRITE_BYTE (green); // green component of RGB color
   WRITE_BYTE (blue); // blue component of RGB color
   WRITE_BYTE (255); // brightness
   WRITE_BYTE (255); // speed
   MESSAGE_END ();

   return; // finished
}


void UTIL_DrawBox (Vector bbmin, Vector bbmax, int life, unsigned char red, unsigned char green, unsigned char blue)
{
   // this function draws an axial box (i.e, a box whose faces are parallel to the map's axises)
   // visible from the client side of the listen server player, encompassing the bounding box
   // defined by the two vector locations bbmin and bbmax, for a duration of life tenths seconds,
   // and having the color defined by RGB.

   if (!IsValidPlayer (pListenserverPlayer))
      return; // reliability check

   // bottom square
   UTIL_DrawLine (Vector (bbmin.x, bbmin.y, bbmin.z), Vector (bbmax.x, bbmin.y, bbmin.z), life, red, green, blue);
   UTIL_DrawLine (Vector (bbmax.x, bbmin.y, bbmin.z), Vector (bbmax.x, bbmax.y, bbmin.z), life, red, green, blue);
   UTIL_DrawLine (Vector (bbmax.x, bbmax.y, bbmin.z), Vector (bbmin.x, bbmax.y, bbmin.z), life, red, green, blue);
   UTIL_DrawLine (Vector (bbmin.x, bbmax.y, bbmin.z), Vector (bbmin.x, bbmin.y, bbmin.z), life, red, green, blue);

   // verticals
   UTIL_DrawLine (Vector (bbmin.x, bbmin.y, bbmin.z), Vector (bbmin.x, bbmin.y, bbmax.z), life, red, green, blue);
   UTIL_DrawLine (Vector (bbmax.x, bbmin.y, bbmin.z), Vector (bbmax.x, bbmin.y, bbmax.z), life, red, green, blue);
   UTIL_DrawLine (Vector (bbmax.x, bbmax.y, bbmin.z), Vector (bbmax.x, bbmax.y, bbmax.z), life, red, green, blue);
   UTIL_DrawLine (Vector (bbmin.x, bbmax.y, bbmin.z), Vector (bbmin.x, bbmax.y, bbmax.z), life, red, green, blue);

   // top square
   UTIL_DrawLine (Vector (bbmin.x, bbmin.y, bbmax.z), Vector (bbmax.x, bbmin.y, bbmax.z), life, red, green, blue);
   UTIL_DrawLine (Vector (bbmax.x, bbmin.y, bbmax.z), Vector (bbmax.x, bbmax.y, bbmax.z), life, red, green, blue);
   UTIL_DrawLine (Vector (bbmax.x, bbmax.y, bbmax.z), Vector (bbmin.x, bbmax.y, bbmax.z), life, red, green, blue);
   UTIL_DrawLine (Vector (bbmin.x, bbmax.y, bbmax.z), Vector (bbmin.x, bbmin.y, bbmax.z), life, red, green, blue);

   return; // finished
}


void UTIL_DrawWalkface (walkface_t *pFace, int life, unsigned char red, unsigned char green, unsigned char blue)
{
   // this function is a higher level wrapper for UTIL_DrawLine() which purpose is to simplify
   // the computation of the boundaries of the face pointed to by face. It draws lines visible
   // from the client side of the listen server client, around the perimeter of the face pointed
   // to by pFace, which is supposed to last life 10hs seconds, having the color defined by RGB.

   int corner_index;
   Vector v_bound1, v_bound2;

   if (!IsValidPlayer (pListenserverPlayer))
      return; // reliability check

   if (pFace == NULL)
      TerminateOnError ("UTIL_DrawWalkface() called with NULL walkface\n");

   // draw the perimeter around the face
   for (corner_index = 0; corner_index < pFace->corner_count; corner_index++)
   {
      // locate the first vertice of this corner and raise it 2 units up for better visibility
      v_bound1 = pFace->v_corners[corner_index] + Vector (0, 0, 2);

      // locate the second vertice of this corner and raise it 2 units up for better visibility
      if (corner_index < pFace->corner_count - 1)
         v_bound2 = pFace->v_corners[corner_index + 1] + Vector (0, 0, 2); // next corner in the array
      else
         v_bound2 = pFace->v_corners[0] + Vector (0, 0, 2); // loop back to corner zero at last corner

      // draw a line between these 2 points
      UTIL_DrawLine (v_bound1, v_bound2, life, red, green, blue);
   }

   return; // finished
}


void UTIL_DrawNavlink (navlink_t *pLink, int life)
{
   // this function is a higher level wrapper for UTIL_DrawLine() which purpose is to simplify
   // the drawing of the navigation link pointed to by pLink, represented under the form of a
   // glowing vertical bar, and being 30 units high. This effect is only visible from the listen
   // server client, during life tenths seconds, and having a color defined by the type of
   // reachability this link has.

   unsigned char rgb[3];

   if (!IsValidPlayer (pListenserverPlayer))
      return; // reliability check

   if (pLink == NULL)
      TerminateOnError ("UTIL_DrawNavlink() called with NULL navlink\n");

   // set the navlink color according to its reachability
   if (pLink->reachability & REACHABILITY_FALLEDGE)
   {
      rgb[0] = 255; // RED color for falledge reachability
      rgb[1] = 0;
      rgb[2] = 0;
   }
   else if (pLink->reachability & REACHABILITY_LADDER)
   {
      rgb[0] = 0; // BLUE color for ladder reachability
      rgb[1] = 0;
      rgb[2] = 255;
   }
   else if (pLink->reachability & REACHABILITY_ELEVATOR)
   {
      rgb[0] = 0; // CYAN color for elevator reachability
      rgb[1] = 255;
      rgb[2] = 255;
   }
   else if (pLink->reachability & REACHABILITY_PLATFORM)
   {
      rgb[0] = 0; // GREEN color for platform reachability
      rgb[1] = 255;
      rgb[2] = 0;
   }
   else if (pLink->reachability & REACHABILITY_CONVEYOR)
   {
      rgb[0] = 255; // PURPLE color for conveyor reachability
      rgb[1] = 0;
      rgb[2] = 255;
   }
   else if (pLink->reachability & REACHABILITY_TRAIN)
   {
      rgb[0] = 0; // GREEN color for train reachability
      rgb[1] = 255;
      rgb[2] = 0;
   }
   else if (pLink->reachability & REACHABILITY_JUMP)
   {
      rgb[0] = 255; // YELLOW color for jump reachability
      rgb[1] = 255;
      rgb[2] = 0;
   }
   else
   {
      rgb[0] = 255; // WHITE color for normal reachability
      rgb[1] = 255;
      rgb[2] = 255;
   }

   // draw the navlink
   UTIL_DrawLine (pLink->v_origin + Vector (0, 0, -25), pLink->v_origin + Vector (0, 0, 25), life, rgb[0], rgb[1], rgb[2]);

   return; // finished
}


void UTIL_DrawPath (pathmachine_t *pPathmachine)
{
   // this function is a facility for UTIL_DrawLine() and UTIL_DrawNavLink() that simplifies the
   // drawing of the path contained in the pathmachine pointed to by pPathmachine, by drawing
   // each navlink in the path and linking it to its parent. The effect has a lifetime of ONE
   // video frame. IMPORTANT: only are drawn the navlinks that the spectator player pointed to
   // by pPlayer can see, the others are discarded.

   int link_index;
   navlink_t *pNavlink;
   bool previous_was_visible;

   previous_was_visible = FALSE;

   // for each navlink in the path, draw the navlink and link it to its parent
   for (link_index = 0; link_index < pPathmachine->path_count; link_index++)
   {
      pNavlink = pPathmachine->path[link_index]; // quick access to navlink

      // is this navlink visible ?
      if (IsInFieldOfView (pListenserverPlayer->pEntity, pNavlink->v_origin))
      {
         UTIL_DrawNavlink (pNavlink, 3); // draw the navlink

         // is there another navlink leading to this one ?
         if (link_index - 1 >= 0)
            UTIL_DrawLine (pPathmachine->path[link_index - 1]->v_origin, pNavlink->v_origin, 3, 0, 0, 255);

         previous_was_visible = TRUE; // remember this navlink is visible for the next one
      }

      // else was the previous visible ?
      else if (previous_was_visible)
      {
         UTIL_DrawLine (pPathmachine->path[link_index - 1]->v_origin, pNavlink->v_origin, 3, 0, 0, 255);
         previous_was_visible = FALSE;
      }
   }

   return;
}


void UTIL_DrawSector (sector_t *pSector, int life, unsigned char red, unsigned char green, unsigned char blue)
{
   // this function is a higher level wrapper for UTIL_DrawLine() which purpose is to simplify
   // the computation of the boundaries of the map topologic sector to which belongs the spatial
   // location v_origin. It draws a line visible from the client side of the listen server
   // client, around the perimeter of the sector to which v_origin belongs, supposed to last
   // life tenths seconds, and having the color defined by RGB.

   int i, j;
   float sector_left, sector_right, sector_top, sector_bottom;

   if (!IsValidPlayer (pListenserverPlayer))
      return; // reliability check

   // we have to determine the position in the topological array where pSector is located

   // loop through array, stop when bucket found (could be faster...)
   for (i = 0; i < map.parallels_count; i++)
      for (j = 0; j < map.meridians_count; j++)
         if (&map.topology[i][j] == pSector)
         {
            // now compute the left, right, top and bottom coordinates indices of the sector
            sector_left = map.v_worldmins.x + (map.v_worldmaxs.x - map.v_worldmins.x) / map.parallels_count * i;
            sector_right = map.v_worldmins.x + (map.v_worldmaxs.x - map.v_worldmins.x) / map.parallels_count * (i + 1);
            sector_bottom = map.v_worldmins.y + (map.v_worldmaxs.y - map.v_worldmins.y) / map.meridians_count * j;
            sector_top = map.v_worldmins.y + (map.v_worldmaxs.y - map.v_worldmins.y) / map.meridians_count * (j + 1);

            // and draw the perimeter around the sector (hopefully the player sees it)
            UTIL_DrawLine (Vector (sector_left, sector_top, pListenserverPlayer->v_origin.z + 1), Vector (sector_left, sector_bottom, pListenserverPlayer->v_origin.z + 1), life, red, green, blue);
            UTIL_DrawLine (Vector (sector_left, sector_bottom, pListenserverPlayer->v_origin.z + 1), Vector (sector_right, sector_bottom, pListenserverPlayer->v_origin.z + 1), life, red, green, blue);
            UTIL_DrawLine (Vector (sector_right, sector_bottom, pListenserverPlayer->v_origin.z + 1), Vector (sector_right, sector_top, pListenserverPlayer->v_origin.z + 1), life, red, green, blue);
            UTIL_DrawLine (Vector (sector_right, sector_top, pListenserverPlayer->v_origin.z + 1), Vector (sector_left, sector_top, pListenserverPlayer->v_origin.z + 1), life, red, green, blue);

            return; // finished
         }

   // sector not found in topological array ?? WTF ???
   TerminateOnError ("UTIL_DrawSector(): function called for unknown sector!\n");
}


int AIConsole_printf (char channel, char pos, const char *fmt, ...)
{
   // this function fills the display buffer of some part of the AI debug console (to be sent
   // each frame as glowing text to the client's HUD)

   va_list argptr;
   static char string[512];

   if ((pos < 0) || (pos > 5))
      return (1); // reliability check: cancel if position out of range

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   vsprintf (string, fmt, argptr);
   va_end (argptr);

   // given the wanted channel, place the debug text at the right position
   if (channel == CHANNEL_EYES)
      strcpy (DebugLevel.text_eyes[pos], string); // 1st sensitive channel (top left)
   else if (channel == CHANNEL_EARS)
      strcpy (DebugLevel.text_ears[pos], string); // 2nd sensitive channel (middle left)
   else if (channel == CHANNEL_BODY)
      strcpy (DebugLevel.text_body[pos], string); // 3rd sensitive channel (bottom left)
   else if (channel == CHANNEL_LEGS)
      strcpy (DebugLevel.text_legs[pos], string); // 1st motile channel (top right)
   else if (channel == CHANNEL_HAND)
      strcpy (DebugLevel.text_hand[pos], string); // 2nd motile channel (middle right)
   else if (channel == CHANNEL_CHAT)
      strcpy (DebugLevel.text_chat[pos], string); // 3rd motile channel (bottom right)
   else if (channel == CHANNEL_COGNITION)
      strcpy (DebugLevel.text_cognition[pos], string); // cognition channel (middle center)
   else if (channel == CHANNEL_NAVIGATION)
      strcpy (DebugLevel.text_navigation[pos], string); // cognition channel (middle center)

   return (0); // string is at the right position, waiting to be displayed
}


void DisplayAIConsole (player_t *pPlayer)
{
   // this function is in charge each frame of updating the glowing HUD text to be sent to the
   // AI debug console. We parse each channel of the console and build the equivalent string,
   // and send it for display at the right position on the screen. After that, we empty the
   // contents of the console for the next frame. The bot_name parameter is passed as the target
   // bot's name we are spectating, to build the "spectating xxx" string in the center channel.

   bot_eyes_t *pBotEyes;
   bot_ears_t *pBotEars;
   bot_body_t *pBotBody;
   bot_legs_t *pBotLegs;
   bot_hand_t *pBotHand;
   bot_chat_t *pBotChat;
   bot_brain_t *pBotBrain;
   static char string[512];
   char pos;

   if (!IsValidPlayer (pListenserverPlayer))
      return; // reliability check

   pBotEyes = &pPlayer->Bot.BotEyes; // quick access to bot eyes
   pBotEars = &pPlayer->Bot.BotEars; // quick access to bot ears
   pBotBody = &pPlayer->Bot.BotBody; // quick access to bot body
   pBotLegs = &pPlayer->Bot.BotLegs; // quick access to bot legs
   pBotHand = &pPlayer->Bot.BotHand; // quick access to bot hand
   pBotChat = &pPlayer->Bot.BotChat; // quick access to bot chat
   pBotBrain = &pPlayer->Bot.BotBrain; // quick access to bot brain

   // if eyes debug level is set, print whether, how and what the bot sees
   if (DebugLevel.eyes > 0)
   {
      AIConsole_printf (CHANNEL_EYES, 0, "%s\n",
                        (pBotEyes->blinded_time > server.time ? "BLINDED!" : "seeing the world"));
      AIConsole_printf (CHANNEL_EYES, 1, "%s\n",
                        (pBotEyes->sample_time > server.time + 0.10 ? "Sampling..." : ""));
      AIConsole_printf (CHANNEL_EYES, 2, "Facing %s\n",
                        (!FNullEnt (pBotEyes->BotFOV[BOT_FOV_WIDTH / 2].pHit) ? STRING (pBotEyes->BotFOV[BOT_FOV_WIDTH / 2].pHit->v.classname) : "nothing"));
   }

   // if ears debug level is set, print out the average noise just computed
   if (DebugLevel.ears > 0)
   {
      AIConsole_printf (CHANNEL_EARS, 0, "Noise (average): %.3f\n",
                        pBotEars->average_noise);
      AIConsole_printf (CHANNEL_EARS, 1, "LAST SOUND: %s\n",
                        pBotEars->noises[pBotEars->new_sound_index].file_path);
      AIConsole_printf (CHANNEL_EARS, 2, "Loudness: %.3f\n",
                        pBotEars->noises[pBotEars->new_sound_index].loudness);
      AIConsole_printf (CHANNEL_EARS, 3, "Coming from: %s%s%s%s\n",
                        (pBotEars->noises[pBotEars->new_sound_index].direction & DIRECTION_FRONT ? "| FRONT": ""),
                        (pBotEars->noises[pBotEars->new_sound_index].direction & DIRECTION_BACK ? "| BACK": ""),
                        (pBotEars->noises[pBotEars->new_sound_index].direction & DIRECTION_LEFT ? "| LEFT": ""),
                        (pBotEars->noises[pBotEars->new_sound_index].direction & DIRECTION_RIGHT ? "| RIGHT": ""));
   }

   // if body debug level is set, print the bot's obstacle bitmap
   if (DebugLevel.body > 0)
   {
      AIConsole_printf (CHANNEL_BODY, 0, "Senses: %s%s%s%s%s%s%s%s%s%s%s%s\n",
                        (pBotBody->hit_state & OBSTACLE_LEFT_LOWWALL ? " | L_LOWWALL" : ""),
                        (pBotBody->hit_state & OBSTACLE_LEFT_LOWCEILING ? " | L_CEILING" : ""),
                        (pBotBody->hit_state & OBSTACLE_LEFT_WALL ? " | L_WALL" : ""),
                        (pBotBody->hit_state & OBSTACLE_LEFT_FALL ? " | L_FALL" : ""),
                        (pBotBody->hit_state & OBSTACLE_FRONT_LOWWALL ? " | F_LOWWALL" : ""),
                        (pBotBody->hit_state & OBSTACLE_FRONT_LOWCEILING ? " | F_CEILING" : ""),
                        (pBotBody->hit_state & OBSTACLE_FRONT_WALL ? " | F_WALL" : ""),
                        (pBotBody->hit_state & OBSTACLE_FRONT_FALL ? " | F_FALL" : ""),
                        (pBotBody->hit_state & OBSTACLE_RIGHT_LOWWALL ? " | R_LOWWALL" : ""),
                        (pBotBody->hit_state & OBSTACLE_RIGHT_LOWCEILING ? " | R_CEILING" : ""),
                        (pBotBody->hit_state & OBSTACLE_RIGHT_WALL ? " | R_WALL" : ""),
                        (pBotBody->hit_state & OBSTACLE_RIGHT_FALL ? " | R_FALL" : ""));
      AIConsole_printf (CHANNEL_BODY, 1, "%s\n",
                        (pPlayer->Bot.is_stuck ? "BOT IS STUCK!!!" : "not stuck"));
   }

   // if legs debug level is set, print the bot's keys
   if (DebugLevel.legs > 0)
   {
      AIConsole_printf (CHANNEL_LEGS, 0, "Keys: %s%s%s%s%s%s%s%s%s%s%s%s\n",
                        (pPlayer->input_buttons & INPUT_KEY_FORWARD ? " | FORWARD" : ""),
                        (pPlayer->input_buttons & INPUT_KEY_BACKWARDS ? " | BACK" : ""),
                        (pPlayer->input_buttons & INPUT_KEY_STRAFELEFT ? " | STRAFE_L" : ""),
                        (pPlayer->input_buttons & INPUT_KEY_STRAFERIGHT ? " | STRAFE_R" : ""),
                        (pPlayer->input_buttons & INPUT_KEY_JUMP ? " | JUMP" : ""),
                        (pPlayer->input_buttons & INPUT_KEY_DUCK ? " | DUCK" : ""),
                        (pPlayer->input_buttons & INPUT_KEY_WALK ? " | WALK" : ""),
                        (pPlayer->input_buttons & INPUT_KEY_USE ? " | USE" : ""),
                        (pPlayer->input_buttons & INPUT_KEY_RELOAD ? " | RELOAD" : ""),
                        (pPlayer->input_buttons & INPUT_KEY_PRONE ? " | PRONE" : ""),
                        (pPlayer->input_buttons & INPUT_KEY_FIRE1 ? " | FIRE" : ""),
                        (pPlayer->input_buttons & INPUT_KEY_FIRE2 ? " | FIRE2" : ""));
   }

   // if hand debug level is set, print the bot's angles
   if (DebugLevel.hand > 0)
   {
      AIConsole_printf (CHANNEL_HAND, 0, "v_angle (%.2f, %.2f)\n",
                        pPlayer->v_angle.x,
                        pPlayer->v_angle.y);
      AIConsole_printf (CHANNEL_HAND, 1, "ideal angles (%.2f, %.2f)\n",
                        pBotHand->ideal_angles.x,
                        pBotHand->ideal_angles.y);
      AIConsole_printf (CHANNEL_HAND, 2, "aim speed (%.2f, %.2f)\n",
                        pBotHand->turn_speed.x,
                        pBotHand->turn_speed.y);
   }

   // if chat debug level is set, print the bot's intents about communication
   if (DebugLevel.chat > 0)
   {
      AIConsole_printf (CHANNEL_CHAT, 0, "%s%s%s\n",
                        (pBotChat->saytext_time > server.time ? " | TYPING TEXT" : ""),
                        (pBotChat->sayaudio_time > server.time ? " | ABOUT TO SPEAK" : ""),
                        ((pBotChat->sayaudio_time < server.time) && (pBotChat->saytext_time < server.time) ? "Nothing to say" : ""));
      AIConsole_printf (CHANNEL_CHAT, 1, "Context: %s%s%s%s%s%s%s%s%s%s%s%s\n",
                        (pBotChat->bot_saytext == BOT_SAYTEXT_HELLO ? "SAYTEXT_HELLO" : ""),
                        (pBotChat->bot_saytext == BOT_SAYTEXT_BYE ? "SAYTEXT_BYE" : ""),
                        (pBotChat->bot_saytext == BOT_SAYTEXT_LAUGH ? "SAYTEXT_LAUGH" : ""),
                        (pBotChat->bot_saytext == BOT_SAYTEXT_WHINE ? "SAYTEXT_WHINE" : ""),
                        (pBotChat->bot_saytext == BOT_SAYTEXT_ALONE ? "SAYTEXT_ALONE" : ""),
                        (pBotChat->bot_saytext == BOT_SAYTEXT_NEEDBACKUP ? "SAYTEXT_NEEDBACKUP" : ""),
                        (pBotChat->bot_saytext == BOT_SAYTEXT_AFFIRMATIVE ? "SAYTEXT_AFFIRMATIVE" : ""),
                        (pBotChat->bot_saytext == BOT_SAYTEXT_NEGATIVE ? "SAYTEXT_NEGATIVE" : ""),
                        (pBotChat->bot_saytext == BOT_SAYTEXT_FOLLOWOK ? "SAYTEXT_FOLLOWOK" : ""),
                        (pBotChat->bot_saytext == BOT_SAYTEXT_STOPOK ? "SAYTEXT_STOPOK" : ""),
                        (pBotChat->bot_saytext == BOT_SAYTEXT_HOLDPOSITIONOK ? "SAYTEXT_HOLDPOSITIONOK" : ""),
                        (pBotChat->bot_saytext == BOT_SAYTEXT_CANTFOLLOW ? "SAYTEXT_CANTFOLLOW" : ""));
      AIConsole_printf (CHANNEL_CHAT, 2, "Audio: %s%s%s%s%s%s%s%s%s%s%s%s\n",
                        (pBotChat->bot_saytext == BOT_SAYAUDIO_AFFIRMATIVE ? "AFFIRMATIVE" : ""),
                        (pBotChat->bot_saytext == BOT_SAYAUDIO_ALERT ? "ALERT" : ""),
                        (pBotChat->bot_saytext == BOT_SAYAUDIO_ATTACKING ? "ATTACKING" : ""),
                        (pBotChat->bot_saytext == BOT_SAYAUDIO_FIRSTSPAWN ? "FIRSTSPAWN" : ""),
                        (pBotChat->bot_saytext == BOT_SAYAUDIO_INPOSITION ? "INPOSITION" : ""),
                        (pBotChat->bot_saytext == BOT_SAYAUDIO_NEGATIVE ? "NEGATIVE" : ""),
                        (pBotChat->bot_saytext == BOT_SAYAUDIO_REPORT ? "REPORT" : ""),
                        (pBotChat->bot_saytext == BOT_SAYAUDIO_REPORTINGIN ? "REPORTINGIN" : ""),
                        (pBotChat->bot_saytext == BOT_SAYAUDIO_SEEGRENADE ? "SEEGRENADE" : ""),
                        (pBotChat->bot_saytext == BOT_SAYAUDIO_TAKINGDAMAGE ? "TAKINGDAMAGE" : ""),
                        (pBotChat->bot_saytext == BOT_SAYAUDIO_THROWGRENADE ? "THROWGRENADE" : ""),
                        (pBotChat->bot_saytext == BOT_SAYAUDIO_VICTORY ? "VICTORY" : ""));
      AIConsole_printf (CHANNEL_CHAT, 3, "%s\n",
                        pBotChat->sayaudio_message);
      AIConsole_printf (CHANNEL_CHAT, 4, "%s\n",
                        pBotChat->saytext_message);
   }

   // if cognition debug level is set, print the bot's goals and task
   if (DebugLevel.cognition > 0)
   {
      AIConsole_printf (CHANNEL_COGNITION, 0, "Goals: %s%s%s%s%s%s%s\n",
                        (pBotBrain->bot_goal & BOT_GOAL_PICKBOMB ? "| PICKBOMB" : ""),
                        (pBotBrain->bot_goal & BOT_GOAL_PLANTBOMB ? "| PLANTBOMB" : ""),
                        (pBotBrain->bot_goal & BOT_GOAL_DEFUSEBOMB ? "| DEFUSEBOMB" : ""),
                        (pBotBrain->bot_goal & BOT_GOAL_PROTECTSITE ? "| PROTECTSITE" : ""),
                        (pBotBrain->bot_goal & BOT_GOAL_REACHSAFETYZONE ? "| REACHSAFETYZONE" : ""),
                        (pBotBrain->bot_goal & BOT_GOAL_ASSASSINATEVIP ? "| ASSASSINATEVIP" : ""),
                        (pBotBrain->bot_goal & BOT_GOAL_RESCUEHOSTAGE ? "| RESCUEHOSTAGE" : ""));
      AIConsole_printf (CHANNEL_COGNITION, 1, "Task: %s%s%s%s%s%s%s%s%s%s\n",
                        (pBotBrain->bot_task == BOT_TASK_IDLE ? "IDLE" : ""),
                        (pBotBrain->bot_task == BOT_TASK_WANDER ? "WANDER" : ""),
                        (pBotBrain->bot_task == BOT_TASK_FINDPATH ? "FINDPATH" : ""),
                        (pBotBrain->bot_task == BOT_TASK_WALKPATH ? "WALKPATH" : ""),
                        (pBotBrain->bot_task == BOT_TASK_PICKBOMB ? "PICKBOMB" : ""),
                        (pBotBrain->bot_task == BOT_TASK_PLANTBOMB ? "PLANTBOMB" : ""),
                        (pBotBrain->bot_task == BOT_TASK_DEFUSEBOMB ? "DEFUSEBOMB" : ""),
                        (pBotBrain->bot_task == BOT_TASK_USECHARGER ? "USECHARGER" : ""),
                        (pBotBrain->bot_task == BOT_TASK_CAMP ? "CAMP" : ""),
                        (pBotBrain->bot_task == BOT_TASK_AVOIDEXPLOSION ? "AVOIDEXPLOSION" : ""));
   }

   // build left panel (sensitive) and send it to the top left corner of the screen
   strcpy (string, "\n\n"); // first reset the channel string

   // eyes
   strcat (string, "\n[EYES]\n"); // print the title
   for (pos = 0; pos < 6; pos++)
   {
      if (DebugLevel.eyes > 0)
      {
         strcat (string, DebugLevel.text_eyes[pos]); // build the string line per line
         if (DebugLevel.text_eyes[pos][strlen (DebugLevel.text_eyes[pos]) - 1] != '\n')
            strcat (string, "\n"); // append an EOL to each line which has none
      }
      else
         strcat (string, "\n");
   }

   // ears
   strcat (string, "\n[EARS]\n"); // print the title
   for (pos = 0; pos < 6; pos++)
   {
      if (DebugLevel.ears > 0)
      {
         strcat (string, DebugLevel.text_ears[pos]); // build the string line per line
         if (DebugLevel.text_ears[pos][strlen (DebugLevel.text_ears[pos]) - 1] != '\n')
            strcat (string, "\n"); // append an EOL to each line which has none
      }
      else
         strcat (string, "\n");
   }

   // body
   strcat (string, "\n[BODY]\n"); // print the title
   for (pos = 0; pos < 6; pos++)
   {
      if (DebugLevel.body > 0)
      {
         strcat (string, DebugLevel.text_body[pos]); // build the string line per line
         if (DebugLevel.text_body[pos][strlen (DebugLevel.text_body[pos]) - 1] != '\n')
            strcat (string, "\n"); // append an EOL to each line which has none
      }
      else
         strcat (string, "\n");
   }

   string[511] = 0; // terminate the string at the 512th position (max message length)
   DisplayHUDText (0, 0, 0, 255, 0, 0, string);

   // build center panel (cognition/navigation) and send it to the top middle of the screen
   sprintf (string, "\n\n\n"
                    "AI console\n"
                    "spectating %s\n"
                    "\n\n\n\n", pPlayer->connection_name); // first prepare the channel string

   // cognition
   strcat (string, "\n[COGNITION]\n"); // print the title
   for (pos = 0; pos < 6; pos++)
   {
      if (DebugLevel.cognition > 0)
      {
         strcat (string, DebugLevel.text_cognition[pos]); // build the string line per line
         if (DebugLevel.text_cognition[pos][strlen (DebugLevel.text_cognition[pos]) - 1] != '\n')
            strcat (string, "\n"); // append an EOL to each line which has none
      }
      else
         strcat (string, "\n");
   }

   // navigation
   strcat (string, "\n[NAVIGATION]\n"); // print the title
   for (pos = 0; pos < 6; pos++)
   {
      if (DebugLevel.navigation > 0)
      {
         strcat (string, DebugLevel.text_navigation[pos]); // build the string line per line
         if (DebugLevel.text_navigation[pos][strlen (DebugLevel.text_navigation[pos]) - 1] != '\n')
            strcat (string, "\n"); // append an EOL to each line which has none
      }
      else
         strcat (string, "\n");
   }

   string[511] = 0; // terminate the string at the 512th position (max message length)
   DisplayHUDText (1, -1, 0, 255, 0, 0, string);

   // build right panel (motile) and send it to the top right corner of the screen
   strcpy (string, "\n\n"); // first reset the channel string

   // legs
   strcat (string, "\n[LEGS]\n"); // print the title
   for (pos = 0; pos < 6; pos++)
   {
      if (DebugLevel.legs > 0)
      {
         strcat (string, DebugLevel.text_legs[pos]); // build the string line per line
         if (DebugLevel.text_legs[pos][strlen (DebugLevel.text_legs[pos]) - 1] != '\n')
            strcat (string, "\n"); // append an EOL to each line which has none
      }
      else
         strcat (string, "\n");
   }

   // hand
   strcat (string, "\n[HAND]\n"); // print the title
   for (pos = 0; pos < 6; pos++)
   {
      if (DebugLevel.hand > 0)
      {
         strcat (string, DebugLevel.text_hand[pos]); // build the string line per line
         if (DebugLevel.text_hand[pos][strlen (DebugLevel.text_hand[pos]) - 1] != '\n')
            strcat (string, "\n"); // append an EOL to each line which has none
      }
      else
         strcat (string, "\n");
   }

   // chat
   strcat (string, "\n[CHAT]\n"); // print the title
   for (pos = 0; pos < 6; pos++)
   {
      if (DebugLevel.chat > 0)
      {
         strcat (string, DebugLevel.text_chat[pos]); // build the string line per line
         if (DebugLevel.text_chat[pos][strlen (DebugLevel.text_chat[pos]) - 1] != '\n')
            strcat (string, "\n"); // append an EOL to each line which has none
      }
      else
         strcat (string, "\n");
   }

   string[511] = 0; // terminate the string at the 512th position (max message length)
   DisplayHUDText (2, 1, 0, 255, 0, 0, string);

   // now we can reset all the AI console text fields
   memset (DebugLevel.text_eyes, 0, sizeof (DebugLevel.text_eyes));
   memset (DebugLevel.text_ears, 0, sizeof (DebugLevel.text_ears));
   memset (DebugLevel.text_body, 0, sizeof (DebugLevel.text_body));
   memset (DebugLevel.text_legs, 0, sizeof (DebugLevel.text_legs));
   memset (DebugLevel.text_hand, 0, sizeof (DebugLevel.text_hand));
   memset (DebugLevel.text_chat, 0, sizeof (DebugLevel.text_chat));
   memset (DebugLevel.text_cognition, 0, sizeof (DebugLevel.text_cognition));
   memset (DebugLevel.text_navigation, 0, sizeof (DebugLevel.text_navigation));

   return; // done
}


void DisplayHUDText (char channel, float x, float y, unsigned char r, unsigned char g, unsigned char b, const char *string)
{
   // higher level wrapper for hudtextparms TE_TEXTMESSAGEs. This function is meant to be called
   // every frame, since the duration of the display is roughly worth the duration of a video
   // frame. The X and Y coordinates are unary fractions which are bound to this rule:
   // 0: top of the screen (Y) or left of the screen (X), left aligned text
   // 1: bottom of the screen (Y) or right of the screen (X), right aligned text
   // -1(only one negative value possible): center of the screen (X and Y), centered text
   // Any value ranging from 0 to 1 will represent a valid position on the screen.

   static short duration;

   if (!IsValidPlayer (pListenserverPlayer))
      return; // reliability check

   duration = (int) server.msecval * 256 / 750; // compute text message duration
   if (duration < 5)
      duration = 5;

   MESSAGE_BEGIN (MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, pListenserverPlayer->pEntity);
   WRITE_BYTE (TE_TEXTMESSAGE);
   WRITE_BYTE (channel); // channel
   WRITE_SHORT ((int) (x * 8192)); // x coordinates * 8192
   WRITE_SHORT ((int) (y * 8192)); // y coordinates * 8192
   WRITE_BYTE (0); // effect (fade in/out)
   WRITE_BYTE (r); // initial RED
   WRITE_BYTE (g); // initial GREEN
   WRITE_BYTE (b); // initial BLUE
   WRITE_BYTE (1); // initial ALPHA
   WRITE_BYTE (r); // effect RED
   WRITE_BYTE (g); // effect GREEN
   WRITE_BYTE (b); // effect BLUE
   WRITE_BYTE (1); // effect ALPHA
   WRITE_SHORT (0); // fade-in time in seconds * 256
   WRITE_SHORT (0); // fade-out time in seconds * 256
   WRITE_SHORT (duration); // hold time in seconds * 256
   WRITE_STRING (string);//string); // send the string
   MESSAGE_END (); // end

   return;
}
