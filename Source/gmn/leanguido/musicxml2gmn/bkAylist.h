#ifndef _bkAylist_h_

#define _bkAylist_h_

#include <stdio.h>

#include <string>

#include "bkList.h"

using namespace std;



const int Vsize=50;



class bkAylist

{

	

	

public:

	int size;

	bkList s[Vsize] ;

	bkAylist(void)

	{

		size=Vsize;

		//s= new bkList[size];

		for(int i=0; i<size;i++)

		{

			bkList *temp= new bkList();

			s[i] = *temp;

		}

	};



	virtual ~bkAylist(void)

	{

		delete [] &s;

	};

	

	bkList element(int x)

	{

		if(x<0||x>=size) return NULL;

		return s[x];		

	};



	void appendNote(bkPosition *x, int i)

	{

		if(i<0||i>=size)

		{

		/*	if(i>=0)

			{

				int a=size+Vsize;

				while(i>=a) a+=Vsize;

				bkList tmp[a];

				

				for(int j=0; j<size; j++)

				tmp[j]=s[j];



					for(int k=size; k<a;k++)

					{

						bkList *temp= new bkList();

						tmp[k] = *temp;

					}

				

				tmp[i].setNext(x);

				

				delete [] s;

			

				s= tmp;

			}



			else return;*/

			return;

		}

		else s[i].setNext(x);

	};

	

	

};

#endif