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
// TFC version
//
// mod_specific.h
//

#ifndef MOD_SPECIFIC_H
#define MOD_SPECIFIC_H


// MOD name, game DLL paths and file names
#define GAME_DLL_PATH "tfc/dlls/tfc.dll"
#define METAMOD_DLL_PATH "tfc/addons/metamod/dlls/metamod.dll"
#define RACC_LOGFILE_PATH "racc/release/messages.log"
#define RACC_WELCOMESOUND "racc/welcome.wav"


// maximum number of known local (radio) sounds in Team Fortress Classic
#define RACC_MAX_LOCAL_SOUNDS 0


// maximum walk speed (without making footstep noises) in Team Fortress Classic
#define MAX_WALK_SPEED 220


// maximum player safe fall speed in Team Fortress Classic
#define MAX_SAFEFALL_SPEED 580


// class ID values for Team Fortress Classic
#define TFC_CLASS_CIVILIAN 0
#define TFC_CLASS_SCOUT 1
#define TFC_CLASS_SNIPER 2
#define TFC_CLASS_SOLDIER 3
#define TFC_CLASS_DEMOMAN 4
#define TFC_CLASS_MEDIC 5
#define TFC_CLASS_HWGUY 6
#define TFC_CLASS_PYRO 7
#define TFC_CLASS_SPY 8
#define TFC_CLASS_ENGINEER 9


// weapon ID values for Team Fortress Classic
#define TF_WEAPON_MEDIKIT 3
#define TF_WEAPON_SPANNER 4
#define TF_WEAPON_AXE 5
#define TF_WEAPON_SNIPERRIFLE 6
#define TF_WEAPON_AUTORIFLE 7
#define TF_WEAPON_SHOTGUN 8
#define TF_WEAPON_SUPERSHOTGUN 9
#define TF_WEAPON_NAILGUN 10
#define TF_WEAPON_SUPERNAILGUN 11
#define TF_WEAPON_GL 12
#define TF_WEAPON_FLAMETHROWER 13
#define TF_WEAPON_RPG 14
#define TF_WEAPON_IC 15
#define TF_WEAPON_AC 17
#define TF_WEAPON_TRANQ 20
#define TF_WEAPON_RAILGUN 21
#define TF_WEAPON_PL 22
#define TF_WEAPON_KNIFE 23
#define TF_WEAPON_GRENADE 24 // this one doesn't actually exists, it's a hack


// game start messages for Team Fortress Classic
#define MENU_TFC_IDLE 1
#define MENU_TFC_TEAMSELECT_TEAMMENU 2
#define MENU_TFC_TEAMSELECT_CLASSMENU 3


// specific bot orders for Team Fortress Classic
#define BOT_ORDER_DETONATEPLASTIC_5SECONDS 5
#define BOT_ORDER_DETONATEPLASTIC_20SECONDS 6
#define BOT_ORDER_DISARMPLASTIC 7
#define BOT_ORDER_DISGUISEENEMY 8
#define BOT_ORDER_DISGUISEFRIENDLY 9
#define BOT_ORDER_FEIGN 10
#define BOT_ORDER_STOPFEIGN 11
#define BOT_ORDER_BUILDSENTRY 12
#define BOT_ORDER_ROTATESENTRY_180DEGREES 13
#define BOT_ORDER_ROTATESENTRY_45DEGREES 14
#define BOT_ORDER_DETONATESENTRY 15
#define BOT_ORDER_BUILDDISPENSER 16
#define BOT_ORDER_DETONATEDISPENSER 17


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
GLOBAL edict_t *pListenserverEntity;
GLOBAL edict_t *pWorldEntity;
GLOBAL edict_t *pent_info_tfdetect;
GLOBAL edict_t *pent_item_tfgoal;
GLOBAL char team_names[32][16];
GLOBAL int num_teams;
GLOBAL int team_allies[4];
GLOBAL int team_class_limits[4];
GLOBAL flag_t flags[5];
GLOBAL int num_flags;
GLOBAL round_t round;
GLOBAL int max_armor[10];
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
void BotClient_TFC_VGUIMenu (void *p, int bot_index);
void BotClient_TFC_WeaponList (void *p, int bot_index);
void BotClient_TFC_CurWeapon (void *p, int bot_index);
void BotClient_TFC_AmmoX (void *p, int bot_index);
void BotClient_TFC_AmmoPickup (void *p, int bot_index);
void BotClient_TFC_SecAmmoVal (void *p, int bot_index);
void BotClient_TFC_WeapPickup (void *p, int bot_index);
void BotClient_TFC_ItemPickup (void *p, int bot_index);
void BotClient_TFC_Health (void *p, int bot_index);
void BotClient_TFC_Battery (void *p, int bot_index);
void BotClient_TFC_Damage (void *p, int bot_index);
void BotClient_TFC_SayText (void *p, int bot_index);
void BotClient_TFC_ScreenFade (void *p, int bot_index);
void BotClient_TFC_DeathMsg_All (void *p, int bot_index);


// bot.cpp function prototypes
void BotCreate (void);
void BotReset (bot_t *pBot);
bool BotCheckForSpecialZones (bot_t *pBot);
bool BotCheckForGrenades (bot_t *pBot);
void BotCheckForItems (bot_t *pBot);
void BotPreThink (bot_t *pBot);
void BotThink (bot_t *pBot);
void BotPostThink (bot_t *pBot);
void BotFindGoal (bot_t *pBot);
void BotAnswerToOrder (bot_t *pBot);
void BotReactToSound (bot_t *pBot, noise_t *sound);
void PlayClientSoundsForBots (edict_t *pPlayer);


// bot_start.cpp function prototypes
void BotStartGame (bot_t *pBot);


// bot_combat.cpp function prototypes
edict_t *BotCheckForEnemies (bot_t *pBot);
void BotSwitchToBestWeapon (bot_t *pBot);
void BotFireWeapon (bot_t *pBot, Vector v_enemy, int weapon_choice);
void BotFireGrenade (bot_t *pBot, int grenade_type);
void BotShootAtEnemy (bot_t *pBot);


#endif // MOD_SPECIFIC_H
