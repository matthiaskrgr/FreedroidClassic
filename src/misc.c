/*----------------------------------------------------------------------
 *
 * Desc: miscellaeous helpful functions for paraplus
 *	 
 *----------------------------------------------------------------------*/

/* 
 *
 *   Copyright (c) 1994, 2002 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *
 *
 *  This file is part of Freedroid
 *
 *  Freedroid is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Freedroid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Freedroid; see the file COPYING. If not, write to the 
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 *
 */
#define _misc_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"




#undef DIAGONAL_KEYS_AUS
// #undef QUEUEDEBUG
#define QUEUEDEBUG

#define MESPOSX 0
#define MESPOSY 64
#define MESHOEHE 8
#define MESBARBREITE 320

#define MAX_MESSAGE_LEN 100
#define MESBAR_MEM	MESBARBREITE*MESHOEHE+1000

// Die Definition f"ur eine Message kann ruhig lokal hier stehen, da sie
// nur innerhalb des Moduls gebraucht wird.

typedef struct
{
  void *NextMessage;
  int MessageCreated;
  char *MessageText;
}
message, Message;

void CreateMessageBar (char *MText);
void CleanMessageLine (void);
void AdvanceQueue (void);
void SinglePlayerMenu (void);

int VectsHaveBeenTurned = 0;
unsigned char *MessageBar;
message *Queue = NULL;

// int ThisMessageTime=0;               /* Counter fuer Message-Timing */

// This Function is for the PORT!!!!
// Replacing all MyRandom-calls with MyMyRandom-calls

void
gotoxy (int x, int y)
{
  return;
};

void
DebugPrintf (char *Print_String)
{
  static int first_time = TRUE;
  FILE *debugfile;

  if (debug_level == 0) return;

  if (first_time)		/* make sure the first call deletes previous log-file */
    {
      debugfile = fopen ("DEBUG.OUT", "w");
      first_time = FALSE;
    }
  else
    debugfile = fopen ("DEBUG.OUT", "a");

  fprintf (debugfile, Print_String);
  fclose (debugfile);
};

void
DebugPrintfFloat (float Print_Float)
{
  FILE *debugfile;

  if (debug_level == 0) return;

  debugfile = fopen ("DEBUG.OUT", "a");

  fprintf (debugfile, "%f", Print_Float);
  fclose (debugfile);
};

void
DebugPrintfInt (int Print_Int)
{
  FILE *debugfile;

  if (debug_level == 0) return;

  debugfile = fopen ("DEBUG.OUT", "a");

  fprintf (debugfile, "%d", Print_Int);
  fclose (debugfile);
};

// This Function is for the PORT!!!!
// Replacing all MyRandom-calls with MyMyRandom-calls

/*-----------------------------------------------------------------
 * Desc: return an integer-random number in the range [0,Obergrenze]
 * 
 *-----------------------------------------------------------------*/
int
MyRandom (int Obergrenze)
{
  float Zwisch;
  int ReinerZufall;
  int dice_val;    /* the result in [0, Obergrenze] */

  ReinerZufall = rand ();
  Zwisch = 1.0*ReinerZufall/RAND_MAX; /* random number in [0;1] */

  /* 
   * we always round OFF for the resulting int, therefore
   * we first add 0.99999 to make sure that Obergrenze has
   * roughly the same probablity as the other numbers 
   */
  dice_val = (int)( Zwisch * (1.0 * Obergrenze + 0.99999) );
  return (dice_val);
} /* MyRandom () */


void
reverse (char s[])
{
  int c, i, j;
  for (i = 0, j = strlen (s) - 1; i < j; i++, j--)
    {
      c = s[i];
      s[i] = s[j];
      s[j] = c;
    }
}/* void reverse(char s[]) siehe Kernighan&Ritchie! */


char *
itoa (int n, char s[], int Dummy)
{
  int i, sign;

  if ((sign = n) < 0)
    n = -n;
  i = 0;
  do
    {
      s[i++] = n % 10 + '0';
    }
  while ((n /= 10) > 0);
  if (sign < 0)
    s[i++] = '-';
  s[i] = '\0';
  reverse (s);
  return s;
}				// void itoa(int n, char s[]) siehe Kernighan&Ritchie!

// This Function is for the PORT!!!!
// Replacing all MyRandom-calls with MyMyRandom-calls

char *
ltoa (long n, char s[], int Dummy)
{
  int i, sign;

  if ((sign = n) < 0)
    n = -n;
  i = 0;
  do
    {
      s[i++] = n % 10 + '0';
    }
  while ((n /= 10) > 0);
  if (sign < 0)
    s[i++] = '-';
  s[i] = '\0';
  reverse (s);
  return s;
}				// void ltoa(long n, char s[]) angelehnt an itoa!

/* **********************************************************************
   Diese Funktion l"oscht alle Roboter auf dem momentanen Level
**********************************************************************/
void
Armageddon (void)
{
  char key =' ';
  int i;

  printf ("\nKill all droids on ship (y/n) ? \n");
  while ((key != 'y') && (key != 'n'))
    key = getchar_raw ();
  if (key == 'n')
    return;
  else
    for (i = 0; i < NumEnemys; i++)
      {
	Feindesliste[i].energy = 0;
	Feindesliste[i].Status = OUT;
      }
} // void Armageddon(void)

/* **********************************************************************
 *	Diese Funktion teleportiert an einen beliebigen Ort im Schiff.
 *	Es werden Levelnummer und Grobkoordinaten "ubergeben.
 *
 **********************************************************************/

void
Teleport (int LNum, int X, int Y)
{
  int curLevel = LNum;
  int array_num = 0;
  Level tmp;
  int i;

  /* Neuen Level und Position setzen */

  if (curLevel != CurLevel->levelnum)
    {				/* wirklich neu ??? */

      FadeLevel ();
      /* Aktuellen Level setzen */
      while ((tmp = curShip.AllLevels[array_num]) != NULL)
	{
	  if (tmp->levelnum == curLevel)
	    break;
	  else
	    array_num++;
	}

      CurLevel = curShip.AllLevels[array_num];

      /* Enemys gut verteilen: */
      ShuffleEnemys ();

      /* Position des Influencer richtig setzen */
      Me.pos.x = Grob2Fein(X);   /* Macro to convert from grob to fein */
      Me.pos.y = Grob2Fein(Y);

      /* Alle Blasts und Bullets loeschen */
      for (i = 0; i < MAXBLASTS; i++)
	AllBlasts[i].type = OUT;
      for (i = 0; i < MAXBULLETS; i++)
	{
	  AllBullets[i].type = OUT;
	  AllBullets[i].mine = FALSE;
	}
    }
  else
    {
      FadeLevel ();
      Me.pos.x = Grob2Fein(X);
      Me.pos.y = Grob2Fein(Y);
    }

  LeaveElevatorSound ();

  UnfadeLevel ();

} /* Teleport() */


/* **********************************************************************
 *  Diese Funktion stellt ein praktisches Cheatmenu zur Verf"ugung
 *
 ***********************************************************************/
extern int CurrentlyCPressed; 	/* the key that brought as in here */
				/* we need to make sure it is set as released */
				/* before we leave ...*/
void
Cheatmenu (void)
{
  char *input;		/* string input from user */
  const char clearscr[] =
    "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
  int Weiter;
  int LNum, X, Y;
  int i, l;
  Waypoint WpList;      /* pointer on current waypoint-list  */

  // Prevent distortion of framerate by the delay coming from 
  // the time spend in the menu.
  Activate_Conservative_Frame_Computation();

  /* 
   * this cheat-menu currently uses the text-window for output
   * therefore, if we're in fullscreen-mode: change to window-mode
   */
  if (fullscreen_on)
    SDL_WM_ToggleFullScreen (ScaledSurface);

  Weiter = FALSE;
  while (!Weiter)
    {
      printf (clearscr);
      printf ("\n\nCurrent position: Level=%d, X=%d, Y=%d\n\n",
	      CurLevel->levelnum, (int)GrobX, (int)GrobY);
      printf (" a. Armageddon (alle Robots sprengen)\n");
      printf (" l. robot list of current level\n");
      printf (" g. complete robot list\n");
      printf (" d. destroy robots on current level\n");
      printf (" t. Teleportation\n");
      printf (" r. change to new robot type\n");
      printf (" i. Invinciblemode: %s\n",
		 InvincibleMode ? "ON" : "OFF");
      printf (" f. full energy\n");
      printf (" b. blink-energy\n");
      printf (" c. Conceptview: %s\n", Conceptview ? "ON" : "OFF");
      printf (" h. Hide invisible map parts: %s\n",
	      HideInvisibleMap ? "ON" : "OFF" );
      printf (" m. Map of Deck xy\n");
      printf (" s. Sound: %s\n", sound_on ? "ON" : "OFF");
      printf (" x. Fullscreen : %s\n", fullscreen_on ? "ON" : "OFF");
      printf (" w. Print current waypoints\n");
      printf ("\n q. RESUME game\n");

      printf ("\n[ Input in Freedroid window please! ]");
      fflush (stdout);

      switch (getchar_raw ())
	{
	case 'a': /* armageddon */
	  Weiter = 1;
	  Armageddon ();
	  break;

	case 'l': /* robot list of this deck */
	  l = 0; /* line counter for enemy output */
	  for (i = 0; i < NumEnemys; i++)
	    {
	      if (Feindesliste[i].levelnum == CurLevel->levelnum) 
		{
		  if (l && !(l%20)) 
		    {
		      printf ("\n --- MORE --- \n");
		      if( getchar_raw () == 'q')
			break;
		    }
		  if (!(l % 20) )  
		    {
		      printf (clearscr);
		      printf ("\n\n NR.\tID\tX\tY\tENERGY.\n");
		      printf ("---------------------------------------------\n");
		    }
		  
		  l ++;
		  printf ("%d.\t%s\t%d\t%d\t%d.\n", i,
			  Druidmap[Feindesliste[i].type].druidname,
			  Fein2Grob (Feindesliste[i].pos.x),
			  Fein2Grob (Feindesliste[i].pos.y),
			  (int)Feindesliste[i].energy);
		} /* if (enemy on current level)  */
	    } /* for (i<NumEnemys) */

	  printf (" --- END --- \n");
	  getchar_raw ();
	  break;

	case 'g': /* complete robot list of this ship */
	  for (i = 0; i < NumEnemys; i++)
	    {
	      if (i && !(i%20)) 
		{
		  printf (" --- MORE --- \n");
		  if( getchar_raw () == 'q')
		    break;
		}
	      if ( !(i % 20) )
		{
		  printf (clearscr);
		  printf ("\n\nNr.\tLev.\tID\tEnergy\n");
		  printf ("------------------------------\n");
		}
	      
	      printf ("%d\t%d\t%s\t%d\n",
		      i, Feindesliste[i].levelnum,
		      Druidmap[Feindesliste[i].type].druidname,
		      (int)Feindesliste[i].energy);
	    } /* for (i<NumEnemys) */

	  printf (" --- END --- \n");
	  getchar_raw ();
	  break;


	case 'd': /* destroy all robots on this level */
	  for (i = 0; i < NumEnemys; i++)
	    {
	      if (Feindesliste[i].levelnum == CurLevel->levelnum)
		Feindesliste[i].energy = 0;
	    }
	  printf ("\n\nAll robots on this deck killed! \n");
	  getchar_raw ();
	  break;


	case 't': /* Teleportation */
	  printf ("\n Enter Levelnummer, X-Pos, Y-Pos: ");
	  input = GetString (40, 1);
	  sscanf (input, "\n%d, %d, %d", &LNum, &X, &Y);
	  free (input);
	  Teleport (LNum, X, Y);
	  printf ("\nThis is your position on level %d.\n", LNum);
	  printf ("\nPress key to continue \n");
	  getchar_raw ();
	  break;

	case 'r': /* change to new robot type */
	  printf ("\nType number of new robot: ");
	  input = GetString (40, 1);
	  for (i = 0; i < ALLDRUIDTYPES; i++)
	    if (!strcmp (Druidmap[i].druidname, input))
	      break;

	  if (i == ALLDRUIDTYPES)
	    {
	      printf ("\nUnrecognized robot-type: %s\n", input);
	      getchar_raw ();
	    }
	  else
	    {
	      Me.type = i;
	      Me.energy = Druidmap[Me.type].maxenergy;
	      Me.health = Me.energy;
	      printf ("\nYou are now a %s. Have fun!\n", input);
	      getchar_raw ();
	      RedrawInfluenceNumber ();
	    }
	  free (input);
	  break;

	case 'i': /* togge Invincible mode */
	  InvincibleMode = !InvincibleMode;
	  break;

	case 'f': /* complete heal */
	  Me.energy = Druidmap[Me.type].maxenergy;
	  Me.health = Me.energy;
	  printf ("\nSie sind wieder gesund! \n");
	  getchar_raw ();
	  break;

	case 'b': /* minimal energy */
	  Me.energy = 1;
	  printf ("\nSie sind jetzt ziemlich schwach! \n");
	  getchar_raw ();
	  break;

	case 'c': /* toggle concept view */
	  Conceptview = !Conceptview;
	  break;
	  
	case 'h': /* toggle hide invisible map */
	  HideInvisibleMap = !HideInvisibleMap;
	  break;

	case 's': /* toggle sound on/off */
	  sound_on = !sound_on;
	  break;

	case 'm': /* Show deck map in Concept view */
	  printf ("\nLevelnum: ");
	  input = GetString (40, 1);
	  sscanf (input, "%d", &LNum);
	  free (input);
	  ShowDeckMap (curShip.AllLevels[LNum]);
	  getchar_raw ();
	  break;

	case 'x': /* toggle fullscreen - mode */
	  fullscreen_on = !fullscreen_on;
	  break;
	  
	case 'w':  /* print waypoint info of current level */
	  WpList = CurLevel->AllWaypoints;
	  for (i=0; i<MAXWAYPOINTS && WpList[i].x; i++)
	    {
	      if (i && !(i%20))
		{
		  printf (" ---- MORE -----\n");
		  if (getchar_raw () == 'q')
		    break;
		}
	      if ( !(i%20) )
		{
		  printf (clearscr);
		  printf ("\n\nNr.   X   Y      C1  C2  C3  C4 \n");
		  printf ("------------------------------------\n");
		}
	      printf ("%2d   %2d  %2d      %2d  %2d  %2d  %2d\n",
		      i, WpList[i].x, WpList[i].y,
		      WpList[i].connections[0],
		      WpList[i].connections[1],
		      WpList[i].connections[2],
		      WpList[i].connections[3]);

	    } /* for (all waypoints) */
	  printf (" --- END ---\n");
	  getchar_raw ();
	  break;

	case ' ':
	case 'q':
	  Weiter = 1;
	  break;
	} /* switch (getchar_raw()) */
    } /* while (!Weiter) */

  InitBars = TRUE;
  
  keyboard_update (); /* treat all pending keyboard events */
  /* 
   * when changing windows etc, sometimes a key-release event gets 
   * lost, so we have to make sure that CPressed is no longer set
   * or we stay here for ever...
   */
  CurrentlyCPressed = FALSE;

  /* clear terminal (if it's not a terminal: tant pis) */
  printf (clearscr);
  printf ("Back in the game!\n");

  /* toggle back to fullscreen, if that's where we came from */
  if (fullscreen_on)
    SDL_WM_ToggleFullScreen (ScaledSurface);

  return;
} /* Cheatmenu() */

/* -----------------------------------------------------------------
 *-----------------------------------------------------------------*/
void
OptionsMenu (void)
{
  int Weiter = 0;
  int MenuPosition=1;

#define FIRST_MENU_ITEM_POS_X 3*(BLOCKBREITE - 7)
  // #define FIRST_MENU_ITEM_POS_Y (BLOCKHOEHE + 12)
  // #define MENU_ITEM_DISTANCE 21
#define SINGLE_PLAYER_POSITION 1
#define MULTI_PLAYER_POSITION 2
#define OPTIONS_POSITION 3
#define HELP_POSITION 4
#define QUIT_POSITION 5

  // Prevent distortion of framerate by the delay coming from 
  // the time spend in the menu.
  Activate_Conservative_Frame_Computation();

  // This is not some Debug Menu but an optically impressive 
  // menu for the player.  Therefore I suggest we just fade out
  // the game screen a little bit.

  while ( EscapePressed() );

  while (!Weiter)
    {

      PutInternFenster( FALSE );

      MakeGridOnScreen( Outline320x200 );

      // Highlight currently selected option with an influencer before it
      DisplayMergeBlock( FIRST_MENU_ITEM_POS_X, (MenuPosition+3) * (FontHeight(Font1)/2) - BLOCKBREITE/4, 
			 Influencepointer, BLOCKBREITE, BLOCKHOEHE, RealScreen );


      PrepareScaledSurface(FALSE);

      CenteredPutString (ScaledSurface ,  4*FontHeight(Font1),    "Single Player");
      CenteredPutString (ScaledSurface ,  5*FontHeight(Font1),    "Multi Player");
      CenteredPutString (ScaledSurface ,  6*FontHeight(Font1),    "Options");
      CenteredPutString (ScaledSurface ,  7*FontHeight(Font1),    "Level Editor");
      CenteredPutString (ScaledSurface ,  8*FontHeight(Font1),    "Quit");

      SDL_UpdateRect(ScaledSurface, 0, 0, SCREENBREITE*SCALE_FACTOR, SCREENHOEHE*SCALE_FACTOR);

      // Wait until the user does SOMETHING

      while( !SpacePressed() && !EnterPressed() && !UpPressed() && !DownPressed() && !EscapePressed() ) 
	{
	  keyboard_update();
	}

      // 
      if ( EscapePressed() )
	{
	  while ( EscapePressed() );
	  Weiter=!Weiter;
	}
      if (EnterPressed() || SpacePressed() ) 
	{
	  MenuItemSelectedSound();
	  switch (MenuPosition) 
	    {

	    case SINGLE_PLAYER_POSITION:
	      while (EnterPressed() || SpacePressed() );
	      SinglePlayerMenu();
	      break;
	    case QUIT_POSITION:
	      DebugPrintf("\nvoid OptionsMenu(void): Quit Requested by user.  Terminating...");
	      Terminate(0);
	      break;
	    default: 
	      break;
	    }
	  // Weiter=!Weiter;
	}
      if (UpPressed()) 
	{
	  if (MenuPosition > 1) MenuPosition--;
	  MoveMenuPositionSound();
	  while (UpPressed());
	}
      if (DownPressed()) 
	{
	  if (MenuPosition < 5) MenuPosition++;
	  MoveMenuPositionSound();
	  while (DownPressed());
	}
    }
  ClearGraphMem (InternalScreen);
  ClearGraphMem (RealScreen);
  Update_SDL_Screen();
  DisplayRahmen (InternalScreen);
  InitBars = TRUE;

  vga_clear ();
  
  // keyboard_init (); /* return to raw keyboard mode */

  return;
} // OptionsMenu

/* -----------------------------------------------------------------
 *-----------------------------------------------------------------*/
void
SinglePlayerMenu (void)
{
  int Weiter = 0;
  int MenuPosition=1;

enum
  { NEW_GAME_POSITION=1, SHOW_HISCORE_POSITION=2, SHOW_MISSION_POSITION=3, BACK_POSITION=4 };

  while (!Weiter)
    {

      PutInternFenster( FALSE );

      MakeGridOnScreen( Outline320x200 );

      // Highlight currently selected option with an influencer before it
      DisplayMergeBlock( FIRST_MENU_ITEM_POS_X, (MenuPosition+3) * (FontHeight(Font1)/2) - BLOCKBREITE/4, 
			 Influencepointer, BLOCKBREITE, BLOCKHOEHE, RealScreen );


      PrepareScaledSurface(FALSE);

      CenteredPutString (ScaledSurface ,  4*FontHeight(Font1),    "New Game");
      CenteredPutString (ScaledSurface ,  5*FontHeight(Font1),    "Show Hiscore List");
      CenteredPutString (ScaledSurface ,  6*FontHeight(Font1),    "Show Mission Instructions");
      CenteredPutString (ScaledSurface ,  7*FontHeight(Font1),    "Back");

      SDL_UpdateRect(ScaledSurface, 0, 0, SCREENBREITE*SCALE_FACTOR, SCREENHOEHE*SCALE_FACTOR);

      // Wait until the user does SOMETHING

      while( !SpacePressed() && !EnterPressed() && !UpPressed() && !DownPressed() ) 
	{
	  keyboard_update();
	}

      // 
      if ( EscapePressed() )
	{
	  Weiter=!Weiter;
	}
      if (EnterPressed() || SpacePressed() ) 
	{
	  MenuItemSelectedSound();
	  switch (MenuPosition) 
	    {

	    case NEW_GAME_POSITION:
	      while (EnterPressed() || SpacePressed() ) ;
	      InitNewGame();
	      break;
	    case SHOW_HISCORE_POSITION: 
	    case SHOW_MISSION_POSITION:
	    case BACK_POSITION:
	      while (EnterPressed() || SpacePressed() ) ;
	      break;
	    default: 
	      break;
	    }
	  Weiter=!Weiter;
	}
      if (UpPressed()) 
	{
	  if (MenuPosition > 1) MenuPosition--;
	  MoveMenuPositionSound();
	  while (UpPressed());
	}
      if (DownPressed()) 
	{
	  if (MenuPosition < 5) MenuPosition++;
	  MoveMenuPositionSound();
	  while (DownPressed());
	}
    }
  ClearGraphMem (InternalScreen);
  ClearGraphMem (RealScreen);
  Update_SDL_Screen();
  DisplayRahmen (InternalScreen);
  InitBars = TRUE;

  vga_clear ();
  
  // keyboard_init (); /* return to raw keyboard mode */

  return;
} // SinglePlayerMenu


/*@Function============================================================
@Desc: Testfunktion fuer InsertMessage()

@Ret: 
@Int:
* $Function----------------------------------------------------------*/
void
InsertNewMessage (void)
{
  static int counter = 0;
  char testmessage[100];

  DebugPrintf
    ("\nvoid InsertNewMessage(void): real function call confirmed...");

  counter++;
  sprintf (testmessage, "Das ist die %d .te Message !!", counter);
  InsertMessage (testmessage);

  DebugPrintf ("\nvoid InsertNewMessage(void): end of function reached...");
  return;
}				// void InsertNewMessage(void)

/*@Function============================================================
@Desc: 	Diese Funktion beendet das Programm und setzt alle notwendigen Dinge
	auf ihre Ausgangswerte zur"uck.
	Wenn ein ExitCode != 0 angegeben wurde wartet er noch auf eine Taste.

@Ret: 
@Int:
* $Function----------------------------------------------------------*/
void
Terminate (int ExitCode)
{

  DebugPrintf ("\nvoid Terminate(int ExitStatus) wurde aufgerufen....\n");
  printf ("GameOver : %i\n", GameOver);
  // keyboard_close ();
  // vga_setmode (TEXT);
  SDL_Quit();
  exit (ExitCode);
  return;

}				// void Terminate(int ExitCode)


/*@Function============================================================
@Desc: 

@Ret: 
@Int:
* $Function----------------------------------------------------------*/
void
KillQueue (void)
{
  while (Queue)
    AdvanceQueue ();
}

/*@Function============================================================
@Desc: AdvanceQueue(): rueckt in der Queue eins vor und loescht Vorgaenger.

@Ret:
@Int:
* $Function----------------------------------------------------------*/
void
AdvanceQueue (void)
{
  message *tmp;

  DebugPrintf ("\nvoid AdvanceQueue(void): Funktion wurde echt aufgerufen.");

  if (Queue == NULL)
    return;

  if (Queue->MessageText)
    free (Queue->MessageText);
  tmp = Queue;

  Queue = Queue->NextMessage;

  free (tmp);

  DebugPrintf
    ("\nvoid AdvanceQueue(void): Funktion hat ihr natuerliches Ende erfolgreich erreicht....");
}				// void AdvanceQueue(void)


/*@Function============================================================
@Desc: Handles all Screenmessages

@Ret: 
@Int:
* $Function----------------------------------------------------------*/
void
PutMessages (void)
{
  static int MesPos = 0;	/* X-Position der Message-Bar */
  static int Working = FALSE;	/* wird gerade eine Message bearbeitet ?? */
  message *LQueue;		/* Bewegl. Queue-Pointer */
  int i;

  if (!PlusExtentionsOn)
    return;

  DebugPrintf ("\nvoid PutMessages(void): Funktion wurde echt aufgerufen.");

  if (!Queue)
    return;			/* nichts liegt an */
  if (!Working)
    ThisMessageTime = 0;	/* inaktiv, aber Queue->reset time */


#ifdef QUEUEDEBUG
  gotoxy (1, 10);
  printf ("Time: %d", ThisMessageTime);
#endif

// Ausgabe der momentanen Liste:
#ifdef QUEUEDEBUG
  LQueue = Queue;
  i = 0;
  gotoxy (1, 5);
  DebugPrintf ("\nvoid PutMessages(void): This is the Queue of Messages:\n");
  while (LQueue != NULL)
    {
      if ((LQueue->MessageText) == NULL)
	{
	  DebugPrintf
	    ("\nvoid PutMessages(void): ERROR: Textpointer is NULL !!!!!!\n");
	  getchar ();
	}
      printf ("%d. '%s' %d\n", i, LQueue->MessageText,
	      LQueue->MessageCreated);
      i++;
      LQueue = LQueue->NextMessage;
    }
  DebugPrintf (" NULL reached !\n");
#endif


  // Wenn die Nachricht schon sehr alt ist, wird sie gel"oscht. 
  if (Working && (ThisMessageTime > MaxMessageTime))
    {
      AdvanceQueue ();
      CleanMessageLine ();
      Working = FALSE;		// inaktiv
      ThisMessageTime = 0;	// Counter init.
      return;
    }


  /* Alte Mes. hat MinTime gelebt, neue wartet */
  if ((ThisMessageTime > MinMessageTime) && (Queue->NextMessage))
    {
      AdvanceQueue ();		/* Queue weiterbewegen */
      Working = FALSE;		/* inaktiv setzen */
      ThisMessageTime = 0;	/* counter neu init. */
      return;
    }

  // Modul inaktiv und neue Message da --> aktivieren
  if ((!Working) && Queue)
    {

      // Wenn die neue Nachricht noch nicht erzeugt ist, dann erzeuge sie
      if (!Queue->MessageCreated)
	{
	  CreateMessageBar (Queue->MessageText);
	  Queue->MessageCreated = TRUE;
	}

      ThisMessageTime = 0;	/* counter init. */
      CleanMessageLine ();	/* Zeile loeschen */
      Working = TRUE;		/* aktiviert */
    }


  // Modul ist gerade aktiv --> bewegen und anzeigen
  if (Working && Queue)
    {

      MesPos = 10 * ThisMessageTime;	/* zeitl. synchronisierte Bewegung */

      /* nicht ueber linken Rand fahren !! */
      if (MesPos > (MESBARBREITE - 2))
	MesPos = MESBARBREITE - 2;

      // 
      // SINCE 'REALSCREEN' NO LONGER RELEVANT, A NEW ROUTINE HAS TO BE FOUND FOR THE PORT
      //
      //    for(i=0;i<MESHOEHE;i++) {
      //      memcpy(
      //             RealScreen+MESPOSX+MESBARBREITE-MesPos+(MESPOSY+i)*SCREENBREITE,
      //             MessageBar+i*MESBARBREITE,
      //             MesPos
      //            );
      //    } /* for */

      //
      // THIS IS THE NEW ROUTINE.
      //
      for (i = 0; i < MESHOEHE; i++)
	{
	  vga_drawscansegment (MessageBar + i * MESBARBREITE,
			       MESPOSX - MesPos, MESPOSY + i, MesPos);
	}
    }				/* if aktiv + Message da */

}				/* Put Messages */

/*@Function============================================================
@Desc: CleanMessageLine: l"oscht die Nachrichtenzeile am Bildschrim

@Ret: 
@Int:
* $Function----------------------------------------------------------*/
void
CleanMessageLine (void)
{
  memset (RealScreen + MESPOSX + MESPOSY * SCREENBREITE, 0,
	  SCREENBREITE * MESHOEHE);
}

/*@Function============================================================
@Desc: 

@Ret: 
@Int:
* $Function----------------------------------------------------------*/
void
CreateMessageBar (char *MText)
{
  char Worktext[42];
  int i, j;

  DebugPrintf
    ("\nvoid CreateMessageBar(char* MText): Funktion echt aufgerufen.");

  // "Uberl"angen checken
  if (strlen (MText) > 40)
    {
      DebugPrintf
	("\nvoid CreateMessageBar(char* MText): Message hat mehr als 40 Zeichen !.\n");
      getchar ();
      Terminate (ERR);
    }

  // Speicher anfordern, wenn noch keiner da
  if (MessageBar == NULL)
    if ((MessageBar = MyMalloc (MESBAR_MEM)) == NULL)
      {
	DebugPrintf
	  ("\nvoid CreateMessageBar(char* MText): Bekomme keinen Speicher fuer MessageBar !!\n");
	getchar ();
	Terminate (ERR);
      }

  // Message auf 40 Zeichen erg"anzen
  strcpy (Worktext, MText);
  while (strlen (Worktext) < 40)
    strcat (Worktext, " ");

  // Im internen Screen die Nachricht anzeigen und dann ausschneiden
  for (i = 0; i < 40; i++)
    {
      for (j = 0; j < 8; j++)
	{
	  memcpy ((MessageBar + i * 8 + j * SCREENBREITE),
		  (Data70Pointer + Worktext[i] * 8 * 8 + j * 8), 8);
	}
    }

// Am unteren Rand die Nachricht anzeigen und dann ausschneiden
//      gotoxy(1,4);
//      printf("%s",MText);
//      memcpy(MessageBar,RealScreen+3*8*SCREENBREITE,8*SCREENBREITE);

  DebugPrintf
    ("\nvoid CreateMessageBar(char* MText): Funktion hat ihr natuerliches Ende erreicht.");
}				// void CreateMessageBar(char* MText)

/*@Function============================================================
@Desc: 

@Ret: 
@Int:
* $Function----------------------------------------------------------*/
void
InsertMessage (char *MText)
{
  message *LQueue = Queue;

  DebugPrintf
    ("\nvoid InsertMessage(char* MText): real function call confirmed...");

  if (LQueue)
    {
      // Bis vor die n"achste freie Position vorr"ucken
      while (LQueue->NextMessage != NULL)
	LQueue = LQueue->NextMessage;
      LQueue->NextMessage = MyMalloc (sizeof (message) + 1);
      LQueue = LQueue->NextMessage;
    }
  else
    {
      Queue = MyMalloc (sizeof (message) + 1);
      LQueue = Queue;
    }

  LQueue->MessageText = MyMalloc (MAX_MESSAGE_LEN + 1);
  strcpy (LQueue->MessageText, MText);
  LQueue->NextMessage = NULL;
  LQueue->MessageCreated = FALSE;
#ifdef QUEUEDEBUG
  gotoxy (1, 11);
  printf ("\nvoid InsertMessage(char* MText): A message has been added:%s",
	  MText);
#endif

  DebugPrintf ("\nvoid InsertMessage(char* MText): end of function reached.");
}				// void InsertMessage(char* MText)

/* **********************************************************************
	Diese Funktion erledigt ein normales Malloc, trifft zuerst aber ein
	Par Sicherheitsvorkehrungen.
	**********************************************************************/
void *
MyMalloc (long Mamount)
{
  void *Mptr = NULL;

  if ((Mptr = malloc ((size_t) Mamount)) == NULL)
    {
      printf (" MyMalloc(%ld) did not succeed!\n", Mamount);
      getchar ();
    }
  return Mptr;
}				// void* MyMalloc(long Mamount)

/*@Function============================================================
@Desc: DirToVect
			int direction: a number representing a direction
			Vect vector: 	a pointer to a vector for the result
								(must be already allocated !)

@Ret: void
@Int:
* $Function----------------------------------------------------------*/
void
DirToVect (int dir, Vect vector)
{

  switch (dir)
    {
    case OBEN:
      vector->x = 0;
      vector->y = -1;
      break;

    case RECHTSOBEN:
      vector->x = 1;
      vector->y = -1;
      break;

    case RECHTS:
      vector->x = 1;
      vector->y = 0;
      break;

    case RECHTSUNTEN:
      vector->x = 1;
      vector->y = 1;
      break;

    case UNTEN:
      vector->x = 0;
      vector->y = 1;
      break;

    case LINKSUNTEN:
      vector->x = -1;
      vector->y = 1;
      break;

    case LINKS:
      vector->x = -1;
      vector->y = 0;
      break;

    case LINKSOBEN:
      vector->x = -1;
      vector->y = -1;
      break;


    default:
      DebugPrintf ("illegal direction in VectToDir() !");
      vector->x = vector->y = 0;
      return;
    }				/* switch */

}				/* DirToVect */

/*@Function============================================================
@Desc: my_sqrt:		Quadrat-Wurzel

@Ret: int
@Int:
* $Function----------------------------------------------------------*/
long
my_sqrt (long wert)
{
  long base = (long) wert - 1;
  long x;
  long res;
  long tmp = wert;
  int prec = 0;			/* Groessenordnung der Zahl */
  long verschiebung = 1;	/* Verschiebung zur Erhoehung der Genauigkeit */
  int counter;

  if (wert < 1)
    return (long) 1;

  while (tmp /= 10)
    prec++;
  prec = 4 - prec;		/* verschiebe auf mind. 4 stellen */
  if (prec < 0)
    prec = 0;

  counter = prec;
  while (counter--)
    verschiebung *= 10;

  x = base;
  x *= verschiebung;

  res = verschiebung + x / 2;
  x *= base;
  res -= x / 8;
  x *= base;
  res += x / 16;

  while (counter--)
    {
      if ((counter == 0) && ((res % 10) >= 5))
	res += 10;
      res /= 10;
    }


  return res;
}

/*@Function============================================================
@Desc: my_abs

@Ret: 
@Int:
* $Function----------------------------------------------------------*/
int
my_abs (int wert)
{
  return (wert < 0) ? -wert : wert;

}

/*@Function============================================================
@Desc: ShowDebugInfos()

@Ret: 
@Int:
* $Function----------------------------------------------------------*/
void
ShowDebugInfos (void)
{
  printf ("\n\n\n\n\n\n\n");
  printf ("Me.energy: %f \n", Me.energy);
  printf ("Me.pos: %f %f \n", Me.pos.x, Me.pos.y);
  getchar_raw ();
  return;
}

#undef _misc_c
