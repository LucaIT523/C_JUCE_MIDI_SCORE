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

#if !defined(__knn_h__ )
#define __knn_h__


#include "k_array.h"
#include "ga.h"




// #define TFeature double

class TPrototype
{
	int classIDI;
//	int cFeaturesI;
	TFeatures *featuresI;

public:
	TPrototype( int classID,
//					int cFeatures,
					TFeatures *features);
	int cFeatures(void) {return featuresI->cFeatures();};
	TFeatures *features(void){return featuresI;};
	virtual ~TPrototype(void);
	int classID(void){return classIDI;};
	double distance(TGene *weights,
						TFeatures *features);
	void Debug(FILE *out);
};



typedef TPrototype *PTPrototype;

double KNNCompare( int cFeatures,
						TGene *weights,
						TFeatures *prototype,
						TFeatures *test);

int KNNSelect( int cFeatures,
					TGene *weights,
					int cPrototypes,
					PTPrototype *prototypes,
					TFeatures *test);
int KNNTest( int cFeatures,
					TGene *weights,
					int cPrototypes,
					PTPrototype *prototypes,
					TPrototype *test);


/*
double KNNTestSet(int cWeights,
					  TGene *weights);

*/
int CreatePrototypes( void );
void DeletePrototypes( void );
int GetClass(TFeatures *tempFeatures,
				 PTPrototype *PROTOTYPES,
				 int maxPrototype);

#endif
