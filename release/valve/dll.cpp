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
// VALVE version
//
// dll.cpp
//

#define DEFINE_GLOBALS
#include "racc.h"


int WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
   // dynamic library entry point, can be used for uninitialization stuff. We use it only as
   // an "exit point" that tells us when ultimately to free() every memory that's freeable.

   if (fdwReason == DLL_PROCESS_DETACH)
      FreeAllTheStuff (); // free all the stuff upon DLL detaching

   return (TRUE);
}


void DLLEXPORT GiveFnptrsToDll (enginefuncs_t* pengfuncsFromEngine, globalvars_t *pGlobals)
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

   // does a metamod DLL exist for this MOD ?
   if (FileExists (METAMOD_DLL_PATH))
      h_Library = LoadLibrary (METAMOD_DLL_PATH);// if so, load the metamod DLL
   else
      h_Library = LoadLibrary (GAME_DLL_PATH); // else, load the official game DLL

   // has the DLL NOT been loaded successfully ?
   if (!h_Library)
      TerminateOnError ("RACC: Fatal - MOD DLL not found"); // if so, bomb out on error

   // load exported symbol table in the game DLL
   LoadSymbols (GAME_DLL_PATH);

   // let's get the REAL engine functions, for our own use (just copy the function table the
   // engine gives us)...

   memcpy (&g_engfuncs, pengfuncsFromEngine, sizeof (enginefuncs_t)); // have a copy of this...
   gpGlobals = pGlobals; // ...and also keep track of the engine global variables structure

   // and now we need to pass engine functions table to the game DLL (in fact it's our own
   // functions we are passing here, but the game DLL won't notice)...

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

   // ...and pass this 'modified' function table to the engine (evil grin)
   (*(GIVEFNPTRSTODLL) GetProcAddress (h_Library, "GiveFnptrsToDll")) (pengfuncsFromEngine, pGlobals);

   return; // finished, interfacing from gamedll to engine complete
}


extern "C" EXPORT int GetEntityAPI (DLL_FUNCTIONS *pFunctionTable, int interfaceVersion)
{
   // this function is called right after GiveFnptrsToDll() by the engine in the game DLL (or
   // what it BELIEVES to be the game DLL), in order to copy the list of MOD functions that can
   // be called by the engine, into a memory block pointed to by the pFunctionTable pointer
   // that is passed into this function (explanation comes straight from botman). This allows
   // the Half-Life engine to call these MOD DLL functions when it needs to spawn an entity,
   // connect or disconnect a player, call Think() functions, Touch() functions, or Use()
   // functions, etc. The bot DLL passes its OWN list of these functions back to the Half-Life
   // engine, and then calls the MOD DLL's version of GetEntityAPI to get the REAL gamedll
   // functions this time (to use in the bot code).

   // check if engine's pointer is valid and version is correct...
   if ((pFunctionTable == NULL) || (interfaceVersion != INTERFACE_VERSION))
      return (FALSE);

   // pass gamedll functions table to engine (in fact it's our own functions we are passing
   // here, but the engine won't notice)...

   pFunctionTable->pfnGameInit = GameDLLInit;
   pFunctionTable->pfnSpawn = Spawn;
   pFunctionTable->pfnThink = Think;
   pFunctionTable->pfnUse = Use;
   pFunctionTable->pfnTouch = Touch;
   pFunctionTable->pfnBlocked = Blocked;
   pFunctionTable->pfnKeyValue = KeyValue;
   pFunctionTable->pfnSave = Save;
   pFunctionTable->pfnRestore = Restore;
   pFunctionTable->pfnSetAbsBox = SetAbsBox;
   pFunctionTable->pfnSaveWriteFields = SaveWriteFields;
   pFunctionTable->pfnSaveReadFields = SaveReadFields;
   pFunctionTable->pfnSaveGlobalState = SaveGlobalState;
   pFunctionTable->pfnRestoreGlobalState = RestoreGlobalState;
   pFunctionTable->pfnResetGlobalState = ResetGlobalState;
   pFunctionTable->pfnClientConnect = ClientConnect;
   pFunctionTable->pfnClientDisconnect = ClientDisconnect;
   pFunctionTable->pfnClientKill = ClientKill;
   pFunctionTable->pfnClientPutInServer = ClientPutInServer;
   pFunctionTable->pfnClientCommand = ClientCommand;
   pFunctionTable->pfnClientUserInfoChanged = ClientUserInfoChanged;
   pFunctionTable->pfnServerActivate = ServerActivate;
   pFunctionTable->pfnServerDeactivate = ServerDeactivate;
   pFunctionTable->pfnPlayerPreThink = PlayerPreThink;
   pFunctionTable->pfnPlayerPostThink = PlayerPostThink;
   pFunctionTable->pfnStartFrame = StartFrame;
   pFunctionTable->pfnParmsNewLevel = ParmsNewLevel;
   pFunctionTable->pfnParmsChangeLevel = ParmsChangeLevel;
   pFunctionTable->pfnGetGameDescription = GetGameDescription;
   pFunctionTable->pfnPlayerCustomization = PlayerCustomization;
   pFunctionTable->pfnSpectatorConnect = SpectatorConnect;
   pFunctionTable->pfnSpectatorDisconnect = SpectatorDisconnect;
   pFunctionTable->pfnSpectatorThink = SpectatorThink;
   pFunctionTable->pfnSys_Error = Sys_Error;
   pFunctionTable->pfnPM_Move = PM_Move;
   pFunctionTable->pfnPM_Init = PM_Init;
   pFunctionTable->pfnPM_FindTextureType = PM_FindTextureType;
   pFunctionTable->pfnSetupVisibility = SetupVisibility;
   pFunctionTable->pfnUpdateClientData = UpdateClientData;
   pFunctionTable->pfnAddToFullPack = AddToFullPack;
   pFunctionTable->pfnCreateBaseline = CreateBaseline;
   pFunctionTable->pfnRegisterEncoders = RegisterEncoders;
   pFunctionTable->pfnGetWeaponData = GetWeaponData;
   pFunctionTable->pfnCmdStart = CmdStart;
   pFunctionTable->pfnCmdEnd = CmdEnd;
   pFunctionTable->pfnConnectionlessPacket = ConnectionlessPacket;
   pFunctionTable->pfnGetHullBounds = GetHullBounds;
   pFunctionTable->pfnCreateInstancedBaselines = CreateInstancedBaselines;
   pFunctionTable->pfnInconsistentFile = InconsistentFile;
   pFunctionTable->pfnAllowLagCompensation = AllowLagCompensation;

   // and now we need to get the REAL gamedll functions, for our own use (call GetEntityAPI()
   // in the game DLL and have it fill in our functions table)...

   // was the call NOT successful ?
   if (!(*(GETENTITYAPI) GetProcAddress (h_Library, "GetEntityAPI")) (&other_gFunctionTable, INTERFACE_VERSION))
      return (FALSE);  // error initializing function table!!!

   return (TRUE); // finished, interfacing from engine to gamedll complete
}


extern "C" EXPORT int GetNewDLLFunctions (NEW_DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion)
{
   // it appears that an extra function table has been added in the engine to gamedll interface
   // since the date where the first enginefuncs table standard was frozen. These ones are
   // facultative and we don't hook them, but since some MODs might be featuring it, we have to
   // pass them too, else the DLL interfacing wouldn't be complete and the game possibly wouldn't
   // run properly.

   static GETNEWDLLFUNCTIONS other_GetNewDLLFunctions = NULL;
   static bool missing = FALSE;

   // if the new DLL functions interface has been formerly reported as missing, give up
   if (missing)
      return (FALSE);

   // do we NOT know if the new DLL functions interface is provided ? if so, look for its address
   if (other_GetNewDLLFunctions == NULL)
      other_GetNewDLLFunctions = (GETNEWDLLFUNCTIONS) GetProcAddress (h_Library, "GetNewDLLFunctions");

   // have we NOT found it ?
   if (other_GetNewDLLFunctions == NULL)
   {
      missing = TRUE; // then mark it as missing, no use to look for it again in the future
      return (FALSE); // and give up
   }

   // else call the function that provides the new DLL functions interface on request
   return (!(*other_GetNewDLLFunctions) (pFunctionTable, interfaceVersion));
}


int DLLEXPORT Server_GetBlendingInterface (int version, struct sv_blending_interface_s **ppinterface, struct engine_studio_api_s *pstudio, float (*rotationmatrix)[3][4], float (*bonetransform)[MAXSTUDIOBONES][3][4])
{
   // this function synchronizes the studio model animation blending interface (i.e, what parts
   // of the body move, which bones, which hitboxes and how) between the server and the game DLL.
   // some MODs can be using a different hitbox scheme than the standard one.

   static SERVER_GETBLENDINGINTERFACE other_Server_GetBlendingInterface = NULL;
   static bool missing = FALSE;

   // if the blending interface has been formerly reported as missing, give up
   if (missing)
      return (FALSE);

   // do we NOT know if the blending interface is provided ? if so, look for its address
   if (other_Server_GetBlendingInterface == NULL)
      other_Server_GetBlendingInterface = (SERVER_GETBLENDINGINTERFACE) GetProcAddress (h_Library, "Server_GetBlendingInterface");

   // have we NOT found it ?
   if (!other_Server_GetBlendingInterface)
   {
      missing = TRUE; // then mark it as missing, no use to look for it again in the future
      return (FALSE); // and give up
   }

   // else call the function that provides the blending interface on request
   return ((other_Server_GetBlendingInterface) (version, ppinterface, pstudio, rotationmatrix, bonetransform));
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
   server.is_multiplayer = (CVAR_GET_FLOAT ("deathmatch") > 0);

   // only init bots if we are in multiplayer mode
   if (server.is_multiplayer && !DebugLevel.is_paused)
   {
      strcpy (server.mod_name, GetModName ()); // set MOD name
      server.max_clients = &gpGlobals->maxClients; // remember the maximum # of clients
      server.min_bots = 0; // initialize the minimal number of bots to zero (no minimum)
      server.max_bots = -1; // initialize the maximal number of bots to -1 (no maximum)
      server.bot_chat_mode = BOT_CHAT_TEXTAUDIO; // initialize the bot chat mode to text+audio
      server.is_autofill = TRUE; // initialize server filling (bots filling up server) to TRUE
      server.is_internetmode = TRUE; // initialize internet mode (bots joining/leaving) to TRUE
      server.is_dedicated = (IS_DEDICATED_SERVER () > 0); // let's see if this is a DS or not
      server.developer_level = CVAR_GET_FLOAT ("developer"); // initialize the developer level
      server.msec_method = METHOD_RICH; // choose a frame duration estimation method
      server.time = &gpGlobals->time; // and set the clock up

      MakeVersion(); // build the bot version string and welcome text
      InitGameLocale (); // get the game language
      InitLogFile (); // initialize the RACC log file
      InitPlayerBones (); // build the player bones database
      InitWeapons (); // build the weapons database
      InitRicochetSounds (); // build the ricochet sounds database

      pfnAddServerCommand ("racc", ServerCommand); // register a new server command: racc
   }

   (*other_gFunctionTable.pfnGameInit) ();
}


int Spawn (edict_t *pent)
{
   // this function asks the game DLL to spawn (i.e, give a physical existence in the virtual
   // world, in other words to 'display') the entity pointed to by pEntity in the game. The
   // Spawn() function is one of the functions any entity is supposed to have in the game DLL,
   // and any MOD is supposed to implement one for each of its entities.

   // only process bots if we are in multiplayer mode
   if (server.is_multiplayer && !DebugLevel.is_paused)
   {
      if (strcmp (STRING (pent->v.classname), "worldspawn") == 0)
         pWorldEntity = pent; // keep track of the world entity when it spawns (it's spawned 1st)

      else if (strncmp (STRING (pent->v.classname), "func_door", 9) == 0)
         SpawnDoor (pent); // save doors origins
   }

   return (*other_gFunctionTable.pfnSpawn) (pent);
}


void Think (edict_t *pent)
{
   // this function asks the game DLL to make the entity pointed to by pEntity 'think' (by
   // calling the appropriate function in the game DLL. The Think() function is another one of
   // the functions any entity is supposed to have in the game DLL, and any MOD is supposed to
   // implement one for each of its entities.

   (*other_gFunctionTable.pfnThink) (pent);
}


void Use (edict_t *pentUsed, edict_t *pentOther)
{
   // this function is called when two entities interact upon another (using a special "use"
   // flag). For example, a player is likely to press a switch for, let's say, opening a door or
   // raising an elevator. When the IN_USE flag is set in the player's input buttons structure,
   // and some usable entity is in range, this function is called. The pentUsed pointer
   // represents the entity being used (button, switch, etc.), whereas the pentOther pointer
   // points to the entity taking the action (usually a player).

   (*other_gFunctionTable.pfnUse) (pentUsed, pentOther);
}


void Touch (edict_t *pentTouched, edict_t *pentOther)
{
   // this function is called when two entities' bounding boxes enter in collision. For example,
   // when a player walks upon a gun, the player entity bounding box collides to the gun entity
   // bounding box, and the result is that this function is called. It is used by the game for
   // taking the appropriate action when such an event occurs (in our example, the player who
   // is walking upon the gun will "pick it up"). Entities that "touch" others are usually
   // entities having a velocity, as it is assumed that static entities (entities that don't
   // move) will never touch anything. Hence, in our example, the pentTouched will be the gun
   // (static entity), whereas the pentOther will be the player (as it is the one moving). When
   // the two entities both have velocities, for example two players colliding, this function
   // is called twice, once for each entity moving.

   (*other_gFunctionTable.pfnTouch) (pentTouched, pentOther);
}


void Blocked (edict_t *pentBlocked, edict_t *pentOther)
{
   // some entities may be blocked by others. It's the case for elevators, doors, trains. For
   // example, if someone is standing still on the way of a bobbing platform, the platform will
   // hit it and go back in the opposite direction. This function is called when an entity is
   // blocked by another one, for the game DLL to take the appropriate actions. The pentBlocked
   // pointer represents the entity being blocked from movement or action (elevator, door, etc),
   // whereas the pentOther pointer points to the blocking entity (usually a player, but can be
   // a pushable crate or any other entity).

   (*other_gFunctionTable.pfnBlocked) (pentBlocked, pentOther);
}


void KeyValue (edict_t *pentKeyvalue, KeyValueData *pkvd)
{
   // this function is called when the game requests a pointer to some entity's keyvalue data.
   // The keyvalue data is held in each entity's infobuffer (basically a char buffer where each
   // game DLL can put the stuff it wants) under - as it says - the form of a key/value pair. A
   // common example of key/value pair is the "model", "(name of player model here)" one which
   // is often used for client DLLs to display player characters with the right model (else they
   // would all have the dull "models/player.mdl" one). The entity for which the keyvalue data
   // pointer is requested is pentKeyvalue, the pointer to the keyvalue data structure pkvd.

   (*other_gFunctionTable.pfnKeyValue) (pentKeyvalue, pkvd);
}


void Save (edict_t *pent, SAVERESTOREDATA *pSaveData)
{
   // this function is probably called in single-player games when the player wants to save its
   // game, although I've not had a chance to verify that. It looks explicit, though.

   (*other_gFunctionTable.pfnSave) (pent, pSaveData);
}


int Restore (edict_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity)
{
   // this function is probably called in single-player games when the player wants to restore
   // a saved game, although I've not had a chance to verify that. It looks a little less
   // explicit than the former one - I have no idea of the purpose of "globalEntity".

   return (*other_gFunctionTable.pfnRestore) (pent, pSaveData, globalEntity);
}


void SetAbsBox (edict_t *pent)
{
   // this function tells the game DLL to send a network message to clients in order to update
   // the bounding box of the entity pointed to by pent, because something changed it in the
   // engine. If that wasn't done, the physics calculations would be different between the server
   // and the clients (especially the collisions). It's the same for much of the data lying in
   // any entity's entvars: if someone changes it on the server, he must be certain that the
   // clients will get notified of it somehow.

   (*other_gFunctionTable.pfnSetAbsBox) (pent);
}


void SaveWriteFields (SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount)
{
   // this function seems to have something to do with the save/restore game process in single
   // player games, but I really don't see it. If someone has info, I'd like to hear it.

   (*other_gFunctionTable.pfnSaveWriteFields) (pSaveData, pname, pBaseData, pFields, fieldCount);
}


void SaveReadFields (SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount)
{
   // this function seems to have something to do with the save/restore game process in single
   // player games, but I really don't see it. If someone has info, I'd like to hear it.

   (*other_gFunctionTable.pfnSaveReadFields) (pSaveData, pname, pBaseData, pFields, fieldCount);
}


void SaveGlobalState (SAVERESTOREDATA *pSaveData)
{
   // this function seems to have something to do with the save/restore game process in single
   // player games, but I really don't see it. If someone has info, I'd like to hear it.

   (*other_gFunctionTable.pfnSaveGlobalState) (pSaveData);
}


void RestoreGlobalState (SAVERESTOREDATA *pSaveData)
{
   // this function seems to have something to do with the save/restore game process in single
   // player games, but I really don't see it. If someone has info, I'd like to hear it.

   (*other_gFunctionTable.pfnRestoreGlobalState) (pSaveData);
}


void ResetGlobalState (void)
{
   // this function seems to have something to do with the save/restore game process in single
   // player games, but I really don't see it. If someone has info, I'd like to hear it.

   (*other_gFunctionTable.pfnResetGlobalState) ();
}


int ClientConnect (edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128])
{ 
   // this function is called in order to tell the MOD DLL that a client attempts to connect the
   // game. The entity pointer of this client is pClient, the name under which he connects is
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
   if (server.is_multiplayer && !DebugLevel.is_paused)
   {
      // is this client the listen server client or a local client ?
      if ((strcmp (pszAddress, "loopback") == 0) || (strcmp (pszAddress, "127.0.0.1") == 0))
         pListenserverEntity = pEntity; // save its edict to pListenserverEntity

      // else are we reaching the max player count with this client ?
      else if (player_count + 1 == *server.max_clients)
         server.bot_check_time = *server.time; // see if we need to disconnect a bot to allow future connections
   }

   return (*other_gFunctionTable.pfnClientConnect) (pEntity, pszName, pszAddress, szRejectReason);
}


void ClientDisconnect (edict_t *pEntity)
{
   // this function is called whenever a client is VOLUNTARILY disconnected from the server,
   // either because the client dropped the connection, or because the server dropped him from
   // the game (latency timeout). The effect is the freeing of a client slot on the server. Note
   // that clients and bots disconnected because of a level change NOT NECESSARILY call this
   // function, because in case of a level change, it's a server shutdown, and not a normal
   // disconnection. I find that completely stupid, but that's it. Anyway it's time to update
   // the bots and players counts, and in case the client disconnecting is a bot, to back its
   // brain(s) up to disk. We also try to notice when a listenserver client disconnects, so as
   // to reset his entity pointer for safety. There are still a few server frames to go once a
   // listen server client disconnects, and we don't want to send him any sort of message then.

   // only process bots if we are in multiplayer mode
   if (server.is_multiplayer && !DebugLevel.is_paused)
   {
      int player_index;
      
      player_index = ENTINDEX (pEntity) - 1; // get this player's index
      bots[player_index].is_active = FALSE; // mark this bot slot as free

      // was this client a bot ?
      if (pEntity->v.flags & FL_THIRDPARTYBOT)
      {
         if (!FNullEnt (bots[player_index].pIllumination))
            bots[player_index].pIllumination->v.flags |= FL_KILLME; // kill its light entity
         BotShutdownPathMachine (&bots[player_index]); // shutdown our bot's pathmachine
         BotNavSaveBrain (&bots[player_index]); // save our bot's nav brain
         BotHALSaveBrain (&bots[player_index]); // save our bot's HAL brain
         bot_count--; // decrement the bot count as we know this bot is disconnecting
      }

      memset (&players[player_index], 0, sizeof (players[player_index])); // reset his structure
      players[player_index].pEntity = NULL; // clear his entity pointer
      player_count--; // decrement the player count as we know this client is disconnected
   }

   (*other_gFunctionTable.pfnClientDisconnect) (pEntity);
}


void ClientKill (edict_t *pEntity)
{
   // this function forcibly kills a living player, by a server-side decision. The player is
   // usually marked as "killed by world" or "killed by self", depending on how the MOD code
   // handles ClientKill() calls. Such a function is usefull for calling when one wants to kill
   // everybody in a round, for example. It is also called when players suicide themselves,
   // using the "kill" client command (in this case, it does a check not to allow players to
   // suicide too often).

   (*other_gFunctionTable.pfnClientKill) (pEntity);
}


void ClientPutInServer (edict_t *pEntity)
{
   // this function is called once a just connected client actually enters the game, after
   // having downloaded and synchronized its resources with the of the server's. It's the
   // perfect place to hook for client connecting, since a client can always try to connect
   // passing the ClientConnect() step, and not be allowed by the server later (because of a
   // latency timeout or whatever reason). We can here keep track of both bots and players
   // counts on occurence, since bots connect the server just like the way normal client do,
   // and their third party bot flag is already supposed to be set then. If it's a bot which
   // is connecting, we also have to awake its brain(s) by reading them from the disk.

   // only process bots if we are in multiplayer mode
   if (server.is_multiplayer && !DebugLevel.is_paused)
   {
      int player_index = ENTINDEX (pEntity) - 1;

      // is this client a bot ?
      if (pEntity->v.flags & FL_THIRDPARTYBOT)
      {
         BotHALLoadBrain (&bots[player_index]); // load this bot's HAL brain
         BotNavLoadBrain (&bots[player_index]); // load this bot's nav brain
         BotInitPathMachine (&bots[player_index]); // initialize this bot's pathmachine
         bot_count++; // increment the bot count as we are certain now this bot is connected
      }

      memset (&players[player_index], 0, sizeof (players[player_index])); // reset his structure
      players[player_index].pEntity = pEntity; // save his entity
      players[player_index].welcome_time = *server.time + 3.0; // send welcome message in 3 sec
      player_count++; // increment the player count as we are certain now this client is connected
   }

   (*other_gFunctionTable.pfnClientPutInServer) (pEntity);
}


void ClientCommand (edict_t *pEntity)
{
   // this function is called whenever the client whose player entity is pClient issues a client
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
   if (server.is_multiplayer && !isFakeClientCommand && !DebugLevel.is_paused)
   {
      char pcmd[129]; // no way, I hate pointers...
      char arg1[129];
      char arg2[129];

      strcpy (pcmd, CMD_ARGV (0));
      strcpy (arg1, CMD_ARGV (1));
      strcpy (arg2, CMD_ARGV (2));

      // listenserver-only allowed commands
      if (pEntity == pListenserverEntity)
      {
         if (strcmp (pcmd, "botcontrol") == 0)
         {
            if ((arg1 != NULL) && (*arg1 != 0))
               for (int i = 0; i < RACC_MAX_CLIENTS; i++)
                  if (strcmp (STRING (bots[i].pEdict->v.netname), arg1) == 0) // find the bot we want
                  {
                     if (bots[i].is_controlled)
                        bots[i].is_controlled = FALSE;
                     else
                        bots[i].is_controlled = TRUE;
                     printf ("BOT %s is now %s\n", STRING (bots[i].pEdict->v.netname), (bots[i].is_controlled ? "PLAYER CONTROLLED" : "AUTONOMOUS"));
                     break;
                  }
            return;
         }
         else if (strcmp (pcmd, "here") == 0)
         {
            v_pathdebug_from = pListenserverEntity->v.origin;
            ServerConsole_printf ("Pathfinder debug start point stored as (%.1f, %.1f, %.1f)\n", v_pathdebug_from.x, v_pathdebug_from.y, v_pathdebug_from.z);
            return;
         }
         else if (strcmp (pcmd, "there") == 0)
         {
            v_pathdebug_to = pListenserverEntity->v.origin;
            ServerConsole_printf ("Pathfinder debug goal point stored as (%.1f, %.1f, %.1f)\n", v_pathdebug_to.x, v_pathdebug_to.y, v_pathdebug_to.z);
            return;
         }
         else if (strcmp (pcmd, "findpath") == 0)
         {
            for (int i = 0; i < RACC_MAX_CLIENTS; i++)
               if (bots[i].is_active && IsValidPlayer (bots[i].pEdict))
               {
                  ServerConsole_printf ("BOT %s finds path from (%.1f, %.1f, %.1f) to (%.1f, %.1f, %.1f)\n", STRING (bots[i].pEdict->v.netname), v_pathdebug_from.x, v_pathdebug_from.y, v_pathdebug_from.z, v_pathdebug_to.x, v_pathdebug_to.y, v_pathdebug_to.z);
                  BotFindPathFromTo (&bots[i], v_pathdebug_from, v_pathdebug_to, TRUE);
               }
            return;
         }
         else if (strcmp (pcmd, "callbots") == 0)
         {
            for (int i = 0; i < RACC_MAX_CLIENTS; i++)
               if (bots[i].is_active && IsValidPlayer (bots[i].pEdict))
               {
                  ServerConsole_printf ("BOT %s finds path to %s\n", STRING (bots[i].pEdict->v.netname), STRING (pListenserverEntity->v.netname));
                  BotFindPathTo (&bots[i], pListenserverEntity->v.origin, TRUE);
               }
            return;
         }
         else if (strcmp (pcmd, "viewsector") == 0)
         {
            sector_t *pSector = SectorUnder (pListenserverEntity);
            ServerConsole_printf ("Drawing sector, %d walkfaces involved\n", pSector->faces_count);
            UTIL_DrawSector (pListenserverEntity, pSector, 100, 255, 0, 0);
            for (int i = 0; i < pSector->faces_count; i++)
               UTIL_DrawWalkface (pListenserverEntity, pSector->faces[i], 100, 255, 255, 255);
            return;
         }
         else if (strcmp (pcmd, "viewlinks") == 0)
         {
            walkface_t *pWalkface = WalkfaceUnder (pListenserverEntity);
            for (int i = 0; i < RACC_MAX_CLIENTS; i++)
               if (bots[i].is_active && IsValidPlayer (bots[i].pEdict))
               {
                  navnode_t *pNavnode = &bots[i].BotBrain.PathMemory[WalkfaceIndexOf (pWalkface)];
                  ServerConsole_printf ("Drawing navlinks, %d entrypoints involved\n", pNavnode->links_count);
                  for (int j = 0; j < pNavnode->links_count; j++)
                  {
                     ServerConsole_printf ("link #%d from walkface %d\n", j, WalkfaceIndexOf (pNavnode->links[j].node_from->walkface));
                     UTIL_DrawNavlink (pListenserverEntity, &pNavnode->links[j], 30);
                  }
               }
            return;
         }
      }

      // both client and listenserver allowed commands
      if ((strcmp (pcmd, "help") == 0) || (strcmp (pcmd, "?") == 0))
      {
         CLIENT_PRINTF (pEntity, print_console, racc_welcometext);
         CLIENT_PRINTF (pEntity, print_console, "\n  -- Available client commmands:\n");
         CLIENT_PRINTF (pEntity, print_console, "botuse - Make the nearest teammate bot follow you\n");
         CLIENT_PRINTF (pEntity, print_console, "botstop - Make the nearest teammate bot in use stop following you\n");
         CLIENT_PRINTF (pEntity, print_console, "botstay - Make the nearest teammate bot stay in position\n");
         return;
      }
      else if (strcmp (pcmd, "botuse") == 0)
      {
         int index = UTIL_GetNearestUsableBotIndex (pEntity); // find the nearest usable bot

         if (index != -1)
         {
            bots[index].BotEars.bot_order = BOT_ORDER_FOLLOW; // let the bot know he has been ordered something
            bots[index].BotEars.pAskingEntity = pEntity; // remember asker
            bots[index].BotEars.f_order_time = *server.time; // remember when it was ordered
         }

         return;
      }
      else if (strcmp (pcmd, "botstop") == 0)
      {
         int index = UTIL_GetNearestUsedBotIndex (pEntity); // find the nearest used bot

         if (index != -1)
         {
            bots[index].BotEars.bot_order = BOT_ORDER_GO; // let the bot know he has been ordered something
            bots[index].BotEars.pAskingEntity = pEntity; // remember asker
            bots[index].BotEars.f_order_time = *server.time; // remember when it was ordered
         }

         return;
      }
      else if (strcmp (pcmd, "botstay") == 0)
      {
         int index = UTIL_GetNearestOrderableBotIndex (pEntity); // find the nearest orderable bot

         if (index != -1)
         {
            bots[index].BotEars.bot_order = BOT_ORDER_STAY; // let the bot know he has been ordered something
            bots[index].BotEars.pAskingEntity = pEntity; // remember asker
            bots[index].BotEars.f_order_time = *server.time; // remember when it was ordered
         }

         return;
      }
   }

   (*other_gFunctionTable.pfnClientCommand) (pEntity);
}


void ClientUserInfoChanged (edict_t *pEntity, char *infobuffer)
{
   // this function is called when a player changes model, or changes team. Occasionally it
   // enforces rules on these changes (for example, some MODs don't want to allow players to
   // change their player model). But most commonly, this function is in charge of handling
   // team changes, recounting the teams population, etc... We have no particular use for it.

   (*other_gFunctionTable.pfnClientUserInfoChanged) (pEntity, infobuffer);
}


void ServerActivate (edict_t *pEdictList, int edictCount, int clientMax)
{
   // this function is called when the server has fully loaded and is about to manifest itself
   // on the network as such. Since a mapchange is actually a server shutdown followed by a
   // restart, this function is also called when a new map is being loaded. Hence it's the
   // perfect place for doing initialization stuff for our bots, such as reading the BSP data,
   // loading the bot profiles, and drawing the world map (ie, filling the navigation hashtable).
   // Once this function has been called, the server can be considered as "running".

   // only process bots if we are in multiplayer mode
   if (server.is_multiplayer && !DebugLevel.is_paused)
   {
      // print a welcome message on the server console
      ServerConsole_printf ("\n");
      ServerConsole_printf ("   %s\n", racc_welcometext);
      ServerConsole_printf ("   This program comes with ABSOLUTELY NO WARRANTY; see license for details.\n");
      ServerConsole_printf ("   This is free software, you are welcome to redistribute it the way you want.\n");
      ServerConsole_printf ("\n");

      strcpy (server.map_name, STRING (gpGlobals->mapname)); // get map name

      // retrieve some CVARs
      server.is_teamplay = (CVAR_GET_FLOAT ("mp_teamplay") > 0);
      server.does_footsteps = (CVAR_GET_FLOAT ("mp_footsteps") > 0);

      MakeTeams (); // figure out what the teams are

      PrecacheStuff (); // precache miscellaneous stuff we need
      LookDownOnTheWorld (); // look down on the world and sort the faces and delimiters
      LoadBotProfiles (); // load profiles

      player_count = 0; // no players connected yet
      bot_count = 0; // no bots either
      server.bot_check_time = *server.time; // ...and start adding bots now
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
   if (server.is_multiplayer && !DebugLevel.is_paused)
   {
      FreeMapData (); // free the map data

      memset (&sounds, 0, sizeof (sounds)); // reset sound list
      sound_count = 0; // reset sound count

      beam_model = 0; // reset indices of models we precached
      speaker_model = 0; // reset indices of models we precached
      dummyent_model = 0; // reset indices of models we precached

      player_count = 0; // no players connected anymore
      bot_count = 0; // no bots either
      server.bot_check_time = 0; // ...and DON'T add bots anymore
   }

   (*other_gFunctionTable.pfnServerDeactivate) ();
}


void PlayerPreThink (edict_t *pPlayer)
{
   // this function is called by the engine before any client player (including bots) starts
   // his thinking cycle (ie, before the MOD's Think() function is called for this player).
   // The player physics are not computed yet by the engine when this function is called. Thus
   // it's the perfect place to fill some info in our player info structures.

   // only process bots if we are in multiplayer mode
   if (server.is_multiplayer && !DebugLevel.is_paused)
   {
      static int player_index;

      player_index = ENTINDEX (pPlayer) - 1; // get this player's index
      players[player_index].is_alive = IsAlive (pPlayer); // see if he's alive or not
   }

   (*other_gFunctionTable.pfnPlayerPreThink) (pPlayer);
}


void PlayerPostThink (edict_t *pPlayer)
{
   // this function is called by the engine after any client player (including bots) has finished
   // his thinking cycle (ie, the MOD's Think() function has been called for this player). Here
   // we use it to check whether this player has moved onto another face, and if so, update the
   // automatic pathbuilder. Also it's a good place to dispatch client sounds to the bot's ears.
   // It's also a safe place to decide whether or not to send a welcome message to new clients.
   // Then since we might have to compare something in this player's entity variables to its last
   // state one day, it's always useful to keep a local copy of his entvars.

   // only process bots if we are in multiplayer mode
   if (server.is_multiplayer && !DebugLevel.is_paused)
   {
      static int player_index;

      player_index = ENTINDEX (pPlayer) - 1; // get this player's index

      // is this player alive ?
      if (players[player_index].is_alive)
      {
         ShowTheWayAroundToBots (pPlayer); // have the bots watch out for this player's movement
         PlayClientSoundsForBots (pPlayer); // see if this client needs to dispatch sounds to the bot's ears

         // is it time to send the welcome message to this client ? (don't send if not alive yet)
         if ((players[player_index].welcome_time > 0) && (players[player_index].welcome_time < *server.time))
         {
            SendWelcomeMessage (pPlayer); // send him the welcome message
            players[player_index].welcome_time = 0; // don't do it twice
         }
      }
      else
      {
         // else player is dead, don't allow bots to track the moves of a ghost :)
         players[player_index].pFaceAtFeet = NULL;
         players[player_index].face_reachability = 0;
      }

      // remember this player's entity variables states
      memcpy (&players[player_index].prev_v, &pPlayer->v, sizeof (entvars_t));
   }

   (*other_gFunctionTable.pfnPlayerPostThink) (pPlayer);
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

   // only process bots if we are in multiplayer mode
   if (server.is_multiplayer && !DebugLevel.is_paused)
   {
      static bool prev_internetmode = TRUE;
      int bot_index;

      // cycle through all bot slots
      for (bot_index = 0; bot_index < RACC_MAX_CLIENTS; bot_index++)
      {
         if (!bots[bot_index].is_active || !IsValidPlayer (bots[bot_index].pEdict))
            continue; // skip inactive bots and invalid player slots

         // make the bot think
         BotPreThink (&bots[bot_index]);
         BotThink (&bots[bot_index]);
         BotPostThink (&bots[bot_index]);

         // if bot is allowed to quit AND it's time to quit
         if ((bots[bot_index].quit_game_time > 0)
             && (bots[bot_index].quit_game_time <= *server.time))
         {
            char servercmd[80];
            sprintf (servercmd, "kick \"%s\"\n", STRING (bots[bot_index].pEdict->v.netname));
            SERVER_COMMAND (servercmd); // let the bot disconnect
            if ((server.bot_check_time > 0) && (server.bot_check_time - 5.0 < *server.time))
               server.bot_check_time = *server.time + RANDOM_FLOAT (10.0, 30.0); // and delay
         }
      }

      // check if a bot needs to be created/removed...
      if ((server.bot_check_time > 0) && (server.bot_check_time < *server.time))
      {
         // can we add a bot AND are there less bots than the maximum # of bots ?
         if (server.is_autofill && (player_count < *server.max_clients - 1)
             && ((server.max_bots == -1) || (bot_count < server.max_bots)))
         {
            BotCreate (); // add a bot
            if (server.is_internetmode && (*server.time > 60.0))
               server.bot_check_time = *server.time + RANDOM_FLOAT (10.0, 30.0); // delay a while
            else
               server.bot_check_time = *server.time + 0.5; // delay half a second
         }

         // else if there are too many bots disconnect one from the server
         else if (((server.max_bots != -1) && (bot_count > server.max_bots))
                  || ((player_count == *server.max_clients) && (bot_count > server.min_bots)))
         {
            // cycle through all bot slots
            for (bot_index = 0; bot_index < RACC_MAX_CLIENTS; bot_index++)
            {
               // is this slot used ?
               if (bots[bot_index].is_active && IsValidPlayer (bots[bot_index].pEdict))
               {
                  char servercmd[80];
                  sprintf (servercmd, "kick \"%s\"\n", STRING (bots[bot_index].pEdict->v.netname));
                  SERVER_COMMAND (servercmd); // let the bot disconnect
                  break;
               }
            }

            server.bot_check_time = *server.time + 0.5; // should delay a while
         }

         // else if internet mode has been switched update the bots' TTLs
         if (!prev_internetmode && server.is_internetmode)
         {
            for (bot_index = 0; bot_index < RACC_MAX_CLIENTS; bot_index++)
               if (bots[bot_index].is_active && IsValidPlayer (bots[bot_index].pEdict))
               {
                  bots[bot_index].time_to_live = *server.time + RANDOM_LONG (300, 3600); // set them a TTL
                  bots[bot_index].quit_game_time = bots[bot_index].time_to_live + RANDOM_FLOAT (3.0, 7.0); // disconnect time
               }
            prev_internetmode = TRUE; // remember new internet mode flag state
            server.bot_check_time = *server.time + 10.0; // we'll check again in 10 seconds
         }
         else if (prev_internetmode && !server.is_internetmode)
         {
            for (bot_index = 0; bot_index < RACC_MAX_CLIENTS; bot_index++)
               if (bots[bot_index].is_active && IsValidPlayer (bots[bot_index].pEdict))
               {
                  bots[bot_index].time_to_live = -1; // don't set them a TTL (time to live)
                  bots[bot_index].quit_game_time = -1; // so never quit
               }
            prev_internetmode = FALSE; // remember new internet mode flag state
            server.bot_check_time = *server.time + 10.0; // we'll check again in 10 seconds
         }
      }

      EstimateNextFrameDuration (); // estimate how long the next frame will last
      server.previous_time = *server.time; // previous time gets updated at each StartFrame
      server.developer_level = CVAR_GET_FLOAT ("developer"); // also update developer level
   }

   (*other_gFunctionTable.pfnStartFrame) ();
}


void ParmsNewLevel (void)
{
   // this function seems to have something to do with level changes, but I don't know its
   // purpose. If anyone has info, I'd like to hear it.

   (*other_gFunctionTable.pfnParmsNewLevel) ();
}


void ParmsChangeLevel (void)
{
   // this function seems to have something to do with level changes, but I don't know its
   // purpose. It seems to build the list of what changes are needed in the entity list between
   // two levels in single-player games. If anyone has info, I'd like to hear it.

   (*other_gFunctionTable.pfnParmsChangeLevel) ();
}


const char *GetGameDescription (void)
{
   // this function is an utility function the engine calls in the mod DLL to have it return
   // a description of the game in a character string. It's the mod name you see when the
   // server says "DLL loaded for MOD mod_name" on bootup, for example. It is mostly useful for
   // people who write HL server browsers, to have them display what type of game is going on.

   return (*other_gFunctionTable.pfnGetGameDescription) ();
}


void PlayerCustomization (edict_t *pEntity, customization_t *pCust)
{
   // this function has something to do with players custom decals, its purpose is probably to
   // get a pointer to the calling player's customization data (which holds the info about the
   // raw .WAD file that holds the decal, among others). I haven't messed with it yet, though.

   (*other_gFunctionTable.pfnPlayerCustomization) (pEntity, pCust);
}


void SpectatorConnect (edict_t *pEntity)
{
   // this function is the pending of ClientConnect() + ClientPutInServer() for HLTV spectators.
   // Since spectators don't need to synchronize with any game physics (they are just invisible
   // cams floating around the level) their connection process is a one-step one. Calling this
   // function tells the game DLL that a spectator more has connected, to allow it to prepare a
   // cam entity to tie this spectator to. All the basic stuff with spectators (floating around,
   // connecting, leaving) is HLTV code, and as such it is engine internals.

   (*other_gFunctionTable.pfnSpectatorConnect) (pEntity);
}


void SpectatorDisconnect (edict_t *pEntity)
{
   // this function is the pending of ClientDisconnect() for HLTV spectators. It tells the game
   // DLL that a spectator left, so the game DLL can make a new use of its cam entity (either
   // reuse or destroy it, whatever). All the basic stuff with spectators is HLTV code, which
   // is engine internals.

   (*other_gFunctionTable.pfnSpectatorDisconnect) (pEntity);
}


void SpectatorThink (edict_t *pEntity)
{
   // this function is the pending of Think() for HLTV spectators. It tells the game to make
   // the cam entity associated with a spectator think. This enables the game DLL to control the
   // behaviour of HLTV spectators, which views they may use, how their interface looks like,
   // and a bunch of other things. Most MODs use the generic Half-Life spectator code, anyway.

   (*other_gFunctionTable.pfnSpectatorThink) (pEntity);
}


void Sys_Error (const char *error_string)
{
   // this function is called when the engine has encountered a fatal error, to tell clients to
   // display a nice MessageBox() with a short description of the error so that the game ends
   // not too abruptly and the players are warned that they are being bombed out of it.

   (*other_gFunctionTable.pfnSys_Error) (error_string);
}


void PM_Move (struct playermove_s *ppmove, int server)
{
   // this is the player movement code clients run to predict things when the server can't update
   // them often enough (or doesn't want to). The server runs exactly the same function for
   // moving players. There is normally no distinction between them, else client-side prediction
   // wouldn't work properly (and it doesn't work that well, already...)

   (*other_gFunctionTable.pfnPM_Move) (ppmove, server);
}


void PM_Init (struct playermove_s *ppmove)
{
   // I sure as hell don't know what this one is up to. I suspect it's one function that must be
   // called before using PM_Move() on a playermove_t structure. Probably for initializing data.
   // If anyone has info...

   (*other_gFunctionTable.pfnPM_Init) (ppmove);
}


char PM_FindTextureType (char *name)
{
   // this function returns a character describing the texture type of the texture with the 
   // specified name. If no texture with that name is found in the database, the default texture
   // type (CHAR_TEX_CONCRETE) is returned.

   return (*other_gFunctionTable.pfnPM_FindTextureType) (name);
}


void SetupVisibility (edict_t *pViewEntity, edict_t *pClient, unsigned char **pvs, unsigned char **pas)
{
   // this has nothing to do with mapper's visibility as they know it, nor with the Vis processor
   // used in compiling maps. It defines the PVS (Potentially Visible Set) and PAS (Potentially
   // Audible Set), that is, a linked list of things that belong to pClient, that pViewEntity
   // MIGHT be able to see and/or hear (but not necessarily). But at least it cleaves through a
   // good amount of data that are certain NOT to be seen/heard by pViewEntity. The optimization
   // here lies not in determining what pViewEntity DOES see or hear, but what it CANNOT (by
   // elimination). Things that are likely to be in the potential visible set are, for example,
   // entities that are just behind the corner (but on the same floor polygon as the viewer), or
   // things that are hidden by a larger model in front of them. But if one or both are by the
   // other side of the map, chances are they won't be met here. The PVS and PAS data is commonly
   // used by the engine as "starting rules" to compute in-game what players see and hear faster
   // than otherwise. We could use this info for our bots, but there's a just little problem: for
   // the PVS and PAS to be set, a real player must be around. As long as no player is visiting
   // the neighbourhood, no PVS/PAS data will be computed. This was done to ensure the player
   // would always land in the middle of action in single-player games wherever he goes, since
   // it's only when the player has approached closely enough that the monsters magically begin
   // to see and hear each other. Else they are all frozen blind and deaf, sorta 'waiting' for
   // him. Interesting for single player games, but bots are not exactly real clients for the
   // engine, and stupidly enough it won't let bots use PVS and PAS data if no player has
   // connected the server yet.

   (*other_gFunctionTable.pfnSetupVisibility) (pViewEntity, pClient, pvs, pas);
}


void UpdateClientData (const struct edict_s *ent, int sendweapons, struct clientdata_s *cd)
{
   // this function is a synchronization tool that is used periodically by the engine to tell
   // the game DLL to send player info over the network to one of its clients when it suspects
   // that this client is desynchronizing. Early bots were using it to ask the game DLL for the
   // weapon list of players (by setting sendweapons to TRUE), but most of the time having a
   // look around the ent->v.weapons bitmask is enough, since that's the place commonly used for
   // MODs to store weapon information. If it can't be read from there, catching a few network
   // messages (like in DMC) do the job better than this function anyway.

   (*other_gFunctionTable.pfnUpdateClientData) (ent, sendweapons, cd);
}


int AddToFullPack (struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet)
{
   // this function tells the game DLL that the engine wants to add updating information about
   // the specified entity in the network datagram it's about to send to its clients. Have you
   // ever wondered how the network code in Half-Life worked ? Well, the server keeps sending
   // long bursts of updating information in cycles (meaning that the updating info is arranged
   // in a particuliar order and it's the same order each time), that they call "packs", and the
   // clients only ask from time to time the info they need ; all they have to send back to the
   // server is their player's input buttons, movement and chat. Here the "pack" is getting
   // bigger as one entity is being added to the pack (a "baseline" will have to be created for
   // it, though). The parameters are a bit esoteric, but it works.

   return (*other_gFunctionTable.pfnAddToFullPack) (state, e, ent, host, hostflags, player, pSet);
}


void CreateBaseline (int player, int eindex, struct entity_state_s *baseline, struct edict_s *entity, int playermodelindex, vec3_t player_mins, vec3_t player_maxs)
{
   // put it simply, a baseline is a "slot" in the network update "pack" reserved for an entity.
   // For example, connecting players that have not spawned yet need to be reserved a baseline
   // for the clients to prepare themselves to compute predictions locally for one player more.
   // Here lies the great mystery (or voodoo magic, if you want) of client-side prediction.
   // Hiding behind this complicated terminology is the client engine's ability to drive the
   // game alone for a short while, would the connection to the server become too slow or
   // fail on occasion. Once the connection is back to normal (or half way), the client engine
   // sends an update to the server telling it for example 'hey dude, my player fired a bullet
   // half a second ago, according to my computations and the baseline of his opponent he should
   // have hit the bastard, so take that in account', then the server takes the appropriate
   // actions. That's why people are so pissed off when they believed half a second they had
   // escaped from the sniper hiding on the top roof when they dodged behind a corner, just to
   // find their death on their hiding spot, where no bullet could have caught them. It's the
   // 'homing bullets' feeling that ruins most of fast-paced FPS games based off Half-Life.
   // Thank you Valve ! The idea was good to help 56k'ers stand a chance against the low ping
   // bastards, but the medication turned out to be worse than the disease. A baseline is just
   // that, a data structure with sufficient information that is sent over the network to help
   // clients predict what the concerned entity is going to do and where it's going to be in an
   // immediate future of a few seconds. It works just like a scientist who would be calculating
   // the trajectory of a meteor to figure out whether the asteroid will hit the Earth in one
   // month or two. This function tells the game DLL to create a baseline and maintain it for
   // the player whose index is given in the first parameter.

   (*other_gFunctionTable.pfnCreateBaseline) (player, eindex, baseline, entity, playermodelindex, player_mins, player_maxs);
}


void RegisterEncoders (void)
{
   // I don't know what this function does practically. If anyone has info, I'd like to know it.

   (*other_gFunctionTable.pfnRegisterEncoders) ();
}


int GetWeaponData (struct edict_s *player, struct weapon_data_s *info)
{
   // this function visibly asks for a pointer to the weapon data of the player entity pointed
   // to by player. I suspect it could have been a better choice than UpdateClientData() by the
   // first-generation bots to read the weapon data, but now that we've found out that reading
   // the player->v.weapons bitmask flag is definitely the best way to go, it's no use for us.

   return (*other_gFunctionTable.pfnGetWeaponData) (player, info);
}


void CmdStart (const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed)
{
   // some MODs don't feel like doing like everybody else. It's the case in DMC, where players
   // don't select their weapons using a simple client command, but have to use an horrible
   // datagram like this. CmdStart() marks the start of a network packet clients send to the
   // server that holds a limited set of requests (see the usercmd_t structure for details).
   // It has been adapted for usage to HLTV spectators, who don't send ClientCommands, but send
   // all their update information to the server using usercmd's instead, it seems.

   (*other_gFunctionTable.pfnCmdStart) (player, cmd, random_seed);
}


void CmdEnd (const edict_t *player)
{
   // like the one above, CmdEnd() marks the end of a network packet clients send to the server
   // that holds a limited set of requests (see the usercmd_t structure for details).

   (*other_gFunctionTable.pfnCmdEnd) (player);
}


int ConnectionlessPacket (const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size)
{
   // This function seems to be called when the engine wants to know whether a network packet is
   // bogus or not. Looks like it returns TRUE if the packet is bogus, and FALSE otherwise. The
   // response_buffer argument is usually zeroed out but I suspect it could be used if the game
   // DLL implemented some sort of special handling for bogus packets. Usually these packets are
   // just discarded. We don't care, really.

   return (*other_gFunctionTable.pfnConnectionlessPacket) (net_from, args, response_buffer, response_buffer_size);
}


int GetHullBounds (int hullnumber, float *mins, float *maxs)
{
   // this function asks the game DLL for the volume of the hull which has the specified number.
   // There are a limited number of hulls in the game, all of which have a number, and not all
   // MODs use the same for describing the spatial occupation of an entity. For example, some
   // MODs will define a 'prone' hull, of which the engine will have to be aware for computing
   // player collisions. That's why it has to ask the game DLL for the size of each hull that is
   // used in the game. The engine asks for the hull number hullnumber, and it gets returned the
   // lower left and upper right position of it towards the center under the form of a vector
   // (float pointer).

   return (*other_gFunctionTable.pfnGetHullBounds) (hullnumber, mins, maxs);
}


void CreateInstancedBaselines (void)
{
   // like CreateBaseline() above, this function tells the game DLL that the engine is creating
   // a "generic" baseline in the network update datagram (the "pack"), for entities that are
   // not spawned at level start but are very likely to appear during the game: i.e, corpses,
   // bolts, grenades, snarks, etc. When one of such entities will be spawned and that they will
   // have to be sent to clients for updating, this baseline will be affected ("instanced") to
   // them, and released once the entity will have disappeared. I still wonder how all this
   // manages to work the way it does.

   (*other_gFunctionTable.pfnCreateInstancedBaselines) ();
}


int InconsistentFile (const edict_t *player, const char *filename, char *disconnect_message)
{
   // This function is called when one of the files that need to be checked for validity by
   // all clients before being allowed to join the game has been reported wrong. If the server
   // allows hax0rs with modified files to join still (usually the "mp_consistency" CVAR is set
   // to zero), this function returns 0 to allow the client to continue, else it returns 1 to
   // tell the engine to force immediate disconnection of the incriminated player, using the
   // optional disconnect message passed down in the disconnect_message buffer pointer.

   return (*other_gFunctionTable.pfnInconsistentFile) (player, filename, disconnect_message);
}


int AllowLagCompensation (void)
{
   // this function is called when the engine wants to know whether the game DLL implements or
   // not some sort of lag compensation (i.e, client-side prediction). It returns TRUE if the
   // client-side prediction is featured, FALSE otherwise.

   return (*other_gFunctionTable.pfnAllowLagCompensation) ();
}


int GetTeam (edict_t *pEntity)
{
   // this function returns the index number of the team the player entity pEntity belongs to.
   // if no team is found (entity not a player, or no teamplay), the function returns -1.

   static char player_model[32];

   // get this player's model
   sprintf (player_model, g_engfuncs.pfnInfoKeyValue ((*g_engfuncs.pfnGetInfoKeyBuffer) (pEntity), "model"));

   // were we unable to get the model from the infobuffer ?
   if (player_model[0] == 0)
      return (-1); // entity is not a player then, we can't know its team

   // compare the model name to each team name in the array...
   for (int i = 0; i < num_teams; i++)
      if (strcmp (player_model, team_names[i]) == 0)
         return (i); // ...and return the matching index (# of team)

   return (-1); // team not found !
}
