// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// util.cpp
//

#include "racc.h"


// failsafe global used in FindWeaponBy...()
weapon_t unknown_weapon;


void InitWeapons (void)
{
   // this function is in charge of opening the weapons.cfg file in the knowledge/MOD directory,
   // and filling the gunshot sounds database accordingly. Each type of weapon gets attributed 4
   // different sounds, depending on the attack used on them, and on the special mode the weapon
   // is in (e.g, silenced or not). Such a task should be performed once and only once,
   // preferably at GameDLLInit(), since gunshot sounds aren't likely to change between each map.

   FILE *fp;
   char filename[256];
   char line_buffer[256];
   char key[256], value[256];
   int length, index;
   bool section_open;

   // had we already allocated memory space for weapons ?
   if (weapons != NULL)
      free (weapons); // if so, free it
   weapons = NULL;
   weapon_count = 0; // and reset the weapon count to zero

   // mallocate the minimal space for languages
   weapons = (weapon_t *) malloc (sizeof (weapon_t));
   if (weapons == NULL)
      TerminateOnError ("InitWeapons(): malloc() failure for weapons on %d bytes (out of memory ?)\n", sizeof (weapon_t));

   // open the "weapons.cfg" file in the knowledge directory, in the MOD's folder
   sprintf (filename, "%s/knowledge/%s/weapons.cfg", GameConfig.racc_basedir, GameConfig.mod_name);
   fp = fopen (filename, "r");
   if (fp == NULL)
   {
      ServerConsole_printf ("RACC: Unable to build weapons database (weapons.cfg file not found)\n");
      return; // if the file doesn't exist, then the bot won't know any gunshot sound at all
   }

   section_open = FALSE; // no section has been open yet in the config file

   // for each line in the file...
   while (fgets (line_buffer, 256, fp) != NULL)
   {
      length = (int) strlen (line_buffer); // get length of line
      if ((length > 0) && (line_buffer[length - 1] == '\n'))
         length--; // remove any final '\n'
      line_buffer[length] = 0; // terminate the string

      if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
          || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
         continue; // ignore line if void or commented

      // is it the start of a new section ?
      if (line_buffer[0] == '[')
      {
         index = 1; // let's check for a closing brace to validate this section
         while ((index < length) && (line_buffer[index] != ']'))
            index++; // reach end of field and see if the section is valid
         if (index == length)
            continue; // if end of line reached but no closing bracket found, section is invalid

         // we found the start of a valid section, so now if another section was already open
         // and being read, we need to close it and advance to the next slot

         if (section_open)
            weapon_count++; // skip to next slot, we know now one weapon more

         line_buffer[index] = 0; // terminate the string at the closing brace

         section_open = TRUE; // declare that a section is being read

         // we have another weapon line, must allocate some space more
         weapons = (weapon_t *) realloc (weapons, (weapon_count + 1) * sizeof (weapon_t));
         if (weapons == NULL)
            TerminateOnError ("InitWeapons(): realloc() failure for weapons on %d bytes (out of memory ?)\n", (weapon_count + 1) * sizeof (weapon_t));

         strcpy (weapons[weapon_count].classname, &line_buffer[1]); // read weapon name from section
      }

      if (!section_open)
         continue; // if no section is open yet, skip that line

      // this line looks like a valid data line, read what sort of key it is and store the data
      strcpy (key, GetConfigKey (line_buffer));
      strcpy (value, GetConfigValue (line_buffer));

      // generic data
      if (strcmp (key, "model") == 0)
         strcpy (weapons[weapon_count].model, value);
      else if (strcmp (key, "id") == 0)
         weapons[weapon_count].id = atoi (value);
      else if (strcmp (key, "weight") == 0)
         weapons[weapon_count].weight = atof (value);
      else if (strcmp (key, "class") == 0)
      {
         if (strcmp ("primary", value) == 0)
            weapons[weapon_count].weapon_class = WEAPON_CLASS_PRIMARY;
         else if (strcmp ("secondary", value) == 0)
            weapons[weapon_count].weapon_class = WEAPON_CLASS_SECONDARY;
         else if (strcmp ("grenade", value) == 0)
            weapons[weapon_count].weapon_class = WEAPON_CLASS_GRENADE;
      }
      else if (strcmp (key, "price") == 0)
         weapons[weapon_count].price = atof (value);
      else if (strcmp (key, "buy_command") == 0)
         strcpy (weapons[weapon_count].buy_command, value);

      // primary rail
      else if (strcmp (key, "primary.properties") == 0)
      {
         if (strstr (value, "disabler") != NULL)
            weapons[weapon_count].primary.properties |= WEAPONRAIL_PROPERTY_DISABLER;
         if (strstr (value, "waterproof") != NULL)
            weapons[weapon_count].primary.properties |= WEAPONRAIL_PROPERTY_WATERPROOF;
         if (strstr (value, "lightdamage") != NULL)
            weapons[weapon_count].primary.properties |= WEAPONRAIL_PROPERTY_LIGHTDAMAGE;
         if (strstr (value, "mediumdamage") != NULL)
            weapons[weapon_count].primary.properties |= WEAPONRAIL_PROPERTY_MEDIUMDAMAGE;
         if (strstr (value, "heavydamage") != NULL)
            weapons[weapon_count].primary.properties |= WEAPONRAIL_PROPERTY_HEAVYDAMAGE;
         if (strstr (value, "radiuseffect") != NULL)
            weapons[weapon_count].primary.properties |= WEAPONRAIL_PROPERTY_RADIUSEFFECT;
         if (strstr (value, "automatic") != NULL)
            weapons[weapon_count].primary.properties |= WEAPONRAIL_PROPERTY_AUTOMATIC;
         if (strstr (value, "buckshot") != NULL)
            weapons[weapon_count].primary.properties |= WEAPONRAIL_PROPERTY_BUCKSHOT;
         if (strstr (value, "scoped") != NULL)
            weapons[weapon_count].primary.properties |= WEAPONRAIL_PROPERTY_SCOPED;
         if (strstr (value, "sniper") != NULL)
            weapons[weapon_count].primary.properties |= WEAPONRAIL_PROPERTY_SNIPER;
         if (strstr (value, "silenced") != NULL)
            weapons[weapon_count].primary.properties |= WEAPONRAIL_PROPERTY_SILENCED;
         if (strstr (value, "missile") != NULL)
            weapons[weapon_count].primary.properties |= WEAPONRAIL_PROPERTY_MISSILE;
         if (strstr (value, "homing") != NULL)
            weapons[weapon_count].primary.properties |= WEAPONRAIL_PROPERTY_HOMING;
         if (strstr (value, "toss") != NULL)
            weapons[weapon_count].primary.properties |= WEAPONRAIL_PROPERTY_TOSS;
         if (strstr (value, "place") != NULL)
            weapons[weapon_count].primary.properties |= WEAPONRAIL_PROPERTY_PLACE;
      }
      else if (strcmp (key, "primary.range") == 0)
      {
         if (strcmp (key, "melee") == 0)
            weapons[weapon_count].primary.range = WEAPONRAIL_RANGE_MELEE;
         else if (strcmp (key, "close") == 0)
            weapons[weapon_count].primary.range = WEAPONRAIL_RANGE_CLOSE;
         else if (strcmp (key, "medium") == 0)
            weapons[weapon_count].primary.range = WEAPONRAIL_RANGE_MEDIUM;
         else if (strcmp (key, "far") == 0)
            weapons[weapon_count].primary.range = WEAPONRAIL_RANGE_FAR;
      }
      else if (strcmp (key, "primary.type_of_ammo") == 0)
         weapons[weapon_count].primary.type_of_ammo = atoi (value);
      else if (strcmp (key, "primary.min_ammo") == 0)
         weapons[weapon_count].primary.min_ammo = atoi (value);
      else if (strcmp (key, "primary.max_ammo") == 0)
         weapons[weapon_count].primary.max_ammo = atoi (value);
      else if (strcmp (key, "primary.charge_delay") == 0)
         weapons[weapon_count].primary.charge_delay = atof (value);
      else if (strcmp (key, "primary.sound1") == 0)
         strcpy (weapons[weapon_count].primary.sound1, value);
      else if (strcmp (key, "primary.sound2") == 0)
         strcpy (weapons[weapon_count].primary.sound2, value);
      else if (strcmp (key, "primary.min_delay") == 0)
      {
         weapons[weapon_count].primary.min_delay[0] = atof (GetField (value, 0));
         weapons[weapon_count].primary.min_delay[1] = atof (GetField (value, 1));
         weapons[weapon_count].primary.min_delay[2] = atof (GetField (value, 2));
         weapons[weapon_count].primary.min_delay[3] = atof (GetField (value, 3));
         weapons[weapon_count].primary.min_delay[4] = atof (GetField (value, 4));
      }
      else if (strcmp (key, "primary.max_delay") == 0)
      {
         weapons[weapon_count].primary.max_delay[0] = atof (GetField (value, 0));
         weapons[weapon_count].primary.max_delay[1] = atof (GetField (value, 1));
         weapons[weapon_count].primary.max_delay[2] = atof (GetField (value, 2));
         weapons[weapon_count].primary.max_delay[3] = atof (GetField (value, 3));
         weapons[weapon_count].primary.max_delay[4] = atof (GetField (value, 4));
      }

      // secondary rail
      else if (strcmp (key, "secondary.properties") == 0)
      {
         if (strstr (value, "disabler") != NULL)
            weapons[weapon_count].secondary.properties |= WEAPONRAIL_PROPERTY_DISABLER;
         if (strstr (value, "waterproof") != NULL)
            weapons[weapon_count].secondary.properties |= WEAPONRAIL_PROPERTY_WATERPROOF;
         if (strstr (value, "lightdamage") != NULL)
            weapons[weapon_count].secondary.properties |= WEAPONRAIL_PROPERTY_LIGHTDAMAGE;
         if (strstr (value, "mediumdamage") != NULL)
            weapons[weapon_count].secondary.properties |= WEAPONRAIL_PROPERTY_MEDIUMDAMAGE;
         if (strstr (value, "heavydamage") != NULL)
            weapons[weapon_count].secondary.properties |= WEAPONRAIL_PROPERTY_HEAVYDAMAGE;
         if (strstr (value, "radiuseffect") != NULL)
            weapons[weapon_count].secondary.properties |= WEAPONRAIL_PROPERTY_RADIUSEFFECT;
         if (strstr (value, "automatic") != NULL)
            weapons[weapon_count].secondary.properties |= WEAPONRAIL_PROPERTY_AUTOMATIC;
         if (strstr (value, "buckshot") != NULL)
            weapons[weapon_count].secondary.properties |= WEAPONRAIL_PROPERTY_BUCKSHOT;
         if (strstr (value, "scoped") != NULL)
            weapons[weapon_count].secondary.properties |= WEAPONRAIL_PROPERTY_SCOPED;
         if (strstr (value, "sniper") != NULL)
            weapons[weapon_count].secondary.properties |= WEAPONRAIL_PROPERTY_SNIPER;
         if (strstr (value, "silenced") != NULL)
            weapons[weapon_count].secondary.properties |= WEAPONRAIL_PROPERTY_SILENCED;
         if (strstr (value, "missile") != NULL)
            weapons[weapon_count].secondary.properties |= WEAPONRAIL_PROPERTY_MISSILE;
         if (strstr (value, "homing") != NULL)
            weapons[weapon_count].secondary.properties |= WEAPONRAIL_PROPERTY_HOMING;
         if (strstr (value, "toss") != NULL)
            weapons[weapon_count].secondary.properties |= WEAPONRAIL_PROPERTY_TOSS;
         if (strstr (value, "place") != NULL)
            weapons[weapon_count].secondary.properties |= WEAPONRAIL_PROPERTY_PLACE;
      }
      else if (strcmp (key, "secondary.range") == 0)
      {
         if (strcmp (key, "melee") == 0)
            weapons[weapon_count].secondary.range = WEAPONRAIL_RANGE_MELEE;
         else if (strcmp (key, "close") == 0)
            weapons[weapon_count].secondary.range = WEAPONRAIL_RANGE_CLOSE;
         else if (strcmp (key, "medium") == 0)
            weapons[weapon_count].secondary.range = WEAPONRAIL_RANGE_MEDIUM;
         else if (strcmp (key, "far") == 0)
            weapons[weapon_count].secondary.range = WEAPONRAIL_RANGE_FAR;
      }
      else if (strcmp (key, "secondary.type_of_ammo") == 0)
         weapons[weapon_count].secondary.type_of_ammo = atoi (value);
      else if (strcmp (key, "secondary.min_ammo") == 0)
         weapons[weapon_count].secondary.min_ammo = atoi (value);
      else if (strcmp (key, "secondary.max_ammo") == 0)
         weapons[weapon_count].secondary.max_ammo = atoi (value);
      else if (strcmp (key, "secondary.charge_delay") == 0)
         weapons[weapon_count].secondary.charge_delay = atof (value);
      else if (strcmp (key, "secondary.sound1") == 0)
         strcpy (weapons[weapon_count].secondary.sound1, value);
      else if (strcmp (key, "secondary.sound2") == 0)
         strcpy (weapons[weapon_count].secondary.sound2, value);
      else if (strcmp (key, "secondary.min_delay") == 0)
      {
         weapons[weapon_count].secondary.min_delay[0] = atof (GetField (value, 0));
         weapons[weapon_count].secondary.min_delay[1] = atof (GetField (value, 1));
         weapons[weapon_count].secondary.min_delay[2] = atof (GetField (value, 2));
         weapons[weapon_count].secondary.min_delay[3] = atof (GetField (value, 3));
         weapons[weapon_count].secondary.min_delay[4] = atof (GetField (value, 4));
      }
      else if (strcmp (key, "secondary.max_delay") == 0)
      {
         weapons[weapon_count].secondary.max_delay[0] = atof (GetField (value, 0));
         weapons[weapon_count].secondary.max_delay[1] = atof (GetField (value, 1));
         weapons[weapon_count].secondary.max_delay[2] = atof (GetField (value, 2));
         weapons[weapon_count].secondary.max_delay[3] = atof (GetField (value, 3));
         weapons[weapon_count].secondary.max_delay[4] = atof (GetField (value, 4));
      }
   }

   // end of file has been reached

   // if a section was being read...
   if (section_open)
   {
      section_open = FALSE; // close it, no section open anymore
      weapon_count++; // we have recorded the last section, that makes one weapon more
   }

   fclose (fp); // finished parsing the file, close it

   // print out how many weapons we read
   ServerConsole_printf ("RACC: Bot weapon specs database identified %d weapons\n", weapon_count);
   return;
}


weapon_t *FindWeaponByName (const char *weapon_name)
{
   // given a weapon name, this function finds the actual weapon entry in the global weapons
   // database, so that we get access to its parameters (rails, usage, type of ammo, sound, etc.)
   // If weapon name is not found in the database, return an empty static structure.

   register int index;

   if (weapon_name == NULL)
      return (&unknown_weapon); // reliability check

   // for each weapon in the global array, compare its name with that one
   for (index = 0; index < weapon_count; index++)
      if (strcmp (weapon_name, weapons[index].classname) == 0)
         return (&weapons[index]); // when found, return a pointer to the weapon entry

   // damnit, weapon not found !
   ServerConsole_printf ("RACC: FindWeaponByName(): weapon \"%s\" not found in database\n", weapon_name);

   return (&unknown_weapon); // return empty weapon
}


weapon_t *FindWeaponByModel (const char *weapon_model)
{
   // given a weapon model, this function finds the actual weapon entry in the global weapons
   // database, so that we get access to its parameters (rails, usage, type of ammo, sound, etc.)
   // If weapon name is not found in the database, return an empty static structure.

   static bool unknown_weapon_initialized = FALSE;
   register int index;

   // if not done yet, initialize the unknown weapon dummy variable
   if (!unknown_weapon_initialized)
   {
      memset (&unknown_weapon, 0, sizeof (unknown_weapon)); // reset the whole variable to zero
      unknown_weapon_initialized = TRUE; // and remember not to do it again
   }

   if (weapon_model == NULL)
      return (&unknown_weapon); // reliability check

   // for each weapon in the global array, check if the model matchs (skip "models/p_" prefix)
   for (index = 0; index < weapon_count; index++)
      if ((weapons[index].model[0] != 0) && (stricmp (weapon_model + 9, weapons[index].model + 9) == 0))
         return (&weapons[index]); // when found, return a pointer to the weapon entry

   // damnit, weapon not found !
   ServerConsole_printf ("RACC: FindWeaponByModel(): weapon model \"%s\" not found in database\n", weapon_model);

   return (&unknown_weapon); // return empty weapon
}


weapon_t *FindWeaponById (const int weapon_id)
{
   // given a weapon id, this function finds the actual weapon entry in the global weapons
   // database, so that we get access to its parameters (rails, usage, type of ammo, sound, etc.)
   // If weapon id is not found in the database, return an empty static structure.

   static bool unknown_weapon_initialized = FALSE;
   register int index;

   // if not done yet, initialize the unknown weapon dummy variable
   if (!unknown_weapon_initialized)
   {
      memset (&unknown_weapon, 0, sizeof (unknown_weapon)); // reset the whole variable to zero
      unknown_weapon_initialized = TRUE; // and remember not to do it again
   }

   if (weapon_id < 0)
      return (&unknown_weapon); // reliability check

   // for each weapon in the global array, check if the ids match
   for (index = 0; index < weapon_count; index++)
      if (weapons[index].id == weapon_id)
         return (&weapons[index]); // when found, return a pointer to the weapon entry

   // damnit, weapon not found !
   ServerConsole_printf ("RACC: FindWeaponById(): weapon id \"%d\" not found in database\n", weapon_id);

   return (&unknown_weapon); // return empty weapon
}


int WeaponIndexOf (weapon_t *weapon)
{
   // this function converts a weapon pointer into its corresponding index in the global weapons
   // walkfaces database. Local variables have been declared static to speedup recurrent calls
   // of this function.

   static bool unknown_weapon_initialized = FALSE;
   register int index;

   // if not done yet, initialize the unknown weapon dummy variable
   if (!unknown_weapon_initialized)
   {
      memset (&unknown_weapon, 0, sizeof (unknown_weapon)); // reset the whole variable to zero
      unknown_weapon_initialized = TRUE; // and remember not to do it again
   }

   if (weapon == NULL)
      TerminateOnError ("WeaponIndexOf(): function called with NULL weapon\n");

   index = -1; // first set index to a bad value, for later error checking
   index = ((unsigned long) weapon - (unsigned long) weapons) / sizeof (weapon_t);

   // check for the index validity (it must ALWAYS be valid, so bomb out on error)
   if ((index < 0) || (index > weapon_count - 1))
      TerminateOnError ("WeaponIndexOf(): bad weapon array index %d (range 0-%d)\n", index, weapon_count - 1);

   return (index); // looks like we found a valid index, so return it
}


bool PlayerHasWeaponOfClass (player_t *pPlayer, char weapon_class)
{
   // this function returns TRUE if the player carries a weapon from the class specified by
   // weapon_class (WEAPON_CLASS_PRIMARY, SECONDARY or GRENADE) - not necessarily holding it in
   // hand, it suffices he owns this weapon in his inventory), FALSE otherwise. To do so we
   // cycle through each weapon in our weapons database and stop by those which belong to the
   // right class, and check if the player owns one of them.

   int weapon_index;

   // loop through all weapons in game...
   for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
   {
      // is this weapon from the right class AND does the player owns this weapon ?
      if ((weapons[weapon_index].weapon_class == weapon_class)
          && (pPlayer->pEntity->v.weapons & (1 << weapons[weapon_index].id)))
         return (TRUE); // if so, the player is carrying a weapon of that class
   }

   return (FALSE); // if we get here, that's the player doesn't own any weapon of that class
}


bool PlayerHoldsWeaponOfClass (player_t *pPlayer, char weapon_class)
{
   // this function returns TRUE if the weapon the player is currently holding in his hands
   // belongs to the class described by weapon_class (WEAPON_CLASS_PRIMARY, SECONDARY or GRENADE),
   // FALSE otherwise. We use the weapon_class field of the weapon hardware, as defined in the
   // weapons.cfg file, after locating the weapon in the database according to its model.

   weapon_t *player_weapon;

   // find the weapon our player is holding in his hands
   if (pPlayer->is_racc_bot)
      player_weapon = pPlayer->Bot.current_weapon->hardware;
   else
      player_weapon = FindWeaponByModel (STRING (pPlayer->pEntity->v.weaponmodel));

   if (player_weapon == &unknown_weapon)
      return (FALSE); // cancel if weapon not found

   return (player_weapon->weapon_class == weapon_class); // now decide and return
}


bool ItemIsWeaponOfClass (edict_t *pItem, char weapon_class)
{
   // this function returns TRUE if the entity pointed to by pItem is a weapon which belongs to
   // the class specified by weapon_class (WEAPON_CLASS_PRIMARY, SECONDARY or GRENADE), and FALSE
   // otherwise. To do so we cycle through each weapon in our weapons database and stop by those
   // which belongs to the wanted class, and check if the models match for both.

   int weapon_index;
   const char *item_model;

   // get the string in the string table and skip the 'models/w_' prefix...
   item_model = STRING (pItem->v.model) + 9;

   // loop through all weapons in game...
   for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
   {
      // is this weapon from the right class AND do the models match ?
      if ((weapons[weapon_index].weapon_class == weapon_class)
          && (strcmp (item_model, weapons[weapon_index].model + 9) == 0))
         return (TRUE); // if so, item is indeed a weapon of class weapon_class
   }

   return (FALSE); // if we get here, that's pItem is not a weapon from the weapon_class class.
}


bool BotSelectWeaponOfClass (player_t *pPlayer, char weapon_class)
{
   // this function makes the bot select a weapon in its inventory which belongs to the class
   // described by the weapon_class parameter (WEAPON_CLASS_PRIMARY, SECONDARY or GRENADE). If
   // a weapon of the right class is found, the function makes the bot select it and returns
   // TRUE ; if no weapon of this class is found in the bot's inventory, it returns FALSE.

   int weapon_index;

   // loop through all weapons in game...
   for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
   {
      // is this weapon from the right class AND does the bot own this weapon
      // AND does the bot have enough ammo to use with this weapon ?
      if ((weapons[weapon_index].weapon_class == weapon_class)
          && (pPlayer->pEntity->v.weapons & (1 << weapons[weapon_index].id))
          && (pPlayer->Bot.bot_weapons[weapon_index].clip_ammo + *pPlayer->Bot.bot_weapons[weapon_index].primary_ammo > weapons[weapon_index].primary.min_ammo))
      {
         FakeClientCommand (pPlayer->pEntity, weapons[weapon_index].classname); // select it
         return (TRUE); // bot found a weapon to discard, so stop looping
      }
   }

   return (FALSE); // bot found no weapon to select in the requested class, return FALSE
}


bool PlayerHasWeaponOfType (player_t *pPlayer, short weaponrail_property)
{
   // this function returns TRUE if the player carries a weapon which has the property specified
   // by weaponrail_property (one of WEAPONRAIL_PROPERTY_xxx) - not necessarily holding it in
   // hand, it suffices he owns this weapon in his inventory), FALSE otherwise. To do so we
   // cycle through each weapon in our weapons database and stop by those that feature one rail
   // with the specified property, and check if the player owns one of them.

   int weapon_index;

   // loop through all weapons in game...
   for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
   {
      // does this weapon feature the wanted property AND does the player owns this weapon ?
      if (((weapons[weapon_index].primary.properties & weaponrail_property)
           || (weapons[weapon_index].secondary.properties & weaponrail_property))
          && (pPlayer->pEntity->v.weapons & (1 << weapons[weapon_index].id)))
         return (TRUE); // if so, the player is carrying a weapon featuring that property
   }

   return (FALSE); // if we get here, that's the player doesn't own any weapon featuring it
}


bool PlayerHoldsWeaponOfType (player_t *pPlayer, short weaponrail_property)
{
   // this function returns TRUE if the weapon the player is currently holding in his hands
   // features the property described by weaponrail_property (one of WEAPONRAIL_PROPERTY_xxx),
   // FALSE otherwise. We use the properties field of each rail of the weapon hardware, as
   // defined in the weapons.cfg file, after locating the weapon in the database according to
   // its model.

   weapon_t *player_weapon;

   // find the weapon our player is holding in his hands
   if (pPlayer->is_racc_bot)
      player_weapon = pPlayer->Bot.current_weapon->hardware;
   else
      player_weapon = FindWeaponByModel (STRING (pPlayer->pEntity->v.weaponmodel));

   if (player_weapon == &unknown_weapon)
      return (FALSE); // cancel if weapon not found

   // now decide if this weapon features the wanted property in one of its two rails and return
   return ((player_weapon->primary.properties & weaponrail_property)
           || (player_weapon->secondary.properties & weaponrail_property));
}


bool ItemIsWeaponOfType (edict_t *pItem, short weaponrail_property)
{
   // this function returns TRUE if the entity pointed to by pItem is a weapon which features
   // the property specified by weaponrail_property (one of WEAPONRAIL_PROPERTY_xxx), and FALSE
   // otherwise. To do so we cycle through each weapon in our weapons database and stop by those
   // which feature the wanted property in one of their two rails, and check if the models match
   // for both.

   int weapon_index;
   const char *item_model;

   // get the string in the string table and skip the 'models/w_' prefix...
   item_model = STRING (pItem->v.model) + 9;

   // loop through all weapons in game...
   for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
   {
      // does this weapon feature the right property AND do the models match ?
      if (((weapons[weapon_index].primary.properties & weaponrail_property)
           || (weapons[weapon_index].secondary.properties & weaponrail_property))
          && (strcmp (item_model, weapons[weapon_index].model + 9) == 0))
         return (TRUE); // if so, item is indeed a weapon featuring the property we want
   }

   return (FALSE); // if we get here, that's pItem does not feature the requested property
}


bool BotSelectWeaponOfType (player_t *pPlayer, short weaponrail_property)
{
   // this function makes the bot select a weapon in its inventory which features the property
   // described by the weaponrail_property parameter (one of WEAPONRAIL_PROPERTY_xxx). If a
   // weapon featuring the right property in one of its two rails is found, the function makes
   // the bot select it and returns TRUE ; if no weapon featuring this property is found in the
   // bot's inventory, it returns FALSE.

   int weapon_index;

   // loop through all weapons in game...
   for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
   {
      // does this weapon feature the right property AND does the bot own this weapon
      // AND does the bot have enough ammo to use with this weapon ?
      if (((weapons[weapon_index].primary.properties & weaponrail_property)
           || (weapons[weapon_index].secondary.properties & weaponrail_property))
          && (pPlayer->pEntity->v.weapons & (1 << weapons[weapon_index].id))
          && (pPlayer->Bot.bot_weapons[weapon_index].clip_ammo + *pPlayer->Bot.bot_weapons[weapon_index].primary_ammo > weapons[weapon_index].primary.min_ammo))
      {
         FakeClientCommand (pPlayer->pEntity, weapons[weapon_index].classname); // select it
         return (TRUE); // bot selected its new weapon, return TRUE
      }
   }

   return (FALSE); // bot found no weapon to select having the requested property, return FALSE
}


bool PlayerHasWeaponOfRange (player_t *pPlayer, char weaponrail_range)
{
   // this function returns TRUE if the player carries a weapon which works best at the range
   // specified by weaponrail_range (one of WEAPONRAIL_RANGE_xxx) - not necessarily holding it in
   // hand, it suffices he owns this weapon in his inventory), FALSE otherwise. To do so we
   // cycle through each weapon in our weapons database and stop by those that feature one rail
   // working best at the specified range, and check if the player owns one of them.

   int weapon_index;

   // loop through all weapons in game...
   for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
   {
      // does this weapon work at the wanted range AND does the player owns this weapon ?
      if (((weapons[weapon_index].primary.range == weaponrail_range)
           || (weapons[weapon_index].secondary.range == weaponrail_range))
          && (pPlayer->pEntity->v.weapons & (1 << weapons[weapon_index].id)))
         return (TRUE); // if so, the player is carrying a weapon featuring that property
   }

   return (FALSE); // if we get here, that's the player doesn't own any such weapon
}


bool PlayerHoldsWeaponOfRange (player_t *pPlayer, char weaponrail_range)
{
   // this function returns TRUE if the weapon the player is currently holding in his hands
   // works best at the range described by weaponrail_range (one of WEAPONRAIL_RANGE_xxx),
   // FALSE otherwise. We use the range field of each rail of the weapon hardware, as defined
   // in the weapons.cfg file, after locating the weapon in the database according to its model.

   weapon_t *player_weapon;

   // find the weapon our player is holding in his hands
   if (pPlayer->is_racc_bot)
      player_weapon = pPlayer->Bot.current_weapon->hardware;
   else
      player_weapon = FindWeaponByModel (STRING (pPlayer->pEntity->v.weaponmodel));

   if (player_weapon == &unknown_weapon)
      return (FALSE); // cancel if weapon not found

   // now decide if this weapon works at the wanted range in one of its two rails and return
   return ((player_weapon->primary.range == weaponrail_range)
           || (player_weapon->secondary.range == weaponrail_range));
}


bool ItemIsWeaponOfRange (edict_t *pItem, char weaponrail_range)
{
   // this function returns TRUE if the entity pointed to by pItem is a weapon which works best
   // at the range specified by weaponrail_range (one of WEAPONRAIL_RANGE_xxx), and FALSE
   // otherwise. To do so we cycle through each weapon in our weapons database and stop by those
   // which work best at the wanted range in one of their two rails, and check if the models
   // match for both.

   int weapon_index;
   const char *item_model;

   // get the string in the string table and skip the 'models/w_' prefix...
   item_model = STRING (pItem->v.model) + 9;

   // loop through all weapons in game...
   for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
   {
      // does this weapon work at the right range AND do the models match ?
      if (((weapons[weapon_index].primary.range == weaponrail_range)
           || (weapons[weapon_index].secondary.range == weaponrail_range))
          && (strcmp (item_model, weapons[weapon_index].model + 9) == 0))
         return (TRUE); // if so, item is indeed a weapon working at the range we want
   }

   return (FALSE); // if we get here, that's pItem does not work at the range we want
}


bool BotSelectWeaponOfRange (player_t *pPlayer, char weaponrail_range)
{
   // this function makes the bot select a weapon in its inventory which works best at the range
   // described by the weaponrail_range parameter (one of WEAPONRAIL_RANGE_xxx). If a weapon
   // working at the right range in one of its two rails is found, the function makes the bot
   // select it and returns TRUE ; if no weapon working at this range is found in the bot's
   // inventory, it returns FALSE.

   int weapon_index;

   // loop through all weapons in game...
   for (weapon_index = 0; weapon_index < weapon_count; weapon_index++)
   {
      // does this weapon work at the right range AND does the bot own this weapon
      // AND does the bot have enough ammo to use with this weapon ?
      if (((weapons[weapon_index].primary.range == weaponrail_range)
           || (weapons[weapon_index].secondary.range == weaponrail_range))
          && (pPlayer->pEntity->v.weapons & (1 << weapons[weapon_index].id))
          && (pPlayer->Bot.bot_weapons[weapon_index].clip_ammo + *pPlayer->Bot.bot_weapons[weapon_index].primary_ammo > weapons[weapon_index].primary.min_ammo))
      {
         FakeClientCommand (pPlayer->pEntity, weapons[weapon_index].classname); // select it
         return (TRUE); // bot selected its new weapon, return TRUE
      }
   }

   return (FALSE); // bot found no weapon to select working at the requested range, return FALSE
}


int BotRateWeapon (player_t *pPlayer, weapon_t *weapon)
{
   // this function returns an integer value describing how much a particular weapon (passed by
   // the "weapon" pointer) is interesting for the bot whose player structure is pointed to by
   // pPlayer. The more interesting the weapon, the higher the number this function returns.

   // FIXME: quick and dirty, empirical. Redo this right.

   int score;

   // does the bot already have this weapon ?
   if (pPlayer->pEntity->v.weapons & (1 << weapon->id))
      return (0); // then this weapon is of no interest for the bot

   // first reset the weapon score
   score = 0;

   // rate the interest regarding the weapon class

   // is weapon primary AND has bot no primary weapon yet ?
   if ((weapon->weapon_class == WEAPON_CLASS_PRIMARY)
       && !PlayerHasWeaponOfClass (pPlayer, WEAPON_CLASS_PRIMARY))
      score += 500; // then weapon is highly interesting, add 500 in a row

   // else is weapon secondary AND has bot no secondary weapon yet ?
   else if ((weapon->weapon_class == WEAPON_CLASS_SECONDARY)
            && !PlayerHasWeaponOfClass (pPlayer, WEAPON_CLASS_SECONDARY))
      score += 500; // then weapon is highly interesting, add 500 in a row

   // else is weapon grenade AND has bot no grenade yet ?
   else if ((weapon->weapon_class == WEAPON_CLASS_GRENADE)
            && !PlayerHasWeaponOfClass (pPlayer, WEAPON_CLASS_GRENADE))
      score += 500; // then weapon is highly interesting, add 500 in a row

   // now rate the interest regarding the weapon range

   // does this weapon work best in melee AND has bot no melee weapon yet ?
   if ((((weapon->primary.range > 0) && (weapon->primary.range == WEAPONRAIL_RANGE_MELEE))
        || ((weapon->secondary.range > 0) && (weapon->secondary.range == WEAPONRAIL_RANGE_MELEE)))
       && !PlayerHasWeaponOfRange (pPlayer, WEAPONRAIL_RANGE_MELEE))
      score += 10; // interesting, add 10

   // else does this weapon work best at close range AND has bot no close range weapon yet ?
   else if ((((weapon->primary.range > 0) && (weapon->primary.range == WEAPONRAIL_RANGE_CLOSE))
             || ((weapon->secondary.range > 0) && (weapon->secondary.range == WEAPONRAIL_RANGE_CLOSE)))
            && !PlayerHasWeaponOfRange (pPlayer, WEAPONRAIL_RANGE_CLOSE))
      score += 10; // interesting, add 10

   // else does this weapon work best at medium range AND has bot no medium range weapon yet ?
   else if ((((weapon->primary.range > 0) && (weapon->primary.range == WEAPONRAIL_RANGE_MEDIUM))
             || ((weapon->secondary.range > 0) && (weapon->secondary.range == WEAPONRAIL_RANGE_MEDIUM)))
            && !PlayerHasWeaponOfRange (pPlayer, WEAPONRAIL_RANGE_MEDIUM))
      score += 10; // interesting, add 10

   // else does this weapon work best at long range AND has bot no long range weapon yet ?
   else if ((((weapon->primary.range > 0) && (weapon->primary.range == WEAPONRAIL_RANGE_FAR))
             || ((weapon->secondary.range > 0) && (weapon->secondary.range == WEAPONRAIL_RANGE_FAR)))
            && !PlayerHasWeaponOfRange (pPlayer, WEAPONRAIL_RANGE_FAR))
      score += 10; // interesting, add 10

   // now parse each parameter individually

   if (weapon->primary.properties & WEAPONRAIL_PROPERTY_DISABLER)
   {
      if (PlayerHasWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_DISABLER))
         score -= 2;
      else
         score += 11;
   }
   if (weapon->primary.properties & WEAPONRAIL_PROPERTY_WATERPROOF)
   {
      if (PlayerHasWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_MISSILE))
         score += 1;
      else
         score += 12;
   }
   if (weapon->primary.properties & WEAPONRAIL_PROPERTY_LIGHTDAMAGE)
   {
      if (PlayerHasWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_LIGHTDAMAGE))
         score -= 49;
      else
         score -= 16;
   }
   if (weapon->primary.properties & WEAPONRAIL_PROPERTY_MEDIUMDAMAGE)
   {
      if (PlayerHasWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_MEDIUMDAMAGE))
         score -= 6;
      else
         score += 7;
   }
   if (weapon->primary.properties & WEAPONRAIL_PROPERTY_HEAVYDAMAGE)
   {
      if (PlayerHasWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_HEAVYDAMAGE))
         score += 5;
      else
         score += 31;
   }
   if (weapon->primary.properties & WEAPONRAIL_PROPERTY_RADIUSEFFECT)
   {
      if (PlayerHasWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_RADIUSEFFECT))
         score -= 5;
      else
         score += 15;
   }
   if (weapon->primary.properties & WEAPONRAIL_PROPERTY_AUTOMATIC)
   {
      if (PlayerHasWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_AUTOMATIC))
         score += 17;
      else
         score += 33;
   }
   if (weapon->primary.properties & WEAPONRAIL_PROPERTY_BUCKSHOT)
   {
      if (PlayerHasWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_BUCKSHOT))
         score -= 9;
      else
         score += 16;
   }
   if (weapon->primary.properties & WEAPONRAIL_PROPERTY_SCOPED)
   {
      if (PlayerHasWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_SCOPED))
         score += 5;
      else
         score += 17;
   }
   if (weapon->primary.properties & WEAPONRAIL_PROPERTY_SNIPER)
   {
      if (PlayerHasWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_LIGHTDAMAGE))
         score -= 22;
      else
         score += 14;
   }
   if (weapon->primary.properties & WEAPONRAIL_PROPERTY_SILENCED)
   {
      if (PlayerHasWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_SILENCED))
         score += 10;
      else
         score += 28;
   }
   if (weapon->primary.properties & WEAPONRAIL_PROPERTY_MISSILE)
   {
      if (PlayerHasWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_MISSILE))
         score -= 2;
      else
         score += 10;
   }
   if (weapon->primary.properties & WEAPONRAIL_PROPERTY_HOMING)
   {
      if (PlayerHasWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_HOMING))
         score += 5;
      else
         score += 20;
   }
   if (weapon->primary.properties & WEAPONRAIL_PROPERTY_TOSS)
   {
      if (PlayerHasWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_TOSS))
         score += 2;
      else
         score += 10;
   }
   if (weapon->primary.properties & WEAPONRAIL_PROPERTY_PLACE)
   {
      if (PlayerHasWeaponOfType (pPlayer, WEAPONRAIL_PROPERTY_PLACE))
         score -= 5;
      else
         score += 5;
   }

   return (score); // return the overall score for this weapon
}
