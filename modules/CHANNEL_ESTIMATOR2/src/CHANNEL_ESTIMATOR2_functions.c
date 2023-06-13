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
#include <fftw3.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

#include "CHANNEL_ESTIMATOR2_functions.h"

extern MODparams_t oParam;



/** PRIMARY SYNCH SIGNALS: DMRS*/
/**
 * @brief Function documentation: setDMRS()
 * This function calculates the Zadoff-Chu sequence.
 * @params
 * @params int phylayerID:(0, 1, 2) Physical Layer Identity within the
 * Physical Layer cell-Identity Group.
 * @params  _Complex float *DMRSsymb: Output array.
 * @params int TxRxMode: -1 (Tx Mode), 1 (Rx Mode) .
 *
 * @return On success returns 1.
 * On error returns -1.
 */

int setDMRS(int phylayerID, _Complex float *DMRSsymb, int TxRxMode)
{
	int i;
	double arg, rootidx;

	if(phylayerID == 0)rootidx = DMRSCELLID0;
	if(phylayerID == 1)rootidx = DMRSCELLID1;
	if(phylayerID == 2)rootidx = DMRSCELLID2;

	for(i=0; i<DMRSLENGTH/2; i++){
		arg=(((double)TxRxMode)*PI*rootidx*((double)i*((double)i+1.0)))/63.0;
		__real__ DMRSsymb[i]=(float)cos(arg);
		__imag__ DMRSsymb[i]=(float)sin(arg);
	}
	for(i=DMRSLENGTH/2; i<DMRSLENGTH; i++){
		arg=(((double)TxRxMode)*PI*rootidx*(((double)i+2.0)*((double)i+1.0)))/63.0;
		__real__ DMRSsymb[i]=(float)cos(arg);
		__imag__ DMRSsymb[i]=(float)sin(arg);
	}
	return 1;
}


/**@ingroup genDMRStime_seq
 * This module generate the DMRS time sequence for the different FFT size
 * \param cellID: Identifies the sequence number: 0, 1, 2
 * \param FFTsize: define the size of the OFMD symbols: 256, 256, 512, 1024, 1536 o 2048
 * \param TxRxmode: defines if the sequence generate is for Tx or Rx side
 */
int genDMRStime_seq(int phylayerID, int FFTsize, _Complex float  *DMRS_time, int TxRxmode){

	int s, i;
	_Complex float DMRS_ID[DMRSLENGTH+2];
	/**FFT*/
	fftw_complex DMRS_freq[2048];
	fftw_complex DMRS_aux[2048];
	fftw_plan plan256genDMRS;

	/**Select cellID: 0, 1, 2*/
	setDMRS(phylayerID, DMRS_ID, TxRxmode);
	//TX DMRS: ROTATE
	memset(DMRS_freq, 0, sizeof(_Complex float)*FFTsize);
	s=1;	//DC at position O
	for(i=DMRSLENGTH/2; i<DMRSLENGTH; i++){
			DMRS_freq[s] = (fftw_complex)DMRS_ID[i];
			s++;
	}
	s=(FFTsize-(DMRSLENGTH/2));
	for(i=0; i<DMRSLENGTH/2; i++){
			DMRS_freq[s] = (fftw_complex)DMRS_ID[i];
			s++;
	}
	if(FFTsize==256){
		plan256genDMRS = fftw_plan_dft_1d(256, DMRS_freq, DMRS_aux, FFTW_BACKWARD, FFTW_ESTIMATE);
		fftw_execute(plan256genDMRS);
	}

	for(i=0; i<256; i++){
			*(DMRS_time+i)=(_Complex float)*(DMRS_aux+i);
	}


	return 0;
}

int genDMRSfreq_seq_rotated(int phylayerID, int FFTsize, _Complex float  *DMRS_freq, int TxRxmode){

	int s, i;
	_Complex float DMRS_ID[DMRSLENGTH+2];
	
	/**Select cellID: 0, 1, 2*/
	setDMRS(phylayerID, DMRS_ID, TxRxmode);
	//TX DMRS: ROTATE
	memset(DMRS_freq, 0, sizeof(_Complex float)*FFTsize);
	s=1;	//DC at position O
	for(i=DMRSLENGTH/2; i<DMRSLENGTH; i++){
			DMRS_freq[s] = DMRS_ID[i];
			s++;
	}
	s=(FFTsize-(DMRSLENGTH/2));
	for(i=0; i<DMRSLENGTH/2; i++){
			DMRS_freq[s] = DMRS_ID[i];
			s++;
	}
	return 0;
}


#define ROOT256	11.3137

int time2freq(int FFTsize, _Complex float *input, _Complex float *output){

	int s, i;
	_Complex float DMRS_ID[DMRSLENGTH+2];
	static int first=0;
	/**FFT*/
	static fftw_complex dataOUT[2048];
	static fftw_complex dataIN[2048];
	static fftw_plan plan256FFT;

	if(first==0)plan256FFT = fftw_plan_dft_1d(FFTsize, dataIN, dataOUT, FFTW_FORWARD, FFTW_ESTIMATE);
	first=1;

	for(i=0; i<FFTsize; i++)dataIN[i]=(fftw_complex)*(input+i);
	fftw_execute(plan256FFT);
	for(i=0; i<FFTsize; i++)*(output+i)=(_Complex float)dataOUT[i]/ROOT256;

	return 0;
}








int channel_estimator_freq(_Complex float *DMRSreceived, _Complex float *DMRSfreqseqTX, int FFTsize, _Complex float *CHestimation){

	static int first=0;
	int i;

	//FFT
	static fftw_complex DMRS_time[2048];
	static fftw_complex DMRS_aux[2048];
	static fftw_plan plangenDMRSfreq;
	_Complex float DMRSfreqRot[2048];
	_Complex float auxC, a=1.0+2.0*I, b=2.0+3.0*I, c=0.0+0.0*I;
	float auxf;
	static _Complex float CORRECT[2048];
	static float nofEstim=0.0;

	//ESTIMATION CORRECTION
	nofEstim +=1.0;
	for(i=0; i<78; i++){
		CORRECT[i] += *(DMRSfreqseqTX+i)/(*(DMRSreceived+i));
		*(CHestimation+i) = CORRECT[i]/nofEstim;
	//	*(CHestimation+i)=1.0+0.0*I;
		if((__real__ *(CHestimation+i)) > 10.0)__real__ *(CHestimation+i)=10.0;
		if((__real__ *(CHestimation+i)) < -10.0)__real__ *(CHestimation+i)=-10.0;
	}

	for(i=178; i<256; i++){
		CORRECT[i] += *(DMRSfreqseqTX+i)/(*(DMRSreceived+i));
		*(CHestimation+i)=CORRECT[i]/nofEstim;
		if((__real__ *(CHestimation+i)) > 10.0)__real__ *(CHestimation+i)=10.0;
		if((__real__ *(CHestimation+i)) < -10.0)__real__ *(CHestimation+i)=-10.0;
	}

	return(0);
}



/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
int bypass_CPLX(_Complex float *input, int inlength, _Complex float *output)
{
	int i,outlength;

//	printf("RUN MY FUNCTION\n");	
	for (i=0;i<inlength;i++) {
		__real__ output[i] = __real__ input[i];
		__imag__ output[i] = __imag__ input[i];
	}
	outlength=inlength;
	return outlength;
}



#define NOFRBsLTE256	256

// REF: Doc TS36.104 page 95 Annex E7
float computeEVM_3GGP_LTE256(_Complex float *inputMeasured, _Complex float *inputReference, int nofsamples){

	int i, k;		
	double EQMdiff=0.0, EQMsignal=0.0;
	static double EVM=0.0, EVMaver=0.0;
	static int nofIterations=0;

	nofIterations++;
	EQMdiff=0.0;
	EQMsignal=0.0;
	for(k=0; k<nofsamples; k++){
		EQMsignal += pow(cabsf(*(inputReference+k)), 2);
		EQMdiff += pow(cabsf(*(inputMeasured+k) - *(inputReference+k)), 2);
		if(k<5){
			printf("RefI=%3.3f, RefQ=%3.3f, SigI=%3.3f, SigQ=%3.3f\n", 
				__real__ *(inputReference+k), __imag__ *(inputReference+k), 
				__real__ *(inputMeasured+k), __imag__ *(inputMeasured+k));
		}

	}

	if(EQMsignal > 0.000001)EVM = EQMdiff/EQMsignal;
	else EVM = 0.00001;

	EVMaver=EVM/(float)nofIterations;
	if(EVMaver > 0.000000001)printf("EVMc=%f%, EQMsignal=%f, EQMdiff=%f, SNR=%f dBs\n", EVMaver*100, EQMsignal, EQMdiff, 10*log10(1.0/EVMaver));

	if(nofIterations==1){
		nofIterations=0;
		EVM=0.0;
		EQMsignal=0.0;
		EQMdiff=0.0;
	}


	return((float)(EVMaver*100.0));
}

// REF: Doc TS36.104 page 95 Annex E7
float computeEVM_3GGP_LTE256_2(_Complex float *inputMeasured, _Complex float *inputReference, int nofsamples){

	int i, k;		
	double EQMdiff=0.0, EQMsignal=0.0;
	static double EVM=0.0, EVMaver=0.0;
	static int nofIterations=0;

	nofIterations++;
	EQMdiff=0.0;
	EQMsignal=0.0;
	for(k=0; k<nofsamples; k++){
		EQMsignal += pow(cabsf(*(inputReference+k)), 2);
		EQMdiff += pow(cabsf(*(inputMeasured+k) - *(inputReference+k)), 2);
		if(k<5){
			printf("RefI=%3.3f, RefQ=%3.3f, SigI=%3.3f, SigQ=%3.3f\n", 
				__real__ *(inputReference+k), __imag__ *(inputReference+k), 
				__real__ *(inputMeasured+k), __imag__ *(inputMeasured+k));
		}

	}

	if(EQMsignal > 0.000001)EVM += EQMdiff/EQMsignal;
	else EVM += 0.00001;

	EVMaver=EVM/(float)nofIterations;
	if(EVMaver > 0.000000001)printf("EVMc=%f%, EQMsignal=%f, EQMdiff=%f, SNR=%f dBs\n", EVMaver*100, EQMsignal, EQMdiff, 10*log10(1.0/EVMaver));

	if(nofIterations==10){
		nofIterations=0;
		EVM=0.0;
		EQMsignal=0.0;
		EQMdiff=0.0;
	}


	return((float)(EVMaver*100.0));
}



#define MAXVALUE	1.0

int normA(_Complex float *inout, int length){
	int i;
	float maxval, auxR, auxI;
	static float ratio;

	maxval = 0.00000001;
	for(i=0; i<length; i++){
		auxR=fabs(__real__ inout[i]);
		auxI=fabs(__imag__ inout[i]);
		if(maxval < auxR){
			maxval = auxR;
		}
		if(maxval < auxI){
			maxval = auxI;
		}
	}
	ratio = (MAXVALUE)/maxval;		

	for(i=0; i<length; i++){
		inout[i] = inout[i]*ratio;
	}
	return(1);
}

int normB(_Complex float *inout, int length){
	int i;
	float maxval, auxR, auxI;
	static float ratio;

	maxval = 0.00000001;
	for(i=0; i<length; i++){
		auxR=fabs(__real__ inout[i]);
		auxI=fabs(__imag__ inout[i]);
		if(maxval < auxR){
			maxval = auxR;
		}
		if(maxval < auxI){
			maxval = auxI;
		}
	}
	ratio = (MAXVALUE)/maxval;		

	for(i=0; i<length; i++){
		inout[i] = inout[i]*ratio;
	}
	return(1);
}

