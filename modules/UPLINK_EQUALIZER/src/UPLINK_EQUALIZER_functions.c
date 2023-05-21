/* 
 * Copyright (c) 2012
 * This file is part of ALOE (http://flexnets.upc.edu/)
 * 
 * ALOE++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ALOE++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with ALOE++.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Functions that generate the test data fed into the DSP modules being developed */
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>

#include "UPLINK_EQUALIZER_functions.h"

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
#define MAXVAL	15.0
int equalize(_Complex float *datain, int datalength, _Complex float *dataout, _Complex float *channel){
	int i, j=0;
	for(i=0; i<datalength; i++){
//		*(dataout+i)= *(datain+i)+(*(channel+j));
		*(dataout+i)=*(datain+i)*(*(channel+j)); //Version0: Complex

		if(__real__ *(dataout+i) > MAXVAL)__real__ *(dataout+i)=MAXVAL;
		if(__imag__ *(dataout+i) > MAXVAL)__imag__ *(dataout+i)=MAXVAL;
		if(__real__ *(dataout+i) < -MAXVAL)__real__ *(dataout+i)=-MAXVAL;
		if(__imag__ *(dataout+i) < -MAXVAL)__imag__ *(dataout+i)=-MAXVAL;
		j++;
		if(j==256)j=0;
	
}
	return(datalength);
}

