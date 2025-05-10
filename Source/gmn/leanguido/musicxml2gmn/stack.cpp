#include <sstream>
#include "stack.h"

template<class SType> stack<SType>:: stack()
{
	tos=0;
}

/*template<class SType> stack<SType>:: ~stack()
{
	destroy();
}*/

template<class SType> void stack<SType>:: push(SType i)
{
	
	if(tos>=SIZE) return;
	else 
	{
		stck[tos++]=i;
		
	}
}

template<class SType> void stack<SType>:: pop(void)
{
	if(!isEmpty) return;
	else 
	{
		stck[--tos]=0;
		 // not return stck[tos--]!!!
	}

}

template<class SType>bool stack<SType>::isEmpty(void)
{
		return(tos==0);
}

template<class SType>SType stack<SType>::peek(void)
{
	if(!isEmpty) return 0;
	else
	{
		int i=tos-1;
		return stck[i];
	}
}

/*template<class SType> void stack<SType>::destroy(void)
{
	while(isEmpty)
		pop();
}*/