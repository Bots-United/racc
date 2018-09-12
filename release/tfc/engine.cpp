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
// engine.cpp
//

#include "extdll.h"
#include "util.h"
#include "bot_common.h"
#include "bot_specific.h"


extern bool is_multiplayer_game;
extern enginefuncs_t g_engfuncs;
extern bot_t bots[32];
extern int team_allies[4];

void (*botMsgFunction) (void *, int) = NULL;
void (*botMsgEndFunction) (void *, int) = NULL;
int botMsgIndex;

extern bool roundend;

// messages created in RegUserMsg which will be "caught"
int message_VGUI = 0;
int message_ShowMenu = 0;
int message_WeaponList = 0;
int message_CurWeapon = 0;
int message_AmmoX = 0;
int message_SecAmmoVal = 0;
int message_WeapPickup = 0;
int message_AmmoPickup = 0;
int message_ItemPickup = 0;
int message_Health = 0;
int message_Battery = 0;
int message_Damage = 0;
int message_DeathMsg = 0;
int message_SayText = 0;
int message_TextMsg = 0;
int message_ScreenFade = 0;


int pfnPrecacheModel (char* s)
{
   return (*g_engfuncs.pfnPrecacheModel) (s);
}
int pfnPrecacheSound (char* s)
{
   return (*g_engfuncs.pfnPrecacheSound) (s);
}
void pfnSetModel (edict_t *e, const char *m)
{
   (*g_engfuncs.pfnSetModel) (e, m);
}
int pfnModelIndex (const char *m)
{
   return (*g_engfuncs.pfnModelIndex) (m);
}
int pfnModelFrames (int modelIndex)
{
   return (*g_engfuncs.pfnModelFrames) (modelIndex);
}
void pfnSetSize (edict_t *e, const float *rgflMin, const float *rgflMax)
{
   (*g_engfuncs.pfnSetSize) (e, rgflMin, rgflMax);
}
void pfnChangeLevel (char* s1, char* s2)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      // disconnect any bot from the server after time/frag limit...
      for (int index = 0; index < 32; index++)
         if (bots[index].is_active && (bots[index].pEdict != NULL))
         {
            char servercmd[80];
            sprintf (servercmd, "kick \"%s\"\n", STRING (bots[index].pEdict->v.netname));
            SERVER_COMMAND (servercmd); // let the bot disconnect
         }
   }

   (*g_engfuncs.pfnChangeLevel) (s1, s2);
}
void pfnGetSpawnParms (edict_t *ent)
{
   (*g_engfuncs.pfnGetSpawnParms) (ent);
}
void pfnSaveSpawnParms (edict_t *ent)
{
   (*g_engfuncs.pfnSaveSpawnParms) (ent);
}
float pfnVecToYaw (const float *rgflVector)
{
   return (*g_engfuncs.pfnVecToYaw) (rgflVector);
}
void pfnVecToAngles (const float *rgflVectorIn, float *rgflVectorOut)
{
   (*g_engfuncs.pfnVecToAngles) (rgflVectorIn, rgflVectorOut);
}
void pfnMoveToOrigin (edict_t *ent, const float *pflGoal, float dist, int iMoveType)
{
   (*g_engfuncs.pfnMoveToOrigin) (ent, pflGoal, dist, iMoveType);
}
void pfnChangeYaw (edict_t* ent)
{
   (*g_engfuncs.pfnChangeYaw) (ent);
}
void pfnChangePitch (edict_t* ent)
{
   (*g_engfuncs.pfnChangePitch) (ent);
}
edict_t* pfnFindEntityByString (edict_t *pEdictStartSearchAfter, const char *pszField, const char *pszValue)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
      if (strcmp (pszValue, "info_map_parameters") == 0)
         roundend = TRUE; // catch new round message (resetting map message)

   return (*g_engfuncs.pfnFindEntityByString) (pEdictStartSearchAfter, pszField, pszValue);
}
int pfnGetEntityIllum (edict_t* pEnt)
{
   return (*g_engfuncs.pfnGetEntityIllum) (pEnt);
}
edict_t* pfnFindEntityInSphere (edict_t *pEdictStartSearchAfter, const float *org, float rad)
{
   return (*g_engfuncs.pfnFindEntityInSphere) (pEdictStartSearchAfter, org, rad);
}
edict_t* pfnFindClientInPVS (edict_t *pEdict)
{
   return (*g_engfuncs.pfnFindClientInPVS) (pEdict);
}
edict_t* pfnEntitiesInPVS (edict_t *pplayer)
{
   return (*g_engfuncs.pfnEntitiesInPVS) (pplayer);
}
void pfnMakeVectors (const float *rgflVector)
{
   (*g_engfuncs.pfnMakeVectors) (rgflVector);
}
void pfnAngleVectors (const float *rgflVector, float *forward, float *right, float *up)
{
   (*g_engfuncs.pfnAngleVectors) (rgflVector, forward, right, up);
}
edict_t* pfnCreateEntity (void)
{
   edict_t *pent = (*g_engfuncs.pfnCreateEntity) ();
   return pent;
}
void pfnRemoveEntity (edict_t* e)
{
   (*g_engfuncs.pfnRemoveEntity) (e);
}
edict_t* pfnCreateNamedEntity (int className)
{
   edict_t *pent = (*g_engfuncs.pfnCreateNamedEntity) (className);
   return pent;
}
void pfnMakeStatic (edict_t *ent)
{
   (*g_engfuncs.pfnMakeStatic) (ent);
}
int pfnEntIsOnFloor (edict_t *e)
{
   return (*g_engfuncs.pfnEntIsOnFloor) (e);
}
int pfnDropToFloor (edict_t* e)
{
   return (*g_engfuncs.pfnDropToFloor) (e);
}
int pfnWalkMove (edict_t *ent, float yaw, float dist, int iMode)
{
   return (*g_engfuncs.pfnWalkMove) (ent, yaw, dist, iMode);
}
void pfnSetOrigin (edict_t *e, const float *rgflOrigin)
{
   (*g_engfuncs.pfnSetOrigin) (e, rgflOrigin);
}
void pfnEmitSound (edict_t *entity, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      int index;
      Vector vecEnd;

      // is someone yelling for a medic?
      if ((strcmp (sample, "speech/saveme1.wav") == 0) || (strcmp (sample, "speech/saveme2.wav") == 0))
      {
         // cycle all bots
         for (index = 0; index < 32; index++)
         {
            if (!bots[index].is_active || (bots[index].pEdict == NULL)) // skip bad bots
               continue;

            if ((bots[index].pEdict->v.playerclass != TFC_CLASS_MEDIC)
                && (bots[index].pEdict->v.playerclass != TFC_CLASS_ENGINEER))
               continue; // skip non-medic and non-engineer bots

            int player_team = GetTeam (entity);
            int bot_team = GetTeam (bots[index].pEdict);

            // don't heal your enemies...
            if ((bot_team != player_team) && !(team_allies[bot_team] & (1 << player_team)))
               continue;

            vecEnd = entity->v.origin + entity->v.view_ofs;

            // if bot can see player AND bot has no enemy AND player is not too far, go and heal him
            if ((BotGetIdealAimVector (&bots[index], entity) != Vector (0, 0, 0))
                && (bots[index].pBotEnemy == NULL)
                && ((bots[index].pEdict->v.origin - entity->v.origin).Length () < 1000))
            {
               bots[index].pBotEnemy = entity; // go and heal this player
               bots[index].b_is_fearful = FALSE; // for bot to run to player
               bots[index].pBotUser = NULL; // don't follow user anymore
            }
         }
      }
   }

   (*g_engfuncs.pfnEmitSound) (entity, channel, sample, volume, attenuation, fFlags, pitch);
}
void pfnEmitAmbientSound (edict_t *entity, float *pos, const char *samp, float vol, float attenuation, int fFlags, int pitch)
{
   (*g_engfuncs.pfnEmitAmbientSound) (entity, pos, samp, vol, attenuation, fFlags, pitch);
}
void pfnTraceLine (const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr)
{
   (*g_engfuncs.pfnTraceLine) (v1, v2, fNoMonsters, pentToSkip, ptr);
}
void pfnTraceToss (edict_t* pent, edict_t* pentToIgnore, TraceResult *ptr)
{
   (*g_engfuncs.pfnTraceToss) (pent, pentToIgnore, ptr);
}
int pfnTraceMonsterHull (edict_t *pEdict, const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr)
{
   return (*g_engfuncs.pfnTraceMonsterHull) (pEdict, v1, v2, fNoMonsters, pentToSkip, ptr);
}
void pfnTraceHull (const float *v1, const float *v2, int fNoMonsters, int hullNumber, edict_t *pentToSkip, TraceResult *ptr)
{
   (*g_engfuncs.pfnTraceHull) (v1, v2, fNoMonsters, hullNumber, pentToSkip, ptr);
}
void pfnTraceModel (const float *v1, const float *v2, int hullNumber, edict_t *pent, TraceResult *ptr)
{
   (*g_engfuncs.pfnTraceModel) (v1, v2, hullNumber, pent, ptr);
}
const char *pfnTraceTexture (edict_t *pTextureEntity, const float *v1, const float *v2)
{
   return (*g_engfuncs.pfnTraceTexture) (pTextureEntity, v1, v2);
}
void pfnTraceSphere (const float *v1, const float *v2, int fNoMonsters, float radius, edict_t *pentToSkip, TraceResult *ptr)
{
   (*g_engfuncs.pfnTraceSphere) (v1, v2, fNoMonsters, radius, pentToSkip, ptr);
}
void pfnGetAimVector (edict_t* ent, float speed, float *rgflReturn)
{
   (*g_engfuncs.pfnGetAimVector) (ent, speed, rgflReturn);
}
void pfnServerCommand (char* str)
{
   (*g_engfuncs.pfnServerCommand) (str);
}
void pfnServerExecute (void)
{
   (*g_engfuncs.pfnServerExecute) ();
}
void pfnClientCommand (edict_t* pEdict, char* szFmt, ...)
{
   if (!(pEdict->v.flags & FL_FAKECLIENT))
   {
      va_list argptr;
      static char string[1024];

      va_start (argptr, szFmt);
      vsprintf (string, szFmt, argptr);
      va_end (argptr);

      (*g_engfuncs.pfnClientCommand) (pEdict, string);
   }
   return;
}
void pfnParticleEffect (const float *org, const float *dir, float color, float count)
{
   (*g_engfuncs.pfnParticleEffect) (org, dir, color, count);
}
void pfnLightStyle (int style, char* val)
{
   (*g_engfuncs.pfnLightStyle) (style, val);
}
int pfnDecalIndex (const char *name)
{
   return (*g_engfuncs.pfnDecalIndex) (name);
}
int pfnPointContents (const float *rgflVector)
{
   return (*g_engfuncs.pfnPointContents) (rgflVector);
}
void pfnMessageBegin (int msg_dest, int msg_type, const float *pOrigin, edict_t *ed)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      int index = -1;

      if (ed)
      {
         index = UTIL_GetBotIndex (ed);

         // is this message for a bot?
         if (index != -1)
         {
            botMsgFunction = NULL; // no msg function until known otherwise
            botMsgEndFunction = NULL; // no msg end function until known otherwise
            botMsgIndex = index; // index of bot receiving message

            if (msg_type == message_VGUI)
               botMsgFunction = BotClient_TFC_VGUI;
            else if (msg_type == message_WeaponList)
               botMsgFunction = BotClient_TFC_WeaponList;
            else if (msg_type == message_CurWeapon)
               botMsgFunction = BotClient_TFC_CurrentWeapon;
            else if (msg_type == message_AmmoX)
               botMsgFunction = BotClient_TFC_AmmoX;
            else if (msg_type == message_SecAmmoVal)
               botMsgFunction = BotClient_TFC_SecAmmoVal;
            else if (msg_type == message_AmmoPickup)
               botMsgFunction = BotClient_TFC_AmmoPickup;
            else if (msg_type == message_WeapPickup)
               botMsgFunction = BotClient_TFC_WeaponPickup;
            else if (msg_type == message_ItemPickup)
               botMsgFunction = BotClient_TFC_ItemPickup;
            else if (msg_type == message_Health)
               botMsgFunction = BotClient_TFC_Health;
            else if (msg_type == message_Battery)
               botMsgFunction = BotClient_TFC_Battery;
            else if (msg_type == message_Damage)
               botMsgFunction = BotClient_TFC_Damage;
            else if (msg_type == message_ScreenFade)
               botMsgFunction = BotClient_TFC_ScreenFade;
            else if (msg_type == message_SayText)
               botMsgFunction = BotClient_TFC_SayText;
         }

         // has someone fired a bullet ?
         if (msg_type == message_AmmoX)
            UpdateBulletSounds (ed); // call this so bots hear bullet sounds
      }
      else if (msg_dest == MSG_ALL)
      {
         botMsgFunction = NULL;  // no msg function until known otherwise
         botMsgIndex = -1;       // index of bot receiving message (none)

         if (msg_type == message_DeathMsg)
            botMsgFunction = BotClient_TFC_DeathMsg;
      }
   }

   (*g_engfuncs.pfnMessageBegin) (msg_dest, msg_type, pOrigin, ed);
}
void pfnMessageEnd (void)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      if (botMsgEndFunction)
         (*botMsgEndFunction) (NULL, botMsgIndex);  // NULL indicated msg end

      // clear out the bot message function pointers...
      botMsgFunction = NULL;
      botMsgEndFunction = NULL;
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
   }

   (*g_engfuncs.pfnWriteEntity) (iValue);
}
void pfnCVarRegister (cvar_t *pCvar)
{
   (*g_engfuncs.pfnCVarRegister) (pCvar);
}
float pfnCVarGetFloat (const char *szVarName)
{
   return (*g_engfuncs.pfnCVarGetFloat) (szVarName);
}
const char* pfnCVarGetString (const char *szVarName)
{
   return (*g_engfuncs.pfnCVarGetString) (szVarName);
}
void pfnCVarSetFloat (const char *szVarName, float flValue)
{
   (*g_engfuncs.pfnCVarSetFloat) (szVarName, flValue);
}
void pfnCVarSetString (const char *szVarName, const char *szValue)
{
   (*g_engfuncs.pfnCVarSetString) (szVarName, szValue);
}
void* pfnPvAllocEntPrivateData (edict_t *pEdict, long cb)
{
   return (*g_engfuncs.pfnPvAllocEntPrivateData) (pEdict, cb);
}
void* pfnPvEntPrivateData (edict_t *pEdict)
{
   return (*g_engfuncs.pfnPvEntPrivateData) (pEdict);
}
void pfnFreeEntPrivateData (edict_t *pEdict)
{
   (*g_engfuncs.pfnFreeEntPrivateData) (pEdict);
}
const char* pfnSzFromIndex (int iString)
{
   return (*g_engfuncs.pfnSzFromIndex) (iString);
}
int pfnAllocString (const char *szValue)
{
   return (*g_engfuncs.pfnAllocString) (szValue);
}
entvars_t* pfnGetVarsOfEnt (edict_t *pEdict)
{
   return (*g_engfuncs.pfnGetVarsOfEnt) (pEdict);
}
edict_t* pfnPEntityOfEntOffset (int iEntOffset)
{
   return (*g_engfuncs.pfnPEntityOfEntOffset) (iEntOffset);
}
int pfnEntOffsetOfPEntity (const edict_t *pEdict)
{
   return (*g_engfuncs.pfnEntOffsetOfPEntity) (pEdict);
}
int pfnIndexOfEdict (const edict_t *pEdict)
{
   return (*g_engfuncs.pfnIndexOfEdict) (pEdict);
}
edict_t* pfnPEntityOfEntIndex (int iEntIndex)
{
   return (*g_engfuncs.pfnPEntityOfEntIndex) (iEntIndex);
}
edict_t* pfnFindEntityByVars (entvars_t* pvars)
{
   return (*g_engfuncs.pfnFindEntityByVars) (pvars);
}
void* pfnGetModelPtr (edict_t* pEdict)
{
   return (*g_engfuncs.pfnGetModelPtr) (pEdict);
}
int pfnRegUserMsg (const char *pszName, int iSize)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
   {
      int msg = (*g_engfuncs.pfnRegUserMsg) (pszName, iSize);

      if (strcmp (pszName, "VGUIMenu") == 0)
         message_VGUI = msg;
      else if (strcmp (pszName, "WeaponList") == 0)
         message_WeaponList = msg;
      else if (strcmp (pszName, "CurWeapon") == 0)
         message_CurWeapon = msg;
      else if (strcmp (pszName, "AmmoX") == 0)
         message_AmmoX = msg;
      else if (strcmp (pszName, "SecAmmoVal") == 0)
         message_SecAmmoVal = msg;
      else if (strcmp (pszName, "AmmoPickup") == 0)
         message_AmmoPickup = msg;
      else if (strcmp (pszName, "WeapPickup") == 0)
         message_WeapPickup = msg;
      else if (strcmp (pszName, "ItemPickup") == 0)
         message_ItemPickup = msg;
      else if (strcmp (pszName, "Health") == 0)
         message_Health = msg;
      else if (strcmp (pszName, "Battery") == 0)
         message_Battery = msg;
      else if (strcmp (pszName, "Damage") == 0)
         message_Damage = msg;
      else if (strcmp (pszName, "SayText") == 0)
         message_SayText = msg;
      else if (strcmp (pszName, "TextMsg") == 0)
         message_TextMsg = msg;
      else if (strcmp (pszName, "DeathMsg") == 0)
         message_DeathMsg = msg;
      else if (strcmp (pszName, "ScreenFade") == 0)
         message_ScreenFade = msg;

      return msg;
   }

   return (*g_engfuncs.pfnRegUserMsg) (pszName, iSize);
}
void pfnAnimationAutomove (const edict_t* pEdict, float flTime)
{
   (*g_engfuncs.pfnAnimationAutomove) (pEdict, flTime);
}
void pfnGetBonePosition (const edict_t* pEdict, int iBone, float *rgflOrigin, float *rgflAngles)
{
   (*g_engfuncs.pfnGetBonePosition) (pEdict, iBone, rgflOrigin, rgflAngles);
}
unsigned long pfnFunctionFromName (const char *pName)
{
   return FUNCTION_FROM_NAME (pName);
}
const char *pfnNameForFunction (unsigned long function)
{
   return NAME_FOR_FUNCTION (function);
}
void pfnClientPrintf (edict_t* pEdict, PRINT_TYPE ptype, const char *szMsg)
{
   if (!(pEdict->v.flags & FL_FAKECLIENT))
      (*g_engfuncs.pfnClientPrintf) (pEdict, ptype, szMsg);
}
void pfnServerPrint (const char *szMsg)
{
   (*g_engfuncs.pfnServerPrint) (szMsg);
}
void pfnGetAttachment (const edict_t *pEdict, int iAttachment, float *rgflOrigin, float *rgflAngles)
{
   (*g_engfuncs.pfnGetAttachment) (pEdict, iAttachment, rgflOrigin, rgflAngles);
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
   return (*g_engfuncs.pfnCRC32_Final) (pulCRC);
}
long pfnRandomLong (long lLow, long lHigh)
{
   return (*g_engfuncs.pfnRandomLong) (lLow, lHigh);
}
float pfnRandomFloat (float flLow, float flHigh)
{
   return (*g_engfuncs.pfnRandomFloat) (flLow, flHigh);
}
void pfnSetView (const edict_t *pClient, const edict_t *pViewent)
{
   (*g_engfuncs.pfnSetView) (pClient, pViewent);
}
float pfnTime (void)
{
   return (*g_engfuncs.pfnTime) ();
}
void pfnCrosshairAngle (const edict_t *pClient, float pitch, float yaw)
{
   (*g_engfuncs.pfnCrosshairAngle) (pClient, pitch, yaw);
}
byte *pfnLoadFileForMe (char *filename, int *pLength)
{
   return (*g_engfuncs.pfnLoadFileForMe) (filename, pLength);
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
   return (*g_engfuncs.pfnCompareFileTime) (filename1, filename2, iCompare);
}
void pfnGetGameDir (char *szGetGameDir)
{
   (*g_engfuncs.pfnGetGameDir) (szGetGameDir);
}
void pfnCvar_RegisterVariable (cvar_t *variable)
{
   (*g_engfuncs.pfnCvar_RegisterVariable) (variable);
}
void pfnFadeClientVolume (const edict_t *pEdict, int fadePercent, int fadeOutSeconds, int holdTime, int fadeInSeconds)
{
   (*g_engfuncs.pfnFadeClientVolume) (pEdict, fadePercent, fadeOutSeconds, holdTime, fadeInSeconds);
}
void pfnSetClientMaxspeed (const edict_t *pEdict, float fNewMaxspeed)
{
   int index = UTIL_GetBotIndex (pEdict);

   if (index !=-1)
      bots[index].BotMove.f_max_speed = fNewMaxspeed;

   (*g_engfuncs.pfnSetClientMaxspeed) (pEdict, fNewMaxspeed);
}
edict_t * pfnCreateFakeClient (const char *netname)
{
   return (*g_engfuncs.pfnCreateFakeClient) (netname);
}
void pfnRunPlayerMove (edict_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, byte impulse, byte msec)
{
   (*g_engfuncs.pfnRunPlayerMove) (fakeclient, viewangles, forwardmove, sidemove, upmove, buttons, impulse, msec);
}
int pfnNumberOfEntities (void)
{
   return (*g_engfuncs.pfnNumberOfEntities) ();
}
char* pfnGetInfoKeyBuffer (edict_t *e)
{
   return (*g_engfuncs.pfnGetInfoKeyBuffer) (e);
}
char* pfnInfoKeyValue (char *infobuffer, char *key)
{
   return (*g_engfuncs.pfnInfoKeyValue) (infobuffer, key);
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
   return (*g_engfuncs.pfnIsMapValid) (filename);
}
void pfnStaticDecal (const float *origin, int decalIndex, int entityIndex, int modelIndex)
{
   (*g_engfuncs.pfnStaticDecal) (origin, decalIndex, entityIndex, modelIndex);
}
int pfnPrecacheGeneric (char* s)
{
   return (*g_engfuncs.pfnPrecacheGeneric) (s);
}
int pfnGetPlayerUserId (edict_t *e)
{
   // only process bots if we are in multiplayer mode
   if (is_multiplayer_game)
      if (UTIL_GetBotIndex (e) != -1)
         return 0; // don't return a valid index if edict is a bot (so bot won't get kicked)

   return (*g_engfuncs.pfnGetPlayerUserId) (e);
}
void pfnBuildSoundMsg (edict_t *entity, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch, int msg_dest, int msg_type, const float *pOrigin, edict_t *ed)
{
   (*g_engfuncs.pfnBuildSoundMsg) (entity, channel, sample, volume, attenuation, fFlags, pitch, msg_dest, msg_type, pOrigin, ed);
}
int pfnIsDedicatedServer (void)
{
   return (*g_engfuncs.pfnIsDedicatedServer) ();
}
cvar_t* pfnCVarGetPointer (const char *szVarName)
{
   return (*g_engfuncs.pfnCVarGetPointer) (szVarName);
}
unsigned int pfnGetPlayerWONId (edict_t *e)
{
   return (*g_engfuncs.pfnGetPlayerWONId) (e);
}
void pfnInfo_RemoveKey (char *s, const char *key)
{
   (*g_engfuncs.pfnInfo_RemoveKey) (s, key);
}
const char *pfnGetPhysicsKeyValue (const edict_t *pClient, const char *key)
{
   return (*g_engfuncs.pfnGetPhysicsKeyValue) (pClient, key);
}
void pfnSetPhysicsKeyValue (const edict_t *pClient, const char *key, const char *value)
{
   (*g_engfuncs.pfnSetPhysicsKeyValue) (pClient, key, value);
}
const char *pfnGetPhysicsInfoString (const edict_t *pClient)
{
   return (*g_engfuncs.pfnGetPhysicsInfoString) (pClient);
}
unsigned short pfnPrecacheEvent (int type, const char *psz)
{
   return (*g_engfuncs.pfnPrecacheEvent) (type, psz);
}
void pfnPlaybackEvent (int flags, const edict_t *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1,float fparam2, int iparam1, int iparam2, int bparam1, int bparam2)
{
   (*g_engfuncs.pfnPlaybackEvent) (flags, pInvoker, eventindex, delay, origin, angles, fparam1, fparam2, iparam1, iparam2, bparam1, bparam2);
}
unsigned char *pfnSetFatPVS (float *org)
{
   return (*g_engfuncs.pfnSetFatPVS) (org);
}
unsigned char *pfnSetFatPAS (float *org)
{
   return (*g_engfuncs.pfnSetFatPAS) (org);
}
int pfnCheckVisibility (const edict_t *entity, unsigned char *pset)
{
   return (*g_engfuncs.pfnCheckVisibility) (entity, pset);
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
   return (*g_engfuncs.pfnGetCurrentPlayer) ();
}
int pfnCanSkipPlayer (const edict_t *player)
{
   return (*g_engfuncs.pfnCanSkipPlayer) (player);
}
int pfnDeltaFindField (struct delta_s *pFields, const char *fieldname)
{
   return (*g_engfuncs.pfnDeltaFindField) (pFields, fieldname);
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
   return (*g_engfuncs.pfnCreateInstancedBaseline) (classname, baseline);
}
void pfnCvar_DirectSet (struct cvar_s *var, char *value)
{
   (*g_engfuncs.pfnCvar_DirectSet) (var, value);
}
void pfnForceUnmodified (FORCE_TYPE type, float *mins, float *maxs, const char *filename)
{
   (*g_engfuncs.pfnForceUnmodified) (type, mins, maxs, filename);
}
void pfnGetPlayerStats (const edict_t *pClient, int *ping, int *packet_loss)
{
   (*g_engfuncs.pfnGetPlayerStats) (pClient, ping, packet_loss);
}
qboolean pfnVoice_GetClientListening (int iReceiver, int iSender)
{
   return (*g_engfuncs.pfnVoice_GetClientListening) (iReceiver, iSender);
}
qboolean pfnVoice_SetClientListening (int iReceiver, int iSender, qboolean bListen)
{
   return (*g_engfuncs.pfnVoice_SetClientListening) (iReceiver, iSender, bListen);
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
const char *pfnGetPlayerAuthId (edict_t *e)
{
   return (*g_engfuncs.pfnGetPlayerAuthId) (e);
}
