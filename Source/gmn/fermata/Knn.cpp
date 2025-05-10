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

/*-----------------------------------------------------------------
	filename: knn.cpp
	author:   Juergen Kilian
	date:     1998-2001
	k-nn functions
-----------------------------------------------------------------*/

//#include "knn.h"
#include "spatternfile.h"


#include <math.h>
#include <stdio.h>
//#include <io.h>

#include "k_array.h"

#ifdef slkjsldkjf

int  nTESTSET = 0;
int  nPROTOTYPES = 0;


PTPrototype *PROTOTYPES = NULL;
PTPrototype *TESTSET = NULL;

#define PFILE "pool.pat"


int GetClass(TFeatures *tempFeatures,
			  PTPrototype *PROTOTYPES,  // array
			  int maxPrototype)			// size of array
/*
	find closest class for tempFeatures in PROTOTYPES
*/
{
	int i,
		 res = maxPrototype;
	TGene *weights;
	weights = new TGene[tempFeatures->cFeatures()];
	for(i =0; i < tempFeatures->cFeatures(); i++ )
	{
		weights[i] = 1;
	}
	for(i = 0; i < maxPrototype; i++ )
	{
		if( PROTOTYPES[i]->distance(weights, tempFeatures) == 0)
		{						
			res = PROTOTYPES[i]->classID();
		}

	}
	delete [] weights;
	return res;
}



int CreatePrototypes( void )
/*
	create a set of prototypes from a patternfile
*/
{
// Todo combine k-nn with TIOI/TMovement

	int result = 0;

  TSPFILE *Pattern;

  Pattern = new TSPFILE(PFILE);
  Pattern->Read();

  nPROTOTYPES = Pattern->CPattern();
  nTESTSET = nPROTOTYPES/2;


  PROTOTYPES = new PTPrototype[nPROTOTYPES];  	// array of pointer
  TESTSET = new PTPrototype[nTESTSET];				// array of pointers


  TSPATTERN *temp;
  TFeatures Features,
				*tempFeatures;
  int i,cId;
  temp = Pattern->first();
  char *tempStr,c;
  for(i=0; i < nPROTOTYPES; i++)
  {
		tempFeatures = GetFeatures( temp->FirstNote(), NULL);
		result = tempFeatures->cFeatures();
		// calculate class ID by comparing features to features of existing prototypes
		cId = GetClass(tempFeatures,
							PROTOTYPES,
							i);
		if(cId != i )
		{
//			tempStr[0]=0;
			printf("Same class (y/n)?\n");
//			tempStr = Pattern->Get(cId)->GetString(tempStr,29);
			tempStr = Pattern->Get(cId)->ToString();
			printf("%s ==",tempStr);
//			tempStr[0]=0;
			delete [] tempStr;
//			temp->GetString(tempStr,29);
			tempStr = temp->ToString();
			printf("%s\n",tempStr);
			delete [] tempStr;
			int ok=0;
			while( !ok )
			{
				c = getchar();
				if(c == 'n')
				{
					cId = i;
					ok = 1;
				}
				else if(c == 'y')
				{
					ok = 1;
				}
			}
		}
		PROTOTYPES[i] = new TPrototype(cId, // class id
									tempFeatures);
		temp = temp->Next();
  }

  FILE *ptypes;
  ptypes = fopen("ptypes","wt");
  int j;
  for(i=0; i<nTESTSET;i++)
  {
		j = rand() % nPROTOTYPES;
		TESTSET[i] = PROTOTYPES[i];
		TESTSET[i]->Debug(ptypes);
  }
  fprintf(ptypes,"prototypes\n");
  for(i=0; i<nPROTOTYPES;i++)
  {
		PROTOTYPES[i]->Debug(ptypes);
  }

  fclose(ptypes);


  delete Pattern;
  return result;
}

void DeletePrototypes( void )
{
	int i;
	for(i = 0; i < nPROTOTYPES; i++)
	{
		delete PROTOTYPES[i];
	}
	if( PROTOTYPES )
		delete [] PROTOTYPES;
	PROTOTYPES = NULL;
	if( TESTSET )
		delete [] TESTSET;
	TESTSET = NULL;
}

TPrototype::TPrototype( int classID,
//					int cFeatures,
					TFeatures *features)
{
	classIDI = classID;
//	cFeaturesI = cFeatures;
	featuresI = features;

}

TPrototype::~TPrototype( void )
{
	delete  featuresI;
}
double TPrototype::distance(TGene *weights,	// array
						   TFeatures *features) // array
{
	return KNNCompare(cFeatures(),
							weights,
							featuresI,
							features);
}

void TPrototype::Debug(FILE *out)
{
//	int i;
	fprintf(out,"id: %d, ",classID());
	featuresI->Write( out );
	fprintf(out,"\n");
	
}


double KNNCompare( int /*cFeatures*/,	// size of vector
				  TGene *weights,
				  TFeatures *prototype,
				  TFeatures *test)
/*
		get distance between *prototype and *test
*/
{
	int i;
	double res = 0;
	for(i=0; i < prototype->cFeatures(); i++ )
	{
		res += (weights[i] * pow(
					(prototype->val(i) - test->val(i)) ,2));
	}
	res = sqrt(res);
	return res;
}

int KNNTest( int cFeatures,
				TGene *weights,
				int cPrototypes,
				PTPrototype *prototypes,
				TPrototype *test)
/*
	find closest class for *test in *prototypes
	and check if result was correct

	return: 1: classified in correct class
			  0: else
	remarks:
		in *test must the correct class be set
*/
{
	int temp;
	temp = KNNSelect(cFeatures,
						  weights,
						  cPrototypes,
						  prototypes,
						  test->features());

	if( test->classID() == temp) // match
		return 1;
	else
		return 0;
}


int KNNSelect( int ,  // ??
				TGene *weights,
				int cPrototypes,
				PTPrototype *prototypes,
				TFeatures *test)
/*
	find closest class in *prototypes for *test
	return:
		class id
*/
{
	double tempFitness;

	int i,
		 bestCount = 0;

	int   bestPos = 0;
	double bestFit = prototypes[0]->distance(weights, test );

	// calculate fitness for all classes and neighrest neighbour
	for(i=1; i < cPrototypes; i++ )
	{
		tempFitness = prototypes[i]->distance(weights, test );
		if( tempFitness < bestFit )
		{
			bestFit = tempFitness;
			bestPos = i;
		}
		if( tempFitness < (bestFit + 0.01))
		{
			bestCount++;
		}

	}
	if(bestCount && 0)
	{
		printf("bestCount %d\n",bestCount);
	}

	return bestPos;
}
double KNNTestSet(int cWeights,
				  TGene *weights)
/*
	do knn for complete testset
	return 
		# of correct matches/# testset
	remarks
		can be used to optimize *weigths  


*/
{
	double matches = 0;
	int i;
	double res, temp;

	for(i=0; i < nTESTSET; i++ )
	{

		matches += KNNTest( cWeights,
					weights,
					nPROTOTYPES,
					PROTOTYPES,
					TESTSET[i]);
	}
	if( nTESTSET )
		temp = nTESTSET;
	else temp = 1;

	res = matches / temp;
	return res;
}

#endif


double KNNTestSet(int cWeights,
                  double *weights)
{
    return 0;
}
