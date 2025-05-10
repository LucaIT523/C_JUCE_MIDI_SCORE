/*-----------------------------------------------------------------
|	filename : DEFS.H
|	Author     : Juergen Kilian
|	Date	    : 17.10.1996-01
|	global constants, variables and typedefinitons
------------------------------------------------------------------*/
#ifndef __defs_h__
#define __defs_h__


#include <stdio.h>

// todo put defines in .ini
// Default value for upbeat recognition
#define BEATINTENS 100

/* reaction time for first metronom click */
#define RTIME 180

/* max time [ms] between two notes starting at the same time*/
#define EQUALTIME 50

/* deafult for relative duration */
#define DEFDURATION 100

/* search window for attackpoints */
// = 1/16
//#define SEARCHATTACK (( ppq * 4 ) / SECH10TEL)
#define SEARCHATTACK (TFrac(1,16))

/* search window for durations*/
// = 1/8
//#define SEARCHLENGTH (( ppq * 4 ) / ACHTEL_T)
#define SEARCHLENGTH (TFrac(1,8))

// search window for patternsearch
// = 1/8.
#define DELTALENGTH  ((ppq * 3 ) /4)


// [bpm]
#define MAXTEMPO 300
#define MINTEMPO 40

// Filenames ----------------------------------------------
#define DEF_MASKFILE "IOIList.gmn"
#define DEF_PATTFILE "qpatternbase.gmn"
#define DEF_LOG_NAME "midi2gmn.log"
//extern char *LOG_NAME;
// extern FILE *LOG_FILE;


//-----------------------------------------------------------------
/* #durations */
/*
#define CNOTENWERTE 12

#define GANZE 1
#define HALBE 2
#define HALBE_T 3
#define VIERTEL 4
#define VIERTEL_T 6
#define ACHTEL 8
#define ACHTEL_T 12
#define SECH10TEL 16
#define SECH10TEL_T 24
#define ZWEIUND30TEL 32
#define ZWEIUND30TEL_T 48
#define VIERUND60TEL 64
*/
//-----------------------------------------------------------------
#include "fragm.h"
//typedef long TAbsTime;
#define TAbsTime TFrac
typedef TFrac *TFracDiff;
#if defined( Windows )
		#undef printf
#endif
#endif
