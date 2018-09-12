// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// console.cpp
//

#include "racc.h"


void ServerCommand (void)
{
   // this function is the dedicated server command handler for the new RACC server command we
   // registered at game start. It will be called by the engine each time a server command that
   // starts with "racc" is entered in the server console. It works exactly the same way as
   // ClientCommand() does, using the CmdArgc() and CmdArgv() facilities of the engine. Argv(0)
   // is the server command itself (here "racc") and the next ones are its arguments. Just like
   // the stdio command-line parsing in C when you write "int main (int argc, char **argv)".

   char pcmd[128];
   char arg1[128];
   char arg2[128];
   char arg3[128];
   char temp_string1[128];
   char temp_string2[128];
   int bot_index;
   int i;
   int j;
   int sector_i;
   int sector_j;
   MFILE *mfp;
   Vector v_pathdebug_from;
   Vector v_pathdebug_to;
   sector_t *pSector;
   walkface_t *pWalkface;
   navnode_t *pNavNode;

   // get the command and up to 3 arguments
   strcpy (pcmd, CMD_ARGV (1));
   strcpy (arg1, CMD_ARGV (2));
   strcpy (arg2, CMD_ARGV (3));
   strcpy (arg3, CMD_ARGV (4));

   // have we been requested for help ?
   if ((strcmp (pcmd, "help") == 0) || (strcmp (pcmd, "?") == 0))
   {
      // then display a nice help page
      ServerConsole_printf ("%s\n", racc_welcometext);
      ServerConsole_printf ("  -- Available server commands:\n");
      ServerConsole_printf ("racc add - Add a bot to the current game\n");
      ServerConsole_printf ("racc kick - Disconnect a bot from the current game\n");
      ServerConsole_printf ("racc kickall - Disconnect all bots from the current game\n");
      ServerConsole_printf ("racc killall - Kill all bots inside the current game\n");
      ServerConsole_printf ("racc autofill - Enable/Disable bots filling up the server automatically\n");
      ServerConsole_printf ("racc internetmode - Enable/Disable bots leaving and joining randomly\n");
      ServerConsole_printf ("racc chatmode - Display/Change bot chat mode\n");
      ServerConsole_printf ("racc forceteam - Get/Set the team to force bots into\n");
      ServerConsole_printf ("racc minbots - Get/Set the minimal number of bots on the server\n");
      ServerConsole_printf ("racc maxbots - Get/Set the maximal number of bots on the server\n");
      ServerConsole_printf ("racc viewprofiles - Display the bot profiles database\n");
      ServerConsole_printf ("racc viewfootstepsounds - Display the bot footsteps sounds database\n");
      ServerConsole_printf ("racc viewricochetsounds - Display the bot ricochet sounds database\n");
      ServerConsole_printf ("racc viewsounds - Display the bot sounds database\n");
      ServerConsole_printf ("racc viewweapons - Display the bot weapons database\n");
      ServerConsole_printf ("racc viewbones - Display the bot bones database\n");
      ServerConsole_printf ("racc viewlanguages - Display the bot languages database\n");
      ServerConsole_printf ("racc mesh2dxf - Dump the global navmesh in a DXF file\n");
      ServerConsole_printf ("racc mesh2bmp - Dump the global navmesh in a BMP file\n");
      ServerConsole_printf ("racc sector2dxf - Dump specified sector mesh in DXF file (if no args, dump all)\n");
      ServerConsole_printf ("racc sector2bmp - Dump specified sector mesh in BMP file (if no args, dump all)\n");
      ServerConsole_printf ("racc trainhal - Train all bot's HAL with the specified file\n");
      ServerConsole_printf ("racc botcount - Display the number of bots currently in the game\n");
      ServerConsole_printf ("racc playercount - Display the total number of players currently in the game\n");
      ServerConsole_printf ("racc time - Display the current map play time\n");
      ServerConsole_printf ("racc debug - [developers only] get/set the various debug modes\n");
      ServerConsole_printf ("racc botorder - [developers only] force a bot to issue a ClientCommand\n");
      ServerConsole_printf ("racc msec - [developers only] get/set the msec calculation method\n");
      ServerConsole_printf ("racc evaluate - [developers only] evaluate a sound file for the bots\n");
      ServerConsole_printf ("  -- Additional listenserver-only commands:\n");
      ServerConsole_printf ("racc botcontrol - Take over/release control on the bot currently spectated\n");
      ServerConsole_printf ("racc here - Set the pathfinder's debug start point to the player's position\n");
      ServerConsole_printf ("racc there - Set the pathfinder's debug goal point to the player's position\n");
      ServerConsole_printf ("racc findpath - Start a pathmachine debug search\n");
      ServerConsole_printf ("racc comehere - Ask all bots to find their way to the player\n");
      ServerConsole_printf ("racc viewsector - Display the whole navmesh sector the player is standing in\n");
      ServerConsole_printf ("racc viewlinks - Display the links for the walkface the player is standing upon\n");
   }

   // else do we want to add a bot ?
   else if (strcmp (pcmd, "add") == 0)
   {
      BotCreate (); // slap a bot in
      server.bot_check_time = server.time + 10.0; // delay a while before checking the bot counts
   }

   // else do we want to kick one bot ?
   else if (strcmp (pcmd, "kick") == 0)
   {
      // cycle through all bot slots and kick the first we find
      for (bot_index = 0; bot_index < server.max_clients; bot_index++)
         if (IsValidPlayer (&players[bot_index]) && players[bot_index].is_racc_bot)
         {
            players[bot_index].Bot.quit_game_time = server.time; // mark this bot for disconnection
            break; // stop looping (don't kick the whole population, eh)
         }

      server.bot_check_time = server.time + 0.5; // delay a while before checking the bot counts
   }

   // else do we want to kick ALL the bots ?
   else if (strcmp (pcmd, "kickall") == 0)
   {
      // cycle through all bot slots and kick all those we find
      for (bot_index = 0; bot_index < server.max_clients; bot_index++)
         if (IsValidPlayer (&players[bot_index]) && players[bot_index].is_racc_bot)
            players[bot_index].Bot.quit_game_time = server.time; // mark all bots for disconnection

      server.bot_check_time = server.time + 0.5; // delay a while before checking the bot counts
   }

   // else do we want to slaughter all the bots in a bloodbath ?
   else if (strcmp (pcmd, "killall") == 0)
   {
      // cycle through all bot slots and kill all those we find
      for (bot_index = 0; bot_index < server.max_clients; bot_index++)
         if (IsValidPlayer (&players[bot_index]) && players[bot_index].is_racc_bot)
         {
            players[bot_index].pEntity->v.frags++; // increment its frag count not to count this as a suicide
            MDLL_ClientKill (players[bot_index].pEntity); // force this bot to suicide
         }
   }

   // else do we want to change the autofill feature ?
   else if (strcmp (pcmd, "autofill") == 0)
   {
      if (arg1[0] != 0)
         server.is_autofill = (atoi (arg1) == 1); // change the variable state when specified
      ServerConsole_printf ("Automatic server filling is %s\n", (server.is_autofill ? "ENABLED" : "DISABLED")); // print feature status
   }

   // else do we want to change the internet mode feature ?
   else if (strcmp (pcmd, "internetmode") == 0)
   {
      if (arg1[0] != 0)
         server.is_internetmode = (atoi (arg1) == 1); // change the variable state when specified
      ServerConsole_printf ("Internet mode is %s\n", (server.is_internetmode ? "ENABLED" : "DISABLED")); // print feature status
   }

   // else do we want to change the chat mode feature ?
   else if (strcmp (pcmd, "chatmode") == 0)
   {
      if (arg1[0] != 0)
         server.is_internetmode = (atoi (arg1) == 1); // change the variable state when specified
      ServerConsole_printf ("Bot chat mode is %d (%s)\n", server.bot_chat_mode,
                            ((server.bot_chat_mode == BOT_CHAT_TEXTAUDIO) ? "TEXT + AUDIO" :
                             ((server.bot_chat_mode == BOT_CHAT_AUDIOONLY) ? "AUDIO ONLY" :
                              ((server.bot_chat_mode == BOT_CHAT_TEXTONLY) ? "TEXT ONLY" : "NONE"))));
   }

   // else do we want to get or set the bot forced teams feature ?
   else if (strcmp (pcmd, "forceteam") == 0)
   {
      // have we been passed an argument ?
      if (arg1[0] != 0)
      {
         // either we are specified to stop forcing bots, or the team to force them into
         if (stricmp ("NONE", arg1) == 0)
            server.bot_forced_team[0] = 0; // disable team forcing
         else
            sprintf (server.bot_forced_team, arg1); // set new team to force
      }

      // is bot team forcing finally enabled ?
      if (server.bot_forced_team[0] != 0)
      {
         ServerConsole_printf ("Bots forced to team %s\n", server.bot_forced_team); // print variable status
         ServerConsole_printf ("To disable, enter: 'racc forceteam NONE'\n"); // and help message
      }
      else
         ServerConsole_printf ("Bots team forcing is DISABLED\n"); // print variable status
   }

   // else do we want to get or set the minimal number of bots ?
   else if (strcmp (pcmd, "minbots") == 0)
   {
      if (arg1[0] != 0)
         server.min_bots = atoi (arg1); // if there's an argument, set min_bots to it
      ServerConsole_printf ("Minimal number of bots is %d\n", server.min_bots); // print variable status
   }

   // else do we want to get or set the maximal number of bots ?
   else if (strcmp (pcmd, "maxbots") == 0)
   {
      if (arg1[0] != 0)
         server.max_bots = atoi (arg1); // if there's an argument, set max_bots to it
      ServerConsole_printf ("Maximal number of bots is %d\n", server.max_bots); // print variable status
   }

   // else do we want to display the profiles database ?
   else if (strcmp (pcmd, "viewprofiles") == 0)
   {
      ServerConsole_printf ("Bot profiles:\n"); // tell what we're about to do

      // cycle through all profiles and display them
      for (i = 0; i < profile_count; i++)
         ServerConsole_printf ("name '%s', model '%s', logo '%s', nation '%s', skill %d, team %d, class %d\n",
                               profiles[i].name,
                               profiles[i].skin,
                               profiles[i].logo,
                               profiles[i].nationality,
                               profiles[i].skill,
                               profiles[i].team,
                               profiles[i].subclass);

      ServerConsole_printf ("   %d profiles.\n", profile_count); // display the count
   }

   // else do we want to display the footsteps sounds database ?
   else if (strcmp (pcmd, "viewfootstepsounds") == 0)
   {
      ServerConsole_printf ("Footstep sounds:\n"); // tell what we're about to do

      // cycle through all footstep sounds and display them
      for (i = 0; i < footstepsound_count; i++)
         ServerConsole_printf ("texture '%c', footstep sound \"%s\"\n",
                               footstepsounds[i].texture_type,
                               footstepsounds[i].file_path);

      ServerConsole_printf ("   %d footstep sounds.\n", footstepsound_count); // display the count
   }

   // else do we want to display the ricochet sounds database ?
   else if (strcmp (pcmd, "viewricochetsounds") == 0)
   {
      ServerConsole_printf ("Ricochet sounds:\n"); // tell what we're about to do

      // cycle through all ricochet sounds and display them
      for (i = 0; i < ricochetsound_count; i++)
         ServerConsole_printf ("texture '%c', ricochet \"%s\"\n",
                               ricochetsounds[i].texture_type,
                               ricochetsounds[i].file_path);

      ServerConsole_printf ("   %d ricochet sounds.\n", ricochetsound_count); // display the count
   }

   // else do we want to display the global sounds database ?
   else if (strcmp (pcmd, "viewsounds") == 0)
   {
      ServerConsole_printf ("Global sounds:\n"); // tell what we're about to do

      // cycle through all sounds and display them
      for (i = 0; i < sound_count; i++)
         ServerConsole_printf ("'%s', loudness %.2f, duration %.2f\n",
                               sounds[i].file_path,
                               sounds[i].loudness,
                               sounds[i].duration);

      ServerConsole_printf ("   %d sounds.\n", sound_count); // display the count
   }

   // else do we want to display the weapons database ?
   else if (strcmp (pcmd, "viewweapons") == 0)
   {
      ServerConsole_printf ("Weapons:\n"); // tell what we're about to do

      // cycle through all weapons and display them
      for (i = 0; i < weapon_count; i++)
         ServerConsole_printf ("[%s]\n"
                               "model=\"%s\"\n"
                               "id=%d\n"
                               "weight=%.1f\n"
                               "class=\"%s\"\n"
                               "price=%.2f\n"
                               "primary.properties=\"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\"\n"
                               "primary.range=\"%s\"\n"
                               "primary.type_of_ammo=%d\n"
                               "primary.min_ammo=%d\n"
                               "primary.max_ammo=%d\n"
                               "primary.charge_delay=%.1f\n"
                               "primary.sound1=\"%s\"\n"
                               "primary.sound2=\"%s\"\n"
                               "primary.min_delay=%.1f %.1f %.1f %.1f %.1f\n"
                               "primary.max_delay=%.1f %.1f %.1f %.1f %.1f\n"
                               "secondary.properties=\"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\"\n"
                               "secondary.range=\"%s\"\n"
                               "secondary.type_of_ammo=%d\n"
                               "secondary.min_ammo=%d\n"
                               "secondary.max_ammo=%d\n"
                               "secondary.charge_delay=%.1f\n"
                               "secondary.sound1=\"%s\"\n"
                               "secondary.sound2=\"%s\"\n"
                               "secondary.min_delay=%.1f %.1f %.1f %.1f %.1f\n"
                               "secondary.max_delay=%.1f %.1f %.1f %.1f %.1f\n"
                               "\n",
                               weapons[i].classname,
                               weapons[i].model,
                               weapons[i].id,
                               weapons[i].weight,
                               (weapons[i].weapon_class == WEAPON_CLASS_PRIMARY ? "primary" :
                                (weapons[i].weapon_class == WEAPON_CLASS_SECONDARY ? "secondary" :
                                 (weapons[i].weapon_class == WEAPON_CLASS_GRENADE ? "grenade" :
                                  ""))),
                               weapons[i].price,
                               (weapons[i].primary.properties & WEAPONRAIL_PROPERTY_DISABLER ? "disabler," : ""),
                               (weapons[i].primary.properties & WEAPONRAIL_PROPERTY_WATERPROOF ? "waterproof," : ""),
                               (weapons[i].primary.properties & WEAPONRAIL_PROPERTY_LIGHTDAMAGE ? "lightdamage," : ""),
                               (weapons[i].primary.properties & WEAPONRAIL_PROPERTY_MEDIUMDAMAGE ? "mediumdamage," : ""),
                               (weapons[i].primary.properties & WEAPONRAIL_PROPERTY_HEAVYDAMAGE ? "heavydamage," : ""),
                               (weapons[i].primary.properties & WEAPONRAIL_PROPERTY_RADIUSEFFECT ? "radiuseffect," : ""),
                               (weapons[i].primary.properties & WEAPONRAIL_PROPERTY_AUTOMATIC ? "automatic," : ""),
                               (weapons[i].primary.properties & WEAPONRAIL_PROPERTY_BUCKSHOT ? "buckshot," : ""),
                               (weapons[i].primary.properties & WEAPONRAIL_PROPERTY_SCOPED ? "scoped," : ""),
                               (weapons[i].primary.properties & WEAPONRAIL_PROPERTY_SNIPER ? "sniper," : ""),
                               (weapons[i].primary.properties & WEAPONRAIL_PROPERTY_SILENCED ? "silenced," : ""),
                               (weapons[i].primary.properties & WEAPONRAIL_PROPERTY_MISSILE ? "missile," : ""),
                               (weapons[i].primary.properties & WEAPONRAIL_PROPERTY_HOMING ? "homing," : ""),
                               (weapons[i].primary.properties & WEAPONRAIL_PROPERTY_TOSS ? "toss," : ""),
                               (weapons[i].primary.properties & WEAPONRAIL_PROPERTY_PLACE ? "place," : ""),
                               (weapons[i].primary.range == WEAPONRAIL_RANGE_MELEE ? "melee" :
                                (weapons[i].primary.range == WEAPONRAIL_RANGE_CLOSE ? "close" :
                                 (weapons[i].primary.range == WEAPONRAIL_RANGE_MEDIUM ? "medium" :
                                  (weapons[i].primary.range == WEAPONRAIL_RANGE_FAR ? "far" :
                                   "")))),
                               weapons[i].primary.type_of_ammo,
                               weapons[i].primary.min_ammo,
                               weapons[i].primary.max_ammo,
                               weapons[i].primary.charge_delay,
                               weapons[i].primary.sound1,
                               weapons[i].primary.sound2,
                               weapons[i].primary.min_delay[0], weapons[i].primary.min_delay[1], weapons[i].primary.min_delay[2], weapons[i].primary.min_delay[3], weapons[i].primary.min_delay[4],
                               weapons[i].primary.max_delay[0], weapons[i].primary.max_delay[1], weapons[i].primary.max_delay[2], weapons[i].primary.max_delay[3], weapons[i].primary.max_delay[4],
                               (weapons[i].secondary.properties & WEAPONRAIL_PROPERTY_DISABLER ? "disabler," : ""),
                               (weapons[i].secondary.properties & WEAPONRAIL_PROPERTY_WATERPROOF ? "waterproof," : ""),
                               (weapons[i].secondary.properties & WEAPONRAIL_PROPERTY_LIGHTDAMAGE ? "lightdamage," : ""),
                               (weapons[i].secondary.properties & WEAPONRAIL_PROPERTY_MEDIUMDAMAGE ? "mediumdamage," : ""),
                               (weapons[i].secondary.properties & WEAPONRAIL_PROPERTY_HEAVYDAMAGE ? "heavydamage," : ""),
                               (weapons[i].secondary.properties & WEAPONRAIL_PROPERTY_RADIUSEFFECT ? "radiuseffect," : ""),
                               (weapons[i].secondary.properties & WEAPONRAIL_PROPERTY_AUTOMATIC ? "automatic," : ""),
                               (weapons[i].secondary.properties & WEAPONRAIL_PROPERTY_BUCKSHOT ? "buckshot," : ""),
                               (weapons[i].secondary.properties & WEAPONRAIL_PROPERTY_SCOPED ? "scoped," : ""),
                               (weapons[i].secondary.properties & WEAPONRAIL_PROPERTY_SNIPER ? "sniper," : ""),
                               (weapons[i].secondary.properties & WEAPONRAIL_PROPERTY_SILENCED ? "silenced," : ""),
                               (weapons[i].secondary.properties & WEAPONRAIL_PROPERTY_MISSILE ? "missile," : ""),
                               (weapons[i].secondary.properties & WEAPONRAIL_PROPERTY_HOMING ? "homing," : ""),
                               (weapons[i].secondary.properties & WEAPONRAIL_PROPERTY_TOSS ? "toss," : ""),
                               (weapons[i].secondary.properties & WEAPONRAIL_PROPERTY_PLACE ? "place," : ""),
                               (weapons[i].secondary.range == WEAPONRAIL_RANGE_MELEE ? "melee" :
                                (weapons[i].secondary.range == WEAPONRAIL_RANGE_CLOSE ? "close" :
                                 (weapons[i].secondary.range == WEAPONRAIL_RANGE_MEDIUM ? "medium" :
                                  (weapons[i].secondary.range == WEAPONRAIL_RANGE_FAR ? "far" :
                                   "")))),
                               weapons[i].secondary.type_of_ammo,
                               weapons[i].secondary.min_ammo,
                               weapons[i].secondary.max_ammo,
                               weapons[i].secondary.charge_delay,
                               weapons[i].secondary.sound1,
                               weapons[i].secondary.sound2,
                               weapons[i].secondary.min_delay[0], weapons[i].secondary.min_delay[1], weapons[i].secondary.min_delay[2], weapons[i].secondary.min_delay[3], weapons[i].secondary.min_delay[4],
                               weapons[i].secondary.max_delay[0], weapons[i].secondary.max_delay[1], weapons[i].secondary.max_delay[2], weapons[i].secondary.max_delay[3], weapons[i].secondary.max_delay[4]);

      ServerConsole_printf ("   %d weapons.\n", weapon_count); // display the count
   }

   // else do we want to display the bones database ?
   else if (strcmp (pcmd, "viewbones") == 0)
   {
      ServerConsole_printf ("Bones:\n"); // tell what we're about to do

      // display all the bones with their numbers
      ServerConsole_printf ("bone 0, 'pelvis', number %d\n"
                            "bone 1, 'spine', number %d\n"
                            "bone 2, 'spine1', number %d\n"
                            "bone 3, 'spine2', number %d\n"
                            "bone 4, 'spine3', number %d\n"
                            "bone 5, 'neck', number %d\n"
                            "bone 6, 'head', number %d\n"
                            "bone 7, 'left clavicle', number %d\n"
                            "bone 8, 'left upperarm', number %d\n"
                            "bone 9, 'left forearm', number %d\n"
                            "bone 10, 'left hand', number %d\n"
                            "bone 11, 'left finger0', number %d\n"
                            "bone 12, 'left finger01', number %d\n"
                            "bone 13, 'left finger1', number %d\n"
                            "bone 14, 'left finger11', number %d\n"
                            "bone 15, 'left thigh', number %d\n"
                            "bone 16, 'left calf', number %d\n"
                            "bone 17, 'left foot', number %d\n"
                            "bone 18, 'right clavicle', number %d\n"
                            "bone 19, 'right upperarm', number %d\n"
                            "bone 20, 'right forearm', number %d\n"
                            "bone 21, 'right hand', number %d\n"
                            "bone 22, 'right finger0', number %d\n"
                            "bone 23, 'right finger01', number %d\n"
                            "bone 24, 'right finger1', number %d\n"
                            "bone 25, 'right finger11', number %d\n"
                            "bone 26, 'right thigh', number %d\n"
                            "bone 27, 'right calf', number %d\n"
                            "bone 28, 'right foot', number %d\n"
                            "\n",
                            GameConfig.playerbones.pelvis,
                            GameConfig.playerbones.spine,
                            GameConfig.playerbones.spine1,
                            GameConfig.playerbones.spine2,
                            GameConfig.playerbones.spine3,
                            GameConfig.playerbones.neck,
                            GameConfig.playerbones.head,
                            GameConfig.playerbones.left_clavicle,
                            GameConfig.playerbones.left_upperarm,
                            GameConfig.playerbones.left_forearm,
                            GameConfig.playerbones.left_hand,
                            GameConfig.playerbones.left_finger0,
                            GameConfig.playerbones.left_finger01,
                            GameConfig.playerbones.left_finger1,
                            GameConfig.playerbones.left_finger11,
                            GameConfig.playerbones.left_thigh,
                            GameConfig.playerbones.left_calf,
                            GameConfig.playerbones.left_foot,
                            GameConfig.playerbones.right_clavicle,
                            GameConfig.playerbones.right_upperarm,
                            GameConfig.playerbones.right_forearm,
                            GameConfig.playerbones.right_hand,
                            GameConfig.playerbones.right_finger0,
                            GameConfig.playerbones.right_finger01,
                            GameConfig.playerbones.right_finger1,
                            GameConfig.playerbones.right_finger11,
                            GameConfig.playerbones.right_thigh,
                            GameConfig.playerbones.right_calf,
                            GameConfig.playerbones.right_foot);

      ServerConsole_printf ("   29 bones.\n"); // display the count
   }

   // else do we want to display the languages database ?
   else if (strcmp (pcmd, "viewlanguages") == 0)
   {
      ServerConsole_printf ("Languages:\n"); // tell what we're about to do

      // display all the languages with some info about them
      for (i = 0; i < language_count; i++)
      {
         ServerConsole_printf ("Language: %s\n", languages[i].language);
         ServerConsole_printf ("TEXT:\n");
         ServerConsole_printf ("Number of affirmatives: %d\n", languages[i].text.affirmative_count);
         ServerConsole_printf ("Number of negatives: %d\n", languages[i].text.negative_count);
         ServerConsole_printf ("Number of hello: %d\n", languages[i].text.hello_count);
         ServerConsole_printf ("Number of laughs: %d\n", languages[i].text.laugh_count);
         ServerConsole_printf ("Number of whines: %d\n", languages[i].text.whine_count);
         ServerConsole_printf ("Number of idles: %d\n", languages[i].text.idle_count);
         ServerConsole_printf ("Number of follows: %d\n", languages[i].text.follow_count);
         ServerConsole_printf ("Number of stops: %d\n", languages[i].text.stop_count);
         ServerConsole_printf ("Number of stays: %d\n", languages[i].text.stay_count);
         ServerConsole_printf ("Number of helps: %d\n", languages[i].text.help_count);
         ServerConsole_printf ("Number of cants: %d\n", languages[i].text.cant_count);
         ServerConsole_printf ("Number of byes: %d\n", languages[i].text.bye_count);
         ServerConsole_printf ("AUDIO:\n");
         ServerConsole_printf ("Number of affirmatives: %d\n", languages[i].audio.affirmative_count);
         ServerConsole_printf ("Number of alerts: %d\n", languages[i].audio.alert_count);
         ServerConsole_printf ("Number of attackings: %d\n", languages[i].audio.attacking_count);
         ServerConsole_printf ("Number of firstspawns: %d\n", languages[i].audio.firstspawn_count);
         ServerConsole_printf ("Number of inpositions: %d\n", languages[i].audio.inposition_count);
         ServerConsole_printf ("Number of negatives: %d\n", languages[i].audio.negative_count);
         ServerConsole_printf ("Number of reports: %d\n", languages[i].audio.report_count);
         ServerConsole_printf ("Number of reportings: %d\n", languages[i].audio.reporting_count);
         ServerConsole_printf ("Number of seegrenades: %d\n", languages[i].audio.seegrenade_count);
         ServerConsole_printf ("Number of takingdamages: %d\n", languages[i].audio.takingdamage_count);
         ServerConsole_printf ("Number of throwgrenades: %d\n", languages[i].audio.throwgrenade_count);
         ServerConsole_printf ("Number of victories: %d\n", languages[i].audio.victory_count);
         ServerConsole_printf ("Sample:\n");
         ServerConsole_printf ("\"%s\"\n", languages[i].text.idle[RandomLong (0, languages[i].text.affirmative_count - 1)]);
         ServerConsole_printf ("\n");
      }

      ServerConsole_printf ("   %d languages.\n", language_count); // display the count
   }

   // else do we want to draw a DXF file of the global map mesh ?
   else if (strcmp (pcmd, "mesh2dxf") == 0)
   {
      // build the DXF file name
      sprintf (temp_string1, "%s/knowledge/%s/%s-MESH.dxf", GameConfig.racc_basedir, GameConfig.mod_name, server.map_name);
      ServerConsole_printf ("RACC: drawing global mesh (%d walkfaces) to DXF...\n", map.walkfaces_count);

      InitDebugDXF (); // first init the debug DXF buffer

      // cycle through all walkfaces in the navmesh...
      for (i = 0; i < map.walkfaces_count; i++)
      {
         sprintf (temp_string2, "Walkface_%d", i); // build layer name after walkface number
         DrawWalkfaceInDebugDXF (&map.walkfaces[i], 7, temp_string2); // and draw this walkface
      }

      WriteDebugDXF (temp_string1); // and then draw it
      ServerConsole_printf ("DXF file created: '%s'\n", temp_string1); // and tell us we're done
   }

   // else do we want to draw a BMP file of the global map mesh ?
   else if (strcmp (pcmd, "mesh2bmp") == 0)
   {
      // build the BMP file name
      sprintf (temp_string1, "%s/knowledge/%s/%s-MESH.bmp", GameConfig.racc_basedir, GameConfig.mod_name, server.map_name);
      ServerConsole_printf ("RACC: drawing global mesh (%d walkfaces) to bitmap file...\n", map.walkfaces_count);

      InitDebugBitmap (); // first init the debug bitmap buffer

      // cycle through all walkfaces in the navmesh...
      for (i = 0; i < map.walkfaces_count; i++)
         DrawWalkfaceInDebugBitmap (&map.walkfaces[i], 1); // and draw them

      WriteDebugBitmap (temp_string1); // and then draw it
      ServerConsole_printf ("BMP file created: '%s'\n", temp_string1); // and tell us we're done
   }

   // else do we want to draw a DXF file of a particuliar sector of the global map mesh ?
   else if (strcmp (pcmd, "sector2dxf") == 0)
   {
      // have we been specified sector coordinates ?
      if ((arg1[0] != 0) && (arg2[0] != 0))
      {
         sector_i = atoi (arg1); // get the sector parallel
         if ((sector_i < 0) || (sector_i >= map.parallels_count))
         {
            ServerConsole_printf ("%d: no such parallel in map '%s'\n", sector_i, server.map_name);
            return; // check for parallel validity
         }
         sector_j = atoi (arg2); // get the sector parallel
         if ((sector_j < 0) || (sector_j >= map.meridians_count))
         {
            ServerConsole_printf ("%d: no such meridian in map '%s'\n", sector_j, server.map_name);
            return; // check for parallel validity
         }

         // build the DXF file name
         sprintf (temp_string1, "%s/knowledge/%s/%s-%d-%d-MESH.dxf", GameConfig.racc_basedir, GameConfig.mod_name, server.map_name, sector_i, sector_j);
         ServerConsole_printf ("RACC: drawing sector [%d,%d] (%d walkfaces) to DXF file...\n", sector_i, sector_j, map.walkfaces_count);

         InitDebugDXF (); // first init the debug DXF buffer

         // cycle through all walkfaces in the specified sector...
         for (i = 0; i < map.topology[sector_i][sector_j].faces_count; i++)
         {
            sprintf (temp_string2, "Walkface_%d", i); // build layer name after walkface number
            DrawWalkfaceInDebugDXF (map.topology[sector_i][sector_j].faces[i], 7, temp_string2); // and draw this walkface
         }

         sprintf (temp_string2, "Sector_%d_%d", sector_i, sector_j); // build layer name after sector ID
         DrawSectorInDebugDXF (sector_i, sector_j, 5, temp_string2);  // now draw the sector itself
         WriteDebugDXF (temp_string1); // and then write the DXF file to disk
         ServerConsole_printf ("DXF file created: '%s'\n", temp_string1); // and tell us we're done
      }

      // no sector specified, let's dump the whole crap out !
      else
      {
         // tell us what we're doing...
         ServerConsole_printf ("RACC: drawing ALL sectors to DXF file...\n");

         // build the DXF file name
         sprintf (temp_string1, "%s/knowledge/%s/%s-MESH.dxf", GameConfig.racc_basedir, GameConfig.mod_name, server.map_name);

         InitDebugDXF (); // first init the debug DXF buffer

         // loop through all sectors...
         for (sector_i = 0; sector_i < map.parallels_count; sector_i++)
            for (sector_j = 0; sector_j < map.meridians_count; sector_j++)
            {
               sprintf (temp_string2, "Sector_%d_%d", sector_i, sector_j); // build layer name after sector ID

               // cycle through all walkfaces in the specified sector...
               for (i = 0; i < map.topology[sector_i][sector_j].faces_count; i++)
                  DrawWalkfaceInDebugDXF (map.topology[sector_i][sector_j].faces[i], 7, temp_string2); // and draw this walkface

               DrawSectorInDebugDXF (sector_i, sector_j, 5, temp_string2);  // now draw the sector itself
            }

         WriteDebugDXF (temp_string1); // and then write the DXF file to disk
         ServerConsole_printf ("All sectors dumped.\n"); // and tell us we're done
      }
   }

   // else do we want to draw a BMP file of a particuliar sector of the global map mesh ?
   else if (strcmp (pcmd, "sector2bmp") == 0)
   {
      // have we been specified sector coordinates ?
      if ((arg1[0] != 0) && (arg2[0] != 0))
      {
         sector_i = atoi (arg1); // get the sector parallel
         if ((sector_i < 0) || (sector_i >= map.parallels_count))
         {
            ServerConsole_printf ("%d: no such parallel in map '%s'\n", sector_i, server.map_name);
            return; // check for parallel validity
         }
         sector_j = atoi (arg2); // get the sector parallel
         if ((sector_j < 0) || (sector_j >= map.meridians_count))
         {
            ServerConsole_printf ("%d: no such meridian in map '%s'\n", sector_j, server.map_name);
            return; // check for parallel validity
         }

         // build the BMP file name
         sprintf (temp_string1, "%s/knowledge/%s/%s-%d-%d-MESH.bmp", GameConfig.racc_basedir, GameConfig.mod_name, server.map_name, sector_i, sector_j);
         ServerConsole_printf ("RACC: drawing sector [%d,%d] (%d walkfaces) to bitmap file...\n", sector_i, sector_j, map.walkfaces_count);

         InitDebugBitmap (); // first init the debug bitmap buffer

         // cycle through all walkfaces in the specified sector...
         for (i = 0; i < map.topology[sector_i][sector_j].faces_count; i++)
            DrawWalkfaceInDebugBitmap (map.topology[sector_i][sector_j].faces[i], 1); // and draw them

         DrawSectorInDebugBitmap (sector_i, sector_j, 5);  // now draw the sector itself
         WriteDebugBitmap (temp_string1); // and then write the BMP file to disk
         ServerConsole_printf ("BMP file created: '%s'\n", temp_string1); // and tell us we're done
      }

      // no sector specified, let's dump the whole crap out !
      else
      {
         // tell us what we're doing...
         ServerConsole_printf ("RACC: drawing ALL sectors to bitmap files...\n");

         // loop through all sectors...
         for (sector_i = 0; sector_i < map.parallels_count; sector_i++)
            for (sector_j = 0; sector_j < map.meridians_count; sector_j++)
            {
               // build the BMP file name
               sprintf (temp_string1, "%s/knowledge/%s/%s-%d-%d-MESH.bmp", GameConfig.racc_basedir, GameConfig.mod_name, server.map_name, sector_i, sector_j);

               InitDebugBitmap (); // first init the debug bitmap buffer

               // cycle through all walkfaces in the specified sector...
               for (i = 0; i < map.topology[sector_i][sector_j].faces_count; i++)
                  DrawWalkfaceInDebugBitmap (map.topology[sector_i][sector_j].faces[i], 1); // and draw them

               DrawSectorInDebugBitmap (sector_i, sector_j, 5);  // now draw the sector itself
               WriteDebugBitmap (temp_string1); // and then write the BMP file to disk
            }

         ServerConsole_printf ("All sectors dumped.\n"); // and tell us we're done
      }
   }

   // else do we want to train our bot's HAL with a text file ?
   else if (strcmp (pcmd, "trainhal") == 0)
   {
      // this one is prolly unsafe, anyway - better notify the caller of it...
      ServerConsole_printf ("trainhal: WARNING: DANGEROUS FACILITY!\n");

      // have we specified the name of the file to mess with ?
      if ((arg1[0] != 0) && FileExists (arg1))
      {
         ServerConsole_printf ("RACC: bot HALs learning from file"); // progress bar start message

         mfp = mfopen (arg1, "r"); // open file (can't fail to do this, FileExists() returned TRUE)

         // have we specified a number of lines to skip ?
         if (arg2[0] != 0)
            for (i = 0; i < atoi (arg2); i++)
               mfgets (temp_string1, 100, mfp); // skip as many lines as necessary

         // get one line in the file, line after line until the count is reached
         i = 0;
         while ((mfgets (temp_string1, 100, mfp) != NULL) && (i < atoi (arg3)))
         {
            temp_string1[100] = 0; // terminate the string just to be sure
            if (temp_string1[strlen (temp_string1) - 1] == '\n')
               temp_string1[strlen (temp_string1) - 1] = 0; // get rid of the carriage return, eventually

            // for every bot in game...
            for (bot_index = 0; bot_index < server.max_clients; bot_index++)
               if (IsValidPlayer (&players[bot_index]) && players[bot_index].is_racc_bot)
               {
                  // break this line into an array of words and make the bot learn it
                  HAL_MakeWords (UpperCase (temp_string1), players[bot_index].Bot.BotBrain.input_words);
                   HAL_Learn (&players[bot_index].Bot.BotBrain.HAL_model, players[bot_index].Bot.BotBrain.input_words);
               }

            if (i % 10 == 0)
               ServerConsole_printf ("."); // print a trailing dot as progress bar every 10 lines

            i++; // we read one line more
         }

         ServerConsole_printf (" skipped %d, read %d\n", atoi (arg2), atoi (arg3)); // terminate progress bar
         ServerConsole_printf ("done\n");
         mfclose (mfp); // learning process complete, close the file
      }
      else
         ServerConsole_printf ("trainhal: must specify filename, lines to skip, lines to read\n");
   }

   // else do we want to display the bot count ?
   else if (strcmp (pcmd, "botcount") == 0)
      ServerConsole_printf ("%d bots in game\n", bot_count); // display the bot count

   // else do we want to display the player count (including bots) ?
   else if (strcmp (pcmd, "playercount") == 0)
      ServerConsole_printf ("%d players in game\n", player_count); // display the player count

   // else do we want to display the server time ?
   else if (strcmp (pcmd, "time") == 0)
      ServerConsole_printf ("Current map play time: %f seconds\n", server.time); // display server time

   // else do we want to mess around with the debug levels ?
   else if (strcmp (pcmd, "debug") == 0)
   {
      // have we specified the debug switch or a debug command to mess with ?
      if (arg1[0] != 0)
      {
         // given the debug level specified, take the appropriate action
         if (strcmp ("eyes", arg1) == 0)
         {
            if (arg2[0] != 0)
               DebugLevel.eyes = atoi (arg2); // if there's an argument, set debug level to it
            ServerConsole_printf ("Vision debug level is %d\n", DebugLevel.eyes); // print debug level
         }
         else if (strcmp ("ears", arg1) == 0)
         {
            if (arg2[0] != 0)
               DebugLevel.ears = atoi (arg2); // if there's an argument, set debug level to it
            ServerConsole_printf ("Hearing debug level is %d\n", DebugLevel.ears); // print debug level
         }
         else if (strcmp ("body", arg1) == 0)
         {
            if (arg2[0] != 0)
               DebugLevel.body = atoi (arg2); // if there's an argument, set debug level to it
            ServerConsole_printf ("Feeling debug level is %d\n", DebugLevel.body); // print debug level
         }
         else if (strcmp ("legs", arg1) == 0)
         {
            if (arg2[0] != 0)
               DebugLevel.legs = atoi (arg2); // if there's an argument, set debug level to it
            ServerConsole_printf ("Movement debug level is %d\n", DebugLevel.legs); // print debug level
         }
         else if (strcmp ("hand", arg1) == 0)
         {
            if (arg2[0] != 0)
               DebugLevel.hand = atoi (arg2); // if there's an argument, set debug level to it
            ServerConsole_printf ("Weapon usage debug level is %d\n", DebugLevel.hand); // print debug level
         }
         else if (strcmp ("chat", arg1) == 0)
         {
            if (arg2[0] != 0)
               DebugLevel.chat = atoi (arg2); // if there's an argument, set debug level to it
            ServerConsole_printf ("Chat debug level is %d\n", DebugLevel.chat); // print debug level
         }
         else if (strcmp ("aiconsole", arg1) == 0)
         {
            DebugLevel.aiconsole ^= TRUE; // switch debug level on/off (XOR it)

            // if AI console is enabled, raise all debug levels if necessary
            if (DebugLevel.aiconsole)
            {
               if (DebugLevel.eyes == 0)
                  DebugLevel.eyes = 1;
               if (DebugLevel.ears == 0)
                  DebugLevel.ears = 1;
               if (DebugLevel.body == 0)
                  DebugLevel.body = 1;
               if (DebugLevel.legs == 0)
                  DebugLevel.legs = 1;
               if (DebugLevel.hand == 0)
                  DebugLevel.hand = 1;
               if (DebugLevel.chat == 0)
                  DebugLevel.chat = 1;
               if (DebugLevel.cognition == 0)
                  DebugLevel.cognition = 1;
               if (DebugLevel.navigation == 0)
                  DebugLevel.navigation = 1;
            }

            ServerConsole_printf ("AI console is %s\n", (DebugLevel.aiconsole ? "ENABLED" : "DISABLED"));
         }

         // else do we want to enable or disable a particular AI vector ?
         else if ((strcmp ("enable", arg1) == 0) || (strcmp ("disable", arg1) == 0))
         {
            // have we been specified a vector name at all ?
            if (arg2[0] != 0)
            {
               // given the AI vector to enable, take the appropriate action
               if (strcmp ("eyes", arg2) == 0)
               {
                  DebugLevel.eyes_disabled = (arg1[0] == 'd'); // enable/disable eyes
                  ServerConsole_printf ("AI sensitive vector 'eyes' %s\n", (!DebugLevel.eyes_disabled ? "ENABLED" : "DISABLED"));
               }
               else if (strcmp ("ears", arg2) == 0)
               {
                  DebugLevel.ears_disabled = (arg1[0] == 'd'); // enable/disable ears
                  ServerConsole_printf ("AI sensitive vector 'ears' %s\n", (!DebugLevel.ears_disabled ? "ENABLED" : "DISABLED"));
               }
               else if (strcmp ("body", arg2) == 0)
               {
                  DebugLevel.body_disabled = (arg1[0] == 'd'); // enable/disable body
                  ServerConsole_printf ("AI sensitive vector 'body' %s\n", (!DebugLevel.body_disabled ? "ENABLED" : "DISABLED"));
               }
               else if (strcmp ("legs", arg2) == 0)
               {
                  DebugLevel.legs_disabled = (arg1[0] == 'd'); // enable/disable legs
                  ServerConsole_printf ("AI motile vector 'legs' %s\n", (!DebugLevel.legs_disabled ? "ENABLED" : "DISABLED"));
               }
               else if (strcmp ("hand", arg2) == 0)
               {
                  DebugLevel.hand_disabled = (arg1[0] == 'd'); // enable/disable hand
                  ServerConsole_printf ("AI motile vector 'hand' %s\n", (!DebugLevel.hand_disabled ? "ENABLED" : "DISABLED"));
               }
               else if (strcmp ("chat", arg2) == 0)
               {
                  DebugLevel.chat_disabled = (arg1[0] == 'd'); // enable/disable chat
                  ServerConsole_printf ("AI motile vector 'chat' %s\n", (!DebugLevel.chat_disabled ? "ENABLED" : "DISABLED"));
               }
               else
                  ServerConsole_printf ("RACC: Not an AI vector '%s'\n", arg2); // bad vector
            }
            else
               ServerConsole_printf ("RACC: must specify an AI vector\n"); // nothing specified
         }

         // special debug switches
         else if (strcmp ("observer", arg1) == 0)
         {
            DebugLevel.is_observer ^= TRUE; // switch debug level on/off (XOR it)
            ServerConsole_printf ("Observer mode is %s\n", (DebugLevel.is_observer ? "ENABLED" : "DISABLED"));
         }
         else if (strcmp ("peacemode", arg1) == 0)
         {
            DebugLevel.is_peacemode ^= TRUE; // switch debug level on/off (XOR it)
            ServerConsole_printf ("Peace mode is %s\n", (DebugLevel.is_peacemode ? "ENABLED" : "DISABLED"));
         }
         else if (strcmp ("dontfind", arg1) == 0)
         {
            DebugLevel.is_dontfindmode ^= TRUE; // switch debug level on/off (XOR it)
            ServerConsole_printf ("Don't find mode is %s\n", (DebugLevel.is_dontfindmode ? "ENABLED" : "DISABLED"));
         }
         else if (strcmp ("inhumanturns", arg1) == 0)
         {
            DebugLevel.is_inhumanturns ^= TRUE; // switch debug level on/off (XOR it)
            ServerConsole_printf ("Inhuman turns mode is %s\n", (DebugLevel.is_inhumanturns ? "ENABLED" : "DISABLED"));
         }
         else if (strcmp ("cognition", arg1) == 0)
         {
            if (arg2[0] != 0)
               DebugLevel.cognition = atoi (arg2); // if there's an argument, set debug level to it
            ServerConsole_printf ("Cognition debug level is %d\n", DebugLevel.cognition); // print debug level
         }
         else if (strcmp ("navigation", arg1) == 0)
         {
            if (arg2[0] != 0)
               DebugLevel.navigation = atoi (arg2); // if there's an argument, set debug level to it
            ServerConsole_printf ("Navigation debug level is %d\n", DebugLevel.navigation); // print debug level
         }
         else if (strcmp ("pause", arg1) == 0)
         {
            DebugLevel.is_paused ^= TRUE; // switch pause on/off (XOR it)
            ServerConsole_printf ("AI is %s\n", (DebugLevel.is_paused ? "PAUSED" : "running")); // print debug level
         }
         else if (strcmp ("break", arg1) == 0)
         {
            DebugLevel.is_broke ^= TRUE; // toggle code break for the current frame...
            ServerConsole_printf ("Breakpoint %s\n", (DebugLevel.is_broke ? "RAISED" : "cleared")); // ...and notify the programmer
         }
         else
            ServerConsole_printf ("RACC: No debug level for '%s'\n", arg1); // typo error ?
      }

      // else if nothing has been specified, just print out the debug levels
      else
      {
         ServerConsole_printf ("AI vectors:\n");
         ServerConsole_printf ("   Eyes %s (debug level %d)\n", (!DebugLevel.eyes_disabled ? "ENABLED" : "DISABLED"), DebugLevel.eyes);
         ServerConsole_printf ("   Ears %s (debug level %d)\n", (!DebugLevel.ears_disabled ? "ENABLED" : "DISABLED"), DebugLevel.ears);
         ServerConsole_printf ("   Body %s (debug level %d)\n", (!DebugLevel.body_disabled ? "ENABLED" : "DISABLED"), DebugLevel.body);
         ServerConsole_printf ("   Legs %s (debug level %d)\n", (!DebugLevel.legs_disabled ? "ENABLED" : "DISABLED"), DebugLevel.legs);
         ServerConsole_printf ("   Hand %s (debug level %d)\n", (!DebugLevel.hand_disabled ? "ENABLED" : "DISABLED"), DebugLevel.hand);
         ServerConsole_printf ("   Chat %s (debug level %d)\n", (!DebugLevel.chat_disabled ? "ENABLED" : "DISABLED"), DebugLevel.chat);
         ServerConsole_printf ("special switches:\n");
         ServerConsole_printf ("   Observer mode is %s\n", (DebugLevel.is_observer ? "ENABLED" : "DISABLED"));
         ServerConsole_printf ("   Peace mode is %s\n", (DebugLevel.is_peacemode ? "ENABLED" : "DISABLED"));
         ServerConsole_printf ("   Don't find mode is %s\n", (DebugLevel.is_dontfindmode ? "ENABLED" : "DISABLED"));
         ServerConsole_printf ("   Inhuman turns mode is %s\n", (DebugLevel.is_inhumanturns ? "ENABLED" : "DISABLED"));
         ServerConsole_printf ("   Navigation debug level: %d\n", DebugLevel.navigation);
         ServerConsole_printf ("AI is %s\n", (DebugLevel.is_paused ? "PAUSED" : "running"));
         ServerConsole_printf ("Breakpoint is %s\n", (DebugLevel.is_broke ? "SET" : "not set"));
         ServerConsole_printf ("AI console is %s\n", (DebugLevel.aiconsole ? "ENABLED" : "DISABLED"));
      }
   }

   // else do we want to force a bot to issue a client command ?
   else if (strcmp (pcmd, "botorder") == 0)
   {
      // check if we've got the right number of arguments
      if ((arg1[0] != 0) && (arg2[0] != 0))
      {
         // we've got the right number of arguments, so find the bot we want according to arg #1
         for (bot_index = 0; bot_index < server.max_clients; bot_index++)
            if (strcmp (players[bot_index].connection_name, arg1) == 0)
            {
               printf ("BOT %s executes command \"%s\"\n", players[bot_index].connection_name, arg2);
               FakeClientCommand (players[bot_index].pEntity, arg2); // and let it execute the client command
               break; // no need to search further
            }
      }
      else
         ServerConsole_printf ("botorder: syntax error\n"
                               "Usage is: racc botorder bot_name \"client_command\"\n"); // syntax error
   }

   // else do we want to get or set the frame time estimation method ?
   else if (strcmp (pcmd, "msec") == 0)
   {
      if (arg1[0] != 0)
         server.msec_method = atoi (arg1); // if there's an argument, change method to the one specified
      ServerConsole_printf ("Msec computation method is METHOD_%s\n", (server.msec_method == METHOD_DEBUG ? "DEBUG (slow-motion)" : (METHOD_HEIMANN ? "HEIMANN" : (server.msec_method == METHOD_HARTWIG ? "HARTWIG" : "WHITEHOUSE"))));
   }

   // else do we want to evaluate a particular sound file for the bots ?
   else if (strcmp (pcmd, "evaluate") == 0)
   {
      if (arg1[0] != 0)
         EvaluateSoundForBots (arg1); // if a sound file was specified, evaluate it
      else
         ServerConsole_printf ("evaluate: syntax error\n"
                               "Usage is: racc evaluate sound_name\n"); // syntax error
   }

   // else do we want to take/release control over the bot being spectated ?
   else if (IsValidPlayer (pListenserverPlayer) && (strcmp (pcmd, "botcontrol") == 0))
   {
      // cycle through all bots and try to find the bot we want
      for (i = 0; i < server.max_clients; i++)
         if (players[i].is_alive && players[i].is_racc_bot
             && (((arg1[0] == 0) && players[i].is_watched))
                 || (strcmp (players[i].connection_name, arg1) == 0))
            break;

      // found target bot ?
      if (i < server.max_clients)
      {
         // was it controlled already ?
         if (players[i].Bot.is_controlled)
         {
            players[i].Bot.is_controlled = FALSE; // then release it
            players[i].Bot.BotBrain.PathMachine.path_count = 0;
            players[i].Bot.BotBrain.bot_goal = 0; // zap bot's brain when we release it
         }
         else
            players[i].Bot.is_controlled = TRUE; // else take control of it

         // and display what we've done
         ServerConsole_printf ("botcontrol: %s is now %s\n", players[i].connection_name, (players[i].Bot.is_controlled ? "PLAYER CONTROLLED" : "AUTONOMOUS"));
      }
      else
         ServerConsole_printf ("botcontrol: no target found\n");
   }

   // else do we want to store the pathfinder debug start point ?
   else if (IsValidPlayer (pListenserverPlayer) && (strcmp (pcmd, "here") == 0))
   {
      v_pathdebug_from = pListenserverPlayer->v_origin; // set it at player's origin
      ServerConsole_printf ("Pathfinder debug start point stored as (%.1f, %.1f, %.1f)\n", v_pathdebug_from.x, v_pathdebug_from.y, v_pathdebug_from.z);
   }

   // else do we want to store the pathfinder debug goal point ?
   else if (IsValidPlayer (pListenserverPlayer) && (strcmp (pcmd, "there") == 0))
   {
      v_pathdebug_to = pListenserverPlayer->v_origin;
      ServerConsole_printf ("Pathfinder debug goal point stored as (%.1f, %.1f, %.1f)\n", v_pathdebug_to.x, v_pathdebug_to.y, v_pathdebug_to.z);
   }

   // else do we want to initiate a pathmachine debug search ?
   else if (IsValidPlayer (pListenserverPlayer) && (strcmp (pcmd, "findpath") == 0))
   {
      // is either the start of the search or the goal missing ?
      if (v_pathdebug_from == g_vecZero)
         ServerConsole_printf ("findpath: path debug FROM not set\n"); // start is not set
      else if (v_pathdebug_to == g_vecZero)
         ServerConsole_printf ("findpath: path debug TO not set\n"); // goal is not set

      // nothing is missing
      else
      {
         // cycle through all bot slots and stop at the first bot we find
         for (i = 0; i < server.max_clients; i++)
            if (IsValidPlayer (&players[i]) && players[i].is_racc_bot)
               break;

         // found one ?
         if (IsValidPlayer (&players[i]) && players[i].is_racc_bot)
         {
            ServerConsole_printf ("findpath: %s finds path from (%.1f, %.1f, %.1f) to (%.1f, %.1f, %.1f)\n", players[i].connection_name, v_pathdebug_from.x, v_pathdebug_from.y, v_pathdebug_from.z, v_pathdebug_to.x, v_pathdebug_to.y, v_pathdebug_to.z);
            BotFindPathFromTo (&players[i], v_pathdebug_from, v_pathdebug_to, TRUE);
         }
         else
            ServerConsole_printf ("findpath: no bot available to think about a path\n");
      }
   }

   // else do we want all bots to find their way around here ?
   else if (IsValidPlayer (pListenserverPlayer) && (strcmp (pcmd, "comehere") == 0))
   {
      // are there bots around ?
      if (bot_count > 0)
      {
         // tell what we're doing
         ServerConsole_printf ("comehere: calling all bots...\n");

         // cycle through all bot slots
         for (i = 0; i < server.max_clients; i++)
            if (IsValidPlayer (&players[i]) && players[i].is_racc_bot)
            {
               // and ask those we find to come around
               ServerConsole_printf ("%s finds path to %s\n", players[i].connection_name, pListenserverPlayer->connection_name);
               BotFindPathTo (&players[i], pListenserverPlayer->v_origin, TRUE);
            }
      }
      else
         ServerConsole_printf ("comehere: no bot available to call\n"); // can't find any bot
   }

   // else do we want to display all the walkfaces in the sector the player is standing on ?
   else if (IsValidPlayer (pListenserverPlayer) && (strcmp (pcmd, "viewsector") == 0))
   {
      // find the sector to which the player belongs now
      pSector = SectorUnder (pListenserverPlayer->v_origin);

      // tell what we're doing
      ServerConsole_printf ("viewsector: drawing sector [%d,%d], %d walkfaces involved\n",
                            (int) ((pListenserverPlayer->v_origin.x - map.v_worldmins.x) / (map.v_worldmaxs.x - map.v_worldmins.x) * map.parallels_count),
                            (int) ((pListenserverPlayer->v_origin.y - map.v_worldmins.y) / (map.v_worldmaxs.y - map.v_worldmins.y) * map.meridians_count),
                            pSector->faces_count);

      // now display the sector...
      UTIL_DrawSector (pSector, 100, 255, 0, 0);
      for (i = 0; i < pSector->faces_count; i++)
         UTIL_DrawWalkface (pSector->faces[i], 100, 255, 255, 255); // and each of its walkfaces
   }

   // else do we want to display all the navlinks in the walkface the player is standing on ?
   else if (IsValidPlayer (pListenserverPlayer) && (strcmp (pcmd, "viewlinks") == 0))
   {
      // are there bots around ?
      if (bot_count > 0)
      {
         // find the walkface the player is standing upon now
         pWalkface = WalkfaceUnder (pListenserverPlayer);

         // now cycle through all clients and stop by the first bot we find or the one we want
         for (i = 0; i < server.max_clients; i++)
            if (IsValidPlayer (&players[i]) && players[i].is_racc_bot
                && ((arg1[0] == 0)
                    || (strcmp (players[i].connection_name, arg1) == 0)))
            {
               pNavNode = &players[i].Bot.BotBrain.PathMemory[WalkfaceIndexOf (pWalkface)];

               // tell what we're doing
               ServerConsole_printf ("viewsector: drawing navnode #%d, %d navlinks involved\n",
                                     WalkfaceIndexOf (pWalkface),
                                     pNavNode->links_count);
               ServerConsole_printf ("viewsector: data according to %s\n", &players[i].connection_name);

               // now display the navnode (i.e, the walkface it represents)...
               UTIL_DrawWalkface (pWalkface, 100, 0, 0, 255);
               for (j = 0; j < pNavNode->links_count; j++)
                  UTIL_DrawNavlink (&pNavNode->links[j], 30); // and each of its navlinks

               break; // stop looping through all bots once this is done
            }
      }
      else
      {
         ServerConsole_printf ("viewlinks: no bot to query brain from\n"); // can't find any bot
         ServerConsole_printf ("Use 'viewlinks bot_name' to query from a particular bot.\n");
      }
   }

   // what sort of RACC server command is that ??
   else
   {
      ServerConsole_printf ("RACC: Unknown command: %s\n", pcmd);
      ServerConsole_printf ("Type \"racc help\" for list of available commands.\n");
   }

   return; // finished processing the server command
}
