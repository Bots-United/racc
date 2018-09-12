// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// player.h
//


// player_t structure members
bool is_connected; // set to TRUE if this player slot is currently filled by a client
bool is_alive; // handy shortcut that tells us each frame if the player is alive or not
bool is_racc_bot; // set to TRUE if this player slot belongs to a RACC bot
bool is_watched; // set to TRUE if this player is spectated by the listen server client (debug feature)
edict_t *pEntity; // this player's entity the engine knows
char connection_name[64]; // local copy of this player's connection name (updated each frame)
char model[256]; // player model file name (or relative path) of this player's skin
char weapon_model[256]; // weapon model file name (or relative path) of this player's weapon
Vector v_origin; // spatial origin of this player in the virtual world
Vector angles; // body angles of this player (i.e, orientation in space)
Vector v_velocity; // world velocity of this player
float health; // this player's health
float max_health; // this player's max health
float armor; // this player's armor
float max_armor; // this player's max armor
Vector v_eyeposition; // spatial origin of the eyes of this player in the virtual world
Vector v_angle; // view angles of this player
Vector v_forward; // normalized forward vector relatively to this player's point of view
Vector v_right; // normalized right vector relatively to this player's point of view
Vector v_up; // normalized up vector relatively to this player's point of view
unsigned char environment; // whether this player is on ground, partly, in mid-air or in water
unsigned long input_buttons; // this player's keyboard state (INPUT_KEY_ #defines)
int money; // amount of money the player owns for buy action based MODs
float welcome_time; // date at which this player will be sent a welcome message
walkface_t *pFaceAtFeet; // pointer to the last face this player was walking on
int face_reachability; // type of reachability from this player's last walkface to the new one
Vector v_lastjump; // spatial location where this player issued his last jump
bool has_just_fallen; // set to TRUE if this player has just landed after a fall
float step_sound_time; // date under which this player will NOT make a footstep sound
float proximityweapon_swing_time; // date under which this player will NOT make a swing sound
test_result_t tr; // results of last test line issued by this player (how handy :))
