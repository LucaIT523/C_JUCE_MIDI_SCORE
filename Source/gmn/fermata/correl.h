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

#if !defined( __CORREL_H__ )
#define __CORREL_H__

class TQNOTE;
class TNOTE; 
class TTimeSignature;


/// calculated the offset to the "best" downbeat position for from
TNOTE *phase( TNOTE *from,
			  int voice,	
			 TFrac barlength,
			 /// result
			 TFrac &phase,
			 TFrac &epsilon,
			 TMIDIFILE *theMidifile);


//--------------- score time autocorrelation --------------------------------------

/// time signature sig auto correlation for an array of times signatures, res = array of peaks
double *autoCorrelation( /// startnote
						 TQNOTE *start,
						 /// -1 == all voices
						 int voice,
						 /// array size
						 int size,	
						 /// array of time signatures
						 TTimeSignature *meterSigs,
						 /// integration size for autocorrelation
						 TFrac integrSize,
						  TMIDIFILE *theMidifile);

/// time signature auto correlation for a specified times signature, res = p(timeSignature)
double autoCorrelation( /// start note
						TQNOTE *start,
						 /// -1 == all voices
						 int voice,
						/// time signature/barlengh
						TTimeSignature signature, 
						/// integration size for autocorrelation
						TFrac integrSize,
						/// epsilon value
						TFrac &epsilon,
						  TMIDIFILE *theMidifile );

/// auto correlation for a single note and a single barlength/distance
double autoCorrelationSingle( /// note to test
							  TQNOTE *start,
							 /// -1 == all voices
							 int voice,
							  /// barlength to test
							  TFrac barLength,
							  /// barlength +/- epsilon size 
							  TFrac &epsilon,
							  /// evaluate nTimes windows
							  int nTimes,
							  TMIDIFILE *theMidifile);

//----------------- ms timing autocorrelation -------------------------------
/// meter sig auto correlation for an array of times signatures, res = array of peaks
double *autoCorrelationMS( TQNOTE *start,	
						 /// -1 == all voices
						 int voice,
						 /// integration Size for autocorrelation
						 double &integrSize,
						 /// max barlength in ms
						 double &maxBarLength,
						 /// resolution of barlength test
						 double &resolution,
						 /// size of result array
						 int *arraySize,
						  TMIDIFILE *theMidifile);

/// meter sig auto correlation for a specified times signature, res = p(timeSignature)
double autoCorrelationMS( TQNOTE *start,
						 /// -1 == all voices
						 int voice,
						double &integrSize,
						/// barlength in ms
						double &barlength,
						/// window = barlength +/- epsilon
						double &espilon,
						  TMIDIFILE *theMidifile );

/// auto correlation for a single note and a  barlength/distance
double autoCorrelationSingleMS( TQNOTE *start,
							 /// -1 == all voices
							 int voice,
							  /// barlength in ms
							  double barLength,
							  double &epsilon,
							  TMIDIFILE *theMidifile);

/// return a weight for note used for timesignature autocorrelation 
double getWeight( TQNOTE *note, TMIDIFILE *theMidifile);
#endif
