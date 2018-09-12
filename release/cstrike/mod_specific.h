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
// mod_specific.h
//

#ifndef MOD_SPECIFIC_H
#define MOD_SPECIFIC_H


// MOD name, game DLL paths and file names
#define GAME_DLL_PATH "cstrike/dlls/mp.dll"
#define METAMOD_DLL_PATH "cstrike/addons/metamod/dlls/metamod.dll"
#define RACC_LOGFILE_PATH "racc/release/messages.log"
#define RACC_WELCOMESOUND "racc/welcome.wav"


// maximum number of known local (radio) sounds in Counter-Strike
#define RACC_MAX_LOCAL_SOUNDS 45


// maximum walk speed (without making footstep noises) in Counter-Strike
#define MAX_WALK_SPEED 150


// maximum player safe fall speed in Counter-Strike
#define MAX_SAFEFALL_SPEED 580


// team ID values for Counter-Strike
#define CS_TERRORIST 0
#define CS_COUNTER_TERRORIST 1


// weapon ID values for Counter-Strike
#define CS_WEAPON_P228 1
#define CS_WEAPON_UNKNOWN2 2
#define CS_WEAPON_SCOUT 3
#define CS_WEAPON_HEGRENADE 4
#define CS_WEAPON_XM1014 5
#define CS_WEAPON_C4 6
#define CS_WEAPON_MAC10 7
#define CS_WEAPON_AUG 8
#define CS_WEAPON_SMOKEGRENADE 9
#define CS_WEAPON_ELITE 10
#define CS_WEAPON_FIVESEVEN 11
#define CS_WEAPON_UMP45 12
#define CS_WEAPON_SG550 13
#define CS_WEAPON_UNKNOWN14 14
#define CS_WEAPON_UNKNOWN15 15
#define CS_WEAPON_USP 16
#define CS_WEAPON_GLOCK18 17
#define CS_WEAPON_AWP 18
#define CS_WEAPON_MP5NAVY 19
#define CS_WEAPON_M249 20
#define CS_WEAPON_M3 21
#define CS_WEAPON_M4A1 22
#define CS_WEAPON_TMP 23
#define CS_WEAPON_G3SG1 24
#define CS_WEAPON_FLASHBANG 25
#define CS_WEAPON_DEAGLE 26
#define CS_WEAPON_SG552 27
#define CS_WEAPON_AK47 28
#define CS_WEAPON_KNIFE 29
#define CS_WEAPON_P90 30


// buy commands for Counter-Strike
#define BUY_USP "buy;menuselect 1;menuselect 1"
#define BUY_GLOCK18 "buy;menuselect 1;menuselect 2"
#define BUY_DEAGLE "buy;menuselect 1;menuselect 3"
#define BUY_P228 "buy;menuselect 1;menuselect 4"
#define BUY_ELITE "buy;menuselect 1;menuselect 5"
#define BUY_FIVESEVEN "buy;menuselect 1;menuselect 6"
#define BUY_M3 "buy;menuselect 2;menuselect 1"
#define BUY_XM1014 "buy;menuselect 2;menuselect 2"
#define BUY_MP5NAVY "buy;menuselect 3;menuselect 1"
#define BUY_TMP "buy;menuselect 3;menuselect 2"
#define BUY_P90 "buy;menuselect 3;menuselect 3"
#define BUY_MAC10 "buy;menuselect 3;menuselect 4"
#define BUY_UMP45 "buy;menuselect 3;menuselect 5"
#define BUY_AK47 "buy;menuselect 4;menuselect 1"
#define BUY_SG552 "buy;menuselect 4;menuselect 2"
#define BUY_M4A1 "buy;menuselect 4;menuselect 3"
#define BUY_AUG "buy;menuselect 4;menuselect 4"
#define BUY_SCOUT "buy;menuselect 4;menuselect 5"
#define BUY_AWP "buy;menuselect 4;menuselect 6"
#define BUY_G3SG1 "buy;menuselect 4;menuselect 7"
#define BUY_SG550 "buy;menuselect 4;menuselect 8"
#define BUY_M249 "buy;menuselect 5;menuselect 1"
#define BUY_PRIMARYAMMO "buyammo1"
#define BUY_SECONDARYAMMO "buyammo2"
#define BUY_KEVLAR "buyequip;menuselect 1"
#define BUY_KEVLARHELMET "buyequip;menuselect 2"
#define BUY_FLASHBANG "buyequip;menuselect 3"
#define BUY_HEGRENADE "buyequip;menuselect 4"
#define BUY_SMOKEGRENADE "buyequip;menuselect 5"
#define BUY_DEFUSEKIT "buyequip;menuselect 6"
#define BUY_NIGHTVISION "buyequip;menuselect 7"


// radio messages definitions for Counter-Strike
#define RADIOMSG_COVERME "radio1;menuselect 1"
#define RADIOMSG_YOUTAKETHEPOINT "radio1;menuselect 2"
#define RADIOMSG_KEEPTHEPOSITION "radio1;menuselect 3"
#define RADIOMSG_STICKTOGETHER "radio1;menuselect 4"
#define RADIOMSG_FOLLOWME "radio1;menuselect 5"
#define RADIOMSG_UNDERFIRE "radio1;menuselect 6"
#define RADIOMSG_GOGOGO "radio2;menuselect 1"
#define RADIOMSG_FALLBACK "radio2;menuselect 2"
#define RADIOMSG_TAKECOVER "radio2;menuselect 3"
#define RADIOMSG_STAYANDWAIT "radio2;menuselect 4"
#define RADIOMSG_STORMTHEFRONT "radio2;menuselect 5"
#define RADIOMSG_REPORTINGIN "radio2;menuselect 6"
#define RADIOMSG_AFFIRMATIVE "radio3;menuselect 1"
#define RADIOMSG_ENEMYSPOTTED "radio3;menuselect 2"
#define RADIOMSG_NEEDBACKUP "radio3;menuselect 3"
#define RADIOMSG_SECTORCLEAR "radio3;menuselect 4"
#define RADIOMSG_INPOSITION "radio3;menuselect 5"
#define RADIOMSG_REPORT "radio3;menuselect 6"
#define RADIOMSG_GONNABLOW "radio3;menuselect 7"
#define RADIOMSG_NEGATIVE "radio3;menuselect 8"
#define RADIOMSG_ENEMYDOWN "radio3;menuselect 9"


// game start messages for Counter-Strike
#define MENU_CS_IDLE 1
#define MENU_CS_TEAMSELECT_MAINMENU 2
#define MENU_CS_TEAMSELECT_COUNTERMENU 3
#define MENU_CS_TEAMSELECT_TERRMENU 4
#define MENU_CS_BUY_MAINMENU 5
#define MENU_CS_BUY_PISTOLMENU 6
#define MENU_CS_BUY_SHOTGUNMENU 7
#define MENU_CS_BUY_RIFLEMENU 8
#define MENU_CS_BUY_SUBMACHINEGUNMENU 9
#define MENU_CS_BUY_MACHINEGUNMENU 10
#define MENU_CS_BUY_EQUIPMENTMENU 11


// global variables declaration and referencing
GLOBAL char racc_version[9];
GLOBAL char racc_welcometext[256];
GLOBAL HINSTANCE h_Library;
GLOBAL DLL_FUNCTIONS other_gFunctionTable;
GLOBAL WORD *p_Ordinals;
GLOBAL DWORD *p_Functions;
GLOBAL DWORD *p_Names;
GLOBAL char *p_FunctionNames[1024];
GLOBAL int num_ordinals;
GLOBAL unsigned long base_offset;
GLOBAL enginefuncs_t g_engfuncs;
GLOBAL globalvars_t  *gpGlobals;
GLOBAL server_t server;
GLOBAL char g_argv[128];
GLOBAL bool isFakeClientCommand;
GLOBAL int fake_arg_count;
GLOBAL bsp_file_t bsp_file;
GLOBAL map_t map;
GLOBAL round_t round;
GLOBAL edict_t *pListenserverEntity;
GLOBAL edict_t *pWorldEntity;
GLOBAL char team_names[32][16];
GLOBAL int num_teams;
GLOBAL int team_allies[4];
GLOBAL float f_team_radiotime[2];
GLOBAL char bot_affirmative[5][100][256];
GLOBAL char bot_negative[5][100][256];
GLOBAL char bot_hello[5][100][256];
GLOBAL char bot_laugh[5][100][256];
GLOBAL char bot_whine[5][100][256];
GLOBAL char bot_idle[5][100][256];
GLOBAL char bot_follow[5][100][256];
GLOBAL char bot_stop[5][100][256];
GLOBAL char bot_stay[5][100][256];
GLOBAL char bot_help[5][100][256];
GLOBAL char bot_cant[5][100][256];
GLOBAL char bot_bye[5][100][256];
GLOBAL int recent_bot_affirmative[5][10];
GLOBAL int recent_bot_negative[5][10];
GLOBAL int recent_bot_hello[5][10];
GLOBAL int recent_bot_laugh[5][10];
GLOBAL int recent_bot_whine[5][10];
GLOBAL int recent_bot_idle[5][10];
GLOBAL int recent_bot_follow[5][10];
GLOBAL int recent_bot_stop[5][10];
GLOBAL int recent_bot_stay[5][10];
GLOBAL int recent_bot_help[5][10];
GLOBAL int recent_bot_cant[5][10];
GLOBAL int recent_bot_bye[5][10];
GLOBAL int affirmative_count[5];
GLOBAL int negative_count[5];
GLOBAL int hello_count[5];
GLOBAL int laugh_count[5];
GLOBAL int whine_count[5];
GLOBAL int idle_count[5];
GLOBAL int follow_count[5];
GLOBAL int stop_count[5];
GLOBAL int stay_count[5];
GLOBAL int help_count[5];
GLOBAL int cant_count[5];
GLOBAL int bye_count[5];
GLOBAL int audio_affirmative_count[5];
GLOBAL int audio_alert_count[5];
GLOBAL int audio_attacking_count[5];
GLOBAL int audio_firstspawn_count[5];
GLOBAL int audio_inposition_count[5];
GLOBAL int audio_negative_count[5];
GLOBAL int audio_report_count[5];
GLOBAL int audio_reporting_count[5];
GLOBAL int audio_seegrenade_count[5];
GLOBAL int audio_takingdamage_count[5];
GLOBAL int audio_throwgrenade_count[5];
GLOBAL int audio_victory_count[5];
GLOBAL profile_t profiles[RACC_MAX_PROFILES];
GLOBAL int profile_count;
GLOBAL player_t players[RACC_MAX_CLIENTS];
GLOBAL int player_count;
GLOBAL bot_t bots[RACC_MAX_CLIENTS];
GLOBAL int bot_count;
GLOBAL sound_t sounds[RACC_MAX_SOUNDS];
GLOBAL int sound_count;
GLOBAL ricochetsound_t ricochetsounds[RACC_MAX_RICOCHETSOUNDS];
GLOBAL int ricochetsound_count;
GLOBAL weapon_t weapons[RACC_MAX_WEAPONS];
GLOBAL int weapon_count;
GLOBAL playerbones_t playerbones;
GLOBAL usermsg_t usermsgs[RACC_MAX_USERMSGS];
GLOBAL int usermsgs_count;
GLOBAL void (*botMsgFunction) (void *p, int bot_index);
GLOBAL int botMsgIndex;
GLOBAL int messagestate;
GLOBAL debug_level_t DebugLevel;
GLOBAL int voiceicon_height;
GLOBAL int dummyent_model;
GLOBAL int speaker_model;
GLOBAL int beam_model;
GLOBAL Vector v_pathdebug_from;
GLOBAL Vector v_pathdebug_to;


// bot_client.cpp function prototypes
void BotClient_CS_VGUIMenu (void *p, int bot_index);
void BotClient_CS_ShowMenu (void *p, int bot_index);
void BotClient_CS_WeaponList (void *p, int bot_index);
void BotClient_CS_CurWeapon (void *p, int bot_index);
void BotClient_CS_AmmoX (void *p, int bot_index);
void BotClient_CS_AmmoPickup (void *p, int bot_index);
void BotClient_CS_WeapPickup (void *p, int bot_index);
void BotClient_CS_ItemPickup (void *p, int bot_index);
void BotClient_CS_Health (void *p, int bot_index);
void BotClient_CS_Battery (void *p, int bot_index);
void BotClient_CS_Damage (void *p, int bot_index);
void BotClient_CS_Money (void *p, int bot_index);
void BotClient_CS_ReloadSound (void *p, int bot_index);
void BotClient_CS_SayText (void *p, int bot_index);
void BotClient_CS_TextMsg (void *p, int bot_index);
void BotClient_CS_ScreenFade (void *p, int bot_index);
void BotClient_CS_StatusIcon (void *p, int bot_index);
void BotClient_CS_BarTime (void *p, int bot_index);
void BotClient_CS_TextMsg_All (void *p, int bot_index);
void BotClient_CS_DeathMsg_All (void *p, int bot_index);


// bot.cpp function prototypes
void BotCreate (void);
void BotReset (bot_t *pBot);
void BotBuyStuff (bot_t *pBot);
bool BotCheckForSpecialZones (bot_t *pBot);
bool BotCheckForGrenades (bot_t *pBot);
void BotCheckForItems (bot_t *pBot);
void BotPreThink (bot_t *pBot);
void BotThink (bot_t *pBot);
void BotPostThink (bot_t *pBot);
bool BotItemIsInteresting (bot_t *pBot, edict_t *pItem);
void BotDiscardItem (bot_t *pBot, edict_t *pItem);
void BotFindGoal (bot_t *pBot);
void BotAnswerToOrder (bot_t *pBot);
void BotReactToSound (bot_t *pBot, noise_t *sound);
bool BotHasPrimary (bot_t *pBot);
bool BotHasSecondary (bot_t *pBot);
bool BotHoldsPrimary (bot_t *pBot);
bool BotHoldsSecondary (bot_t *pBot);
bool ItemIsPrimary (edict_t *pItem);
bool ItemIsSecondary (edict_t *pItem);
bool PlayerIsVIP (edict_t *pPlayer);
void PlayClientSoundsForBots (edict_t *pPlayer);


// bot_start.cpp function prototypes
void BotStartGame (bot_t *pBot);


// bot_combat.cpp function prototypes
edict_t *BotCheckForEnemies (bot_t *pBot);
void BotSwitchToBestWeapon (bot_t *pBot);
void BotFireWeapon (bot_t *pBot, Vector v_enemy, int weapon_choice);
void BotShootAtEnemy (bot_t *pBot);
void BotPlantBomb (bot_t *pBot, Vector v_target);
void BotDefuseBomb (bot_t *pBot, edict_t *pBomb);


#endif // MOD_SPECIFIC_H

