/* 
 * Copyright (c) 2012.
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

#include <complex.h>
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <phal_sw_api.h>
#include "skeleton.h"
#include "params.h"

#include "CHANNEL_ESTIMATOR2_interfaces.h"
#include "CHANNEL_ESTIMATOR2_functions.h"
#include "CHANNEL_ESTIMATOR2.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="CHANNEL_ESTIMATOR2";

//Module User Defined Parameters
MODparams_t oParam={FREQ_CHANNEL_ESTIMATION, 256, 100, 7.9, "BABY"};		// Initialize module params struct

//Global Variables
_Complex float bufferA[2048];
float bufferB[2048];

//DMRS
int DMRS_length=DMRSLENGTH;
_Complex float DMRStime[DMRSLENGTH];
_Complex float DMRSTIME[MAXFFTSIZE];
_Complex float DMRSfreqRotated[MAXFFTSIZE];
int DMRS_idx=0; 	//0, 1 or 2
int TxRxMode=-1; //TX: 1, RX: -1

//FFTs
#define FFTMAXSZ	2048
fftw_complex fftin[FFTMAXSZ], fftout[FFTMAXSZ];
fftw_plan fftplan256;
fftw_plan ifftplan256;
int FFTsize=256;
int CP=0;
int FFTCPsize=256;

// CHANNEL ESTIMATION
_Complex float aux[2048];



/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	/* Get control parameters*/
	param_get_int("opMODE", &oParam.opMODE);		//Initialized by hand or config file

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      opMODE=%d\n", oParam.opMODE);
	printf("O--------------------------------------------------------------------------------------------O\n");


	// Generate DMRS time at RX
	genDMRStime_seq(DMRS_idx, FFTsize, DMRSTIME, TxRxMode);
	genDMRSfreq_seq_rotated(DMRS_idx, FFTsize, DMRSfreqRotated, -TxRxMode);

	return 0;
}



/**
 * @brief Function documentation
 *
 * @param inp Input interface buffers. Data from other interfaces is stacked in the buffer.
 * Use in(ptr,idx) to access the address. To obtain the number of received samples use the function
 * int get_input_samples(int idx) where idx is the interface index.
 *
 * @param out Input interface buffers. Data to other interfaces must be stacked in the buffer.
 * Use out(ptr,idx) to access the address.
 *
 * @return On success, returns a non-negative number indicating the output
 * samples that should be transmitted through all output interface. To specify a different length
 * for certain interface, use the function set_output_samples(int idx, int len)
 * On error returns -1.
 *
 * @code
 * 	input_t *first_interface = inp;
	input_t *second_interface = in(inp,1);
	output_t *first_output_interface = out;
	output_t *second_output_interface = out(out,1);
 *
 */
int work(input_t *inp, output_t *out) {
	int i;
	static unsigned long int Tslot=-1;
	int snd_samples, rcv_samples;
	input_t *input0;
	output_t *output0;
	output_t *output1;
	output_t *output2;
	output_t *output3;

	float EVM=0.0;

	// UPDATE TIME SLOT COUNTER
	Tslot++;

	/* GET THE POINTER FOR INPUT AND OUTPUT DATA BUFFERS*/
	input0 = in(inp,0);
	output0 = out(out,0);
	output1 = out(out,1);
	output2 = out(out,2);
	output3 = out(out,3);

	/* GET THE NUMBER OF SAMPLES RECEIVED AT PORT i*/
	rcv_samples=get_input_samples(0);
	if(rcv_samples==0){
return(0);}
	
	
	
	/* PUT HERE YOUR DSP TASKS ///////////////////////////////*/
	if(oParam.opMODE==DEBUGG){
		memcpy(output0, DMRSfreqRotated, sizeof(_Complex float)*rcv_samples);
		snd_samples=rcv_samples;
		
		
	}
	if(oParam.opMODE==BYPASS){
		bypass_CPLX(input0, rcv_samples, output0);
		snd_samples=rcv_samples;
		
	}
	if(oParam.opMODE==FREQ_CHANNEL_ESTIMATION){
			time2freq(256, input0, aux);
			// CHANNEL ESTIMATION
			channel_estimator_freq(aux, DMRSfreqRotated, FFTsize, output0);
			snd_samples=rcv_samples;
		
	}

	// SEND OUT 1: DMRS 
	for(i=0; i<256; i++)*(output1+i)=*(DMRSfreqRotated+i);
	set_output_samples(1, 256);

	// SEND OUT 2: DMRS Received
	for(i=0; i<256; i++)*(output2+i)=*(aux+i);
	set_output_samples(2, 256);	

	// SEND OUT 3: PPS corrected
	for(i=0; i<256; i++){
		*(output3+i)=*(aux+i)*(*(output0+i));
		/*
		if(__real__ *(output3+i) > 1.0)__real__ *(output3+i)=1.0;
		if(__real__ *(output3+i) < -1.0)__real__ *(output3+i)=-1.0;
		if(__imag__ *(output3+i) > 1.0)__real__ *(output3+i)=1.0;
		if(__imag__ *(output3+i) < -1.0)__real__ *(output3+i)=-1.0;
		*/

	}
	set_output_samples(3, 256);	

	// COMPUTE EVM 
	normA(output1, 256);
	normB(output2, 256);
//	EVM=computeEVM_3GGP_LTE256(output1, output2, 256);
//	if(Tslot%10==0)printf("Original EVM=%f\n", EVM);
//	EVM=computeEVM_3GGP_LTE256_2(output3, output1, 256);
//	if(Tslot%10==0)printf("Corrected EVM=%f\n", EVM);

	// COMPUTE EVM CORRECTED



	/* INDICATE THE NUMBER OF OUTPUT SAMPLES AT EACH OUTPUT */
//	set_output_samples(0, snd_samples);								//	set_output_samples(Output_number, number_of_samples);

	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


