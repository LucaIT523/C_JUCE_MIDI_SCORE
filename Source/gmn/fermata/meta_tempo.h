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

#if !defined ( __meta_tempo_h__ )
#define __meta_tempo_h__

/*
  Meta 
  TMetaTempo;
  and Tool functions 

  Author: Juergen Kilian
*/


#include "note.h"
#include "quantised_obj.h"

//! class for tempo tag  based on TQuantizedObject
class TMetaTempo : public  TQuantizedObject
{
    /// tempo =	numerator/denominator per minute
    int tempoI;
    /// tactus level / beat division
    int numerator,		
        denominator;
public:
	//! return abstime in ms
	void setAbsTimeMs( double ms);
	double getAbsTimeMs( void );

	void SetTempo(int tempo)
	{tempoI = tempo;};
	int GetTempo(void)
	{return tempoI;};

	virtual void Debug( FILE *out )
	{
		TQuantizedObject::Debug(out);
		fprintf(out, "Tempo\n");
	}
	virtual TMetaTempo *ToMetaTempo( void )
	{return this;};
	TMetaTempo( //! attackpoint in tick or score timing depending on context
			TAbsTime absTime,	
			//! beats per minute
		int tempo,					
		//! division/duration of beat
		int tactusNum,		
		//! division/duration of beat			
		int tactusDenom) : TQuantizedObject(absTime)
	{ 
		tempoI = tempo;
		numerator = tactusNum;
		denominator = tactusDenom;
		absTimeMs = 0; 
		hidden = 0;
	};
	
	virtual ~TMetaTempo(void){};
	/// get next TMetaTempo, skip other types!
	virtual TMusicalObject *GetNext(int voice);
	//! write in GUIDO syntax
	virtual TAbsTime Convert( /// gmn output file
						      ostream &gmnOut,
						      /// enpoint of pre-note
		TAbsTime preEndtime,
		/// denominator of pre-note	
		TAbsTime  prev,	
		/// control track (tempo, key, meter, ...)	
		TTRACK *Track0  
                );
	//! if 1 write  only \tempo<"","1/4=x"> to .gmn
	char hidden;
protected:
	//! abstime in ms
	double absTimeMs;
};


TMetaTempo * MetaTempo( TMusicalObject *s );

#endif
