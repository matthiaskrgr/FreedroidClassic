/*----------------------------------------------------------------------
 *
 * Desc: miscellaeous helpful functions for Freedroid
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


// The definition of the message structure can stay here,
// because its only needed in this module.
typedef struct
{
  void *NextMessage;
  int MessageCreated;
  char *MessageText;
}
message, Message;
#define MESPOSX 0
#define MESPOSY 64
#define MESHOEHE 8
#define MESBARBREITE 320
#define MAX_MESSAGE_LEN 100
#define MESBAR_MEM MESBARBREITE*MESHOEHE+1000

void CreateMessageBar (char *MText);
void AdvanceQueue (void);

unsigned char *MessageBar;
message *Queue = NULL;
// int ThisMessageTime=0;               /* Counter fuer Message-Timing */

struct timeval now, oneframetimestamp, tenframetimestamp,
  onehundredframetimestamp, differenz;
long oneframedelay = 0;
long tenframedelay = 0;
long onehundredframedelay = 0;
float FPSover1 = 10;
float FPSover10 = 10;
float FPSover100 = 10;
Uint32 Now_SDL_Ticks;
Uint32 One_Frame_SDL_Ticks;
Uint32 Ten_Frame_SDL_Ticks;
Uint32 Onehundred_Frame_SDL_Ticks;
int framenr = 0;

/*@Function============================================================
@Desc: realise Pause-Mode: the game process is halted,
       while the graphics and animations are not.  This mode 
       can further be toggled from PAUSE to CHEESE, which is
       a feature from the original program that should probably
       allow for better screenshots.
       
@Ret: 
* $Function----------------------------------------------------------*/
void
Pause (void)
{
  int Pause = TRUE;

  Activate_Conservative_Frame_Computation();

  Me.status = PAUSE;
  Assemble_Combat_Picture ( DO_SCREEN_UPDATE );

  while ( Pause )
    {
      // usleep(10);
      AnimateInfluence ();
      AnimateRefresh ();
      RotateBulletColor ();
      AnimateEnemys ();
      DisplayBanner (NULL, NULL, 0);
      Assemble_Combat_Picture ( DO_SCREEN_UPDATE );
      
      if (CPressed ())
	{
	  Me.status = CHEESE;
	  DisplayBanner (NULL, NULL,  0 );
	  Assemble_Combat_Picture ( DO_SCREEN_UPDATE );

	  while (!SpacePressed ()); /* stay CHEESE until Space pressed */
	  while ( SpacePressed() ); /* then wait for Space released */
	  
	  Me.status = PAUSE;       /* return to normal PAUSE */
	} /* if (CPressed) */

      if ( SpacePressed() )
	{
	  Pause = FALSE;
	  while ( SpacePressed() );  /* wait for release */
	}

    } /* while (Pause) */

  return;

} /* Pause () */


/*@Function============================================================
@Desc: This function starts the time-taking process.  Later the results
       of this function will be used to calculate the current framerate

       Two methods of time-taking are available.  One uses the SDL 
       ticks.  This seems LESS ACCURATE.  The other one uses the
       standard ansi c gettimeofday functions and are MORE ACCURATE
       but less convenient to use.
@Ret: 
* $Function----------------------------------------------------------*/
void 
StartTakingTimeForFPSCalculation(void)
{
  /* This ensures, that 0 is never an encountered framenr,
   * therefore count to 100 here
   * Take the time now for calculating the frame rate
   * (DO NOT MOVE THIS COMMAND PLEASE!) */
  framenr++;
  
#ifdef USE_SDL_FRAMERATE
  One_Frame_SDL_Ticks=SDL_GetTicks();
  if (framenr % 10 == 1)
    Ten_Frame_SDL_Ticks=SDL_GetTicks();
  if (framenr % 100 == 1)
    {
      Onehundred_Frame_SDL_Ticks=SDL_GetTicks();
      // printf("\n%f",1/Frame_Time());
      // printf("Me.pos.x: %g Me.pos.y: %g Me.speed.x: %g Me.speed.y: %g \n",
      //Me.pos.x, Me.pos.y, Me.speed.x, Me.speed.y );
      //printf("Me.maxspeed.x: %g \n",
      //	     Druidmap[Me.type].maxspeed );
    }
#else
  gettimeofday (&oneframetimestamp, NULL);
  if (framenr % 10 == 1)
    gettimeofday (&tenframetimestamp, NULL);
  if (framenr % 100 == 1)
    {
      gettimeofday (&onehundredframetimestamp, NULL);
      printf("\n%f",1/Frame_Time());
    }
#endif
  
} // void StartTakingTimeForFPSCalculation(void)


/*@Function============================================================
@Desc: This function computes the framerate that has been experienced
       in this frame.  It will be used to correctly calibrate all 
       movements of game objects.

       NOTE:  To query the actual framerate a DIFFERENT function must
       be used, namely Frame_Time().

       Two methods of time-taking are available.  One uses the SDL 
       ticks.  This seems LESS ACCURATE.  The other one uses the
       standard ansi c gettimeofday functions and are MORE ACCURATE
       but less convenient to use.
@Ret: 
* $Function----------------------------------------------------------*/
void 
ComputeFPSForThisFrame(void)
{

  // In the following paragraph the framerate calculation is done.
  // There are basically two ways to do this:
  // The first way is to use SDL_GetTicks(), a function measuring milliseconds
  // since the initialisation of the SDL.
  // The second way is to use gettimeofday, a standard ANSI C function I guess,
  // defined in time.h or so.
  // 
  // I have arranged for a definition set in defs.h to switch between the two
  // methods of ramerate calculation.  THIS MIGHT INDEED MAKE SENSE, SINCE THERE
  // ARE SOME UNEXPLAINED FRAMERATE PHENOMENA WHICH HAVE TO TO WITH KEYBOARD
  // SPACE KEY, SO PLEASE DO NOT ERASE EITHER METHOD.  PLEASE ASK JP FIRST.
  //

#ifdef USE_SDL_FRAMERATE

  Now_SDL_Ticks=SDL_GetTicks();
  oneframedelay=Now_SDL_Ticks-One_Frame_SDL_Ticks;
  tenframedelay=Now_SDL_Ticks-Ten_Frame_SDL_Ticks;
  onehundredframedelay=Now_SDL_Ticks-Onehundred_Frame_SDL_Ticks;
  
  FPSover1 = 1000 * 1 / (float) oneframedelay;
  FPSover10 = 1000 * 10 / (float) tenframedelay;
  FPSover100 = 1000 * 100 / (float) onehundredframedelay;
  
#else
  
  gettimeofday (&now, NULL);
  
  oneframedelay =
    (now.tv_usec - oneframetimestamp.tv_usec) + (now.tv_sec -
						 oneframetimestamp.
						 tv_sec) * 1000000;
  if (framenr % 10 == 0)
    tenframedelay =
      ((now.tv_usec - tenframetimestamp.tv_usec)) + (now.tv_sec -
						     tenframetimestamp.
						     tv_sec) *
      1000000;
  if ((framenr % 100) == 0)
    {
      onehundredframedelay =
	(now.tv_sec - onehundredframetimestamp.tv_sec) * 1000000 +
	(now.tv_usec - onehundredframetimestamp.tv_usec);
      framenr = 0;
    }
  
  FPSover1 = 1000000 * 1 / (float) oneframedelay;
  FPSover10 = 1000000 * 10 / (float) tenframedelay;
  FPSover100 = 1000000 * 100 / (float) onehundredframedelay;
  
#endif
  
  
} // void ComputeFPSForThisFrame(void)

/*@Function============================================================
  @Desc: 

 * This function is the key to independence of the framerate for various game elements.
 * It returns the average time needed to draw one frame.
 * Other functions use this to calculate new positions of moving objects, etc..
 *

 * Also there is of course a serious problem when some interuption occurs, like e.g.
 * the options menu is called or the debug menu is called or the console or the elevator
 * is entered or a takeover game takes place.  This might cause HUGE framerates, that could
 * box the influencer out of the ship if used to calculate the new position.

 * To counter unwanted effects after such events we have the SkipAFewFramerates counter,
 * which instructs Rate_To_Be_Returned to return only the overall default framerate since
 * no better substitute exists at this moment.  But on the other hand, this seems to
 * work REALLY well this way.

 * This counter is most conveniently set via the function Activate_Conservative_Frame_Computation,
 * which can be conveniently called from eveywhere.

@Ret: 
@Int:
* $Function----------------------------------------------------------*/
float
Frame_Time (void)
{
  float Rate_To_Be_Returned;
  
  if ( SkipAFewFrames ) 
    {
      Rate_To_Be_Returned = Overall_Average;
      return Rate_To_Be_Returned;
    }

  Rate_To_Be_Returned = (1.0 / FPSover1);

  return Rate_To_Be_Returned;

} // float Frame_Time(void)

/*@Function============================================================
@Desc: 

 * With framerate computation, there is a problem when some interuption occurs, like e.g.
 * the options menu is called or the debug menu is called or the console or the elevator
 * is entered or a takeover game takes place.  This might cause HUGE framerates, that could
 * box the influencer out of the ship if used to calculate the new position.

 * To counter unwanted effects after such events we have the SkipAFewFramerates counter,
 * which instructs Rate_To_Be_Returned to return only the overall default framerate since
 * no better substitute exists at this moment.

 * This counter is most conveniently set via the function Activate_Conservative_Frame_Computation,
 * which can be conveniently called from eveywhere.

@Ret: 
@Int:
* $Function----------------------------------------------------------*/
void 
Activate_Conservative_Frame_Computation(void)
{
  // SkipAFewFrames=212;
  // SkipAFewFrames=22;
  SkipAFewFrames=3;

  // Now we are in some form of pause.  It can't
  // hurt to have the top status bar redrawn after that,
  // so we set this variable...
  BannerIsDestroyed=TRUE;

} // void Activate_Conservative_Frame_Computation(void)


/*@Function============================================================
@Desc: This function is used for debugging purposes.  It writes the
       given string either into a file, on the screen, or simply does
       nothing according to currently set debug level.

@Ret: none
* $Function----------------------------------------------------------*/
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

/*@Function============================================================
@Desc: This function is used for debugging purposes.  It writes the
       given float either into a file, on the screen, or simply does
       nothing according to currently set debug level.

@Ret: none
* $Function----------------------------------------------------------*/
void
DebugPrintfFloat (float Print_Float)
{
  FILE *debugfile;

  if (debug_level == 0) return;

  debugfile = fopen ("DEBUG.OUT", "a");

  fprintf (debugfile, "%f", Print_Float);
  fclose (debugfile);
};

/*@Function============================================================
@Desc: This function is used for debugging purposes.  It writes the
       given int either into a file, on the screen, or simply does
       nothing according to currently set debug level.

@Ret: none
* $Function----------------------------------------------------------*/
void
DebugPrintfInt (int Print_Int)
{
  FILE *debugfile;

  if (debug_level == 0) return;

  debugfile = fopen ("DEBUG.OUT", "a");

  fprintf (debugfile, "%d", Print_Int);
  fclose (debugfile);
};

/*@Function============================================================
@Desc: This function is used to generate an integer in range of all
       numbers from 0 to UpperBound.

@Ret:  the generated integer
* $Function----------------------------------------------------------*/
int
MyRandom (int UpperBound)
{
  float tmp;
  int PureRandom;
  int dice_val;    /* the result in [0, Obergrenze] */

  PureRandom = rand ();
  tmp = 1.0*PureRandom/RAND_MAX; /* random number in [0;1] */

  /* 
   * we always round OFF for the resulting int, therefore
   * we first add 0.99999 to make sure that Obergrenze has
   * roughly the same probablity as the other numbers 
   */
  dice_val = (int)( tmp * (1.0 * UpperBound + 0.99999) );
  return (dice_val);
} /* MyRandom () */


/*@Function============================================================
@Desc: This function is used to revers the order of the chars in a
       given string.

@Ret:  none
* $Function----------------------------------------------------------*/
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


/*@Function============================================================
@Desc: This function is used to transform an integer into an ascii
       string that can then be written to a file.

@Ret:  the given pointer to the string.
* $Function----------------------------------------------------------*/
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
}// void itoa(int n, char s[]) siehe Kernighan&Ritchie!

/*@Function============================================================
@Desc: This function is used to transform a long into an ascii
       string that can then be written to a file.

@Ret:  the given pointer to the string.
* $Function----------------------------------------------------------*/
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
} // void ltoa(long n, char s[]) angelehnt an itoa!

/*@Function============================================================
@Desc: This function is kills all enemy robots on the whole ship.
       It querys the user once for safety.

@Ret:  none
* $Function----------------------------------------------------------*/
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
	AllEnemys[i].energy = 0;
	AllEnemys[i].Status = OUT;
      }
} // void Armageddon(void)

/*@Function============================================================
@Desc: This function teleports the influencer to a new position on the
       ship.  THIS CAN BE A POSITION ON A DIFFERENT LEVEL.

@Ret:  none
* $Function----------------------------------------------------------*/
void
Teleport (int LNum, int X, int Y)
{
  int curLevel = LNum;
  int array_num = 0;
  Level tmp;
  int i;

  if (curLevel != CurLevel->levelnum)
    {	

      //--------------------
      // In case a real level change has happend,
      // we need to do a lot of work:

      while ((tmp = curShip.AllLevels[array_num]) != NULL)
	{
	  if (tmp->levelnum == curLevel)
	    break;
	  else
	    array_num++;
	}

      CurLevel = curShip.AllLevels[array_num];

      ShuffleEnemys ();

      Me.pos.x = X;
      Me.pos.y = Y;

      // turn off all blasts and bullets from the old level
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
      //--------------------
      // If no real level change has occured, everything
      // is simple and we just need to set the new coordinates, haha
      //
      Me.pos.x = X;
      Me.pos.y = Y;
    }

  LeaveLiftSound ();

  UnfadeLevel ();

} /* Teleport() */


/*@Function============================================================
@Desc: This is a test function for InsertMessage()

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
@Desc: 	This function is used for terminating freedroid.  It will close
        the SDL submodules and exit.

@Ret: 
* $Function----------------------------------------------------------*/
void
Terminate (int ExitCode)
{
  DebugPrintf ("\nvoid Terminate(int ExitStatus) wurde aufgerufen....");
  printf("\n----------------------------------------------------------------------\nTermination of Freedroid initiated... \nUnallocation all resouces...");

  // free the allocated surfaces...
  // SDL_FreeSurface( ne_blocks );
  // SDL_FreeSurface( ne_static );

  // free the mixer channels...
  // Mix_CloseAudio();

  printf("\nAnd now the final step...\n\n");
  SDL_Quit();
  exit (ExitCode);
  return;
}  // void Terminate(int ExitCode)


/*@Function============================================================
@Desc: This function empties the message queue of messages to be
       displayed in a moving font on screen.

@Ret: none
* $Function----------------------------------------------------------*/
void
KillQueue (void)
{
  while (Queue)
    AdvanceQueue ();
}

/*@Function============================================================
@Desc: This functin deletes the currently displayed message and
       advances to the next message.

@Ret: none
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
} // void AdvanceQueue(void)


/*@Function============================================================
@Desc: This function should put a message from the queue to the scren.
       It surely does not work now and it also needs not work now, 
       since this is a feature not incorporated into the original game
       from the C64 and therefore has less priority.

@Ret: 
* $Function----------------------------------------------------------*/
void
PutMessages (void)
{
  static int MesPos = 0;	// X-position of the message bar 
  static int Working = FALSE;	// is a message beeing processed? 
  message *LQueue;		// mobile queue pointer
  int i;

  if (!PlusExtentionsOn)
    return;

  DebugPrintf ("\nvoid PutMessages(void): Funktion wurde echt aufgerufen.");

  if (!Queue)
    return;			// nothing to be done
  if (!Working)
    ThisMessageTime = 0;	// inactive, but Queue->reset time

  printf ("Time: %d", ThisMessageTime);

// display the current list:

  LQueue = Queue;
  i = 0;
  DebugPrintf ("\nvoid PutMessages(void): This is the Queue of Messages:\n");
  while (LQueue != NULL)
    {
      if ((LQueue->MessageText) == NULL)
	{
	  DebugPrintf
	    ("\nvoid PutMessages(void): ERROR: Textpointer is NULL !!!!!!\n");
	  Terminate(ERR);
	}
      printf ("%d. '%s' %d\n", i, LQueue->MessageText,
	      LQueue->MessageCreated);
      i++;
      LQueue = LQueue->NextMessage;
    }
  DebugPrintf (" NULL reached !\n");

  // if the message is very old, it can be deleted...
  if (Working && (ThisMessageTime > MaxMessageTime))
    {
      AdvanceQueue ();
      //      CleanMessageLine ();
      Working = FALSE;		// inactive
      ThisMessageTime = 0;	// Counter init.
      return;
    }


  // old message has lived for MinTime, new one is waiting
  if ((ThisMessageTime > MinMessageTime) && (Queue->NextMessage))
    {
      AdvanceQueue ();		/* Queue weiterbewegen */
      Working = FALSE;		/* inaktiv setzen */
      ThisMessageTime = 0;	/* counter neu init. */
      return;
    }

  // function currenlty inactive and new message waiting --> activate it
  if ((!Working) && Queue)
    {

      // if message not yet generated, generate it
      if (!Queue->MessageCreated)
	{
	  CreateMessageBar (Queue->MessageText);
	  Queue->MessageCreated = TRUE;
	}

      ThisMessageTime = 0;	/* initialize counter  */
      //      CleanMessageLine ();	/* delete line */
      Working = TRUE;		/* activated */
    }

  // function currently inactive --> move and display
  if (Working && Queue)
    {

      MesPos = 10 * ThisMessageTime;	/* move synchronized this time */

      /* don't go beyond the left border!! */
      if (MesPos > (MESBARBREITE - 2))
	MesPos = MESBARBREITE - 2;

      for (i = 0; i < MESHOEHE; i++)
	{
	  ;
	}
    }	/* if aktiv + Message there */
}	/* Put Messages */


/*@Function============================================================
@Desc: This function prepares the graphics for the next message to
       be displayed

@Ret: 
* $Function----------------------------------------------------------*/
void
CreateMessageBar (char *MText)
{
  char Worktext[42];
  int i, j;

  DebugPrintf
    ("\nvoid CreateMessageBar(char* MText): real function call confirmed.");

  // check for too long message
  if (strlen (MText) > 40)
    {
      DebugPrintf
	("\nvoid CreateMessageBar(char* MText): Message hat mehr als 40 Zeichen !.\n");
      Terminate (ERR);
    }

  // allocate memory if this hasn't happened yet
  if (MessageBar == NULL)
    if ((MessageBar = MyMalloc (MESBAR_MEM)) == NULL)
      {
	DebugPrintf
	  ("\nvoid CreateMessageBar(char* MText): Bekomme keinen Speicher fuer MessageBar !!\n");
	Terminate (ERR);
      }

  // fill in spaces to get 40 chars as message length
  strcpy (Worktext, MText);
  while (strlen (Worktext) < 40)
    strcat (Worktext, " ");

  // display the current message to the internal screen and then cut it out from there
  for (i = 0; i < 40; i++)
    {
      for (j = 0; j < 8; j++)
	{
	  memcpy ((MessageBar + i * 8 + j * SCREENBREITE),
		  (Data70Pointer + Worktext[i] * 8 * 8 + j * 8), 8);
	}
    }

  DebugPrintf
    ("\nvoid CreateMessageBar(char* MText): end of function reached.");
} // void CreateMessageBar(char* MText)

/*@Function============================================================
@Desc: This function insers a new message for the user to be 
       displayed into the message queue

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
      // move to the next free position in the message queue
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
} // void InsertMessage(char* MText)

/*@Function============================================================
@Desc: This function works a malloc, except that it also checks for
       success and terminates in case of "out of memory", so we dont
       need to do this always in the code.

@Ret: 
* $Function----------------------------------------------------------*/
void *
MyMalloc (long Mamount)
{
  void *Mptr = NULL;

  if ((Mptr = malloc ((size_t) Mamount)) == NULL)
    {
      printf (" MyMalloc(%ld) did not succeed!\n", Mamount);
      Terminate(ERR);
    }

  return Mptr;
}				// void* MyMalloc(long Mamount)

#undef _misc_c
