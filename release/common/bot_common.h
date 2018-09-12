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
// bot_common.h
//

#ifndef BOT_COMMON_H
#define BOT_COMMON_H


// bot version
#define RACC_VERSION "20020911"
#define RACC_WELCOMETEXT "RACC version " RACC_VERSION " - http://www.racc-ai.com"


// stuff for Win32 builds...
typedef int (FAR *GETENTITYAPI) (DLL_FUNCTIONS *, int);
typedef int (FAR *GETNEWDLLFUNCTIONS) (NEW_DLL_FUNCTIONS *, int *);
typedef void (DLLEXPORT *GIVEFNPTRSTODLL) (enginefuncs_t *, globalvars_t *);
typedef void (FAR *LINK_ENTITY_FUNC) (entvars_t *);


// bot nationalities
#define NATIONALITY_ENGLISH 0
#define NATIONALITY_FRENCH 1
#define NATIONALITY_GERMAN 2
#define NATIONALITY_ITALIAN 3
#define NATIONALITY_SPANISH 4


#define LADDER_UNKNOWN 0
#define LADDER_UP 1
#define LADDER_DOWN 2

#define BOT_PITCH_SPEED 20
#define BOT_YAW_SPEED 20


// common bot orders
#define BOT_ORDER_NOORDER 0
#define BOT_ORDER_REPORT 1
#define BOT_ORDER_FOLLOW 2
#define BOT_ORDER_STAY 3
#define BOT_ORDER_GO 4


// file-related structure definitions
typedef struct
{                       
   WORD e_magic; // magic number
   WORD e_cblp; // bytes on last page of file
   WORD e_cp; // pages in file
   WORD e_crlc; // relocations
   WORD e_cparhdr; // size of header in paragraphs
   WORD e_minalloc; // minimum extra paragraphs needed
   WORD e_maxalloc; // maximum extra paragraphs needed
   WORD e_ss; // initial (relative) SS value
   WORD e_sp; // initial SP value
   WORD e_csum; // checksum
   WORD e_ip; // initial IP value
   WORD e_cs; // initial (relative) CS value
   WORD e_lfarlc; // file address of relocation table
   WORD e_ovno; // overlay number
   WORD e_res[4]; // reserved words
   WORD e_oemid; // OEM identifier (for e_oeminfo)
   WORD e_oeminfo; // OEM information; e_oemid specific
   WORD e_res2[10]; // reserved words
   LONG e_lfanew; // file address of new exe header
} DOS_HEADER, *P_DOS_HEADER; // DOS .EXE header


typedef struct
{
   WORD Machine;
   WORD NumberOfSections;
   DWORD TimeDateStamp;
   DWORD PointerToSymbolTable;
   DWORD NumberOfSymbols;
   WORD SizeOfOptionalHeader;
   WORD Characteristics;
} PE_HEADER, *P_PE_HEADER;


typedef struct
{
   BYTE Name[8];
   union
   {
      DWORD PhysicalAddress;
      DWORD VirtualSize;
   } Misc;
   DWORD VirtualAddress;
   DWORD SizeOfRawData;
   DWORD PointerToRawData;
   DWORD PointerToRelocations;
   DWORD PointerToLinenumbers;
   WORD NumberOfRelocations;
   WORD NumberOfLinenumbers;
   DWORD Characteristics;
} SECTION_HEADER, *P_SECTION_HEADER;


typedef struct
{
   DWORD VirtualAddress;
   DWORD Size;
} DATA_DIRECTORY, *P_DATA_DIRECTORY;


typedef struct
{
   WORD Magic;
   BYTE MajorLinkerVersion;
   BYTE MinorLinkerVersion;
   DWORD SizeOfCode;
   DWORD SizeOfInitializedData;
   DWORD SizeOfUninitializedData;
   DWORD AddressOfEntryPoint;
   DWORD BaseOfCode;
   DWORD BaseOfData;
   DWORD ImageBase;
   DWORD SectionAlignment;
   DWORD FileAlignment;
   WORD MajorOperatingSystemVersion;
   WORD MinorOperatingSystemVersion;
   WORD MajorImageVersion;
   WORD MinorImageVersion;
   WORD MajorSubsystemVersion;
   WORD MinorSubsystemVersion;
   DWORD Win32VersionValue;
   DWORD SizeOfImage;
   DWORD SizeOfHeaders;
   DWORD CheckSum;
   WORD Subsystem;
   WORD DllCharacteristics;
   DWORD SizeOfStackReserve;
   DWORD SizeOfStackCommit;
   DWORD SizeOfHeapReserve;
   DWORD SizeOfHeapCommit;
   DWORD LoaderFlags;
   DWORD NumberOfRvaAndSizes;
   DATA_DIRECTORY DataDirectory[16];
} OPTIONAL_HEADER, *P_OPTIONAL_HEADER;


typedef struct
{
   DWORD Characteristics;
   DWORD TimeDateStamp;
   WORD MajorVersion;
   WORD MinorVersion;
   DWORD Name;
   DWORD Base;
   DWORD NumberOfFunctions;
   DWORD NumberOfNames;
   DWORD AddressOfFunctions; // RVA from base of image
   DWORD AddressOfNames; // RVA from base of image
   DWORD AddressOfNameOrdinals; // RVA from base of image
} EXPORT_DIRECTORY, *P_EXPORT_DIRECTORY;


// bot-related structure definitions
typedef struct
{
   bool mdl_match;
   int team_no;
   edict_t *edict;
} flag_t;


typedef struct
{
   char szClassname[64];
   int iAmmo1; // ammo index for primary ammo
   int iAmmo1Max; // max primary ammo
   int iAmmo2; // ammo index for secondary ammo
   int iAmmo2Max; // max secondary ammo
   int iSlot; // HUD slot (0 based)
   int iPosition; // slot position
   int iId; // weapon ID
   int iFlags; // flags ???
} bot_weapon_t;


typedef struct
{
   int iId; // weapon ID
   int iClip; // amount of ammo in the clip
   int iAmmo1; // amount of ammo in primary reserve
   int iAmmo2; // amount of ammo in secondary reserve
} bot_current_weapon_t;


typedef struct
{
   float f_forward_time;
   float f_backwards_time;
   float f_jump_time;
   float f_duck_time;
   float f_strafeleft_time;
   float f_straferight_time;
   bool b_is_walking;
   bool b_emergency_walkback;
   float f_max_speed;
   float f_move_speed;
   float f_strafe_speed;
} bot_move_t;


typedef struct
{
   bool b_saytext_hello;
   bool b_saytext_kill;
   bool b_saytext_killed;
   bool b_saytext_idle;
   bool b_saytext_follow;
   bool b_saytext_stop;
   bool b_saytext_stay;
   bool b_saytext_help;
   bool b_saytext_cant;
   bool b_saytext_affirmative;
   bool b_saytext_negative;
   bool b_saytext_bye;
   bool b_sayaudio_affirmative;
   bool b_sayaudio_alert;
   bool b_sayaudio_attacking;
   bool b_sayaudio_firstspawn;
   bool b_sayaudio_inposition;
   bool b_sayaudio_negative;
   bool b_sayaudio_report;
   bool b_sayaudio_reporting;
   bool b_sayaudio_seegrenade;
   bool b_sayaudio_takingdamage;
   bool b_sayaudio_throwgrenade;
   bool b_sayaudio_victory;
} bot_chat_t;


typedef struct
{
   bool is_active;
   edict_t *pEdict;
   int bot_skill;
   int menu_notify;
   bool b_not_started;
   char buy_state;
   float time_to_live;
   float quit_game_time;

   int msecnum;
   float msecdel;
   float msecval;

   bot_move_t BotMove;

   int bot_nationality;
   int bot_team;
   int bot_class;
   int bot_health;
   int bot_armor;
   int bot_weapons; // bit map of weapons the bot is carrying
   int bot_money; // for Counter-Strike buy actions
   float idle_angle;
   float idle_angle_time;
   float blinded_time;
   float fall_time;

   float f_find_item_time;
   bool b_is_picking_item;

   int ladder_dir;
   float f_start_use_ladder_time;
   float f_end_use_ladder_time;

   bool b_is_fearful;
   Vector v_prev_position;
   bool b_is_stuck;
   float f_check_stuck_time;

   float f_exit_water_time;

   edict_t *pBotEnemy;
   Vector v_lastseenenemy_position;
   float f_see_enemy_time;
   float f_lost_enemy_time;
   bool b_enemy_hidden;
   edict_t *pVictimEntity;
   edict_t *pKillerEntity;

   float f_aim_adjust_time;
   float f_reload_time;
   float f_throwgrenade_time;

   int bot_order;
   edict_t *pAskingEntity;
   float f_order_time;
   edict_t *pBotUser;
   Vector v_lastseenuser_position;
   float f_bot_use_time;
   float f_randomturn_time;
   Vector v_place_to_keep;
   float f_place_time;

   bot_chat_t BotChat;
   float f_bot_saytext_time;
   float f_bot_sayaudio_time;
   float f_bot_alone_timer;
   bool b_help_asked;

   float f_shoot_time;
   float f_primary_charging;
   float f_secondary_charging;
   int charging_weapon_id;
   int bot_grenades_1; // first grenades amount
   int bot_grenades_2; // second grenades amount
   float f_togglesniper_time;

   float f_rush_time;
   float f_pause_time;
   float f_buy_time;
   float f_sound_update_time;

   float f_fallcheck_time;
   float f_turncorner_time;
   float f_avoid_time;
   bool b_is_walking_straight;
   float f_find_goal_time;
   Vector v_goal;
   bool b_has_valuable;
   float f_camp_time;
   Vector v_reach_point;
   float f_reach_time;
   float f_samplefov_time;

   bool b_interact;
   float f_interact_time;
   Vector v_interactive_entity;
   bool b_use_station;
   float f_use_station_time;
   Vector v_station_entity;

   bool b_lift_moving;
   float f_spraying_logo_time;
   bool b_logo_sprayed;
   bool b_can_plant;
   bool b_is_planting;
   bool b_is_defusing;
   bool b_has_defuse_kit;

   bot_current_weapon_t current_weapon; // one current weapon for each bot
   int m_rgAmmo[MAX_AMMO_SLOTS]; // total ammo amounts (1 array for each bot)

} bot_t;


// dll.cpp function prototypes
void DLLEXPORT GiveFnptrsToDll (enginefuncs_t* pengfuncsFromEngine, globalvars_t *pGlobals);
void GameDLLInit (void);
int DispatchSpawn (edict_t *pent);
void DispatchThink (edict_t *pent);
void DispatchUse (edict_t *pentUsed, edict_t *pentOther);
void DispatchTouch (edict_t *pentTouched, edict_t *pentOther);
void DispatchBlocked (edict_t *pentBlocked, edict_t *pentOther);
void DispatchKeyValue (edict_t *pentKeyvalue, KeyValueData *pkvd);
void DispatchSave (edict_t *pent, SAVERESTOREDATA *pSaveData);
int DispatchRestore (edict_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity);
void DispatchObjectCollsionBox (edict_t *pent);
void SaveWriteFields (SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount);
void SaveReadFields (SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount);
void SaveGlobalState (SAVERESTOREDATA *pSaveData);
void RestoreGlobalState (SAVERESTOREDATA *pSaveData);
void ResetGlobalState (void);
BOOL ClientConnect (edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] );
void ClientDisconnect (edict_t *pEntity);
void ClientKill (edict_t *pEntity);
void ClientPutInServer (edict_t *pEntity);
void ClientCommand (edict_t *pEntity);
void ClientUserInfoChanged (edict_t *pEntity, char *infobuffer);
void ServerActivate (edict_t *pEdictList, int edictCount, int clientMax);
void ServerDeactivate (void);
void PlayerPreThink (edict_t *pEntity);
void PlayerPostThink (edict_t *pEntity);
void StartFrame (void);
void ParmsNewLevel (void);
void ParmsChangeLevel (void);
const char *GetGameDescription (void);
void PlayerCustomization (edict_t *pEntity, customization_t *pCust);
void SpectatorConnect (edict_t *pEntity);
void SpectatorDisconnect (edict_t *pEntity);
void SpectatorThink (edict_t *pEntity);
void Sys_Error (const char *error_string);
void PM_Move (struct playermove_s *ppmove, int server);
void PM_Init (struct playermove_s *ppmove);
char PM_FindTextureType (char *name);
void SetupVisibility (edict_t *pViewEntity, edict_t *pClient, unsigned char **pvs, unsigned char **pas);
void UpdateClientData (const struct edict_s *ent, int sendweapons, struct clientdata_s *cd);
int AddToFullPack (struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet);
void CreateBaseline (int player, int eindex, struct entity_state_s *baseline, struct edict_s *entity, int playermodelindex, vec3_t player_mins, vec3_t player_maxs);
void RegisterEncoders (void);
int GetWeaponData (struct edict_s *player, struct weapon_data_s *info);
void CmdStart (const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed);
void CmdEnd (const edict_t *player);
int ConnectionlessPacket (const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size);
int GetHullBounds (int hullnumber, float *mins, float *maxs);
void CreateInstancedBaselines (void);
int InconsistentFile (const edict_t *player, const char *filename, char *disconnect_message);
int AllowLagCompensation (void);
int GetTeam (edict_t *pEntity);
void ServerCommand (char *command);

// bot_navigation.cpp function prototypes
void BotMove (bot_t *pBot);
float BotChangePitch (bot_t *pBot, float speed);
float BotChangeYaw (bot_t *pBot, float speed);
void BotOnLadder (bot_t *pBot);
void BotFollowOnLadder (bot_t *pBot);
void BotUnderWater (bot_t *pBot);
void BotUseLift (bot_t *pBot);
bool BotCanUseInteractives (bot_t *pBot);
void BotInteractWithWorld (bot_t *pBot);
bool BotCantMoveForward (bot_t *pBot, TraceResult *tr);
void BotTurnAtWall (bot_t *pBot, TraceResult *tr);
bool BotCanFallOnTheLeft (bot_t *pBot);
bool BotCanFallOnTheRight (bot_t *pBot);
bool BotCanFallForward (bot_t *pBot, TraceResult *tr);
void BotTurnAtFall (bot_t *pBot, TraceResult *tr);
bool BotCantSeeForward (bot_t *pBot);
bool BotCanJumpUp (bot_t *pBot);
bool BotCanDuckUnder (bot_t *pBot);
void BotRandomTurn (bot_t *pBot);
void BotFollowUser (bot_t *pBot);
void BotFindLadder (bot_t *pBot);
void BotStayInPosition (bot_t *pBot);
bool BotCheckForWall (bot_t *pBot, Vector v_direction);
void BotCheckForCorners (bot_t *pBot);
void BotWander (bot_t *pBot);
void BotReachPosition (bot_t *pBot, Vector v_position);
void BotSampleFOV (bot_t *pBot);
bool BotIsStuck (bot_t *pBot);
void BotUnstuck (bot_t *pBot);
void BotAvoidObstacles (bot_t *pBot);
bool BotCanSeeThis (bot_t *pBot, Vector v_destination);
bool BotCanSeeThisBModel (bot_t *pBot, edict_t *pBModel);
Vector BotGetIdealAimVector (bot_t *pBot, edict_t *pPlayer);
void BotPointGun (bot_t *pBot, Vector v_target_angles);
bool BotCanCampNearHere (bot_t *pBot, Vector v_here);

// bot_chat.cpp function prototypes
void BotSayText (bot_t *pBot);
void BotSayAudio (bot_t *pBot);
void BotTalk (bot_t *pBot, char *sound_path);
const char *Name (const char *string);
const char *HumanizeChat (const char *string);
const char *StripBlanks (const char *string);
const char *StripTags (const char *string);

// util.cpp function prototypes
Vector UTIL_VecToAngles (const Vector &vec);
void UTIL_TraceLine (const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t *pentIgnore, TraceResult *ptr);
void UTIL_TraceLine (const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr);
void UTIL_TraceHull (const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t *pentIgnore, TraceResult *ptr);
void UTIL_MakeVectors (const Vector &vecAngles);
edict_t *UTIL_FindEntityInSphere (edict_t *pentStart, const Vector &vecCenter, float flRadius);
edict_t *UTIL_FindEntityByString (edict_t *pentStart, const char *szKeyword, const char *szValue);
edict_t *UTIL_FindEntityByClassname (edict_t *pentStart, const char *szName);
edict_t *UTIL_FindEntityByTargetname (edict_t *pentStart, const char *szName);
int UTIL_PointContents (const Vector &vec);
void UTIL_SetSize (entvars_t *pev, const Vector &vecMin, const Vector &vecMax);
void UTIL_SetOrigin (entvars_t *pev, const Vector &vecOrigin);
void ClientPrint (edict_t *pEntity, int msg_dest, const char *msg_name);
void UTIL_SayText (const char *pText, edict_t *pEdict);
void UTIL_HostSay (edict_t *pSenderEntity, int teamonly, char *message);
int BotCount (void);
int UTIL_GetBotIndex (const edict_t *pEdict);
bool IsAlive (edict_t *pEdict);
bool FInViewCone (Vector pOrigin, edict_t *pEdict);
bool FVisible (const Vector &vecOrigin, edict_t *pEdict);
bool IsReachable (Vector v_dest, edict_t *pEdict);
bool IsAtHumanHeight (Vector v_location);
Vector DropAtHumanHeight (Vector v_location);
Vector GetGunPosition (edict_t *pEdict);
Vector VecBModelOrigin (edict_t *pEdict);
bool BotCheckForSounds (bot_t *pBot, edict_t *pPlayer);
void UTIL_ShowMenu (edict_t *pEdict, int slots, int displaytime, bool needmore, char *pText);
void UTIL_DrawBeam (edict_t *pEntity, Vector start, Vector end, int life, int width, int noise, int red, int green, int blue, int brightness, int speed);
void UTIL_printf (char *fmt, ...);
void UTIL_ServerConsole_printf (char *fmt, ...);
void BotSetIdealYaw (bot_t *pBot, float value);
void BotAddIdealYaw (bot_t *pBot, float value);
void BotSetIdealPitch (bot_t *pBot, float value);
void BotAddIdealPitch (bot_t *pBot, float value);
void BotSetViewAngles (bot_t *pBot, Vector v_angles);
void BotAddViewAngles (bot_t *pBot, Vector v_angles);
float UTIL_WrapAngle (float angle_to_wrap);
float UTIL_EngineWrapAngle (float angle_to_wrap);
Vector UTIL_WrapAngles (Vector angles_to_wrap);
Vector UTIL_EngineWrapAngles (Vector angles_to_wrap);
float UTIL_AngleOfVectors (Vector v1, Vector v2);
float BotAngleToLocation (bot_t *pBot, Vector dest);
int UTIL_GetEntityIllum (edict_t *pEdict);
int UTIL_GetNearestOrderableBotIndex (edict_t *pAskingEntity);
int UTIL_GetNearestUsableBotIndex (edict_t *pAskingEntity);
int UTIL_GetNearestUsedBotIndex (edict_t *pAskingEntity);
void GetGameLocale (void);
void InitCVARs (void);
void LoadBotProfiles (void);
void LoadBotTextChat (void);
void PrecacheStuff (void);
void SaveDoorsOrigins (void);
void LoadSymbols (char *filename);
void GetFunctionName (char *name, FILE *fp);
void StripNameForEngine (char *out_name, char *in_name);
unsigned long FUNCTION_FROM_NAME (const char *pName);
const char *NAME_FOR_FUNCTION (unsigned long function);
void FakeClientCommand (edict_t *pFakeClient, char *command);
int Cmd_Argc (void);
const char *Cmd_Args (void);
const char *Cmd_Argv (int argc);
const char *GetArg (const char *command, int arg_number);
void UTIL_DisplaySpeakerIcon (bot_t *pBot, edict_t *pViewerClient, int duration);
void UTIL_DestroySpeakerIcon (bot_t *pBot, edict_t *pViewerClient);
void UTIL_SendWelcomeMessage (void);

// engine function prototypes...
int pfnPrecacheModel (char* s);
int pfnPrecacheSound (char* s);
void pfnSetModel (edict_t *e, const char *m);
int pfnModelIndex (const char *m);
int pfnModelFrames (int modelIndex);
void pfnSetSize (edict_t *e, const float *rgflMin, const float *rgflMax);
void pfnChangeLevel (char* s1, char* s2);
void pfnGetSpawnParms (edict_t *ent);
void pfnSaveSpawnParms (edict_t *ent);
float pfnVecToYaw (const float *rgflVector);
void pfnVecToAngles (const float *rgflVectorIn, float *rgflVectorOut);
void pfnMoveToOrigin (edict_t *ent, const float *pflGoal, float dist, int iMoveType);
void pfnChangeYaw (edict_t* ent);
void pfnChangePitch (edict_t* ent);
edict_t* pfnFindEntityByString (edict_t *pEdictStartSearchAfter, const char *pszField, const char *pszValue);
int pfnGetEntityIllum (edict_t* pEnt);
edict_t* pfnFindEntityInSphere (edict_t *pEdictStartSearchAfter, const float *org, float rad);
edict_t* pfnFindClientInPVS (edict_t *pEdict);
edict_t* pfnEntitiesInPVS (edict_t *pplayer);
void pfnMakeVectors (const float *rgflVector);
void pfnAngleVectors (const float *rgflVector, float *forward, float *right, float *up);
edict_t* pfnCreateEntity (void);
void pfnRemoveEntity (edict_t* e);
edict_t* pfnCreateNamedEntity (int className);
void pfnMakeStatic (edict_t *ent);
int pfnEntIsOnFloor (edict_t *e);
int pfnDropToFloor (edict_t* e);
int pfnWalkMove (edict_t *ent, float yaw, float dist, int iMode);
void pfnSetOrigin (edict_t *e, const float *rgflOrigin);
void pfnEmitSound (edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch);
void pfnEmitAmbientSound (edict_t *entity, float *pos, const char *samp, float vol, float attenuation, int fFlags, int pitch);
void pfnTraceLine (const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr);
void pfnTraceToss (edict_t* pent, edict_t* pentToIgnore, TraceResult *ptr);
int pfnTraceMonsterHull (edict_t *pEdict, const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr);
void pfnTraceHull (const float *v1, const float *v2, int fNoMonsters, int hullNumber, edict_t *pentToSkip, TraceResult *ptr);
void pfnTraceModel (const float *v1, const float *v2, int hullNumber, edict_t *pent, TraceResult *ptr);
const char *pfnTraceTexture (edict_t *pTextureEntity, const float *v1, const float *v2);
void pfnTraceSphere (const float *v1, const float *v2, int fNoMonsters, float radius, edict_t *pentToSkip, TraceResult *ptr);
void pfnGetAimVector (edict_t* ent, float speed, float *rgflReturn);
void pfnServerCommand (char* str);
void pfnServerExecute (void);
void pfnClientCommand (edict_t* pEdict, char* szFmt, ...);
void pfnParticleEffect (const float *org, const float *dir, float color, float count);
void pfnLightStyle (int style, char* val);
int pfnDecalIndex (const char *name);
int pfnPointContents (const float *rgflVector);
void pfnMessageBegin (int msg_dest, int msg_type, const float *pOrigin, edict_t *ed);
void pfnMessageEnd (void);
void pfnWriteByte (int iValue);
void pfnWriteChar (int iValue);
void pfnWriteShort (int iValue);
void pfnWriteLong (int iValue);
void pfnWriteAngle (float flValue);
void pfnWriteCoord (float flValue);
void pfnWriteString (const char *sz);
void pfnWriteEntity (int iValue);
void pfnCVarRegister (cvar_t *pCvar);
float pfnCVarGetFloat (const char *szVarName);
const char* pfnCVarGetString (const char *szVarName);
void pfnCVarSetFloat (const char *szVarName, float flValue);
void pfnCVarSetString (const char *szVarName, const char *szValue);
void pfnAlertMessage (ALERT_TYPE atype, char *szFmt, ...);
void pfnEngineFprintf (FILE *pfile, char *szFmt, ...);
void* pfnPvAllocEntPrivateData (edict_t *pEdict, long cb);
void* pfnPvEntPrivateData (edict_t *pEdict);
void pfnFreeEntPrivateData (edict_t *pEdict);
const char* pfnSzFromIndex (int iString);
int pfnAllocString (const char *szValue);
struct entvars_s* pfnGetVarsOfEnt (edict_t *pEdict);
edict_t* pfnPEntityOfEntOffset (int iEntOffset);
int pfnEntOffsetOfPEntity (const edict_t *pEdict);
int pfnIndexOfEdict (const edict_t *pEdict);
edict_t* pfnPEntityOfEntIndex (int iEntIndex);
edict_t* pfnFindEntityByVars (struct entvars_s* pvars);
void* pfnGetModelPtr (edict_t* pEdict);
int pfnRegUserMsg (const char *pszName, int iSize);
void pfnAnimationAutomove (const edict_t* pEdict, float flTime);
void pfnGetBonePosition (const edict_t* pEdict, int iBone, float *rgflOrigin, float *rgflAngles);
unsigned long pfnFunctionFromName (const char *pName);
const char *pfnNameForFunction (unsigned long function);
void pfnClientPrintf (edict_t* pEdict, PRINT_TYPE ptype, const char *szMsg);
void pfnServerPrint (const char *szMsg);
const char *pfnCmd_Args (void);
const char *pfnCmd_Argv (int argc);
int pfnCmd_Argc (void);
void pfnGetAttachment (const edict_t *pEdict, int iAttachment, float *rgflOrigin, float *rgflAngles);
void pfnCRC32_Init (CRC32_t *pulCRC);
void pfnCRC32_ProcessBuffer (CRC32_t *pulCRC, void *p, int len);
void pfnCRC32_ProcessByte (CRC32_t *pulCRC, unsigned char ch);
CRC32_t pfnCRC32_Final (CRC32_t pulCRC);
long pfnRandomLong (long  lLow,  long  lHigh);
float pfnRandomFloat (float flLow, float flHigh);
void pfnSetView (const edict_t *pClient, const edict_t *pViewent);
float pfnTime (void);
void pfnCrosshairAngle (const edict_t *pClient, float pitch, float yaw);
byte *pfnLoadFileForMe (char *filename, int *pLength);
void pfnFreeFile (void *buffer);
void pfnEndSection (const char *pszSectionName);
int pfnCompareFileTime (char *filename1, char *filename2, int *iCompare);
void pfnGetGameDir (char *szGetGameDir);
void pfnCvar_RegisterVariable (cvar_t *variable);
void pfnFadeClientVolume (const edict_t *pEdict, int fadePercent, int fadeOutSeconds, int holdTime, int fadeInSeconds);
void pfnSetClientMaxspeed (const edict_t *pEdict, float fNewMaxspeed);
edict_t *pfnCreateFakeClient (const char *netname);
void pfnRunPlayerMove (edict_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, byte impulse, byte msec);
int pfnNumberOfEntities (void);
char* pfnGetInfoKeyBuffer (edict_t *e);
char* pfnInfoKeyValue (char *infobuffer, char *key);
void pfnSetKeyValue (char *infobuffer, char *key, char *value);
void pfnSetClientKeyValue (int clientIndex, char *infobuffer, char *key, char *value);
int pfnIsMapValid (char *filename);
void pfnStaticDecal (const float *origin, int decalIndex, int entityIndex, int modelIndex);
int pfnPrecacheGeneric (char* s);
int pfnGetPlayerUserId (edict_t *e);
void pfnBuildSoundMsg (edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch, int msg_dest, int msg_type, const float *pOrigin, edict_t *ed);
int pfnIsDedicatedServer (void);
cvar_t *pfnCVarGetPointer (const char *szVarName);
unsigned int pfnGetPlayerWONId (edict_t *e);
void pfnInfo_RemoveKey (char *s, const char *key);
const char *pfnGetPhysicsKeyValue (const edict_t *pClient, const char *key);
void pfnSetPhysicsKeyValue (const edict_t *pClient, const char *key, const char *value);
const char *pfnGetPhysicsInfoString (const edict_t *pClient);
unsigned short pfnPrecacheEvent (int type, const char*psz);
void pfnPlaybackEvent (int flags, const edict_t *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2);
unsigned char *pfnSetFatPVS (float *org);
unsigned char *pfnSetFatPAS (float *org);
int pfnCheckVisibility (const edict_t *entity, unsigned char *pset);
void pfnDeltaSetField (struct delta_s *pFields, const char *fieldname);
void pfnDeltaUnsetField (struct delta_s *pFields, const char *fieldname);
void pfnDeltaAddEncoder (char *name, void conditionalencode (struct delta_s *pFields, const unsigned char *from, const unsigned char *to));
int pfnGetCurrentPlayer (void);
int pfnCanSkipPlayer (const edict_t *player);
int pfnDeltaFindField (struct delta_s *pFields, const char *fieldname);
void pfnDeltaSetFieldByIndex (struct delta_s *pFields, int fieldNumber);
void pfnDeltaUnsetFieldByIndex (struct delta_s *pFields, int fieldNumber);
void pfnSetGroupMask (int mask, int op);
int pfnCreateInstancedBaseline (int classname, struct entity_state_s *baseline);
void pfnCvar_DirectSet (struct cvar_s *var, char *value);
void pfnForceUnmodified (FORCE_TYPE type, float *mins, float *maxs, const char *filename);
void pfnGetPlayerStats (const edict_t *pClient, int *ping, int *packet_loss);
void pfnAddServerCommand (char *cmd_name, void function (void));
qboolean pfnVoice_GetClientListening (int iReceiver, int iSender);
qboolean pfnVoice_SetClientListening (int iReceiver, int iSender, qboolean bListen);
const char *pfnGetPlayerAuthId (edict_t *e);


#endif // BOT_COMMON_H

