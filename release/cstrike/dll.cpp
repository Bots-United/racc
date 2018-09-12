// RACC - AI development project for first-person shooter games derivated from Valve's Half-Life
// (http://www.racc-ai.com/)
//
// The game to engine interfacing code is based on the work done by Jeffrey 'Botman' Broome
// (http://planethalflife.com/botman/)
//
// This project is partially based on the work done by Eric Bieschke in his BSDbot
// (http://gamershomepage.com/csbot/)
//
// This project is partially based on the work done by Brendan 'Spyro' McCarthy in his ODD Bot
// (http://oddbot.hlfusion.com/)
//
// This project is partially based on the work done by Alistair 'eLiTe' Stewart in his TEAMbot
// (http://www.planethalflife.com/teambot/)
//
// This project is partially based on the work done by Johannes '@$3.1415rin' Lampel in his JoeBot
// (http://www.joebot.net/)
//
// CSTRIKE version
//
// dll.cpp
//

#include "racc.h"


DECLARE_CUSTOM_EXPORTS_ARRAY

dll_t h_Library = NULL;
GETENTITYAPI other_GetEntityAPI = NULL;
GETNEWDLLFUNCTIONS other_GetNewDLLFunctions = NULL;
GIVEFNPTRSTODLL other_GiveFnptrsToDll = NULL;
DLL_FUNCTIONS other_gFunctionTable;
enginefuncs_t g_engfuncs;
globalvars_t  *gpGlobals;
char mod_name[256];
char map_name[256];
map_t map;
char *g_argv;
bool isFakeClientCommand = FALSE;
int fake_arg_count = 0;
char arg[256];
bool is_dedicated_server = FALSE;
bool is_multiplayer_game = FALSE;
bool welcome_sent = FALSE;
float bot_check_time = 0;
entity_t *pListenserverEntity = NULL;
round_t round;
sound_t sounds[MAX_SOUNDS + MAX_LOCAL_SOUNDS];
int sound_count = 0;
texture_t textures[MAX_TEXTURES];
int texture_count = 0;
bot_personality_t bot_personalities[MAX_BOT_PERSONALITIES];
int personality_count = 0;
player_t players[MAX_CLIENTS_SUPPORTED_BY_ENGINE];
int player_count = 0;
bot_t bots[MAX_CLIENTS_SUPPORTED_BY_ENGINE];
int bot_count = 0;
int msec_method = METHOD_RICH;
weapon_t weapon_defs[MAX_WEAPONS];
debug_level_t DebugLevel;


DLL_ENTRYPOINT
{
   // dynamic library entry point, can be used for uninitialization stuff. Since it is very OS
   // specific, we are using handy #define's to declare the function name and arguments given
   // the operating system for which the library is compiled. We use it only as "exit point".

   // are we detaching the DLL ?
   if (DLL_DETACHING)
      FreeAllTheStuff (); // free everything that's freeable

   RETURN_ENTRYPOINT; // the return data type is OS specific too
}


DLL_GIVEFNPTRSTODLL DLLEXPORT GiveFnptrsToDll (enginefuncs_t* pengfuncsFromEngine, globalvars_t *pGlobals)
{
   // this is the very first function that is called in the game DLL by the engine. Its purpose
   // is to set the functions interfacing up, by exchanging the pengfuncsFromEngine function list
   // along with a pointer to the engine's global varibales structure pGlobals, with the game
   // DLL. We can there decide if we want to load the normal game DLL just behind our bot DLL,
   // or any other game DLL that is present, such as Will Day's metamod. Also, since there is
   // a known bug on Win32 platforms that prevent hook DLLs (such as our bot DLL) to be used in
   // single player games (because they don't export all the stuff they should), we may need to
   // build our own array of exported symbols from the actual game DLL in order to use it as
   // such if necessary. Nothing really bot-related is done in this function. The actual bot
   // initialization stuff will be done later, when we'll be certain to have a multiplayer game.

   // does a metamod library exist for this MOD ?
   if (FileExists (METAMOD_LIBRARY_NAME))
      h_Library = LoadDynamicLibrary (METAMOD_LIBRARY_NAME);// load metamod's dynamic library
   else
      h_Library = LoadDynamicLibrary (MOD_LIBRARY_NAME); // load MOD's own dynamic library

   LoadSymbols (MOD_LIBRARY_NAME); // build our own array of exported symbols table if necessary

   // failed to load library ?
   if (h_Library == NULL)
      TerminateOnError ("Fatal - MOD dynamic library not found\n"); // then terminate with error

   // get the engine functions from the engine...
   memcpy (&g_engfuncs, pengfuncsFromEngine, sizeof (enginefuncs_t));

   gpGlobals = pGlobals; // keep track of the server's global variables structure

   // locate GetEntityAPI (), GetNewDLLFunctions () and GiveFnptrsToDll () functions in game DLL
   other_GetEntityAPI = (GETENTITYAPI) GetSymbolInDynamicLibrary (h_Library, "GetEntityAPI");
   other_GetNewDLLFunctions = (GETNEWDLLFUNCTIONS) GetSymbolInDynamicLibrary (h_Library, "GetNewDLLFunctions");
   other_GiveFnptrsToDll = (GIVEFNPTRSTODLL) GetSymbolInDynamicLibrary (h_Library, "GiveFnptrsToDll");

   // build the interface to engine DLL
   pengfuncsFromEngine->pfnPrecacheModel = pfnPrecacheModel;
   pengfuncsFromEngine->pfnPrecacheSound = pfnPrecacheSound;
   pengfuncsFromEngine->pfnSetModel = pfnSetModel;
   pengfuncsFromEngine->pfnModelIndex = pfnModelIndex;
   pengfuncsFromEngine->pfnModelFrames = pfnModelFrames;
   pengfuncsFromEngine->pfnSetSize = pfnSetSize;
   pengfuncsFromEngine->pfnChangeLevel = pfnChangeLevel;
   pengfuncsFromEngine->pfnGetSpawnParms = pfnGetSpawnParms;
   pengfuncsFromEngine->pfnSaveSpawnParms = pfnSaveSpawnParms;
   pengfuncsFromEngine->pfnVecToYaw = pfnVecToYaw;
   pengfuncsFromEngine->pfnVecToAngles = pfnVecToAngles;
   pengfuncsFromEngine->pfnMoveToOrigin = pfnMoveToOrigin;
   pengfuncsFromEngine->pfnChangeYaw = pfnChangeYaw;
   pengfuncsFromEngine->pfnChangePitch = pfnChangePitch;
   pengfuncsFromEngine->pfnFindEntityByString = pfnFindEntityByString;
   pengfuncsFromEngine->pfnGetEntityIllum = pfnGetEntityIllum;
   pengfuncsFromEngine->pfnFindEntityInSphere = pfnFindEntityInSphere;
   pengfuncsFromEngine->pfnFindClientInPVS = pfnFindClientInPVS;
   pengfuncsFromEngine->pfnEntitiesInPVS = pfnEntitiesInPVS;
   pengfuncsFromEngine->pfnMakeVectors = pfnMakeVectors;
   pengfuncsFromEngine->pfnAngleVectors = pfnAngleVectors;
   pengfuncsFromEngine->pfnCreateEntity = pfnCreateEntity;
   pengfuncsFromEngine->pfnRemoveEntity = pfnRemoveEntity;
   pengfuncsFromEngine->pfnCreateNamedEntity = pfnCreateNamedEntity;
   pengfuncsFromEngine->pfnMakeStatic = pfnMakeStatic;
   pengfuncsFromEngine->pfnEntIsOnFloor = pfnEntIsOnFloor;
   pengfuncsFromEngine->pfnDropToFloor = pfnDropToFloor;
   pengfuncsFromEngine->pfnWalkMove = pfnWalkMove;
   pengfuncsFromEngine->pfnSetOrigin = pfnSetOrigin;
   pengfuncsFromEngine->pfnEmitSound = pfnEmitSound;
   pengfuncsFromEngine->pfnEmitAmbientSound = pfnEmitAmbientSound;
   pengfuncsFromEngine->pfnTraceLine = pfnTraceLine;
   pengfuncsFromEngine->pfnTraceToss = pfnTraceToss;
   pengfuncsFromEngine->pfnTraceMonsterHull = pfnTraceMonsterHull;
   pengfuncsFromEngine->pfnTraceHull = pfnTraceHull;
   pengfuncsFromEngine->pfnTraceModel = pfnTraceModel;
   pengfuncsFromEngine->pfnTraceTexture = pfnTraceTexture;
   pengfuncsFromEngine->pfnTraceSphere = pfnTraceSphere;
   pengfuncsFromEngine->pfnGetAimVector = pfnGetAimVector;
   pengfuncsFromEngine->pfnServerCommand = pfnServerCommand;
   pengfuncsFromEngine->pfnServerExecute = pfnServerExecute;
   pengfuncsFromEngine->pfnClientCommand = pfnClientCommand;
   pengfuncsFromEngine->pfnParticleEffect = pfnParticleEffect;
   pengfuncsFromEngine->pfnLightStyle = pfnLightStyle;
   pengfuncsFromEngine->pfnDecalIndex = pfnDecalIndex;
   pengfuncsFromEngine->pfnPointContents = pfnPointContents;
   pengfuncsFromEngine->pfnMessageBegin = pfnMessageBegin;
   pengfuncsFromEngine->pfnMessageEnd = pfnMessageEnd;
   pengfuncsFromEngine->pfnWriteByte = pfnWriteByte;
   pengfuncsFromEngine->pfnWriteChar = pfnWriteChar;
   pengfuncsFromEngine->pfnWriteShort = pfnWriteShort;
   pengfuncsFromEngine->pfnWriteLong = pfnWriteLong;
   pengfuncsFromEngine->pfnWriteAngle = pfnWriteAngle;
   pengfuncsFromEngine->pfnWriteCoord = pfnWriteCoord;
   pengfuncsFromEngine->pfnWriteString = pfnWriteString;
   pengfuncsFromEngine->pfnWriteEntity = pfnWriteEntity;
   pengfuncsFromEngine->pfnCVarRegister = pfnCVarRegister;
   pengfuncsFromEngine->pfnCVarGetFloat = pfnCVarGetFloat;
   pengfuncsFromEngine->pfnCVarGetString = pfnCVarGetString;
   pengfuncsFromEngine->pfnCVarSetFloat = pfnCVarSetFloat;
   pengfuncsFromEngine->pfnCVarSetString = pfnCVarSetString;
   pengfuncsFromEngine->pfnAlertMessage = pfnAlertMessage;
   pengfuncsFromEngine->pfnEngineFprintf = pfnEngineFprintf;
   pengfuncsFromEngine->pfnPvAllocEntPrivateData = pfnPvAllocEntPrivateData;
   pengfuncsFromEngine->pfnPvEntPrivateData = pfnPvEntPrivateData;
   pengfuncsFromEngine->pfnFreeEntPrivateData = pfnFreeEntPrivateData;
   pengfuncsFromEngine->pfnSzFromIndex = pfnSzFromIndex;
   pengfuncsFromEngine->pfnAllocString = pfnAllocString;
   pengfuncsFromEngine->pfnGetVarsOfEnt = pfnGetVarsOfEnt;
   pengfuncsFromEngine->pfnPEntityOfEntOffset = pfnPEntityOfEntOffset;
   pengfuncsFromEngine->pfnEntOffsetOfPEntity = pfnEntOffsetOfPEntity;
   pengfuncsFromEngine->pfnIndexOfEdict = pfnIndexOfEdict;
   pengfuncsFromEngine->pfnPEntityOfEntIndex = pfnPEntityOfEntIndex;
   pengfuncsFromEngine->pfnFindEntityByVars = pfnFindEntityByVars;
   pengfuncsFromEngine->pfnGetModelPtr = pfnGetModelPtr;
   pengfuncsFromEngine->pfnRegUserMsg = pfnRegUserMsg;
   pengfuncsFromEngine->pfnAnimationAutomove = pfnAnimationAutomove;
   pengfuncsFromEngine->pfnGetBonePosition = pfnGetBonePosition;
   pengfuncsFromEngine->pfnFunctionFromName = pfnFunctionFromName;
   pengfuncsFromEngine->pfnNameForFunction = pfnNameForFunction;
   pengfuncsFromEngine->pfnClientPrintf = pfnClientPrintf;
   pengfuncsFromEngine->pfnServerPrint = pfnServerPrint;
   pengfuncsFromEngine->pfnCmd_Args = pfnCmd_Args;
   pengfuncsFromEngine->pfnCmd_Argv = pfnCmd_Argv;
   pengfuncsFromEngine->pfnCmd_Argc = pfnCmd_Argc;
   pengfuncsFromEngine->pfnGetAttachment = pfnGetAttachment;
   pengfuncsFromEngine->pfnCRC32_Init = pfnCRC32_Init;
   pengfuncsFromEngine->pfnCRC32_ProcessBuffer = pfnCRC32_ProcessBuffer;
   pengfuncsFromEngine->pfnCRC32_ProcessByte = pfnCRC32_ProcessByte;
   pengfuncsFromEngine->pfnCRC32_Final = pfnCRC32_Final;
   pengfuncsFromEngine->pfnRandomLong = pfnRandomLong;
   pengfuncsFromEngine->pfnRandomFloat = pfnRandomFloat;
   pengfuncsFromEngine->pfnSetView = pfnSetView;
   pengfuncsFromEngine->pfnTime = pfnTime;
   pengfuncsFromEngine->pfnCrosshairAngle = pfnCrosshairAngle;
   pengfuncsFromEngine->pfnLoadFileForMe = pfnLoadFileForMe;
   pengfuncsFromEngine->pfnFreeFile = pfnFreeFile;
   pengfuncsFromEngine->pfnEndSection = pfnEndSection;
   pengfuncsFromEngine->pfnCompareFileTime = pfnCompareFileTime;
   pengfuncsFromEngine->pfnGetGameDir = pfnGetGameDir;
   pengfuncsFromEngine->pfnCvar_RegisterVariable = pfnCvar_RegisterVariable;
   pengfuncsFromEngine->pfnFadeClientVolume = pfnFadeClientVolume;
   pengfuncsFromEngine->pfnSetClientMaxspeed = pfnSetClientMaxspeed;
   pengfuncsFromEngine->pfnCreateFakeClient = pfnCreateFakeClient;
   pengfuncsFromEngine->pfnRunPlayerMove = pfnRunPlayerMove;
   pengfuncsFromEngine->pfnNumberOfEntities = pfnNumberOfEntities;
   pengfuncsFromEngine->pfnGetInfoKeyBuffer = pfnGetInfoKeyBuffer;
   pengfuncsFromEngine->pfnInfoKeyValue = pfnInfoKeyValue;
   pengfuncsFromEngine->pfnSetKeyValue = pfnSetKeyValue;
   pengfuncsFromEngine->pfnSetClientKeyValue = pfnSetClientKeyValue;
   pengfuncsFromEngine->pfnIsMapValid = pfnIsMapValid;
   pengfuncsFromEngine->pfnStaticDecal = pfnStaticDecal;
   pengfuncsFromEngine->pfnPrecacheGeneric = pfnPrecacheGeneric;
   pengfuncsFromEngine->pfnGetPlayerUserId = pfnGetPlayerUserId;
   pengfuncsFromEngine->pfnBuildSoundMsg = pfnBuildSoundMsg;
   pengfuncsFromEngine->pfnIsDedicatedServer = pfnIsDedicatedServer;
   pengfuncsFromEngine->pfnCVarGetPointer = pfnCVarGetPointer;
   pengfuncsFromEngine->pfnGetPlayerWONId = pfnGetPlayerWONId;
   pengfuncsFromEngine->pfnInfo_RemoveKey = pfnInfo_RemoveKey;
   pengfuncsFromEngine->pfnGetPhysicsKeyValue = pfnGetPhysicsKeyValue;
   pengfuncsFromEngine->pfnSetPhysicsKeyValue = pfnSetPhysicsKeyValue;
   pengfuncsFromEngine->pfnGetPhysicsInfoString = pfnGetPhysicsInfoString;
   pengfuncsFromEngine->pfnPrecacheEvent = pfnPrecacheEvent;
   pengfuncsFromEngine->pfnPlaybackEvent = pfnPlaybackEvent;
   pengfuncsFromEngine->pfnSetFatPVS = pfnSetFatPVS;
   pengfuncsFromEngine->pfnSetFatPAS = pfnSetFatPAS;
   pengfuncsFromEngine->pfnCheckVisibility = pfnCheckVisibility;
   pengfuncsFromEngine->pfnDeltaSetField = pfnDeltaSetField;
   pengfuncsFromEngine->pfnDeltaUnsetField = pfnDeltaUnsetField;
   pengfuncsFromEngine->pfnDeltaAddEncoder = pfnDeltaAddEncoder;
   pengfuncsFromEngine->pfnGetCurrentPlayer = pfnGetCurrentPlayer;
   pengfuncsFromEngine->pfnCanSkipPlayer = pfnCanSkipPlayer;
   pengfuncsFromEngine->pfnDeltaFindField = pfnDeltaFindField;
   pengfuncsFromEngine->pfnDeltaSetFieldByIndex = pfnDeltaSetFieldByIndex;
   pengfuncsFromEngine->pfnDeltaUnsetFieldByIndex = pfnDeltaUnsetFieldByIndex;
   pengfuncsFromEngine->pfnSetGroupMask = pfnSetGroupMask;
   pengfuncsFromEngine->pfnCreateInstancedBaseline = pfnCreateInstancedBaseline;
   pengfuncsFromEngine->pfnCvar_DirectSet = pfnCvar_DirectSet;
   pengfuncsFromEngine->pfnForceUnmodified = pfnForceUnmodified;
   pengfuncsFromEngine->pfnGetPlayerStats = pfnGetPlayerStats;
   pengfuncsFromEngine->pfnAddServerCommand = pfnAddServerCommand;
   pengfuncsFromEngine->pfnVoice_GetClientListening = pfnVoice_GetClientListening;
   pengfuncsFromEngine->pfnVoice_SetClientListening = pfnVoice_SetClientListening;
   pengfuncsFromEngine->pfnGetPlayerAuthId = pfnGetPlayerAuthId;

   // give the engine functions to the game DLL...
   (*other_GiveFnptrsToDll) (pengfuncsFromEngine, pGlobals);
}


void GameDLLInit (void)
{
   // this function is a one-time call, and appears to be the second function called in the
   // DLL after GiveFnptrsToDll() has been called. Its purpose is to tell the MOD DLL to
   // initialize the game before the engine actually hooks into it with its video frames and
   // clients connecting. Note that it is a different step than the *server* initialization.
   // This one is called once, and only once, when the game process boots up before the first
   // server is enabled. Here is a good place to do our own game session initialization, and
   // to register by the engine side the server commands we need to administrate our bots.

   // make sure we are in multiplayer mode
   is_multiplayer_game = IsMultiplayerGame ();

   // only init bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      RegisterServerCommand ("racc", ServerCommand); // register a new server command: racc
      RegisterServerVariable ("racc_minbots", "0"); // register a new CVAR: racc_minbots
      RegisterServerVariable ("racc_maxbots", "32"); // register a new CVAR: racc_maxbots
      RegisterServerVariable ("racc_botmanager", "1"); // register a new CVAR: racc_botmanager
      RegisterServerVariable ("racc_botforceteam", "0"); // register a new CVAR: racc_botforceteam
      RegisterServerVariable ("racc_internetmode", "1"); // register a new CVAR: racc_internetmode
      RegisterServerVariable ("racc_chatmode", "1"); // register a new CVAR; racc_chatmode
      RegisterServerVariable ("racc_voicechatmode", "0"); // register a new CVAR; racc_voicechatmode

      memset (bots, 0, sizeof (bots)); // initialize the bots array of structures...
      g_argv = (char *) malloc (1024); // allocate space for the bots' client commands argv field
      InitLogFile (); // initialize the log file
   }

   (*other_gFunctionTable.pfnGameInit) ();
}


int Spawn (entity_t *pent)
{
   return (*other_gFunctionTable.pfnSpawn) (pent);
}


void Think (entity_t *pent)
{
   (*other_gFunctionTable.pfnThink) (pent);
}


void Use (entity_t *pentUsed, entity_t *pentOther)
{
   (*other_gFunctionTable.pfnUse) (pentUsed, pentOther);
}


void Touch (entity_t *pentTouched, entity_t *pentOther)
{
   (*other_gFunctionTable.pfnTouch) (pentTouched, pentOther);
}


void Blocked (entity_t *pentBlocked, entity_t *pentOther)
{
   (*other_gFunctionTable.pfnBlocked) (pentBlocked, pentOther);
}


void KeyValue (entity_t *pentKeyvalue, KeyValueData *pkvd)
{
   (*other_gFunctionTable.pfnKeyValue) (pentKeyvalue, pkvd);
}


void Save (entity_t *pent, SAVERESTOREDATA *pSaveData)
{
   (*other_gFunctionTable.pfnSave) (pent, pSaveData);
}


int Restore (entity_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity)
{
   return (*other_gFunctionTable.pfnRestore) (pent, pSaveData, globalEntity);
}


void SetAbsBox (entity_t *pent)
{
   (*other_gFunctionTable.pfnSetAbsBox) (pent);
}


void SaveWriteFields (SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount)
{
   (*other_gFunctionTable.pfnSaveWriteFields) (pSaveData, pname, pBaseData, pFields, fieldCount);
}


void SaveReadFields (SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount)
{
   (*other_gFunctionTable.pfnSaveReadFields) (pSaveData, pname, pBaseData, pFields, fieldCount);
}


void SaveGlobalState (SAVERESTOREDATA *pSaveData)
{
   (*other_gFunctionTable.pfnSaveGlobalState) (pSaveData);
}


void RestoreGlobalState (SAVERESTOREDATA *pSaveData)
{
   (*other_gFunctionTable.pfnRestoreGlobalState) (pSaveData);
}


void ResetGlobalState (void)
{
   (*other_gFunctionTable.pfnResetGlobalState) ();
}


int ClientConnect (entity_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128])
{
   // this function is called in order to tell the MOD DLL that a client attempts to connect the
   // game. The entity pointer of this client is pEntity, the name under which he connects is
   // pointed to by the pszName pointer, and its IP address string is pointed by the pszAddress
   // one. Note that this does not mean this client will actually join the game ; he could as
   // well be refused connection by the server later, because of latency timeout, unavailable
   // game resources, or whatever reason. In which case the reason why the game DLL (read well,
   // the game DLL, *NOT* the engine) refuses this player to connect will be printed in the
   // szRejectReason string in all letters. Understand that a client connecting process is done
   // in three steps. First, the client requests a connection from the server. This is engine
   // internals. When there are already too many players, the engine will refuse this client to
   // connect, and the game DLL won't even notice. Second, if the engine sees no problem, the
   // game DLL is asked. This is where we are. Once the game DLL acknowledges the connection,
   // the client downloads the resources it needs and synchronizes its local engine with the one
   // of the server. And then, the third step, which comes *AFTER* ClientConnect(), is when the
   // client officially enters the game, through the ClientPutInServer() function, later below.
   // Here we hook this function in order to keep track of the listen server client entity,
   // because a listen server client always connects with a "loopback" address string. Also we
   // tell the bot manager to check the bot population, in order to always have one free slot on
   // the server for incoming clients.

   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      // is this client the listen server client ?
      if (strcmp (pszAddress, "loopback") == 0)
         pListenserverEntity = pEntity; // save its edict to pListenserverEntity

      // else are we reaching the max player count with this client ?
      else if (player_count + 1 == MaxClientsOnServer ())
         bot_check_time = CurrentTime (); // see if we need to disconnect a bot to allow future connections
   }

   return (*other_gFunctionTable.pfnClientConnect) (pEntity, pszName, pszAddress, szRejectReason);
}


void ClientDisconnect (entity_t *pEntity)
{
   // this function is called whenever a client is VOLUNTARILY disconnected from the server,
   // either because the client dropped the connection, or because the server dropped him from
   // the game (latency timeout). The effect is the freeing of a client slot on the server. Note
   // that clients and bots disconnected because of a level change NOT NECESSARILY call this
   // function, because in case of a level change, it's a server shutdown, and not a normal
   // disconnection. Anyway it's time to update the bots and players counts, and in case the
   // client disconnecting is a bot, to back its brain(s) up to disk. We also try to notice when
   // a listenserver client disconnects, so as to reset his entity pointer for safety. There are
   // still a few server frames to go once a listen server client disconnects, and we don't want
   // to send him any sort of messages then.

   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      int player_index = PlayerIndexOf (pEntity); // get player index

      // is the just disconnected client one of our bots ?
      if (IsABot (pEntity))
      {
         SaveHALBrainForPersonality (bots[player_index].pPersonality); // save our bot's HAL brain
         SaveNavBrainForPersonality (bots[player_index].pPersonality); // save our bot's NAV brain

         bots[player_index].is_active = FALSE; // free the slot
         DeleteEntity (bots[player_index].pLightEntity); // don't forget to kill its light entity
         bot_count--; // there is one bot less in the game
      }

      // else is the client disconnecting the listen server client ?
      else if (pEntity == pListenserverEntity)
         pListenserverEntity = NULL; // if so, reset his entity pointer

      player_count--; // decrement the player count as we know this client is disconnected
      players[player_index].pEntity = NULL; // fool the pointer to this player's entity
      players[player_index].welcome_time = 0; // reset this client's welcome time (in case not sent yet)
   }

   (*other_gFunctionTable.pfnClientDisconnect) (pEntity);
}


void ClientKill (entity_t *pEntity)
{
   // this function forcibly kills a living player, by a server-side decision. The player is
   // usually marked as "killed by world" or "killed by self", depending on how the MOD code
   // handles ClientKill() calls. Such a function is usefull for calling when one wants to kill
   // all the bots in a round, for example.

   (*other_gFunctionTable.pfnClientKill) (pEntity);
}


void ClientPutInServer (entity_t *pEntity)
{
   // this function is called once a just connected client actually enters the server, after
   // having downloaded and synchronized its resources with the of the server's. It's the
   // perfect place to hook for client connecting, since a client can always try to connect
   // passing the ClientConnect() step, and not be allowed by the server later (because of a
   // latency timeout or whatever reasone). We can here keep track of both bots and players
   // counts in realtime, since bots connect the server just like the way normal client do, and
   // their fakeclient flag is already supposed to be set then.

   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      int player_index = PlayerIndexOf (pEntity); // get player index

      // is the connecting client one of our bots ?
      if (IsABot (pEntity))
      {
         bots[player_index].is_active = TRUE; // set its 'is active' flag
         bot_count++; // there is one bot more in the game
      }

      player_count++; // increment the player count as we are certain now this client is connected
      players[player_index].pEntity = pEntity; // link a pointer to this player's entity
      players[player_index].welcome_time = CurrentTime () + 5.0; // send the welcome message in 5 s
   }

   (*other_gFunctionTable.pfnClientPutInServer) (pEntity); // tell the game DLL a client enters
}

void ClientCommand (entity_t *pEntity)
{
   // this function is called whenever the client whose player entity is pEntity issues a client
   // command. How it works is that clients all have a global string in their client DLL that
   // stores the command string; if ever that string is filled with characters, the client DLL
   // sends it to the engine as a command to be executed. When the engine has executed that
   // command, that string is reset to zero. By the server side, we can access this string
   // by asking the engine with the CmdArgv(), CmdArgs() and CmdArgc() functions that work just
   // like executable files argument processing work in C (argc gets the number of arguments,
   // command included, args returns the whole string, and argv returns the wanted argument
   // only). Here is a good place to set up either bot debug commands the listen server client
   // could type in his game console, or real new client commands, but we wouldn't want to do
   // so as this is just a bot DLL, not a MOD. The purpose is not to add functionality to
   // clients. Hence it can lack of commenting a bit, since this code is very subject to change.

   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game && !isFakeClientCommand)
   {
      char pcmd[128]; // no way, I hate pointers...
      char arg1[128];
      char arg2[128];
      char arg3[128];

      sprintf (pcmd, CMD_ARGV (0));
      sprintf (arg1, CMD_ARGV (1));
      sprintf (arg2, CMD_ARGV (2));
      sprintf (arg3, CMD_ARGV (3));

      // listenserver-only allowed commands
      if (pEntity == pListenserverEntity)
      {
         if (strcmp (pcmd, "botorder") == 0)
         {
            if ((arg1 != NULL) && (*arg1 != 0) && (arg2 != NULL) && (*arg2 != 0))
               for (int i = 0; i < MaxClientsOnServer (); i++)
                  if (strcmp (arg1, NetnameOf (bots[i].pEntity)) == 0)
                  {
                     ServerConsole_printf ("BOT %s executes command %s\n", NetnameOf (bots[i].pEntity), arg2);
                     FakeClientCommand (bots[i].pEntity, arg2);
                     break;
                  }
            return;
         }
         else if (strcmp (pcmd, "observer") == 0)
         {
            if ((arg1 != NULL) && (*arg1 != 0))
               DebugLevel.is_observer = (atoi (arg1) > 0);
            ServerConsole_printf ("Observer mode is %s\n", (DebugLevel.is_observer ? "ENABLED" : "DISABLED"));
            return;
         }
         else if (strcmp (pcmd, "debug") == 0)
         {
            if ((arg1 != NULL) && (*arg1 != 0) && (arg2 != NULL) && (*arg2 != 0))
            {
               if (strcmp (arg1, "eyes") ==  0)
                  DebugLevel.eyes = atoi (arg2);
               else if (strcmp (arg1, "ears") == 0)
                  DebugLevel.ears = atoi (arg2);
               else if (strcmp (arg1, "body") == 0)
                  DebugLevel.body = atoi (arg2);
               else if (strcmp (arg1, "legs") == 0)
                  DebugLevel.legs = atoi (arg2);
               else if (strcmp (arg1, "hand") == 0)
                  DebugLevel.hand = atoi (arg2);
               else if (strcmp (arg1, "chat") == 0)
                  DebugLevel.chat = atoi (arg2);
               ServerConsole_printf ("Debug level set to %d\n", atoi (arg2));
            }
            return;
         }
         else if (strcmp (pcmd, "botdontshoot") == 0)
         {
            if ((arg1 != NULL) && (*arg1 != 0))
               DebugLevel.is_peacemode = (atoi (arg1) > 0);
            ServerConsole_printf ("Bots don't shoot mode is %s\n", (DebugLevel.is_peacemode ? "ENABLED" : "DISABLED"));
            return;
         }
         else if (strcmp (pcmd, "auxkeyword") == 0)
         {
            if ((arg1 != NULL) && (*arg1 != 0) && (arg2 != NULL) && (*arg2 != 0))
               for (int i = 0; i < MaxClientsOnServer (); i++)
                  if (strcmp (arg1, NetnameOf (bots[i].pEntity)) == 0)
                  {
                     FILE *fp;
                     char aux_filename[256], line_buffer[256];
                     ServerConsole_printf ("BOT %s remembers keyword %s as an auxiliary keyword\n", NetnameOf (bots[i].pEntity), arg2);
                     sprintf (aux_filename, "racc/profiles/%s/%s/%s/racc-hal.aux", bots[i].pPersonality->name, mod_name, map_name);
                     fp = fopen (aux_filename, "a");
                     if (fp != NULL)
                     {
                        sprintf (line_buffer, "%s\n", arg2);
                        UpperCase (line_buffer);
                        fputs (line_buffer, fp);
                        fclose (fp);
                     }
                     break;
                  }
            return;
         }
         else if (strcmp (pcmd, "bankeyword") == 0)
         {
            if ((arg1 != NULL) && (*arg1 != 0) && (arg2 != NULL) && (*arg2 != 0))
               for (int i = 0; i < MaxClientsOnServer (); i++)
                  if (strcmp (arg1, NetnameOf (bots[i].pEntity)) == 0)
                  {
                     FILE *fp;
                     char ban_filename[256], line_buffer[256];
                     ServerConsole_printf ("BOT %s remembers not to choose word %s as a keyword\n", NetnameOf (bots[i].pEntity), arg2);
                     sprintf (ban_filename, "racc/profiles/%s/%s/%s/racc-hal.ban", bots[i].pPersonality->name, mod_name, map_name);
                     fp = fopen (ban_filename, "a");
                     if (fp != NULL)
                     {
                        sprintf (line_buffer, "%s\n", arg2);
                        UpperCase (line_buffer);
                        fputs (line_buffer, fp);
                        fclose (fp);
                     }
                     break;
                  }
            return;
         }
         else if (strcmp (pcmd, "swapkeyword") == 0)
         {
            if ((arg1 != NULL) && (*arg1 != 0) && (arg2 != NULL) && (*arg2 != 0) && (arg3 != NULL) && (*arg3 != 0))
               for (int i = 0; i < MaxClientsOnServer (); i++)
                  if (strcmp (arg1, NetnameOf (bots[i].pEntity)) == 0)
                  {
                     FILE *fp;
                     char swp_filename[256], line_buffer[256];
                     ServerConsole_printf ("BOT %s remembers to swap keyword %s to %s\n", NetnameOf (bots[i].pEntity), arg2, arg3);
                     sprintf (swp_filename, "racc/profiles/%s/%s/%s/racc-hal.swp", bots[i].pPersonality->name, mod_name, map_name);
                     fp = fopen (swp_filename, "a");
                     if (fp != NULL)
                     {
                        sprintf (line_buffer, "%s\t%s\n", arg2, arg3);
                        UpperCase (line_buffer);
                        fputs (line_buffer, fp);
                        fclose (fp);
                     }
                     break;
                  }
            return;
         }
         else if (strcmp (pcmd, "trainhal") == 0)
         {
            if ((arg1 != NULL) && (*arg1 != 0) && (arg2 != NULL) && (*arg2 != 0))
               for (int i = 0; i < MaxClientsOnServer (); i++)
                  if (strcmp (arg1, NetnameOf (bots[i].pEntity)) == 0)
                  {
                     FILE *fp;
                     HAL_DICTIONARY *words = HAL_NewDictionary ();
                     char trn_filename[256], line_buffer[256];
                     sprintf (trn_filename, "racc/profiles/%s/%s/%s/%s", bots[i].pPersonality->name, mod_name, map_name, arg2);
                     fp = fopen (trn_filename, "r");
                     if (fp == NULL)
                     {
                        ServerConsole_printf ("BOT %s can't find file \"%s\"\n", NetnameOf (bots[i].pEntity), arg2);
                        return;
                     }
                     ServerConsole_printf ("BOT %s learns to speak by reading \"%s\"\n", NetnameOf (bots[i].pEntity), arg2);
                     while (fgets (line_buffer, 255, fp) != NULL)
                     {
                        if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
                           continue; // ignore line if void or commented
                        if (line_buffer[strlen (line_buffer) - 1] == '\n')
                           line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
                        UpperCase (line_buffer); // turn it uppercase
                        HAL_MakeWords (line_buffer, words); // break it into an array of words
                        HAL_Learn (bots[i].pPersonality->bot_model, words); // learn that array into the Markov model
                     }
                     fclose (fp); // close file
                     if (words)
                     {
                        for (int j = 0; j < (int) words->size; j++)
                           if (words->entry[j].word)
                              free (words->entry[j].word); // free every word
                        HAL_EmptyDictionary (words); // destroy the training dictionary
                        free (words); // and frees it
                     }
                     break;
                  }
            return;
         }
      }
   }

   (*other_gFunctionTable.pfnClientCommand) (pEntity);
}


void ClientUserInfoChanged (entity_t *pEntity, char *infobuffer)
{
   (*other_gFunctionTable.pfnClientUserInfoChanged) (pEntity, infobuffer);
}


void ServerActivate (entity_t *pEdictList, int edictCount, int clientMax)
{
   // this function is called when the server has fully loaded and is about to manifest itself
   // on the network as such. Since a mapchange is actually a server shutdown followed by a
   // restart, this function is also called when a new map is being loaded. Hence it's the
   // perfect place for doing initialization stuff for our bots, such as reading the BSP data,
   // loading the bot profiles, and drawing the world map (ie, filling the navigation hashtable).
   // Once this function has been called, the server can be considered as "running".

   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      // print a welcome message on the server console
      ServerConsole_printf ("\n");
      ServerConsole_printf ("   " RACC_WELCOMETEXT "\n");
      ServerConsole_printf ("   This program comes with ABSOLUTELY NO WARRANTY; see license for details.\n");
      ServerConsole_printf ("   This is free software, you are welcome to redistribute it the way you want.\n");
      ServerConsole_printf ("\n");

      sprintf (mod_name, GetModName ()); // get MOD name
      sprintf (map_name, GetMapName ()); // get map name

      // check if we are running on a dedicated server
      is_dedicated_server = IsDedicatedServer ();

      // precache the local client sounds that aren't precached by the game
      PrecacheSoundForBots ("radio/elim.wav", -1);
      PrecacheSoundForBots ("radio/blow.wav", -1);
      PrecacheSoundForBots ("radio/bombdef.wav", -1);
      PrecacheSoundForBots ("radio/bombpl.wav", -1);
      PrecacheSoundForBots ("radio/circleback.wav", -1);
      PrecacheSoundForBots ("radio/clear.wav", -1);
      PrecacheSoundForBots ("radio/com_followcom.wav", -1);
      PrecacheSoundForBots ("radio/com_getinpos.wav", -1);
      PrecacheSoundForBots ("radio/com_go.wav", -1);
      PrecacheSoundForBots ("radio/com_reportin.wav", -1);
      PrecacheSoundForBots ("radio/ctwin.wav", -1);
      PrecacheSoundForBots ("radio/ct_affirm.wav", -1);
      PrecacheSoundForBots ("radio/ct_backup.wav", -1);
      PrecacheSoundForBots ("radio/ct_coverme.wav", -1);
      PrecacheSoundForBots ("radio/ct_enemys.wav", -1);
      PrecacheSoundForBots ("radio/ct_fireinhole.wav", -1);
      PrecacheSoundForBots ("radio/ct_imhit.wav", -1);
      PrecacheSoundForBots ("radio/ct_inpos.wav", -1);
      PrecacheSoundForBots ("radio/ct_point.wav", -1);
      PrecacheSoundForBots ("radio/ct_reportingin.wav", -1);
      PrecacheSoundForBots ("radio/enemydown.wav", -1);
      PrecacheSoundForBots ("radio/fallback.wav", -1);
      PrecacheSoundForBots ("radio/fireassis.wav", -1);
      PrecacheSoundForBots ("radio/followme.wav", -1);
      PrecacheSoundForBots ("radio/getout.wav", -1);
      PrecacheSoundForBots ("radio/go.wav", -1);
      PrecacheSoundForBots ("radio/hitassist.wav", -1);
      PrecacheSoundForBots ("radio/hosdown.wav", -1);
      PrecacheSoundForBots ("radio/letsgo.wav", -1);
      PrecacheSoundForBots ("radio/locknload.wav", -1);
      PrecacheSoundForBots ("radio/matedown.wav", -1);
      PrecacheSoundForBots ("radio/meetme.wav", -1);
      PrecacheSoundForBots ("radio/moveout.wav", -1);
      PrecacheSoundForBots ("radio/negative.wav", -1);
      PrecacheSoundForBots ("radio/position.wav", -1);
      PrecacheSoundForBots ("radio/regroup.wav", -1);
      PrecacheSoundForBots ("radio/rescued.wav", -1);
      PrecacheSoundForBots ("radio/roger.wav", -1);
      PrecacheSoundForBots ("radio/rounddraw.wav", -1);
      PrecacheSoundForBots ("radio/sticktog.wav", -1);
      PrecacheSoundForBots ("radio/stormfront.wav", -1);
      PrecacheSoundForBots ("radio/takepoint.wav", -1);
      PrecacheSoundForBots ("radio/terwin.wav", -1);
      PrecacheSoundForBots ("radio/vip.wav", -1);
      PrecacheSoundForBots ("radio/flankthem.wav", -1);

      PrecacheStuff (); // precache miscellaneous stuff we need
      InitTextureSoundsForBots (); // build the texture-related footstep sounds database
      LookDownOnTheWorld (); // look down on the world and sort the faces and delimiters
      LoadBotProfiles (); // load profiles

      // force racc_maxbots in bounds now that profiles are loaded
      if (((GetServerVariable ("racc_maxbots") > personality_count) && (personality_count > 0))
          || (GetServerVariable ("racc_maxbots") < 0) || (GetServerVariable ("racc_maxbots") > 31))
         SetServerVariable ("racc_maxbots", personality_count); // adjust racc_maxbots to the bot list count
   }

   (*other_gFunctionTable.pfnServerActivate) (pEdictList, edictCount, clientMax);
}


void ServerDeactivate (void)
{
   // this function is called when the server is shutting down. A particular note about map
   // changes: changing the map means shutting down the server and starting a new one. Of course
   // this process is transparent to the user, but either in single player when the hero reaches
   // a new level and in multiplayer when it's time for a map change, be aware that what happens
   // is that the server actually shuts down and restarts with a new map. Hence we can use this
   // function to free and deinit anything which is map-specific, for example we free the memory
   // space we m'allocated for our BSP data, since a new map means new BSP data to interpret. In
   // any case, when the new map will be booting, ServerActivate() will be called, so we'll do
   // the loading of new bots and the new BSP data parsing there.

   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      int i, j;

      memset (&sounds, 0, sizeof (sounds)); // reset sound list
      sound_count = 0; // reset sound count

      // do we need to free the walkable faces memory space ?
      if (map.walkfaces)
      {
         for (i = 0; i < map.walkfaces_count; i++)
         {
            free (map.walkfaces[i].v_corners); // free the walkable face corners
            map.walkfaces[i].v_corners = NULL; // fools the pointer
            free (map.walkfaces[i].v_delimiters); // free the walkable face delimiters
            map.walkfaces[i].v_delimiters = NULL; // fools the pointer
         }

         free (map.walkfaces); // free the walkable face memory space itself
         map.walkfaces = NULL; // fools the pointer
      }

      // do we need to free the hashtable data ?
      for (i = 0; i < map.parallels_count; i++)
         for (j = 0; j < map.meridians_count; j++)
         {
            if (map.topology[i][j].faces)
            {
               free (map.topology[i][j].faces); // don't forget to free the faces array in the hashtable
               map.topology[i][j].faces = NULL; // fools the pointer
            }
         }

      // kill all the bots, cycle through all of them
      for (i = 0; i < personality_count; i++)
      {
         SaveHALBrainForPersonality (&bot_personalities[i]); // save this personality's HAL brain
         SaveNavBrainForPersonality (&bot_personalities[i]); // save this personality's NAV brain

         // free every word in their global chat dictionary
         if (bot_personalities[i].input_words)
         {
            for (j = 0; j < (int) bot_personalities[i].input_words->size; j++)
            {
               if (bot_personalities[i].input_words->entry[j].word)
                  free (bot_personalities[i].input_words->entry[j].word); // free the word
               bot_personalities[i].input_words->entry[j].word = NULL; // and fools the pointer
            }
            HAL_EmptyDictionary (bot_personalities[i].input_words); // empty that dictionary itself
            if (bot_personalities[i].input_words)
               free (bot_personalities[i].input_words); // now frees the dictionary
            bot_personalities[i].input_words = NULL; // and fools the pointer
         }

         // free every link in their navigation nodes array
         for (j = 0; j < map.walkfaces_count; j++)
         {
            if (bot_personalities[i].PathMemory[j].links)
               free (bot_personalities[i].PathMemory[j].links); // free the link
            bot_personalities[i].PathMemory[j].links = NULL; // and fools the pointer
         }
         if (bot_personalities[i].PathMemory)
            free (bot_personalities[i].PathMemory); // free the navigation nodes array itself
         bot_personalities[i].PathMemory = NULL; // and fools the pointer
      }
   }

   (*other_gFunctionTable.pfnServerDeactivate) ();
}


void PlayerPreThink (entity_t *pEntity)
{
   // this function is called by the engine before any client player (including bots) starts
   // his thinking cycle (ie, before the MOD's Think() function is called for this player).
   // Here we hook this function in order to check when to send the welcome message to newly
   // connected players.

   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      int player_index;

      // are we dealing with a valid, REAL player (not a bot) AND this player is alive ?
      if (!IsNull (pEntity) && IsAPlayer (pEntity) && !IsABot (pEntity) && IsAlive (pEntity))
      {
         player_index = PlayerIndexOf (pEntity); // get this player's index

         // if it's time to send the welcome message to this client...
         if ((players[player_index].welcome_time > 0) && (players[player_index].welcome_time < CurrentTime ()))
         {
            SendWelcomeMessageTo (pEntity); // send welcome message to this player if needed
            players[player_index].welcome_time = 0; // don't do it twice
         }
      }
   }

   (*other_gFunctionTable.pfnPlayerPreThink) (pEntity);
}


void PlayerPostThink (entity_t *pEntity)
{
   // this function is called by the engine after any client player (including bots) has finished
   // his thinking cycle (ie, the MOD's Think() function has been called for this player). Here
   // we use it to check whether this player has moved onto another face, and if so, update the
   // automatic pathbuilder. Also it's a good place to dispatch client sounds to the bot's ears.
   // Then since we might have to compare something in this player's entity variables to its last
   // state one day, it's always useful to keep a local copy of his entvars. 

   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      walkface_t *face;
      int player_index;

      // are we dealing with a valid player AND this player is still alive ?
      if (!IsNull (pEntity) && IsAPlayer (pEntity) && IsAlive (pEntity))
      {
         // check for the face this player is walking on and remember if it has changed
         player_index = PlayerIndexOf (pEntity); // get this player's index
         face = WalkfaceUnder (pEntity); // get this player's ground face

         // is this player the listen server client AND do we want high navigation debug level ?
         if ((pEntity == pListenserverEntity) && (DebugLevel.navigation > 1))
         {
            // draw the face on which it is on and the sector to which it belongs
            UTIL_DrawSector (pListenserverEntity, BottomOriginOf (pEntity), 3, 255, 0, 0);
            UTIL_DrawWalkface (pListenserverEntity, face, 3, 0, 255, 0);
         }

         // has it changed since last time (ie has this player moved onto another face ?)
         if ((face != NULL) && (face != players[player_index].pFaceAtFeet))
         {
            // then update the automatic pathbuilder

            // WRITE YOUR STUFF HERE
            // my algo is: for each bot who sees this player, make it store a new link into its
            // path nodes describing this connection between the previous and new walkface. Also
            // it's good to monitor what type of reachability it is, and whether the player
            // needed to jump, duck, or use a button or whatever complicated action in order to
            // reach this face.

            players[player_index].pFaceAtFeet = face; // remember the new face this player stands on
         }

         // see if this client needs to dispatch sounds to the bot's ears
         PlayClientSoundsForBots (pEntity);

         // and remember this player's entity variables states
         players[player_index].prev_v = players[player_index].pEntity->v;
      }
   }

   (*other_gFunctionTable.pfnPlayerPostThink) (pEntity);
}


void StartFrame (void)
{
   // this function starts a video frame. It is called once per video frame by the engine. If
   // you run Half-Life at 90 fps, this function will then be called 90 times per second. By
   // placing a hook on it, we have a good place to do things that should be done continuously
   // during the game, for example making the bots think (yes, because no Think() function exists
   // for the bots by the MOD side, remember). Also here we have control on the bot population,
   // for example if a new player joins the server, we should disconnect a bot, and if the
   // player population decreases, we should fill the server with other bots. Such checks
   // (amongst others) are done here.

   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      static float previous_time = 9999.0;
      static int bot_index, minbots, maxbots;

      // if a new map has started...
      if (CurrentTime () + 0.1 < previous_time)
         bot_check_time = CurrentTime () + 1.0; // start adding bots again in 1 second

      // cycle through all bot slots
      for (bot_index = 0; bot_index < MaxClientsOnServer (); bot_index++)
      {
         // is this slot used ?
         if (bots[bot_index].is_active && !IsNull (bots[bot_index].pEntity))
         {
            MarkPlayerAsBot (bots[bot_index].pEntity); // mark this player entity as being a bot

            BotPreThink (&bots[bot_index]); // make bot sense
            BotThink (&bots[bot_index]); // make bot think
            BotPostThink (&bots[bot_index]); // make bot act
         }
      }

      // check if a bot needs to be created...
      if ((bot_check_time < CurrentTime ()) && (GetServerVariable ("racc_botmanager") > 0))
      {
         minbots = GetServerVariable ("racc_minbots"); // get the "racc_minbots" server variable
         maxbots = GetServerVariable ("racc_maxbots"); // get the "racc_maxbots" server variable

         // can we add a bot AND are there less bots than the maximum # of bots ?
         if ((player_count < MaxClientsOnServer () - 1)
             && ((bot_count < maxbots) || (maxbots == -1))
             && ((GetServerVariable ("racc_internetmode") == 0)
                 || (CurrentTime () < 60)
                 || (RandomInteger (1, 100) < 2)))
            BotCreate (); // add a bot

         // else if there are too many bots disconnect one from the server
         else if (((maxbots != -1) && (bot_count > maxbots))
                  || ((player_count == MaxClientsOnServer ()) && (bot_count > minbots)))
         {
            // cycle through all bot slots
            for (bot_index = 0; bot_index < MaxClientsOnServer (); bot_index++)
            {
               // is this slot used ?
               if (bots[bot_index].is_active && (bots[bot_index].pEntity != NULL))
               {
                  ServerCommand ("kick \"%s\"\n", NetnameOf (bots[bot_index].pEntity)); // let the bot disconnect
                  break;
               }
            }
         }

         bot_check_time = CurrentTime () + 0.5; // next check in 0.5 second
      }

      previous_time = CurrentTime (); // previous time gets updated at each StartFrame
   }

   (*other_gFunctionTable.pfnStartFrame) ();
}


void ParmsNewLevel (void)
{
   (*other_gFunctionTable.pfnParmsNewLevel) ();
}


void ParmsChangeLevel (void)
{
   (*other_gFunctionTable.pfnParmsChangeLevel) ();
}


const char *GetGameDescription (void)
{
   return (*other_gFunctionTable.pfnGetGameDescription) ();
}


void PlayerCustomization (entity_t *pEntity, customization_t *pCust)
{
   (*other_gFunctionTable.pfnPlayerCustomization) (pEntity, pCust);
}


void SpectatorConnect (entity_t *pEntity)
{
   (*other_gFunctionTable.pfnSpectatorConnect) (pEntity);
}


void SpectatorDisconnect (entity_t *pEntity)
{
   (*other_gFunctionTable.pfnSpectatorDisconnect) (pEntity);
}


void SpectatorThink (entity_t *pEntity)
{
   (*other_gFunctionTable.pfnSpectatorThink) (pEntity);
}


void Sys_Error (const char *error_string)
{
   (*other_gFunctionTable.pfnSys_Error) (error_string);
}


void PM_Move (struct playermove_s *ppmove, int server)
{
   (*other_gFunctionTable.pfnPM_Move) (ppmove, server);
}


void PM_Init (struct playermove_s *ppmove)
{
   (*other_gFunctionTable.pfnPM_Init) (ppmove);
}


char PM_FindTextureType (char *name)
{
   return (*other_gFunctionTable.pfnPM_FindTextureType) (name);
}


void SetupVisibility (entity_t *pViewEntity, entity_t *pClient, unsigned char **pvs, unsigned char **pas)
{
   (*other_gFunctionTable.pfnSetupVisibility) (pViewEntity, pClient, pvs, pas);
}


void UpdateClientData (const struct edict_s *ent, int sendweapons, struct clientdata_s *cd)
{
   (*other_gFunctionTable.pfnUpdateClientData) (ent, sendweapons, cd);
}


int AddToFullPack (struct entity_state_s *state, int e, entity_t *ent, entity_t *host, int hostflags, int player, unsigned char *pSet)
{
   return (*other_gFunctionTable.pfnAddToFullPack) (state, e, ent, host, hostflags, player, pSet);
}


void CreateBaseline (int player, int eindex, struct entity_state_s *baseline, struct edict_s *entity, int playermodelindex, vec3_t player_mins, vec3_t player_maxs)
{
   (*other_gFunctionTable.pfnCreateBaseline) (player, eindex, baseline, entity, playermodelindex, player_mins, player_maxs);
}


void RegisterEncoders (void)
{
   (*other_gFunctionTable.pfnRegisterEncoders) ();
}


int GetWeaponData (struct edict_s *player, struct weapon_data_s *info)
{
   return (*other_gFunctionTable.pfnGetWeaponData) (player, info);
}


void CmdStart (const entity_t *player, const struct usercmd_s *cmd, unsigned int random_seed)
{
   (*other_gFunctionTable.pfnCmdStart) (player, cmd, random_seed);
}


void CmdEnd (const entity_t *player)
{
   (*other_gFunctionTable.pfnCmdEnd) (player);
}


int ConnectionlessPacket (const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size)
{
   return (*other_gFunctionTable.pfnConnectionlessPacket) (net_from, args, response_buffer, response_buffer_size);
}


int GetHullBounds (int hullnumber, float *mins, float *maxs)
{
   return (*other_gFunctionTable.pfnGetHullBounds) (hullnumber, mins, maxs);
}


void CreateInstancedBaselines (void)
{
   (*other_gFunctionTable.pfnCreateInstancedBaselines) ();
}


int InconsistentFile (const entity_t *player, const char *filename, char *disconnect_message)
{
   return (*other_gFunctionTable.pfnInconsistentFile) (player, filename, disconnect_message);
}


int AllowLagCompensation (void)
{
   return (*other_gFunctionTable.pfnAllowLagCompensation) ();
}


void ServerCommand (void)
{
   // this is the function we registered by the engine side as the RACC server command handler,
   // it will get called as soon as the server will receive a "racc" command. Arguments to it
   // can then be accessed by pfnCmdArgv, just like client commands. Life is so simple when
   // things are done the right way...

   char pcmd[129]; // no way, I hate pointers...
   char arg1[129];

   sprintf (pcmd, CMD_ARGV (1));
   sprintf (arg1, CMD_ARGV (2));

   // handle RACC server commands
   if ((strcmp (pcmd, "help") == 0) || (strcmp (pcmd, "?") == 0))
   {
      ServerConsole_printf ("%s\n", RACC_WELCOMETEXT);
      ServerConsole_printf ("  -- Available server commands:\n");
      ServerConsole_printf ("racc add - Add a bot to the current game\n");
      ServerConsole_printf ("racc kick - Disconnect a bot from the current game\n");
      ServerConsole_printf ("racc kickall - Disconnect all bots from the current game\n");
      ServerConsole_printf ("racc killall - Kill all bots in the current round\n");
      ServerConsole_printf ("racc viewprofiles - Display profiles of bots currently in the game\n");
      ServerConsole_printf ("racc viewallprofiles - Display all the bot profiles that have been woken up\n");
      ServerConsole_printf ("racc botcount - Display the number of bots currently in the game\n");
      ServerConsole_printf ("racc playercount - Display the total number of players currently in the game\n");
      ServerConsole_printf ("racc time - Display the current map play time\n");
      ServerConsole_printf ("racc msec [method] - Change the msec calculation method from 1 to 4\n");
   }
   else if (strcmp (pcmd, "add") == 0)
   {
      BotCreate (); // create a bot
      bot_check_time = CurrentTime () + 0.5; // delay a while before checking if another one has to be created
   }
   else if (strcmp (pcmd, "kick") == 0)
   {
      // cycle through all bots and client slots
      for (int i = 0; i < MaxClientsOnServer (); i++)
         if (bots[i].is_active && (bots[i].pEntity != NULL))
         {
            ServerCommand ("kick \"%s\"\n", NetnameOf (bots[i].pEntity)); // let the bot disconnect
            bot_check_time = CurrentTime () + 0.5; // delay a while before checking if another one has to be created
            break;
         }
   }
   else if (strcmp (pcmd, "kickall") == 0)
   {
      // cycle through all bots and client slots
      for (int i = 0; i < MaxClientsOnServer (); i++)
      {
         if (bots[i].is_active && (bots[i].pEntity != NULL))
            ServerCommand ("kick \"%s\"\n", NetnameOf (bots[i].pEntity)); // let the bot disconnect
         bot_check_time = CurrentTime () + 0.5; // delay a while before checking if another one has to be created
      }
   }
   else if (strcmp (pcmd, "killall") == 0)
   {
      // cycle through all bots and client slots
      for (int i = 0; i < MaxClientsOnServer (); i++)
         if (bots[i].is_active && (bots[i].pEntity != NULL))
         {
            SetFrags (bots[i].pEntity, FragsOf (bots[i].pEntity) + 1); // don't count this as a suicide
            ClientKill (bots[i].pEntity); // kill the bot
         }
   }
   else if (strcmp (pcmd, "viewprofiles") == 0)
   {
      ServerConsole_printf ("RACC: Current bot profiles\n"); // tell people what we are doing

      // cycle through all bots and client slots
      for (int i = 0; i < MaxClientsOnServer (); i++)
         if (bots[i].is_active && (bots[i].pEntity != NULL)) // is this slot used?
         {
            ServerConsole_printf ("Name: %s - Model: %s - Logo: %s - Skill: %d\n",
                                  NetnameOf (bots[i].pEntity),
                                  ModelOf (bots[i].pEntity),
                                  "default", // unable to handle different bot logos so far
                                  bots[i].pPersonality->skill);
         }
   }
   else if (strcmp (pcmd, "viewallprofiles") == 0)
   {
      ServerConsole_printf ("RACC: Awaken bot profiles\n"); // tell people what we are doing

      // cycle through all bot personalities
      for (int i = 0; i < personality_count; i++)
         ServerConsole_printf ("Name: %s - Model: %s - Logo: %s - Skill: %d\n",
                               bot_personalities[i].name,
                               bot_personalities[i].skin,
                               "default", // unable to handle different bot logos so far
                               bot_personalities[i].skill);
   }
   else if (strcmp (pcmd, "botcount") == 0)
      ServerConsole_printf ("There are %d bots in game\n", bot_count);
   else if (strcmp (pcmd, "playercount") == 0)
      ServerConsole_printf ("There are %d players in game\n", player_count);
   else if (strcmp (pcmd, "time") == 0)
      ServerConsole_printf ("Current map play time is %f seconds\n", CurrentTime ());
   else if (strcmp (pcmd, "msec") == 0)
   {
      msec_method = atoi (arg1); // change the method to the one specified
      ServerConsole_printf ("Msec calculation method set to METHOD_%s\n", (msec_method == METHOD_TOBIAS ? "TOBIAS" : (msec_method == METHOD_LEON ? "LEON" : (msec_method == METHOD_RICH ? "RICH" : "PM"))));
   }
   else
   {
      ServerConsole_printf ("RACC: Unknown command \"%s\"\n", pcmd);
      ServerConsole_printf ("Type \"racc help\" for list of available commands.\n");
   }
}


void PlayClientSoundsForBots (entity_t *pPlayer)
{
   // this function determines if the player pPlayer is walking or running, or climbing a ladder,
   // or landing on the ground, and so if he's likely to emit some client sound or not. Since
   // these types of sounds are predicted on the client side only, and bots have no client DLL,
   // we have to simulate their emitting in order for the bots to hear them. So in case a player
   // is moving, we bring his footstep sounds to the ears of the bots around. This sound is based
   // on the texture the player is walking on. Using TraceTexture(), we ask the engine for that
   // texture, then look up in the step sounds database in order to determine which footstep
   // sound is related to that texture. The ladder check then assumes that a player moving
   // vertically, not on the ground, having a ladder in his immediate surroundings is climbing
   // it, and the ladder sound is emitted periodically the same way footstep sounds are emitted.
   // Then, the landing check looks for non-null value of the player's punch angles (screen
   // tilting) while this player's damage inflictor be either null, or worldspawn. If the test
   // success, a landing sound is emitted as well.
   // thanks to Tom Simpson from FoxBot for the water sounds handling

   entity_t *pEntity = NULL;
   const char *texture_name;
   char sound_name[256], sound_path[256], player_weapon[256];
   int player_index, count;
   float player_velocity, volume;
   playerbuttons_t playerbuttons;
   vector v_bbmin, v_bbmax;

   if (IsNull (pPlayer) || !IsAlive (pPlayer))
      return; // skip invalid and dead players

   if (DebugLevel.is_observer && !IsABot (pPlayer))
      return; // skip real players if in observer mode

   player_index = PlayerIndexOf (pPlayer); // get the player index
   player_velocity = VelocityOf (pPlayer); // get the player velocity
   sprintf (player_weapon, WeaponModelOf (pPlayer)); // get the player's weapon name
   playerbuttons = InputButtonsOf (pPlayer); // get the player's input buttons

   // does the server allow footstep sounds AND this player is actually moving
   // AND is player on the ground AND is it time for him to make a footstep sound
   // OR has that player just landed on the ground after a jump ?
   if (((GetServerVariable ("mp_footsteps") > 0) && IsOnFloor (pPlayer) && (player_velocity > 0)
        && (players[player_index].step_sound_time < CurrentTime ()))
       || (((DamageInflictorOf (pPlayer) == NULL) || (strcmp ("worldspawn", ClassnameOf (DamageInflictorOf (pPlayer))) == 0))
           && (PunchAnglesOf (pPlayer) != Vector (0, 0, 0)) && !(players[player_index].prev_v.flags & FL_ONGROUND)))
   {
      // is this player sloshing in water ?
      if (WaterLevelOf (pPlayer) != WATERLEVEL_NOT)
      {
         sprintf (sound_path, "player/pl_slosh%d.wav", RandomInteger (1, 4)); // build a slosh sound path

         // bring slosh sound from this player's feet to bots' ears
         DispatchSound (sound_path, OriginOf (pPlayer) + vector (0, 0, -18), 0.9, ATTN_NORM);
         players[player_index].step_sound_time = CurrentTime () + 0.300; // next slosh in 300 milliseconds
      }

      // else this player is definitely not in water, does he move fast enough to make sounds ?
      else if (player_velocity > MAX_WALK_SPEED)
      {
         // assign the player a default texture under the feet in case none specified yet
         if (players[player_index].pTextureAtFeet == NULL)
            players[player_index].pTextureAtFeet = &textures[0]; // default texture

         // ask the engine for the texture name under the player's feet
         texture_name = TextureNameOn (EntityUnder (pPlayer), OriginOf (pPlayer), AnglesOfVector (vector (0, 0, -1)));

         // did the engine fail reporting the texture ?
         if (texture_name == NULL)
            texture_name = textures[0].name; // take the default texture as replacement

         // has the player started walking on a new texture ?
         if (strnicmp (texture_name, players[player_index].pTextureAtFeet->name, CBTEXTURENAMEMAX - 1) != 0)
            players[player_index].pTextureAtFeet = FindTextureByName (texture_name); // link a pointer to this new texture in the textures array

         // given the type of texture under player's feet, prepare a sound file for being played
         switch (players[player_index].pTextureAtFeet->type)
         {
            default:
            case CHAR_TEX_CONCRETE:
               strcpy (sound_name, "player/pl_step%d.wav");
               count = 4; // there are 4 different step sounds
               volume = 0.9;
               break;
            case CHAR_TEX_METAL:
               strcpy (sound_name, "player/pl_metal%d.wav");
               count = 4; // there are 4 different metal sounds
               volume = 0.9;
               break;
            case CHAR_TEX_DIRT:
               strcpy (sound_name, "player/pl_dirt%d.wav");
               count = 4; // there are 4 different dirt sounds
               volume = 0.9;
               break;
            case CHAR_TEX_VENT:
               strcpy (sound_name, "player/pl_duct%d.wav");
               count = 4; // there are 4 different duct sounds
               volume = 0.5;
               break;
            case CHAR_TEX_GRATE:
               strcpy (sound_name, "player/pl_grate%d.wav");
               count = 4; // there are 4 different grate sounds
               volume = 0.9;
               break;
            case CHAR_TEX_TILE:
               strcpy (sound_name, "player/pl_tile%d.wav");
               count = 5; // there are 5 different tile sounds
               volume = 0.8;
               break;
            case CHAR_TEX_SLOSH:
               strcpy (sound_name, "player/pl_slosh%d.wav");
               count = 4; // there are 4 different slosh sounds
               volume = 0.9;
               break;
            case CHAR_TEX_WOOD:
               strcpy (sound_name, "debris/wood%d.wav");
               count = 3; // there are 3 different wood debris sounds
               volume = 0.9;
               break;
            case CHAR_TEX_GLASS:
            case CHAR_TEX_COMPUTER:
               strcpy (sound_name, "debris/glass%d.wav");
               count = 4; // there are 4 different glass debris sounds
               volume = 0.8;
               break;
            case 'N':
               strcpy (sound_name, "player/pl_snow%d.wav");
               count = 6; // there are 6 different snow sounds (Counter-Strike specific)
               volume = 0.8;
               break;
         }

         // now build a random sound path amongst the count
         sprintf (sound_path, sound_name, RandomInteger (1, count));

         // did we hit a breakable ?
         if (strcmp ("func_breakable", ClassnameOf (EntityUnder (pPlayer))) == 0)
            volume /= 1.5; // drop volume, the object will already play a damaged sound

         // bring footstep sound from this player's feet to bots' ears
         DispatchSound (sound_path, OriginOf (pPlayer) + vector (0, 0, -18), volume, ATTN_NORM);
         players[player_index].step_sound_time = CurrentTime () + 0.300; // next step in 300 milliseconds
      }
   }

   // is this player completely in water AND it's time to play a wade sound
   // AND this player is pressing the jump key for swimming up ?
   if ((players[player_index].step_sound_time < CurrentTime ())
       && (WaterLevelOf (pPlayer) == WATERLEVEL_COMPLETELY) && (playerbuttons.f_jump_time == CurrentTime ()))
   {
      sprintf (sound_name, "player/pl_wade%d.wav", RandomInteger (1, 4)); // build a wade sound path

      // bring wade sound from this player's feet to bots' ears
      DispatchSound (sound_path, OriginOf (pPlayer) + vector (0, 0, -18), 0.9, ATTN_NORM);
      players[player_index].step_sound_time = CurrentTime () + 0.500; // next wade in 500 milliseconds
   }

   // now let's see if this player is on a ladder, for that we consider that he's not on the
   // ground, he's actually got a velocity (especially vertical), and that he's got a
   // func_ladder entity right in front of him. Is that player moving anormally NOT on ground ?
   if ((VerticalVelocityOf (pPlayer) > 0) && IsFlying (pPlayer) && !IsOnFloor (pPlayer))
   {
      // cycle through all ladders...
      while ((pEntity = UTIL_FindEntityByClassname (pEntity, "func_ladder")) != NULL)
      {
         GetEntityBoundingBox (pEntity, v_bbmin, v_bbmax); // get ladder bounding box

         // is this ladder at the same height as the player AND the player is next to it (in
         // which case, assume he's climbing it), AND it's time for him to emit ladder sound ?
         if ((v_bbmin.z < OriginOf (pPlayer).z) && (v_bbmax.z > OriginOf (pPlayer).z)
             && ((OriginOf (pEntity) - OriginOf (pPlayer)).Length2D () < 40)
             && (players[player_index].step_sound_time < CurrentTime ()))
         {
            volume = 0.8; // default volume for ladder sounds (empirical)

            // now build a random sound path amongst the 4 different ladder sounds
            sprintf (sound_path, "player/pl_ladder%d.wav", RandomInteger (1, 4));

            // is the player ducking ?
            if (playerbuttons.f_duck_time == CurrentTime ())
               volume /= 1.5; // drop volume, the player is trying to climb silently

            // bring ladder sound from this player's feet to bots' ears
            DispatchSound (sound_path, OriginOf (pPlayer) + vector (0, 0, -18), volume, ATTN_NORM);
            players[player_index].step_sound_time = CurrentTime () + 0.500; // next in 500 milliseconds
         }
      }
   }

   // and now let's see if this player is pulling the pin of a grenade...
   if (((stricmp (player_weapon, "models/p_flashbang.mdl") == 0)
        || (stricmp (player_weapon, "models/p_hegrenade.mdl") == 0)
        || (stricmp (player_weapon, "models/p_smokegrenade.mdl") == 0))
       && (playerbuttons.f_fire1_time == CurrentTime ()) && !(players[player_index].prev_v.button & IN_ATTACK))
      DispatchSound ("weapons/pinpull.wav", GunOriginOf (pPlayer), 1.0, ATTN_NORM);

   return;
}


void PlayBulletSoundsForBots (entity_t *pPlayer)
{
   // this function is in charge of emulating the gunshot sounds for the bots. Since these sounds
   // are only predicted by the client, and bots have no client DLL, obviously we have to do the
   // work for them. We consider a client is told gunshot sound occurs when he receives the
   // msg_CurWeapon network message, which gets sent whenever a player is lowering his ammo.
   // That's why we hook those messages in MessageBegin(), and send the entity responsible of
   // it have a walk around here. Given the weapon this player is holding in his hand then, the
   // appropriate gunshot sound is played. A special check is done for weapons that have several
   // fire modes, such as the silenced M4A1 and USP. Then DispatchSound() is called to bring the
   // selected sound to the bot's ears.

   texture_t *pHitTexture;
   const char *texture_name;
   char sound_path[256], sound_name[256], player_weapon[256];
   int player_index, count;
   playerbuttons_t playerbuttons;

   if (IsNull (pPlayer) || !IsAlive (pPlayer))
      return; // skip invalid and dead players

   if (DebugLevel.is_observer && !IsABot (pPlayer))
      return; // skip real players if in observer mode

   sound_path[0] = 0; // reset the sound path first
   player_index = PlayerIndexOf (pPlayer); // get the player index
   sprintf (player_weapon, WeaponModelOf (pPlayer)); // get the player's weapon name
   playerbuttons = InputButtonsOf (pPlayer); // get the player's input buttons

   // given the weapon this player is holding in its hands, select the corresponding fire sound
   if ((stricmp (player_weapon, "models/p_ak47.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/ak47-%d.wav", RandomInteger (1, 2));
   else if ((stricmp (player_weapon, "models/p_aug.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/aug-1.wav");
   else if ((stricmp (player_weapon, "models/p_awm.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/awp1.wav");
   else if ((stricmp (player_weapon, "models/p_c4.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/c4_plant.wav");
   else if ((stricmp (player_weapon, "models/p_deagle.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/deagle-%d.wav", RandomInteger (1, 2));
   else if ((stricmp (player_weapon, "models/p_elite.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/elite_fire.wav");
   else if ((stricmp (player_weapon, "models/p_fiveseven.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/fiveseven-1.wav");
   else if ((stricmp (player_weapon, "models/p_g3sg1.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/g3sg1-1.wav");
   else if ((stricmp (player_weapon, "models/p_glock18.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      if (WeaponModeOf (pPlayer) == CS_WEAPON_GLOCK18_BURST)
         sprintf (sound_path, "weapons/glock18-1.wav"); // burst mode
      else
         sprintf (sound_path, "weapons/glock18-2.wav"); // semi-automatic mode
   else if ((stricmp (player_weapon, "models/p_m249.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/m249-%d.wav", RandomInteger (1, 2));
   else if ((stricmp (player_weapon, "models/p_g3sg1.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/g3sg1-1.wav");
   else if ((stricmp (player_weapon, "models/p_m3.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/m3-1.wav");
   else if ((stricmp (player_weapon, "models/p_m3super90.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/m3-1.wav");
   else if ((stricmp (player_weapon, "models/p_m4a1.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      if (WeaponModeOf (pPlayer) == CS_WEAPON_M4A1_SILENCED)
         sprintf (sound_path, "weapons/m4a1-1.wav"); // silenced mode
      else
         sprintf (sound_path, "weapons/m4a1_unsil-%d.wav", RandomInteger (1, 2)); // unsilenced mode
   else if ((stricmp (player_weapon, "models/p_mac10.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/mac10-1.wav");
   else if ((stricmp (player_weapon, "models/p_mp5.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/mp5-%d.wav", RandomInteger (1, 2));
   else if ((stricmp (player_weapon, "models/p_p228.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/p228-1.wav");
   else if ((stricmp (player_weapon, "models/p_p90.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/p90-1.wav");
   else if ((stricmp (player_weapon, "models/p_scout.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/scout_fire-1.wav");
   else if ((stricmp (player_weapon, "models/p_scout.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/scout_fire-1.wav");
   else if ((stricmp (player_weapon, "models/p_sg550.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/sg550-1.wav");
   else if ((stricmp (player_weapon, "models/p_sg552.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/sg552-%d.wav", RandomInteger (1, 2));
   else if ((stricmp (player_weapon, "models/p_tmp.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/tmp-%d.wav", RandomInteger (1, 2));
   else if ((stricmp (player_weapon, "models/p_ump45.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/ump45-1.wav");
   else if ((stricmp (player_weapon, "models/p_usp.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      if (WeaponModeOf (pPlayer) == CS_WEAPON_USP_SILENCED)
         sprintf (sound_path, "weapons/usp%d.wav", RandomInteger (1, 2)); // silenced mode
      else
         sprintf (sound_path, "weapons/usp_unsil-1.wav"); // unsilenced mode
   else if ((stricmp (player_weapon, "models/p_xm1014.mdl") == 0) && (playerbuttons.f_fire1_time == CurrentTime ()))
      sprintf (sound_path, "weapons/xm1014-1.wav");

   // have we something to play (i.e, a bullet that makes noise and pops ricochets up) ?
   if (sound_path[0] != 0)
   {
      DispatchSound (sound_path, GunOriginOf (pPlayer), 1.0, ATTN_NORM); // make the bots hear it

      // did this player's last traceline hit something AND it is not a player ?
      if ((players[player_index].prev_tr.flFraction < 1.0) && !IsAPlayer (players[player_index].prev_tr.pHit))
      {
         // ask the engine for the texture name of the bullet hit point
         texture_name = TextureNameOn (players[player_index].prev_tr.pHit, GunOriginOf (pPlayer), ViewAnglesOf (pPlayer));

         // did the engine fail reporting the texture ?
         if (texture_name == NULL)
            texture_name = textures[0].name; // take the default texture as replacement

         // get texture from the bullet hit point
         pHitTexture = FindTextureByName (texture_name);

         // given the type of texture hit, prepare a different ricochet sound file
         switch (pHitTexture->type)
         {
            case CHAR_TEX_CONCRETE:
               strcpy (sound_name, "weapons/ric_conc-%d.wav");
               count = 2; // there are 2 different concrete ricochet sounds
               break;
            case CHAR_TEX_METAL:
               strcpy (sound_name, "weapons/ric_metal-%d.wav");
               count = 2; // there are 4 different metal ricochet sounds
               break;
            default:
               strcpy (sound_name, "weapons/ric%d.wav");
               count = 5; // there are 5 different ricochet sounds
               break;
         }

         // now build a random sound path amongst the count
         sprintf (sound_path, sound_name, RandomInteger (1, count));

         // bring this ricochet sound to the bots' ears
         DispatchSound (sound_path, (vector) players[player_index].prev_tr.vecEndPos, 0.9, ATTN_NORM);
      }
   }

   return;
}


void LoadSymbols (const char *filename)
{
   // the purpose of this function is to perfect the bot DLL interfacing. Having all the
   // MOD entities listed and linked to their proper function with LINK_ENTITY_TO_FUNC is
   // not enough, procs are missing, and that's the reason why most bot DLLs don't allow
   // to run single player games. This function loads the symbols in the game DLL by hand,
   // strips their MSVC-style case mangling, and builds an exports array which supercedes
   // the one the engine would get afterwards from the MOD DLL, which can't pass through
   // the bot DLL. This way we are sure that *nothing is missing* in the interfacing. Note
   // this is a fix for WIN32 systems only. But since UNIX systems only host dedicated
   // servers, there's no need to run single-player games on them.

   #ifdef _WIN32
   {
      FILE *fp;
      DOS_HEADER dos_header;
      LONG nt_signature;
      PE_HEADER pe_header;
      SECTION_HEADER section_header;
      OPTIONAL_HEADER optional_header;
      LONG edata_offset;
      LONG edata_delta;
      EXPORT_DIRECTORY export_directory;
      LONG name_offset;
      LONG ordinal_offset;
      LONG function_offset;
      char function_name[256], ch;
      int i, j;
      void *game_GiveFnptrsToDll;

      // reset function names array first
      for (i = 0; i < num_ordinals; i++)
         p_FunctionNames[i] = NULL;

      // open MOD DLL file in binary read mode
      fp = fopen (filename, "rb"); // can't fail to do this, since we LoadLibrary()'ed it before

      fread (&dos_header, sizeof (dos_header), 1, fp); // get the DOS header
      fseek (fp, dos_header.e_lfanew, SEEK_SET);
      fread (&nt_signature, sizeof (nt_signature), 1, fp); // get the NT signature
      fread (&pe_header, sizeof (pe_header), 1, fp); // get the PE header
      fread (&optional_header, sizeof (optional_header), 1, fp); // get the optional header

      edata_offset = optional_header.DataDirectory[0].VirtualAddress; // no edata by default
      edata_delta = 0;

      // cycle through all sections of the PE header to look for edata
      for (i = 0; i < pe_header.NumberOfSections; i++)
         if (strcmp ((char *) section_header.Name, ".edata") == 0)
         {
            edata_offset = section_header.PointerToRawData; // if found, save its offset
            edata_delta = section_header.VirtualAddress - section_header.PointerToRawData;
         }

      fseek (fp, edata_offset, SEEK_SET);
      fread (&export_directory, sizeof (export_directory), 1, fp); // get the export directory

      num_ordinals = export_directory.NumberOfNames; // save number of ordinals

      ordinal_offset = export_directory.AddressOfNameOrdinals - edata_delta; // save ordinals offset
      fseek (fp, ordinal_offset, SEEK_SET);
      p_Ordinals = (WORD *) malloc (num_ordinals * sizeof (WORD)); // allocate space for ordinals
      fread (p_Ordinals, num_ordinals * sizeof (WORD), 1, fp); // get the list of ordinals

      function_offset = export_directory.AddressOfFunctions - edata_delta; // save functions offset
      fseek (fp, function_offset, SEEK_SET);
      p_Functions = (DWORD *) malloc (num_ordinals * sizeof (DWORD)); // allocate space for functions
      fread (p_Functions, num_ordinals * sizeof (DWORD), 1, fp); // get the list of functions

      name_offset = export_directory.AddressOfNames - edata_delta; // save names offset
      fseek (fp, name_offset, SEEK_SET);
      p_Names = (DWORD *) malloc (num_ordinals * sizeof (DWORD)); // allocate space for names
      fread (p_Names, num_ordinals * sizeof (DWORD), 1, fp); // get the list of names

      // cycle through all function names and fill in the exports array
      for (i = 0; i < num_ordinals; i++)
      {
         if (fseek (fp, p_Names[i] - edata_delta, SEEK_SET) != -1)
         {
            j = 0; // start at beginning of string

            // while end of file is not reached
            while ((ch = fgetc (fp)) != EOF)
            {
               function_name[j] = ch; // store what is read in the name variable
               if (ch == 0)
                  break; // return the name with the trailing \0
               j++;
            }

            // allocate space
            p_FunctionNames[i] = (char *) malloc (strlen (function_name) + 1);

            // is this a MSVC C++ mangled name ?
            if (function_name[0] == '?')
            {
               j = 1; // skip the leading '?'

               // while the first @@ is not reached
               while (!((function_name[j] == '@') && (function_name[j + 1] == '@')))
               {
                  p_FunctionNames[i][j - 1] = function_name[j]; // store what is read in the name variable
                  if (function_name[j + 1] == 0)
                     break; // return the name
                  j++;
               }

               p_FunctionNames[i][j] = 0; // terminate string at the "@@"
            }

            // else no change needed
            else
               strcpy (p_FunctionNames[i], function_name);
         }
      }

      fclose (fp); // close MOD DLL file

      // cycle through all function names to find the GiveFnptrsToDll function
      for (i = 0; i < num_ordinals; i++)
      {
         if (strcmp ("GiveFnptrsToDll", p_FunctionNames[i]) == 0)
         {
            game_GiveFnptrsToDll = (void *) GetSymbolInDynamicLibrary (h_Library, "GiveFnptrsToDll");
            base_offset = (unsigned long) (game_GiveFnptrsToDll) - p_Functions[p_Ordinals[i]];
            break; // base offset has been saved
         }
      }
   }
   #endif
}


void FreeAllTheStuff (void)
{
   // this function is in charge of freeing all the memory space we allocated, because the DLL
   // is going to shutdown. Of course a check is made upon the space pointer we want to free not
   // to free it twice in case it would have already been done (malloc and free implementations
   // are usually so crappy they hardly ever give error messages, rather crash without warning).
   // For safety reasons, we also reset the pointer to NULL, in order not to try to access it
   // later. Yeah, this should never happen, but who knows.

   int i, j;

   // free the server variables memory space
   FreeServerVariables ();

   // do we need to free our table of exported symbols ? (only on Win32 platforms)
   #ifdef _WIN32
   {
      if (p_Ordinals)
         free (p_Ordinals); // free our...
      p_Ordinals = NULL;

      if (p_Functions)
         free (p_Functions); // ... table of...
      p_Functions = NULL;

      if (p_Names)
         free (p_Names); // ... exported symbols
      p_Names = NULL;

      for (i = 0; i < num_ordinals; i++)
      {
         if (p_FunctionNames[i])
            free (p_FunctionNames[i]); // finish freeing the table of exported symbols
         p_FunctionNames[i] = NULL;
      }
   }
   #endif

   // do we need to free the DLL library space ? (of course we do)
   if (h_Library)
      CloseDynamicLibrary (h_Library); // free the DLL library space
   h_Library = NULL;

   // do we need to free the bot's client command argv memory space ? (certainly)
   if (g_argv)
      free (g_argv); // free the bot's client command argv memory space
   g_argv = NULL;

   // do we need to free the map's walkable faces database ? (probably)
   if (map.walkfaces)
   {
      // for each walkable face slot in the array...
      for (i = 0; i < map.walkfaces_count; i++)
      {
         if (map.walkfaces[i].v_corners)
            free (map.walkfaces[i].v_corners); // free the walkable face corners...
         map.walkfaces[i].v_corners = NULL;

         if (map.walkfaces[i].v_delimiters)
            free (map.walkfaces[i].v_delimiters); // ...and free the walkable face delimiters
         map.walkfaces[i].v_delimiters = NULL;
      }

      free (map.walkfaces); // then free the walkable face memory space itself
   }
   map.walkfaces = NULL;

   // for each slot in the topological hashtable, see if we need to free something there
   for (i = 0; i < map.parallels_count; i++)
      for (j = 0; j < map.meridians_count; j++)
      {
         // do we need to free this slot in the map's topological hashtable ?
         if (map.topology[i][j].faces)
            free (map.topology[i][j].faces); // don't forget to free the hasthable data...
         map.topology[i][j].faces = NULL;
      }

   // now for each bot personality we loaded, see if we need to free its brain memory space
   for (i = 0; i < personality_count; i++)
   {
      // do we need to free this bot HAL brain's global chat dictionary ?
      if (bot_personalities[i].input_words)
      {
         // then, for each word in this dictionary...
         for (j = 0; j < (int) bot_personalities[i].input_words->size; j++)
         {
            if (bot_personalities[i].input_words->entry[j].word)
               free (bot_personalities[i].input_words->entry[j].word); // free every word
            bot_personalities[i].input_words->entry[j].word = NULL;
         }

         // if we need to free the word entries too...
         if (bot_personalities[i].input_words->entry != NULL)
            free (bot_personalities[i].input_words->entry); // ... then so do it
         bot_personalities[i].input_words->entry = NULL;

         // if we need to free the indexes too...
         if (bot_personalities[i].input_words->index != NULL)
            free (bot_personalities[i].input_words->index); // ... also free them
         bot_personalities[i].input_words->index = NULL;

         free (bot_personalities[i].input_words); // now frees the dictionary
      }
      bot_personalities[i].input_words = NULL;

      // do we need to free this bot's nav brain ?
      if (bot_personalities[i].PathMemory)
      {
         // for each slot in the bot's nav brain, check if we need to free a link
         for (j = 0; j < map.walkfaces_count; j++)
         {
            // do we need to free this link ?
            if (bot_personalities[i].PathMemory[j].links)
               free (bot_personalities[i].PathMemory[j].links); // free the link
            bot_personalities[i].PathMemory[j].links = NULL;
         }

         free (bot_personalities[i].PathMemory); // then free the navigation nodes array
      }
      bot_personalities[i].PathMemory = NULL;
   }

   return; // whoa, that was loads of stuff, eh ?
}


DLL_FUNCTIONS gFunctionTable =
{
   GameDLLInit,               //pfnGameInit
   Spawn,                     //pfnSpawn
   Think,                     //pfnThink
   Use,                       //pfnUse
   Touch,                     //pfnTouch
   Blocked,                   //pfnBlocked
   KeyValue,                  //pfnKeyValue
   Save,                      //pfnSave
   Restore,                   //pfnRestore
   SetAbsBox,                 //pfnSetAbsBox
   SaveWriteFields,           //pfnSaveWriteFields
   SaveReadFields,            //pfnSaveReadFields
   SaveGlobalState,           //pfnSaveGlobalState
   RestoreGlobalState,        //pfnRestoreGlobalState
   ResetGlobalState,          //pfnResetGlobalState
   ClientConnect,             //pfnClientConnect
   ClientDisconnect,          //pfnClientDisconnect
   ClientKill,                //pfnClientKill
   ClientPutInServer,         //pfnClientPutInServer
   ClientCommand,             //pfnClientCommand
   ClientUserInfoChanged,     //pfnClientUserInfoChanged
   ServerActivate,            //pfnServerActivate
   ServerDeactivate,          //pfnServerDeactivate
   PlayerPreThink,            //pfnPlayerPreThink
   PlayerPostThink,           //pfnPlayerPostThink
   StartFrame,                //pfnStartFrame
   ParmsNewLevel,             //pfnParmsNewLevel
   ParmsChangeLevel,          //pfnParmsChangeLevel
   GetGameDescription,        //pfnGetGameDescription - Returns string describing current .dll game.
   PlayerCustomization,       //pfnPlayerCustomization - Notifies .dll of new customization for player.
   SpectatorConnect,          //pfnSpectatorConnect - Called when spectator joins server
   SpectatorDisconnect,       //pfnSpectatorDisconnect - Called when spectator leaves the server
   SpectatorThink,            //pfnSpectatorThink - Called when spectator sends a command packet (usercmd_t)
   Sys_Error,                 //pfnSys_Error - Called when engine has encountered an error
   PM_Move,                   //pfnPM_Move
   PM_Init,                   //pfnPM_Init - Server version of player movement initialization
   PM_FindTextureType,        //pfnPM_FindTextureType
   SetupVisibility,           //pfnSetupVisibility - Set up PVS and PAS for networking for this client
   UpdateClientData,          //pfnUpdateClientData - Set up data sent only to specific client
   AddToFullPack,             //pfnAddToFullPack
   CreateBaseline,            //pfnCreateBaseline - Tweak entity baseline for network encoding, allows setup of player baselines, too.
   RegisterEncoders,          //pfnRegisterEncoders - Callbacks for network encoding
   GetWeaponData,             //pfnGetWeaponData
   CmdStart,                  //pfnCmdStart
   CmdEnd,                    //pfnCmdEnd
   ConnectionlessPacket,      //pfnConnectionlessPacket
   GetHullBounds,             //pfnGetHullBounds
   CreateInstancedBaselines,  //pfnCreateInstancedBaselines
   InconsistentFile,          //pfnInconsistentFile
   AllowLagCompensation,      //pfnAllowLagCompensation
};


extern "C" EXPORT int GetEntityAPI (DLL_FUNCTIONS *pFunctionTable, int interfaceVersion)
{
   if (!pFunctionTable || (interfaceVersion != INTERFACE_VERSION))
      return FALSE; // reliability check

   // pass engine callback function table to engine...
   memcpy (pFunctionTable, &gFunctionTable, sizeof (gFunctionTable));

   // pass other DLLs engine callbacks to function table...
   if (!(*other_GetEntityAPI) (&other_gFunctionTable, INTERFACE_VERSION))
      return FALSE;  // error initializing function table!!!

   return TRUE;
}


extern "C" EXPORT int GetNewDLLFunctions (NEW_DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion)
{
   if (other_GetNewDLLFunctions == NULL)
      return FALSE; // reliability check

   // pass other DLLs engine callbacks to function table...
   if (!(*other_GetNewDLLFunctions) (pFunctionTable, interfaceVersion))
      return FALSE;  // error initializing function table

   return TRUE;
}
