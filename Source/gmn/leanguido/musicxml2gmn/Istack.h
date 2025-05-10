#ifndef _Istack_h_
#define _Istack_h_

const int size=40;


class Istack
{
		
	int stck[SIZE];
	int tos;
	public:
		Istack(void);
		void push(int i);
		int peek(void);
		void pop(void);
		bool isEmpty();
		bool isInclude(int x);
};
#include <sstream>
#include "Istack.h"

Istack:: Istack(void)
{
	tos=0;
	for(int i=0; i<SIZE; i++)
		stck[i]=0;
}

void Istack:: push(int i)
{
	
	if(tos>=SIZE) return;
	else 
	{
		stck[tos++]=i;
		
	}
}

void Istack:: pop()
{
	if(isEmpty()) return ;
	else 
	{
		stck[--tos]=0;
		 // not return stck[tos--]!!!
	}

}

bool Istack::isEmpty()
{
		return(tos==0);
}

int Istack::peek()
{
	if(isEmpty()) return 0;
	else
	{
		int i=tos-1;
		return stck[i];
	}
}

bool Istack:: isInclude(int x)
{
	if(isEmpty()||x==0) return false;
	
	for(int i=0; i<tos; i++)
	{
		if(x==stck[i]&&(x!=0)) return true;
		
	}

	 return false;
}

#endif