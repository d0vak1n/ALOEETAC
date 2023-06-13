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
#include <math.h>
#include <string.h>

#include <phal_sw_api.h>
#include "skeleton.h"
#include "PSS_SYNCHROT21_functions.h"


extern char mname[];

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
		arg=(((double)TxRxMode)*PI*rootidx*((double)i*((double)i+1.0)))/63.0;
		__real__ PSSsymb[i]=(float)cos(arg);
		__imag__ PSSsymb[i]=(float)sin(arg);
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
int genPSStime_seq(int phylayerID, int FFTsize, _Complex float  *PSS_time, int TxRxmode){

	int s, i;
	_Complex float PSS_ID[PSSLENGTH+2];
	/**FFT*/
	fftw_complex PSS_freq[2048];
	fftw_complex PSS_aux[2048];
	fftw_plan plan128genPSS;

	/**Select cellID: 0, 1, 2*/
	setPSS(phylayerID, PSS_ID, TxRxmode);
	//TX PSS: ROTATE
	memset(PSS_freq, 0, sizeof(_Complex float)*FFTsize);
	s=1;	//DC at position O
	for(i=PSSLENGTH/2; i<PSSLENGTH; i++){
			PSS_freq[s] = (fftw_complex)PSS_ID[i];
			s++;
	}
	s=(FFTsize-(PSSLENGTH/2));
	for(i=0; i<PSSLENGTH/2; i++){
			PSS_freq[s] = (fftw_complex)PSS_ID[i];
			s++;
	}
	if(FFTsize==128){
		plan128genPSS = fftw_plan_dft_1d(128, PSS_freq, PSS_aux, FFTW_BACKWARD, FFTW_ESTIMATE);
		fftw_execute(plan128genPSS);
	}

	for(i=0; i<128; i++){
			*(PSS_time+i)=(_Complex float)*(PSS_aux+i);
	}


	return 0;
}

int genPSSfreq_seq_rotated(int phylayerID, int FFTsize, _Complex float  *PSS_freq, int TxRxmode){

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
	return 0;
}

void conv_cc(_Complex float *input, _Complex float *filter, _Complex float *output, int input_len, int filter_len) {
	int i,j;
	int output_len;
	int half_length;
	static _Complex float corr[2048];

	output_len=input_len+filter_len; //-1;
	half_length=output_len/2;

	memset(corr,0.0+0.0i,output_len*sizeof(_Complex float));
	for (i=0;i<input_len;i++) {
		for (j=0;j<filter_len;j++) {
			corr[i+j]+=input[i]*filter[j];
		}
	}
	memcpy(output, corr, sizeof(_Complex float)*half_length);
}



#define PSS_THRESHOLD		9.0
#define PSS_MAXTHRESHOLD	600.0
#define PSS_RATIO2THRESHOLD	2.0

float EXTthreshold=PSS_THRESHOLD;//PSS_MAXTHRESHOLD; //PSS_THRESHOLD;
float ExtAverRatio3=1.0;

int detect_2onPSS(_Complex float *inout, int length, int pMAX, float *variance){

	float MinRatio=1e10;
	float percentage;

	int pMAX0=pMAX-128, pMAX1=pMAX, pMAX2=pMAX+128;
	float ratio0 = ((fabsf(__real__ inout[pMAX0])+fabsf(__imag__ inout[pMAX0]))/(*variance))/ExtAverRatio3;
	float ratio1 = ((fabsf(__real__ inout[pMAX1])+fabsf(__imag__ inout[pMAX1]))/(*variance))/ExtAverRatio3;
	float ratio2 = ((fabsf(__real__ inout[pMAX2])+fabsf(__imag__ inout[pMAX2]))/(*variance))/ExtAverRatio3;

	percentage=(int)((ratio0/ratio1)*100.0);
	if((int)percentage >= 75){
		return(pMAX1);
	}

	percentage=(int)((ratio2/ratio1)*100.0);
	if((int)percentage >= 75){
		return(pMAX2);
	}	



	return(-1);
}





#define PSS_RATIO3THRESHOLD	0.0
#define WINDOWSZSHORT	15
#define WINDOWSZWIDE	256
#define WINDOWSZWIDE2	64

int detect_PSS(_Complex float *inout, int length, float *varianze){
	int i, k, pMAX=-1, pMAX2=0, block;
	float maxval, smaxval=1000000.0, auxR, auxI, ratio, ratio2, ratio3, ratio4;
	float vmedio=0.0;
	static float threshold=PSS_THRESHOLD;
	static float thresholdratio3=PSS_RATIO3THRESHOLD;
	float varianceSHORT=0.0, varianceWIDE=0.0, variance2=0.0;
	int Llimit=0, Hlimit=length;
	int P=0;
	static float ratio3MAXave=PSS_RATIO3THRESHOLD, ratio3ave=1.0;
	static int numave=1, numMAXave=1;

	float MAX5=0, ratio5;
	float varianceWIDE5=0.0;
	float auxR5, auxI5;

	static float Ratio3MEM[5]={1000000.0, 1000000.0, 1000000.0, 1000000.0, 1000000.0};
	static int CountRatio3=0;
	float AverRatio3;
	float percentage=0.0;
	

	//*variance=0.0;
	maxval = -1000000.0;
	for(i=0; i<length; i++){
		auxR=(float)(fabs(__real__ *(inout+i)) + fabs(__imag__ *(inout+i)));
			if(maxval < auxR){
					smaxval=fabs(maxval);
				maxval = fabs(auxR);
				pMAX=i;
			}
	}


	// CHECK IF PSS
	k=0;
	auxR5=(float)(fabs(__real__ *(inout+pMAX)) + fabs(__imag__ *(inout+pMAX)));
	auxI5=(float)(fabs(__real__ *(inout+pMAX)) - fabs(__imag__ *(inout+pMAX)));
	MAX5=fabs(auxR5*auxI5);
	if(pMAX - WINDOWSZWIDE2 > 0)Llimit=pMAX-WINDOWSZWIDE2;
	else Llimit=0;
	if(pMAX+WINDOWSZWIDE2 < length)Hlimit=pMAX+WINDOWSZWIDE2;
	else Hlimit=length;
	for(i=Llimit; i<Hlimit; i++){
		auxR5=(float)(fabs(__real__ *(inout+i)) + fabs(__imag__ *(inout+i)));
		auxI5=(float)(fabs(__real__ *(inout+i)) - fabs(__imag__ *(inout+i)));
		auxR=fabs(auxR5*auxI5);
		if(abs(pMAX - i) > 5){
			varianceWIDE5 += auxR;
			k++;
		}
	}
	varianceWIDE5 = varianceWIDE5/(float)k;
	if(isnan(varianceWIDE5))varianceWIDE5=1.0;
	if(varianceWIDE5 < 0.001){   //0.01
		varianceWIDE5 = 0.001;
	}
	ratio5=fabs(1.0 - (varianceWIDE5/MAX5))*1000.0; 


	// CALCULATE SIGNAL VARIANCE
	k=0;
	if(pMAX - WINDOWSZWIDE > 0)Llimit=pMAX-WINDOWSZWIDE;
	if(pMAX+WINDOWSZWIDE < length)Hlimit=pMAX+WINDOWSZWIDE;
	for(i=Llimit; i<Hlimit; i++){
		auxR=(float)(fabs(__real__ *(inout+i)) + fabs(__imag__ *(inout+i)));
		if(abs(pMAX - i) > 5){
			varianceWIDE += auxR/maxval;
			k++;
		}
	}
	varianceWIDE = varianceWIDE/(float)k;
	if(isnan(varianceWIDE))varianceWIDE=1.0;
	if(varianceWIDE < 0.001){   //0.01
		varianceWIDE = 0.001;
	}	
	// CALCULATE SIGNAL VARIANCE AROUND THE pMAX
	k=0;
	if(pMAX - WINDOWSZSHORT > 0)Llimit=pMAX-WINDOWSZSHORT;
	if(pMAX+WINDOWSZSHORT < length)Hlimit=pMAX+WINDOWSZSHORT;
	for(i=Llimit; i<Hlimit; i++){
		auxR=(float)(fabs(__real__ *(inout+i)) + fabs(__imag__ *(inout+i)));
		if(abs(pMAX - i) > 5){
			varianceSHORT += auxR/maxval;
			k++;
		}
	}
	varianceSHORT = varianceSHORT/(float)k;
	if(isnan(varianceSHORT))varianceSHORT=1.0;
	if(varianceSHORT < 0.001){   //0.01
		varianceSHORT = 0.001;
	}	
	*varianze=varianceSHORT;
	ratio=maxval/(varianceSHORT);
	ratio4=maxval/(varianceWIDE);
	ratio2=maxval/smaxval;
	ratio3=ratio*ratio2;

	thresholdratio3=3000.0;
	printf("\033[1;35m \tratio=%-10.3f, ratio2=%-10.3f, ratio3=%-10.3f, ratio4=%-10.3f, thresholdratio3=%-10.3f \033[0m\n", 
					ratio, ratio2, ratio3, ratio4, thresholdratio3);


	if(ratio3 > thresholdratio3){
		//printf("\033[1;35m: detect_PSS(): \033[0m\n");
		numMAXave++;
		ratio3MAXave += ratio3;

/*		printf("\033[1;35m %s: detect_PSS(): maxval=%3.2f, smaxval=%3.2f, varianceSHORT=%3.5f, varianceWIDE=%3.5f\033[0m\n", 
					mname, maxval, smaxval, varianceSHORT, varianceWIDE);
		printf("\033[1;35m ratio=%3.3f, ratio2=%3.3f, ratio3=%3.3f, ratio4=%3.3f, ratio3MAXave=%3.3f, ratio3ave=%3.3f, thresholdratio3=%3.3f \033[0m\n", 
					ratio, ratio2, ratio3, ratio4, ratio3MAXave/numMAXave, ratio3ave/numave, thresholdratio3);
*/
	}
	else {
		pMAX=-1;
		//printf("NOT detect_PSS():\n");
		ratio3ave += ratio3;
		numave++;

/*		printf("%s: detect_PSS(): maxval=%3.2f, smaxval=%3.2f, varianceSHORT=%3.5f, varianceWIDE=%3.5f\n", 
					mname, maxval, smaxval,varianceSHORT, varianceWIDE);
		printf("ratio=%3.3f, ratio2=%3.3f, ratio3=%3.3f, ratio4=%3.3f, ratio3MAXave=%3.3f, ratio3ave=%3.3f, thresholdratio3=%3.3f\n", 
					ratio, ratio2, ratio3, ratio4, ratio3MAXave/numMAXave, ratio3ave/numave, thresholdratio3);
*/

	}

	if(numave+numMAXave > 10)thresholdratio3=(0.4*(ratio3ave/(float)numave)+0.6*(ratio3MAXave/(float)numMAXave))/2.0;
	else thresholdratio3=varianceWIDE*10.0;

	EXTthreshold=thresholdratio3;

	return(pMAX-1);
}



float compute_in_gain(_Complex float *inout, int length, int pPSS){
	int i;
	float maxValue=0.00000001;
	float value;
	float gain=0.0;
	int initPos=pPSS-128;
	int endPos=pPSS;

	if(initPos < 0)initPos=0;
	for(i=initPos; i<endPos; i++){
		value = fabs(__real__ *(inout+i));
		if(value > maxValue)maxValue=value;
	}
//	printf("compute_in_gain(): maxvalue=%3.3f\n", maxValue);
	gain=1.0/maxValue;
	return(gain);
}

float compute_in_gainAver(_Complex float *inout, int length, int pPSS){
	int i;
	float maxValue=0.00000001;
	float value;
	float gain=0.0;
	int initPos=pPSS-128;
	int numsamples;
	int endPos=pPSS;
	float aver=0.0;

	if(initPos < 0)initPos=0;
	numsamples=endPos-initPos;
	for(i=initPos; i<endPos; i++){
		aver = aver+cabsf(*(inout+i));
	}
	aver = aver/(float)numsamples;
	gain=(1.0/aver)/20.0;
	return(gain);
}



#define FILTERLENGTH	256
_Complex float aux[FILTERLENGTH];
#define MAX_DATA 50*2048

int stream_conv_CPLX(_Complex float *ccinput, int datalength, _Complex float *filtercoeff, int filterlength, _Complex float *ccoutput){

	int i, j;
	static int first=0;
	_Complex float CCinput[MAX_DATA];
	_Complex float CCoutput[MAX_DATA];

	if(datalength>MAX_DATA){
		printf("ERROR!!!.In stream_conv_CPLX() datalength=%d exceeding INPUT_MAX_DATA=%d\n", datalength, MAX_DATA);
		exit(0);
	}
	if(filterlength>FILTERLENGTH){
		printf("ERROR!!!.In stream_conv_CPLX() filterlength=%d exceeding FILTERLENGTH=%d\n",filterlength, FILTERLENGTH);
		exit(0);
	}
//	printf("datalength=%d, filterlength=%d\n", datalength, filterlength);

	memcpy(CCinput, ccinput, sizeof(_Complex float)*datalength);

	if(first==0){
		for(j=0; j<filterlength; j++){
			aux[j]=0.0+0.0i;
			//printf("FIRST CCONV real=%3.6f, imag=%3.6f\n", __real__ filtercoeff[j], __imag__ filtercoeff[j]);
		}
		first=1;
	}

	for (i=0;i<datalength;i++) {
		for (j=filterlength-2;j>=0;j--) {
			aux[j+1]=aux[j];
		}
		aux[0]=CCinput[i];
		CCoutput[i]=0.0;
		for (j=0;j<filterlength;j++) {
			CCoutput[i]+=aux[j]*filtercoeff[j];
		}
	}
	memcpy(ccoutput, CCoutput, sizeof(_Complex float)*datalength);
	return datalength;
}

void process_correllation(_Complex float *inCorrel, int datalength, _Complex float *outCorrel){

	int i;

	for(i=0; i<datalength; i++){
		__real__ *(outCorrel+i)=(float)(fabs(__real__ *(inCorrel+i)) + fabs(__imag__ *(inCorrel+i)));
		__imag__ *(outCorrel+i)=(float)(fabs(__real__ *(inCorrel+i)) - fabs(__imag__ *(inCorrel+i)));
	}

}

void process_correllation2(_Complex float *inCorrel, int datalength, _Complex float *outCorrel){

	int i;

	for(i=0; i<datalength; i++){
		__real__ *(outCorrel+i)=(float)(fabs(__real__ *(inCorrel+i)) + fabs(__imag__ *(inCorrel+i)));
		__imag__ *(outCorrel+i)=(float)(fabs(__real__ *(inCorrel+i)) - fabs(__imag__ *(inCorrel+i)));
		__imag__ *(outCorrel+i)=(__imag__ *(outCorrel+i))*(__real__ *(outCorrel+i));
	}

}




int gain_vectorCPLX(_Complex float *in, _Complex float *out, int length, float gain){
	int i;

	for(i=0; i<length; i++){
		*(out+i)=(*(in+i))*gain;
	}

	return(1);
}

////////////////////////////////////////////
#define SUBFRAME_BUFF_SZ		28672
#define NOFSUBFRAMEINBUFFER		5
#define EMPTY		-1
#define NOTFULL	 	0
#define FULL	 	1
#define ACTIVE	 	1
#define NONACTIVE	0
_Complex float subframe_buffer[NOFSUBFRAMEINBUFFER][SUBFRAME_BUFF_SZ];
int stat_subframe[NOFSUBFRAMEINBUFFER]={EMPTY, EMPTY, EMPTY, EMPTY, EMPTY};	//-1: Empty, 0: Not Full, 1: Full

/**
 * @write_subframe_buffer(): Write uplink subframe in the output buffer but aligned
 *                           with the uplink LTE frame format. 
 *							 Each aligned subframe in the buffer is marked as FULL 
 *							 when ready to be read and send to the output.
 * @params	_Complex float *datain: Subframe Input sequence.
 * @params  int pMAX: Position of the last sample of the DMRS sequence.
 * @params int rcv_samples: received number of samples .
 * @params int FFTsize: SC-FDMA symbol length.
 * @return On success returns correlation 1.
 * On error returns -1.
 */
int write_subframe_buffer(_Complex float *datain, int pMAX, int rcv_samples, int FFTsize){

	int i;
	static int w_idx=0, w_subf_idx=0, first=0, firstSUBframe=0, RXActive=NONACTIVE;
	static int subframe_sz, PSS_POS/*, DMRS1_POS*/;
	int remain;

	if(first==0){
		subframe_sz=FFTsize*NOFOFDMSYMBPERSUBFRAME;
		PSS_POS=FFTsize*7-1;				// Final Position of PSS sequence

//		printf("write_subframe_buffer(): PSS_POS=%d, subframe_sz=%d\n", PSS_POS, subframe_sz);
		first=1;
	}

/*	printf("IN SUBFRAME BUFFERS [%d][%d][%d][%d][%d]\n", 
		stat_subframe[0], stat_subframe[1],stat_subframe[2],stat_subframe[3],stat_subframe[4]);
	printf("w_subf_idx=%d, w_idx=%d\n", w_subf_idx, w_idx);
*/
	if(pMAX >= 0){
		RXActive=ACTIVE; // RX active from here
		if(pMAX == PSS_POS){
			for(i=0; i<rcv_samples; i++){
				subframe_buffer[w_subf_idx][i]=*(datain+i);
			}
//			printf("pMAX == PSS_POS stat_subframe[%d]=%d \n", w_subf_idx, stat_subframe[w_subf_idx]);
			stat_subframe[w_subf_idx]=FULL;
			w_subf_idx++;
			w_idx=0;
			if(w_subf_idx == NOFSUBFRAMEINBUFFER)w_subf_idx=0;	
		}
		if(pMAX > PSS_POS){
			// Continue writting in current subframe for the pMAX-PSS_POS samples	
//			printf("INITIAL w_idx=%d\n", w_idx);
//			printf("pMAX > PSS_POS stat_subframe[%d]=%d \n", w_subf_idx, stat_subframe[w_subf_idx]);
			if(w_idx != subframe_sz-(pMAX-PSS_POS) && firstSUBframe==1){
//				printf("AB ERROR!!! w_idx=%d != subframe_sz-pMAX-PSS_POS=%d\n", w_idx, subframe_sz-(pMAX-PSS_POS));
			}
			w_idx=subframe_sz-(pMAX-PSS_POS);		
			for(i=0; i<pMAX-PSS_POS; i++){
				subframe_buffer[w_subf_idx][w_idx]=*(datain+i);
				w_idx++;
				if(w_idx == subframe_sz && firstSUBframe==1){
					stat_subframe[w_subf_idx]=FULL;
//					printf("AB stat_subframe[%d]=%d \n", w_subf_idx, stat_subframe[w_subf_idx]);
//					printf("i=%d, pMAX-PSS_POS=%d, w_idx=%d\n", i, pMAX-PSS_POS, w_idx);
					w_subf_idx++;
					if(w_subf_idx == NOFSUBFRAMEINBUFFER)w_subf_idx=0;	
					w_idx=0;
					break;
				}
			}
			if(firstSUBframe==0)firstSUBframe=1;
			w_idx=0;
//			if(stat_subframe[w_subf_idx]!=EMPTY)printf("B stat_subframe[%d]=%d (NO EMPTY)\n", w_subf_idx, stat_subframe[w_subf_idx]);
			for(i=0; i<rcv_samples-(pMAX-PSS_POS); i++){
				subframe_buffer[w_subf_idx][w_idx]=*(datain+i+(pMAX-PSS_POS));
				w_idx++;
			}
//			printf("FINAL w_idx=%d\n", w_idx);
		}
		if(pMAX < PSS_POS){
//			printf("pMAX < PSS_POS IN stat_subframe[%d]=%d \n", w_subf_idx, stat_subframe[w_subf_idx]);
			// Automatic synchronization
			w_idx=PSS_POS-pMAX;
			// Continue writting in current subframe for the rcv_samples samples					
			for(i=0; i<rcv_samples; i++){
				subframe_buffer[w_subf_idx][w_idx]=*(datain+i);
				w_idx++;
				if(w_idx == subframe_sz){
					stat_subframe[w_subf_idx]=FULL;
//					printf("AC stat_subframe[%d]=%d w_idx=%d\n", w_subf_idx, stat_subframe[w_subf_idx], w_idx);
					w_idx=0;
					w_subf_idx++;
					if(w_subf_idx == NOFSUBFRAMEINBUFFER)w_subf_idx=0;
//					if(stat_subframe[w_subf_idx]!=EMPTY)printf("C stat_subframe[%d]=%d (NO EMPTY)\n", w_subf_idx, stat_subframe[w_subf_idx]);
				}
			}
//			printf("pMAX=%d < PSS_POS=%d OUT stat_subframe[%d]=%d \n", pMAX, PSS_POS, w_subf_idx, stat_subframe[w_subf_idx]);
		}
	}else{
		if(RXActive==ACTIVE){
			//printf("pMAX < 0 stat_subframe[%d]=%d \n", w_subf_idx, stat_subframe[w_subf_idx]);
			for(i=0; i<rcv_samples; i++){
				subframe_buffer[w_subf_idx][w_idx]=*(datain+i);
				w_idx++;
				if(w_idx == subframe_sz){
					stat_subframe[w_subf_idx]=FULL;
					w_idx=0;
					w_subf_idx++;
					if(w_subf_idx == NOFSUBFRAMEINBUFFER)w_subf_idx=0;
					if(stat_subframe[w_subf_idx]!=EMPTY)printf("E stat_subframe[%d]=%d (NO EMPTY)\n", w_subf_idx, stat_subframe[w_subf_idx]);
				}
			}
		}
	}
/*	printf("OUT SUBFRAME BUFFERS [%d][%d][%d][%d][%d]\n", stat_subframe[0], stat_subframe[1],stat_subframe[2],stat_subframe[3],stat_subframe[4]);
	printf("OUT w_subf_idx=%d, w_idx=%d\n", w_subf_idx, w_idx);
*/
	return(0);
}

/**
 * @read_subframe_buffer():  Read uplink subframe from the output buffer to the module
 *                           output when marked as FULL. After reading buffer is marked
 * 							 EMPTY. 
 * @params	_Complex float *dataout: Pointer to output.
 * @params int FFTsize: SC-FDMA symbol length.
 * @return On success returns number of samples send.
 * On error returns -1.
 */
int read_subframe_buffer(_Complex float *dataout, int FFTsize){

	int i, read_samples=0;
	static int r_subf_idx=0, first=0;
	static int subframe_sz, PSS_POS/*, DMRS1_POS*/;

	if(first==0){
		subframe_sz=FFTsize*NOFOFDMSYMBPERSUBFRAME;
		first=1;
	}

	if(stat_subframe[r_subf_idx] == FULL){
		read_samples=subframe_sz;
		for(i=0; i<read_samples; i++){
			*(dataout+i) = subframe_buffer[r_subf_idx][i];
		}
		stat_subframe[r_subf_idx] = EMPTY;
		r_subf_idx++;
		if(r_subf_idx == NOFSUBFRAMEINBUFFER)r_subf_idx=0;
	}else{
		printf("stat_subframe[%d]=EMPTY\n",r_subf_idx);
	}
	return(read_samples);
}

//////////////////////////////////////////////////////////////////////7


///// CORRECT CFO ////////////////////////////////////////////////

/**@ingroup rotateCvector()
 * This module introduce a phase change of Adegrees in the data sequence.
 * \param *inout: Input/output complex vector pointer
 * \param length: Length of data sequence.
 * \param Adegrees: Degrees of phase variation.
 * \param Pdegrees: Accuracy in degrees of phasortable.
 * Return -1 if error, 1 if OK.
 */
void rotateCvector(_Complex float *in, _Complex float *out, int length, float Adegrees){
	int i;
	_Complex float phasor;
	float degrees;

	degrees = Adegrees;
	phasor=cosf(degrees*PI/180.0)+(sinf(degrees*PI/180.0))*I;
	for(i=0; i<length; i++){
		*(out+i)=*(in+i)*phasor;
	}
}



float checkPSSphase(_Complex float *PSSreceived, _Complex float *PSSseqRX, int length){

	int i, c;
	_Complex float Correl, PSSval, CFO;
	float Repre=0., Impre=0., Repost=0., Impost=0., arctngPre, arctngPost, diff, aux=(180.0/PI);
	double CFOdegrees=0.0;
	
	for(i=0; i<length; i++){
		Repre += __real__ PSSreceived[i];
		Impre += __imag__ PSSreceived[i];	//Rx Imag part sign is modified from Tx
		Repost += __real__ PSSseqRX[i];
		Impost += __imag__ PSSseqRX[i];
	}
	arctngPost = atan2f(Impost, Repost)*aux;
	arctngPre = atan2f(Impre, Repre)*aux;
	diff = 90-(arctngPost - arctngPre);

	printf("AOOocheckPSSphase(): arctngPost=%1.6f, arctngPre=%1.6f, diff=%2.3f\n", arctngPost, arctngPre, diff);
	return(diff);
}



/**@checkCFO()
 * This function estimates CFO acording: "An efficient CFO Estimation Algorithm for the Downlink of 3GPP-LTE"
 * \param *PSScorrelation: Correlation of the received PSS signal with the original PSS
 * \param fftsize: FFT size.
 * \param CFO: CFO estimation in .
 * Return -1 if error, 1 if OK.
 */

float checkPhaseOffset(_Complex float *PSScorrelation){
	_Complex float PhaseOffset;
	double aux=(180.0/PI); 
	double PhaseOffsetdegrees=0.0;
	
	PhaseOffset=*PSScorrelation;	
	PhaseOffsetdegrees=atan2f((double)__imag__ PhaseOffset, (double)__real__ PhaseOffset)*aux;
//	printf("AOOocheckCFO(): realCFO=%1.6f, imagCFO=%1.6f, CFOdegree=%2.3f\n", 
//					(float)__real__ CFO, (float)__imag__ CFO, (float)CFOdegrees);
	return((float)PhaseOffsetdegrees);
}





#define CBUFFER_SZ	64*1024

void testPhaseOffset(){
	_Complex float  inputSEQ[1024*2];
	_Complex float  inputSEQ2[1024*2];
	int i;
	_Complex float CorrResult[CBUFFER_SZ];
	float variance=0.0;
	_Complex float PSSTIMETX[128];
	_Complex float PSSTIME[128];
	int PSS_idx=0; 
	int FFTsize=128;
	int length;
	int pMAX1=-1;
	float CFOadddegrees=0.0;
	float CFOdetectdegrees=0;


	////TEST
	genPSStime_seq(PSS_idx, FFTsize, PSSTIMETX, 1);

	for(i=0; i<1024; i++)inputSEQ[i]=((float)(i%4))/4.0+(((float)(i%6))/6.0)*I;
	for(i=0; i<128; i++)inputSEQ[578+i]=PSSTIMETX[i];
	CFOadddegrees=67.0;
	rotateCvector(inputSEQ, inputSEQ2, length, CFOadddegrees);
	stream_conv_CPLX(inputSEQ2, length, PSSTIME, FFTsize, CorrResult);
	pMAX1=detect_PSS(CorrResult, length, &variance);

	CFOdetectdegrees=checkPSSphase((_Complex float*)(&inputSEQ2[0]+pMAX1-128), PSSTIME, FFTsize);
	printf("checkPSSphase(): pMAX1=%d, CFOdetectdegrees=%3.6f\n", pMAX1, CFOdetectdegrees);

	CFOdetectdegrees=checkPhaseOffset(CorrResult+pMAX1);
	printf("checkCFO(): pMAX1=%d, CFOdetectdegrees=%3.6f\n", pMAX1, CFOdetectdegrees);

	CFOdetectdegrees=-CFOdetectdegrees;
	rotateCvector(inputSEQ2, inputSEQ, length, CFOdetectdegrees);

}


#define ROOT128	11.3137

int channel_estimator(_Complex float *PSSreceived, _Complex float *PSSfreqseqTX, int FFTsize, _Complex float *CHestimation){

	

	return(0);
}





