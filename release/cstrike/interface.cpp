// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// CSTRIKE version
//
// interface.cpp
//

#define DEFINE_GLOBALS
#include "racc.h"


#ifdef WIN32 
int WINAPI DllMain (HINSTANCE hinstDLL, unsigned long fdwReason, void *lpvReserved)
{
   // dynamic library entry point, can be used for uninitialization stuff. NOT for initializing
   // anything because if you ever attempt to wander outside the scope of this function on a
   // DLL attach, LoadLibrary() will simply fail. Nice eh ? That's why we do init stuff in
   // Meta_Attach() and Meta_Detach, which are by the way platform-independent.

   return (TRUE);
}
#endif


////////////////////
// metamod interface


C_DLLEXPORT int Meta_Query (char *ifvers, plugin_info_t **pPlugInfo, mutil_funcs_t *pMetaUtilFuncs)
{
   // this function is the first function ever called by metamod in the plugin DLL. Its purpose
   // is for metamod to retrieve basic information about the plugin, such as its meta-interface
   // version, for ensuring compatibility with the current version of the running metamod.

   static char *plugin_ifvers = META_INTERFACE_VERSION;
   static char *plugin_name = "RACC";
   static char *plugin_author = "Pierre-Marie Baty <pm@bots-united.com>";
   static char *plugin_url = "http://racc.bots-united.com";

   // first, set up our plugin info structure (except version stuff, will be built dynamically)
   Plugin_info.ifvers = plugin_ifvers;
   Plugin_info.name = plugin_name;
   Plugin_info.author = plugin_author;
   Plugin_info.url = plugin_url;
   Plugin_info.logtag = plugin_name;
   Plugin_info.loadable = PT_CHANGELEVEL;
   Plugin_info.unloadable = PT_ANYTIME;

   // secondly, set up the plugin's API and function tables
   memset (&gMetaFunctionTable, 0, sizeof (gMetaFunctionTable));
   gMetaFunctionTable.pfnGetEntityAPI2 = GetEntityAPI2; // pre-game table
   gMetaFunctionTable.pfnGetEntityAPI2_Post = GetEntityAPI2_Post; // post-game table
   gMetaFunctionTable.pfnGetEngineFunctions = GetEngineFunctions; // pre-engine table
   gMetaFunctionTable.pfnGetEngineFunctions_Post = GetEngineFunctions_Post; // post-engine table

   // keep track of the pointers to metamod function tables metamod gives us
   gpMetaUtilFuncs = pMetaUtilFuncs;
   *pPlugInfo = &Plugin_info;

   // check for interface version compatibility
   if (strcmp (ifvers, META_INTERFACE_VERSION) != 0)
   {
      int mmajor = 0, mminor = 0, pmajor = 0, pminor = 0;

      LOG_CONSOLE (PLID, "RACC: meta-interface version mismatch (metamod: %s, RACC: %s)", ifvers, META_INTERFACE_VERSION);
      LOG_MESSAGE (PLID, "RACC: meta-interface version mismatch (metamod: %s, RACC: %s)", ifvers, META_INTERFACE_VERSION);

      // if plugin has later interface version, it's incompatible (update metamod)
      sscanf (ifvers, "%d:%d", &mmajor, &mminor);
      sscanf (META_INTERFACE_VERSION, "%d:%d", &pmajor, &pminor);

      if ((pmajor > mmajor) || ((pmajor == mmajor) && (pminor > mminor)))
      {
         LOG_CONSOLE (PLID, "metamod version is too old for this plugin; update metamod");
         LOG_ERROR (PLID, "metamod version is too old for this plugin; update metamod");
         return (FALSE);
      }

      // if plugin has older major interface version, it's incompatible (update plugin)
      else if (pmajor < mmajor)
      {
         LOG_CONSOLE (PLID, "metamod version is incompatible with this plugin; please find a newer version of this plugin");
         LOG_ERROR (PLID, "metamod version is incompatible with this plugin; please find a newer version of this plugin");
         return (FALSE);
      }
   }

   return (TRUE); // tell metamod this plugin looks safe
}


C_DLLEXPORT int Meta_Attach (PLUG_LOADTIME now, META_FUNCTIONS *pFunctionTable, meta_globals_t *pMGlobals, gamedll_funcs_t *pGamedllFuncs)
{
   // this function is called when metamod attempts to load the plugin. Since it's the place
   // where we can tell if the plugin will be allowed to run or not, we wait until here to make
   // our initialization stuff, like registering CVARs and dedicated server commands.

   // are we allowed to load this plugin now ?
   if (now > PT_CHANGELEVEL)
   {
      LOG_CONSOLE (PLID, "RACC: plugin NOT attaching (can't load plugin right now)");
      LOG_ERROR (PLID, "RACC: plugin NOT attaching (can't load plugin right now)");
      return (FALSE); // returning FALSE prevents metamod from attaching this plugin
   }

   // keep track of the pointers to engine function tables metamod gives us
   gpMetaGlobals = pMGlobals;
   memcpy (pFunctionTable, &gMetaFunctionTable, sizeof (META_FUNCTIONS));
   gpGamedllFuncs = pGamedllFuncs;

   // print a message to notify about plugin attaching
   LOG_CONSOLE (PLID, "RACC: plugin attaching");
   LOG_MESSAGE (PLID, "RACC: plugin attaching");

   // ask the engine to register the server commands this plugin uses
   REG_SVR_COMMAND ("racc", ServerCommand);

   DllAttaching (); // do the stuff we should do when the DLL is attaching

   return (TRUE); // returning TRUE enables metamod to attach this plugin
}


C_DLLEXPORT int Meta_Detach (PLUG_LOADTIME now, PL_UNLOAD_REASON reason)
{
   // this function is called when metamod unloads the plugin. A basic check is made in order
   // to prevent unloading the plugin if its processing should not be interrupted.

   // is metamod allowed to unload the plugin ?
   if ((now > PT_ANYTIME) && (reason != PNL_CMD_FORCED))
   {
      LOG_CONSOLE (PLID, "RACC: plugin NOT detaching (can't unload plugin right now)");
      LOG_ERROR (PLID, "RACC: plugin NOT detaching (can't unload plugin right now)");
      return (FALSE); // returning FALSE prevents metamod from unloading this plugin
   }

   DllDetaching (); // do the stuff we should do when the DLL is detaching

   return (TRUE); // returning TRUE enables metamod to unload this plugin
}


/////////////////////
// game DLL interface


void WINAPI GiveFnptrsToDll (enginefuncs_t* pengfuncsFromEngine, globalvars_t *pGlobals)
{
   // get the engine functions and global variables from the engine...

   memcpy (&g_engfuncs, pengfuncsFromEngine, sizeof (enginefuncs_t));
   gpGlobals = pGlobals;
}


int Spawn (edict_t *pent)
{
   // this function asks the game DLL to spawn (i.e, give a physical existence in the virtual
   // world, in other words to 'display') the entity pointed to by pent in the game. The Spawn()
   // function is one of the functions any entity is supposed to have in the game DLL, and any
   // MOD is supposed to implement one for each of its entities.

   // only process bots if we are in multiplayer mode
   if (server.is_multiplayer && !DebugLevel.is_paused)
   {
      if (strcmp (STRING (pent->v.classname), "worldspawn") == 0)
      {
         server.just_booted = TRUE; // raise a flag to tell this server has just booted
         pWorldEntity = pent; // keep track of the world entity when it spawns (it's spawned 1st)

         // reset the mission indicators
         mission.bomb = BOMB_NONE;
         mission.vip = VIP_NONE;
         mission.hostage_count = 0;

         PrecacheStuff (); // precache miscellaneous stuff we need
      }

      else if ((strncmp (STRING (pent->v.classname), "func_bomb", 9) == 0)
               || (strncmp (STRING (pent->v.classname), "info_bomb", 9) == 0))
         mission.bomb = BOMB_CARRIED; // remember when it is a bomb map

      else if (strcmp (STRING (pent->v.classname), "hostage_entity") == 0)
      {
         mission.hostages[mission.hostage_count] = HOSTAGE_ATSTART; // reset hostage states
         mission.hostage_count++; // there is one hostage more on this map
      }

      else if (strncmp (STRING (pent->v.classname), "func_door", 9) == 0)
         SpawnDoor (pent); // save doors origins
   }

   RETURN_META_VALUE (MRES_IGNORED, 0);
}


int Spawn_Post (edict_t *pent)
{
   // solves the bots unable to see through certain types of glass bug (engine bug).

   if (pent->v.rendermode == kRenderTransTexture)
      pent->v.flags &= ~FL_WORLDBRUSH; // clear the FL_WORLDBRUSH flag out of transparent ents

   RETURN_META_VALUE (MRES_IGNORED, 0);
}


void StartFrame (void)
{
   // this function starts a video frame. It is called once per video frame by the engine. If
   // you run Half-Life at 90 fps, this function will then be called 90 times per second. By
   // placing a hook on it, we have a good place to do things that should be done continuously
   // during the game, for example making the bots think (yes, because no Think() function exists
   // for the bots by the MOD side, remember). Also here we have control on the bot population,
   // for example if a new player joins the server, we should disconnect a bot, and if the
   // player population decreases, we should fill the server with other bots.

   int client_index;
   bool is_valid_client;
   player_t *pPlayer;
   edict_t *pPlayerEdict;

   // only process bots if we are in multiplayer mode
   if (server.is_multiplayer && !DebugLevel.is_paused)
   {
      // before anything, let's see if a new server has just booted up...
      if (server.just_booted)
         TheServerHasJustStarted (); // do the stuff we must do each time a server starts

      // see what time it is.
      server.time = ProcessTime () - server.start_time;

      // first off, check who has connected and who has disconnected since last frame.
      // NOTE THAT THE ORDER IN WHICH WE DO THINGS IS IMPORTANT! We don't want to reset the
      // connection name for disconnecting clients BEFORE they are recorded as disconnected,
      // and we don't want to set it for connecting clients AFTER they are being recorded as
      // connected as well!!
      for (client_index = 0; client_index < server.max_clients; client_index++)
      {
         pPlayer = &players[client_index]; // quick access to player
         pPlayerEdict = INDEXENT (client_index + 1); // attempt to bind this player's entity

         // now see if it's a valid entity or not (only if these conditions are met)
         is_valid_client = (!FNullEnt (pPlayerEdict) && (pPlayerEdict->v.flags & FL_CLIENT)
                            && (STRING (pPlayerEdict->v.netname) != NULL)
                            && (STRING (pPlayerEdict->v.netname)[0] != 0));

         // check if this player has just left the game.
         // is his edict NOT valid anymore AND was this client recorded as connected ?
         if (!is_valid_client && pPlayer->is_connected)
         {
            APlayerHasDisconnected (pPlayer); // then this client just disconnected
            pPlayer->pEntity = NULL; // clear his entity pointer

            // was this player the listenserver client ?
            if (pListenserverPlayer == pPlayer)
               pListenserverPlayer = NULL; // then forget about the listen server client pointer
         }

         // is he a valid client ?
         if (is_valid_client)
         {
            // save away the old values
            memcpy (&players[client_index].prev, &players[client_index], sizeof (player_prev_t));

            // store away some useful information about this client
            pPlayer->is_alive = IsAlive (pPlayerEdict); // see if he's alive or not
            strcpy (pPlayer->connection_name, STRING (pPlayerEdict->v.netname)); // remember his name
            strcpy (pPlayer->model, INFOKEY_VALUE (GET_INFOKEYBUFFER (pPlayerEdict), "model")); // remember his skin
            strcpy (pPlayer->weapon_model, STRING (pPlayerEdict->v.weaponmodel)); // weapon model
            pPlayer->v_origin = OriginOf (pPlayerEdict);
            pPlayer->angles = WrapAngles (pPlayerEdict->v.angles);
            pPlayer->v_velocity = pPlayerEdict->v.velocity;
            pPlayer->health = pPlayerEdict->v.health;
            pPlayer->max_health = 100;
            pPlayer->armor = pPlayerEdict->v.armorvalue;
            pPlayer->max_armor = 100;
            pPlayer->v_eyeposition = GetGunPosition (pPlayerEdict);
            pPlayer->v_angle = WrapAngles (pPlayerEdict->v.v_angle);
            BuildPlayerReferential (pPlayerEdict->v.v_angle, pPlayer);
            pPlayer->environment = EnvironmentOf (pPlayerEdict);
            pPlayer->input_buttons = InputButtonsOf (pPlayerEdict);
            pPlayer->has_just_fallen = ((FNullEnt (pPlayerEdict->v.dmg_inflictor) || (pPlayerEdict->v.dmg_inflictor == pWorldEntity))
                                        && (pPlayerEdict->v.punchangle != g_vecZero)
                                        && (pPlayer->prev.environment == ENVIRONMENT_MIDAIR));
         }

         // check if this player has just joined the game.
         // is his edict valid AND was this client NOT recorded as connected ?
         if (is_valid_client && !pPlayer->is_connected)
         {
            APlayerHasConnected (pPlayer); // then it means this client just connected
            pPlayer->pEntity = pPlayerEdict; // save this player's edict

            // is this client the listen server client ? (first real client to connect)
            if ((pListenserverPlayer == NULL) && !pPlayer->is_racc_bot)
               pListenserverPlayer = pPlayer; // save the listenserver client pointer
         }
      }

      // also, check if the listen server client is spectating someone (debug feature). We must
      // do this in a separate loop because we need that all the player's origins be fixed
      // before comparing them (which is done in the previous loop).
      if (IsValidPlayer (pListenserverPlayer))
         for (client_index = 0; client_index < server.max_clients; client_index++)
            if ((pListenserverPlayer->pEntity->v.flags & FL_SPECTATOR)
                && (abs (players[client_index].v_origin.x - pListenserverPlayer->v_origin.x) < 16)
                && (abs (players[client_index].v_origin.y - pListenserverPlayer->v_origin.y) < 16)
                && (abs (players[client_index].v_origin.z - pListenserverPlayer->v_origin.z) < 16))
               players[client_index].is_watched = TRUE;
            else
               players[client_index].is_watched = FALSE;
   
      // another thing here is, when we receive the "end of mission" signal, it's time to reset
      // the mission parameters by hand, so that the bots are made aware that they begin a new
      // mission. We also force a call of BotReset() on each so as to get rid of any trailing
      // task they might have. IMPORTANT: the end of mission flag is to be reset at the VERY END
      // of the frame, so as to have one complete frame elapse without any computation. For
      // obvious reasons, this part must happen BEFORE the bots are made to think.
      if (mission.finished)
      {
         mission.start_time = server.time + atof (CVAR_GET_STRING ("mp_freezetime")); // start time
         if (mission.bomb != BOMB_NONE)
            mission.bomb = BOMB_CARRIED; // reset the bomb state on bomb maps

         // cycle through all bot slots, reset them and back their brain up to disk
         for (client_index = 0; client_index < server.max_clients; client_index++)
         {
            pPlayer = &players[client_index]; // quick access to player

            if (!IsValidPlayer (pPlayer) || !pPlayer->is_racc_bot)
               continue; // skip invalid players and real clients

            BotReset (pPlayer); // reset this bot for the new round
            BotNavSaveBrain (pPlayer); // save our bot's nav brain to disk
            BotHALSaveBrain (pPlayer); // save our bot's HAL brain to disk
         }
      }

      // run the bot manager, and check on various intervals if a bot needs to be created or
      // removed. If time to, if there are less than the maximum number of bots and the
      // automatic server traffic feature has been requested, create a bot and delay the next
      // check. Else if there are too many bots, kick one out so as to allow humans to join.
      if ((server.bot_check_time > 0) && (server.bot_check_time < server.time))
      {
         // can we add a bot AND are there less bots than the maximum # of bots ?
         if (server.is_autofill && (player_count < server.max_clients - 1)
             && ((server.max_bots == -1) || (bot_count < server.max_bots)))
         {
            BotCreate (); // add a bot
            if (server.is_internetmode && (server.time > 60.0))
               server.bot_check_time = server.time + RandomFloat (10.0, 30.0); // delay a while
            else
               server.bot_check_time = server.time + 0.5; // delay half a second
         }

         // else if there are too many bots disconnect one from the server
         else if (((server.max_bots != -1) && (bot_count > server.max_bots))
                  || ((player_count == server.max_clients) && (bot_count > server.min_bots)))
         {
            // cycle through all bot slots
            for (client_index = 0; client_index < server.max_clients; client_index++)
            {
               pPlayer = &players[client_index]; // quick access to player

               if (!IsValidPlayer (pPlayer) || !pPlayer->is_racc_bot)
                  continue; // discard invalid players and real clients

               pPlayer->Bot.quit_game_time = server.time; // mark this bot for disconnection
               break;
            }

            server.bot_check_time = server.time + 0.5; // should delay a while
         }
      }

      // now we can let the AI do its task, and call each step of the Think() trilogy - that is,
      // sense, think, act.
      for (client_index = 0; client_index < server.max_clients; client_index++)
      {
         pPlayer = &players[client_index]; // quick access to player

         if (!IsValidPlayer (pPlayer) || !pPlayer->is_racc_bot)
            continue; // skip invalid players and real clients

         // make the bot think
         BotSense (pPlayer);
         BotThink (pPlayer);
         BotAct (pPlayer);
      }

      // last off, we check whether our players have moved onto another face, and if so, update
      // the bot's automatic pathbuilder. Also it's time to dispatch client sounds to the bot's
      // ears. It's also time to decide whether or not to send a welcome message to new clients.
      // Then since we might have to compare something in each player's entity variables to its
      // last state one day, it's always useful to keep a local copy of the player's entvars.
      for (client_index = 0; client_index < server.max_clients; client_index++)
      {
         pPlayer = &players[client_index]; // quick access to player

         if (!IsValidPlayer (pPlayer))
            continue; // discard invalid players

         // is this player alive AND the mission is still running ?
         if (pPlayer->is_alive && !mission.finished)
         {
            ShowTheWayAroundToBots (pPlayer); // have the bots watch out for this player's movement
            PlayClientSoundsForBots (pPlayer); // see if this client needs to dispatch sounds to the bot's ears

            // is it time to send the welcome message to this client ? (don't send if not alive yet)
            if ((pPlayer->welcome_time > 0) && (pPlayer->welcome_time < server.time))
            {
               SendWelcomeMessage (pPlayer); // send him the welcome message
               pPlayer->welcome_time = 0; // don't do it twice
            }
         }
         else
         {
            // else player is dead or respawning, don't allow bots to track a ghost :)
            pPlayer->pFaceAtFeet = NULL;
            pPlayer->face_reachability = 0;
         }
      }

      // have we been requested to issue a server command ?
      if (server.server_command[0] != 0)
         SERVER_COMMAND (server.server_command); // issue it
      server.server_command[0] = 0; // and reset the server command string

      EstimateNextFrameDuration (); // estimate how long the next frame will last
      server.previous_time = server.time; // previous time gets updated at each StartFrame

      // did the current mission end during the current frame ?
      if (mission.finished)
         mission.finished = FALSE; // reset 'end of round' signal
   }

   RETURN_META (MRES_IGNORED);
}


void PlayerPreThink (edict_t *pPlayer)
{
   // this whole function hook can be deleted in release

   int index;

   // for "botcontrol" command: don't let listen server client DLL intercept spectator keys
   if ((pListenserverPlayer != NULL) && (pPlayer == pListenserverPlayer->pEntity))
      for (index = 0; index < server.max_clients; index++)
         if (players[index].is_alive && players[index].is_racc_bot
             && players[index].is_watched && players[index].Bot.is_controlled)
            RETURN_META (MRES_SUPERCEDE);

   RETURN_META (MRES_IGNORED);
}


C_DLLEXPORT int GetEntityAPI2 (DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion)
{
   // This exported function declares to metamod which functions from the game DLL interface our
   // DLL will use, by linking each used slot in the gFunctionTable structure to the address
   // of our actual functions. This way, when metamod will bind any function in any slot of
   // gFunctionTable, it will land directly in our functions here, just by following the pointer.

   // These functions will be called by metamod BEFORE calling the actual game DLL function.

   static DLL_FUNCTIONS gFunctionTable;

   // keep track of pointers to the functions we use in the game DLL API
   gFunctionTable.pfnSpawn = Spawn;
   gFunctionTable.pfnStartFrame = StartFrame;
   gFunctionTable.pfnPlayerPreThink = PlayerPreThink; // for "botcontrol" debug (can be removed)

   // copy the whole table for metamod to know which functions we are using here
   memcpy (pFunctionTable, &gFunctionTable, sizeof (DLL_FUNCTIONS));
   return (TRUE); // alright
}


C_DLLEXPORT int GetEntityAPI2_Post (DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion)
{
   // This exported function declares to metamod which functions from the game DLL interface our
   // DLL will use, by linking each used slot in the gFunctionTable structure to the address
   // of our actual functions. This way, when metamod will bind any function in any slot of
   // gFunctionTable, it will land directly in our functions here, just by following the pointer.

   // These functions will be called by metamod AFTER calling the actual game DLL function.

   static DLL_FUNCTIONS gFunctionTable;

   // keep track of pointers to the functions we use in the game DLL API
   gFunctionTable.pfnSpawn = Spawn_Post;

   // copy the whole table for metamod to know which functions we are using here
   memcpy (pFunctionTable, &gFunctionTable, sizeof (DLL_FUNCTIONS));
   return (TRUE); // alright
}


///////////////////
// engine interface


void SetOrigin (edict_t *e, const float *rgflOrigin)
{
   // this function tells the engine to change the origin (vector location in the virtual world)
   // of entity e to the vector location rgflOrigin (which must point to a vector). Such a
   // function is called, for example, by the engine to set the initial placement of a particular
   // entity, such as for putting players at their spawn points when they respawn.

   // only process bots if we are in multiplayer mode
   if (server.is_multiplayer && !DebugLevel.is_paused)
   {
      int entity_index = ENTINDEX (e) - 1; // get entity index

      // if it's a player respawn, have the bots stop monitoring him
      if (mission.finished && (entity_index >= 0) && (entity_index < server.max_clients))
      {
         players[entity_index].pFaceAtFeet = NULL; // forget where this player was walking
         players[entity_index].face_reachability = 0;
      }
   }

   RETURN_META (MRES_IGNORED);
}


void EmitSound (edict_t *entity, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch)
{
   // this function tells the engine that the entity pointed to by "entity", is emitting a sound
   // which filename is "sample", at level "channel" (CHAN_VOICE, etc...), with "volume" as
   // loudness multiplicator (normal volume VOL_NORM is 1.0), with a pitch of "pitch" (normal
   // pitch PITCH_NORM is 100.0), and that this sound has to be attenuated by distance in air
   // according to the value of "attenuation" (normal attenuation ATTN_NORM is 0.8 ; ATTN_NONE
   // means no attenuation with distance). Optionally flags "fFlags" can be passed, which I don't
   // know the heck of the purpose. After we tell the engine to emit the sound, we have to call
   // DispatchSound() to bring the sound to the ears of the bots. Since bots have no client DLL
   // to handle this for them, such a job has to be done manually.

   // only process bots if we are in multiplayer mode
   if (server.is_multiplayer && !DebugLevel.is_paused)
      DispatchSound (sample, OriginOf (entity), volume, attenuation); // dispatch that sound

   RETURN_META (MRES_IGNORED);
}


void EmitAmbientSound (edict_t *entity, float *pos, const char *samp, float vol, float attenuation, int fFlags, int pitch)
{
   // this function tells the engine that the entity pointed to by "entity", is emitting a sound
   // which filename is "samp", at position "pos" (pos must point to a vector), with "vol" as
   // loudness multiplicator (normal volume VOL_NORM is 1.0), with a pitch of "pitch" (normal
   // pitch PITCH_NORM is 100.0), and that this sound has to be attenuated by distance in air
   // according to the value of "attenuation" (normal attenuation ATTN_NORM is 0.8 ; ATTN_NONE
   // means no attenuation with distance). Optionally flags "fFlags" can be passed, which I don't
   // know the heck of the purpose. After we tell the engine to emit the sound, we have to call
   // DispatchSound() to bring the sound to the ears of the bots. Since bots have no client DLL
   // to handle this for them, such a job has to be done manually.

   // only process bots if we are in multiplayer mode
   if (server.is_multiplayer && !DebugLevel.is_paused)
      DispatchSound (samp, pos, vol, attenuation); // dispatch that sound for the bots to hear it

   RETURN_META (MRES_IGNORED);
}


void TraceLine_Post (const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr)
{
   // this function is one of the most important of the game. It is a test line. By calling
   // TraceLine the engine is told to check the direct reachability between two points, and to
   // return the answer in a special dedicated structure. In detail, it is about starting from
   // spatial location v1, going in direct line to v2 (v1 and v2 point to vectors), ignoring or
   // not entities that are monsters along the way (depending whether fNoMonsters is set to
   // enumerated type ignore_monsters or dont_ignore_monsters), ignoring if needed the entity it
   // starts within, pentToSkip, and returning the results of the trace in structure ptr, which
   // results may contain lots of useful info. The most important is the flFraction field, a
   // fraction of 1 proportionnal to the distance that has been successfully reached without
   // obstacles. I.e, if flFraction is 0.5, the trace has covered half the distance successfully.

   player_t *pPlayer;

   // if this traceline was fired by a valid player AND this player is alive...
   if (!FNullEnt (pentToSkip) && (pentToSkip->v.flags & FL_CLIENT) && (pentToSkip->v.deadflag == DEAD_NO))
   {
      pPlayer = &players[ENTINDEX (pentToSkip) - 1]; // quick access to player

      // copy away its trace results
      pPlayer->tr.fraction = ptr->flFraction;
      pPlayer->tr.pHit = ptr->pHit;
      pPlayer->tr.v_endposition = ptr->vecEndPos;
      pPlayer->tr.v_normal = ptr->vecPlaneNormal;
   }

   RETURN_META (MRES_IGNORED);
}


void MessageBegin (int msg_dest, int msg_type, const float *pOrigin, edict_t *ed)
{
   // this function is part of the message catcher. It is called by the game DLL to tell the
   // engine to prepare a header for a network packet whose data will follow. We hook it so as
   // to record in the message structure the header of the message which is about to be sent,
   // according to the following parameters : msg_dest (destination of message, everyone or
   // individual), type of message (registered by game DLL at start), optional vector origin
   // (for world events I assume), and the destination player entity pointed to by ed.

   // ANY message that passes through this DLL is suspicious, that's what I say! :)

   // first reset the message catcher structures
   memset (&message_header, 0, sizeof (message_header));
   memset (message, 0, sizeof (message));
   message_size = 0; // and set suspicious message size to zero

   // is it a broadcasted message ?
   if ((msg_dest == MSG_ALL) || (msg_dest == MSG_BROADCAST))
      message_header.is_broadcasted = TRUE; // if so, remember it (all players will get it at once)

   // is this message directed towards a player ?
   if (!FNullEnt (ed) && (ed->v.flags & FL_CLIENT))
      message_header.player_index = ENTINDEX (ed) - 1; // if so, keep track of the player index
   else
      message_header.player_index = -1; // else invalidate the player index

   message_header.message_type = msg_type; // remember what type of message it is

   RETURN_META (MRES_IGNORED);
}


void MessageEnd_Post (void)
{
   // this function is part of the message catcher. The termination of a network message has been
   // triggered, so we call the appropriate function for examining the message queue in order
   // to decide whether, or not, this message will be allowed to pass.

   ExamineNetworkMessage (); // now we've received the full message, check it before sending it

   RETURN_META (MRES_IGNORED);
}


void WriteByte (int iValue)
{
   // this function is part of the message catcher. It records in the message queue a packet
   // sent by the game DLL whose type is a byte.

   message[message_size].packet_type = PACKET_BYTE; // remember packet type
   message[message_size].iValuePassed = iValue; // remember value passed
   message_size++; // the suspicious message has gained one packet in size

   RETURN_META (MRES_IGNORED);
}


void WriteChar (int iValue)
{
   // this function is part of the message catcher. It records in the message queue a packet
   // sent by the game DLL whose type is a character.

   message[message_size].packet_type = PACKET_CHAR; // remember packet type
   message[message_size].iValuePassed = iValue; // remember value passed
   message_size++; // the suspicious message has gained one packet in size

   RETURN_META (MRES_IGNORED);
}


void WriteShort (int iValue)
{
   // this function is part of the message catcher. It records in the message queue a packet
   // sent by the game DLL whose type is a short integer.

   message[message_size].packet_type = PACKET_SHORT; // remember packet type
   message[message_size].iValuePassed = iValue; // remember value passed
   message_size++; // the suspicious message has gained one packet in size

   RETURN_META (MRES_IGNORED);
}


void WriteLong (int iValue)
{
   // this function is part of the message catcher. It records in the message queue a packet
   // sent by the game DLL whose type is a long integer.

   message[message_size].packet_type = PACKET_LONG; // remember packet type
   message[message_size].iValuePassed = iValue; // remember value passed
   message_size++; // the suspicious message has gained one packet in size

   RETURN_META (MRES_IGNORED);
}


void WriteAngle (float flValue)
{
   // this function is part of the message catcher. It records in the message queue a packet
   // sent by the game DLL whose type is a floating-point angle.

   message[message_size].packet_type = PACKET_ANGLE; // remember packet type
   message[message_size].flValuePassed = flValue; // remember value passed
   message_size++; // the suspicious message has gained one packet in size

   RETURN_META (MRES_IGNORED);
}


void WriteCoord (float flValue)
{
   // this function is part of the message catcher. It records in the message queue a packet
   // sent by the game DLL whose type is a floating-point coordinate.

   message[message_size].packet_type = PACKET_COORD; // remember packet type
   message[message_size].flValuePassed = flValue; // remember value passed
   message_size++; // the suspicious message has gained one packet in size

   RETURN_META (MRES_IGNORED);
}


void WriteString (const char *sz)
{
   // this function is part of the message catcher. It records in the message queue a packet
   // sent by the game DLL whose type is a character string.

   message[message_size].packet_type = PACKET_STRING; // remember packet type
   message[message_size].szValuePassed = sz; // remember value passed
   message_size++; // the suspicious message has gained one packet in size

   RETURN_META (MRES_IGNORED);
}


void WriteEntity (int iValue)
{
   // this function is part of the message catcher. It records in the message queue a packet
   // sent by the game DLL whose type is Entity.

   message[message_size].packet_type = PACKET_ENTITY; // remember packet type
   message[message_size].iValuePassed = iValue; // remember value passed
   message_size++; // the suspicious message has gained one packet in size

   RETURN_META (MRES_IGNORED);
}


const char *Cmd_Args (void)
{
   // this function returns a pointer to the whole current client command string. Since bots
   // have no client DLL and we may want a bot to execute a client command, we had to implement
   // a g_argv string in the bot DLL for holding the bots' commands, and also keep track of the
   // argument count. Hence this hook not to let the engine ask an unexistent client DLL for a
   // command we are holding here. Of course, real clients commands are still retrieved the
   // normal way, by asking the engine.

   if (isFakeClientCommand)
   {
      // is it a "say" or "say_team" client command ? (engine bug fix)
      if (strncmp ("say ", g_argv, 4) == 0)
         RETURN_META_VALUE (MRES_SUPERCEDE, &g_argv[0] + 4); // skip the "say" bot client command (bug in HL engine)
      else if (strncmp ("say_team ", g_argv, 9) == 0)
         RETURN_META_VALUE (MRES_SUPERCEDE, &g_argv[0] + 9); // skip the "say_team" bot client command (bug in HL engine)

      RETURN_META_VALUE (MRES_SUPERCEDE, &g_argv[0]); // else return the whole bot client command string we know
   }

   RETURN_META_VALUE (MRES_IGNORED, 0);
}


const char *Cmd_Argv (int argc)
{
   // this function returns a pointer to a certain argument of the current client command. Since
   // bots have no client DLL and we may want a bot to execute a client command, we had to
   // implement a g_argv string in the bot DLL for holding the bots' commands, and also keep
   // track of the argument count. Hence this hook not to let the engine ask an unexistent client
   // DLL for a command we are holding here. Of course, real clients commands are still retrieved
   // the normal way, by asking the engine.

   if (isFakeClientCommand)
      RETURN_META_VALUE (MRES_SUPERCEDE, GetField (g_argv, argc)); // returns the wanted argument

   RETURN_META_VALUE (MRES_IGNORED, 0);
}


int Cmd_Argc (void)
{
   // this function returns the number of arguments the current client command string has. Since
   // bots have no client DLL and we may want a bot to execute a client command, we had to
   // implement a g_argv string in the bot DLL for holding the bots' commands, and also keep
   // track of the argument count. Hence this hook not to let the engine ask an unexistent client
   // DLL for a command we are holding here. Of course, real clients commands are still retrieved
   // the normal way, by asking the engine.

   if (isFakeClientCommand)
      RETURN_META_VALUE (MRES_SUPERCEDE, fake_arg_count); // return the argument count

   RETURN_META_VALUE (MRES_IGNORED, 0);
}


C_DLLEXPORT int GetEngineFunctions (enginefuncs_t *pengfuncsFromEngine, int *interfaceVersion)
{
   // This exported function declares to metamod which functions from the engine interface our
   // DLL will use, by linking each used slot in the meta_engfuncs structure to the address of
   // our actual functions. This way, when metamod will bind any function in any slot of meta_
   // engfuncs, it will land directly in our functions here, just by following the pointer.

   // These functions will be called by metamod BEFORE calling the actual engine function.

   static enginefuncs_t meta_engfuncs;

   // keep track of pointers to the functions we use in the engine API
   meta_engfuncs.pfnSetOrigin = SetOrigin;
   meta_engfuncs.pfnEmitSound = EmitSound;
   meta_engfuncs.pfnEmitAmbientSound = EmitAmbientSound;
   meta_engfuncs.pfnMessageBegin = MessageBegin;
   meta_engfuncs.pfnWriteByte = WriteByte;
   meta_engfuncs.pfnWriteChar = WriteChar;
   meta_engfuncs.pfnWriteShort = WriteShort;
   meta_engfuncs.pfnWriteLong = WriteLong;
   meta_engfuncs.pfnWriteAngle = WriteAngle;
   meta_engfuncs.pfnWriteCoord = WriteCoord;
   meta_engfuncs.pfnWriteString = WriteString;
   meta_engfuncs.pfnWriteEntity = WriteEntity;
   meta_engfuncs.pfnCmd_Args = Cmd_Args;
   meta_engfuncs.pfnCmd_Argv = Cmd_Argv;
   meta_engfuncs.pfnCmd_Argc = Cmd_Argc;

   memcpy (pengfuncsFromEngine, &meta_engfuncs, sizeof (enginefuncs_t));
   return (TRUE);
}


C_DLLEXPORT int GetEngineFunctions_Post (enginefuncs_t *pengfuncsFromEngine, int *interfaceVersion)
{
   // This exported function declares to metamod which functions from the engine interface our
   // DLL will use, by linking each used slot in the meta_engfuncs_Post structure to the address
   // of our actual functions. This way, when metamod will bind any function in any slot of meta_
   // engfuncs_Post, it will land directly in our functions here, just by following the pointer.

   // These functions will be called by metamod AFTER calling the actual engine function.

   static enginefuncs_t meta_engfuncs;

   // keep track of pointers to the functions we use in the engine API
   meta_engfuncs.pfnTraceLine = TraceLine_Post;
   meta_engfuncs.pfnMessageEnd = MessageEnd_Post;

   // copy the whole table for metamod to know which functions we are using here
   memcpy (pengfuncsFromEngine, &meta_engfuncs, sizeof (enginefuncs_t));
   return (TRUE); // alright
}
