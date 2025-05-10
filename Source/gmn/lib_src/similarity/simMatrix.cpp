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

akjshdkajshd






template <class T, class U>
CSimMatrix<T,U>::CSimMatrix(int x, int y)
{
	xI = x;
	yI = y;
	tableV = NULL; 
	vectorX = NULL;
	vectorY = NULL;
}

template <class T, class U>
CSimMatrix<T,U>::~CSimMatrix( void )
{
	if( tableV )
		delete [] tableV;
}

template <class T, class U>
double CSimMatrix<T,U>::V(int x, int y)
{
	if( tableV &&
		x < xI &&
		y < yI )
	{
		return tableV[x*yI + y];
	}
	printf("range error\n");
	return 0;
}


template <class T, class U>
void CSimMatrix<T,U>::V(int x, int y, double val)
{
	if( tableV &&
		x < xI &&
		y < yI )
	{
		tableV[x*yI + y] = val;
	}

}

template <class T, class U>
void CSimMatrix<T,U>::setXVector(T *val)
{
	vectorX = val;
}
template<class T, class U>
void CSimMatrix<T,U>::setYVector(U *val)
{
	vectorY = val;
}

template <class T, class U>
void CSimMatrix<T,U>::calcTable( int windowSize,
						    int stepSize)
{
	tableV = new double[xI*yI];

	int x,y;

	// init
	for( y = 0; y < yI; y++ )
	{
		for( x = 0; x < xI; x++ )
		{
			V(x,y,0);
		};
	}

	if( vectorY == NULL ) // only corner needed!
	{
		for( y = 0; y+windowSize-1 < yI; y += stepSize )
		{
			for( x = 0; x < y; x += stepSize )
			{
				V(x,y,sim(vectorX[x],vectorX[y],windowSize) );
			}
		}
	}
	else
	{
		for( y = 0; y+windowSize-1 < yI; y += stepSize )
		{
			for( x = 0; x+windowSize-1 < xI; x += stepSize )
			{
				V(x,y,sim(vectorX[x],vectorY[x],windowSize) );
			}
		}
	}


}

template <class T, class U>
void CSimMatrix<T,U>::debug(FILE *out)
{
	char mustClose = 0;
	if( !out )
	{
		out = fopen("_simmatrix.txt","wt");
		mustClose = 1;
	}

	int x, y;
	if( vectorX )
	{
		fprintf(out,"xxxxxx ");
		for( x = 0; x < xI; x++ )
			fprintf(out,"%3.3f ", val(vectorX[x]));
		fprintf(out, "\n");
		fprintf(out, "\n");

	}


	if( vectorY )
	{
		fprintf(out,"xxxxxx ");
		for( y = 0; y < yI; y++ )
			fprintf(out,"%3.3f ", val(vectorY[y]));
		fprintf(out, "\n");
		fprintf(out, "\n");
	}


	// exact output
	int xMax = xI;
	for( y = 0; y < yI; y++ )
	{
		// do only diagonal?
		if( !vectorY )
		{
			xMax = y;
			fprintf(out,"%3.3f| ", val(vectorX[y]));
		}
		else
		{
			fprintf(out,"%3.3f| ", val(vectorY[y]));
		}

		for( x = 0; x < xMax; x++ )
		{
			fprintf(out, "%3.3f ", V(x,y));
		}
		fprintf(out, "\n");
	}
	fprintf(out, "\n");

	// summary outPut
	xMax = xI;
	int value;
	for( y = 0; y < yI; y++ )
	{
		// do only diagonal?
		if( !vectorY )
		{
			xMax = y;
		}

		for( x = 0; x < xMax; x++ )
		{
			value = (int)(V(x,y)*10);
			if( value > 5 )
				fprintf(out, "%d", value-1);
			else
				fprintf(out, ".");

		}
		fprintf(out, "\n");
	}
	fprintf(out, "\n");


	if( mustClose )
		fclose( out );
}

template <class T, class U>
Cxy *CSimMatrix<T,U>::getBestIDs( double limit )
{
	int xMax = xI,
				y,
				x,
				cur = 0;

	Cxy *res;
	if( vectorX )
		res = new Cxy[xI*yI+1];
	else
		res = new Cxy[(xI*yI)/2+1];


	int yMin;
//	if( vectorY )
	// look diagonal by diagonal
	int xStart,
		yStart;
	if( vectorY )
	{
		xStart = xI-1;
		yStart = 0;
	}
	else
	{
		xStart = 0;
		yStart = 1;
	}
	int prevMaxPos; // diag distance to preMax
	while(yStart < yI )
	{
		prevMaxPos = 0;
		
		x = xStart;
		y = yStart;
		// do one diagonal
		while( x < xI &&
			   y < yI )
		{
			if(V(x,y) >= limit )
			{	
//				if(prevMaxPos != -1)
				{
					res[cur].x = x;
					res[cur].y = y;
					cur++;
					prevMaxPos = -1;
				}
/*				else // better than prev?
				{
					if(V(x,y) > V(res[cur-1].x,
								  res[cur-1].y) )
					{
						// overwrite prev
						res[cur-1].x = x;
						res[cur-1].y = y;
					}
					else // skip this max
					{
					}
				}
*/
			}
//			printf("%d,%d\n",x,y);
			x++;
			y++;
		} // while

		if( xStart > 0 )
			xStart--;
		else
			yStart++;
		
	} // while
/* does not work, see all items more then once
	else
	{
		int y2;
		// do y/1--y/y-1---yI-1/y-1
		for( y = 1; y < yI; y++ )
		{
			// go to right until diag
			for( x = 0; x < y; x++ )
			{
				if(V(x,y) >= limit )
				{	
					res[cur].x = x;
					res[cur].y = y;
					cur++;
				}
			}
			// go down column
			x--;
			for( y2 = y+1; y2 < yI; y2++ )
			{
				if(V(x,y2) >= limit )
				{	
					res[cur].x = x;
					res[cur].y = y2;
					cur++;
				}
			}

		} // for y
	}
	*/
	// mark end of list
	res[cur].x = -1;
	res[cur].y = -1;

	return res;
}
