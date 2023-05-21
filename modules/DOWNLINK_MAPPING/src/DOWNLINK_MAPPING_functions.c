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
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "DOWNLINK_MAPPING_functions.h"


/** PRIMARY SYNCH SIGNALS: PSS*/
/**
 * @brief Function documentation: setPSS()
 * This function calculates the Zadoff-Chu sequence.
 * @params
 * @params int phylayerID:(0, 1, 2) Physical Layer Identity within the
 * Physical Layer cell-Identity Group.
 * @params  _Complex float *PSSsymb: Output array.
 * @params int TxRxMode: -1 (Tx Mode), 1 (Rx Mode) .
 *
 * @return On success returns 1.
 * On error returns -1.
 */

int setPSS(int phylayerID, _Complex float *PSSsymb, int TxRxMode)
{
	int i;
	double arg, rootidx;

	if(phylayerID == 0)rootidx = PSSCELLID0;
	if(phylayerID == 1)rootidx = PSSCELLID1;
	if(phylayerID == 2)rootidx = PSSCELLID2;

	for(i=0; i<PSSLENGTH/2; i++){
		//arg=((float)TxRxMode)*PI*rootidx*((float)i*((float)i+1.0))/63.0;
		arg=(((double)TxRxMode)*PI*rootidx*((double)i*((double)i+1.0)))/63.0;
		//printf("arg(%d)=%lf\n", i, arg);
		__real__ PSSsymb[i]=(float)cos(arg);
		__imag__ PSSsymb[i]=(float)sin(arg);
//		printf("__real__ PSSsymb[%d]=%lf\n", i, __real__ PSSsymb[i]);

	}
	for(i=PSSLENGTH/2; i<PSSLENGTH; i++){
		arg=(((double)TxRxMode)*PI*rootidx*(((double)i+2.0)*((double)i+1.0)))/63.0;
		__real__ PSSsymb[i]=(float)cos(arg);
		__imag__ PSSsymb[i]=(float)sin(arg);
	}
	return 1;
}


/**@ingroup genPSStime_seq
 * This module generate the PSS time sequence for the different FFT size
 * \param cellID: Identifies the sequence number: 0, 1, 2
 * \param FFTsize: define the size of the OFMD symbols: 128, 256, 512, 1024, 1536 o 2048
 * \param TxRxmode: defines if the sequence generate is for Tx or Rx side
 */


int genPSStime_seq(int phylayerID, int FFTsize, fftwf_complex  *PSS_time, int TxRxmode){

	int s, i;
	_Complex float PSS_ID[PSSLENGTH+2];
	/**FFT*/
	fftwf_complex PSS_freq[2048];
	fftwf_plan plan128genPSS;

	/**Select cellID: 0, 1, 2*/
	setPSS(phylayerID, PSS_ID, TxRxmode);
	//TX PSS: ROTATE
	memset(PSS_freq, 0, sizeof(_Complex float)*FFTsize);
	s=1;	//DC at position O
	for(i=PSSLENGTH/2; i<PSSLENGTH; i++){
			PSS_freq[s] = PSS_ID[i];
			s++;
	}
	s=(FFTsize-(PSSLENGTH/2));
	for(i=0; i<PSSLENGTH/2; i++){
			PSS_freq[s] = PSS_ID[i];
			s++;
	}
	if(FFTsize==128){
		plan128genPSS = fftw_plan_dft_1d(128, PSS_freq, PSS_time, FFTW_BACKWARD, FFTW_ESTIMATE);
		fftw_execute(plan128genPSS);
	}

	return 0;
}

int genPSSfreq_seq(int phylayerID, int FFTsize, _Complex float  *PSS_freq, int TxRxmode){
	int s, i;
	_Complex float PSS_ID[PSSLENGTH+2];

	/**Select cellID: 0, 1, 2*/
	setPSS(phylayerID, PSS_ID, TxRxmode);
	//TX PSS: ROTATE
	memset(PSS_freq, 0, sizeof(_Complex float)*FFTsize);
	s=1;	//DC at position O
	for(i=PSSLENGTH/2; i<PSSLENGTH; i++){
			PSS_freq[s] = PSS_ID[i];
			s++;
	}
	s=(FFTsize-(PSSLENGTH/2));
	for(i=0; i<PSSLENGTH/2; i++){
			PSS_freq[s] = PSS_ID[i];
			s++;
	}
	return(0);
}







#define NUMSUBFRAMES	10
#define NUMPHASES		4

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */

int control_SFrame(MODvars_t *oVars, MODparams_t *oParam, int rcv_data){
	int datalength=0;
	static int Tslot=-1;
	static int TslotLTE=0;
	static int first=0;

//	printf("IN control_SFrame(): Tslot=%d, TslotLTE=%d, datalength=%d\n", Tslot, TslotLTE, datalength);
	if(Tslot == 0){

		if(TslotLTE == 0)datalength=oParam->numchars13FFT;
		else datalength=oParam->numchars14FFT;
		if(first==0){
			datalength=oParam->numchars7FFT;
			first=1;
		}

		TslotLTE++;
		if(TslotLTE==5)TslotLTE=0;		// One of 5 subframes PSS must be added
	}

	Tslot++;
	if(Tslot==oParam->numTSNOT2SEND)Tslot=0;

//	printf("OUT control_SFrame(): Tslot=%d, TslotLTE=%d, datalength=%d\n", Tslot-1, TslotLTE-1, datalength);
	return(datalength);
}


/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */


int generate_LTE_subframe(MODvars_t *oVars, MODparams_t *oParam, 
							_Complex float *inMQAMsymb, int rcv_samples, 
							_Complex float *PSSseq,
							_Complex float *outbuffer,
							char *OBJname){
	int numOutSamples=oParam->FFTsz*14;;
	
	if(oParam->debugMODE == DEBUGPRINTMODE){
		printf("generate_LTE_subframe()\n");
		printf("numSubframe=%d, action=%d\n", oVars->NUMSubframe, oVars->action);
	}
	// CHECK IF rcv_samples VALUE IS CORRECT?
	if((rcv_samples != oVars->ActiveCarriers*14) && (rcv_samples != oVars->ActiveCarriers*13)
		&& (rcv_samples != oVars->ActiveCarriers*7) && rcv_samples != 0){
		printf("\n");
		printf("###################################################################################################\n");
		printf(" %s.generate_LTE_subframe()\n", OBJname);
		printf(" ERROR!!! rcv_samples=%d differs from expected = ActiveCarriers(%d)*14=%d or ActiveCarriers*7=%d\n", 
					rcv_samples, oVars->ActiveCarriers, oVars->ActiveCarriers*14, oVars->ActiveCarriers*7);
		printf(" Please, correct 'numTSChainDelay' value in %s.params!!!\n", OBJname);
		printf(" Execution Cancelled\n");
		printf("###################################################################################################\n");
		printf("\n");
		exit(0);
	}
	// DO NOTHING
	if(rcv_samples==0)numOutSamples=0;
	// SEND PSS+DATA SUBFRAME
	if(rcv_samples==oVars->ActiveCarriers*13)send_pss_data_SUBFRAME(PSSseq, inMQAMsymb, oParam->FFTsz, oVars->ActiveCarriers, outbuffer);	
	// SEND DATA SUBFRAME
	if(rcv_samples==oVars->ActiveCarriers*14)send_data_SUBFRAME(inMQAMsymb, oParam->FFTsz, oVars->ActiveCarriers, outbuffer);
	// FIRST SUBFRAME 2PSS
	if(rcv_samples==oVars->ActiveCarriers*7)send_first_SUBFRAME(PSSseq, inMQAMsymb, oParam->FFTsz, oVars->ActiveCarriers, outbuffer);

	return(numOutSamples);
}



/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
int create_LTEspectrumNORS(_Complex float *MQAMsymb, int FFTlength, int datalength, _Complex float *out_spectrum){
	int i, j;

	for(i=0; i<FFTlength; i++)*(out_spectrum+i)=0.0+0.0*I;
	j=FFTlength-datalength/2;
	for(i=0; i<datalength; i++){
		*(out_spectrum+j)=*(MQAMsymb+i);
		j++;
		if(j==FFTlength) j=1;
	}
	return(1);
}



int sendzeros_SUBFRAME(int fftlength, _Complex float *outbuffer){
	int k;
	for(k=0; k<fftlength*14; k++){
		*(outbuffer+k)=0.0+0.0i;
	}
	return(0);
}

int sendzerosPSS_SUBFRAME(_Complex float *PSS, int fftlength, _Complex float *outbuffer){
	int k;

	//printf("MAPPINGII_functions.c: sendzerosPSS()\n");
	//ALL ZEROS
	for(k=0; k<fftlength*14; k++){
		*(outbuffer+k)=0.0+0.0i;
	}	
	//ADD PSS
	create_LTEspectrumNORS(PSS, fftlength, PSSLENGTH, &outbuffer[6*fftlength]);

	return(0);
}

int send_first_SUBFRAME(_Complex float *PSS, _Complex float *inbuffer, int fftlength, int datasize, _Complex float *outbuffer){
	int k, j;

	for(k=0; k<fftlength*5; k++){
		*(outbuffer+k)=0.0+0.0i;
	}
	//ADD first PSS
	create_LTEspectrumNORS(PSS, fftlength, PSSLENGTH, &outbuffer[5*fftlength]);
	//ADD second PSS
	create_LTEspectrumNORS(PSS, fftlength, PSSLENGTH, &outbuffer[6*fftlength]);
	//ADD DATA
	j=0;
	for(k=7; k<14; k++){
		create_LTEspectrumNORS(&inbuffer[j*datasize], fftlength, datasize, &outbuffer[k*fftlength]);
		j++;
	}		
	return(0);
}


int send_data_SUBFRAME(_Complex float *inbuffer, int fftlength, int datasize, _Complex float *outbuffer){
	int k;
	//ADD DATA
	for(k=0; k<14; k++){
// TEST0
//	for(k=0; k<13; k++){
		create_LTEspectrumNORS(&inbuffer[k*datasize], fftlength, datasize, &outbuffer[k*fftlength]);
	}		
	return(0);
}

int send_pss_data_SUBFRAME(_Complex float *PSS, _Complex float *inbuffer, 
							int fftlength, int datasize, _Complex float *outbuffer){
	int k,j;

	//ADD DATA
	for(k=0; k<6; k++){
		create_LTEspectrumNORS(&inbuffer[k*datasize], fftlength, datasize, &outbuffer[k*fftlength]);
	}	
	//ADD PSS
	create_LTEspectrumNORS(PSS, fftlength, PSSLENGTH, &outbuffer[6*fftlength]);
	//ADD DATA
	j=6;
	for(k=7; k<14; k++){
		create_LTEspectrumNORS(&inbuffer[j*datasize], fftlength, datasize, &outbuffer[k*fftlength]);
		j++;
	}		
	return(0);
}




////////////////////////////////////WITH RS SIGNALS
#define MAXPHYID		3
#define NOFOFDMSYMB	14


_Complex float addRS(){

	return(0.7+0.7*I);
}

void addRSinOFDMsymb(_Complex float *MQAMsymb, int FFTlength, 
										 _Complex float *out_data, int phylayerID){

	int i, k=0, j=0;
	static int first=0;
	static int nofcarriers=0;

	if(first==0){
		if(FFTlength==128)nofcarriers=3*4*6;
		first=1;
	}
	
	for(i=0; i<nofcarriers; i++){
		if(k==phylayerID)*(out_data+i)=addRS();
		else {
			*(out_data+i)=*(MQAMsymb+j);
			j++;
		}
		k++;
		if(k==MAXPHYID)k=0;
	}
}


#define NOFOFDMSYMWITHRS	4

int addRSinDataFlow(_Complex float *MQAMsymb, int FFTlength, 
												int datalength, _Complex float *out_data, 
												int phylayerID){

	int NofOFDMsymb=0, datasize=0;
	int inPosition=0, outPosition=0;
	int i, k=0, j=0;
	static int first=0;
	static int nofcarriers=0;										//Nof of carriers per OFDM symbol
	static int nofRSs;													//Nof of RS per OFDM symbol
	static int withPSSdatalength=0, withoutPSSdatalength=0, firstdatalength=0;

	if(first==0){
		if(FFTlength==128){
			nofcarriers=3*4*6;
			nofRSs=1*4*6;
			withoutPSSdatalength=nofcarriers*NOFOFDMSYMB - nofRSs*NOFOFDMSYMWITHRS;
			withPSSdatalength=nofcarriers*(NOFOFDMSYMB-1) - nofRSs*NOFOFDMSYMWITHRS;
			firstdatalength=withoutPSSdatalength/2;
			printf("firstdatalength=%d, withPSSdatalength=%d, withoutPSSdatalength=%d\n", 
					firstdatalength, withPSSdatalength, withoutPSSdatalength);
		}
		first=1;
	}
	if(datalength==0)return(0);

	if((datalength != firstdatalength) && (datalength != withPSSdatalength) && (datalength != withoutPSSdatalength)){
		printf("MAPPINGV.addRSinDataFlow(): ERROR!!! datalength =%d different than expected\n", datalength);
		printf("firstdatalength=%d, withPSSdatalength=%d, withoutPSSdatalength=%d\n", 
					firstdatalength, withPSSdatalength, withoutPSSdatalength);
		exit(0);
	}
	


	inPosition=0;
	outPosition=0;
	// OFDM SYMBOL 0
	addRSinOFDMsymb(MQAMsymb+inPosition, FFTlength, out_data+outPosition, phylayerID);
	inPosition += nofcarriers-nofRSs;
	outPosition += nofcarriers;
	// OFDM SYMBOLS 1,2 and 3
	datasize=sizeof(_Complex float)*3*nofcarriers;
	memcpy(out_data+outPosition, MQAMsymb+inPosition, datasize);
	inPosition += 3*nofcarriers;
	outPosition += 3*nofcarriers;
	// OFDM SYMBOL 4
	addRSinOFDMsymb(MQAMsymb+inPosition, FFTlength, out_data+outPosition, phylayerID);
	inPosition += nofcarriers-nofRSs;
	outPosition += nofcarriers;
	// OFDM SYMBOLS 5
	datasize=sizeof(_Complex float)*nofcarriers;
	memcpy(out_data+outPosition, MQAMsymb+inPosition, datasize);
	inPosition += nofcarriers;
	outPosition += nofcarriers;
	// OFDM SYMBOLS 6
	if((datalength == withoutPSSdatalength) || (datalength == firstdatalength)){
		datasize=sizeof(_Complex float)*nofcarriers;
		memcpy(out_data+outPosition, MQAMsymb+inPosition, datasize);
		inPosition += nofcarriers;
		outPosition += nofcarriers;
	}

	if(datalength != firstdatalength){
		// OFDM SYMBOL 7
		addRSinOFDMsymb(MQAMsymb+inPosition, FFTlength, out_data+outPosition, phylayerID);
		inPosition += nofcarriers-nofRSs;
		outPosition += nofcarriers;
		// OFDM SYMBOLS 8,9 and 10
		datasize=sizeof(_Complex float)*3*nofcarriers;
		memcpy(out_data+outPosition, MQAMsymb+inPosition, datasize);
		inPosition += 3*nofcarriers;
		outPosition += 3*nofcarriers;
		// OFDM SYMBOL 11
		addRSinOFDMsymb(MQAMsymb+inPosition, FFTlength, out_data+outPosition, phylayerID);
		inPosition += nofcarriers-nofRSs;
		outPosition += nofcarriers;
		// OFDM SYMBOLS 12. 13
		datasize=sizeof(_Complex float)*2*nofcarriers;
		memcpy(out_data+outPosition, MQAMsymb+inPosition, datasize);
		inPosition += 2*nofcarriers;
		outPosition += 2*nofcarriers;
	}

	// CHECK THAT POINTERS FITS EXPECTED
	if(datalength != inPosition){
//		printf("MAPPINGV: addRSinDataFlow(): ERROR!!! datalength=%d != inPosition=%d\n", datalength, inPosition);
	}
	if((outPosition != nofcarriers*NOFOFDMSYMB) && 
			(outPosition != nofcarriers*(NOFOFDMSYMB-1)) && 
				(outPosition != nofcarriers*7)){
				printf("MAPPINGV: addRSinDataFlow(): ERROR!!! outPosition=%d != nofcarriers*NOFOFDMSYMB=%d\n", 
									outPosition, nofcarriers*NOFOFDMSYMB);
	}
	
	return(outPosition);

}





int create_LTEspectrumWITHRS(_Complex float *MQAMsymb, int FFTlength, 
												int datalength, _Complex float *out_spectrum, int phylayerID){
	int i, j, k;
	int nofcarriers=0;

	if(FFTlength==128)nofcarriers=3*4*6;

	for(i=0; i<FFTlength; i++)*(out_spectrum+i)=0.0+0.0*I;
	

	k=0;
	j=0;
	for(i=FFTlength-nofcarriers/2; i<FFTlength; i++){
		if(k%phylayerID == 0){
			*(out_spectrum+i)=addRS();
		}else{
			*(out_spectrum+i)=*(MQAMsymb+j);
			j++;
		}
		k++;
	}

	for(i=1; i < (FFTlength-nofcarriers/2 + 1); i++){
		if(k%phylayerID == 0){
			*(out_spectrum+i)=addRS();
		}else{
			*(out_spectrum+i)=*(MQAMsymb+j);
			j++;
		}
		k++;
	}
	if(datalength-1 != j) printf("create_LTEspectrumWITHRS(): ERROR!!! datalength=%d =! j=%d\n", datalength, j);
	return(1);
}





