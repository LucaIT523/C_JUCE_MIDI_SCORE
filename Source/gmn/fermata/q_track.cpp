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

/*------------------------------------------------------------------
 |	Filename   : TQRACK2.CPP
 |	Author     : Juergen Kilian
 |	Date	   : 17.10.1996-98-2003,2011
 | 	 	 	 	 10/2010, 2011 refactoring
 |	Contents   : Implementation of TQTRACK
 |	remark: don't use old qtrack.cpp
 ------------------------------------------------------------------*/
#include <iostream>
using namespace std;
#include <sstream>
#include <stack>
#include <string.h>

#include "portable.h"
#include "import.h" // yesnoquestion
#include <stdio.h>

#include "tags.h"

#include "track.h"
#include "q_track.h"
#include "q_note.h"
#include "h_track.h"
#include "q_funcs.h"
#include "funcs.h"
#include "event.h"

#include "correl.h"
#include "anvoice.h"
#include "project.h"
#include "question.h"
#include "key.h"
#include "statist.h"

#include "voiceinfo.h"

#include "../lib_src/ini/ini.h"

#include "meta_tempo.h"
#include "q_chord.h"

// #define _DEBUG_VSEP
/*
 extern int cAttacks;
 extern int cDurations;
 */
//-----------------------------------------------------------------------

/// values will be re-read from .ini at TQMIDIFILE
//int STACCATO_TIME=0;
// int ORNAMENT_TIME=0; 
//-----------------------------------------------------------------------
#undef UNUSED

#define denomLSize 17
int denomList[denomLSize] = { 1, 2, 4, 8, 16, 32, 3, 6, 12, 24, 48, 5, 7, 11,
		64, 20, 28 };

// todo: put following in .ini !!!
/*! #Notes in one line of output file */
#define MAXNOTEINLINE 6
//#define MAX_EVENT_IN_LINE 3
//! max interval in one voice
// #define MAX_INTERVAL 10

//----------------------------------------------------------------------
TQTRACK::~TQTRACK(void) {
	DeleteTags(Tags);
}
;
//----------------------------------------------------------------------
TNOTE * TQTRACK::CreateNewNote(lgEvent *event) {
	TNOTE *res = NULL;
	// switch event type
	lgNote *tNote = dynamic_cast<lgNote *> (event);
	if (tNote) {
		TFrac dummy(1, 4);
		res = CreateNewNote(dummy, dummy, 0, 0);
		res->fill(tNote);
	} else {
		lgChord * tChord = dynamic_cast<lgChord *> (event);
		if (tChord) {
			res = CreateNewChord(tChord);
		}
	}
	return res;
}
TQChord * TQTRACK::CreateNewChord(lgChord *chord) {
	lgVoice *tVoice = chord->firstVoice();
	lgNote *tNote = tVoice->firstNote();

	TQChord *res = new TQChord(new TQNOTE(tNote), NULL, NULL, NULL);

	tVoice = dynamic_cast<lgVoice *> (tVoice->next());
	while (tVoice) {
		tNote = tVoice->firstNote();
		res->Add(new TQNOTE(tNote), NULL, 0);
		tVoice = dynamic_cast<lgVoice *> (tVoice->next());

	}
	return res;
}

/// append the lgNote, or chord, ignore rests
void TQTRACK::append(lgEvent *event) {
	TNOTE *curNote = CreateNewNote(event);
	if (curNote)
		Insert(curNote);
}

int readLNote(lgTag *tag, double &attack, double &dur, double &sustainDur,
/// scoretime with 96ppq
		long int &st, int &pitch, int &vel, int &v, int &bar, int &beat) {
	int res = 0;
	if (!strcmp(tag->name().c_str(), "\\note")) {
		res = 1;
		attack = -1;
		sustainDur = -1;
		dur = -1;
		vel = -1;
		pitch = -1;
		bar = -1;
		beat = -1;
		v = -1;
		for (int i = 1; i <= tag->cArgs(); i++) {

			double val = -1;
			lgTagArg *tArg = tag->getArg(i);
			lgFloatTagArg *tFArg = dynamic_cast<lgFloatTagArg *> (tArg);
			lgIntTagArg *tIArg = dynamic_cast<lgIntTagArg *> (tArg);
			if (tFArg) {
				val = tFArg->valFloat();
			} else if (tIArg) {
				val = tIArg->valInt();
			}
			// const char *argName = tArg->name().c_str();
			if (!strcmp(tArg->name().c_str(), "a"))
				attack = val;
			else if (!strcmp(tArg->name().c_str(), "d"))
				dur = val;
			else if (!strcmp(tArg->name().c_str(), "i"))
				vel = (double) val;
			else if (!strcmp(tArg->name().c_str(), "p"))
				pitch = (double) val;
			else if (!strcmp(tArg->name().c_str(), "sd"))
				sustainDur = val;
			else if (!strcmp(tArg->name().c_str(), "voice"))
				v = (double) val;
			else if (!strcmp(tArg->name().c_str(), "beat"))
				beat = (double) val;
			else if (!strcmp(tArg->name().c_str(), "bar"))
				bar = (double) val;
			else if (!strcmp(tArg->name().c_str(), "st"))
				st = val;
		} // for
	}// if
	return res;
}

TNOTE *TQTRACK::append(lgTag *tag, int voice, int &maxVoice) {

	int res = 0;
	double attack = 0, sustainDur = 0, dur = 0;
	int vel = 0, pitch = 0;
	long st;
	int bar, beat, v;
	TNOTE *curNote = NULL;

	if (readLNote(tag, attack, dur, sustainDur,
	/// scoretime with 96parts per quarter!!
			st, pitch, vel, v, bar, beat) && attack >= 0 && dur > 0) {
		if (v == voice || voice < 0) {
			res++;
			// normalise to 960 ppq
			attack = attack * 960.0 + 0.5;
			dur = dur * 960.0 + 0.5;
			TFrac dF = TFrac((long) dur, 1L);
			TFrac aF = TFrac((long) attack, 1L);
			unsigned char p, intens;
			p = pitch;
			intens = vel;
			curNote = CreateNewNote(aF, dF, pitch, intens);
			if (curNote) {
				curNote->createTag = tag;
				Insert(curNote);
				if (sustainDur > 0) {
					sustainDur = 960 * sustainDur + 0.5;
					curNote->sustainOffset = (double) (attack + sustainDur
							+ dur);
				}
			} // curNote
		} // if voice
		else if (v > maxVoice) {
			maxVoice = v;
		}
	} // if read
	return curNote;
}
/*!
 return an array with the ranking of each voice
 sort key: av-pitch of first 4 notes in voice
 !! result must be deleted
 */
int * TQTRACK::sortVoices(void) {
	int *sortedVoices = new int[CVoice + 1];
	int *pitch = new int[CVoice + 1];

	// init arrays
	int i;
	for (i = 0; i < CVoice; i++) {
		sortedVoices[i] = i;
		// get av of first 4 notes;
		int avPitch = 0;
		TNOTE *tempNote = FirstNoteObject(i);
		int count = 0;
		while (tempNote && count < 4) {
			count++;
			avPitch += tempNote->GetMIDIPitch();
			tempNote = NOTE(tempNote->GetNext(i));
		}
		if (count)
			pitch[i] = (double) avPitch / (double) count;
		else
			pitch[i] = 128; // put empty voice at end
	}

	// sort array
	char swap = 0;
	do {
		swap = 0;
		for (i = 0; i < CVoice - 1; i++) {
			if (pitch[i] < pitch[i + 1])
			// swap elements
			{
				swap = 1;
				int temp = pitch[i];
				pitch[i] = pitch[i + 1];
				pitch[i + 1] = temp;

				temp = sortedVoices[i];
				sortedVoices[i] = sortedVoices[i + 1];
				sortedVoices[i + 1] = temp;
			}
		} // for i
	} while (swap);

	delete[] pitch;
	return sortedVoices;
}

/*
 Quantize attackpoints
 calc two alternative values for each attackpoint
 in range [from ... to)
 */
void TQTRACK::QuantizeAttackpoints(TQNOTE *from, TQNOTE *to, TAbsTime diff, // |shifting| of pre note
		TFracBinList *qAttacks,
		//									int cAttacks,
		TFracBinList *qDurations,
		//									int cDurations,
		double accuracy)

{

	int cAttacks = qAttacks->Count();
	TFrac *delta = new TFrac[cAttacks];
	double *prob = new double[cAttacks];

	// get smallest attackpoint grid
	qAttacks->sort();
	TFrac minQAttack = (*qAttacks)[cAttacks - 1];

	char *valid = new char[cAttacks];

	TFracBinList *selIOIs =
			new TFracBinList(100 * (accuracy) /* alpha == acurateness*/);
	selIOIs->setMinMaxRel(30.0);
	selIOIs->setMinweightDeltaRel(20.0);

	// process notes of track -----------------------------------------------------
	TQNOTE *Now = from;
	int Count = 0;
	while (Now && Now != to) // repeat until Now == to
	{
		Count++;
		TFrac Start = Now->GetAbsTime();

		// add shifting distacne/2 of pre-note to attackpoint
		// todo: reuse attackpoint shifting got quantisation? for testing
		diff = 0L;

		diff /= 2;
		//		Start += diff;
		//		Now->shiftQAbstime( diff, diff );	// store in note
		// calculate search window size
		// std window == plAttack +/- 1/8
		TFrac lsWindow = TFrac((long) (100 * (1 - accuracy) + 0.5), 400L);
		TFrac rsWindow = lsWindow;

		//        lsWindow.Write(stdout);

		// constraints: nowAttack - lsWindow >= prev->attack+duration/2
		TQNOTE *prev = dynamic_cast<TQNOTE *> (Now->GetPrev(Now->GetVoice()));
		if (prev) {
			TFrac p1, p2;
			p1 = prev->qAttackpoint(&p2);
			if (p2 < p1)
				p1 = p2;
			// p2 == erliest attack
			p2 += prev->GetDuration() / 2;
			if (Start - lsWindow < p2)
				lsWindow = Start - p2;
		}
		if (lsWindow < 0)
			lsWindow = 0L;

		int oneMoreTime = 0;
		do {
			// calculate distances and possible attackpoints
			int cValid = possibleAttackpoints(Start, lsWindow, rsWindow,
					qAttacks, valid, delta);

			/// allow at least one grid position
			if (!cValid && accuracy >= 0.99) // machine played
			{
				if (prev) {
					TFrac pIOI = prev->ioi(1, prev->GetVoice());
				}
				// add very small grid class, smaller values should already be marked as ornaments
				TFrac tDur(1, 32);
				// get gcd of attackpoint 
				int i = 0;
				while (i < denomLSize) {
					tDur = TFrac(1, denomList[i]);
					if ((Start % tDur) == 0L)
						i = denomLSize;
					i++;
				}
				/*
				 if( (Start % tDur) != 0L )
				 tDur = TFrac(1,48);
				 else if( (Start % TFrac(1,48)) == 0L )
				 tDur = TFrac(1,48);
				 else
				 tDur = TFrac( 1,64 );
				 */
				if (!qAttacks->findExact(tDur)) {
					qAttacks->addClass(tDur);
					cout << " Added " << tDur.numerator()  <<
							"/" << tDur.denominator() <<
							" to attackpoint grid!\n";
					tDur = TFrac(1, 64);

					qAttacks->sort();
					cAttacks = qAttacks->Count();
					delete[] valid;
					valid = new char[cAttacks];
					delete[] prob;
					prob = new double[cAttacks];
					delete[] delta;
					delta = new TFrac[cAttacks];

					cValid = possibleAttackpoints(Start, lsWindow, rsWindow,
							qAttacks,
							// cAttacks,
							valid, delta);
				} // if not in grid				
			} // if machine played

			if (!cValid) {
#ifdef _DEBUG
				Printf("Warning: no valid attackpoint!\n");
				Now->Debug(cout);
#endif
				// allow at least a smallest grid
				valid[cAttacks - 1] = 1;
				//				valid[cAttacks-2] = 1;
			}
			// calc significance the distances
			valueAttackpoint(Now, valid, qAttacks, delta, accuracy, prob);
			/*
			 oneMoreTime = CheckTriplets( qAttacks,
			 valid,
			 cAttacks,
			 delta,
			 prob,
			 Start);
			 */
		} while (oneMoreTime);
		// select best and second best alternative
		int classID = selectAttackpoint(Now, delta, prob, valid, selIOIs,
				cAttacks);
#ifdef _DEBUG
		/*
		 if( Now->qAttackpoint().denominator() > 32 )
		 {
		 Now->Debug( stdout );
		 qAttacks->write( stdout );
		 }
		 */
#endif

		// add the four possible IOIs to the selList
		if (prev) {
			TFrac n1, n2, p1, p2;
			n1 = Now->qAttackpoint(&n2);
			p1 = prev->qAttackpoint(&p2);
			TFrac pIOI = Now->GetAbsTime() - prev->GetAbsTime();

			TDoubleBinClass *class1 = NULL, *class2 = NULL, *class3 = NULL,
					*class4 = NULL;
			char found = 0;
			TFrac qIOI;
			for (int i = 0; i < 4; i++) {
				switch (i) {
				case 0:
					qIOI = n2 - p2;
					break;
				case 1:
					qIOI = n2 - p1;
					break;
				case 2:
					qIOI = n1 - p2;
					break;
				case 3:
					qIOI = n1 - p1;
				}
				if (qIOI > 0 && pIOI > 0) {
					double rel = qIOI.toDouble() / pIOI.toDouble();
					if (rel >= accuracy && rel <= 1 / accuracy) {
						found++;
						TFracBinClass *classPtr = selIOIs->findExact(qIOI);
						if (!classPtr) {
							classPtr = selIOIs->addClass(qIOI, 0);
							selIOIs->createDistribution();
						}
						switch (i) {
						case 0:
							class1 = classPtr;
							break;
						case 1:
							class2 = classPtr;
							break;
						case 2:
							class3 = classPtr;
							break;
						case 3:
							class4 = classPtr;
							break;
						}
					} // if
				} // if qIOI
			} // for
			// add possible IOIs to selIOI list
			if (found)
				selIOIs->addValue(pIOI, class1, 0.5, pIOI, class2, 0.2, pIOI,
						class3, 0.2, pIOI, class4);
		}// if prev
		//        selIOIs->write(stdout);


		TFracBinClass *atClass;
		if (classID > -1) {
			atClass = dynamic_cast<TFracBinClass *> (qAttacks->Find(classID));
			qAttacks->addValue(atClass->duration() + delta[classID], atClass);
		} else {
			atClass = NULL;
		}
		diff = Now->GetBestDiffAttack();

		// Todo: eval check error probability
		{
			TQNOTE *prevNote = QNOTE(Now->GetPrev(Now->GetVoice()));
			if (prevNote) {
				// todo: compare old IOI ro new IOI 
				// if change is drastic -> error -> detect ornament, change grid
				TFrac temp;
				TFrac prevEarlAt = prevNote->qAttackpoint(&temp);
				if (temp < prevEarlAt)
					prevEarlAt = temp;
				TFrac nowLatAt = Now->qAttackpoint(&temp);
				if (temp > nowLatAt)
					temp = nowLatAt;

				double quantizedIOI = (nowLatAt.toDouble()
						- prevEarlAt.toDouble());
				double playedIOI = (Now->GetAbsTime().toDouble()
						- prevNote->GetAbsTime().toDouble());
				double qErrorProb;

				if (playedIOI > quantizedIOI) {
					qErrorProb = 1 - (quantizedIOI / playedIOI);
				} else {
					qErrorProb = 1 - (playedIOI / quantizedIOI);
				}
				if (qErrorProb > 0.5) {
				}
			} // if prevNote			
		} // block


		// Process next note
		//		TMusicalObject *temp;
		prev = Now;
		//Now->Debug(stdout);
		Now = dynamic_cast<TQNOTE *> (Now->GetNext(Now->GetVoice()));
	} // while

	// unselect triplets
	if( Parent()->TripletYesNoQuestion == Q_NO ) {
		for (int i = 0; i < qDurations->Count(); i++) {
			if ((*qDurations)[i].nonBinary()) // no binary duration
				valid[i] = 0;
		}
	}
	delete[] valid;
	delete[] prob;
	delete[] delta;
	delete selIOIs;
} // Quantize
//----------------------------------------------------------------------
/*!
 calculate two alternatives for the duration
 apply to [from ... to)
 */
void TQTRACK::QuantizeDurations(TQNOTE *from, TQNOTE *to,
		TFracBinList *qDurations,
		//								   int cDurations,
		double accuracy) {
	//	this->Debug();
	// Todo: make attackpoint fix before calling duaration quantisation!
	int cDurations = qDurations->Count();
	char *valid = new char[cDurations + 1];

	double *prob = new double[cDurations + 1];
	TFrac *delta = new TFrac[cDurations + 1];

	/// statistical list for possible best and second best durations
	TFracBinList *bestDurations = new TFracBinList(20);
	bestDurations->setMinMaxRel(30.0);
	bestDurations->setMinweightDeltaRel(20.0);
	bestDurations->createDistribution();

	/// read acurateness from .ini
	qDurations->sort();
	TFrac minQDuration = (*qDurations)[cDurations - 1];

	/// process notes


	//	qDurations->write(stdout);

	TQNOTE *Now = from;
	while (Now && Now != to) // until Now == to
	{
		//		if( Now->GetAbsTime() == TFrac(643,6) )
		//			printf("");
		// calculate search windo size
		/// standard window size = offset +/- 4/16
		// 70 % accuracy -> 1/8 search window
		TFrac lsWindow = TFrac((int) (500 * (1 - accuracy) + 0.5), 1600);
		TFrac rsWindow = lsWindow;
		/*
		 printf("lWindow:");
		 lsWindow.Write(stdout);
		 printf("\n");
		 */
		// constraint lsWindow <= duration/2
		TFrac maxWindow = Now->GetDuration() / 2;
		if (lsWindow > maxWindow)
			lsWindow = maxWindow;

		// constraint earliest offset + rWindow <= latestNext
		TQNOTE *next = dynamic_cast<TQNOTE *> (Now->GetNext(Now->GetVoice()));
		if (next) {
			TFrac n1, n2;
			n1 = Now->qAttackpoint(&n2);
			if (n2 < n1)
				n1 = n2;
			TFrac a1, a2;
			a1 = next->qAttackpoint(&a2);
			if (a2 > a1)
				a1 = a2;
			maxWindow = a1 - (n1 + Now->GetDuration());
			if (rsWindow > maxWindow)
				rsWindow = maxWindow;
		}
		/// limit right window to next attackpoint
		if (rsWindow < 0)
			rsWindow = 0L;

		// calculate distances
		int pDurations = possibleDurations(Now, Now->GetDuration(), lsWindow,
				rsWindow, qDurations, delta, valid
		//							cDurations
				);
		if (!pDurations && accuracy >= 0.99) // machine played
		{
			// add very small grid class, smaller values should already be marked as ornaments
			TFrac tDur = Now->GetDuration();
			if (tDur.denominator() > 64)
				tDur = TFrac(1, 64);

			if (!qDurations->findExact(tDur)) {
				qDurations->addClass(tDur);
				cout << " Added "<< tDur.numerator() << "/" << tDur.denominator() <<
						" to duration grid!\n";
				tDur = TFrac(1, 64);
				if (!qDurations->findExact(tDur)) {
					/*
					 qDurations->addClass( TFrac(1,48) );
					 Printf(" Added 1/48 to duration grid!\n");
					 */
					qDurations->addClass(tDur);
					Printf(" Added");
					Printf(tDur);
					Printf(" to duration grid!\n");

				} // if
				qDurations->sort();
				cDurations = qDurations->Count();
				delete[] valid;
				valid = new char[cDurations + 1];
				delete[] prob;
				prob = new double[cDurations + 1];
				delete[] delta;
				delta = new TFrac[cDurations + 1];

				pDurations = possibleDurations(Now, Now->GetDuration(),
						lsWindow, rsWindow, qDurations, delta, valid
				//							,cDurations
						);
				//					Now->Debug(stdout);
			} // if not in liste			
		}// if machine played


		// allow at least a single duration
		if (pDurations == 0) {
			if (cDurations > 0)
				valid[cDurations - 1] = 1;
			if (cDurations > 1)
				valid[cDurations - 2] = 1;
		}
		// value the alternatives
		valueDuration(Now, valid, qDurations, delta, accuracy, prob);

		// select best and second best alternatives#
		int classID = selectDuration(Now, delta, prob, valid, bestDurations,
				cDurations);
#ifdef _DEBUG
		/*
		 if( Now->qDuration().denominator() > 32 )
		 {
		 // Now->Debug( stdout );
		 }
		 */
#endif
		TFrac bestDur, secDur;
		bestDur = Now->qDuration(&secDur);

		/*
		 double d1,
		 d2;
		 */

		TFracBinClass *classPtr = bestDurations->findExact(secDur);
		if (!classPtr) {
			classPtr = bestDurations->addClass(secDur);
			// bestDurations->setDelta(1/(1+bestDurations->Count()*20.0));
			//bestDurations->createDistribution();
		}

		bestDurations->addValue(secDur, classPtr);

		classPtr = bestDurations->findExact(bestDur);
		if (!classPtr) {
			classPtr = bestDurations->addClass(bestDur);
			//			bestDurations->setDelta(1/(1+bestDurations->Count()*20.0));
			//			bestDurations->addValue(bestDur, dist.classPtr );
			//			bestDurations->createDistribution();
			//			bestDurations->write( stdout);
		}
		bestDurations->addValue(bestDur, classPtr);

		TDoubleBinClass *dClass = qDurations->Find(classID);
		qDurations->addValue(Now->GetDuration(), dClass);

		// process next note
		Now = QNOTE(Now->GetNext(Now->GetVoice()));
	} // while

	delete[] valid;
	delete[] prob;
	delete[] delta;

	//	bestDurations->write(stdout);
	delete bestDurations;

}
//----------------------------------------------------------------------
/*!
 remove small overlapping or small rests between notes,
 by changing the duration notes.

 remove small diffenzes of attackpoints between notes by
 shifting the notes to an average attackpoint

 remarks:
 -this function changes Attackpoint und lgDuration of notes
 unrecoverable.
 -this function can deteced chords and legato
 -TrackNotes must be sorted with ascending AbsTime
 */

void TQTRACK::mergeEqualAttackpoints(int recTempo, // recording tempo
		int recPPQ // ppq at recording
) {
	if (recTempo == 0 || // don't do this for .gmn files
			recPPQ == 0)
		return;

	/*
	 TNOTE *Now,
	 *Temp,
	 *endPtr;	// end of current equals

	 int count;

	 char ende = 0;

	 long DiffTicks;	// Difference in MIDI-Ticks
	 long DiffTime;		// Difference in ms
	 TAbsTime
	 OldAbsTime,
	 NewAbsTime;


	 long a2toa1,  // notes are sorted > must be > 0,
	 e1toa2;  // < 0 gap, > 0 legato
	 */

	// check for notes with zero duration!
	TNOTE *Now = dynamic_cast<TQNOTE *> (FirstNoteObject(-1 /* all voices */));
	while (Now) // process all notes of current track
	{
		TMusicalObject *Temp;
		if (Now->GetDuration() <= 0)
			Temp = Now;
		else
			Temp = NULL;
		Now = dynamic_cast<TNOTE *> (Now->GetNext(-1));
		if (Temp) {
			cout << "Warning: removed zero duration note at " <<
					Temp->GetAbsTime().numerator() << "/" <<
 					Temp->GetAbsTime().numerator() << endl;
			Temp = DetachedVoice(Temp, Now);
			delete Temp;
		} //if Temp
	} // while notes


	/* compare |attack(now) attack(temp|
	 - merge all attacks inside searchwindow if note-offset is  outside window
	 - stop at first attack outside window
	 */
	Now = dynamic_cast<TQNOTE *> (FirstNoteObject(-1 /* all voices */));
	while (Now) // process all notes of current track
	{
		// compare Now.Attack to attacks close to now attack
		TFrac OldAbsTime = Now->GetAbsTime();
		TNOTE *endPtr = NULL;
		TNOTE *Temp = dynamic_cast<TNOTE *> (Now->GetNext(-1));
		char ende = 0;
		TFrac NewAbsTime = OldAbsTime; // = Now->GetAbsTime();
		int count = 1;
		do // search until first note not in equal distance, or
		// first note starting after end of Now
		{
			if (Temp) {

				long a2toa1 = Temp->GetAbsTime().toLong()
						- Now->GetAbsTime().toLong();
				// e1toa2 = |overlap|, if < 0 -> don't move attack
				long e1toa2 = Now->GetAbsTime().toLong()
						+ Now->GetDuration().toLong()
						- Temp->GetAbsTime().toLong();

				// DiffTime = distance attack <-> attack
				long DiffTicks = a2toa1; // sorted -> geq 0
				long DiffTime = DTIMEtoMS(recPPQ, recTempo, DiffTicks);
				if (DiffTime < (long)(  Parent()->EQUAL_TIME) && e1toa2 > 0 && // real overlap
						(DiffTime > 0L))
				// attacks are close enough together
				{
					// check size of overlapping
					if (e1toa2 > a2toa1 && // main part of now is overlapping with Temp
							Temp->GetDuration().toLong() > a2toa1)
					// merge Attacks
					{
						// ok merge
						count++;
						NewAbsTime += Temp->GetAbsTime();
						Temp = dynamic_cast<TNOTE *> (Temp->GetNext(-1));
					} else {
						ende = 1;
					}
				} // if
				else if (DiffTime == 0) {
					count++;
					NewAbsTime += Temp->GetAbsTime();
					Temp = dynamic_cast<TNOTE *> (Temp->GetNext(-1));
				} else
					// Temp is definitly outside searchwindow
					ende = 1;
			} // if
		} while (Temp && !ende);

		// -> Temp == first note outside window

		// Merge all attackpoints [Now, Temp)
		endPtr = Temp;
		// todo: use average attackpoint instead of Now attackpoint
		TFrac nDur;
		if (count > 1) {
			Temp = Now;
			NewAbsTime = NewAbsTime / count;
			NewAbsTime = NewAbsTime.toLong();
			while (Temp && count > 0 && Temp != endPtr) {
				// keep endpoint fixed
				nDur = (Temp->GetDuration() + Temp->GetAbsTime() - NewAbsTime);

				if (nDur > 0l) {
					Temp->SetDuration(nDur);
					Temp->SetAbsTime(NewAbsTime);
				} else {
					Printf("Illegal new duration in removeLegato!\n");
					count = 0; // stop 
					Temp = NULL;
				}
				Now = Temp;
				if( Temp != NULL )
				{
					Temp = dynamic_cast<TNOTE *> (Temp->GetNext(-1));
				}
				count--;
			} // while Temp
		} else {
			Now = dynamic_cast<TNOTE *> (Now->GetNext(-1));
		}
	}// while Now
	CheckSort(); // Check the sorting of the list
} // merge Attackpoints


/*!
 remove small overlapping or small rests between notes,
 by changing the duration notes.

 remove small diffenzes of attackpoints between notes by
 shifting the notes to an average attackpoint
 remarks:
 -this function changes Attackpoint und lgDuration of notes
 unrecoverable.
 -this function can deteced chords and legato
 -TrackNotes must be sorted with ascending AbsTime
 */
void TQTRACK::removeLegatoOverlaps(int recTempo, // recording tempo
		int recPPQ // ppq at recording

) {
	//	Debug();

	if (recTempo == 0 || // don't do this for .gmn files
			recPPQ == 0)
		return;
	/*
	 TNOTE *Now,
	 *Temp;
	 char ende = 0;
	 long DiffTicks;	// Difference in MIDI-Ticks
	 long DiffTime;		// Difference in ms
	 TAbsTime
	 OldAbsTime,
	 NewAbsTime;
	 long a2toa1,  // notes are sorted > must be > 0,
	 e1toa2;  // < 0 gap, > 0 legato
	 //		 e2toa1,
	 //		 e1toe2;
	 // check zero duration!
	 */
	TNOTE *Now = dynamic_cast<TQNOTE *> (FirstNoteObject(-1 /* all voices */));
	while (Now) // process all notes of current track
	{
		TMusicalObject *Temp;
		if (Now->GetDuration() <= 0)
			Temp = Now;
		else
			Temp = NULL;
		Now = dynamic_cast<TNOTE *> (Now->GetNext(-1));
		if (Temp) {
			cout << "Warning: removed zero duration note at " <<
					Temp->GetAbsTime().numerator() << "/" <<
					Temp->GetAbsTime().numerator() << endl;
			Temp = DetachedVoice(Temp, Now);
			delete Temp;
		}
	}// while
	// compare Now.End to attacks close to now.end
	/*
	 remove legato OVERLAPS
	 set now.end = temp.attack if
	 - in distance in searchwindow
	 - distance << duration(now)
	 stop after first attack in search distance
	 */
	/* small overlapps should be removed during voice separation */
	Now = dynamic_cast<TQNOTE *> (FirstNoteObject(-1 /* all voices */));
	while (Now) // process all notes of current track
	{
		TNOTE *Temp = dynamic_cast<TNOTE *> (Now->GetNext(-1));
		char ende = 0;
		do // search for first note not in equal distance
		{
			if (Temp) {
				// > 0 -> legato, < 0 -> gap
				long e1toa2 = Now->GetAbsTime().toLong()
						+ Now->GetDuration().toLong()
						- Temp->GetAbsTime().toLong();
				long a2toa1 = Temp->GetAbsTime().toLong()
						- Now->GetAbsTime().toLong();
				// DiffTime = distance now.end <-> temp.attack
				long DiffTicks = e1toa2;
				long DiffTime = DTIMEtoMS(recPPQ, recTempo, DiffTicks);
				// DiffTime > 0 -> overlapp, Legato
				// DiffTime < 0 -> gap, staccato
				if ((DiffTime > 0 && // overlap, legato
						e1toa2 < a2toa1 && DiffTime < (long) ( (TQMIDIFILE*) Parent())->LEGATO_TIME))
				// attack and end are close together
				{
					// Temp == earliest Attack close to end of Now
					// check size of overlapping
					if (DiffTime > 0 && a2toa1 > e1toa2) {
						Now->SetDuration(Temp->GetAbsTime() - Now->GetAbsTime());
						if (Now->GetDuration() <= 0l)
							printf(
									"Error created illegal duration in remove overlap\n");
						ende = 1; // stop at first attack
					}
				} // if
				else if (DiffTime <= 0) // no overlap
					ende = 1;
				Temp = dynamic_cast<TNOTE *> (Temp->GetNext(-1));
			} // if
		} while (Temp && !ende);
		Now = dynamic_cast<TNOTE*> (Now->GetNext(-1));
	} // while
	// evaluate sustain pedal durations
	//	Debug();
	resolveSustain();
	//	Debug();

} // remove legato
//----------------------------------------------------------------------
/*!
 count nr of different voices in different voice segments
 */
int TQTRACK::countVoices(void) {
	int maxVoice = -1;
	/* ,
	 i,
	 *usedVoices,
	 nrOfVoices = 1;
	 */
	// find highest voice index
	TNOTE *now = this->FirstNoteObject(-1);
	while (now) {
		if (now->GetVoice() > maxVoice) {
			maxVoice = now->GetVoice();
		} else if (now->GetVoice() < 0) {
			Printf("Error: Voice = -1!\n");
		}
		now = NOTE(now->GetNext(-1));
	} // while
	// init array
	int *usedVoices = new int[maxVoice + 1];
	for (int i = 0; i < maxVoice; i++)
		usedVoices[i] = 0;

	int nrOfVoices = 1;
	// search for unused voices
	now = this->FirstNoteObject(-1);
	while (now) {
		if (!usedVoices[now->GetVoice()]) // already counted
		{
			nrOfVoices++;
		}
		usedVoices[now->GetVoice()]++;

		now = NOTE(now->GetNext(-1));
	} // while
	delete[] usedVoices;
	return nrOfVoices;
}
//----------------------------------------------------------------------
//----------------------------------------------------------------------
/*
 return array with features/profile of voices
 */
/*
 TVoiceInfo *TQTRACK::getVoiceFeatures( int *nrOfVoices )
 {
 TVoiceInfo *voiceInfo;
 *nrOfVoices = countVoices(); // size of array
 voiceInfo = new TVoiceInfo[*nrOfVoices];

 TNOTE *now;

 // create profiles of all voices
 now = FirstNoteObject(-1);
 while( now )
 {
 addToVoice( now,
 voiceInfo,
 *nrOfVoices );

 now = NOTE(now->GetNext(-1));
 }
 return voiceInfo ;
 }
 */
//----------------------------------------------------------------------
/*
 merge splitted voices of this track
 re-number the voices to distinc track1->voice1 and track2->voice1
 */
/*
 void TQTRACK::mergeVoices(int offsetSize ) // size of voiceOffset
 {
 // 1. check for sounding voices,
 // 2. check for ambitus


 int newVoice,
 nrOfVoices = -1,
 segStart,
 segEnd,
 i,
 index;

 TVoiceInfo *voiceInfo; // info about unmerged voices
 // result must be deleted
 voiceInfo = getVoiceFeatures( &nrOfVoices );

 int segNr = 0;

 TSearchInfo *searchInfo; // info about merged/target voices
 searchInfo = new TSearchInfo[offsetSize];

 TVoiceInfo *targetInfo;
 targetInfo = new TVoiceInfo[offsetSize];

 segStart = 0;
 // set new voice numbers to voiceInfo
 for( index = 0; index < nrOfVoices; index++ )
 {
 // search for segEnd
 while( index < nrOfVoices &&
 voiceInfo[index].oldVoice <
 (segNr+1) * offsetSize )
 {
 index++;
 }
 segEnd = index;
 for(i =  0; i < segEnd-segStart; i++ )
 {
 // relation: voiceInfo x voiceList
 findClosestVoices( voiceInfo,
 nrOfVoices,
 segStart,
 segEnd,
 targetInfo, // search result and merged voices
 offsetSize,
 searchInfo);
 // set new voices for voiceInfo[
 selectClosestVoice( voiceInfo,
 nrOfVoices,
 segStart,
 segEnd,
 targetInfo,  // search result and merged voices
 offsetSize,
 searchInfo);
 } // for
 segNr++;
 index = segEnd;
 segStart = index; // assign here, before index++ in for(..)
 } // for
 delete [] targetInfo;
 delete [] searchInfo;



 // -------- copy new numbers to all notes -------
 CVoice = offsetSize; // = max voice

 TQNOTE *Now;
 Now    = (TQNOTE *)FirstNoteObject(-1);
 while( Now )
 {
 index = getVoiceIndex( Now->GetVoice(),
 voiceInfo,
 nrOfVoices );

 newVoice = voiceInfo[index].newVoice;
 #ifdef _DEBUG
 if( newVoice < 0 )
 Printf("Error  mergeVoices: newVoice < 0\n!");

 #endif
 Now->SetVoice(newVoice);
 Now = QNOTE(Now->GetNext(-1));
 } // while
 delete [] voiceInfo;
 } // mergeVoices
 */
//----------------------------------------------------------------

/// set color for all notes in range [from,to}
void setColor(TQNOTE *from, TNOTE *to, int color) {
	while (from && from != to) {
		from->color = color;
		from = dynamic_cast<TQNOTE *> (from->GetNext(from->GetVoice()));
	}
}

/*!
 split Voices in Range [from...to)
 start voice numbering at offset
 */
void TQTRACK::splitVoices(TNOTE *from, TNOTE *to, int offset, // nr of start voice
		double &pChord, // cost for chord detection
		double &pPitch, // cost for pitch distance
		double &pGap, // cost for gaps
		double &pOverlap,// cost for overlapping
		/// insignificance limit
		double gapThreshold,
		/// colorize voices?
		char colorizeVoices,
		TMIDIFILE *theMidifile) {

	if (!from) {
		Printf("Warning splitVoices: from == NULL\n");
		return;
	}

	// read decay values from Ini!!
	int tempCVoice = -1;
	// decay values from fermata.ini
	int lSearchDepth = 15;
	int lookBack = 0, emptyVoiceIv = 11;
	double walkThresh = 80; // 0...100
	double newDecay = 800; // !! oldDeday + newDeay == 1000

	TInifile *tempIni = Parent()->getInifile();
	if (tempIni) {
		lSearchDepth = tempIni->GetValInt("LSEARCHDEPTH", "15",
				"voice separation search depth of local search");

		double tempVal = tempIni->GetValFloat("SPLITVOICEDECAY", "0.8",
				"voice separation decay of pitch look back");
		if (tempVal <= 1 && tempVal > 0) {
			newDecay = tempVal * 1000;
		} else if (tempVal <= 10 && tempVal > 1) {
			newDecay = tempVal * 100;
		} else if (tempVal <= 100 && tempVal > 10) {
			newDecay = tempVal * 10;
		} else if (tempVal <= 1000 && tempVal > 100) {
			newDecay = tempVal;
		} else {
			cout << "Warning: " << tempVal <<
					" illegal value for SPLITVOICEDECAY!\n";
		}
		lookBack = tempIni->GetValInt("PITCHLOOKBACK", "2",
				"voice separation, #of notes");
		emptyVoiceIv = tempIni->GetValInt("EMPTYVOICEIV", "11",
				"voice separation interval penalty for starting a new voice");
		tempCVoice = tempIni->GetValInt("MAXVOICES", "-1",
				"voice separation: max voices per TRACK");
		walkThresh = Parent()->getInifile()->GetValFloat("RWALKTRESH", "0.8",
						"don't touch! [0...1] Voices eparation: treshhold for random walk");
	} // if tempIni

	if (walkThresh <= 1) {
		walkThresh *= 100;
	}
	if (walkThresh <= 0) {
		cout << "WARNING: illegal value for RWALKTRESH: " << walkThresh << endl;
		walkThresh = 80;
	} else if (walkThresh > 100) {
		cout << "WARNING: illegal value for RWALKTRESH: " << walkThresh << endl;
		walkThresh = 80;
	}
	// get info about voices in range [from..to)
	TVoiceCountList *voiceProfile = GetVoiceProfile(from, to);

	/// #attacks to be processed in next step (= next slice)	
	// set #voices to max voices in range, some voices might not needed
	int cAttacks = voiceProfile->maxVoice();

	CVoice = cAttacks; // = max cAttacks in all Slices

	// limit CVoice to ini entry
	if (tempCVoice > 0) // use only if > 0 else ignore
	{
		if (tempCVoice > CVoice)
			cout << "Warning: MAXVOICE = " << tempCVoice <<
					" > needed voices = " <<CVoice << endl;
		CVoice = tempCVoice;
	}
	if (CVoice <= 0) {
		cout << "Warning: replaced illegal MAXVOICES " << CVoice <<
				"by -1!\n";
	}

	if (CVoice > cAttacks)
		cAttacks = CVoice;
	if (CVoice < 1) {
		cout << "Cvoice = "<< CVoice << endl;
		exit(1);
	}
 
	/// infolist[cAttacks]  about pending notes (= notes of slice)
	TNoteInfo *noteList = new TNoteInfo[cAttacks];

	/// infos about opened voices (feature list)
	TVoiceInfo *voiceInfo = new TVoiceInfo[cAttacks];

	// array for storing current, global, and local minimun values
	TNoteInfo *minVoiceSet = new TNoteInfo[cAttacks * 2];

	// voice settings before testing voice settings for notes
	TNoteInfo *oldVoice = new TNoteInfo[cAttacks];

	//---------------------- global Version ----------------------------------------

	// global version is stored in backup 1.3c


	//----------------------- init arrays ----------------------
	TQNOTE *Now = dynamic_cast<TQNOTE *> (from);

	// count  all notes in range
	TQNOTE *tempNote = Now;

	double allAttacks = 1;
	while (tempNote) {
		allAttacks++;
		tempNote = QNOTE(tempNote->GetNext(-1));
	}

	// set global offset to very first note
	voiceInfo[0].gOffset = from->GetAbsTime();

	// split voices -----------------------------------------

	cout << "SplitVoices track: " << TrackNumber() << endl;

	double selAttacks = 1;
	int modCount = 0;
	int color = 0;

	// Next == first unprocessed note after init
	TQNOTE *Next = Now;
	while (Next && // for all notes in [Next..to)
			Next != to) {
		theMidifile->cVoiceSlices++;

		// reset random walk threshold
		double curWalkThresh = walkThresh;

		// colorise slice
		if (colorizeVoices) {
			setColor(Next, to, color);
			color++;
			color %= 4;
		}

		// progress indicator
		if (((int) ((selAttacks / allAttacks) * 100)) / 10 == modCount) {
			Printf(".");
			modCount++;
		}

		// count #attacks of next equal-time notes (= #notes in slice)
		cAttacks = countAttacks(Next, to);

		// init voice array / noteList
		for( int i = 0; i < cAttacks; i++) {
			noteList[i].attack = NULL;
			noteList[i].chordRoot = -1;
			noteList[i].minDiff = 0;
			noteList[i].minVoice = -1;
		} // for

		// set nextToDO to first note after cAttacks (= first note after slice)
		TQNOTE *nextToDo = Next;
		// keep attacktime of first note
		TFrac curAttackTime = nextToDo->GetAbsTime();

		// copy pending attacks into array
		for(int i = 0; i < cAttacks; i++) {
			if (nextToDo != to) {
				noteList[i].attack = nextToDo;
				nextToDo = QNOTE(nextToDo->GetNext(-1));
			} 
			else // no more notes to do
			{
				// stop processing and init to NULL;
				for (int j = i; j < cAttacks; j++) {
					noteList[j].attack = NULL;
					noteList[j].minVoice = -1;
					noteList[j].chordRoot = -1;
				}
				i = cAttacks; // stop processing
			} // else			
		} // for cAttacks

		// process cAttack notes ------------------------------

		// select closest voice for each entry in noteList
		// get inital voice settings
		//        printf("<CVoice"); fflush(stdout);
		findClosestVoices(noteList, cAttacks, voiceInfo, CVoice); // # of voices


		//	-------------------- try to optimize selection/slice ---------------		
		if (cAttacks == 0)
			Printf("Error in voice separation: cAttacks == 0!\n");

		// best EVER seen value
		double gMinDistance = voiceDistance(noteList, cAttacks, voiceInfo,
				CVoice, -1, -1, -1, // dummys
				pChord, // cost for chord detection
				pPitch, // cost for pitch distance
				pGap, // cost for gaps
				pOverlap, // cost for overlapping
				lookBack, emptyVoiceIv, gapThreshold,
				theMidifile);

		for(int i = 0; i < cAttacks; i++) // copy values
		{
			minVoiceSet[i] = noteList[i]; // local
			minVoiceSet[i + cAttacks] = noteList[i]; // global == local
		}

		//		maxRuns = log(cAttacks * max(voiceProfile->maxVoice(),CVoice) )* lSearchDepth;
		// this should not be quadratic!!!
		//        maxRuns = (cAttacks * max(voiceProfile->maxVoice(),CVoice) )/2 * lSearchDepth;
		int maxRuns = max(cAttacks, CVoice);
		maxRuns = max(voiceProfile->maxVoice(),maxRuns) * lSearchDepth;
		if (cAttacks == 1) {
			maxRuns = 1; // we have only direct neighbours!!!
			curWalkThresh = 100; // no random walk
		}
		// maxInvalid  = maxRuns; // stop after #invalid tries
		//        maxInvalid = 1 + (cAttacks * min(voiceProfile->maxVoice(),CVoice) )/2 * lSearchDepth;
		int maxInvalid = 1 + (min(voiceProfile->maxVoice(),CVoice))
				* lSearchDepth;

		// very useful for debugging!
		//        printf("cAttacks=%d, CVoice=%d, maxRun=%d\n", cAttacks, CVoice, maxRuns);

		int forceRandom = 0; // == 1 after unsuccesfull neighbour search


		/// do the local search optimisation
		double lMinDistance = gMinDistance;
		/// limit # of runs without any improvement
		int runs = 0;
		do { //----------------------------------------------
			double startDistance = lMinDistance; // search for better local min
			for(int i = 0; i < cAttacks; i++) // store current settings
			{
				oldVoice[i] = noteList[i];
				noteList[i].flag = 0;
			}

			// go trough all neighbours or do random step?
			double getNeighbours = rand() % 100;

			if ((getNeighbours > curWalkThresh || forceRandom) && // decide for random step
					cAttacks > 1) // no random walk for single attacks, only direct neighbours
			{
				forceRandom = 0; // do only one time

				int cInvalid = 0;
				// select a a random direct neighbour ----------
				int validSelection;
				do {
					// useful for debug
					//                    printf("r"); fflush(stdout);
					char linked = 0;
					int k = rand() % cAttacks; // select index pos for change
					// change randomly only to a DIRECT neighbour
					{
						// get number of possible chords with k
						int possibleRoots = countPChords(noteList, cAttacks, k);
						// is k already part of a chord?
						if (noteList[k].linkID() > -1)
							linked = 1;
						else
							linked = 0;
						// get random new voice OR linkID
						int u;
						if (linked) // all voices possible
							u = rand() % (CVoice + possibleRoots);
						else // skip own voice
						{
							u = rand() % (CVoice + possibleRoots - 1);
							if (u >= noteList[k].minVoice)
								u++;
						}
						int randRoot;
						if (u >= CVoice) // change chord-link
						{
							randRoot = u - CVoice;
							u = rand() % CVoice; // will be used if randRoot == k!
						} else
							// change only voice
							randRoot = -1;

						noteList[k].minVoice = u;
						//						noteList[k].chordRoot = randRoot;
						//						rootK = noteList->rootID(k);

						if (randRoot > -1 /* &&	// don't link to itself
						 randRoot != k */) {
							// search for root nr randRoot
							int selRoot = getCChord(noteList, cAttacks, k,
									randRoot + 1);
							noteList->unlink(k); // remove from chord
							noteList[k].minVoice = noteList[selRoot].minVoice;
							noteList->link(selRoot, k);
						} else
							noteList->unlink(k); // remove from chord

					} // block

					// check for equal attacks in voices
					validSelection = valid(noteList, cAttacks, voiceInfo,
							CVoice);

					if (!validSelection) {
						//                        printf("i"); fflush(stdout);
						cInvalid++;
						// restore settings 
						for(int i = 0; i < cAttacks; i++) // keep current settings
						{
							noteList[i] = oldVoice[i];
						}
					}
				} while (cInvalid < maxInvalid && !validSelection);

				if (validSelection) // else keep old values
				{
					startDistance = voiceDistance(noteList, cAttacks,
							voiceInfo, CVoice, -1, -1, -1, //dummies
							pChord, // cost for chord detection
							pPitch, // cost for pitch distance
							pGap, // cost for gaps
							pOverlap, // cost for overlapping
							lookBack, emptyVoiceIv, gapThreshold,
							theMidifile);
					// new start selection for next step
					for (int k = 0; k < cAttacks; k++)
						minVoiceSet[k] = noteList[k];

					lMinDistance = startDistance; // new start of search, this is new lMin
					//                    printf("V>\n"); fflush(stdout);
				} else // selection not valid ->  restore, search again
				{
					//                    printf("I\n"); fflush(stdout);
					for(int i = 0; i < cAttacks; i++)
						noteList[i] = oldVoice[i];
					forceRandom = 1;
					runs++;
				}
			} // if			
			else // find closest next neighbour-----------------------------------------
			{
				forceRandom = 1; // will be turned to 0 if a new local min found in this neighbourhood

				// strategie 1: view ALL neighbours and select very best
				// strategie start at random neighbour pos and select first best
#ifdef FIRSTBEST_STOP
				int startPos = rand() % cAttacks;
#else
				int startPos = 0;
#endif
				for (int vC = 0; vC < cAttacks; vC++) // test each next neighbour
				{
					int v = (vC + startPos) % cAttacks;

					// get number of possible chords
					int possibleRoots = countPChords(noteList, cAttacks, v);

					int uMax;
					char linked;
					if (noteList[v].linkID() > -1) {
						linked = 1;
						uMax = CVoice + possibleRoots;
					} else // skip minVoice
					{
						linked = 0;
						uMax = CVoice + possibleRoots - 1;
					}

					char doCompare = 0;

					for (int uTemp = 0; uTemp < uMax; uTemp++) // test voice u for note[v]
					{
						int u2 = uTemp;

						doCompare = 1;
						// keep current selection for later restore
						for(int i = 0; i < cAttacks; i++) {
							oldVoice[i] = noteList[i];
						}

						int u;
						int chordRoot;
						if (u2 >= CVoice) // link, use voice of root
						{
							chordRoot = u2 - CVoice;

							chordRoot = getCChord(noteList, cAttacks, v,
									chordRoot + 1);

							// use voice of root 
							u = noteList[chordRoot].minVoice;

						} else // not link -> unlink[v]
						{
							u = u2;
							chordRoot = -1;
						}

						{
							noteList->unlink(v);
							noteList[v].minVoice = u;

							if (chordRoot > -1) {
								noteList[v].minVoice
										= noteList[chordRoot].minVoice;
								noteList->link(v, chordRoot);
							}

							//							sortLinks( noteList, cAttacks); // only change voices
							doCompare = valid(noteList, cAttacks, voiceInfo,
									CVoice);

						}

						if (doCompare) {
							double curDistance = voiceDistance(noteList,
									cAttacks, voiceInfo, CVoice, -1, -1, -1, // dummies
									pChord, // cost for chord detection
									pPitch, // cost for pitch distance
									pGap, // cost for gaps
									pOverlap, // cost for overlapping
									lookBack, emptyVoiceIv, gapThreshold,
									theMidifile);
							// found new optimum
							if (curDistance < lMinDistance) // new localMin
							{
								for(int i = 0; i < cAttacks; i++) // copy values
								{
									minVoiceSet[i] = noteList[i]; // copy local setting, keep global Setting
								}
								lMinDistance = curDistance;

								forceRandom = 0; // this neighbour was better than a previuos selection
#ifdef FIRSTBEST_STOP
								vC = cAttacks; // stop loop
#endif
							}
						} // if doCompare
						// restore for next neighbour search
						for(int i = 0; i < cAttacks; i++)
							noteList[i] = oldVoice[i];
						// noteList[v].chordWith = oldChord;
					} // for test current note for all voices
				} // for v (cAttacks)
			} // else (search neighbours)


			// copy current == best direct neighbour (localMin) into noteList
			// start new search from new local min
			for(int i = 0; i < cAttacks; i++) {
				noteList[i] = minVoiceSet[i];
			}// for attacks			


			// found new global min
			if (lMinDistance < gMinDistance) {
				gMinDistance = lMinDistance;
				// copy into global setting
				for(int i = 0; i < cAttacks; i++) {
					minVoiceSet[i + cAttacks] = minVoiceSet[i];
				}
				runs = 0;
			} else
				// no new global optimum
				runs++;
			if (cAttacks == 1) // single note
			{
				forceRandom = 0;
			}
		} while (runs < maxRuns); // ---------------------------------------------------------------------------------

		// copy global optimum into notes----------------------
		for(int i = 0; i < cAttacks; i++) {
			noteList[i] = minVoiceSet[i + cAttacks];
		} // for


		// select to best setting
		for(int i = 0; i < cAttacks; i++) {
			int VoiceID = noteList[i].minVoice;
			if (noteList[i].attack && VoiceID > CVoice)
				Printf("VoiceID > CVoice error");
			if (noteList[i].attack) // maybe already removed
				noteList[i].attack->SetVoice(VoiceID + offset);
		}

		// attacks maybe removed from noteList!
		makeChords(noteList, cAttacks, CVoice, this);


		for(int i = 0; i < cAttacks; i++) {
			int VoiceID = noteList[i].minVoice;
			if (noteList[i].attack) {
				noteList[i].attack->removeLegato(noteList[i].attack->GetVoice());
				// calc new average values for selected voice
				voiceInfo[VoiceID].addToVoice(noteList[i].attack, newDecay);
			}
			noteList[i].attack = NULL;
		} // for i
		Next = nextToDo;
		selAttacks += cAttacks;
	} // while
	delete[] minVoiceSet;
	delete[] oldVoice;
	delete[] noteList;
	delete[] voiceInfo;

	if (cVoice() > 1)
		cout << "into "<< cVoice() << " voices";
	else
		cout << "into "<< cVoice() << " voice";
	if (tempCVoice > 0)
		cout << ", limited by MAXVOICES=" << tempCVoice;
	Printf(".\n");
	Delete(voiceProfile);

} // split Voices

TFrac TQTRACK::optimumOffset(TQNOTE *from, TQNOTE *to, int voice,
		TFrac barlength, TFrac resolution, double &oldQuality,
		double &newQuality) {
	TFrac offset;
	TFrac epsilon(1, 24);

	return offset;
}

//----------------------------------------------------------------------
/*
 Voice Separation
 split track into different voices
 use shortes way rule

 remarks:
 -can only be used before quantization, because alternatives are not used
 -removeLegato needs to be called before!
 */
void TQTRACK::SplitVoices(int recTempo, int recPPQ,
        TMIDIFILE *theMidifile) {
	clock_t start = clock();

	// init, all notes to voice -1-----------------------------
	TQNOTE *Now = dynamic_cast<TQNOTE *> (FirstNoteObject(-1 /* all voices */));
	if (!Now)
		return;
	while (Now) {
		Now->SetVoice(-1);
		Now = QNOTE(Now->GetNext(-1/* all voices */));
	} // while
	/*
	 // do voice profile analysis--------------------------------
	 // - calc MaxVoices
	 TVoiceCount *voiceProfile;
	 voiceProfile = GetVoiceProfile(this);
	 // needs to be deleted by Delete() !!
	 */

	//	this->Debug();
	TInifile *tempIni = Parent()->getInifile();

	double pChord = tempIni->GetValFloat("PCHORD", "0.5",
			"voice separation chord penalty");
	double pGap = tempIni->GetValFloat("PGAP", "0.5",
			"voice separation gap penalty");
	double pPitch = tempIni->GetValFloat("PPITCH", "0.5",
			"voice separation pitch penalty");
	double pOverlap = tempIni->GetValFloat("POVERLAP", "0.5",
			"voice separation overlap penalty");

	char colorizeVoices = 0;
	const char *colorVoices = tempIni->GetValChar("COLOURVOICESLICES", "OFF",
			"[ON|OFF] colourise voice slices");
	if (!strcmp(colorVoices, "ON") || !strcmp(colorVoices, "1")) {
		colorizeVoices = 1;
	}

	/// convert 50ms to ticktiming as insignificance limit for pGap
	/// new convert 1s into ticks for calculating gap distance
	double gapThreshold = MStoDTime(recPPQ, recTempo, 1000 /* ms */);
	if (gapThreshold <= 0) {
		printf("Illegal gap Threshold (ppq=%d, tempi=%d)!\n", recPPQ, recTempo);
		exit(1);
	}

	int offset = 0; // start voice nr 

	TNOTE *from = FirstNoteObject(-1 /* all voices */);
	splitVoices(from, NULL, offset, pChord, // cost for chord detection
			pPitch, // cost for pitch distance
			pGap, // cost for gaps
			pOverlap,// cost for overlapping
			gapThreshold, // 1000ms in miditicks
			colorizeVoices,
			theMidifile);
	clock_t finish = clock();
	double processTime = (double) (finish - start) / (double) CLOCKS_PER_SEC;
	cout << "(" << processTime << "s)\n";
} // SplitVoices


//----------------------------------------------------------------------
/*!
 check sorting of note list
 Bubblesort
 */
void TQTRACK::CheckSort(void) {
	TMusicalObject *Now = FirstObject(-1);

	TMusicalObject *Prev = NULL;
	TMusicalObject *Next = NULL;
	if (Now)
		Next = Now->TMusicalObject::GetNext(-1/* all voices */);
	while (Next) // check complete list
	{
		while (Next && *Next >= *Now) // Sorting OK ?
		{
			Now = Next;
			Next = Next->TMusicalObject::GetNext(-1/* all voices */);
		} // while OK

		if (Next) // if !listend -> sorting error
		{
			// printf("sort \n");fflush(stdout);
			// Next
			// search back for right position of Next
			Prev = Now->TMusicalObject::GetPrev(-1);
			while (Prev && *(dynamic_cast<TNOTE *> (Prev))
					> *(dynamic_cast<TNOTE *> (Next))) {
				Prev = Prev->TMusicalObject::GetPrev(-1);
			}
			// detach Next from list
			Next = DetachedVoice(Next, Next->TMusicalObject::GetNext(-1));
			// insert Next
			if (Prev) {
				Next->SetNext(Prev->TMusicalObject::GetNext(-1));
				Prev->SetNext(Next);
			} else {
				Next->SetNext(Notes);
				Notes = Next;
			}

			Next = Now->TMusicalObject::GetNext(-1);
		} // if sort error
	} // while
} // CheckSort
//----------------------------------------------------------------------
#ifdef aksdjhkasjdh
char TQTRACK::QuantizeTempo( TAbsTime ppn )
/*
 QuantizeTempo
 quantize tempotrack to n*ppn

 use only for Track[0] of MIDI file

 */
{

	TMetaTempo *Ptr,
	*Prev,
	*Next;
	Ptr = FirstTempo();
	Printf("Error: exit in quantize Tempo!\n");
	Prev = 0;
	while( Ptr )
	{
		Next = MetaTempo(Ptr->GetNext(-1/* all voices */));
		Ptr->QuantizeToN( 1 );

		// check if two tempo changes were quantized to the same attackpoint
		if( Prev &&
				(Prev->qAttackpoint() == Ptr->qAttackpoint()) )
		{ // delete one tempo change
			Prev->SetTempo( Ptr->GetTempo() );
			Prev->SetNext( Next );
			delete Ptr;
			Ptr = Next;
		}
		else // next element
		{
			Prev = Ptr;
			Ptr = Next;
		}
	} // while
	return 1;
} // QuantizeTempo
#endif
//----------------------------------------------------------------------
/*
 Call SetQData for each voice
 remarks:
 - attackpoints and durations must be quantized before
 */
void TQTRACK::SetQData(void) {
	// Debug(); 
	int i = 0;
	while (i < CVoice) {
		SetQData(i); // CVoice may be changed in SetQData
		i++;
	}
	// Debug();
} // SetQData
//----------------------------------------------------------------------
/*
 Call SetQData for voice v,
 if overlappings in a voice are found, a new voice will be created

 remarks:
 - attackpoints and durations must be quantized before
 */
void TQTRACK::SetQData(int v) {
	//	this->Debug();
	TFracBinList *bestDurations = new TFracBinList(20);
	// all best and secBest durations should be valid, so add them to a durations list
	bestDurations->setMinMaxRel(20.0);
	bestDurations->setMinweightDeltaRel(30.0);
	bestDurations->createDistribution();
	// go to end of list
	TQNOTE *Note = (TQNOTE *) FirstNoteObject(v); // v = voice
	while (Note && Note->GetNext(v)) // all notes in voice v
	{
		Note = QNOTE(Note->GetNext(v));
	};
	// Now add from end to begin
	while (Note) {
		TFrac dur1, dur2;
		dur1 = Note->qDuration(&dur2);

		bestDurations->addExact(dur1);
		bestDurations->addExact(dur2);
		Note = QNOTE(Note->GetPrev(v));
	}
	// bestDurations->write(stdout);
	TFrac AbsTime = 0L;
	TQNOTE *Prev = NULL;
	TQNOTE *lastUnique = NULL;
	Note = (TQNOTE *) FirstNoteObject(v); // v = voice
	while (Note) // all notes in voice v
	{

		/// conflict ?
		if (Note->SetQData(Prev, bestDurations) < 0L) {
			// -> endPrev >= attackNext
			/*
			 a) cut prev
			 b) move prev->prev->prev
			 c)

			 */
			TQNOTE *pNote = NULL;
			TQNOTE *tNote = Note;
			//Note->GetNext(Note->GetVoice())->Debug(stdout);
			char change = 0;
			int end = 0;
			// try to mow all earlier notes to an earlier alternativ
			// stop at firt note with unqie alternatives
			while (!end) {

				pNote = tNote;
				tNote = dynamic_cast<TQNOTE *> (tNote->GetPrev(
						tNote->GetVoice()));
				//tNote->Debug(stdout);
				if (tNote && !tNote->TOK()) {
					// select earlier attack
					TFrac secAt;
					TFrac bestAt = tNote->qAttackpoint(&secAt);
					// swap selection
					if (secAt < bestAt) {
						if (tNote->atSelection == firstSel)
							tNote->atSelection = secondSel;
						else
							tNote->atSelection = firstSel;
						tNote->dSelection = undefSel;
						change = 1;
					} // if earlier
				} // if prev note
				else {
					end = 1;
				}
			} // while
			if (change) {
				Note = pNote;
				Prev = dynamic_cast<TQNOTE *> (Note->GetPrev(Note->GetVoice()));
				if (Prev) {
					AbsTime = Prev->qAttackpoint();
				} else {
					AbsTime = 0L;
				}
			} else {
				cout << "WARNING: could not resolve overlap at " <<
						Note->GetAbsTime().numerator() << "/" <<
						Note->GetAbsTime().denominator() << endl;
			}
		} // if conflict

		int qerror = 0;
		// still a conflict?
		if (Note->SetQData(Prev, bestDurations) >= 0L) {

			// no conflict
			if (Prev && Prev->qAttackpoint() + Prev->qDuration()
					> Note->qAttackpoint()) {
				// cut previous note
				TFrac newDuration = Note->qAttackpoint() - Prev->qAttackpoint();
				if (newDuration > 0) {
					Prev->expandTo(newDuration, 0);
				} else {
					qerror = 1;
				}
			} else {
				// check sorting
				if (Note->TOK())
					lastUnique = Note;
				// sorting is ok
				Prev = Note;
				AbsTime = Note->qAttackpoint();
				AbsTime += Note->qDuration();

				// add selected duration to bestDurations
				{
					TFrac selDuration = Note->qDuration();
					TFracBinClass * classPtr = bestDurations->findExact(
							selDuration);
					if (!classPtr) {
						classPtr = bestDurations->addClass(selDuration);
					}
					bestDurations->addValue(selDuration, classPtr);
				}

				/*
				 check size of inaccuracy
				 */
				if (Note && dynamic_cast<TQNOTE *> (Note)) {
					TFrac playDur = Note->GetDuration();
					TFrac attackDiff = Note->GetAbsTime()
							- Note->qAttackpoint();
					attackDiff = abs(attackDiff) * 2;
					if (attackDiff >= playDur && playDur > 0L) {
#ifdef _DEBUG
						Printf( "Warning SetQData::Big Move\n");
						Note->Debug(cout);
#endif
					}
				} // if qnote						
			} // else
		} else // unresolvable error
		{
			// overlapping Prev-Note-> Merge to chord
			qerror = 1;
		}
		if (qerror) {
			// try to shift pre notes 

			// 1. search back for first Note with rest before attack !! chords
			// 2. search forward until first note with rest before attack
			// 3. requantize from 1. to 2. use random select
			// 4. try n- times until success
			// 5. decide if succesfull
			// 6. if success set QData from 1. to 2. !! recursiv

			// keep current note in Temp (will be removed)

			TQNOTE *Temp = Note;
			Temp->atSelection = firstSel;
			Temp->dSelection = firstSel;

			//switch to Note next note
			Note = QNOTE(Note->GetNext(Note->GetVoice()));
			// create new chord (Prev+Temp)
			TAbsTime chordAt, chordDur;

			if (Prev) {
				chordAt = Prev->qAttackpoint();
				chordDur = Prev->qDuration();
			}

			int trace = 0;
			if (Prev && !dynamic_cast<TQChord *> (Prev)) // Prev is not a chord
			{
				TQChord *newChord = NULL;

				if (!dynamic_cast<TQChord *> (Temp)) {
					// replace Prev by chord
					// Prev must be != NULL, else no overlap could have happend!

					newChord = new TQChord(Prev, Temp, Note, this); // [Temp, Note)  will be detached, and newCHord will be inserted

					newChord->color = 0; // bugfix newChord->color = 4;
					Prev = newChord;
					Temp = Prev;

					trace = 1;

				} else // Temp is already a chord
				{
					newChord = new TQChord(Prev, NULL, NULL, this);
					newChord->color = 0; // bugfix newChord->color = 4;
					Prev = newChord;

					// add all notes of {Temp} to Prev, detach Tenp
					newChord->Add(Temp, this, 0);

					Temp = Prev; // keep Prev
					trace = 2;
				}
			} else if (Prev) // Prev = Chord, add Temp to prevLastChord

			{
				// Prev is a chord, remove Temp from list
				dynamic_cast<TQChord *> (Prev)->Add(Temp, this, 0);
				dynamic_cast<TQChord *> (Prev)->color = 0; // 4;

				Temp = Prev; // Keep Prev
				trace = 3;
			}
			//Temp->SetVoice( XtraVoice ); // store in extra voice
			cout << "Quantize error " << Temp->GetAbsTime().numerator() <<
					"/" << Temp->GetAbsTime().denominator() <<
					"; merged to chord\n";

		} // if qerror

		//Prev->Debug(stdout);
#ifdef _DEBUG
		if(Prev &&
				(
						Prev->dSelection == undefSel ||
						Prev->atSelection == undefSel) )
		{
			Printf("------------------\n");
		}
		if (Prev != NULL && primeDenomDistance(Prev->qAttackpoint().denominator(),
				Prev->qDuration().denominator()) >= 1) {
			Printf("PrimeError: ");
			Prev->Debug(cout);
		}
#endif
		// switch to next
		if (Prev)
			Note = QNOTE(Prev->GetNext(v));

	} // while

	//	Debug();
	//	bestDurations->write(stdout);
	delete bestDurations;
} // SetQData
//----------------------------------------------------------------------
/*
 writes a gmn description of this track
 result:
 for Track0: AbsTime of last processed event
 for other Tracks: 0 ok,
 -1 error
 */

TAbsTime TQTRACK::Convert(ostream &gmnOut, // outfile
		TTRACK *Track0, TAbsTime from, // first note > from
		TAbsTime to, // last note <= to
		TAbsTime glOffset) {
	/*
	 int  	i = 0,
	 v;
	 char res;

	 TAbsTime 	AbsTime;         // attackpoint

	 TFrac DefFrac;
	 TAbsTime Offset;

	 //	Offset = from;

	 TQNOTE 	*Note;
	 */
	char glVoiceOffset = 0;
	// guard
	if (Track0 == this || Track0 == NULL) {
		// ----------- convert control information of Track0 ------------
		TQuantizedObject *current;
		TMusicalObject *temp = Current(); // use track function to prevent double output of meta events
		if (temp)
			current = dynamic_cast<TQuantizedObject *> (temp);
		else
			current = NULL;

		//  search for first note in range
		while (current && current->qAttackpoint() < from) {
			temp = NextNote(-1);// current->TMusicalObject::GetNext(-1);
			if (temp)
				current = dynamic_cast<TQuantizedObject *> (temp);
			else
				current = NULL;
		} // while
		// at track0 should only Meta events occur!

		// current > from
		// convert until <= to
		while (current && current->qAttackpoint() <= to) {
			if (to < current->qAttackpoint()) // maybe already written in Track0
			{
				gmnOut << '_';
				TFrac DefFrac = WriteDuration(gmnOut, from
						- current->qAttackpoint(), TFrac(-1, -1));
			} // if
			current->Convert(gmnOut, from, // preEndTime
					0L, // pre denominator
					NULL); // Track0,

			from = current->qAttackpoint(); // contrls have no duration! + current->qDuration();
			if (from > to)
				from = from;
			//			temp = current->TMusicalObject::GetNext(-1);
			// this == track 0, switch to next note
			temp = NextNote(-1);// current->TMusicalObject::GetNext(-1);
			if (temp)
				current = dynamic_cast<TQuantizedObject *> (temp);
			else
				current = NULL;
		} // while

		return from;
		//------------- end of Tracl0 ----------------
	} else // this is not Track0 ----------------
	{

		TTagType CurTags = 0, PrevTags = 0;
		TTagList // *Slurs = NULL,
		*Staccato = NULL;

		TInifile *tempIni;

		// read ornament settings from ini file
		// create  Tag Lists
		tempIni = Parent()->getInifile();
		if (tempIni) {

			if (strcmp(tempIni->GetValChar("STACC_OUT", "OFF",
					"[ON|OFF] infer and output of \\stacc tag"), "OFF"))
				Staccato = MarkStaccato(this);
		} else // can't open inifile
		{
			//			Slurs = MarkSlur( this );
			Staccato = MarkStaccato(this);
		}
		//-----------------------------

		MergeTags(Staccato); // merge Staccato to Track::Tags
		char res = 0;

		/*
		 if( !Notes )
		 {
		 fputc( '[', file );
		 fprintf( file, "(* empty voice *)");
		 fputc( ']', file );
		 }
		 else */
		if (Notes) // any notes in this track
		{

#ifdef HEISENBERG_DVL
			int ppq = 1;
			long barLengths[3] = {ppq*3,ppq*4,ppq*2},
			windowSize = (long)ppq *80L;
#endif
			int voiceWritten = 0; // turns to 1 if at least on voice was written

			// ---------------- get sorted voices id -------------------------

			int * sortedVoices;
			sortedVoices = sortVoices();
			int iv;
			// ---------------------------------------------------------------
			char instrOut = 0;
			if (!strcmp(Parent()->getInifile()->GetValChar("INSTR_OUT", "OFF",
					"[ON|OFF] write instr tags to output"), "ON")) {
				instrOut = 1;
			}

			for (iv = 0; iv < CVoice; iv++) // process all voices
			{
				cout << " Converting voice "<< iv + 1 << "... ";

				if (glOffset.numerator() > 0L)
					glVoiceOffset = 1;
				int v = sortedVoices[iv]; // v = id of voice[iv] of sorted list

				if (Track0) // init current note
					Track0->FirstObject(-1);

				TQNOTE *Note = (TQNOTE *) FirstNoteObject(v);

				if (Note) {
					// convert voice v
					if (voiceWritten) // check for voice delimiter
					{
						gmnOut << ',' << endl;
						

					}
					gmnOut << "\n(* voice " << v + 1 << " *)";
					gmnOut << "[";

					if (!voiceWritten) {
						const char *val =
										Parent()->getInifile()->GetValChar("TITLE_OUT",
												"ON",
												"[ON|OFF] add  \\title<\"filename\"> to the GUIDO output");
						if (!strcmp(val, "ON")) {
							// put tile tag in very first voice
							gmnOut << "\\title<\"" << Parent()->filename() << "\">\n";
						}
					} // if


					voiceWritten = 1;
					// check for single staff setting
					{
						const char *staffSetting;
						staffSetting
								= Parent()->getInifile()->GetValChar("SINGLESTAFF",
										"OFF",
										"[ON|OFF] write all voices into a single staff");
						if (!strcmp(staffSetting, "ON")) {
							Printf("  Writing to single staff!\n");
							gmnOut << "\\staff<1> \n";
							if (v == 0) {
								gmnOut <<  "\\stemsUp \n";
							} else {
								gmnOut << "\\stemsDown \n";
							}
						}
					}
					gmnOut <<  "\\barFormat<\"system\"> ";

					if (instrOut && voiceName(v) != string()) {
						gmnOut << " \\instr<\"" << voiceName(v) << "\", dx=-2cm> ";
					}

					res = 1;

					int i = 0;
					TFrac AbsTime = 0L;
					//					Projection( Note, NULL );
					TFrac DefFrac = TFrac(0, 0);

					//-----    Statistic functions
#ifdef HEISENBERG_DVL
					/*
					 DevValues Devs;
					 Devs = GetDeviations( Note, NULL );
					 fprintf(file,"\n(* AttackDev %f, %f\n",
					 Devs.BestAttDev,
					 Devs.SecondAttDev);
					 fprintf(file,"AttackSSQ %f, %f\n",
					 Devs.BestAttSSQ,
					 Devs.SecondAttSSQ);
					 fprintf(file,"DurationDev %f, %f\n",
					 Devs.BestDurDev,
					 Devs.SecondDurDev);
					 fprintf(file,"DurationSSQ %f, %f*)\n",
					 Devs.BestDurSSQ,
					 Devs.SecondDurSSQ);
					 */
					// obsolete, meter estimation autoCorrelation already implemented
					/*
					 GetBestBarLength( Note,
					 3,
					 barLengths,
					 windowSize // 20bars
					 );
					 */
#endif
					do {
						if (i == MAXNOTEINLINE) // line break
						{
							gmnOut << "\n";
							//fputc( 13 , file );
							i = 0;
						}
						if (Note) // notes to process
						{

							/*
							 write here all control info
							 don't write any control info in TQNOTE::Convert
							 write rests in Track0::COnvert
							 */

							// init from = -1 !!

							// Space between EP(LastNote) and AP(Note)
							// write rest
							if (AbsTime > Note->qAttackpoint()) {
								gmnOut <<  "(* Quantize error: Attackpoint shifted! *)";
							}
							if (Track0) {
								// write meta notes and rests during rest [from,to]
								AbsTime = Track0->Convert(gmnOut, NULL, // == Track0
										AbsTime, // from
										Note->qAttackpoint(),// to
										0L /*dummy offset*/
								);
								DefFrac = TFrac(0, -1);
							}

							if (glVoiceOffset > 0L && glOffset.numerator() > 0L) {
								gmnOut <<  "_*" <<glOffset.numerator()  <<"/" <<glOffset.denominator()<< "(*voice offset*) ";
								glVoiceOffset = 0;
							}

							if (AbsTime < Note->qAttackpoint()) // maybe already written in Track0
							{
								gmnOut <<  '_';
								DefFrac	= WriteDuration(gmnOut,
												Note->qAttackpoint() - AbsTime,
												DefFrac);
								AbsTime = Note->qAttackpoint(); // == end of rest
							} else if (AbsTime > Note->qAttackpoint()) {
								gmnOut <<  "(* Quantize error: Attackpoint shifted! *)";
							}
							// write tags at attackpoint
							CurTags = GetTags(Tags, Note->GetAbsTime(),
									Note->GetVoice());
							if (CurTags != PrevTags) {
								PrintTags(gmnOut, PrevTags, CurTags);
								PrevTags = CurTags;
							}

							// write note
							DefFrac = Note->Convert(gmnOut, AbsTime, // current time in file
									DefFrac, Track0);
							// Write Tags at note end
							CurTags = GetTags(Tags, Note->GetAbsTime()
									+ Note->GetDuration(), Note->GetVoice());
							if (CurTags != PrevTags) {
								PrintTags(gmnOut, PrevTags, CurTags);
								PrevTags = CurTags;
							}

							AbsTime = Note->qAttackpoint();
							AbsTime += Note->qDuration();
							i++;
							Note = QNOTE(Note->GetNext(Note->GetVoice()));
						} // if Note
					} while (Note); // until voice end
					// check for opened tags
					if (CurTags) {
						PrintTags(gmnOut, CurTags, 0);
					}

					// Todo: add rest to fill last measure of piece

					gmnOut <<  ']';
					Printf("done.\n");
				} // if Note
				else
				{
					gmnOut << "(* empty voice *)";
					Printf("empty\n");
				}
			} // for( iv = 0....
			delete[] sortedVoices;
		} //if Notes
		else
			res = 0;

		//		DeleteTags(Slurs);
		return res;
	} // if !Track0
	return -1; // error
} // Convert
//----------------------------------------------------------------------
/*
 delete all tempochanges in current track.
 use for reset of quantization -> requantize
 */
void TQTRACK::DelTempoNotes(void) {
	TQuantizedObject *Now, *Prev = NULL;
	TMusicalObject *TempObj = FirstObject(-1);
	if (TempObj)
		Now = dynamic_cast<TQuantizedObject *> (TempObj);
	else
		Now = NULL;
	while (Now) // complete track
	{
		TQuantizedObject *Next = QuantizedObject(
				Now->TQuantizedObject::GetNext(-1/* all voices */));
		//		if( Now->GetDuration() == 0L )	// this is a tempochange -> delete
		if (dynamic_cast<TMetaTempo *> (Now)) {
			if (Now == Notes) // head of list
			{
				Notes = Next;
			} else if (Prev) {
				Prev->SetNext(Next);
			}
			delete Now;
		} // if lgDuration
		else {
			Prev = Now;
		}
		Now = Next;
	} // while Now
	CurrentNote = Notes;
} // DelTempoNotes

//-------------------------------------------------------------------
void TQTRACK::ResetDiffs(void)
// reset alternatives of notes
//
{
	TQNOTE *Now = (TQNOTE *) FirstNoteObject(-1 /* all voices */);
	while (Now) {
		Now->ResetDiffs();
		Now = QNOTE(Now->GetNext(-1/* all voices */));
	}
} // ResetDiffs
//-------------------------------------------------------------------
/*
 factory function to create now notes
 */
TNOTE *TQTRACK::CreateNewNote(TAbsTime abstime, TAbsTime duration,
		unsigned char note, unsigned char intens) {
	TQNOTE *temp = new TQNOTE(abstime, duration, note, intens);
	return temp;
}
//-------------------------------------------------------------------

void TQTRACK::preProcess(void) {
}

//-------------------------------------------------------------------
// not used with local search split voices
/*
 remove legato needs to be called before
 split voices needs to be called later !!
 */
/*
 void TQTRACK::markChords( void )
 {
 TVoiceCount *voiceProfile;
 voiceProfile = GetVoiceProfile(this);
 #ifdef _DEBUG
 FILE *temp;
 temp = fopen("voiceProfile.txt","wt");
 ::Debug(voiceProfile, temp );
 fclose(temp);
 #endif
 ::markChords(this, voiceProfile);
 Delete(voiceProfile);
 }
 */
//-----------------------------------------------------------------------
void TQTRACK::Merge(TTRACK *track2) {
	MergeTags(((TQTRACK*) track2)->Tags);
	((TQTRACK*) track2)->Tags = NULL;

	TTRACK::Merge(track2);
}

//-----------------------------------------------------------------------
//-------------------------------------------------------------------
#ifdef UNSEDFUNCTIONS

long OldPos,
long NewPos,
long NewDuration,
char MoveNext,
int Tonhoehe )
/*
 not used at the moment
 change the attackpoint and duration of the note at OldPos in voice Voice
 */
{
	TQNOTE *Temp,
	*NextInVoice,
	*TheNote; // search note

	long erg = 0;
	long DiffPos, // distance to move
	OldStart,
	DiffToNext; // == Distance quantized notend to unquantized attackpoint
	// of next note


	DelTempoNotes(); // reset the track
	TheNote = 0;

	if( Voice > -1 )
	{
		Temp = (TQNOTE *)FirstNote( Voice );
		while( Temp &&
		(Temp->qAttackpoint() != OldPos) )
		{
			Temp = (TQNOTE *)Temp->GetNext( Temp->GetVoice() );
		}

		if( Temp ) // found
		{
			NextInVoice = (TQNOTE *)Temp->GetNext( Temp->GetVoice() );
			if( NextInVoice )
			DiffToNext = NextInVoice->GetAbsTime() -
			Temp->qAttackpoint() -
			Temp->qDuration();
			else
			DiffToNext = 0;

			OldStart = Temp->GetAbsTime();

			DiffPos = NewPos + NewDuration - OldPos - Temp->GetDuration();
			Temp->Unlock( ABSTIME );
			Temp->SetAbsTime( NewPos );
			Temp->Lock( ABSTIME );
			Temp->Unlock( DURATION );
			Temp->SetDuration( NewDuration );
			Temp->Lock( DURATION );
			Temp->SetPitch( Tonhoehe );

			TheNote = Temp;

			erg = Temp->GetAbsTime()
			+ Temp->GetDuration()
			+ DiffToNext;

			if( NextInVoice )
			erg -= NextInVoice->GetAbsTime();

			if( NewPos < OldStart )
			{
				DiffPos = NewPos - OldStart;
				// Auf Überschneidungen mit vorherigen Noten testen
				Temp = (TQNOTE *)FirstNote( Voice );
				while( Temp &&
				(Temp->GetAbsTime() < NewPos) )
				{
					if( (Temp->GetAbsTime() + Temp->GetDuration()) > NewPos )
					Temp->SetAbsTime( Temp->qAttackpoint() );
					Temp = (TQNOTE *)Temp->GetNext( Temp->GetVoice() );
				}
			} // if < 0
			NewPos = erg + OldPos;
		} // if Temp
		else
		ErrorMsg( 22 );
	} // if Voice > -1


	if( MoveNext ) // move the next notes also
	{
		DiffPos = NewPos - OldPos;
		Temp = (TQNOTE *)FirstNote();
		while( Temp )
		{
			if( (Temp != TheNote) && // don't move TheNote again
			(Temp->qAttackpoint() >= OldPos) )
			{
				Temp->Unlock( ABSTIME );
				Temp->SetAbsTime( Temp->GetAbsTime()
				+ DiffPos );
			}
			Temp = (TQNOTE *)Temp->GetNext();
		} // while
	} // if( move
	ResetDiffs();
	CheckSort();
	return erg;
} // Change Note
#endif
//----------------------------------------------------------------------
/*
 void TTRACK::Add( TNOTE *ptr )
 */
/*
 apped ptr at listend

 {
 if( Notes )	// ptr an Liste anh„ngen
 {
 while( CurrentNote->GetNext() )
 CurrentNote = CurrentNote->GetNext();
 CurrentNote->SetNext( ptr );
 }
 else	// Liste ist leer
 {
 Notes   = ptr;
 CurrentNote = ptr;
 }
 LastNote = ptr;
 } // Add
 */
//----------------------------------------------------------------------
//----------------------------------------------------------------------
/*
 Check if tempoCHanges in TempoTrack can be used in this Track.
 if collisions occur, the tempoChanges will be moved or deleted
 result:
 1 : changes were made
 0 : nothing changed
 remarks:
 - SetQData for this Track must be called before!
 */

#ifdef OLD_SETTEMPO
char TQTRACK::CheckTempo( TTRACK *TempoTrack )
{
	TQNOTE *Now; // current note at track
	TMetaTempo *TempoChange; // next tempo change

	TAbsTime TempoTime, // attackpoint of tempochange
	NoteEnd; // endpoint of current note (Now)
	char erg = 0;
	int i;

	for( i = 0; i<CVoice; i++ ) // check every voice in track
	{
		Now = (TQNOTE *)FirstNoteObject( i );
		TempoChange = TempoTrack->FirstTempo(); // next tempo change
		while( Now && TempoChange )
		{
			TempoTime = TempoChange->qAttackpoint();
			if( Now->qAttackpoint() < TempoTime ) // check for collision
			{
				NoteEnd = Now->qAttackpoint() + Now->qDuration();
				if( NoteEnd > TempoTime ) // collision == overlapping
				{

#ifdef UNUSED
					TQNOTE *NextTempoChange; // next-next tempochange
					NextTempoChange = (TQNOTE *)TempoChange->GetNext();
					if( NextTempoChange &&
					(NoteEnd >= NextTempoChange->qAttackpoint() ))
					// more than one tempo change during now
					{
						// move tempo change
						TempoChange->SetAbsTime( NoteEnd );
						if( NextTempoChange )
						{
							// store tempo of NextTempoChange into TempoChange
							TempoChange->SetTempo( NextTempoChange->GetTempo());
							// remove NextTempoChange from list
							Temp = NextTempoChange;
							TempoChange->SetNext( Temp->GetNext() );
							delete Temp;
						} // if( NextTempoChange
						erg = 1;
					} // if >=
					else // move TempoChange
					{
#endif
						Printf("Unresolved TempoChange shifted from %ld/%ld",
						TempoTime.numerator(),
						TempoTime.denominator());
						Printf(" to %ld/%ld\n",
						NoteEnd.numerator(),
						NoteEnd.denominator());
						TempoChange->SetAbsTime( NoteEnd );
						erg = 1; // changes happened
						TempoChange = TempoChange->GetNext(-1/* all voices */);
					}
					else // no collision
					{
						Now = (TQNOTE *)Now->GetNext( Now->GetVoice() );
					}
				}
				else // no collision
				{
					TempoChange = TempoChange->GetNext(-1/* all voices */);
				}
			} // while
		}// for
		return erg;
	} // CheckTempo
#endif
	//----------------------------------------------------------------------
#ifdef OLD_SETTEMPO
	void TQTRACK::SetTempo(TTRACK *TempoTrack)
	/*
	 Store the tempo profile of TempoTrack in the notes of current track.
	 remarks:
	 - SplitVoices must be called before.
	 */
	{

		TQNOTE *NewNote,
		*Temp,
		*Now,
		*Prev;

		TMetaTempo *NextTempo;

		Printf("re-implement TQTRACK::SetTempo!\n");
		return;
#ifdef skjdfjgf
		int i;

		TAbsTime TempoTime, // attackpoint of tempo change
		NowTime;

		for( i = 0; i<CVoice; i++ ) // process every voice
		{
			Now = (TQNOTE *)FirstNoteObject( i /* voice i*/);
			Prev = (TQNOTE *)FirstNoteObject( i /* voice i*/);

			NextTempo = TempoTrack->FirstTempo(); // Next tempo change


			while( Now ) // until track end
			{
				if( NextTempo )
				TempoTime = NextTempo->qAttackpoint();
				else
				TempoTime = 0;

				NowTime = Now->qAttackpoint();
				NewNote = NULL;

				if( NowTime == TempoTime ) // Attackpoint(TempoChange) == AttackPoint(Now)
				// store tempo change in note
				{
					if( NextTempo )
					Now->SetTempo( -1 * // tempo < 1 -> TempoChange
					NextTempo->GetTempo() );
					else
					Now->SetTempo( -1 * DEFAULT_TEMPO );

					// Next TempoChange
					if( NextTempo )
					NextTempo = NextTempo->GetNext(-1/* all voices */);
					if( NextTempo )
					TempoTime = NextTempo->qAttackpoint();

					// If Next TempoChange during Now, process again
					if( NextTempo &&
					((Now->qAttackpoint() + Now->qDuration()) <= TempoTime) )
					Now = (TQNOTE *)Now->GetNext(Now->GetVoice() );
				} // if( NowTime
				else if( NowTime > TempoTime ) // Attackpoint(TempoCHange) < Attackpoint(Now)
				// store new TempoChange in voice i
				{
					NewNote = new TQNOTE( TempoTime,
					0,
					0,
					0 );
					myAssert( NewNote != 0 );
				} // else if
				/* else split note
				 else if( NowTime + Now->GetDuration() > TempoTime )
				 // Note teilen
				 {
				 NewNote = new TNOTE(
				 TempoTime,
				 Now->GetDuration() -
				 ( TempoTime -
				 Now->GetAbsTime() ),
				 Now->GetNote(),
				 Now->Intens() );
				 Now->SetDuration( TempoTime -
				 Now->GetAbsTime() );
				 } // else if( NowTime...
				 */
				else
				Now = (TQNOTE *)Now->GetNext( Now->GetVoice() ); // Process next note

				if( NewNote ) // If new note has been created, insert into list
				{
					NewNote->SetVoice( i );
					NewNote->SetTempo( -1 * // TempoChange
					NextTempo->GetTempo() );
					Temp = (TQNOTE *)FirstNoteObject(-1 /* all voices */);
					Prev = NULL;
					while( Temp ) // search for correct position in list
					{
						if( Temp->qAttackpoint()
						+ Temp->qDuration()
						> TempoTime )
						// found
						Temp = NULL;
						else
						{
							Prev = Temp;
							Temp = Temp->GetNext(-1/* all voices */);
						} // else
					} // while( Temp )
					if( Prev ) // insert into list
					{
						NewNote->SetNext( Prev->GetNext(-1/* all voices */) );
						Prev->SetNext( NewNote );
					}
					else // append as first element
					{
						NewNote->SetNext( Notes );
						Notes = NewNote;
					}
					Prev = NULL;

					// next tempo change
					NextTempo = NextTempo->GetNext(-1/* all voices */);
					if( NextTempo )
					TempoTime = NextTempo->qAttackpoint();
					NewNote = NULL;
				} // if NewNote
				if( !NextTempo ) // cancel, no more tempo changes
				Now = NULL;
			} // while ( Now )
		} // for( i =
#endif
	} // SetTempo
#endif
