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

//#include "utils.h"
#include "PSS_SYNCHROT21_interfaces.h"
#include "PSS_SYNCHROT21_functions.h"
#include "PSS_SYNCHROT21.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="PSS_SYNCHROT21";
int run_times=1;
int subframe_length=14*128;
char plot_modeIN[STR_LEN]="DEFAULT";
char plot_modeOUT[STR_LEN]="DEFAULT";
float samplingfreqHz=1.0;
#define DEBUGG	1		// o: no print, 1: print
int debugg=DEBUGG;

//Module User Defined Parameters
//Global Variables
//PSS
int pss_length=PSSLENGTH;
_Complex float PSStime[PSSLENGTH];
_Complex float PSSTIME[MAXFFTSIZE];
_Complex float PSSfreqRotated[MAXFFTSIZE];
int PSS_idx=0; 	//0, 1 or 2
int TxRxMode=-1; //TX: 1, RX: -1

//FFTs
#define FFTMAXSZ	2048
fftw_complex fftin[FFTMAXSZ], fftout[FFTMAXSZ];
fftw_plan fftplan128;
fftw_plan ifftplan128;
int FFTsize=128;
int CP=0;
int FFTCPsize=128;				//FFTsize+CP;

// Circular buffer
//#define HALFBUFF	32768
#define BUFFERSIZE	64*1024

// Correlation
_Complex float inbuff[BUFFERSIZE];
#define OFDMSINSUBFRAME	14
_Complex float CorrResult[BUFFERSIZE];
_Complex float CorrResultP[BUFFERSIZE];
_Complex float CorrResultP2[BUFFERSIZE];
float variance=0.0;

/**CFO Functions */
float CFOdetectdegrees=0.0;		// CFO Base Phase
float IncCFO=0.0;						// CFO Phase Variation 

// CHANNEL ESTIMATION
_Complex float CH_ESTIMATION[2048];


/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	/* Get control parameters*/
	param_get_int("subframe_length", &subframe_length);		//Initialized by hand or config file
	param_get_int("PSS_idx", &PSS_idx);
	param_get_float("samplingfreqHz", &samplingfreqHz);

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      subframe_length=%d, sampligfreqHz=%3.3f\n", subframe_length, samplingfreqHz);
	printf("O--------------------------------------------------------------------------------------------O\n");

	// Generate PSS time at RX
	genPSStime_seq(PSS_idx, FFTsize, PSSTIME, TxRxMode);
	genPSSfreq_seq_rotated(PSS_idx, FFTsize, PSSfreqRotated, -TxRxMode);
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

	int rcv_samples = get_input_samples(0); /** number of samples at itf 0 buffer */
	int snd_samples=0, snd_samples1=0, snd_samples2=0, snd_samples3=0;
	_Complex float *input, *corr_out, *CHestimator, *graph2, *datout, *dataout3, newphase=0.0;
	static int Tslot=-1;
	int i,k, l, j; 
	static int flagINITRX=0;
	int pMAX1=-1;
	int pMAX1_0=0, pMAX1_1=0, pMAX0, pMAX2;
	static float gain=1.0;
	static int firstTX=0;
	float RotateDegrees;
	int Pdummy;

	input=inp;
	datout=out; //out(out, 0);
	corr_out=out(out, 1);
	CHestimator=out(out, 2);
	dataout3=out(out, 3);


	// INCREASE TSLOT COUNTER
	Tslot++;
	// DO NOTHING 
	if(rcv_samples==0)return 0;
	// CHECK NUMBER OF RECEIVED SAMPLES: SHOULD BE A SUBFRAME
	if(rcv_samples != subframe_length){
		printf("PSS_SYNCHO:work() ERROR: rcv_samples=%d != subframe_length=%d\n", rcv_samples, subframe_length);
		return(0);
	}

	// NORMALIZE INPUT DATA: CAG
	gain_vectorCPLX(inp, inbuff, rcv_samples, gain);

	// CORRELATE INPUT SAMPLES WITH PSS*
	stream_conv_CPLX(inbuff, rcv_samples, PSSTIME, FFTsize, CorrResult);

	// COMPUTE ABS() VALUE OF EACH SAMPLE
	process_correllation(CorrResult, rcv_samples, CorrResultP);

	// COPY CORRELATION RESULTS INTO OUTPUT1 
	memcpy(corr_out, CorrResult, sizeof(_Complex float)*rcv_samples);
	snd_samples1=rcv_samples;

	// DETECT PSS USING ABS() SAMPLES: if pMAX1 >= 0 ==> PSS DETECTED
	pMAX1=detect_PSS(CorrResultP, rcv_samples, &variance);

	// IF PSS DETECTED COMPUTE CAG AND CFO
	if(pMAX1 >= 0){
		// UPDATE CAG GAIN VALUE
		gain *= compute_in_gainAver(inbuff, rcv_samples, pMAX1);
		if(debugg)printf("\033[1;31m Tslot=%d PSS DETECTED!!!             %s: flagINITRX=%d, rcv_samples=%d, pMAX1=%d, gain=%3.6f \033[0m\n", 
																		Tslot, mname, flagINITRX, rcv_samples, pMAX1, gain);
		// DETECT CF0
		IncCFO=checkPhaseOffset(&CorrResult[0]+pMAX1+1);
		CFOdetectdegrees = IncCFO; //CFOdetectdegrees - IncCFO; ///2.0;
//		printf("Correct phase by rotating  -%3.2f degrees\n", (float)(CFOdetectdegrees));

		// SEND NORMALIZED PSS RECEIVED TO CHANNEL ESTIMATOR
/*		if(pMAX1>=FFTsize){
			gain_vectorCPLX(&inp[0]+pMAX1-FFTsize, CHestimator, FFTsize, gain*0.1);
			//memcpy(CHestimator, inbuff, sizeof(_Complex float)*FFTsize);	
			snd_samples2=FFTsize;
		}
*/
	} 

	// CORRECT CFO
//	printf("Correct phase by rotating  -%3.2f degrees\n", (float)(CFOdetectdegrees));
	rotateCvector(inbuff, inbuff, rcv_samples, -CFOdetectdegrees);


	// CAPTURE NORMAL SUBFRAME
	if(flagINITRX == 1){

		if(write_subframe_buffer(inbuff, pMAX1, rcv_samples, 128) == -1)return -1;
		snd_samples3=rcv_samples;
		memcpy(dataout3, inbuff, sizeof(_Complex float)*(rcv_samples));
	}
	// CAPTURE FIRST SUBFRAME STAGE
	if(flagINITRX == 0){
		if(pMAX1>0){
			pMAX1_0=pMAX1;
			pMAX1=detect_2onPSS(CorrResultP, rcv_samples, pMAX1, &variance);

		}
		if(pMAX1>0){
			flagINITRX = 1;
			pMAX1_1=pMAX1;
			if(debugg)printf("\033[1;37;41m DOUBLE PSS DETECTED!!!              %s: flagINITRX=%d, rcv_samples=%d, pMAX2=%d, gain=%3.6f \033[0m\n", 
																						mname, flagINITRX, rcv_samples, pMAX1, gain);
			if(write_subframe_buffer(inbuff, pMAX1, rcv_samples, 128) == -1)return -1;
			// COPY TO OUTPUT3 INCLUDING DOUBLE PSS
			if(pMAX1_0 > pMAX1_1)pMAX1_1=pMAX1_0;
			
			if(pMAX1_1>=256){
				snd_samples3=rcv_samples-(pMAX1_1-256);
				memcpy(dataout3, inbuff+pMAX1_1-256, sizeof(_Complex float)*(snd_samples3));
			}else {
				printf("%s: ERROR!!! pMAX1_1=%d < 256\n", mname, pMAX1_1);
				exit(0);
			}

//			snd_samples3=rcv_samples;
//			memcpy(dataout3, inbuff, sizeof(_Complex float)*(snd_samples3));
		}else{
			snd_samples=0;	
		}
	}
	// CALCULATE THE NUMBER OF SAMPLES TO SENT
	snd_samples=read_subframe_buffer(datout, 128);


	//printf("%s OUT Tslot=%d: snd_samples=%d, snd_samples1=%d, snd_samples2=%d\n", 
	//					mname, Tslot, snd_samples, snd_samples1, snd_samples2);

	set_output_samples(1, snd_samples1);		//	set_output_samples(N, out_samples_at_N_port);
	set_output_samples(2, snd_samples2);		//	set_output_samples(N, out_samples_at_N_port);
	set_output_samples(3, snd_samples3);		//	set_output_samples(N, out_samples_at_N_port);
	// Indicate the number of samples at output 0 with return value
	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


