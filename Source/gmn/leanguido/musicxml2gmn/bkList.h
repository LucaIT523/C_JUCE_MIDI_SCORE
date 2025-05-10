#ifndef _bkList_h_

#define _bkList_h_

#include <stdio.h>

#include <string>

#include "bkPosition.h"

using namespace std;



class bkList

{

	

	

	

public:

	

	bkPosition *head;

	bkPosition *end;

	bkList()

	{

		head=NULL;

		end=NULL;

	};



	bkList(bkPosition *init)

	{



			head=init;

			end=init;

	};



	void setNext(bkPosition *a)

	{

		if(!head)

		{

			head=a;

			end=a;

		}

		else if(!end)

		{	

			printf("this List's end is null but head is not--error");

			exit(0);

		}

		else 

		{

			end->setNext(a);

			end=a;

		}



		

	};



	bkPosition* first()

	{

		return head;

	};



	bkPosition* last()

	{

		return end;

	};



	bool isEmpty()

	{

		if((!head)&&(!end)) return true;

		else return false;

	};

};



#endif