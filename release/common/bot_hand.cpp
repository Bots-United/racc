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
// Rational Autonomous Cybernetic Commandos AI
//
// bot_hand.cpp
//

#include "racc.h"

extern weapon_t weapon_defs[MAX_WEAPONS];
extern int player_count;
extern debug_level_t DebugLevel;


typedef struct
{
   int   iId; // the weapon ID value
   char  weapon_name[64]; // name of the weapon when selecting it
   int   skill_level; // bot skill must at least equal this value for bot to use this weapon
   float primary_min_distance; // 0 = no minimum
   float primary_max_distance; // 9999 = no maximum
   bool  can_use_underwater; // can use this weapon underwater
   int   min_primary_ammo; // minimum ammout of primary ammo needed to fire
   bool  primary_fire_hold; // hold down primary fire button to use?
   bool  primary_fire_burst; // use burst fire?
   float primary_burst_delay; // time to charge weapon
} bot_weapon_select_t;

typedef struct
{
   int iId;
   float primary_base_delay;
   float primary_min_delay[5];
   float primary_max_delay[5];
} bot_fire_delay_t;


bot_weapon_select_t cs_weapon_select[] = {
   {CS_WEAPON_HEGRENADE, "weapon_hegrenade", 3, 300, 1000, FALSE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_FLASHBANG, "weapon_flashbang", 3, 200, 850, FALSE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_SMOKEGRENADE, "weapon_smokegrenade", 3, 500, 1600, FALSE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_M249, "weapon_m249", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.1},
   {CS_WEAPON_MP5NAVY, "weapon_mp5navy", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.1},
   {CS_WEAPON_TMP, "weapon_tmp", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.2},
   {CS_WEAPON_P90, "weapon_p90", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.2},
   {CS_WEAPON_MAC10, "weapon_mac10", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.2},
   {CS_WEAPON_UMP45, "weapon_ump45", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.2},
   {CS_WEAPON_SCOUT, "weapon_scout", 1, 5.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_AWP, "weapon_awp", 1, 5.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_G3SG1, "weapon_g3sg1", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.1},
   {CS_WEAPON_SG550, "weapon_sg550", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.1},
   {CS_WEAPON_AK47, "weapon_ak47", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.1},
   {CS_WEAPON_SG552, "weapon_sg552", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.2},
   {CS_WEAPON_M4A1, "weapon_m4a1", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.2},
   {CS_WEAPON_AUG, "weapon_aug", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.2},
   {CS_WEAPON_M3, "weapon_m3", 1, 5.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_XM1014, "weapon_xm1014", 1, 5.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_KNIFE, "weapon_knife", 1, 0.0, 50.0, TRUE, 0, TRUE, FALSE, 0.0},
   {CS_WEAPON_USP, "weapon_usp", 1, 0.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_GLOCK18, "weapon_glock18", 1, 0.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0}, 
   {CS_WEAPON_DEAGLE, "weapon_deagle", 1, 0.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_P228, "weapon_p228", 1, 0.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_ELITE, "weapon_elite", 1, 0.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_FIVESEVEN, "weapon_fiveseven", 1, 0.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   /* terminator */
   {0, "", 0, 0.0, 0.0, TRUE, 1, FALSE, FALSE, 0.0}
};


bot_fire_delay_t cs_fire_delay[] = {
   {CS_WEAPON_HEGRENADE, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_FLASHBANG, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_SMOKEGRENADE, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_M249, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_MP5NAVY, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_TMP, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_P90, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_MAC10, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_UMP45, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_SCOUT, 0.5, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_AWP, 0.5, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_G3SG1, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_SG550, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_AK47, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_SG552, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_M4A1, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_AUG, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_M3, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_XM1014, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_KNIFE, 0.1, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_USP, 0.1, {0.2, 0.2, 0.1, 0.1, 0.0}, {0.3, 0.2, 0.2, 0.1, 0.1}},
   {CS_WEAPON_GLOCK18, 0.1, {0.2, 0.2, 0.1, 0.1, 0.0}, {0.3, 0.2, 0.2, 0.1, 0.1}},
   {CS_WEAPON_DEAGLE, 0.1, {0.2, 0.2, 0.1, 0.1, 0.0}, {0.3, 0.2, 0.2, 0.1, 0.1}},
   {CS_WEAPON_P228, 0.1, {0.2, 0.2, 0.1, 0.1, 0.0}, {0.3, 0.2, 0.2, 0.1, 0.1}},
   {CS_WEAPON_ELITE, 0.1, {0.2, 0.2, 0.1, 0.1, 0.0}, {0.3, 0.2, 0.2, 0.1, 0.1}},
   {CS_WEAPON_FIVESEVEN, 0.1, {0.2, 0.2, 0.1, 0.1, 0.0}, {0.3, 0.2, 0.2, 0.1, 0.1}},
   /* terminator */
   {0, 0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}}
};
