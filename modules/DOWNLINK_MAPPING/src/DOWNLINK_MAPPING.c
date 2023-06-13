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

#include "DOWNLINK_MAPPING_interfaces.h"
#include "DOWNLINK_MAPPING_functions.h"
#include "DOWNLINK_MAPPING.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="DOWNLINK_MAPPING";

//Module User Defined Parameters
MODparams_t oParam={NORMAL_NORS, DEBUGPRINTMODE, 0, 0, 0, NOT2SEND, 0, 0, 128};	

//Global Variables
MODvars_t oVars={S_INIT, A_ZEROSF, 0, 0};

unsigned long int Tslot=-1;
#define FFTSIZE	128
int pss_length=PSSLENGTH;
_Complex float PSSseq[PSSLENGTH];

int TxRxMode=1; 					//TX: 1, RX: -1
int DATAsize=72;
int PSS_idx=0; 						//0, 1 or 2
_Complex float PSSfreqseq[FFTSIZE];



#define BUFFER_SZ	2048*10
//_Complex float buffer[BUFFER_SZ];
_Complex float bufferA[BUFFER_SZ];
_Complex float aux[BUFFER_SZ];



/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {
	int i;

	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
	param_get_int("opMODE", &oParam.opMODE);
	param_get_int("debugMODE", &oParam.debugMODE);
	param_get_int("numchars7FFT", &oParam.numchars7FFT);
	param_get_int("numchars13FFT", &oParam.numchars13FFT);
	param_get_int("numchars14FFT", &oParam.numchars14FFT);
	param_get_int("numTSNOT2SEND", &oParam.numTSNOT2SEND);
	param_get_int("numTSChainDelay", &oParam.numTSChainDelay);
	param_get_int("phylayerID", &oParam.phylayerID);
	param_get_int("FFTsz", &oParam.FFTsz);

	// INIT LTE VARS
	if(oParam.FFTsz==128)oVars.ActiveCarriers=6*12;
	if(oParam.FFTsz==256)oVars.ActiveCarriers=15*12;
	if(oParam.FFTsz==512)oVars.ActiveCarriers=25*12;
	if(oParam.FFTsz==1024)oVars.ActiveCarriers=50*12;
	if(oParam.FFTsz==1536)oVars.ActiveCarriers=75*12;
	if(oParam.FFTsz==2048)oVars.ActiveCarriers=100*12;


	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());

	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      opMODE=%d [0-NORMAL_NORS, 1-NORMAL_WITHRS, 2-BYPASS, 3-DEBUGG]\n", 
					oParam.opMODE);
	printf("O      debugMODE=%d [1-NO ACTIVE, 2-PRINT MODE, 3-INPUT RAMP, 4-INPUT ALL ZEROS, 5-BYPASS]\n", 
					oParam.debugMODE);
	printf("O      numTSNOT2SEND=%d, numTSChainDelay=%d\n", 
					oParam.numTSNOT2SEND, oParam.numTSChainDelay);
	printf("O      numchars7FFT=%d, numchars13FFT=%d, numchars14FFT=%d\n", 
					oParam.numchars7FFT, oParam.numchars13FFT, oParam.numchars14FFT);
	printf("O      phylayerID=%d, FFTsz=%d\n", 
					oParam.phylayerID, oParam.FFTsz);
	printf("O--------------------------------------------------------------------------------------------O\n");

	//CREATE PSS SEQUENCE
	PSS_idx=oParam.phylayerID%3;
	setPSS(PSS_idx, PSSseq, TxRxMode);
	//CREATE PSS SEQUENCE IN FREQ DOMAIN
	genPSSfreq_seq(PSS_idx, oParam.FFTsz, PSSfreqseq, TxRxMode);

	//CREATE TEST SEQUENCE
	if(oParam.debugMODE==DEBUG_IN_RAMP){
		for(i=0; i<2048; i++)bufferA[i]=(((float)(i%72))/72.0+0.25+0.0*I);
	}
	if(oParam.debugMODE==DEBUG_IN_ALLZERO){
		for(i=0; i<2048; i++)bufferA[i]=0.0+0.0*I;
	}
	// INITIALIZE CONTROL
	//control_SFrame(&oVars, &oParam, 0);

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
	int rcv_samples0 = get_input_samples(0); /** number of samples at itf 0 buffer */
	int snd_samples0=0, snd_samples1=1, snd_samples2=0;

	input_t *input0;
	input0=in(inp,0);
	output_t *output0;
	output0=out(out,0);
	unsigned int *output1;
	output1=out(out,1);
	_Complex float *output2;
	output2=out(out,2);

	int outval=0, i;

	Tslot++;

	if((oParam.debugMODE == DEBUG_IN_ALLZERO) || (oParam.debugMODE == DEBUG_IN_RAMP) || (oParam.debugMODE == DEBUG_IN_ALLONES)){
		if(rcv_samples0>0)printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Tslot=%d, rcv_samples0=%d\n", Tslot, rcv_samples0);
		for(i=0; i<rcv_samples0; i++)*(input0+i)=bufferA[i];
	}

	if(oParam.opMODE == BYPASS){
		memcpy(output0, input0, sizeof(_Complex float)*rcv_samples0);
		snd_samples0=rcv_samples0;
		if(rcv_samples0>0)for(i=0; i<6; i++)printf("%2.2f+%2.2fi\n", __real__ *(input0+i), __imag__ *(input0+i));

	}
/////////////////////////////////////////////////////////////////////////
	// NORMAL MAPPING WITHOUT ADDING RS
	if(oParam.opMODE == NORMAL_NORS){

		// CONTROL OF SUBFRAME
		outval=control_SFrame(&oVars, &oParam, rcv_samples0);

		// GENERATE SUBFRAME
		snd_samples0=generate_LTE_subframe(&oVars, &oParam, 
								input0, rcv_samples0, 
								PSSseq,
								output0,
								mname);
	}

/////////////////////////////////////////////////////////////////////////

	// NORMAL MAPPING ADDING RS
	if(oParam.opMODE == NORMAL_WITHRS){
		// TODO
		// CONTROL OF SUBFRAME
		outval=control_SFrame(&oVars, &oParam, rcv_samples0);
		// AD RS signals
		rcv_samples0=addRSinDataFlow(input0, oParam.FFTsz, 
									rcv_samples0, aux,  oParam.phylayerID);

		// GENERATE SUBFRAME
		snd_samples0=generate_LTE_subframe(&oVars, &oParam, 
								aux, rcv_samples0, 
								PSSseq,
								output0,
								mname);
	}

	if(oParam.opMODE == DEBUGG){
		// CONTROL OF SUBFRAME
		outval=control_SFrame(&oVars, &oParam, rcv_samples0);
		// GENERATE SUBFRAME
		if(oParam.debugMODE == DEBUG_IN_ALLONES){
			for(i=0; i<rcv_samples0; i++)*(input0+i)=1.0+1.0*I;
		}
		snd_samples0=generate_LTE_subframe(&oVars, &oParam, 
								input0, rcv_samples0, 
								PSSseq,
								output0,
								mname);
	}


/*	if(snd_samples0>0)printf("%s>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Tslot=%d, rcv_samples0=%d, snd_samples0=%d\n", 
									mname, Tslot, rcv_samples0, snd_samples0);
*/	
	// SEND CTRL OUTPUT
	*output1=outval;
	set_output_samples(1,snd_samples1);
	// SEND COPY OF DATA FOR OUTPUT2
	// Copy to output 2
	if(rcv_samples0>0){
		snd_samples2=snd_samples0;
		memcpy(output2, output0, sizeof(_Complex float)*snd_samples2);
	}
	set_output_samples(2,snd_samples2);


	return snd_samples0;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


