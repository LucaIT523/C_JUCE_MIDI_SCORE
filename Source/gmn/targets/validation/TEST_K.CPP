#include "hpattern.h"
#include "statist.h"
#include "../C/ini/ini.h"

int myAssert( int b )
{

	if( !b )
	{
		printf("Assertion violation!\n");
		exit(1);
	}
	return 1;
}
void ShowErrorMsg( char *s)
{
	printf(s);
}
void main()
{
/* Test k_Array for patternpool */
{

	TDecayAverage *flAverage;
	TDecayStruct decayStruct[6];

	decayStruct[0].decay = 0.9;
	decayStruct[0].range = 4;
	decayStruct[1].decay = 0.5;
	decayStruct[1].range = 4;
	decayStruct[2].decay = 0.3;
	decayStruct[2].range = 4;

	decayStruct[3].decay = 0.9;
	decayStruct[3].range = 2;
	decayStruct[4].decay = 0.5;
	decayStruct[4].range = 2;
	decayStruct[5].decay = 0.3;
	decayStruct[5].range = 2;

	flAverage = new TDecayAverage(6, decayStruct);
	#define SIZE 20
//	long int values[SIZE] = {1,2,3,4,5,6,7,8,9,10,10,9,8,7,6,5,4,3,2,1};
	long int values[SIZE] = {1,2,2,4,10,3,3,4,5,10,10,9,8,3,4,5,4,3,6,2};
//	long int values[SIZE] = {20,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,20};
	int i;
	FILE *out;
	out = fopen("avergae.txt","wt");
	for(i = 0; i < SIZE; i++ )
	{
		flAverage->addValue(values[i]);
	}
	flAverage->writeValues(out);
	i = 0;
	while( flAverage->iterate(i) )
	{
		flAverage->writeAvs(out);
		i = 1;
	}
	fclose(out);

	delete decayStruct;
//	return;
	out = fopen( "k_array.txt","wt");
	THPFile *temp;
	temp = new THPFile( "Pool.pat" );
	if( !temp )
		exit(0);
	temp->Read(480);
	THPattern *tempPattern;
	tempPattern = (THPattern*)(temp->First());
	TFeatures K_Array;
	while( tempPattern )
	{
		K_Array = tempPattern->Features();
		K_Array.Write(out);
		tempPattern = (THPattern*)(temp->Next());
	}
	fclose( out );
	delete( temp );
}

}
