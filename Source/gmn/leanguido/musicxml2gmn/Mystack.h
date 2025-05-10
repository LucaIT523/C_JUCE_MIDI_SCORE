#ifndef _Mystack_h_
#define _Mystack_h_

#include <sstream>
using namespace std;



const int chunkSize = 50;

class lgTag;
class lgSequence;
string typeName(lgTag *tag);
string typeName(const char *); 
int typeID(lgSequence *sq);
//template<class SType> string typeName(SType);

template<class SType> class Mystack{
	SType *stck;
	int tos,
		SIZE;
		SType lookAt( int i );
	public:
		Mystack(void);
		virtual ~Mystack( void )
		{
			delete [] stck;
		}
		
		SType searchByName(const char * name);
		void push(SType i);
		SType peek(int i = -1);
		int remove( SType val);
		SType pop(void);
		bool isEmpty();
		void clear(void);
		void append(SType x, int i);
		SType searchByID(int id);
		int size( void )
		{
			return tos;
		}
		//void destroy(void);

};


template<class SType> SType Mystack<SType>::lookAt( int i )
{
	if( i >= 0 && i < tos )
	{
		return stck[i];
	}
	return NULL;
}

template<class SType> void Mystack<SType>:: clear(void)
{
	for(int i=0; i<SIZE; i++)
		stck[i]=NULL;
	tos=0;
}

template<class SType> int Mystack<SType>::remove( SType val )
{
	int res = 0;
	for( int i = 0; i < tos; i++ )
	{
		if( val == stck[i] )
		{
			res = 1;
		}
		if( res == 1 && i < SIZE)
		{
			stck[i] = stck[i+1];
		}
	} // for
	if( res )
		tos--;
	return tos;
}


template<class SType> Mystack<SType>:: Mystack()
{
	SIZE = chunkSize;
	stck=new SType[SIZE];
	if(stck)
	{
            tos=0;
            clear();
	}
	else
	{
		printf("there's not enough place for initialize objectstack");
		exit(0);
	}
}

template<class SType> void Mystack<SType>::push(SType i)
{	
	if(tos>=SIZE)
	{
            /// increase stacksize and copy existing entries
            SType *newStck=new SType[SIZE+ chunkSize];
            for( int j = 0; j < SIZE; j++ )
            {
                newStck[j] = stck[j];
            }
            delete [] stck;
            SIZE += chunkSize;
            stck = newStck;
        }
		// neu 23.2.04
		/*
			in allen Abfragen SIZE durch curSize ersetzen
			// neuen Speicher
			SType *temp;
			temp = new[chunkSize + SIZE];
			kompletten Stack nach temp kopieren

			SIZE += chunkSize;
			
			delte [] stck;
			stck = temp;
			
			dann kein else, kein return
		*/
/*
		return;
	}
	else 
 */
	{
		stck[tos++]=i;
		
	}
}

/*
template<class SType> void Mystack<SType>::append(SType x, int i)
{
	if( i < 0 || i >= SIZE) return;
	stck[i]=x;
	tos=i;
}
*/

template<class SType> SType Mystack<SType>:: pop(void)
{
	if(isEmpty()) 
            return NULL;
	
	SType temp = stck[tos];
	stck[--tos]= NULL;
	return temp;
}

template<class SType>bool Mystack<SType>::isEmpty(void)
{
    return(tos==0);
}

template<class SType>SType Mystack<SType>::peek(int pos)
{
	if( pos >= 0 )
		return lookAt(pos);
        
        // else get top of stack
	if(isEmpty()) 
        {
            return NULL;
        }
	else
	{
		int i = tos-1;
		return stck[i];
	}
}

template<class SType> SType Mystack<SType>::searchByName(const char *name)
{
	if(isEmpty()) return NULL;
    
	if(!name)
		return NULL;
	
	for(int i=0; i<tos; i++)
	{
		const char* tmp;int x=strlen((typeName(stck[i])).c_str());
		int y=strlen(name);
		if(x>y)
                    tmp=strstr(typeName(stck[i]).c_str(), name);	
		else 
                    tmp=strstr(name, typeName(stck[i]).c_str());
		
		if(tmp) 
                    return stck[i];
	} // for
	 return NULL;
}





#endif