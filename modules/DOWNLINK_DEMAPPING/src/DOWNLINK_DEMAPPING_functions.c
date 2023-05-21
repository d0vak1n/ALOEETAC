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

#include "DOWNLINK_DEMAPPING_functions.h"

#define CARRIERS1RB	12

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
int getDATAfromLTE_UPLINKspectrum(_Complex float *LTEspect, int FFTlength, int datalength, _Complex float *QAMsymb){
	int i, j;

	j=FFTlength-datalength/2;
	for(i=0; i<datalength; i++){
		*(QAMsymb+i)=*(LTEspect+j);
		j++;
		if(j==FFTlength)j=0;
	}
	return(1);
}

int getDATAfromLTE_DOWNLINKspectrum(_Complex float *LTEspect, int FFTlength, int datalength, _Complex float *QAMsymb){
	int i, j;

	j=FFTlength-datalength/2;
	for(i=0; i<datalength; i++){
		*(QAMsymb+i)=*(LTEspect+j);
		j++;
		if(j==FFTlength)j=1;
	}
	return(1);
}




int rmoveRSinOFDMsymb(_Complex float *OFDMsymb, int nofcarriers, 
										 _Complex float *out_data, int phylayerID){

	int i, k=0, j;

	j=0;
//	printf("nofcarriers=%d, phylayerID=%d\n", nofcarriers, phylayerID);
	for(i=0; i<nofcarriers; i++){
		if(i%3 != phylayerID){
//			printf("0 i=%d, j=%d\n", i, j);
			*(out_data+j)=*(OFDMsymb+i);
//			printf("1 i=%d, j=%d\n", i, j);
			j++;
		}
	}
	return(j);	//Return the data extracted
}


// Return the number of Output Data
int extractDataFromOFDMflowWithRS(_Complex float *OFDMsymb, int rcv_samples, int FFTsize, 
										 _Complex float *out_data, int phylayerID){
	
	int i, numdata;		
	int numFFTs=rcv_samples/FFTsize;
	static int first=0;
	static int nofcarriers=0;
	_Complex float aux[2048*14];
	int inCounter=0, outCounter=0;


	if(first==0){
		if(FFTsize==128)nofcarriers=3*4*6;
		first=1;
	}

//	printf("extractDataFromOFDMflowWithRS(): rcv_samples=%d, nofcarriers=%d\n", rcv_samples, nofcarriers);

	if (rcv_samples != numFFTs*FFTsize) {
		printf("DEMAPPING.extractDataFromOFDMflowWithRS(): ERROR!!! Received samples=%d is not multiple of FFTsize=%d\n", 
				rcv_samples, FFTsize);
		return(-1);
	}

//	printf("extractDataFromOFDMflowWithRS(): numFFTs=%d\n", numFFTs);
	// GET DATA FROM ACTIVE CARRIERS
	for(i=0; i<numFFTs; i++){
		getDATAfromLTE_DOWNLINKspectrum(&OFDMsymb[FFTsize*i], FFTsize, nofcarriers, &aux[nofcarriers*i]);
	}
	// 7 FFTs
	if(numFFTs == 7){
		printf("7\n");
		outCounter += rmoveRSinOFDMsymb(&aux[0], nofcarriers, &out_data[0], phylayerID);
		inCounter += nofcarriers;
		memcpy(out_data+outCounter, aux+inCounter, sizeof(_Complex float)*3*nofcarriers);
		inCounter += nofcarriers*3;
		outCounter += nofcarriers*3;
		outCounter += rmoveRSinOFDMsymb(aux+inCounter, nofcarriers, out_data+outCounter, phylayerID);
		inCounter += nofcarriers;
		memcpy(out_data+outCounter, aux+inCounter, sizeof(_Complex float)*2*nofcarriers);
		inCounter += nofcarriers*2;
		outCounter += nofcarriers*2;
	}

	// 14 FFTs
	if(numFFTs == 14){
//		printf("14\n");
//		printf("0 inCounter=%d, outCounter=%d\n", inCounter, outCounter);
		outCounter += rmoveRSinOFDMsymb(&aux[0], nofcarriers, &out_data[0], phylayerID);
		inCounter += nofcarriers;
//		printf("1 inCounter=%d, outCounter=%d\n", inCounter, outCounter);
		memcpy(out_data+outCounter, aux+inCounter, sizeof(_Complex float)*3*nofcarriers);
		inCounter += nofcarriers*3;
		outCounter += nofcarriers*3;
//		printf("2 inCounter=%d, outCounter=%d\n", inCounter, outCounter);
		outCounter += rmoveRSinOFDMsymb(aux+inCounter, nofcarriers, out_data+outCounter, phylayerID);
		inCounter += nofcarriers;
//		printf("3 inCounter=%d, outCounter=%d\n", inCounter, outCounter);
		memcpy(out_data+outCounter, aux+inCounter, sizeof(_Complex float)*2*nofcarriers);
		inCounter += nofcarriers*2;
		outCounter += nofcarriers*2;
//		printf("4 inCounter=%d, outCounter=%d\n", inCounter, outCounter);
		outCounter += rmoveRSinOFDMsymb(aux, nofcarriers, out_data, phylayerID);
		inCounter += nofcarriers;
//		printf("5 inCounter=%d, outCounter=%d\n", inCounter, outCounter);
		memcpy(out_data+outCounter, aux+inCounter, sizeof(_Complex float)*3*nofcarriers);
		inCounter += nofcarriers*3;
		outCounter += nofcarriers*3;
//		printf("6 inCounter=%d, outCounter=%d\n", inCounter, outCounter);
		outCounter += rmoveRSinOFDMsymb(aux+inCounter, nofcarriers, out_data+outCounter, phylayerID);
		inCounter += nofcarriers;
//		printf("7 inCounter=%d, outCounter=%d\n", inCounter, outCounter);
		memcpy(out_data+outCounter, aux+inCounter, sizeof(_Complex float)*2*nofcarriers);
		inCounter += nofcarriers*2;
		outCounter += nofcarriers*2;
//		printf("8 inCounter=%d, outCounter=%d\n", inCounter, outCounter);
//		printf("14_20\n");
	}
	// 13 FFTs
	if(numFFTs == 13){
		printf("13\n");
		outCounter += rmoveRSinOFDMsymb(aux, nofcarriers, out_data, phylayerID);
		inCounter += nofcarriers;
		memcpy(out_data+outCounter, aux+inCounter, sizeof(_Complex float)*3*nofcarriers);
		inCounter += nofcarriers*3;
		outCounter += nofcarriers*3;
		outCounter += rmoveRSinOFDMsymb(aux+inCounter, nofcarriers, out_data+outCounter, phylayerID);
		inCounter += nofcarriers;
		memcpy(out_data+outCounter, aux+inCounter, sizeof(_Complex float)*1*nofcarriers);
		inCounter += nofcarriers*1;
		outCounter += nofcarriers*1;
		outCounter += rmoveRSinOFDMsymb(aux, nofcarriers, out_data, phylayerID);
		inCounter += nofcarriers;
		memcpy(out_data+outCounter, aux+inCounter, sizeof(_Complex float)*3*nofcarriers);
		inCounter += nofcarriers*3;
		outCounter += nofcarriers*3;
		outCounter += rmoveRSinOFDMsymb(aux+inCounter, nofcarriers, out_data+outCounter, phylayerID);
		inCounter += nofcarriers;
		memcpy(out_data+outCounter, aux+inCounter, sizeof(_Complex float)*2*nofcarriers);
		inCounter += nofcarriers*2;
		outCounter += nofcarriers*2;
	}
	printf("DOWNLINK_DEMAPPING.extractDataFromOFDMflowWithRS(): outCounter=%d\n", outCounter);
	return(outCounter);
}

