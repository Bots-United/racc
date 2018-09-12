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
// TFC version
//
// dll.cpp
//

#include "extdll.h"
#include "enginecallback.h"
#include "util.h"
#include "cbase.h"
#include "entity_state.h"
#include "bot_common.h"
#include "bot_specific.h"

HINSTANCE h_Library = NULL;
HGLOBAL h_global_argv = NULL;
GETENTITYAPI other_GetEntityAPI = NULL;
GETNEWDLLFUNCTIONS other_GetNewDLLFunctions = NULL;
GIVEFNPTRSTODLL other_GiveFnptrsToDll = NULL;
DLL_FUNCTIONS other_gFunctionTable;
DLL_GLOBAL const Vector g_vecZero = Vector (0, 0, 0);
WORD *p_Ordinals = NULL;
DWORD *p_Functions = NULL;
DWORD *p_Names = NULL;
char *p_FunctionNames[1024];
int num_ordinals;
unsigned long base_offset;
enginefuncs_t g_engfuncs;
globalvars_t  *gpGlobals;
char *g_argv;
bool isFakeClientCommand = 0;
int fake_arg_count = 0;
char arg[128];
char language[32];
bool is_dedicated_server = FALSE;
bool is_multiplayer_game = FALSE;
bool is_team_play = FALSE;
bool footstep_sounds_on = FALSE;
bool welcome_sent = FALSE;
int playerCount = 0;
float bot_check_time = 0;
edict_t *listenserver_edict = NULL;
edict_t *pent_info_tfdetect = NULL;
edict_t *pent_item_tfgoal = NULL;
int team_class_limits[4]; // for TFC
int team_allies[4]; // TFC bit mapped allies BLUE, RED, YELLOW, and GREEN
flag_t flags[5]; // for TFC (5 flags max)
int num_flags = 0; // for TFC
bool roundend = FALSE;
cvar_t server_command = {"racc", "", FCVAR_SERVER | FCVAR_UNLOGGED};
cvar_t racc_minbots = {"racc_minbots", "0", FCVAR_SERVER};
cvar_t racc_maxbots = {"racc_maxbots", "32", FCVAR_SERVER};
cvar_t racc_autofill = {"racc_autofill", "1", FCVAR_SERVER};
cvar_t racc_botforceteam = {"racc_botforceteam", "0", FCVAR_SERVER};
cvar_t racc_internetmode = {"racc_internetmode", "1", FCVAR_SERVER};
cvar_t racc_chatmode = {"racc_chatmode", "1", FCVAR_SERVER};
cvar_t racc_voicechatmode = {"racc_voicechatmode", "0", FCVAR_SERVER};
cvar_t racc_defaultbotnationality = {"racc_defaultbotnationality", "0", FCVAR_SERVER};
cvar_t racc_internationalmode = {"racc_internationalmode", "1", FCVAR_SERVER};
cvar_t racc_defaultbotskill = {"racc_defaultbotskill", "2", FCVAR_SERVER};
char bot_names[100][32];
char bot_skins[100][32];
char bot_logos[100][32];
int bot_nationalities[100];
int bot_skills[100];
int number_names = 0;
bot_t bots[32];
bool b_observer_mode = FALSE;
bool b_botdontshoot = FALSE;
bool b_botdontfind = FALSE;
bool b_debug_nav = FALSE;
bool b_doors_saved = FALSE;
float pause_frequency[5] = {8, 4, 2, 1, 0};
float pause_time[5][2] = {{2.4, 4.0}, {1.6, 3.0}, {1.0, 2.0}, {0.6, 1.2}, {0.2, 0.6}};
int max_armor[10] = {0, 50, 50, 200, 120, 100, 300, 150, 100, 50};
int beam_texture = 0;
int speaker_texture = 0;
int voiceicon_height = 45;
bot_weapon_t weapon_defs[MAX_WEAPONS];

void UpdateClientData (const struct edict_s *ent, int sendweapons, struct clientdata_s *cd);



// Required DLL entry point
BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
   if (fdwReason == DLL_PROCESS_DETACH)
   {
      if (h_Library)
         FreeLibrary (h_Library); // free the DLL library space

      if (p_Ordinals)
         free (p_Ordinals);
      if (p_Functions)
         free (p_Functions);
      if (p_Names)
         free (p_Names);

      for (int i = 0; i < num_ordinals; i++)
      {
         if (p_FunctionNames[i])
            free (p_FunctionNames[i]); // free the table of exported symbols
      }

      if (h_global_argv)
      {
         GlobalUnlock (h_global_argv);
         GlobalFree (h_global_argv);
      }
   }

   return TRUE;
}


void DLLEXPORT GiveFnptrsToDll (enginefuncs_t* pengfuncsFromEngine, globalvars_t *pGlobals)
{
   FILE *fp;

   // test if Metamod DLL exists, attempt to open the file
   if ((fp = fopen ("tfc/dlls/metamod.dll", "rb")) != NULL)
   {
      fclose (fp); // DLL exists, close it
      h_Library = LoadLibrary ("tfc/dlls/metamod.dll");// load Metamod METAMOD.DLL
   }
   else
      h_Library = LoadLibrary ("tfc/dlls/tfc.dll"); // load Team Fortress Classic TFC.DLL

   if (!h_Library)
      ALERT (at_error, "RACC: Fatal - MOD DLL not found");

   // load exported symbol table
   LoadSymbols ("tfc/dlls/tfc.dll");

   // get the engine functions from the engine...
   memcpy (&g_engfuncs, pengfuncsFromEngine, sizeof (enginefuncs_t));

   other_GetEntityAPI = (GETENTITYAPI) GetProcAddress (h_Library, "GetEntityAPI");
   other_GetNewDLLFunctions = (GETNEWDLLFUNCTIONS) GetProcAddress (h_Library, "GetNewDLLFunctions");
   other_GiveFnptrsToDll = (GIVEFNPTRSTODLL) GetProcAddress (h_Library, "GiveFnptrsToDll");

   gpGlobals = pGlobals;
   h_global_argv = GlobalAlloc (GMEM_SHARE, 1024);
   g_argv = (char *) GlobalLock (h_global_argv);

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
   pengfuncsFromEngine->pfnCmd_Args = Cmd_Args;
   pengfuncsFromEngine->pfnCmd_Argv = Cmd_Argv;
   pengfuncsFromEngine->pfnCmd_Argc = Cmd_Argc;
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

   // give the engine functions to the other DLL...
   (*other_GiveFnptrsToDll) (pengfuncsFromEngine, pGlobals);
}


void GameDLLInit (void)
{
   // make sure we are in multiplayer mode
   is_multiplayer_game = (CVAR_GET_FLOAT ("deathmatch") > 0);

   // only init bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      GetGameLocale (); // get Half-Life's language

      CVAR_REGISTER (&server_command); // register the racc CVAR for dedicated server commands
      CVAR_REGISTER (&racc_minbots); // register a new CVAR: racc_minbots
      CVAR_REGISTER (&racc_maxbots); // register a new CVAR: racc_maxbots
      CVAR_REGISTER (&racc_autofill); // register a new CVAR: racc_autofill
      CVAR_REGISTER (&racc_botforceteam); // register a new CVAR: racc_botforceteam
      CVAR_REGISTER (&racc_internetmode); // register a new CVAR: racc_internetmode
      CVAR_REGISTER (&racc_chatmode); // register a new CVAR; racc_chatmode
      CVAR_REGISTER (&racc_voicechatmode); // register a new CVAR; racc_voicechatmode
      CVAR_REGISTER (&racc_defaultbotnationality); // register a new CVAR: racc_defaultbotnationality
      CVAR_REGISTER (&racc_internationalmode); // register a new CVAR: racc_internationalmode
      CVAR_REGISTER (&racc_defaultbotskill); // register a new CVAR: racc_defaultbotskill

      memset (bots, 0, sizeof (bots)); // initialize the bots array of structures...

      LoadBotTextChat (); // load text chat
      LoadBotProfiles (); // load profiles

      is_team_play = (CVAR_GET_FLOAT ("mp_teamplay") > 0); // retrieve some CVARs...
      footstep_sounds_on = (CVAR_GET_FLOAT ("mp_footsteps") > 0); // retrieve some CVARs...
      is_dedicated_server = (IS_DEDICATED_SERVER () > 0); // check if we are running on a dedicated server

      // force racc_maxbots in bounds now that profiles are loaded
      if (((CVAR_GET_FLOAT ("racc_maxbots") > number_names) && (number_names > 0))
          || (CVAR_GET_FLOAT ("racc_maxbots") < 0) || (CVAR_GET_FLOAT ("racc_maxbots") > 31))
         CVAR_SET_FLOAT ("racc_maxbots", number_names); // adjust racc_maxbots to the bot list count
   }

   (*other_gFunctionTable.pfnGameInit) ();
}


int DispatchSpawn (edict_t *pent)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      // if we are spawning the world itself (worldspawn is the first entity spawned)
      if (strcmp (STRING (pent->v.classname), "worldspawn") == 0)
         PrecacheStuff (); // precache radio sounds and misc other stuff
   }

   return (*other_gFunctionTable.pfnSpawn) (pent);
}


void DispatchThink (edict_t *pent)
{
   (*other_gFunctionTable.pfnThink) (pent);
}


void DispatchUse (edict_t *pentUsed, edict_t *pentOther)
{
   (*other_gFunctionTable.pfnUse) (pentUsed, pentOther);
}


void DispatchTouch (edict_t *pentTouched, edict_t *pentOther)
{
   (*other_gFunctionTable.pfnTouch) (pentTouched, pentOther);
}


void DispatchBlocked (edict_t *pentBlocked, edict_t *pentOther)
{
   (*other_gFunctionTable.pfnBlocked) (pentBlocked, pentOther);
}


void DispatchKeyValue (edict_t *pentKeyvalue, KeyValueData *pkvd)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      static edict_t *temp_pent;
      static int flag_index;

      if (pentKeyvalue == pent_info_tfdetect)
      {
         if (strcmp(pkvd->szKeyName, "maxammo_shells") == 0) // BLUE class limits
            team_class_limits[0] = atoi (pkvd->szValue);
         else if (strcmp(pkvd->szKeyName, "maxammo_nails") == 0) // RED class limits
            team_class_limits[1] = atoi (pkvd->szValue);
         else if (strcmp(pkvd->szKeyName, "maxammo_rockets") == 0) // YELLOW class limits
            team_class_limits[2] = atoi (pkvd->szValue);
         else if (strcmp(pkvd->szKeyName, "maxammo_cells") == 0) // GREEN class limits
            team_class_limits[3] = atoi (pkvd->szValue);

         else if (strcmp(pkvd->szKeyName, "team1_allies") == 0) // BLUE allies
            team_allies[0] = atoi (pkvd->szValue);
         else if (strcmp(pkvd->szKeyName, "team2_allies") == 0) // RED allies
            team_allies[1] = atoi (pkvd->szValue);
         else if (strcmp(pkvd->szKeyName, "team3_allies") == 0) // YELLOW allies
            team_allies[2] = atoi (pkvd->szValue);
         else if (strcmp(pkvd->szKeyName, "team4_allies") == 0) // GREEN allies
            team_allies[3] = atoi (pkvd->szValue);
      }
      else if (pent_info_tfdetect == NULL)
         if ((strcmp(pkvd->szKeyName, "classname") == 0) && (strcmp(pkvd->szValue, "info_tfdetect") == 0))
            pent_info_tfdetect = pentKeyvalue;

      if (pentKeyvalue == pent_item_tfgoal)
      {
         if (strcmp(pkvd->szKeyName, "team_no") == 0)
            flags[flag_index].team_no = atoi (pkvd->szValue);

         if ((strcmp(pkvd->szKeyName, "mdl") == 0)
             && ((strcmp(pkvd->szValue, "models/flag.mdl") == 0)
                 || (strcmp(pkvd->szValue, "models/keycard.mdl") == 0)
                 || (strcmp(pkvd->szValue, "models/ball.mdl") == 0)))
         {
            flags[flag_index].mdl_match = TRUE;
            num_flags++;
         }
      }
      else if (pent_item_tfgoal == NULL)
      {
         if ((strcmp(pkvd->szKeyName, "classname") == 0)
             && (strcmp(pkvd->szValue, "item_tfgoal") == 0))
         {
            if (num_flags < 5)
            {
               pent_item_tfgoal = pentKeyvalue;

               flags[num_flags].mdl_match = FALSE;
               flags[num_flags].team_no = 0;  // any team unless specified
               flags[num_flags].edict = pentKeyvalue;

               flag_index = num_flags;  // in case the mdl comes before team_no
            }
         }
      }
      else
         pent_item_tfgoal = NULL;  // reset for non-flag item_tfgoal's

      if ((strcmp(pkvd->szKeyName, "classname") == 0)
          && ((strcmp(pkvd->szValue, "info_player_teamspawn") == 0)
              || (strcmp(pkvd->szValue, "i_p_t") == 0)))
         temp_pent = pentKeyvalue;
   }

   (*other_gFunctionTable.pfnKeyValue) (pentKeyvalue, pkvd);
}


void DispatchSave (edict_t *pent, SAVERESTOREDATA *pSaveData)
{
   (*other_gFunctionTable.pfnSave) (pent, pSaveData);
}


int DispatchRestore (edict_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity)
{
   return (*other_gFunctionTable.pfnRestore) (pent, pSaveData, globalEntity);
}


void DispatchObjectCollsionBox (edict_t *pent)
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


BOOL ClientConnect (edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128])
{ 
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      // is this client the listen server client ?
      if (strcmp (pszAddress, "loopback") == 0)
         listenserver_edict = pEntity; // save its edict to listenserver_edict

      // else are we reaching the max player count with this client ?
      else if (playerCount + 1 == gpGlobals->maxClients)
         bot_check_time = gpGlobals->time; // see if we need to disconnect a bot to allow future connections
   }

   return (*other_gFunctionTable.pfnClientConnect) (pEntity, pszName, pszAddress, szRejectReason);
}


void ClientDisconnect (edict_t *pEntity)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      int index = UTIL_GetBotIndex (pEntity);

      // is the just disconnected client one of our bots ?
      if (index != -1)
         bots[index].is_active = FALSE; // free the slot
   }

   (*other_gFunctionTable.pfnClientDisconnect) (pEntity);

   playerCount--; // decrement the player count as we know this client is disconnected
}


void ClientKill (edict_t *pEntity)
{
   (*other_gFunctionTable.pfnClientKill) (pEntity);
}


void ClientPutInServer (edict_t *pEntity)
{
   (*other_gFunctionTable.pfnClientPutInServer) (pEntity);

   playerCount++; // increment the player count as we are certain now this client is connected
}


void ClientCommand (edict_t *pEntity)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game && !isFakeClientCommand)
   {
      char pcmd[129]; // no way, I hate pointers...
      char arg1[129];
      char arg2[129];
      char msg[129];

      sprintf (pcmd, Cmd_Argv (0));
      sprintf (arg1, Cmd_Argv (1));
      sprintf (arg2, Cmd_Argv (2));

      // listenserver-only allowed commands
      if (pEntity == listenserver_edict)
      {
         if (FStrEq (pcmd, "botstat"))
         {
            if ((arg1 != NULL) && (*arg1 != 0))
               for (int i = 0; i < 32; i++)
                  if (strcmp (STRING (bots[i].pEdict->v.netname), arg1) == 0) // find the bot we want
                  {
                     char msg[512];
                     sprintf (msg, "BOT %s (skin %s) - skill:%d - team:%d - class:%d - health:%d - armor:%d - pause time:%f\n",
                                   STRING (bots[i].pEdict->v.netname),
                                   g_engfuncs.pfnInfoKeyValue ((*g_engfuncs.pfnGetInfoKeyBuffer) (bots[i].pEdict), "model"),
                                   bots[i].bot_skill,
                                   bots[i].pEdict->v.team,
                                   bots[i].pEdict->v.playerclass,
                                   bots[i].pEdict->v.health,
                                   bots[i].pEdict->v.armorvalue,
                                   bots[i].f_pause_time);
                     ClientPrint (pEntity, HUD_PRINTNOTIFY, msg);
                     break;
                  }
            return;
         }
         else if (FStrEq (pcmd, "botorder"))
         {
            if ((arg1 != NULL) && (*arg1 != 0) && (arg2 != NULL) && (*arg2 != 0))
               for (int i = 0; i < 32; i++)
                  if (strcmp (STRING (bots[i].pEdict->v.netname), arg1) == 0) // find the bot we want
                  {
                     char msg[256];
                     char command[256];
                     sprintf (command, "%s", arg2);
                     sprintf (msg, "BOT %s executes command %s\n", STRING (bots[i].pEdict->v.netname), command);
                     ClientPrint (pEntity, HUD_PRINTNOTIFY, msg);
                     FakeClientCommand (bots[i].pEdict, command);
                     break;
                  }
            return;
         }
         else if (FStrEq (pcmd, "observer"))
         {
            if ((arg1 != NULL) && (*arg1 != 0))
            {
               int temp = atoi (arg1);
               if (temp)
                  b_observer_mode = TRUE;
               else
                  b_observer_mode = FALSE;
            }
            if (b_observer_mode)
               ClientPrint (pEntity, HUD_PRINTNOTIFY, "Observer mode is ENABLED\n");
            else
               ClientPrint (pEntity, HUD_PRINTNOTIFY, "Observer mode is DISABLED\n");
            return;
         }
         else if (FStrEq (pcmd, "debug_nav"))
         {
            if ((arg1 != NULL) && (*arg1 != 0))
            {
               int temp = atoi (arg1);
               if (temp)
                  b_debug_nav = TRUE;
               else
                  b_debug_nav = FALSE;
            }
            if (b_debug_nav)
               ClientPrint (pEntity, HUD_PRINTNOTIFY, "Navigation debug mode is ENABLED\n");
            else
               ClientPrint (pEntity, HUD_PRINTNOTIFY, "Navigation debug mode is DISABLED\n");
            return;
         }
         else if (FStrEq (pcmd, "botdontfind"))
         {
            if ((arg1 != NULL) && (*arg1 != 0))
            {
               int temp = atoi (arg1);
               if (temp)
                  b_botdontfind = TRUE;
               else
                  b_botdontfind = FALSE;
            }
            if (b_botdontfind)
               ClientPrint (pEntity, HUD_PRINTNOTIFY, "Bots don't check for items mode is ENABLED\n");
            else
               ClientPrint (pEntity, HUD_PRINTNOTIFY, "Bots don't check for items mode is DISABLED\n");
            return;
         }
         else if (FStrEq (pcmd, "botdontshoot"))
         {
            if ((arg1 != NULL) && (*arg1 != 0))
            {
               int temp = atoi (arg1);
               if (temp)
                  b_botdontshoot = TRUE;
               else
                  b_botdontshoot = FALSE;
            }
            if (b_botdontshoot)
               ClientPrint (pEntity, HUD_PRINTNOTIFY, "Bots don't shoot is ENABLED\n");
            else
               ClientPrint (pEntity, HUD_PRINTNOTIFY, "Bots don't shoot is DISABLED\n");
            return;
         }
         else if (FStrEq (pcmd, "search"))
         {
            edict_t *pent = NULL;
            float radius = 500;

            ClientPrint (pEntity, HUD_PRINTNOTIFY, "Searching for VISIBLE entities in player's neighbourhood...\n");
            ClientPrint (pEntity, HUD_PRINTNOTIFY, "CLASSNAME: Origin x, y, z (distance) - BModelOrigin x, y, z (distance)\n");
            while ((pent = UTIL_FindEntityInSphere (pent, pEntity->v.origin, radius)) != NULL)
            {
               Vector v_origin = pent->v.origin;
               Vector v_bmodelorigin = VecBModelOrigin (pent);
               float f_distance = (v_origin - pEntity->v.origin).Length ();
               float f_bmodeldistance = (v_bmodelorigin - pEntity->v.origin).Length();

               if ((FVisible (v_origin, pEntity) || FVisible (v_bmodelorigin, pEntity))
                   && (FInViewCone (v_origin, pEntity) || FInViewCone (v_bmodelorigin, pEntity)))
               {
                  sprintf (msg, "%s: %.1f, %.1f, %.1f (distance %.1f) - %.1f, %.1f, %.1f (distance %.1f)\n",
                     STRING (pent->v.classname), v_origin.x, v_origin.y, v_origin.z, f_distance, v_bmodelorigin.x, v_bmodelorigin.y, v_bmodelorigin.z, f_bmodeldistance);
                  ClientPrint (pEntity, HUD_PRINTCONSOLE, msg);
               }
            }
            return;
         }
         else if (FStrEq (pcmd, "asearch"))
         {
            edict_t *pent = NULL;
            float radius = 100;

            if ((arg1 != NULL) && (*arg1 != 0))
               if ((atoi (arg1) > 0) && (atoi (arg1) <= 10000))
                  radius = atoi (arg1);

            ClientPrint (pEntity, HUD_PRINTNOTIFY, "Searching for ALL entities in player's neighbourhood...\n");
            ClientPrint (pEntity, HUD_PRINTNOTIFY, "CLASSNAME: Origin x, y, z (dist) - BModelOrigin x, y, z (dist)\n");
            while ((pent = UTIL_FindEntityInSphere (pent, pEntity->v.origin, radius)) != NULL)
            {
               Vector v_origin = pent->v.origin;
               Vector v_bmodelorigin = VecBModelOrigin (pent);
               float f_distance = (v_origin - pEntity->v.origin).Length ();
               float f_bmodeldistance = (v_bmodelorigin - pEntity->v.origin).Length();

               sprintf (msg, "%s: %.1f, %.1f, %.1f (distance %.1f) - %.1f, %.1f, %.1f (distance %.1f)\n",
                  STRING (pent->v.classname), v_origin.x, v_origin.y, v_origin.z, f_distance, v_bmodelorigin.x, v_bmodelorigin.y, v_bmodelorigin.z, f_bmodeldistance);
               ClientPrint (pEntity, HUD_PRINTCONSOLE, msg);
            }
            return;
         }
         else if (FStrEq (pcmd, "traceline"))
         {
            if ((arg1 != NULL) && (*arg1 != 0))
            {
               if ((atoi (arg1) > 0) && (atoi (arg1) <= 10000))
               {
                  TraceResult tr;
                  UTIL_MakeVectors (pEntity->v.v_angle); // build base vectors

                  sprintf (msg, "Tracing forward from player's view angles (%.1f %.1f %.1f)...\n", pEntity->v.v_angle.x, pEntity->v.v_angle.y, pEntity->v.v_angle.z);
                  ClientPrint (pEntity, HUD_PRINTNOTIFY, msg);

                  UTIL_TraceLine (pEntity->v.origin + pEntity->v.view_ofs,
                                  pEntity->v.origin + pEntity->v.view_ofs + gpGlobals->v_forward * atoi (arg1),
                                  ignore_monsters, pEntity->v.pContainingEntity, &tr);

                  UTIL_DrawBeam (pEntity, pEntity->v.origin + pEntity->v.view_ofs, pEntity->v.origin + pEntity->v.view_ofs + gpGlobals->v_forward * atoi (arg1) * tr.flFraction, 600, 10, 2, 250, 250, 250, 200, 10);

                  sprintf (msg, "distance traced: %.1f - Normal: vecPlaneNormal (%.1f, %.1f, %.1f) - Plane hit: %s\n", atoi (arg1) * tr.flFraction, tr.vecPlaneNormal.x, tr.vecPlaneNormal.y, tr.vecPlaneNormal.z, STRING (tr.pHit->v.classname));
                  ClientPrint (pEntity, HUD_PRINTCONSOLE, msg);
               }
            }
            else
               ClientPrint (pEntity, HUD_PRINTCONSOLE, "Usage: traceline <maxdist>\n");
            return;
         }
      }

      // both client and listenserver allowed commands
      if ((FStrEq (pcmd, "help")) || (FStrEq (pcmd, "?")))
      {
         ClientPrint (pEntity, HUD_PRINTNOTIFY, RACC_WELCOMETEXT);
         ClientPrint (pEntity, HUD_PRINTNOTIFY, "\n  -- Available client commmands:\n");
         ClientPrint (pEntity, HUD_PRINTNOTIFY, "botuse - Make the nearest teammate bot follow you\n");
         ClientPrint (pEntity, HUD_PRINTNOTIFY, "botstop - Make the nearest teammate bot in use stop following you\n");
         ClientPrint (pEntity, HUD_PRINTNOTIFY, "botstay - Make the nearest teammate bot stay in position\n");
         ClientPrint (pEntity, HUD_PRINTNOTIFY, "botdet5 - Make the nearest DEMOMAN bot place the plastic (5 sec. delay)\n");
         ClientPrint (pEntity, HUD_PRINTNOTIFY, "botdet20 - Make the nearest DEMOMAN bot place the plastic (20 sec. delay)\n");
         ClientPrint (pEntity, HUD_PRINTNOTIFY, "botdetstop - Make the nearest DEMOMAN bot defuse enemy's plastic\n");
         ClientPrint (pEntity, HUD_PRINTNOTIFY, "botdisguiseenemy - Make the nearest SPY bot disguise as enemy\n");
         ClientPrint (pEntity, HUD_PRINTNOTIFY, "botdisguisefriend - Make the nearest SPY bot disguise as friend\n");
         ClientPrint (pEntity, HUD_PRINTNOTIFY, "botfeign - Make the nearest SPY bot feign death without a noise\n");
         ClientPrint (pEntity, HUD_PRINTNOTIFY, "botfeignstop - Make the nearest SPY bot stop feigning death\n");
         ClientPrint (pEntity, HUD_PRINTNOTIFY, "botbuildsentry - Make the nearest ENGINEER bot build a sentry gun\n");
         ClientPrint (pEntity, HUD_PRINTNOTIFY, "botrotatesentry180 - Make the nearest ENGINEER bot turn his sentry gun back\n");
         ClientPrint (pEntity, HUD_PRINTNOTIFY, "botrotatesentry - Make the nearest ENGINEER bot turn his sentry gun\n");
         ClientPrint (pEntity, HUD_PRINTNOTIFY, "botdetsentry - Make the nearest ENGINEER bot destroy his sentry gun\n");
         ClientPrint (pEntity, HUD_PRINTNOTIFY, "botbuilddispenser - Make the nearest ENGINEER bot build a dispenser\n");
         ClientPrint (pEntity, HUD_PRINTNOTIFY, "botdetdispenser - Make the nearest ENGINEER bot destroy his dispenser\n");
         return;
      }
      else if (FStrEq (pcmd, "botuse"))
      {
         int index = UTIL_GetNearestUsableBotIndex (pEntity); // find the nearest usable bot

         if (index != -1)
         {
            bots[index].bot_order = BOT_ORDER_FOLLOW; // let the bot know he has been ordered something
            bots[index].pAskingEntity = pEntity; // remember asker
            bots[index].f_order_time = gpGlobals->time; // remember when it was ordered
         }

         return;
      }
      else if (FStrEq (pcmd, "botstop"))
      {
         int index = UTIL_GetNearestUsedBotIndex (pEntity); // find the nearest used bot

         if (index != -1)
         {
            bots[index].bot_order = BOT_ORDER_GO; // let the bot know he has been ordered something
            bots[index].pAskingEntity = pEntity; // remember asker
            bots[index].f_order_time = gpGlobals->time; // remember when it was ordered
         }

         return;
      }
      else if (FStrEq (pcmd, "botstay"))
      {
         int index = UTIL_GetNearestOrderableBotIndex (pEntity); // find the nearest orderable bot

         if (index != -1)
         {
            bots[index].bot_order = BOT_ORDER_STAY; // let the bot know he has been ordered something
            bots[index].pAskingEntity = pEntity; // remember asker
            bots[index].f_order_time = gpGlobals->time; // remember when it was ordered
         }

         return;
      }
      else if (FStrEq (pcmd, "botdet5"))
      {
         int index = UTIL_GetNearestOrderableBotIndex (pEntity);

         if (index != -1)
         {
            bots[index].bot_order = BOT_ORDER_DETONATEPLASTIC_5SECONDS; // let the bot know he has been ordered something
            bots[index].pAskingEntity = pEntity; // remember asker
            bots[index].f_order_time = gpGlobals->time; // remember when it was ordered
         }

         return;
      }
      else if (FStrEq (pcmd, "botdet20"))
      {
         int index = UTIL_GetNearestOrderableBotIndex (pEntity);

         if (index != -1)
         {
            bots[index].bot_order = BOT_ORDER_DETONATEPLASTIC_20SECONDS; // let the bot know he has been ordered something
            bots[index].pAskingEntity = pEntity; // remember asker
            bots[index].f_order_time = gpGlobals->time; // remember when it was ordered
         }

         return;
      }
      else if (FStrEq (pcmd, "botdetstop"))
      {
         int index = UTIL_GetNearestOrderableBotIndex (pEntity);

         if (index != -1)
         {
            bots[index].bot_order = BOT_ORDER_DISARMPLASTIC; // let the bot know he has been ordered something
            bots[index].pAskingEntity = pEntity; // remember asker
            bots[index].f_order_time = gpGlobals->time; // remember when it was ordered
         }

         return;
      }
      else if (FStrEq (pcmd, "botdisguiseenemy"))
      {
         int index = UTIL_GetNearestOrderableBotIndex (pEntity);

         if (index != -1)
         {
            bots[index].bot_order = BOT_ORDER_DISGUISEENEMY; // let the bot know he has been ordered something
            bots[index].pAskingEntity = pEntity; // remember asker
            bots[index].f_order_time = gpGlobals->time; // remember when it was ordered
         }

         return;
      }
      else if (FStrEq (pcmd, "botdisguisefriendly"))
      {
         int index = UTIL_GetNearestOrderableBotIndex (pEntity);

         if (index != -1)
         {
            bots[index].bot_order = BOT_ORDER_DISGUISEFRIENDLY; // let the bot know he has been ordered something
            bots[index].pAskingEntity = pEntity; // remember asker
            bots[index].f_order_time = gpGlobals->time; // remember when it was ordered
         }

         return;
      }
      else if (FStrEq (pcmd, "botfeign"))
      {
         int index = UTIL_GetNearestOrderableBotIndex (pEntity);

         if (index != -1)
         {
            bots[index].bot_order = BOT_ORDER_FEIGN; // let the bot know he has been ordered something
            bots[index].pAskingEntity = pEntity; // remember asker
            bots[index].f_order_time = gpGlobals->time; // remember when it was ordered
         }

         return;
      }
      else if (FStrEq (pcmd, "botfeignstop"))
      {
         int index = UTIL_GetNearestOrderableBotIndex (pEntity);

         if (index != -1)
         {
            bots[index].bot_order = BOT_ORDER_STOPFEIGN; // let the bot know he has been ordered something
            bots[index].pAskingEntity = pEntity; // remember asker
            bots[index].f_order_time = gpGlobals->time; // remember when it was ordered
         }

         return;
      }
      else if (FStrEq (pcmd, "botbuildsentry"))
      {
         int index = UTIL_GetNearestOrderableBotIndex (pEntity);

         if (index != -1)
         {
            bots[index].bot_order = BOT_ORDER_BUILDSENTRY; // let the bot know he has been ordered something
            bots[index].pAskingEntity = pEntity; // remember asker
            bots[index].f_order_time = gpGlobals->time; // remember when it was ordered
         }

         return;
      }
      else if (FStrEq (pcmd, "botrotatesentry180"))
      {
         int index = UTIL_GetNearestOrderableBotIndex (pEntity);

         if (index != -1)
         {
            bots[index].bot_order = BOT_ORDER_ROTATESENTRY_180DEGREES; // let the bot know he has been ordered something
            bots[index].pAskingEntity = pEntity; // remember asker
            bots[index].f_order_time = gpGlobals->time; // remember when it was ordered
         }

         return;
      }
      else if (FStrEq (pcmd, "botrotatesentry"))
      {
         int index = UTIL_GetNearestOrderableBotIndex (pEntity);

         if (index != -1)
         {
            bots[index].bot_order = BOT_ORDER_ROTATESENTRY_45DEGREES; // let the bot know he has been ordered something
            bots[index].pAskingEntity = pEntity; // remember asker
            bots[index].f_order_time = gpGlobals->time; // remember when it was ordered
         }

         return;
      }
      else if (FStrEq (pcmd, "botdetsentry"))
      {
         int index = UTIL_GetNearestOrderableBotIndex (pEntity);

         if (index != -1)
         {
            bots[index].bot_order = BOT_ORDER_DETONATESENTRY; // let the bot know he has been ordered something
            bots[index].pAskingEntity = pEntity; // remember asker
            bots[index].f_order_time = gpGlobals->time; // remember when it was ordered
         }

         return;
      }
      else if (FStrEq (pcmd, "botbuilddispenser"))
      {
         int index = UTIL_GetNearestOrderableBotIndex (pEntity);

         if (index != -1)
         {
            bots[index].bot_order = BOT_ORDER_BUILDDISPENSER; // let the bot know he has been ordered something
            bots[index].pAskingEntity = pEntity; // remember asker
            bots[index].f_order_time = gpGlobals->time; // remember when it was ordered
         }

         return;
      }
      else if (FStrEq (pcmd, "botdetdispenser"))
      {
         int index = UTIL_GetNearestOrderableBotIndex (pEntity);

         if (index != -1)
         {
            bots[index].bot_order = BOT_ORDER_DETONATEDISPENSER; // let the bot know he has been ordered something
            bots[index].pAskingEntity = pEntity; // remember asker
            bots[index].f_order_time = gpGlobals->time; // remember when it was ordered
         }

         return;
      }
   }

   (*other_gFunctionTable.pfnClientCommand) (pEntity);
}


void ClientUserInfoChanged (edict_t *pEntity, char *infobuffer)
{
   (*other_gFunctionTable.pfnClientUserInfoChanged) (pEntity, infobuffer);
}


void ServerActivate (edict_t *pEdictList, int edictCount, int clientMax)
{
   (*other_gFunctionTable.pfnServerActivate) (pEdictList, edictCount, clientMax);
}


void ServerDeactivate (void)
{
   (*other_gFunctionTable.pfnServerDeactivate) ();
}


void PlayerPreThink (edict_t *pEntity)
{
   (*other_gFunctionTable.pfnPlayerPreThink) (pEntity);
}


void PlayerPostThink (edict_t *pEntity)
{
   (*other_gFunctionTable.pfnPlayerPostThink) (pEntity);
}


void StartFrame (void)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      static bool previous_internetmode = TRUE;
      static float previous_time = 9999.0;
      static float client_update_time = 0.0;
      static float check_servercmd_time = 0.0;
      int bot_index;
      clientdata_s cd;

      // if a new map has started...
      if (gpGlobals->time + 0.1 < previous_time)
      {
         welcome_sent = FALSE; // send welcome message again
         b_doors_saved = FALSE; // reset saved doors locations
         client_update_time = gpGlobals->time + 10.0; // start updating client data again
         check_servercmd_time = gpGlobals->time + 1.0; // start checking for server commands again
         bot_check_time = gpGlobals->time + 15.0; // wait 15 seconds before adding bots
      }

      // if round has ended...
      if (roundend)
      {
         roundend = FALSE; // reset 'end of round' signal

         for (bot_index = 0; bot_index < 32; bot_index++)
            if (bots[bot_index].is_active && (bots[bot_index].pEdict != NULL))
               BotReset (&bots[bot_index]); // reset active bots
      }

      // if time to check for server commands...
      if (check_servercmd_time <= gpGlobals->time)
      {
         ServerCommand ((char *) CVAR_GET_STRING ("racc")); // execute what's in the "racc" CVAR
         CVAR_SET_STRING ("racc", ""); // reset the variable field
         check_servercmd_time = gpGlobals->time + 1.0; // next check in one second
      }

      // if time to update the bots' infobuffers...
      if (client_update_time <= gpGlobals->time)
      {
         client_update_time = gpGlobals->time + 1.0; // next update in 1 second

         // cycle through all bots slots
         for (bot_index = 0; bot_index < 32; bot_index++)
         {
            // is this one active ?
            if (bots[bot_index].is_active && (bots[bot_index].pEdict != NULL))
            {
               memset (&cd, 0, sizeof (cd));
               UpdateClientData (bots[bot_index].pEdict, 1, &cd); // update his infobuffer
               bots[bot_index].bot_weapons = cd.weapons; // update his weapons list
            }
         }
      }

      // cycle through all bot slots
      for (bot_index = 0; bot_index < 32; bot_index++)
      {
         // is this slot used ?
         if (bots[bot_index].is_active && (bots[bot_index].pEdict != NULL))
         {
            BotThink (&bots[bot_index]); // make bot think

            // if bot is allowed to quit AND it's time to quit
            if ((bots[bot_index].quit_game_time > 0)
                && (bots[bot_index].quit_game_time <= gpGlobals->time))
            {
               char servercmd[80];
               sprintf (servercmd, "kick \"%s\"\n", STRING (bots[bot_index].pEdict->v.netname));
               SERVER_COMMAND (servercmd); // let the bot disconnect
               if (bot_check_time - 5.0 < gpGlobals->time)
                  bot_check_time = gpGlobals->time + RANDOM_FLOAT (10.0, 30.0); // and delay
            }
         }
      }

      // check if a bot needs to be created/removed...
      if (bot_check_time < gpGlobals->time)
      {
         // has the welcome message been sent ?
         if (welcome_sent)
         {
            // can we add a bot AND are there less bots than the maximum # of bots ?
            if ((playerCount < gpGlobals->maxClients - 1)
                && (CVAR_GET_FLOAT ("racc_autofill") > 0)
                && ((BotCount () < CVAR_GET_FLOAT ("racc_maxbots"))
                    || (CVAR_GET_FLOAT ("racc_maxbots") == -1)))
            {
               BotCreate (NULL, NULL, -1, -1, -1, -1); // add a bot
               if (CVAR_GET_FLOAT ("racc_internetmode") > 0)
                  bot_check_time = gpGlobals->time + RANDOM_FLOAT (10.0, 30.0); // delay a while
               else
                  bot_check_time = gpGlobals->time + 10.0; // delay ten seconds
            }

            // else if there are too many bots disconnect one from the server
            else if (((CVAR_GET_FLOAT ("racc_maxbots") != -1) && (BotCount () > CVAR_GET_FLOAT ("racc_maxbots")))
                     || ((playerCount == gpGlobals->maxClients) && (BotCount () > CVAR_GET_FLOAT ("racc_minbots"))))
            {
               // cycle through all bot slots
               for (bot_index = 0; bot_index < 32; bot_index++)
               {
                  // is this slot used ?
                  if (bots[bot_index].is_active && (bots[bot_index].pEdict != NULL))
                  {
                     char servercmd[80];
                     sprintf (servercmd, "kick \"%s\"\n", STRING (bots[bot_index].pEdict->v.netname));
                     SERVER_COMMAND (servercmd); // let the bot disconnect
                     break;
                  }
               }

               bot_check_time = gpGlobals->time + 0.5; // should delay a while
            }

            // else if internet mode has been switched update the bots' TTLs
            if (!previous_internetmode && (CVAR_GET_FLOAT ("racc_internetmode") > 0))
            {
               for (bot_index = 0; bot_index < 32; bot_index++)
                  if (bots[bot_index].is_active && (bots[bot_index].pEdict != NULL))
                  {
                     bots[bot_index].time_to_live = gpGlobals->time + RANDOM_LONG (300, 3600); // set them a TTL
                     bots[bot_index].quit_game_time = bots[bot_index].time_to_live + RANDOM_FLOAT (3.0, 7.0); // disconnect time
                  }
               previous_internetmode = TRUE; // remember new internet mode flag state
               bot_check_time = gpGlobals->time + 10.0; // we'll check again in 10 seconds
            }
            else if (previous_internetmode && (CVAR_GET_FLOAT ("racc_internetmode") == 0))
            {
               for (bot_index = 0; bot_index < 32; bot_index++)
                  if (bots[bot_index].is_active && (bots[bot_index].pEdict != NULL))
                  {
                     bots[bot_index].time_to_live = -1; // don't set them a TTL (time to live)
                     bots[bot_index].quit_game_time = -1; // so never quit
                  }
               previous_internetmode = FALSE; // remember new internet mode flag state
               bot_check_time = gpGlobals->time + 10.0; // we'll check again in 10 seconds
            }

            // else nothing to do
            else
               bot_check_time = gpGlobals->time + 10.0; // we'll check again in 10 seconds
         }

         // else welcome message has not been sent yet
         else
         {
            UTIL_SendWelcomeMessage (); // send the welcome message
            bot_check_time = gpGlobals->time + 3.0; // delay 3 seconds
         }
      }

      previous_time = gpGlobals->time; // previous time gets updated at each StartFrame

      // systematically wrap all bot angles to avoid engine freezes
      for (bot_index = 0; bot_index < 32; bot_index++)
         if (bots[bot_index].is_active && (bots[bot_index].pEdict != NULL))
         {
            bots[bot_index].pEdict->v.angles = UTIL_WrapAngles (bots[bot_index].pEdict->v.angles);
            bots[bot_index].pEdict->v.v_angle = UTIL_WrapAngles (bots[bot_index].pEdict->v.v_angle);
         }
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


void PlayerCustomization (edict_t *pEntity, customization_t *pCust)
{
   (*other_gFunctionTable.pfnPlayerCustomization) (pEntity, pCust);
}


void SpectatorConnect (edict_t *pEntity)
{
   (*other_gFunctionTable.pfnSpectatorConnect) (pEntity);
}


void SpectatorDisconnect (edict_t *pEntity)
{
   (*other_gFunctionTable.pfnSpectatorDisconnect) (pEntity);
}


void SpectatorThink (edict_t *pEntity)
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


void SetupVisibility (edict_t *pViewEntity, edict_t *pClient, unsigned char **pvs, unsigned char **pas)
{
   (*other_gFunctionTable.pfnSetupVisibility) (pViewEntity, pClient, pvs, pas);
}


void UpdateClientData (const struct edict_s *ent, int sendweapons, struct clientdata_s *cd)
{
   (*other_gFunctionTable.pfnUpdateClientData) (ent, sendweapons, cd);
}


int AddToFullPack (struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet)
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


void CmdStart (const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed)
{
   (*other_gFunctionTable.pfnCmdStart) (player, cmd, random_seed);
}


void CmdEnd (const edict_t *player)
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


int InconsistentFile (const edict_t *player, const char *filename, char *disconnect_message)
{
   return (*other_gFunctionTable.pfnInconsistentFile) (player, filename, disconnect_message);
}


int AllowLagCompensation (void)
{
   return (*other_gFunctionTable.pfnAllowLagCompensation) ();
}


int GetTeam (edict_t *pEntity)
{
   return pEntity->v.team - 1; // TFC teams are 1-4 based
}


void ServerCommand (char *command)
{
   // handle dedicated server commands (passed through the racc CVAR string)
   char pcmd[128];
   
   if ((command == NULL) || (*command == 0))
      return; // if no command, return

   strcpy (pcmd, GetArg (command, 0)); // get the command itself from the string

   if ((FStrEq (pcmd, "help")) || (FStrEq (pcmd, "?")))
   {
      UTIL_ServerConsole_printf (RACC_WELCOMETEXT);
      UTIL_ServerConsole_printf ("\n  -- Available server commands:\n");
      UTIL_ServerConsole_printf ("racc add [name logo nationality skill team class] - Add a bot to the current game\n");
      UTIL_ServerConsole_printf ("racc kick - Disconnect a bot from the current game\n");
      UTIL_ServerConsole_printf ("racc kickall - Disconnect all bots from the current game\n");
      UTIL_ServerConsole_printf ("racc viewprofiles - Display profiles of bots currently in the game\n");
      UTIL_ServerConsole_printf ("racc botcount - Display the number of bots currently in the game\n");
      UTIL_ServerConsole_printf ("racc playercount - Display the total number of players currently in the game\n");
      UTIL_ServerConsole_printf ("racc time - Display the current map play time\n");
   }
   else if (FStrEq (pcmd, "add"))
   {
      if (welcome_sent)
      {
         BotCreate (GetArg (command, 1), GetArg (command, 2), atoi (GetArg (command, 3)), atoi (GetArg (command, 4)), atoi (GetArg (command, 5)), atoi (GetArg (command, 6)));
         bot_check_time = gpGlobals->time + 10.0; // delay a while
      }
      else
         UTIL_ServerConsole_printf ("RACC: Can't create bot yet\n");
   }
   else if (FStrEq (pcmd, "kick"))
   {
      for (int i = 0; i < 32; i++)
         if (bots[i].is_active && (bots[i].pEdict != NULL))
         {
            char servercmd[80];
            sprintf (servercmd, "kick \"%s\"\n", STRING (bots[i].pEdict->v.netname));
            SERVER_COMMAND (servercmd); // let the bot disconnect
            bot_check_time = gpGlobals->time + 0.5;
            break;
         }
   }
   else if (FStrEq (pcmd, "kickall"))
   {
      for (int i = 0; i < 32; i++)
         if (bots[i].is_active && (bots[i].pEdict != NULL))
         {
            char servercmd[80];
            sprintf (servercmd, "kick \"%s\"\n", STRING (bots[i].pEdict->v.netname));
            SERVER_COMMAND (servercmd); // let the bot disconnect
            bot_check_time = gpGlobals->time + 0.5;
         }
   }
   else if (FStrEq (pcmd, "viewprofiles"))
   {
      char nationality[32];
      UTIL_ServerConsole_printf ("RACC: Current bot profiles\n");
      for (int i = 0; i < 32; i++)
         if (bots[i].is_active && (bots[i].pEdict != NULL)) // is this slot used?
         {
            if (bots[i].bot_nationality == NATIONALITY_FRENCH)
               sprintf (nationality, "french");
            else if (bots[i].bot_nationality == NATIONALITY_GERMAN)
               sprintf (nationality, "german");
            else if (bots[i].bot_nationality == NATIONALITY_ITALIAN)
               sprintf (nationality, "italian");
            else if (bots[i].bot_nationality == NATIONALITY_SPANISH)
               sprintf (nationality, "spanish");
            else
               sprintf (nationality, "english");
            UTIL_ServerConsole_printf ("Name: %s - Model: %s - Logo: %s - Nation: %s - Skill: %d\n",
                                       STRING (bots[i].pEdict->v.netname),
                                       g_engfuncs.pfnInfoKeyValue ((*g_engfuncs.pfnGetInfoKeyBuffer) (bots[i].pEdict), "model"),
                                       g_engfuncs.pfnInfoKeyValue ((*g_engfuncs.pfnGetInfoKeyBuffer) (bots[i].pEdict), "logo"),
                                       nationality,
                                       bots[i].bot_skill);
         }
   }
   else if (FStrEq (pcmd, "botcount"))
      UTIL_ServerConsole_printf ("There are %d bots in game\n", BotCount ());
   else if (FStrEq (pcmd, "playercount"))
      UTIL_ServerConsole_printf ("There are %d players in game\n", playerCount);
   else if (FStrEq (pcmd, "time"))
      UTIL_ServerConsole_printf ("Current map play time is %f seconds\n", gpGlobals->time);
}


DLL_FUNCTIONS gFunctionTable =
{
   GameDLLInit,               //pfnGameInit
   DispatchSpawn,             //pfnSpawn
   DispatchThink,             //pfnThink
   DispatchUse,               //pfnUse
   DispatchTouch,             //pfnTouch
   DispatchBlocked,           //pfnBlocked
   DispatchKeyValue,          //pfnKeyValue
   DispatchSave,              //pfnSave
   DispatchRestore,           //pfnRestore
   DispatchObjectCollsionBox, //pfnAbsBox
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
   // check if engine's pointer is valid and version is correct...
   if (!pFunctionTable || interfaceVersion != INTERFACE_VERSION)
      return FALSE;

   // pass engine callback function table to engine...
   memcpy (pFunctionTable, &gFunctionTable, sizeof (DLL_FUNCTIONS));

   // pass other DLLs engine callbacks to function table...
   if (!(*other_GetEntityAPI) (&other_gFunctionTable, INTERFACE_VERSION))
      return FALSE;  // error initializing function table!!!

   return TRUE;
}


extern "C" EXPORT int GetNewDLLFunctions (NEW_DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion)
{
   if (other_GetNewDLLFunctions == NULL)
      return FALSE;

   // pass other DLLs engine callbacks to function table...
   if (!(*other_GetNewDLLFunctions) (pFunctionTable, interfaceVersion))
      return FALSE;  // error initializing function table!!!

   return TRUE;
}
