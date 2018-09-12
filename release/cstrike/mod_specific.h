// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// CSTRIKE version
//
// mod_specific.h
//

#ifndef MOD_SPECIFIC_H
#define MOD_SPECIFIC_H


// team ID values for Counter-Strike
#define CSTRIKE_TERRORIST 0
#define CSTRIKE_COUNTER_TERRORIST 1


// special buy commands for Counter-Strike
#define BUY_PRIMARYAMMO "buyammo1"
#define BUY_SECONDARYAMMO "buyammo2"
#define BUY_KEVLAR "buyequip;menuselect 1"
#define BUY_KEVLARHELMET "buyequip;menuselect 2"
#define BUY_DEFUSEKIT "buyequip;menuselect 6"
#define BUY_NIGHTVISION "buyequip;menuselect 7"


// radio messages definitions for Counter-Strike
#define RADIOMSG_COVERME "radio1;menuselect 1"
#define RADIOMSG_YOUTAKETHEPOINT "radio1;menuselect 2"
#define RADIOMSG_KEEPTHEPOSITION "radio1;menuselect 3"
#define RADIOMSG_STICKTOGETHER "radio1;menuselect 4"
#define RADIOMSG_FOLLOWME "radio1;menuselect 5"
#define RADIOMSG_TAKINGFIRE "radio1;menuselect 6"
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
#define MENU_CSTRIKE_IDLE 1
#define MENU_CSTRIKE_TEAMSELECT_MAINMENU 2
#define MENU_CSTRIKE_TEAMSELECT_COUNTERMENU 3
#define MENU_CSTRIKE_TEAMSELECT_TERRMENU 4
#define MENU_CSTRIKE_BUY_MAINMENU 5
#define MENU_CSTRIKE_BUY_PISTOLMENU 6
#define MENU_CSTRIKE_BUY_SHOTGUNMENU 7
#define MENU_CSTRIKE_BUY_RIFLEMENU 8
#define MENU_CSTRIKE_BUY_SUBMACHINEGUNMENU 9
#define MENU_CSTRIKE_BUY_MACHINEGUNMENU 10
#define MENU_CSTRIKE_BUY_EQUIPMENTMENU 11


// HUD icons
#define HUD_ICON_BOMB 0
#define HUD_ICON_DEFUSER 1


// interface globals
GLOBAL gamedll_funcs_t *gpGamedllFuncs; // for MDLL_*
GLOBAL mutil_funcs_t *gpMetaUtilFuncs; // for CALL_GAME_ENTITY
GLOBAL meta_globals_t *gpMetaGlobals; // for RETURN_META_*
GLOBAL META_FUNCTIONS gMetaFunctionTable; // for pre- and post- function tables
GLOBAL plugin_info_t Plugin_info; // plugin info structure
GLOBAL enginefuncs_t g_engfuncs; // engine functions
GLOBAL globalvars_t  *gpGlobals; // engine globals


// global variables declaration and referencing
GLOBAL char racc_version[9];
GLOBAL char racc_welcometext[256];
GLOBAL network_message_header_t message_header;
GLOBAL network_message_t message[256];
GLOBAL int message_size;
GLOBAL server_t server;
GLOBAL char g_argv[128];
GLOBAL bool isFakeClientCommand;
GLOBAL int fake_arg_count;
GLOBAL bsp_file_t bsp_file;
GLOBAL map_t map;
GLOBAL mission_t mission;
GLOBAL int bot_count;
GLOBAL player_t *pListenserverPlayer;
GLOBAL edict_t *pWorldEntity;
GLOBAL float f_team_radiotime[2];
GLOBAL player_t players[RACC_MAX_CLIENTS];
GLOBAL int player_count;
GLOBAL bot_language_t *languages;
GLOBAL int language_count;
GLOBAL profile_t *profiles;
GLOBAL int profile_count;
GLOBAL sound_t *sounds;
GLOBAL int sound_count;
GLOBAL ricochetsound_t *ricochetsounds;
GLOBAL int ricochetsound_count;
GLOBAL footstepsound_t *footstepsounds;
GLOBAL int footstepsound_count;
GLOBAL weapon_t *weapons;
GLOBAL int weapon_count;
GLOBAL debug_level_t DebugLevel;
GLOBAL game_config_t GameConfig;
GLOBAL referential_t referential;
GLOBAL likelevel_t default_likelevels;
GLOBAL int beam_model;


// bot_client.cpp function prototypes
void ExamineNetworkMessage (void);
void NetworkMessage_VGUIMenu (void);
void NetworkMessage_ShowMenu (void);
void NetworkMessage_WeaponList (void);
void NetworkMessage_CurWeapon (void);
void NetworkMessage_AmmoX (void);
void NetworkMessage_AmmoPickup (void);
void NetworkMessage_WeapPickup (void);
void NetworkMessage_ItemPickup (void);
void NetworkMessage_Health (void);
void NetworkMessage_Battery (void);
void NetworkMessage_Damage (void);
void NetworkMessage_Money (void);
void NetworkMessage_ReloadSound (void);
void NetworkMessage_SayText (void);
void NetworkMessage_TextMsg (void);
void NetworkMessage_ScreenFade (void);
void NetworkMessage_StatusIcon (void);
void NetworkMessage_BarTime (void);
void NetworkMessage_BombDrop (void);
void NetworkMessage_BombPickup (void);
void NetworkMessage_RoundTime (void);
void NetworkMessageAll_TextMsg (void);
void NetworkMessageAll_DeathMsg (void);


// bot.cpp function prototypes
void BotCreate (void);
void BotReset (player_t *pPlayer);
bool BotCheckForSpecialZones (player_t *pPlayer);
bool BotCheckForGrenades (player_t *pPlayer);
void BotCheckForItems (player_t *pPlayer);
void BotSense (player_t *pPlayer);
void BotThink (player_t *pPlayer);
void BotAct (player_t *pPlayer);
bool BotItemIsInteresting (player_t *pPlayer, edict_t *pItem);
void BotDiscardItem (player_t *pPlayer, edict_t *pItem);
void BotAnswerToOrder (player_t *pPlayer);
void BotReactToSound (player_t *pPlayer, noise_t *sound);
void PlayClientSoundsForBots (player_t *pPlayer);
int GetTeam (player_t *pPlayer);


// bot_start.cpp function prototypes
void BotStartGame (player_t *pPlayer);
void BotBuyStuff (player_t *pPlayer);


// bot_combat.cpp function prototypes
void BotCheckForEnemies (player_t *pPlayer);
void BotSwitchToBestWeapon (player_t *pPlayer);
void BotFireWeapon (player_t *pPlayer);
void BotShootAtEnemy (player_t *pPlayer);
void BotPlantBomb (player_t *pPlayer, Vector v_target);
void BotDefuseBomb (player_t *pPlayer, Vector v_target);


// bot_cognition.cpp function prototypes
void BotFindGoal (player_t *pPlayer);
void BotAnalyzeGoal (player_t *pPlayer);
void BotExecuteTask (player_t *pPlayer);


#endif // MOD_SPECIFIC_H

