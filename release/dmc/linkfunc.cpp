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
// DMC version
//
// linkfunc.cpp
//

#include "racc.h"


// entities for Deathmatch Classic 1.1.1.0...
LINK_ENTITY_TO_FUNC(DelayedUse);
LINK_ENTITY_TO_FUNC(ambient_generic);
LINK_ENTITY_TO_FUNC(beam);
LINK_ENTITY_TO_FUNC(bodyque);
LINK_ENTITY_TO_FUNC(button_target);
LINK_ENTITY_TO_FUNC(env_beam);
LINK_ENTITY_TO_FUNC(env_beverage);
LINK_ENTITY_TO_FUNC(env_blood);
LINK_ENTITY_TO_FUNC(env_bubbles);
LINK_ENTITY_TO_FUNC(env_debris);
LINK_ENTITY_TO_FUNC(env_explosion);
LINK_ENTITY_TO_FUNC(env_fade);
LINK_ENTITY_TO_FUNC(env_fog);
LINK_ENTITY_TO_FUNC(env_funnel);
LINK_ENTITY_TO_FUNC(env_global);
LINK_ENTITY_TO_FUNC(env_glow);
LINK_ENTITY_TO_FUNC(env_laser);
LINK_ENTITY_TO_FUNC(env_lightning);
LINK_ENTITY_TO_FUNC(env_message);
LINK_ENTITY_TO_FUNC(env_render);
LINK_ENTITY_TO_FUNC(env_shake);
LINK_ENTITY_TO_FUNC(env_shooter);
LINK_ENTITY_TO_FUNC(env_sound);
LINK_ENTITY_TO_FUNC(env_spark);
LINK_ENTITY_TO_FUNC(env_sprite);
LINK_ENTITY_TO_FUNC(fireanddie);
LINK_ENTITY_TO_FUNC(func_breakable);
LINK_ENTITY_TO_FUNC(func_button);
LINK_ENTITY_TO_FUNC(func_conveyor);
LINK_ENTITY_TO_FUNC(func_door);
LINK_ENTITY_TO_FUNC(func_door_rotating);
LINK_ENTITY_TO_FUNC(func_friction);
LINK_ENTITY_TO_FUNC(func_guntarget);
LINK_ENTITY_TO_FUNC(func_illusionary);
LINK_ENTITY_TO_FUNC(func_ladder);
LINK_ENTITY_TO_FUNC(func_monsterclip);
LINK_ENTITY_TO_FUNC(func_pendulum);
LINK_ENTITY_TO_FUNC(func_plat);
LINK_ENTITY_TO_FUNC(func_platrot);
LINK_ENTITY_TO_FUNC(func_pushable);
LINK_ENTITY_TO_FUNC(func_rot_button);
LINK_ENTITY_TO_FUNC(func_rotating);
LINK_ENTITY_TO_FUNC(func_tank);
LINK_ENTITY_TO_FUNC(func_tankcontrols);
LINK_ENTITY_TO_FUNC(func_tanklaser);
LINK_ENTITY_TO_FUNC(func_tankmortar);
LINK_ENTITY_TO_FUNC(func_tankrocket);
LINK_ENTITY_TO_FUNC(func_trackautochange);
LINK_ENTITY_TO_FUNC(func_trackchange);
LINK_ENTITY_TO_FUNC(func_tracktrain);
LINK_ENTITY_TO_FUNC(func_train);
LINK_ENTITY_TO_FUNC(func_traincontrols);
LINK_ENTITY_TO_FUNC(func_wall);
LINK_ENTITY_TO_FUNC(func_wall_toggle);
LINK_ENTITY_TO_FUNC(func_water);
LINK_ENTITY_TO_FUNC(game_counter);
LINK_ENTITY_TO_FUNC(game_counter_set);
LINK_ENTITY_TO_FUNC(game_end);
LINK_ENTITY_TO_FUNC(game_player_equip);
LINK_ENTITY_TO_FUNC(game_player_hurt);
LINK_ENTITY_TO_FUNC(game_player_team);
LINK_ENTITY_TO_FUNC(game_score);
LINK_ENTITY_TO_FUNC(game_team_master);
LINK_ENTITY_TO_FUNC(game_team_set);
LINK_ENTITY_TO_FUNC(game_text);
LINK_ENTITY_TO_FUNC(game_zone_player);
LINK_ENTITY_TO_FUNC(gibshooter);
LINK_ENTITY_TO_FUNC(info_intermission);
LINK_ENTITY_TO_FUNC(info_landmark);
LINK_ENTITY_TO_FUNC(info_node);
LINK_ENTITY_TO_FUNC(info_node_air);
LINK_ENTITY_TO_FUNC(info_null);
LINK_ENTITY_TO_FUNC(info_player_deathmatch);
LINK_ENTITY_TO_FUNC(info_player_start);
LINK_ENTITY_TO_FUNC(info_target);
LINK_ENTITY_TO_FUNC(info_teleport_destination);
LINK_ENTITY_TO_FUNC(infodecal);
LINK_ENTITY_TO_FUNC(item_armor1);
LINK_ENTITY_TO_FUNC(item_armor2);
LINK_ENTITY_TO_FUNC(item_armor3);
LINK_ENTITY_TO_FUNC(item_armorInv);
LINK_ENTITY_TO_FUNC(item_artifact_envirosuit);
LINK_ENTITY_TO_FUNC(item_artifact_invisibility);
LINK_ENTITY_TO_FUNC(item_artifact_invulnerability);
LINK_ENTITY_TO_FUNC(item_artifact_super_damage);
LINK_ENTITY_TO_FUNC(item_backpack);
LINK_ENTITY_TO_FUNC(item_cells);
LINK_ENTITY_TO_FUNC(item_health);
LINK_ENTITY_TO_FUNC(item_rockets);
LINK_ENTITY_TO_FUNC(item_shells);
LINK_ENTITY_TO_FUNC(item_sodacan);
LINK_ENTITY_TO_FUNC(item_spikes);
LINK_ENTITY_TO_FUNC(item_weapon);
LINK_ENTITY_TO_FUNC(light);
LINK_ENTITY_TO_FUNC(light_environment);
LINK_ENTITY_TO_FUNC(light_spot);
LINK_ENTITY_TO_FUNC(momentary_door);
LINK_ENTITY_TO_FUNC(momentary_rot_button);
LINK_ENTITY_TO_FUNC(monster_hevsuit_dead);
LINK_ENTITY_TO_FUNC(multi_manager);
LINK_ENTITY_TO_FUNC(multisource);
LINK_ENTITY_TO_FUNC(node_viewer);
LINK_ENTITY_TO_FUNC(node_viewer_fly);
LINK_ENTITY_TO_FUNC(node_viewer_human);
LINK_ENTITY_TO_FUNC(node_viewer_large);
LINK_ENTITY_TO_FUNC(path_corner);
LINK_ENTITY_TO_FUNC(path_track);
LINK_ENTITY_TO_FUNC(player);
LINK_ENTITY_TO_FUNC(player_loadsaved);
LINK_ENTITY_TO_FUNC(player_weaponstrip);
LINK_ENTITY_TO_FUNC(quake_nail);
LINK_ENTITY_TO_FUNC(quake_rocket);
LINK_ENTITY_TO_FUNC(spark_shower);
LINK_ENTITY_TO_FUNC(speaker);
LINK_ENTITY_TO_FUNC(target_cdaudio);
LINK_ENTITY_TO_FUNC(teledeath);
LINK_ENTITY_TO_FUNC(test_effect);
LINK_ENTITY_TO_FUNC(testhull);
LINK_ENTITY_TO_FUNC(trigger);
LINK_ENTITY_TO_FUNC(trigger_auto);
LINK_ENTITY_TO_FUNC(trigger_autosave);
LINK_ENTITY_TO_FUNC(trigger_camera);
LINK_ENTITY_TO_FUNC(trigger_cdaudio);
LINK_ENTITY_TO_FUNC(trigger_changelevel);
LINK_ENTITY_TO_FUNC(trigger_changetarget);
LINK_ENTITY_TO_FUNC(trigger_counter);
LINK_ENTITY_TO_FUNC(trigger_endsection);
LINK_ENTITY_TO_FUNC(trigger_env_hurt);
LINK_ENTITY_TO_FUNC(trigger_gravity);
LINK_ENTITY_TO_FUNC(trigger_hurt);
LINK_ENTITY_TO_FUNC(trigger_monsterjump);
LINK_ENTITY_TO_FUNC(trigger_multiple);
LINK_ENTITY_TO_FUNC(trigger_once);
LINK_ENTITY_TO_FUNC(trigger_push);
LINK_ENTITY_TO_FUNC(trigger_relay);
LINK_ENTITY_TO_FUNC(trigger_teleport);
LINK_ENTITY_TO_FUNC(trigger_transition);
LINK_ENTITY_TO_FUNC(weapon_grenadelauncher);
LINK_ENTITY_TO_FUNC(weapon_lightning);
LINK_ENTITY_TO_FUNC(weapon_nailgun);
LINK_ENTITY_TO_FUNC(weapon_quakegun);
LINK_ENTITY_TO_FUNC(weapon_rocketlauncher);
LINK_ENTITY_TO_FUNC(weapon_supernailgun);
LINK_ENTITY_TO_FUNC(weapon_supershotgun);
LINK_ENTITY_TO_FUNC(weaponbox);
LINK_ENTITY_TO_FUNC(worldspawn);
