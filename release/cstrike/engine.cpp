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
// engine.cpp
//

#include "racc.h"

extern bool is_multiplayer_game;
extern enginefuncs_t g_engfuncs;
extern player_t players[MAX_CLIENTS_SUPPORTED_BY_ENGINE];
extern bot_t bots[MAX_CLIENTS_SUPPORTED_BY_ENGINE];
extern char *g_argv;
extern bool isFakeClientCommand;
extern int fake_arg_count;
extern round_t round;

usermsg_t usermsgs[MAX_USERMSG_TYPES];
int usermsgs_count = 0;
void (*botMsgFunction) (void *, int) = NULL;
int botMsgIndex;
int messagestate = 0;



int pfnPrecacheModel (char *s)
{
   // the purpose of this function is to precache (load in memory) the 3D models that can have
   // to be spawned during the game. This involves player skins, weapons, items, etc. The
   // precaching of items can only be done at each loading of a new map.

   return ((*g_engfuncs.pfnPrecacheModel) (s));
}


int pfnPrecacheSound (char *s)
{
   // the purpose of this function is to precache (load in memory) the sounds that can have to
   // be played during the game. The precaching of items is done at each loading of a new map.
   // We hook this function in order to compute at precache time the average loudness of the sound
   // being precached, for having a quick reference when the bots have to hear one of them. Local
   // variables have been made static to speed up recursive calls of this function, which is
   // extensively called at server boot time.

   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      static int sound_index = 0;

      sound_index = (*g_engfuncs.pfnPrecacheSound) (s); // ask the engine to precache the sound
      PrecacheSoundForBots (s, sound_index); // precache that sound for the bots too
      return (sound_index); // and return the index at which it has been precached
   }

   return ((*g_engfuncs.pfnPrecacheSound) (s)); // let the engine precache the sound
}


void pfnSetModel (entity_t *e, const char *m)
{
   // the purpose of this function is to ask the engine to set a model m (like "models/
   // mechgibs.mdl") to entity e. The engine itself will provide the necessary changes
   // in the edict's structure for it.

   (*g_engfuncs.pfnSetModel) (e, m);
}


int pfnModelIndex (const char *m)
{
   return ((*g_engfuncs.pfnModelIndex) (m));
}


int pfnModelFrames (int modelIndex)
{
   return ((*g_engfuncs.pfnModelFrames) (modelIndex));
}


void pfnSetSize (entity_t *e, const float *rgflMin, const float *rgflMax)
{
   // the purpose of this function is to ask the engine to set the bounding box size of the
   // entity pointed to by e. The vector values passed in rgflMin and rgflMax as arrays of
   // floats correspond to the lower left and upper right corners of the bounding box.
   // Note that bounding boxes are always axial, i.e. parallel to the map axises.

   (*g_engfuncs.pfnSetSize) (e, rgflMin, rgflMax); // tell the engine about e's new bounding box
}


void pfnChangeLevel (char *s1, char *s2)
{
   // the purpose of this function is to ask the engine to shutdown the server and restart a
   // new one running the map whose name is s1. It is used ONLY IN SINGLE PLAYER MODE and is
   // transparent to the user, because it saves the player state and equipment and restores it
   // back in the new level. The "changelevel trigger point" in the old level is linked to the
   // new level's spawn point using the s2 string, which is formatted as follows: "trigger_name
   // to spawnpoint_name", without spaces (for example, "tr_1atotr_2lm" would tell the engine
   // the player has reached the trigger point "tr_1a" and has to spawn in the next level on the
   // spawn point named "tr_2lm".

   (*g_engfuncs.pfnChangeLevel) (s1, s2); // tell the engine to issue the change level procedure
}


void pfnGetSpawnParms (entity_t *ent)
{
   (*g_engfuncs.pfnGetSpawnParms) (ent);
}


void pfnSaveSpawnParms (entity_t *ent)
{
   (*g_engfuncs.pfnSaveSpawnParms) (ent);
}


float pfnVecToYaw (const float *rgflVector)
{
   return ((*g_engfuncs.pfnVecToYaw) (rgflVector));
}


void pfnVecToAngles (const float *rgflVectorIn, float *rgflVectorOut)
{
   // the purpose of this function is to convert a spatial location determined by the vector
   // passed in by rgflVectorIn as an array of float into absolute angles from the origin of
   // the world that are written back in the rgflVectorOut array of float (which can be easily
   // converted to angle vector).

   (*g_engfuncs.pfnVecToAngles) (rgflVectorIn, rgflVectorOut); // translate vector to absolute angles
}


void pfnMoveToOrigin (entity_t *ent, const float *pflGoal, float dist, int iMoveType)
{
   (*g_engfuncs.pfnMoveToOrigin) (ent, pflGoal, dist, iMoveType);
}


void pfnChangeYaw (entity_t *ent)
{
   // the purpose of this function is to ask the engine to change the view angle yaw (horizontal
   // angle) of a monster entity pointed to by ent towards this entity's ideal view angles
   // (stored in the entity's entvars_t field in the idealpitch and ideal_yaw variables), by
   // increment of the value of this entity's yaw speed (which is also stored in its entity
   // variables as yaw_speed, this value setting the maximal amount of degrees the monster entity
   // is allowed to turn in ONE frame).

   (*g_engfuncs.pfnChangeYaw) (ent); // make the engine update this monster's yaw
}


void pfnChangePitch (entity_t *ent)
{
   // the purpose of this function is to ask the engine to change the view angle pitch (vertical
   // angle) of a monster entity pointed to by ent towards this entity's ideal view angles
   // (stored in the entity's entvars_t field in the idealpitch and ideal_yaw variables), by
   // increment of the value of this entity's pitch speed (which is also stored in its entity
   // variables as pitch_speed, this value setting the maximal amount of degrees the monster
   // entity is allowed to turn in ONE frame).

   (*g_engfuncs.pfnChangePitch) (ent); // make the engine update this monster's pitch
}


entity_t *pfnFindEntityByString (entity_t *pEdictStartSearchAfter, const char *pszField, const char *pszValue)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      int index;

      // is it a map reset ?
      if (strcmp (pszValue, "info_map_parameters") == 0)
      {
         round.f_start_time = CurrentTime (); // save round start time

         for (index = 0; index < MaxClientsOnServer (); index++)
            if (bots[index].is_active && !IsNull (bots[index].pEntity))
               BotReset (&bots[index]); // reset active bots
      }
   }

   return ((*g_engfuncs.pfnFindEntityByString) (pEdictStartSearchAfter, pszField, pszValue));
}


int pfnGetEntityIllum (entity_t *pEnt)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      // if pEnt is a bot, we need to create a temporary entity to correctly retrieve the
      // fakeclient's illumination (thanks to Tom Simpson from FoxBot for this engine bug fix)
      if (IsABot (pEnt))
         return ((*g_engfuncs.pfnGetEntityIllum) (bots[PlayerIndexOf (pEnt)].pLightEntity));
   }

   return ((*g_engfuncs.pfnGetEntityIllum) (pEnt));
}


entity_t *pfnFindEntityInSphere (entity_t *pEdictStartSearchAfter, const float *org, float rad)
{
   return ((*g_engfuncs.pfnFindEntityInSphere) (pEdictStartSearchAfter, org, rad));
}


entity_t *pfnFindClientInPVS (entity_t *pEntity)
{
   return ((*g_engfuncs.pfnFindClientInPVS) (pEntity));
}


entity_t *pfnEntitiesInPVS (entity_t *pplayer)
{
   return ((*g_engfuncs.pfnEntitiesInPVS) (pplayer));
}


void pfnMakeVectors (const float *rgflVector)
{
   (*g_engfuncs.pfnMakeVectors) (rgflVector);
}


void pfnAngleVectors (const float *rgflVector, float *forward, float *right, float *up)
{
   (*g_engfuncs.pfnAngleVectors) (rgflVector, forward, right, up);
}


entity_t *pfnCreateEntity (void)
{
   entity_t *pent = (*g_engfuncs.pfnCreateEntity) ();
   return (pent);
}


void pfnRemoveEntity (entity_t *e)
{
   (*g_engfuncs.pfnRemoveEntity) (e);
}


entity_t *pfnCreateNamedEntity (int className)
{
   entity_t *pent = (*g_engfuncs.pfnCreateNamedEntity) (className);
   return (pent);
}


void pfnMakeStatic (entity_t *ent)
{
   (*g_engfuncs.pfnMakeStatic) (ent);
}


int pfnEntIsOnFloor (entity_t *e)
{
   // this function asks the engine to check if the entity e is in mid-air, or touches the floor.
   // It returns a positive value (1) if entity e is touching the ground, 0 otherwise. Player
   // entities are handled differently though, because such entities store their "ground state"
   // directly in their entvars_t entity variables, under the flags bitmap variable (either
   // FL_ONGROUND, or FL_PARTIALGROUND). That's why this function should only be used for non
   // player entities.

   return ((*g_engfuncs.pfnEntIsOnFloor) (e)); // return whether entity touches the floor or not
}


int pfnDropToFloor (entity_t *e)
{
   // this function asks the engine to change the vector location (origin) of entity e so that
   // it will touch the ground. Most map makers place entities slightly in mid-air, to avoid
   // the moving ones to get stuck in case they would slightly overlap each other with the
   // world. Engine programmers got it fine: at each start of a new level, the appropriate
   // entities are dropped to floor. Note we are not using engine physics for this, the result
   // is immediate, like a teleport.

   return ((*g_engfuncs.pfnDropToFloor) (e)); // drop entity e to floor
}


int pfnWalkMove (entity_t *ent, float yaw, float dist, int iMode)
{
   // this function is the equivalent of pfnRunPlayerMove, but for monsters. It tells the engine
   // to play the movement animation of the monster character ent, which will face the direction
   // yaw (yaw is a view angle), using mode iMode (determining whether the monster is allowed to
   // pass through entities, for example WALKMOVE_NORMAL means the monster will collide with the
   // world and activate triggers like every player would instead). dist is the distance that
   // will have been walked once the animation is over, i.e. for most monsters, the step size.
   // Setting it to a low value could turn your monster into a Michael Jackson "moonwalking" :)
   // Note that whereas a RunPlayerMove movement lasts strictly the duration of the frame, a
   // monster WalkMove extends during several frames, until the animation is completed.

   // ask the engine to make that monster move
   return ((*g_engfuncs.pfnWalkMove) (ent, yaw, dist, iMode));
}


void pfnSetOrigin (entity_t *e, const float *rgflOrigin)
{
   // this function tells the engine to change the origin (vector location in the virtual world)
   // of entity e to the vector location rgflOrigin (which must point to a vector).

   (*g_engfuncs.pfnSetOrigin) (e, rgflOrigin); // tells the engine about the new origin
}


void pfnEmitSound (entity_t *entity, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch)
{
   // this function tells the engine that the entity pointed to by "entity", is emitting a sound
   // which filename is "sample", at level "channel" (CHAN_VOICE, etc...), with "volume" as
   // loudness multiplicator (normal volume VOL_NORM is 1.0), with a pitch of "pitch" (normal
   // pitch PITCH_NORM is 100.0), and that this sound has to be attenuated by distance in air
   // according to the value of "attenuation" (normal attenuation ATTN_NORM is 0.8 ; ATTN_NONE
   // means no attenuation with distance). Optionally flags "fFlags" can be passed, which I don't
   // know the heck of the purpose. After we tell the engine to emit the sound, we have to call
   // DispatchSound() to bring the sound to the ears of the bots. Since bots have no client DLL
   // to handle this for them, such a job has to be done manually.

   // tell the engine to emit the sound
   (*g_engfuncs.pfnEmitSound) (entity, channel, sample, volume, attenuation, fFlags, pitch);

   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
      DispatchSound (sample, OriginOf (entity), volume, attenuation); // dispatch that sound
}


void pfnEmitAmbientSound (entity_t *entity, float *pos, const char *samp, float vol, float attenuation, int fFlags, int pitch)
{
   // this function tells the engine that the entity pointed to by "entity", is emitting a sound
   // which filename is "samp", at position "pos" (pos must point to a vector), with "vol" as
   // loudness multiplicator (normal volume VOL_NORM is 1.0), with a pitch of "pitch" (normal
   // pitch PITCH_NORM is 100.0), and that this sound has to be attenuated by distance in air
   // according to the value of "attenuation" (normal attenuation ATTN_NORM is 0.8 ; ATTN_NONE
   // means no attenuation with distance). Optionally flags "fFlags" can be passed, which I don't
   // know the heck of the purpose. After we tell the engine to emit the sound, we have to call
   // DispatchSound() to bring the sound to the ears of the bots. Since bots have no client DLL
   // to handle this for them, such a job has to be done manually.

   // tell the engine to emit the sound
   (*g_engfuncs.pfnEmitAmbientSound) (entity, pos, samp, vol, attenuation, fFlags, pitch);

   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
      DispatchSound (samp, pos, vol, attenuation); // dispatch that sound for the bots to hear it
}


void pfnTraceLine (const float *v1, const float *v2, int fNoMonsters, entity_t *pentToSkip, TraceResult *ptr)
{
   // this function is one of the most important of the game. It is a test line. By calling
   // TraceLine the engine is told to check the direct reachability between two points, and to
   // return the answer in a special dedicated structure. In detail, it is about starting from
   // spatial location v1, going in direct line to v2 (v1 and v2 point to vectors), ignoring or
   // not entities that are monsters along the way (depending whether fNoMonsters is set to
   // enumerated type ignore_monsters or dont_ignore_monsters), ignoring if needed the entity it
   // starts within, pentToSkip, and returning the results of the trace in structure ptr, which
   // results may contain lots of useful info. The most important is the flFraction field, a
   // fraction of 1 proportionnal to the distance that has been successfully reached without
   // obstacles. I.e, if flFraction is 0.5, the trace has covered half the distance successfully.
   // Have a look at UTIL_TraceLine where things are explained a little better. Also note that
   // local variables have been declared static for speedup purposes.

   static int player_index;
   static entity_t *pPlayer;

   // tell the engine to fire a test line from v1 to v2
   (*g_engfuncs.pfnTraceLine) (v1, v2, fNoMonsters, pentToSkip, ptr);

   // if this traceline was fired by a valid player AND this player is alive...
   if (!IsNull (pentToSkip) && IsAPlayer (pentToSkip) && IsAlive (pentToSkip))
   {
      // cycle through all players
      for (player_index = 0; player_index < MaxClientsOnServer (); player_index++)
      {
         pPlayer = PlayerAtIndex (player_index); // get pointer to player entity

         if (IsNull (pPlayer) || !IsAlive (pPlayer))
            continue; // skip invalid players and dead players

         // if this player fired that traceline...
         if (pentToSkip == pPlayer)
            players[player_index].prev_tr = *ptr; // remember its trace results
      }
   }
}


void pfnTraceToss (entity_t *pent, entity_t *pentToIgnore, TraceResult *ptr)
{
   (*g_engfuncs.pfnTraceToss) (pent, pentToIgnore, ptr);
}


int pfnTraceMonsterHull (entity_t *pEntity, const float *v1, const float *v2, int fNoMonsters, entity_t *pentToSkip, TraceResult *ptr)
{
   return ((*g_engfuncs.pfnTraceMonsterHull) (pEntity, v1, v2, fNoMonsters, pentToSkip, ptr));
}


void pfnTraceHull (const float *v1, const float *v2, int fNoMonsters, int hullNumber, entity_t *pentToSkip, TraceResult *ptr)
{
   (*g_engfuncs.pfnTraceHull) (v1, v2, fNoMonsters, hullNumber, pentToSkip, ptr);
}


void pfnTraceModel (const float *v1, const float *v2, int hullNumber, entity_t *pent, TraceResult *ptr)
{
   (*g_engfuncs.pfnTraceModel) (v1, v2, hullNumber, pent, ptr);
}


const char *pfnTraceTexture (entity_t *pTextureEntity, const float *v1, const float *v2)
{
   return ((*g_engfuncs.pfnTraceTexture) (pTextureEntity, v1, v2));
}


void pfnTraceSphere (const float *v1, const float *v2, int fNoMonsters, float radius, entity_t *pentToSkip, TraceResult *ptr)
{
   (*g_engfuncs.pfnTraceSphere) (v1, v2, fNoMonsters, radius, pentToSkip, ptr);
}


void pfnGetAimVector (entity_t *ent, float speed, float *rgflReturn)
{
   (*g_engfuncs.pfnGetAimVector) (ent, speed, rgflReturn);
}


void pfnServerCommand (char *str)
{
   // the purpose of this function is to execute the server command contained in the string
   // pointed to by str on the server

   (*g_engfuncs.pfnServerCommand) (str);
}


void pfnServerExecute (void)
{
   (*g_engfuncs.pfnServerExecute) ();
}


void pfnClientCommand (entity_t *pEntity, char *szFmt, ...)
{
   // this function forces the client whose player entity is pEntity to issue a client command.
   // How it works is that clients all have a g_argv global string in their client DLL that
   // stores the command string; if ever that string is filled with characters, the client DLL
   // sends it to the engine as a command to be executed. When the engine has executed that
   // command, this g_argv string is reset to zero. Here is somehow a curious implementation of
   // ClientCommand: the engine sets the command it wants the client to issue in his g_argv, then
   // the client DLL sends it back to the engine, the engine receives it then executes the
   // command therein. Don't ask me why we need all this complicated crap. Anyhow since bots have
   // no client DLL, be certain never to call this function upon a bot entity, else it will just
   // make the server crash. Since hordes of uncautious, not to say stupid, programmers don't
   // even imagine some players on their servers could be bots, this check is performed less than
   // sometimes actually by their side, that's why we strongly recommend to check it here too. In
   // case it's a bot asking for a client command, we handle it like we do for bot commands, ie
   // using FakeClientCommand().

   va_list argptr;
   static char string[1024];

   // concatenate szFmt into one single string
   va_start (argptr, szFmt);
   vsprintf (string, szFmt, argptr);
   va_end (argptr);

   // is it a bot wanting to issue that client command ?
   if (IsABot (pEntity))
      FakeClientCommand (pEntity, string); // if so, handle it like we do for normal bot commands
   else
      (*g_engfuncs.pfnClientCommand) (pEntity, string); // else ask the engine
}


void pfnParticleEffect (const float *org, const float *dir, float color, float count)
{
   (*g_engfuncs.pfnParticleEffect) (org, dir, color, count);
}


void pfnLightStyle (int style, char *val)
{
   (*g_engfuncs.pfnLightStyle) (style, val);
}


int pfnDecalIndex (const char *name)
{
   return ((*g_engfuncs.pfnDecalIndex) (name));
}


int pfnPointContents (const float *rgflVector)
{
   return ((*g_engfuncs.pfnPointContents) (rgflVector));
}


void pfnMessageBegin (int msg_dest, int msg_type, const float *pOrigin, entity_t *ed)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      messagestate = 0; // clear message state (message is starting)

      // if this message is directed to a player in particular AND this player is a bot...
      if (!IsNull (ed) && IsABot (ed))
      {
         // if developer mode is high, notify that a bot is getting a message
         if (DeveloperMode () == DEVELOPER_VERBOSE)
            ServerConsole_printf ("Bot \"%s\" receives message: %s\n", NetnameOf (ed), GetUserMsgName (msg_type));

         botMsgFunction = NULL; // no msg function until known otherwise
         botMsgIndex = PlayerIndexOf (ed); // index of bot receiving message

         // assign the right parsing function pointer given the type of message it is
         if (msg_type == GetUserMsgId ("AmmoPickup"))
            botMsgFunction = BotClient_CS_AmmoPickup;
         else if (msg_type == GetUserMsgId ("AmmoX"))
            botMsgFunction = BotClient_CS_AmmoX;
         else if (msg_type == GetUserMsgId ("Battery"))
            botMsgFunction = BotClient_CS_Battery;
         else if (msg_type == GetUserMsgId ("CurWeapon"))
            botMsgFunction = BotClient_CS_CurrentWeapon;
         else if (msg_type == GetUserMsgId ("Damage"))
            botMsgFunction = BotClient_CS_Damage;
         else if (msg_type == GetUserMsgId ("Health"))
            botMsgFunction = BotClient_CS_Health;
         else if (msg_type == GetUserMsgId ("ItemPickup"))
            botMsgFunction = BotClient_CS_ItemPickup;
         else if (msg_type == GetUserMsgId ("Money"))
            botMsgFunction = BotClient_CS_Money;
         else if (msg_type == GetUserMsgId ("ReloadSound"))
            botMsgFunction = BotClient_CS_ReloadSound;
         else if (msg_type == GetUserMsgId ("SayText"))
            botMsgFunction = BotClient_CS_SayText;
         else if (msg_type == GetUserMsgId ("ScreenFade"))
            botMsgFunction = BotClient_CS_ScreenFade;
         else if (msg_type == GetUserMsgId ("ShowMenu"))
            botMsgFunction = BotClient_CS_ShowMenu;
         else if (msg_type == GetUserMsgId ("StatusIcon"))
            botMsgFunction = BotClient_CS_StatusIcon;
         else if (msg_type == GetUserMsgId ("TextMsg"))
            botMsgFunction = BotClient_CS_TextMsg;
         else if (msg_type == GetUserMsgId ("VGUIMenu"))
            botMsgFunction = BotClient_CS_VGUIMenu;
         else if (msg_type == GetUserMsgId ("WeaponList"))
            botMsgFunction = BotClient_CS_WeaponList;
         else if (msg_type == GetUserMsgId ("WeapPickup"))
            botMsgFunction = BotClient_CS_WeaponPickup;

         // has someone fired a bullet ?
         if ((msg_type == GetUserMsgId ("CurWeapon")) || (msg_type == GetUserMsgId ("AmmoX")))
            PlayBulletSoundsForBots (ed); // call this so bots hear bullet sounds
      }

      // else this message is not directed to a player, is it a broadcast message ?
      else if (msg_dest == MSG_ALL)
      {
         // if developer mode is on, notify that a broadcast message is being sent
         if (DeveloperMode () == DEVELOPER_ON)
            ServerConsole_printf ("All clients receive message: %s\n", GetUserMsgName (msg_type));

         botMsgFunction = NULL; // no msg function until known otherwise

         if (msg_type == GetUserMsgId ("DeathMsg"))
            botMsgFunction = BotClient_CS_DeathMsgAll;
         else if (msg_type == GetUserMsgId ("TextMsg"))
            botMsgFunction = BotClient_CS_TextMsgAll;
      }
   }

   (*g_engfuncs.pfnMessageBegin) (msg_dest, msg_type, pOrigin, ed);
}


void pfnMessageEnd (void)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      // was this a message directed to a bot ?
      if (botMsgFunction)
         botMsgFunction = NULL; // clear out the bot message function pointers...

      messagestate = 0; // clear message state (message has finished)
   }

   (*g_engfuncs.pfnMessageEnd) ();
}


void pfnWriteByte (int iValue)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction) ((void *) &iValue, botMsgIndex);

      messagestate++; // increment message state (one packet more has been sent)
   }

   (*g_engfuncs.pfnWriteByte) (iValue);
}


void pfnWriteChar (int iValue)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction) ((void *) &iValue, botMsgIndex);

      messagestate++; // increment message state (one packet more has been sent)
   }

   (*g_engfuncs.pfnWriteChar) (iValue);
}


void pfnWriteShort (int iValue)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction) ((void *) &iValue, botMsgIndex);

      messagestate++; // increment message state (one packet more has been sent)
   }

   (*g_engfuncs.pfnWriteShort) (iValue);
}


void pfnWriteLong (int iValue)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction) ((void *) &iValue, botMsgIndex);

      messagestate++; // increment message state (one packet more has been sent)
   }

   (*g_engfuncs.pfnWriteLong) (iValue);
}


void pfnWriteAngle (float flValue)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction) ((void *) &flValue, botMsgIndex);

      messagestate++; // increment message state (one packet more has been sent)
   }

   (*g_engfuncs.pfnWriteAngle) (flValue);
}


void pfnWriteCoord (float flValue)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction) ((void *) &flValue, botMsgIndex);

      messagestate++; // increment message state (one packet more has been sent)
   }

   (*g_engfuncs.pfnWriteCoord) (flValue);
}


void pfnWriteString (const char *sz)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction) ((void *) sz, botMsgIndex);

      messagestate++; // increment message state (one packet more has been sent)
   }

   (*g_engfuncs.pfnWriteString) (sz);
}


void pfnWriteEntity (int iValue)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction) ((void *) &iValue, botMsgIndex);

      messagestate++; // increment message state (one packet more has been sent)
   }

   (*g_engfuncs.pfnWriteEntity) (iValue);
}


void pfnCVarRegister (cvar_t *pCvar)
{
   (*g_engfuncs.pfnCVarRegister) (pCvar);
}


float pfnCVarGetFloat (const char *szVarName)
{
   return ((*g_engfuncs.pfnCVarGetFloat) (szVarName));
}


const char *pfnCVarGetString (const char *szVarName)
{
   return ((*g_engfuncs.pfnCVarGetString) (szVarName));
}


void pfnCVarSetFloat (const char *szVarName, float flValue)
{
   (*g_engfuncs.pfnCVarSetFloat) (szVarName, flValue);
}


void pfnCVarSetString (const char *szVarName, const char *szValue)
{
   (*g_engfuncs.pfnCVarSetString) (szVarName, szValue);
}


void *pfnPvAllocEntPrivateData (entity_t *pEntity, long cb)
{
   return ((*g_engfuncs.pfnPvAllocEntPrivateData) (pEntity, cb));
}


void *pfnPvEntPrivateData (entity_t *pEntity)
{
   return ((*g_engfuncs.pfnPvEntPrivateData) (pEntity));
}


void pfnFreeEntPrivateData (entity_t *pEntity)
{
   (*g_engfuncs.pfnFreeEntPrivateData) (pEntity);
}


const char *pfnSzFromIndex (int iString)
{
   return ((*g_engfuncs.pfnSzFromIndex) (iString));
}


int pfnAllocString (const char *szValue)
{
   return ((*g_engfuncs.pfnAllocString) (szValue));
}


entvars_t *pfnGetVarsOfEnt (entity_t *pEntity)
{
   return ((*g_engfuncs.pfnGetVarsOfEnt) (pEntity));
}


entity_t *pfnPEntityOfEntOffset (int iEntOffset)
{
   return ((*g_engfuncs.pfnPEntityOfEntOffset) (iEntOffset));
}


int pfnEntOffsetOfPEntity (const entity_t *pEntity)
{
   // this function returns the entity offset memory address of the entity pointed to by pEntity

   return ((*g_engfuncs.pfnEntOffsetOfPEntity) (pEntity));
}


int pfnIndexOfEdict (const entity_t *pEntity)
{
   // this one returns the index in the global entity list of the entity pointed to by pEntity

   return ((*g_engfuncs.pfnIndexOfEdict) (pEntity));
}


entity_t *pfnPEntityOfEntIndex (int iEntIndex)
{
   // this one returns a pointer to the entity whose index in the global entity list is iEntIndex

   return ((*g_engfuncs.pfnPEntityOfEntIndex) (iEntIndex));
}


entity_t *pfnFindEntityByVars (entvars_t *pvars)
{
   // this function returns a pointer to the entity whose entvars_t structure is pvars. Unless it
   // would be for recovering an entity the engine orphaned of its entvars, I don't fucking see
   // the necessity of calling an engine function for this, since any entvars keeps track of its
   // containing entity : basically it's the same as pointing to pvars->pContainingEntity !!

   return ((*g_engfuncs.pfnFindEntityByVars) (pvars));
}


void *pfnGetModelPtr (entity_t *pEntity)
{
   return ((*g_engfuncs.pfnGetModelPtr) (pEntity));
}


int pfnRegUserMsg (const char *pszName, int iSize)
{
   // this function registers a "user message" by the engine side. User messages are network
   // messages the game DLL asks the engine to send to clients. Since many MODs have completely
   // different client features (Counter-Strike has a radar and a timer, for example), network
   // messages just can't be the same for every MOD. Hence here the MOD DLL tells the engine,
   // "Hey, you have to know that I use a network message whose name is pszName and it is iSize
   // packets long". The engine books it, and returns the ID number under which he recorded that
   // custom message. Thus every time the MOD DLL will be wanting to send a message named pszName
   // using pfnMessageBegin(), it will know what message ID number to send, and the engine will
   // know what to do.

   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      int index = -1;
      int msg = (*g_engfuncs.pfnRegUserMsg) (pszName, iSize); // register the message

      // if developer mode is high, log message to debug file
      if (DeveloperMode () == DEVELOPER_ON)
         LogToFile ("Game registering user message \"%s\" to index %d\n", pszName, msg);

      // is this message NOT already registered ?
      for (index = 0; index < usermsgs_count; index++)
         if (strcmp (usermsgs[index].name, pszName) == 0)
            break; // cycle through usermsgs types array and break when a similar record is found

      // now keep track (or update if necessary) this user message in our own array...
      usermsgs[index].name = pszName; // record this message's name
      usermsgs[index].id = msg; // record this message's index
      usermsgs[index].size = iSize; // record this message's size

      // are we certain this message has not been registered in the past ?
      if (index == usermsgs_count)
         usermsgs_count++; // we know now one user message more

      return (msg); // ...and return the message ID number the engine gave us back
   }

   return ((*g_engfuncs.pfnRegUserMsg) (pszName, iSize));
}


void pfnAnimationAutomove (const entity_t *pEntity, float flTime)
{
   (*g_engfuncs.pfnAnimationAutomove) (pEntity, flTime);
}


void pfnGetBonePosition (const entity_t *pEntity, int iBone, float *rgflOrigin, float *rgflAngles)
{
   (*g_engfuncs.pfnGetBonePosition) (pEntity, iBone, rgflOrigin, rgflAngles);
}


unsigned long pfnFunctionFromName (const char *pName)
{
   // this function returns the address of a certain function in the exports array. We don't call
   // the engine function for this, because since our bot DLL doesn't exports ALL the functions
   // the game DLL is exporting (functions beginning by "@@" are missing), we had to build our
   // own exports array by reading the game DLL file to complete the one of our bot DLL. That's
   // the purpose of the LoadSymbols() function, which is called as soon as LoadLibrary() is
   // called in GameDLLInit(). Note this fix is only enabled on Win32 systems, since UNIXes
   // don't need it (only hosting dedicated server and thus never single player games).

   #ifdef _WIN32
   {
      extern WORD *p_Ordinals;
      extern DWORD *p_Functions;
      extern char *p_FunctionNames[1024];
      extern int num_ordinals;
      extern unsigned long base_offset;
      int index;

      // cycle through our exports array and find the entry containing the function name we want
      for (index = 0; index < num_ordinals; index++)
         if (strcmp (pName, p_FunctionNames[index]) == 0)
            return (p_Functions[p_Ordinals[index]] + base_offset); // return the address of that function

      return (0); // couldn't find the function name to return address, return offset 0
   }
   #else
   {
      return ((*g_engfuncs.pfnFunctionFromName) (pName)); // let the engine do this
   }
   #endif
}


const char *pfnNameForFunction (unsigned long function)
{
   // this function returns the name of the function at a certain address in the exports array.
   // We don't call the engine function for this, because since our bot DLL doesn't exports ALL
   // the functions the game DLL is exporting (functions beginning by "@@" are missing), we had
   // to build our own exports array by reading the game DLL file to complete the one of our bot
   // DLL. That's the purpose of the LoadSymbols() function, which is called as soon as
   // LoadLibrary() is called in GameDLLInit(). Note this fix is only enabled on Win32 systems,
   // since UNIXes don't need it (only hosting dedicated server and thus never single player
   // games).

   #ifdef _WIN32
   {
      extern WORD *p_Ordinals;
      extern DWORD *p_Functions;
      extern char *p_FunctionNames[1024];
      extern int num_ordinals;
      extern unsigned long base_offset;
      int index;

      // cycle through our exports array and stop at the function offset index we want
      for (index = 0; index < num_ordinals; index++)
         if ((function - base_offset) == p_Functions[p_Ordinals[index]])
            return (p_FunctionNames[index]); // return the name of that function

      return (NULL); // couldn't find the function address to return name, return void pointer
   }
   #else
   {
      return ((*g_engfuncs.pfnNameForFunction) (function)); // let the engine do this
   }
   #endif
}


void pfnClientPrintf (entity_t *pEntity, PRINT_TYPE ptype, const char *szMsg)
{
   // this function prints the text message string pointed to by szMsg by the client side of
   // the client entity pointed to by pEntity, in a manner depending of ptype (print_console,
   // print_center or print_chat). Be certain never to try to feed a bot with this function,
   // as it will crash your server. Why would you, anyway ? bots have no client DLL as far as
   // we know, right ? But since stupidity rules this world, we do a preventive check :)

   if (IsABot (pEntity))
      return; // reliability check

   (*g_engfuncs.pfnClientPrintf) (pEntity, ptype, szMsg); // ask engine to print szMsg at pEntity's
}


void pfnServerPrint (const char *szMsg)
{
   (*g_engfuncs.pfnServerPrint) (szMsg);
}


int pfnCmd_Argc (void)
{
   // this function returns the number of arguments the current client command string has. Since
   // bots have no client DLL and we may want a bot to execute a client command, we had to
   // implement a g_argv string in the bot DLL for holding the bots' commands, and also keep
   // track of the argument count. Hence this hook not to let the engine ask an unexistent client
   // DLL for a command we are holding here. Of course, real clients commands are still retrieved
   // the normal way, by asking the engine.

   // is this a bot issuing that client command ?
   if (isFakeClientCommand)
      return (fake_arg_count); // if so, then return the argument count we know

   return ((*g_engfuncs.pfnCmd_Argc) ()); // ask the engine how many arguments there are
}


const char *pfnCmd_Args (void)
{
   // this function returns a pointer to the whole current client command string. Since bots
   // have no client DLL and we may want a bot to execute a client command, we had to implement
   // a g_argv string in the bot DLL for holding the bots' commands, and also keep track of the
   // argument count. Hence this hook not to let the engine ask an unexistent client DLL for a
   // command we are holding here. Of course, real clients commands are still retrieved the
   // normal way, by asking the engine.

   // is this a bot issuing that client command ?
   if (isFakeClientCommand)
   {
      // is it a "say" or "say_team" client command ?
      if (strncmp ("say ", g_argv, 4) == 0)
         return (&g_argv[0] + 4); // skip the "say" bot client command (bug in HL engine)
      else if (strncmp ("say_team ", g_argv, 9) == 0)
         return (&g_argv[0] + 9); // skip the "say_team" bot client command (bug in HL engine)

      return (&g_argv[0]); // else return the whole bot client command string we know
   }

   return ((*g_engfuncs.pfnCmd_Args) ()); // ask the client command string to the engine
}


const char *pfnCmd_Argv (int argc)
{
   // this function returns a pointer to a certain argument of the current client command. Since
   // bots have no client DLL and we may want a bot to execute a client command, we had to
   // implement a g_argv string in the bot DLL for holding the bots' commands, and also keep
   // track of the argument count. Hence this hook not to let the engine ask an unexistent client
   // DLL for a command we are holding here. Of course, real clients commands are still retrieved
   // the normal way, by asking the engine.

   // is this a bot issuing that client command ?
   if (isFakeClientCommand)
      return (GetArg (g_argv, argc)); // if so, then return the wanted argument we know

   return ((*g_engfuncs.pfnCmd_Argv) (argc)); // ask the argument number "argc" to the engine
}


void pfnGetAttachment (const entity_t *pEntity, int iAttachment, float *rgflOrigin, float *rgflAngles)
{
   (*g_engfuncs.pfnGetAttachment) (pEntity, iAttachment, rgflOrigin, rgflAngles);
}


void pfnCRC32_Init (CRC32_t *pulCRC)
{
   (*g_engfuncs.pfnCRC32_Init) (pulCRC);
}


void pfnCRC32_ProcessBuffer (CRC32_t *pulCRC, void *p, int len)
{
   (*g_engfuncs.pfnCRC32_ProcessBuffer) (pulCRC, p, len);
}


void pfnCRC32_ProcessByte (CRC32_t *pulCRC, unsigned char ch)
{
   (*g_engfuncs.pfnCRC32_ProcessByte) (pulCRC, ch);
}


CRC32_t pfnCRC32_Final (CRC32_t pulCRC)
{
   return ((*g_engfuncs.pfnCRC32_Final) (pulCRC));
}


long pfnRandomLong (long lLow, long lHigh)
{
   return ((*g_engfuncs.pfnRandomLong) (lLow, lHigh));
}


float pfnRandomFloat (float flLow, float flHigh)
{
   return ((*g_engfuncs.pfnRandomFloat) (flLow, flHigh));
}


void pfnSetView (const entity_t *pClient, const entity_t *pViewent)
{
   (*g_engfuncs.pfnSetView) (pClient, pViewent);
}


float pfnTime (void)
{
   return ((*g_engfuncs.pfnTime) ());
}


void pfnCrosshairAngle (const entity_t *pClient, float pitch, float yaw)
{
   (*g_engfuncs.pfnCrosshairAngle) (pClient, pitch, yaw);
}


byte *pfnLoadFileForMe (char *filename, int *pLength)
{
   return ((*g_engfuncs.pfnLoadFileForMe) (filename, pLength));
}


void pfnFreeFile (void *buffer)
{
   (*g_engfuncs.pfnFreeFile) (buffer);
}


void pfnEndSection (const char *pszSectionName)
{
   (*g_engfuncs.pfnEndSection) (pszSectionName);
}


int pfnCompareFileTime (char *filename1, char *filename2, int *iCompare)
{
   return ((*g_engfuncs.pfnCompareFileTime) (filename1, filename2, iCompare));
}


void pfnGetGameDir (char *szGetGameDir)
{
   (*g_engfuncs.pfnGetGameDir) (szGetGameDir);
}


void pfnCvar_RegisterVariable (cvar_t *variable)
{
   (*g_engfuncs.pfnCvar_RegisterVariable) (variable);
}


void pfnFadeClientVolume (const entity_t *pEntity, int fadePercent, int fadeOutSeconds, int holdTime, int fadeInSeconds)
{
   (*g_engfuncs.pfnFadeClientVolume) (pEntity, fadePercent, fadeOutSeconds, holdTime, fadeInSeconds);
}


void pfnSetClientMaxspeed (const entity_t *pEntity, float fNewMaxspeed)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      // is this message for a bot ?
      if (IsABot (pEntity))
         bots[PlayerIndexOf (pEntity)].BotLegs.f_max_speed = fNewMaxspeed; // update this bot's max speed
   }

   (*g_engfuncs.pfnSetClientMaxspeed) (pEntity, fNewMaxspeed);
}


entity_t *pfnCreateFakeClient (const char *netname)
{
   return ((*g_engfuncs.pfnCreateFakeClient) (netname));
}


void pfnRunPlayerMove (entity_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, byte impulse, byte msec)
{
   (*g_engfuncs.pfnRunPlayerMove) (fakeclient, viewangles, forwardmove, sidemove, upmove, buttons, impulse, msec);
}


int pfnNumberOfEntities (void)
{
   return ((*g_engfuncs.pfnNumberOfEntities) ());
}


char *pfnGetInfoKeyBuffer (entity_t *e)
{
   return ((*g_engfuncs.pfnGetInfoKeyBuffer) (e));
}


char *pfnInfoKeyValue (char *infobuffer, char *key)
{
   return ((*g_engfuncs.pfnInfoKeyValue) (infobuffer, key));
}


void pfnSetKeyValue (char *infobuffer, char *key, char *value)
{
   (*g_engfuncs.pfnSetKeyValue) (infobuffer, key, value);
}


void pfnSetClientKeyValue (int clientIndex, char *infobuffer, char *key, char *value)
{
   (*g_engfuncs.pfnSetClientKeyValue) (clientIndex, infobuffer, key, value);
}


int pfnIsMapValid (char *filename)
{
   return ((*g_engfuncs.pfnIsMapValid) (filename));
}


void pfnStaticDecal (const float *origin, int decalIndex, int entityIndex, int modelIndex)
{
   (*g_engfuncs.pfnStaticDecal) (origin, decalIndex, entityIndex, modelIndex);
}


int pfnPrecacheGeneric (char *s)
{
   return ((*g_engfuncs.pfnPrecacheGeneric) (s));
}


int pfnGetPlayerUserId (entity_t *e)
{
   return ((*g_engfuncs.pfnGetPlayerUserId) (e));
}


void pfnBuildSoundMsg (entity_t *entity, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch, int msg_dest, int msg_type, const float *pOrigin, entity_t *ed)
{
   (*g_engfuncs.pfnBuildSoundMsg) (entity, channel, sample, volume, attenuation, fFlags, pitch, msg_dest, msg_type, pOrigin, ed);
}


int pfnIsDedicatedServer (void)
{
   return ((*g_engfuncs.pfnIsDedicatedServer) ());
}


cvar_t *pfnCVarGetPointer (const char *szVarName)
{
   return ((*g_engfuncs.pfnCVarGetPointer) (szVarName));
}


unsigned int pfnGetPlayerWONId (entity_t *e)
{
   return ((*g_engfuncs.pfnGetPlayerWONId) (e));
}


void pfnInfo_RemoveKey (char *s, const char *key)
{
   (*g_engfuncs.pfnInfo_RemoveKey) (s, key);
}


const char *pfnGetPhysicsKeyValue (const entity_t *pClient, const char *key)
{
   return ((*g_engfuncs.pfnGetPhysicsKeyValue) (pClient, key));
}


void pfnSetPhysicsKeyValue (const entity_t *pClient, const char *key, const char *value)
{
   (*g_engfuncs.pfnSetPhysicsKeyValue) (pClient, key, value);
}


const char *pfnGetPhysicsInfoString (const entity_t *pClient)
{
   return ((*g_engfuncs.pfnGetPhysicsInfoString) (pClient));
}


unsigned short pfnPrecacheEvent (int type, const char *psz)
{
   return ((*g_engfuncs.pfnPrecacheEvent) (type, psz));
}


void pfnPlaybackEvent (int flags, const entity_t *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1,float fparam2, int iparam1, int iparam2, int bparam1, int bparam2)
{
   (*g_engfuncs.pfnPlaybackEvent) (flags, pInvoker, eventindex, delay, origin, angles, fparam1, fparam2, iparam1, iparam2, bparam1, bparam2);
}


unsigned char *pfnSetFatPVS (float *org)
{
   return ((*g_engfuncs.pfnSetFatPVS) (org));
}


unsigned char *pfnSetFatPAS (float *org)
{
   return ((*g_engfuncs.pfnSetFatPAS) (org));
}


int pfnCheckVisibility (const entity_t *entity, unsigned char *pset)
{
   return ((*g_engfuncs.pfnCheckVisibility) (entity, pset));
}


void pfnDeltaSetField (struct delta_s *pFields, const char *fieldname)
{
   (*g_engfuncs.pfnDeltaSetField) (pFields, fieldname);
}


void pfnDeltaUnsetField (struct delta_s *pFields, const char *fieldname)
{
   (*g_engfuncs.pfnDeltaUnsetField) (pFields, fieldname);
}


void pfnDeltaAddEncoder (char *name, void (*conditionalencode) (struct delta_s *pFields, const unsigned char *from, const unsigned char *to))
{
   (*g_engfuncs.pfnDeltaAddEncoder) (name, conditionalencode);
}


int pfnGetCurrentPlayer (void)
{
   return ((*g_engfuncs.pfnGetCurrentPlayer) ());
}


int pfnCanSkipPlayer (const entity_t *player)
{
   return ((*g_engfuncs.pfnCanSkipPlayer) (player));
}


int pfnDeltaFindField (struct delta_s *pFields, const char *fieldname)
{
   return ((*g_engfuncs.pfnDeltaFindField) (pFields, fieldname));
}


void pfnDeltaSetFieldByIndex (struct delta_s *pFields, int fieldNumber)
{
   (*g_engfuncs.pfnDeltaSetFieldByIndex) (pFields, fieldNumber);
}


void pfnDeltaUnsetFieldByIndex (struct delta_s *pFields, int fieldNumber)
{
   (*g_engfuncs.pfnDeltaUnsetFieldByIndex) (pFields, fieldNumber);
}


void pfnSetGroupMask (int mask, int op)
{
   (*g_engfuncs.pfnSetGroupMask) (mask, op);
}


int pfnCreateInstancedBaseline (int classname, struct entity_state_s *baseline)
{
   return ((*g_engfuncs.pfnCreateInstancedBaseline) (classname, baseline));
}


void pfnCvar_DirectSet (struct cvar_s *var, char *value)
{
   (*g_engfuncs.pfnCvar_DirectSet) (var, value);
}


void pfnForceUnmodified (FORCE_TYPE type, float *mins, float *maxs, const char *filename)
{
   (*g_engfuncs.pfnForceUnmodified) (type, mins, maxs, filename);
}


void pfnGetPlayerStats (const entity_t *pClient, int *ping, int *packet_loss)
{
   (*g_engfuncs.pfnGetPlayerStats) (pClient, ping, packet_loss);
}


qboolean pfnVoice_GetClientListening (int iReceiver, int iSender)
{
   return ((*g_engfuncs.pfnVoice_GetClientListening) (iReceiver, iSender));
}


qboolean pfnVoice_SetClientListening (int iReceiver, int iSender, qboolean bListen)
{
   return ((*g_engfuncs.pfnVoice_SetClientListening) (iReceiver, iSender, bListen));
}


void pfnAddServerCommand (char *cmd_name, void function (void))
{
   (*g_engfuncs.pfnAddServerCommand) (cmd_name, function);
}


void pfnEngineFprintf (FILE *pfile, char *szFmt, ...)
{
   va_list argptr;
   static char string[1024];

   va_start (argptr, szFmt);
   vsprintf (string, szFmt, argptr);
   va_end (argptr);

   (*g_engfuncs.pfnEngineFprintf) (pfile, string);
}


void pfnAlertMessage (ALERT_TYPE atype, char *szFmt, ...)
{
   va_list argptr;
   static char string[1024];

   va_start (argptr, szFmt);
   vsprintf (string, szFmt, argptr);
   va_end (argptr);

   (*g_engfuncs.pfnAlertMessage) (atype, string);
}


const char *pfnGetPlayerAuthId (entity_t *e)
{
   // is this player a bot ?
   if (IsABot (e))
      return ("0"); // return 0 as the AuthId number of the bot

   return ((*g_engfuncs.pfnGetPlayerAuthId) (e));
}
