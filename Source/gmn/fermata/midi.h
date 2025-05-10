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
|	filename : MIDI.H
|	Autor : Juergen Kilian
|	Datum	: 17.10.1996-1998, 2011
|	classdeclaration of TMIDI
|   03/05/26 code cleaning
--------------------------------------------------------------*/
#ifndef __MIDI_H__
#define __MIDI_H__
//---------------------------------------------------------------
#include <string>
#include <sstream>

using namespace std;
#include "portable.h"

#include "defs.h"
#include "liste.h"

/// turn on interactive questions (TInifile)
//#define INTERACTIVE

class TInifile;
//---------------------------------------------------------------

//----- constants used for askDetectKey and askDetect
//----- constants used for askDetectKey and askDetectMeter -------
#define OFF 0
#define MIDIFILE 1
#define DETECT 2
#define UNKNOWN 3
#define YES 4
//-----------------------------------------------------------------


//-----------------------------------------------------------------
class TTRACK;
class TMusicalObject;

//! basic midifile class with read function
class TMIDIFILE  {
protected  :
	void initData();
	double qLength;

    virtual char readIni(TInifile *inif);
    char mMustReload;
    char mDirty;
    TMusicalObject *mCurControl;
    string name;
    int Relduration;
    TTRACK *EvTracks;
    TTRACK *ControlTrack;
	FILE *file; // input MIDIFile
    virtual TTRACK *CreateNewTrack(long  offset);
    void DeleteTracks(void);
    int CountIn;
    char mAuftakt;
    char CalcTempo;
    unsigned char format;
    int TrackCountI;
    unsigned char Status;
    char CurrentTrack;
    char OK;
    const char *mIniFilename;
    TInifile *mInifile;
    std::string *mGLNBuffer;
public:
    int tickTiming;
    int RecPPQ;
    int RecTempo;
    TMIDIFILE(const char *name);
    TMIDIFILE(std::string *buffer);
    virtual ~TMIDIFILE(void);
    char Open(FILE *log);
    virtual char Read(FILE *out);
    char Close(void);
    virtual int currentKey(TAbsTime absTime);
    virtual int askDetectMeter(void)
    {
        return OFF;
    }

    ;
    virtual int askDetectKey(void)
    {
        return OFF;
    }

    ;
    unsigned char Type(void)
    {
        return format;
    }

    ;
    const string filename(void)
    {

        return name;
    }

    ;
    char Ok(void)
    {
        return OK;
    }

    ;
    int TrackC(void)
    {
        return TrackCountI;
    }

    ;
    void addTrack(TTRACK *ptr);
    int Ppq(void)
    {
        return RecPPQ;
    }

    ;
    int Tempo(TAbsTime *absTime = NULL);
    TTRACK *GetTrack(int i);
    char getAuftaktI() const;
    char getCalcTempo() const;
    TTRACK *getControlTrack() const;
    int getCountIn() const;
    TMusicalObject *getCurControl() const;
    char getCurrentTrack() const;
    char getDirty() const;
    TTRACK *getEvTracks() const;
    FILE *getFile() const;
    unsigned char getFormat() const;
    char getMustReload() const;
    string getName() const;
    int getRecPPQ() const;
    int getRecTempo() const;
    int getRelduration() const;
    unsigned char getStatus() const;
    int getTickTiming() const;
    int getTrackCountI() const;
    int getVoiceSlices() const;
    void setAuftaktI(char AuftaktI);
    void setCalcTempo(char CalcTempo);
    void setControlTrack(TTRACK *ControlTrack);
    void setCountIn(int CountIn);
    void setCurControl(TMusicalObject *curControl);
    void setCurrentTrack(char CurrentTrack);
    void setDirty(char Dirty);
    void setEvTracks(TTRACK *EvTracks);
    void setFile(FILE *file);
    void setFormat(unsigned char format);
    void setMustReload(char mustReload);
    void setName(char *name);
    void setRecPPQ(int RecPPQ);
    void setRecTempo(int RecTempo);
    void setRelduration(int Relduration);
    void setStatus(unsigned char Status);
    void setTickTiming(int tickTiming);
    void setTrackCountI(int TrackCountI);
    void setVoiceSlices(int cVoiceSlices);
    double quarterLengthMS();
    TTRACK *FirstTrack(void)
    {
        return EvTracks;
    }

    ;
    void SetRelduration(int relduration)
    {
        if(relduration != Relduration){
            mDirty = 1;
            Relduration = relduration;
        }
    }

    ;
    int cVoiceSlices;
    TInifile *getInifile();
    void setInifile( TInifile *inifile);
    void setIniFilename( const char* filename);
	int EQUAL_TIME;
	int LEGATO_TIME;
	int TripletYesNoQuestion;
};
//---------------------------------------------------------------
#endif
