/*
	midi2gmn Library
	Copyright (C) 1992 - 2012 Juergen F. Kilian


	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

/*-----------------------------------------------------------------------
	filename: h_track.cpp
	author:   Juergen Kilian
	date:     1998-2001, 2011
	content:  implementation of THTRACK
-----------------------------------------------------------------------*/



//-----------------------------------------------------------------------
#include <iostream>
using namespace std;
#include <string>
#include <stack>
#include <string.h>


#include "funcs.h"
#include "q_note.h"
#include "h_track.h"

#include "anvoice.h"
#include "ornament.h"

#include "../lib_src/ini/ini.h"
#include "portable.h"

#include "funcs.h"

//-----------------------------------------------------------------------
// values will be read from .ini at TQMIDIFILE
// extern int EQUAL_TIME;
// extern int LEGATO_TIME;
// extern int STACCATO_TIME;
// extern int ORNAMENT_TIME; 
//-----------------------------------------------------------------------
/*
	factory function to create now notes
*/
/*
TNOTE *THTRACK::CreateNewNote(
	TFrac abstime,
	TFrac duration,
				unsigned char note,
				unsigned char intens )
{
	TQNOTE *temp;
	temp = new TQNOTE(abstime,
						  duration,
							note,
							intens);
	return temp;
}
*/
//-----------------------------------------------------------------------
void THTRACK::preQuantize( TTRACK *Track0,
							int ppq,
							int ORNAMENT_TIME)
{
	// mark all grace notes
	TInifile *tempIni = Parent()->getInifile();
	const char *val;
	val = tempIni->GetValChar ("ORNAMENT","OFF", "ornament detection [OFF|DETECT]");
	if( ppq > 0 ) // don't do for .gmn files
	{
		if( !strcmp(val, "DETECT") ||
			!strcmp(val, "ON") )
			markOrnament( Track0, ppq, ORNAMENT_TIME );
	}
	// write default 
	tempIni->GetValChar ("ORNAMENT_OUT","ON","[ON|OFF] Output of ornament tags \\grace, \\trill, ...");
}
//-----------------------------------------------------------------------
const char *ornamentString( int ornamentType )
{
	const char *ornString = NULL;
				switch( ornamentType )
				{
					case META_GRACE_AT_ONSET :
						ornString = ORNAMENT_GRACE;
						break;
						// slide onset(Now) to onset( Ornamentstart )
				/*
							OrnamentOnset = OrnamentStart->GetAbsTime();
							Now->SetDuration( Now->GetDuration() +
													Now->GetAbsTime() - OrnamentOnset );
							Now->SetAbsTime( OrnamentOnset );
					*/
					case META_GRACE_PRE_ONSET :
						ornString = ORNAMENT_GRACE;
						break;
					// slide onset(Ornamentstart) to onset(Now)
//							OrnamentOnset = Now->GetAbsTime();
					case META_GLISSANDO :
						ornString = ORNAMENT_GLISSANDO;
						break;
					case META_TRILL :
						ornString = ORNAMENT_TRILL;
						break;
					case META_MORDENT_DOWN :
						ornString = ORNAMENT_MORDENT;
						break;
					case META_MORDENT_UP :
						ornString = ORNAMENT_MORDENT;
						break;

//							OrnamentOnset = OrnamentStart->GetAbsTime();
							// delete all notes  |OrnamentStart ) Now
//							deleteInVoice( OrnamentStart, Now );
							// put tag to Now
//							Now->markOrnament(ornamentType);
/*							Now->SetDuration( Now->GetDuration() +
													Now->GetAbsTime() - OrnamentOnset );

							Now->SetAbsTime(OrnamentOnset);
*/
					case META_TURN :
						ornString = ORNAMENT_TURN;
						break;

							// slide onset(Ornamentstart) to onset(Now)
//							OrnamentOnset = Now->GetAbsTime();

					default :
//							OrnamentOnset = Now->GetAbsTime();
							ornString = ORNAMENT_UNDEF;
							break;
				} // switch
	return ornString;
}
/*
	Detect and mark ornaments, using k-nn for detection
	remarks:
		detected ornaments will be detached from note list and
		appended to base note
*/
void THTRACK::markOrnament( TTRACK *Track0,
							int ppq,
							double ORNAMENT_TIME)
{
	int i;
	int tempo;
	TQNOTE *Now,
	   		 *NextNote,
			 *OrnamentStart = NULL;
	long ms;
	int ornamentType;
	
	if( !Track0 )
	{
		Printf("THTRACK::markOrnament: Track == NULL!\n");
		return;
	}
//#undef _DEBUG_ORN
#define _DEBUG_ORN
	FILE *outFile = NULL;
#ifdef _DEBUG_ORN
	stringstream strOut;
#endif

	int cDetectedOrnaments = 0;
	for( i = 0; i<CVoice; i++ ) // check every voice in track
	{
#ifdef _DEBUG_ORN
	strOut << "voice "<< i << " -------ornduration "<<ORNAMENT_TIME <<" --------\n";
#endif

		Now  = QNOTE(FirstNoteObject( i ));
		if( Now )
			cout << "MarkOrnament: track "<< TrackNumber() <<
					", channel " << Channel() <<
					", voice " <<  i;

		// calc mean duration, and variance for all notes of voice
		double mean = 0, 
			   var = 0,
			   varLeft = 0,
			   min = 0,
			   cur,
			   max = 0;
		int count = 0;
		// calculate the mean
		while( Now )
		{
			count++;
			cur = Now->GetDuration().toDouble();
			mean += cur;
			Now = QNOTE(Now->GetNext(Now->GetVoice()));
			if( count == 1 )
			{
				min = cur;
				max = cur;
			}
			else if( cur > max )
			{
				max = cur;
			}
			else if( cur < min )
			{
				min = cur;
			}
		} // while 
		
		if( count > 1)
		{
			double cLeft = 0;
			mean /= count;
			// calc variance
			Now  = QNOTE(FirstNoteObject( i ));
			while( Now )
			{
				cur = Now->GetDuration().toDouble();
				var += pow(mean - cur, 2 );
				if( cur < mean )
				{
					varLeft += pow(mean - cur, 2 );
					cLeft++;
				}
				Now = QNOTE(Now->GetNext(Now->GetVoice()));
			}
			if( cLeft > 1 )
			{
				varLeft = sqrt( varLeft/(cLeft-1));
			}
			var = sqrt( var/(count-1));
		} // if Now
#ifdef _DEBUG_ORN
	strOut << "mean dur "<< mean << ", var "<< var << ", varLeft "<< varLeft << "\n";
#endif

		Now  = QNOTE(FirstNoteObject( i ));
		OrnamentStart = NULL;
		while( Now &&
			   Now->GetNext(Now->GetVoice()) )
		{
			NextNote = QNOTE(Now->GetNext(Now->GetVoice()));
// todo cleanup
			if(NextNote &&
				NextNote->GetVoice() != Now->GetVoice() )
						Printf("VoiceID Error");


			tempo = Track0->GetTempo( Now->GetAbsTime() );
			ms = DTIMEtoMS(ppq,
							tempo,
							Now->ioi(1, i).toLong() );

			double pDur = 0, 
				   pDelta = 0,
				   pMs = 0;

#ifdef _DEBUG_ORN
				Now->Debug( strOut ); 
				strOut << "dur_ms: "<<ms<<endl;
#endif

			if( ORNAMENT_TIME > mean )
				ORNAMENT_TIME = mean;
			if( Now->GetDuration().toDouble() < mean )
			{
				// use ornament time as variance
				pMs = GaussWindow(ms, ORNAMENT_TIME );
				// use mean as sigma
				pDur = 1 - GaussWindow(Now->GetDuration().toDouble(),
										mean, varLeft);

#ifdef _DEBUG_ORN
				strOut << "pMs: "<< pMs <<", pDur: "<< pDur <<", pMsxpDur "<<pMs * pDur <<"\n";
#endif
				if( pDur * pMs > 0.3 )
				{
					pMs = 0.6;
				}
				else
				{
					pMs = 0;
				}
				if( OrnamentStart ) // ornament has already started, prefer notes with similar length
				{
					// allow 10% deviation from ornament start duration
					pDelta = GaussWindow( log(OrnamentStart->GetDuration().toDouble() /
											  Now->GetDuration().toDouble()), 1.05 );
					if( pDelta > 0.9 )
						pMs = 0.6;
				}

			}
			if( pMs >= 0.6
				// ms < ORNAMENT_TIME 
				/* &&
// todo check chords in ornaments
					// no gap to next
				  ( !Now->GetNext(Now->GetVoice()) 
				   ||  // last note in this voice
					  Now->GetAbsTime() + Now->GetDuration() >=
					  Now->GetNext(Now->GetVoice())->GetAbsTime()
				)*/ 
				)
			{
				if( !OrnamentStart  ) // first Ornamented note
				{
	#ifdef _DEBUG_ORN
	 strOut << " -> xxxxx ornament start xxxxx pMS:"<< pMs <<", pDur:"<< pDur << "\n";
	#endif
						OrnamentStart = Now;
				}
			}
			else if( OrnamentStart ) // Now = 1st note after ornament
			// Now == first note > Ornament_TIME
			{
				// check Ornament type by knn
				ornamentType = retrieveOrnamentType( OrnamentStart,
												  Now,
												  strOut,
												  Parent()->getInifile());
	#ifdef _DEBUG_ORN
	strOut << "xxxxx ornament end == root xxxxx\n";
	#endif

				TOrnament *temp = NULL;
				const char *ornString;
				ornString = ornamentString( ornamentType );
				
				if( ornString )
				{
					cDetectedOrnaments++;
					// convert Now -> TOrnament, 
					// insert is done inside constructor
					if( ornamentType ==  META_TRILL )
					{
						// check pitch classes, stop at non trill classes
						// root note of trill MUST have same pitch like first ornamental note!
						int piStart, piStop;
						piStart = OrnamentStart->GetMIDIPitch();
						piStop = Now->GetMIDIPitch();
						if( piStart != piStop  )
						{

							// don't use long note as root!
							Now = QNOTE(Now->GetPrev( i ));
							if( Now == OrnamentStart )
								Now = QNOTE(Now->GetNext(i));
#ifdef _DEBUG_ORN
							strOut << "---- skip root backwards -----\n";
#endif
						}
					}
					temp = new TOrnament(Now,	// base note = 1st note after ornament
										OrnamentStart, // as played from = 1st ornament note
										Now,	// as played to = 1st note after ornament
										this,	// track
										ornString);
					NextNote = QNOTE((temp->GetNext(temp->GetVoice())));						
				}
				// reset values
				OrnamentStart = NULL;
				Now = NULL;
				if(temp) // avoid optimizing
				{
					temp->GetAbsTime();
				}
			} // else if Ornament Start
			Now = NextNote;
		} // while
	} // for
#ifdef _DEBUG_ORN
	outFile = fopen("_ornament.log","wt");
	fprintf(outFile, "%s", strOut.str().c_str());
	fclose(outFile);
#endif
	if(cDetectedOrnaments)
	{
		cout << cDetectedOrnaments <<" ornaments detected\n";
	}
}
//-----------------------------------------------------------------------

//!! all constants must be also defined in getParseVal
#define DIR_UNDEF 0
#define DIR_FUZZY 1
#define DIR_DOWN 2
#define DIR_STAY 3
#define DIR_ALTERNATE 4
#define DIR_UP 5

#define C_SINGLE 1
#define C_TWO 2
#define C_THREE 3
#define C_FOUR 4
#define C_MANY 5

#define DECRESC 1
#define CRESC 2

#define A_SMALL_UP 1
#define A_SMALL_DOWN 2
#define A_SMALL_ALTER 3
#define A_BIG_UP 4
#define A_BIG_DOWN 5
#define A_BIG_ALTER 6
int retrieveOrnamentType( TQNOTE *ornStart,
							TQNOTE *ornEnd,
							ostream &log,
							TInifile *inifile)
{

	if( !ornStart )
		return 0;

	int erg = 0;
	TQNOTE *temp;

	int prevPitch,
		 pitchDiff,
		 minPitch,
		 maxPitch;

	int count = 1,
		 direction = DIR_UNDEF;


	typedef  struct 
		{
			int count;  // single, two, less, many
			int direction;
			int gl_direction;
			int intens;  // -1: f-p, 0:f-f, 1:p-f
			int ambitus;
	} featureList;

	featureList ornFeatures;

	temp = ornStart;
	prevPitch = temp->GetMIDIPitch();
	minPitch = prevPitch;
	maxPitch = prevPitch;

	temp =  QNOTE(temp->GetNext(temp->GetVoice()));

	pitchDiff = ornEnd->GetMIDIPitch() - ornStart->GetMIDIPitch();
	if( pitchDiff == 0 )
		ornFeatures.gl_direction = DIR_STAY;
	else if( pitchDiff < 0 )
		ornFeatures.gl_direction = DIR_DOWN;
	else // > 0
		ornFeatures.gl_direction = DIR_UP;

	log << "pDiff "<< pitchDiff << "(" << ornFeatures.gl_direction <<") ";

	while( temp &&
			 temp != ornEnd )
	{
		pitchDiff = temp->GetMIDIPitch() - prevPitch;

		minPitch = min_( temp->GetMIDIPitch(), minPitch );
		maxPitch = max_( temp->GetMIDIPitch(), minPitch );
		// check pitch line
		if( pitchDiff < 0 )
		{
			if( direction == DIR_UNDEF )
				direction = DIR_DOWN;
			else
				direction = DIR_FUZZY;
		}
		else if( pitchDiff > 0 )
		{
			if( direction == DIR_UNDEF )
				direction = DIR_UP;
			else
				direction = DIR_FUZZY;
		}
		else // pitch Diff == 0
		{
			if( direction == DIR_UNDEF )
				direction = DIR_STAY;
			else
				direction = DIR_FUZZY;
		}
		count++;
		temp =  QNOTE(temp->GetNext(temp->GetVoice()));
	} // while
	ornFeatures.direction = direction;

	log << "dir:"<< direction  << " ";

	if( abs(maxPitch-minPitch) < 3 )
	{
		if( maxPitch <= ornStart->GetMIDIPitch() &&
			 maxPitch <= ornEnd->GetMIDIPitch() )
		{
			ornFeatures.ambitus = A_SMALL_DOWN;
		}
		else if( minPitch >= ornStart->GetMIDIPitch() &&
					minPitch >= ornEnd->GetMIDIPitch() )
		{
			ornFeatures.ambitus = A_SMALL_UP;
		}
		else
		{
			ornFeatures.ambitus = A_SMALL_ALTER;
		}
	}
	else
	{
		if( maxPitch <= ornStart->GetMIDIPitch() &&
			 maxPitch <= ornEnd->GetMIDIPitch() )
		{
			ornFeatures.ambitus = A_BIG_DOWN;
		}
		else if( minPitch >= ornStart->GetMIDIPitch() &&
					minPitch >= ornEnd->GetMIDIPitch() )
		{
			ornFeatures.ambitus = A_BIG_UP;
		}
		else
		{
			ornFeatures.ambitus = A_BIG_ALTER;
		}
	}
	log << "amb: "<< ornFeatures.ambitus;


	if( count == 1 )
	{
		ornFeatures.count = C_SINGLE;
	}
	else if( count == 2 )
	{
		ornFeatures.count = C_TWO;
	}
	else if( count == 4 )
	{
		ornFeatures.count = C_FOUR;
	}
	else if( count == 3 )
	{
		ornFeatures.count = C_THREE;
	}
	else
	{
		ornFeatures.count = C_MANY;
	}
	log << "count:"<< count <<"("<< ornFeatures.count << ") ";

	#define CFEATURES 5
	/*
			int count;  // single, two, less, many
			int direction;
			int gl_direction;
			int intens;  // DECRESC, CRESC
			int ambitus; // SMALL, BIG, UNDEF

			int
	*/
	int /*fGraceAt[CFEATURES+1] = {C_SINGLE,0,0,DECRESC,0,META_GRACE_AT_ONSET},
		 fGracePre[CFEATURES+1] = {C_SINGLE,0,0,CRESC,0,META_GRACE_PRE_ONSET},
		 fMordentUp[CFEATURES+1] = {C_TWO,DIR_UP,DIR_STAY,0,A_SMALL_UP,META_MORDENT_UP},
		 fMordentDown[CFEATURES+1] = {C_TWO,DIR_DOWN,DIR_STAY,0,A_SMALL_DOWN,META_MORDENT_DOWN},
		 fTrill[CFEATURES+1] = {C_MANY,0,0,0,A_SMALL_UP,META_TRILL},
		 fTurn[CFEATURES+1] = {C_FOUR,0,0,0,A_SMALL_ALTER,META_TURN},
		 fGlissUp[CFEATURES+1] = {C_MANY,DIR_UP,DIR_UP,0,A_BIG_UP,META_GLISSANDO},
		 fGlissDown[CFEATURES+1] = {C_MANY,DIR_DOWN,DIR_DOWN,0,A_BIG_DOWN,META_GLISSANDO},
		 */fOrnament[CFEATURES];

	int *tempFeatures;
	#define TEMPLATE_NUM 8
	int *templates[TEMPLATE_NUM];
	int i;
	i = 1;
	for( i=0; i < TEMPLATE_NUM; i++ )
	{
		tempFeatures = new int[CFEATURES+1];
		ReadFeatures(tempFeatures, i+1, inifile);
		templates[i] = tempFeatures;
	}

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	ornFeatures.intens = 0; // todo needs to be calculated from input

	fOrnament[0] = ornFeatures.count;
	fOrnament[1] = ornFeatures.direction;
	fOrnament[2] = ornFeatures.gl_direction;
	fOrnament[3] = ornFeatures.intens;
	fOrnament[4] = ornFeatures.ambitus;

	//								count, direction, gl_direction, intens, ambitus
	double weights[CFEATURES] = {2,	   // count
										  1,   //direction
										  1,	//gl_direction
										  1,	//intens
										  2},	// ambitus
			weightsGliss[CFEATURES] = {7,	   // count
										  1,   //direction
										  1,	//gl_direction
										  1,	//intens
										  7},	// ambitus
			weightsTurn[CFEATURES] = {10,	   // count
									  7,   //direction
									  7,	//gl_direction
									  0,	//intens
									  7};   // ambitus									

	/*
	 = {fGraceAt,
								fGracePre,
								fMordentUp,
								fMordentDown,
								fTrill,
								fTurn,
								fGlissUp,
								fGlissDown};
	*/
	double bestFit = MAXINT,
			 fit;
	{
				log << "\nprot: ";
				for( int ji = 0; ji < CFEATURES; ji++ )
				{
					log << (fOrnament)[ji] << " ";
				}
				log << "\n";
			} // if log

	int j;
	for( j = 0; j < TEMPLATE_NUM; j++ )
	{
		if( (templates[j])[CFEATURES] == META_TURN )
		{
			fit = CompareKNN( templates[j],
					fOrnament,
					weightsTurn,
					CFEATURES );
		}
		else if ( (templates[j])[CFEATURES] == META_GLISSANDO )
		{
			fit = CompareKNN( templates[j],
					fOrnament,
					weightsGliss,
					CFEATURES );
		}
		
		else
		{
			fit = CompareKNN( templates[j],
					fOrnament,
					weights,
					CFEATURES );
		}
			{
				log << fit << ": ";
				for( int ji = 0; ji < CFEATURES; ji++ )
				{
					log << (templates[j])[ji];
				}
				log << ornamentString((templates[j])[CFEATURES]) << endl;
			} // if log
		if( fit < bestFit )
		{
			bestFit = fit;
			erg = (templates[j])[CFEATURES];
		} // if < best fit
	} // for
	
	for( j = 0; j < TEMPLATE_NUM; j++ )
	{
		delete [] templates[j];
	}
		log << "erg = " << ornamentString(erg)  << endl;
	return erg;
}
//-----------------------------------------------------------------------

/*
	k-nn comparing of list1 and list2
	todo put compareKnn into global file
*/
double CompareKNN( int *list1,
					 int *list2,
					 double *weights,
					 int items )	// array size
{
	double res;
	int j;
	double diff;
	res = 0;

	for( j = 0; j < items; j++ )
	{
		if( list1[j] != 0 )
		{
			diff = pow(list1[j] - list2[j],2.0);
			res += (diff * weights[j] );
		}
	}
	res = sqrt(res);
	return res;
}
//-----------------------------------------------------------------------

/*
	delete a [from,to) in Voice(from)
*/
void THTRACK::deleteInVoice( TQNOTE *from, TQNOTE *to )
{
	if( !from )
		return;

	if( to &&	// to and needs to be in the same voice
		from->GetVoice() != to->GetVoice() )
		return;

	TQNOTE *prev,
			 *temp,
			 *next;

	while( from &&
			 from != to )
	{
		prev = QNOTE(from->GetPrev(-1/*all voices */));
		temp = QNOTE(from->GetNext(-1 /* all voices */));
// todo check deleteinvoice
		next = QNOTE(from->GetNext(from->GetVoice()));
		if( prev )
		{
			prev->SetNext( temp );
		}
		else
		{
			Notes = temp;
		}
		temp->SetPrev( prev );
		delete from;
		from = next;
	}
}
//-----------------------------------------------------------------------

/*
	read features of ornaments from .ini
*/
void ReadFeatures( int *features, int i, TInifile *tempIni)
{

	int j;
	char *ornament,
		  *current,
		  *valStr;

	ornament = new char[strlen(ORNAMENT_PREFIX)+5];

	//create entryname
	strcpy(ornament,ORNAMENT_PREFIX);
	sprintf(ornament+strlen(ornament),"%d",i);


	if( tempIni )
	{
		j = 0;
		const char *RefVal;
		const char *defVal;
		char	  *oldVal;
		switch(i)
		{
		case 1 : defVal = "(C_SINGLE,0,0,DECRESC,0,META_GRACE_AT_ONSET}";
			break;
		case 2 : defVal = "{C_SINGLE,0,0,CRESC,0,META_GRACE_PRE_ONSET}";
			break;
		case 3 : defVal = "{C_TWO,DIR_UP,DIR_STAY,0,A_SMALL_UP,META_MORDENT_UP}";
			break;
		case 4 : defVal = "{C_TWO,DIR_DOWN,DIR_STAY,0,A_SMALL_DOWN,META_MORDENT_DOWN}";
			break;
		case 5 : defVal = "(C_MANY,0,0,0,A_SMALL_UP,META_TRILL}";
			break;
		case 6 : defVal = "{C_FOUR,0,0,0,A_SMALL_ALTER,META_TURN}";
			break;
		case 7 : defVal = "{C_MANY,DIR_UP,DIR_UP,0,A_BIG_UP,META_GLISSANDO}";
			break;
		case 8 : defVal = "{C_MANY,DIR_DOWN,DIR_DOWN,0,A_BIG_DOWN,META_GLISSANDO}";
			break;
		default : defVal = "()";
		}
		RefVal = tempIni->GetValChar(ornament,defVal,"don't touch");
		if(RefVal)
		{
			valStr = new char[strlen(RefVal)+1];
			strcpy(valStr, RefVal);
			oldVal = valStr; // store for later delete
		}
		else
		{
			valStr = NULL;
			oldVal = NULL;
		}
		while(valStr &&
				j < CFEATURES+1)
		// Parse valStr
		{
			current = GetNextVal(valStr, &valStr);
			features[j] = parseVal(current);
			j++;
		}
		if(oldVal) // == valStr
			delete [] oldVal;
		if(j < CFEATURES+1)
		{
			Printf("H_TRACK:ReadFeatures: Syntax error fermata.ini!\n");
		}
	}
	delete [] ornament;
}
//-----------------------------------------------------------------------

/*
	get next value in feature string
*/
char *GetNextVal(char *valStr, char **tail)
{
			char *current;
			// skip (
			if( valStr[0] == '(' )
				valStr++;
			if( valStr[0] == '{' )
				valStr++;
			current = valStr;
			// search for next val
			valStr = strstr(valStr,",");
			if(valStr)
			// skip ','
			{
				*valStr = 0; // end of string
				valStr ++;
			}
			// skip ')'
			if(current &&
				(current[strlen(current)-1] == '}' ||
				current[strlen(current)-1] == ')'))
			{
				current[strlen(current)-1] = 0;
			}
			// skip ' ' at begin of current
			while(current &&
					current != 0 &&
					current[0] == ' ')
			{
				current++;
			}

			// skip ' ' at end of current
			while(current &&
					strlen(current) &&
					current[strlen(current)] == ' ')
			{
				current[strlen(current)] = 0;
			}

			*tail = valStr;

			return current;
}
//-----------------------------------------------------------------------

/*
	parse one value of a feature string
*/
int parseVal( char *valStr)
{
	int res;
	if(!strcmp("DIR_UNDEF",valStr))
	{
		res = DIR_UNDEF;
	}
	else if(!strcmp("DIR_FUZZY",valStr))
	{
		res = DIR_FUZZY;
	}
	else if(!strcmp("DIR_DOWN",valStr))
	{
		res = DIR_DOWN;
	}
	else if(!strcmp("DIR_STAY",valStr))
	{
		res = DIR_STAY;
	}
	else if(!strcmp("DIR_ALTERNATE ",valStr))
	{
		res = DIR_ALTERNATE;
	}
	else if(!strcmp("DIR_UP",valStr))
	{
		res = DIR_UP;
	}
	else if(!strcmp("C_SINGLE",valStr))
	{
		res = C_SINGLE;
	}
	else if(!strcmp("C_TWO",valStr))
	{
		res = C_TWO;
	}
	else if(!strcmp("C_THREE",valStr))
	{
		res = C_THREE;
	}
	else if(!strcmp("C_FOUR",valStr))
	{
		res = C_FOUR;
	}
	else if(!strcmp("C_MANY",valStr))
	{
		res = C_MANY;
	}
	else if(!strcmp("DECRESC",valStr))
	{
		res = DECRESC;
	}
	else if(!strcmp("CRESC",valStr))
	{
		res = CRESC;
	}
	else if(!strcmp("A_SMALL_UP",valStr))
	{
		res = A_SMALL_UP;
	}
	else if(!strcmp("A_SMALL_DOWN",valStr))
	{
		res = A_SMALL_DOWN;
	}
	else if(!strcmp("A_SMALL_ALTER",valStr))
	{
		res = A_SMALL_ALTER;
	}
	else if(!strcmp("A_BIG_UP",valStr))
	{
		res = A_BIG_UP;
	}
	else if(!strcmp("A_BIG_DOWN",valStr))
	{
		res = A_BIG_DOWN;
	}
	else if(!strcmp("A_BIG_ALTER",valStr))
	{
		res = A_BIG_ALTER;
	}
	else if(!strcmp("META_GRACE_AT_ONSET",valStr))
	{
		res = META_GRACE_AT_ONSET;
	}
	else if(!strcmp("META_GRACE_PRE_ONSET",valStr))
	{
		res = META_GRACE_PRE_ONSET;
	}
	else if(!strcmp("META_TRILL",valStr))
	{
		res = META_TRILL;
	}
	else if(!strcmp("META_GLISSANDO",valStr))
	{
		res = META_GLISSANDO;
	}
	else if(!strcmp("META_TURN",valStr))
	{
		res = META_TURN;
	}
	else if(!strcmp("META_MORDENT_UP",valStr))
	{
		res = META_MORDENT_UP;
	}
	else if(!strcmp("META_MORDENT_DOWN",valStr))
	{
		res = META_MORDENT_DOWN;
	}
	else
	{
		res = saveAtoi(valStr);
	}
	return res;
}
//-----------------------------------------------------------------------

#ifdef IMUTUS_DLL
THTRACK::THTRACK(MidiSeqPtr seq ) : TQTRACK(0)
{

	// Read Events from Midishare stream
	TMidiEv *curEv;

	curEv = seq->first;

	TNoteOnList *onOffList;
	onOffList = new TNoteOnList[128];
	// init table ------------------------------
	for ( int i = 0; i<128; i++ )
	{
		// NoteOnTones[i].Time = 0; 
		onOffList[i].Ptr  = NULL;
		onOffList[i].overhead  = NULL;
	};


	FILE *log = NULL;

	double bpm;
	char *str;
	while( curEv )
	{
		if( EvType(curEv) == typeKeyOn )
			addNoteOn( onOffList,
					   Date(curEv),
					   Pitch(curEv),
					   Vel(curEv),
					   Chan(curEv),
					   log);
		else if(EvType(curEv) == typeKeyOff )
			addNoteOff( onOffList,
					   Date(curEv),
					   Pitch(curEv),
					   Vel(curEv),
					   Chan(curEv),
					   log );
		else if(EvType(curEv) == typeTempo )
		{
			bpm = 60000000/Tempo(curEv);
			addSetTempo( onOffList,
					   Date(curEv),
					   bpm,
					   log );
		}
		else if(EvType(curEv) == typeTimeSign ) 
			addTimeSig( onOffList,
					   Date(curEv),
					   TSNum(curEv),
					   TSDenom(curEv),
					   log );
		else if(EvType(curEv) == typeKeySign )
			addKeySig( onOffList,
					   Date(curEv),
					   KSTon(curEv),
					   KSMode(curEv),
					   log );

		else if(EvType(curEv) == typeSeqName /*test*/ )
		{
			addText( onOffList,
					   Date(curEv),
					   str,
					   Chan(curEv),
					   log);
		}

		curEv = Link(curEv);

	}
	delete [] onOffList;
}
#endif

/// create a attack, duration, ioi ms table for each note
void THTRACK::toGLN(const char *filename,
					int ppq,
					TTRACK *ctrlTrack)
{
	FILE *out;
	out = fopen( filename, "at");
	int i;
	for( i = 0; i < cVoice(); i++ )
	{
		TNOTE *note;
		long int durTick, 
				 IOITick,
				 atTick,
				 atMS,
				 durMS,
				 IOIMS;
		long int ctempo;
		double IOIratio;
		note = FirstNoteObject(i);
		while( note )
		{
			atTick = note->GetAbsTime().toLong();
			durTick = note->GetDuration().toLong();
			IOITick = note->ioi(1,i).toLong();
			ctempo = ctrlTrack->GetTempo(note->GetAbsTime() );
			atMS = DTIMEtoMS(ppq,
							  ctempo,
							  atTick);
			durMS = DTIMEtoMS(ppq,
							  ctempo,
							  durTick);
			IOIratio = note->IOIratio(-1,1,note->GetVoice());
			if( IOIratio < 0 )
				IOIratio = -1 / IOIratio;
			if( IOITick <= 0 )
				IOIMS = -1;
			else
				IOIMS = DTIMEtoMS(ppq,
							  ctempo,
							  IOITick);
//			fprintf(out, "%f, %f, %f\n", 
			int mPitch = note->GetMIDIPitch();
			fprintf(out, "%ld, %d, %ld, %ld, %.3f\n", 
						atMS, mPitch, durMS, IOIMS, IOIratio);
			note = dynamic_cast<TNOTE *>(note->GetNext(i));
		}
	}
	fclose( out );
}

int THTRACK::deleteClicks(int channel, int clickType)
{
	int count = 0;
	TMusicalObject *temp = this->FirstObject(-1);
	while( temp )
	{
		TMusicalObject *rmPtr = NULL;
		TNOTE *note = dynamic_cast<TNOTE *>(temp);
		if( note && 
			clickType < 128)
		{
			if( channel < 1 || 
				channel == ChannelI )
			{
				if( clickType < 0 ||
					clickType > 127 ||
					clickType == note->GetMIDIPitch() )
				{
					rmPtr = note;
				}
			} // if channel
		} // if clicktype
		temp = temp->TMusicalObject::GetNext(-1);
		if( rmPtr )
		{
			DetachedVoice(rmPtr, temp);
			delete rmPtr;
			count++;
		}
	} // while notes 
	return count;
}
