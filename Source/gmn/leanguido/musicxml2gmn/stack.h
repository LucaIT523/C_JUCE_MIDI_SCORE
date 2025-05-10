#include <iostream>
#ifndef _stack_h_
#define _stack_h_

const int SIZE=100;
template<class SType> class stack{
	SType stck[SIZE];
	int tos;
	public:
		stack(void);
		
		void push(SType i);
		SType peek(void);
		void pop(void);
		bool isEmpty();
		//void destroy(void);

};

#endif