#ifndef _bkPosition_h_
#define _bkPosition_h_
#include <stdio.h>
#include <string>
#include "../duration.h"
using namespace std;

class bkPosition
{
	
public:
	lgDuration WrtPos;
	lgDuration Duration;
	bkPosition *Next; 
	bkPosition *prev;
	bool access;
	bool append;
	bkPosition(lgDuration d1, lgDuration d2)
	{
		WrtPos=d1;
		Duration=d2;
		Next=NULL;
		prev=NULL;
		access=false;
		append=false;
	};

	bkPosition(lgDuration d1, lgDuration d2, bkPosition* post)
	{
		WrtPos=d1;
		Duration=d2;
		Next=post;
		post->prev=this;
		prev=NULL;
		access=false;
		append=false;
	};

	void setScorPos(lgDuration d)
	{
		WrtPos=d;
	};
	
	bool isAppend()
	{
		return append;
	};
	
	void appended()
	{
		append=true;
	};

	void setD(lgDuration d)
	{
		Duration=d;
	};

	lgDuration getWrtPos()
	{
		return WrtPos;
	};

	lgDuration getDur()
	{
		return Duration;
	};

	bkPosition* getNext()
	{
		return Next;
	};

	bkPosition* getPrev()
	{
		return prev;		

	};
	void setNext(bkPosition* post)
	{
		Next=post;
		post->prev=this;
	};
	
	bool isAccessed(void)
	{
		return access;
	};

	void accessed()
	{
		access=true;
	};
	
};


#endif