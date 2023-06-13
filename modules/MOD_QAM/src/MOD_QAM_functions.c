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
#include <sys/time.h>
#include <phal_sw_api.h>

#include "print_utils.h"
#include "skeletonSK15.h"
#include "MOD_QAM_functions.h"

#define COMPACT_256QAM
#define COMPACT_1024QAM

extern MODparams_t oParam;


#define QAM4_LEVEL      0.7071    ///< QAM4 amplitude (RMS=1)

#define QAM16_LEVEL_1   0.3162    ///< Low 16-QAM amplitude (RMS=1)
#define QAM16_LEVEL_2   0.9487    ///< High 16-QAM amplitude (RMS=1)
#define QAM16_THRESHOLD	0.6324    ///< 16-QAM threshold for RMS=1 signal

#define QAM64_LEVEL_1	0.1543    ///< Low 16-QAM amplitude (RMS=1)
#define QAM64_LEVEL_2	0.4629    ///< High 16-QAM amplitude (RMS=1) 
#define QAM64_LEVEL_3	0.7715
#define QAM64_LEVEL_4	1.0801

#define QAM64_THRESHOLD_1	0.3086    ///< 16-QAM threshold for RMS=1 signal
#define QAM64_THRESHOLD_2	0.6172    ///< 16-QAM threshold for RMS=1 signal
#define QAM64_THRESHOLD_3	0.9258


#ifdef COMPACT_256QAM

#define QAM256_LEVEL_1	0.0766 
#define QAM256_LEVEL_2	0.23
#define QAM256_LEVEL_3	0.3834
#define QAM256_LEVEL_4	0.5368
#define QAM256_LEVEL_5	0.6902 
#define QAM256_LEVEL_6	0.8436
#define QAM256_LEVEL_7	0.997
#define QAM256_LEVEL_8	1.1504

#define QAM256_THRESHOLD_1	0.1533    
#define QAM256_THRESHOLD_2	0.3067    
#define QAM256_THRESHOLD_3	0.6135

#else

#define QAM256_LEVEL_1	0.076696   
#define QAM256_LEVEL_2	0.230089   
#define QAM256_LEVEL_3	0.383482
#define QAM256_LEVEL_4	0.536875
#define QAM256_LEVEL_5	0.690227   
#define QAM256_LEVEL_6	0.843661   
#define QAM256_LEVEL_7	0.997054
#define QAM256_LEVEL_8	1.150447

#define QAM256_THRESHOLD_1	0.153393   
#define QAM256_THRESHOLD_2	0.306786   
#define QAM256_THRESHOLD_3	0.460179
#define QAM256_THRESHOLD_4	0.613572   
#define QAM256_THRESHOLD_5	0.776696    
#define QAM256_THRESHOLD_6	0.920357
#define QAM256_THRESHOLD_7	1.073751  

#endif



#ifdef COMPACT_1024QAM

#define QAM1024_THRESHOLD_1	0.0765
#define QAM1024_THRESHOLD_2	0.1531
#define QAM1024_THRESHOLD_3	0.3063
#define QAM1024_THRESHOLD_4	0.6126

#else

#define QAM1024_LEVEL_1		0.038292   
#define QAM1024_LEVEL_2		0.114876   
#define QAM1024_LEVEL_3		0.191460
#define QAM1024_LEVEL_4		0.268044
#define QAM1024_LEVEL_5		0.344628   
#define QAM1024_LEVEL_6		0.421212   
#define QAM1024_LEVEL_7		0.497796
#define QAM1024_LEVEL_8		0.574380
#define QAM1024_LEVEL_9		0.650964   
#define QAM1024_LEVEL_10	0.727548   
#define QAM1024_LEVEL_11	0.804132
#define QAM1024_LEVEL_12	0.880716
#define QAM1024_LEVEL_13	0.957299   
#define QAM1024_LEVEL_14	1.033883   
#define QAM1024_LEVEL_15	1.110467
#define QAM1024_LEVEL_16	1.187051

#define QAM1024_THRESHOLD_1	0.076584   
#define QAM1024_THRESHOLD_2	0.153168   
#define QAM1024_THRESHOLD_3	0.229752
#define QAM1024_THRESHOLD_4	0.306336   
#define QAM1024_THRESHOLD_5	0.382920    
#define QAM1024_THRESHOLD_6	0.459504
#define QAM1024_THRESHOLD_7	0.536088
#define QAM1024_THRESHOLD_8	0.612672   
#define QAM1024_THRESHOLD_9	0.689256   
#define QAM1024_THRESHOLD_10	0.765840
#define QAM1024_THRESHOLD_11	0.842424   
#define QAM1024_THRESHOLD_12	0.919007    
#define QAM1024_THRESHOLD_13	0.995591
#define QAM1024_THRESHOLD_14	1.072175
#define QAM1024_THRESHOLD_15	1.148759
#endif


/* 4QAM #####################################################################################################3*/

	// LTE-QPSK constellation:
	//     Q
	// 10  |  00
	//-----------> I
	// 11  |  01
	
int mod_4QAM(char *bits, int numbits, _Complex float *symbols) {
	int i, k, j = 0;

	if (numbits==0) return 0;

	for (i=0;i<numbits;i+=2) {
		k = ((bits[i] == 1) ? 1 : 0) << 1 | ((bits[i + 1] == 1) ? 1 : 0);
		switch (k) {
			case 0:
				*(symbols+j)=QAM4_LEVEL+QAM4_LEVEL*I;
				break;
			case 1:
				*(symbols+j)=QAM4_LEVEL-QAM4_LEVEL*I;
				break;
			case 2:
				*(symbols+j)=-QAM4_LEVEL+QAM4_LEVEL*I;
				break;
			case 3:
				*(symbols+j)=-QAM4_LEVEL-QAM4_LEVEL*I;
				break;
			default:
				Log("ERROR: Unknown symbol for QAM4: %d\n",k);
				return ;
		}
		j++;
	}
	return j;
}

int soft_demod_4QAM(_Complex float *symbols, int numinputsymb, float *softbits) {
	int i, j=0;
	float in_real, in_imag;

	// Normalize input
	norm4QAM(symbols, numinputsymb);


	for (i=0; i < numinputsymb; i++) {
		softbits[j+0] = -creal(*(symbols+i));
		softbits[j+1] = -cimag(*(symbols+i));
		j += 2;
	}
	return j;
}

int norm4QAM(_Complex float *inout, int length) {
	int i;
	float auxR, auxI, averg = 0.0, Q = -0.01; 
	static float ratio = 0.0;

	for(i = 0; i < length; i++){
		auxR=fabs(__real__ inout[i]);
		auxI=fabs(__imag__ inout[i]);
		averg = averg + auxR + auxI;
	}

	// The small offset is to prevent DIV0 error
	averg = averg / (2.0 * (float)length + 0.00000001);
	ratio = QAM4_LEVEL / (averg + Q);		
	for(i=0; i < length; i++){
		inout[i] = inout[i] * ratio;
	}
	return 1;
}

/* 16QAM #####################################################################################################3*/

	/* LTE-16QAM constellation:
	              Q
	  1011	1001  |	 0001  0011
	  1010	1000  |	 0000  0010
	---------------------------------> I
	  1110  1100  |  0100  0110
	  1111  1101  |  0101  0111 	*/
//
int mod_16QAM (char *bits, int numbits, _Complex float *symbols)
{
	int i, j=0;

	if(numbits==0)return(0);

	for (i=0;i<numbits;i+=4)
	{
		symbols[j]  =   ((bits[i+0]==1)?(-1):(+1))*((bits[i+2]==1)?(QAM16_LEVEL_2):(QAM16_LEVEL_1));
		symbols[j] = I*((bits[i+1]==1)?(-1):(+1))*((bits[i+3]==1)?(QAM16_LEVEL_2):(QAM16_LEVEL_1));
		j++;
	}
	if((numbits/4) != j){
		printf("mod_16QAM(): ERROR numbits=%d, j=%d\n", numbits, j);
		exit(0);
	}

//	for(i=0; i<numbits/4; i++)*(symbols+i)=(float)i-((float)i)*I;

	return j;
}


int hard_demod_16QAM (char * bits, complex * symbols, int numsymb)
{
	int i, j=0;
	float in_real, in_imag;

	for (i=0;i<numsymb;i++)
	{
		in_real = creal(symbols[i]);
		in_imag = cimag(symbols[i]);
		bits[j++] = (in_real<0)?1:0;
		bits[j++] = (in_imag<0)?1:0;
		bits[j++] = (fabs(in_real)-QAM16_THRESHOLD<0)?1:0;
		bits[j++] = (fabs(in_imag)-QAM16_THRESHOLD<0)?1:0;
	}
	return j;
}

int soft_demod_16QAM(_Complex float *symbols, int numinputsymb, float *softbits)
{
	int i, j=0;
	float in_real, in_imag;

	// Normalize input
	norm16QAM(symbols, numinputsymb);

	for (i=0;i<numinputsymb;i++)
	{
		in_real = -creal(symbols[i]);
		in_imag = -cimag(symbols[i]);
		softbits[j++] = in_real;
		softbits[j++] = in_imag;
		softbits[j++] = fabs(in_real)-QAM16_THRESHOLD;
		softbits[j++] = fabs(in_imag)-QAM16_THRESHOLD;
	}
//	printf("soft_demod_16QAM(): numoutputbits=%d\n", j);
	return j;
}

int norm16QAM(_Complex float *inout, int length){
	int i;
	float auxR, auxI, averg=0.0, Q=-0.0015; //2.5
	static float ratio=0.0;

	for(i=0; i<length; i++){
		auxR=fabs(__real__ inout[i]);
		auxI=fabs(__imag__ inout[i]);
		averg = averg + auxR + auxI;
	}

	averg=averg/(2.0*(float)length+0.00000001);
	ratio = (QAM16_THRESHOLD)/(averg+Q);		//BER=0.038 SNR=-4dB
	for(i=0; i<length; i++){
		inout[i] = inout[i]*ratio;
	}
	return(1);
}


/* 64QAM #####################################################################################################3*/

	/* LTE-64QAM constellation:
                                         	   Q
	47-101111  45-101101  37-100101  39-100111 |  7-000111   5-000101  13-001101  15-001111
	46-101110  44-101100  36-100100  38-100110 |  6-000110   4-000100  12-001100  14-001110
	42-101010  40-101000  32-100000  34-100010 |  2-000010	 0-000000   8-001000  10-001010
	43-101011  41-101001  33-100001  35-100011 |  3-000011	 1-000001   9-001001  11-001011
	-------------------------------------------------------------------------------------------------------------> I
	59-111011  57-111001  49-110001  51-110011 | 19-010011  17-010001  25-011001  27-011011
	58-111010  56-111000  48-110000  50-110010 | 18-010010  16-010000  24-011000  26-011010
	62-111110  60-111100  52-110100  54-110110 | 22-010110	20-010100  28-011100  30-011110
	63-111111  61-111101  53-110101  55-110111 | 23-010111	21-010101  29-011101  31-011111  */

int mod_64QAM (char *bits, int numbits, _Complex float *symbols){
	int i, j=0;

	for (i=0; i<numbits; i+=6)
	{
		symbols[j] = ((bits[i+0]==1)?(-1):(+1))*(QAM64_THRESHOLD_2+((bits[i+2]==1)?(+1):(-1))*((bits[i+4]==1)?(QAM64_LEVEL_2):(QAM64_LEVEL_1)));
		symbols[j] += I*((bits[i+1]==1)?(-1):(+1))*(QAM64_THRESHOLD_2+((bits[i+3]==1)?(+1):(-1))*((bits[i+5]==1)?(QAM64_LEVEL_2):(QAM64_LEVEL_1)));
		j++;
	}
	return j;
}

int soft_demod_64QAM(_Complex float *symbols, int numinputsymb, float *softbits)
{
	int i, j=0;
	float in_real, in_imag;

	// Normalize input
	norm64QAM(symbols, numinputsymb);

	for (i=0;i<numinputsymb;i++)
	{
		in_real = -creal(symbols[i]);
		in_imag = -cimag(symbols[i]);

		softbits[j+0] = in_real;
		softbits[j+1] = in_imag;
		softbits[j+2] = fabs(in_real)-QAM64_THRESHOLD_2;
		softbits[j+3] = fabs(in_imag)-QAM64_THRESHOLD_2;
    	softbits[j+4] = fabsf(softbits[j+2]) - QAM64_THRESHOLD_1;
    	softbits[j+5] = fabsf(softbits[j+3]) - QAM64_THRESHOLD_1;
		j+=6;
	}

	return j;
}


/*void demod_64qam_lte(const cf_t* symbols, float* llr, int nsymbols)
{
  for (int i = 0; i < nsymbols; i++) {
    float yre = crealf(symbols[i]);
    float yim = cimagf(symbols[i]);

    llr[6 * i + 0] = -yre;
    llr[6 * i + 1] = -yim;
    llr[6 * i + 2] = fabsf(yre) - 4 / sqrtf(42);
    llr[6 * i + 3] = fabsf(yim) - 4 / sqrtf(42);
    llr[6 * i + 4] = fabsf(llr[6 * i + 2]) - 2 / sqrtf(42);
    llr[6 * i + 5] = fabsf(llr[6 * i + 3]) - 2 / sqrtf(42);
  }
}*/


int norm64QAM(_Complex float *inout, int length){
	int i;
	float auxR, auxI, averg=0.0, Q=0.0; 
	static float ratio=0.0;

	for(i=0; i<length; i++){
		auxR=fabs(__real__ inout[i]);
		auxI=fabs(__imag__ inout[i]);
		averg = averg + auxR + auxI;
	}

	averg=averg/(2.0*(float)length+0.00000001);
	ratio = (QAM64_THRESHOLD_2)/(averg+Q);		
	for(i=0; i<length; i++){
		inout[i] = inout[i]*ratio;
	}
	return(1);
}

/* 256QAM #####################################################################################################3*/

#ifdef COMPACT_256QAM
int mod_256QAM (char *bits, int numbits, _Complex float *symbols)
{

	int i, j=0;
	if(numbits==0)return(0);
	for (i=0;i<numbits;i+=8)
	{		
		symbols[j] = ((bits[i+0]==1)?(-1):(+1)) * (QAM256_THRESHOLD_3 + QAM256_THRESHOLD_2*((bits[i+2]==1)?(+1):(-1)) + ((bits[i+2]==1)?(+1):(-1)) * ((bits[i+4]==1)?(+1):(-1)) * QAM256_THRESHOLD_1 + ((bits[i+2]==1)?(+1):(-1)) * ((bits[i+4]==1)?(+1):(-1)) * ((bits[i+6]==1)?(+1):(-1)) * (QAM256_THRESHOLD_1/2.0));
		symbols[j] += I*((bits[i+1]==1)?(-1):(+1)) * (QAM256_THRESHOLD_3 + QAM256_THRESHOLD_2*((bits[i+3]==1)?(+1):(-1)) + ((bits[i+3]==1)?(+1):(-1)) * ((bits[i+5]==1)?(+1):(-1)) * QAM256_THRESHOLD_1 + ((bits[i+3]==1)?(+1):(-1)) * ((bits[i+5]==1)?(+1):(-1)) * ((bits[i+7]==1)?(+1):(-1)) * (QAM256_THRESHOLD_1/2.0));
		j++;
	}
	if((numbits/8) != j){
		printf("mod_256QAM(): ERROR numbits=%d, j=%d\n", numbits, j);
		exit(0);
	}
	return j;
}

int soft_demod_256QAM(_Complex float *symbols, int numinputsymb, float *softbits)
{
	int i, j=0;
	float in_real, in_imag;

	// Normalize input
	norm256QAM(symbols, numinputsymb);
	// Demodulate
	for (i=0;i<numinputsymb;i++)
	{
		in_real = -creal(symbols[i]);
		in_imag = -cimag(symbols[i]);
		softbits[j++] = in_real;
		softbits[j++] = in_imag;
		in_real = fabsf(in_real) - QAM256_THRESHOLD_3;
		in_imag = fabsf(in_imag) - QAM256_THRESHOLD_3;
		softbits[j++] = in_real;
		softbits[j++] = in_imag;
		in_real = fabsf(in_real) - QAM256_THRESHOLD_2;
		in_imag = fabsf(in_imag) - QAM256_THRESHOLD_2;
		softbits[j++] = in_real;
		softbits[j++] = in_imag;	
		in_real = fabsf(in_real) - QAM256_THRESHOLD_1;
		in_imag = fabsf(in_imag) - QAM256_THRESHOLD_1;
		softbits[j++] = in_real;
		softbits[j++] = in_imag;
	}
	return j;
}


int norm256QAM(_Complex float *inout, int length){
	int i;
	float auxR, auxI, averg=0.0, Q=0.0; 
	static float ratio=0.0;

	for(i=0; i<length; i++){
		auxR=fabs(__real__ inout[i]);
		auxI=fabs(__imag__ inout[i]);
		averg = averg + auxR + auxI;
	}
	averg=averg/(2.0*(float)length+0.00000001);
	ratio = (QAM256_THRESHOLD_3)/(averg+Q);		
	for(i=0; i<length; i++){
		inout[i] = inout[i]*ratio;
	}
	return(1);
}

#else

int mod_256QAM (char *bits, int numbits, _Complex float *symbols)
{	
	int i, j=0;

	if(numbits==0)return(0);
	for (i=0;i<numbits;i+=8)
	{
		if((bits[i+2]==1)&&(bits[i+4]==1)&&(bits[i+6]==1)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM256_LEVEL_8);
		}
		else if((bits[i+2]==1)&&(bits[i+4]==1)&&(bits[i+6]==0)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM256_LEVEL_7);
		}
		else if((bits[i+2]==1)&&(bits[i+4]==0)&&(bits[i+6]==0)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM256_LEVEL_6);
		}
		else if((bits[i+2]==1)&&(bits[i+4]==0)&&(bits[i+6]==1)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM256_LEVEL_5);
		}
		else if((bits[i+2]==0)&&(bits[i+4]==0)&&(bits[i+6]==1)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM256_LEVEL_4);
		}
		else if((bits[i+2]==0)&&(bits[i+4]==0)&&(bits[i+6]==0)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM256_LEVEL_3);
		}
		else if((bits[i+2]==0)&&(bits[i+4]==1)&&(bits[i+6]==0)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM256_LEVEL_2);
		}
		else if((bits[i+2]==0)&&(bits[i+4]==1)&&(bits[i+6]==1)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM256_LEVEL_1);
		}

		//Imaginary axis
		if((bits[i+3]==1)&&(bits[i+5]==1)&&(bits[i+7]==1)){
		symbols[j] += I*((bits[i+1]==1)?(-1.0):(+1.0))*QAM256_LEVEL_8;
		}
		else if((bits[i+3]==1)&&(bits[i+5]==1)&&(bits[i+7]==0)){
		symbols[j] += I*((bits[i+1]==1)?(-1.0):(+1.0))*QAM256_LEVEL_7;
		}
		else if((bits[i+3]==1)&&(bits[i+5]==0)&&(bits[i+7]==0)){
		symbols[j] += I*((bits[i+1]==1)?(-1.0):(+1.0))*QAM256_LEVEL_6;
		}
		else if((bits[i+3]==1)&&(bits[i+5]==0)&&(bits[i+7]==1)){
		symbols[j] += I*((bits[i+1]==1)?(-1.0):(+1.0))*QAM256_LEVEL_5;
		}
		else if((bits[i+3]==0)&&(bits[i+5]==0)&&(bits[i+7]==1)){
		symbols[j] += I*((bits[i+1]==1)?(-1.0):(+1.0))*QAM256_LEVEL_4;
		}
		else if((bits[i+3]==0)&&(bits[i+5]==0)&&(bits[i+7]==0)){
		symbols[j] += I*((bits[i+1]==1)?(-1.0):(+1.0))*QAM256_LEVEL_3;
		}
		else if((bits[i+3]==0)&&(bits[i+5]==1)&&(bits[i+7]==0)){
		symbols[j] += I*((bits[i+1]==1)?(-1.0):(+1.0))*QAM256_LEVEL_2;
		}
		else if((bits[i+3]==0)&&(bits[i+5]==1)&&(bits[i+7]==1)){
		symbols[j] += I*((bits[i+1]==1)?(-1.0):(+1.0))*QAM256_LEVEL_1;
		}
		j++;
	}
	if((numbits/8) != j){
		printf("mod_2566QAM(): ERROR numbits=%d, j=%d\n", numbits, j);
		exit(0);
	}
	return j;
}


int soft_demod_256QAM(_Complex float *symbols, int numinputsymb, float *softbits)
{
	int i, j=0;
	float in_real, in_imag;
	// Normalize input
	norm256QAM(symbols, numinputsymb);
	// Demodulate
	for (i=0;i<numinputsymb;i=i+1)
	{
		in_real = -creal(symbols[i]);
		in_imag = -cimag(symbols[i]);
		softbits[j++] = in_real; //bit0
		softbits[j++] = in_imag; //bit1
		//bit 2
		softbits[j++] = fabs(in_real)-QAM256_THRESHOLD_4;
		//bit 3
		softbits[j++] = fabs(in_imag)-QAM256_THRESHOLD_4;
		//bit 4
		if(fabs(in_real)>QAM256_THRESHOLD_4){
			softbits[j++] = fabs(in_real)-QAM256_THRESHOLD_6;
		}
		else if((fabs(in_real)<QAM256_THRESHOLD_4)&&(fabs(in_real)>QAM256_THRESHOLD_2)){
			softbits[j++] = -fabs(in_real); //Ponemos el negativo para que siempre sea 0
		}
		else if(fabs(in_real)<QAM256_THRESHOLD_2){
			softbits[j++] = fabs(in_real); //Ponemos que todos sean positivos ya que siempre seran 1s
		}
		//bit 5
		if(fabs(in_imag)>QAM256_THRESHOLD_4){
			softbits[j++] = fabs(in_imag)-QAM256_THRESHOLD_6;
		}
		else if((fabs(in_imag)<QAM256_THRESHOLD_4)&&(fabs(in_imag)>QAM256_THRESHOLD_2)){
			softbits[j++] = -fabs(in_imag); //Ponemos el negativo para que siempre sea 0
		}
		else if(fabs(in_imag)<QAM256_THRESHOLD_2){
			softbits[j++] = fabs(in_imag); //Ponemos que todos sean positivos ya que siempre seran 1s
		}
		//bit 6
		if((fabs(in_real)<QAM256_THRESHOLD_1) || (fabs(in_real)>QAM256_THRESHOLD_3 && fabs(in_real)<QAM256_THRESHOLD_5) || (fabs(in_real)>QAM256_THRESHOLD_7)){
			softbits[j++] = fabs(in_real); //Ponemos todos los bits positivos (a 1).
		}
		else if((fabs(in_real)>QAM256_THRESHOLD_1 && fabs(in_real)<QAM256_THRESHOLD_3) || (fabs(in_real)>QAM256_THRESHOLD_5 && fabs(in_real)<QAM256_THRESHOLD_7)){
			softbits[j++] = -fabs(in_real); //Ponemos todos los bits negativos (a 0).
		}
		//bit 7
		if((fabs(in_imag)<QAM256_THRESHOLD_1) || (fabs(in_imag)>QAM256_THRESHOLD_3 && fabs(in_imag)<QAM256_THRESHOLD_5) || (fabs(in_imag)>QAM256_THRESHOLD_7)){
			softbits[j++] = fabs(in_imag); //Ponemos todos los bits positivos (a 1).
		}
		else if((fabs(in_imag)>QAM256_THRESHOLD_1 && fabs(in_imag)<QAM256_THRESHOLD_3) || (fabs(in_imag)>QAM256_THRESHOLD_5 && fabs(in_imag)<QAM256_THRESHOLD_7)){
			softbits[j++] = -fabs(in_imag); //Ponemos todos los bits negativos (a 0).
		}
	}
	return j;
}


int norm256QAM(_Complex float *inout, int length){
	int i;
	float auxR, auxI, averg=0.0, Q=0.0; 
	static float ratio=0.0;

	for(i=0; i<length; i++){
		auxR=fabs(__real__ inout[i]);
		auxI=fabs(__imag__ inout[i]);
		averg = averg + auxR + auxI;
	}
	averg=averg/(2.0*(float)length+0.00000001);
	ratio = (QAM256_THRESHOLD_4)/(averg+Q);		
	for(i=0; i<length; i++){
		inout[i] = inout[i]*ratio;
	}
	return(1);
}

#endif













/* 1024QAM #####################################################################################################3*/
#ifdef COMPACT_1024QAM
int mod_1024QAM (char *bits, int numbits, _Complex float *symbols)
{

	int i, j=0;

	if(numbits==0)return(0);
	for (i=0;i<numbits;i+=10)
	{		
		symbols[j] = 0.03829*(1-2*bits[i+0])*(16-(1-2*bits[i+2])*(8-(1-2*bits[i+4])*(4-(1-2*bits[i+6])*(2-(1-2*bits[i+8])))));
		
		symbols[j] += I*0.03829*(1-2*bits[i+1])*(16-(1-2*bits[i+3])*(8-(1-2*bits[i+5])*(4-(1-2*bits[i+7])*(2-(1-2*bits[i+9])))));;

		j++;
	}

	if((numbits/10) != j){
		printf("mod_1024QAM(): ERROR numbits=%d, j=%d\n", numbits, j);
		exit(0);
	}

	return j;


}

int soft_demod_1024QAM(_Complex float *symbols, int numinputsymb, float *softbits)
{
	int i, j=0;
	float in_real, in_imag;

	// Normalize input
	norm1024QAM(symbols, numinputsymb);
	// Demodulate
	for (i=0;i<numinputsymb;i++)
	{
		in_real = -creal(symbols[i]);
		in_imag = -cimag(symbols[i]);
		softbits[j++] = in_real;
		softbits[j++] = in_imag;

		in_real = fabsf(in_real) - QAM1024_THRESHOLD_4;
		in_imag = fabsf(in_imag) - QAM1024_THRESHOLD_4;

		softbits[j++] = in_real;
		softbits[j++] = in_imag;

		in_real = fabsf(in_real) - QAM1024_THRESHOLD_3;
		in_imag = fabsf(in_imag) - QAM1024_THRESHOLD_3;

		softbits[j++] = in_real;
		softbits[j++] = in_imag;	

		in_real = fabsf(in_real) - QAM1024_THRESHOLD_2;
		in_imag = fabsf(in_imag) - QAM1024_THRESHOLD_2;

		softbits[j++] = in_real;
		softbits[j++] = in_imag;

		in_real = fabsf(in_real) - QAM1024_THRESHOLD_1;
		in_imag = fabsf(in_imag) - QAM1024_THRESHOLD_1;

		softbits[j++] = in_real;
		softbits[j++] = in_imag;
	}

	return j;
}


int norm1024QAM(_Complex float *inout, int length){
	int i;
	float auxR, auxI, averg=0.0, Q=0.0; 
	static float ratio=0.0;

	for(i=0; i<length; i++){
		auxR=fabs(__real__ inout[i]);
		auxI=fabs(__imag__ inout[i]);
		averg = averg + auxR + auxI;
	}
	averg=averg/(2.0*(float)length+0.00000001);
	printf("averg=%f\n", averg);
	ratio = (QAM1024_THRESHOLD_4)/(averg+Q);		
	for(i=0; i<length; i++){
		inout[i] = inout[i]*ratio;
	}
	return(1);
}


#else



int mod_1024QAM (char *bits, int numbits, _Complex float *symbols){	
	int i, j=0;

	if(numbits==0)return(0);
	for (i=0;i<numbits;i+=10){
		if((bits[i+2]==1)&&(bits[i+4]==1)&&(bits[i+6]==1)&&(bits[i+8]==1)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_16);
		}
		else if((bits[i+2]==1)&&(bits[i+4]==1)&&(bits[i+6]==1)&&(bits[i+8]==0)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_15);
		}
		else if((bits[i+2]==1)&&(bits[i+4]==1)&&(bits[i+6]==0)&&(bits[i+8]==0)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_14);
		}
		else if((bits[i+2]==1)&&(bits[i+4]==1)&&(bits[i+6]==0)&&(bits[i+8]==1)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_13);
		}
		else if((bits[i+2]==1)&&(bits[i+4]==0)&&(bits[i+6]==0)&&(bits[i+8]==1)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_12);
		}
		else if((bits[i+2]==1)&&(bits[i+4]==0)&&(bits[i+6]==0)&&(bits[i+8]==0)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_11);
		}
		else if((bits[i+2]==1)&&(bits[i+4]==0)&&(bits[i+6]==1)&&(bits[i+8]==0)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_10);
		}
		else if((bits[i+2]==1)&&(bits[i+4]==0)&&(bits[i+6]==1)&&(bits[i+8]==1)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_9);
		}
		else if((bits[i+2]==0)&&(bits[i+4]==0)&&(bits[i+6]==1)&&(bits[i+8]==1)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_8);
		}
		else if((bits[i+2]==0)&&(bits[i+4]==0)&&(bits[i+6]==1)&&(bits[i+8]==0)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_7);
		}
		else if((bits[i+2]==0)&&(bits[i+4]==0)&&(bits[i+6]==0)&&(bits[i+8]==0)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_6);
		}
		else if((bits[i+2]==0)&&(bits[i+4]==0)&&(bits[i+6]==0)&&(bits[i+8]==1)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_5);
		}
		else if((bits[i+2]==0)&&(bits[i+4]==1)&&(bits[i+6]==0)&&(bits[i+8]==1)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_4);
		}
		else if((bits[i+2]==0)&&(bits[i+4]==1)&&(bits[i+6]==0)&&(bits[i+8]==0)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_3);
		}
		else if((bits[i+2]==0)&&(bits[i+4]==1)&&(bits[i+6]==1)&&(bits[i+8]==0)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_2);
		}
		else if((bits[i+2]==0)&&(bits[i+4]==1)&&(bits[i+6]==1)&&(bits[i+8]==1)){
		symbols[j]  =   ((bits[i+0]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_1);
		}
		
		//Imaginary axis
		if((bits[i+3]==1)&&(bits[i+5]==1)&&(bits[i+7]==1)&&(bits[i+9]==1)){
		symbols[j]  += I*((bits[i+1]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_16);
		}
		else if((bits[i+3]==1)&&(bits[i+5]==1)&&(bits[i+7]==1)&&(bits[i+9]==0)){
		symbols[j]  += I*((bits[i+1]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_15);
		}
		else if((bits[i+3]==1)&&(bits[i+5]==1)&&(bits[i+7]==0)&&(bits[i+9]==0)){
		symbols[j]  += I*((bits[i+1]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_14);
		}
		else if((bits[i+3]==1)&&(bits[i+5]==1)&&(bits[i+7]==0)&&(bits[i+9]==1)){
		symbols[j]  += I*((bits[i+1]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_13);
		}
		else if((bits[i+3]==1)&&(bits[i+5]==0)&&(bits[i+7]==0)&&(bits[i+9]==1)){
		symbols[j]  += I*((bits[i+1]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_12);
		}
		else if((bits[i+3]==1)&&(bits[i+5]==0)&&(bits[i+7]==0)&&(bits[i+9]==0)){
		symbols[j]  += I*((bits[i+1]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_11);
		}
		else if((bits[i+3]==1)&&(bits[i+5]==0)&&(bits[i+7]==1)&&(bits[i+9]==0)){
		symbols[j]  += I*((bits[i+1]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_10);
		}
		else if((bits[i+3]==1)&&(bits[i+5]==0)&&(bits[i+7]==1)&&(bits[i+9]==1)){
		symbols[j]  += I*((bits[i+1]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_9);
		}
		else if((bits[i+3]==0)&&(bits[i+5]==0)&&(bits[i+7]==1)&&(bits[i+9]==1)){
		symbols[j]  += I*((bits[i+1]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_8);
		}
		else if((bits[i+3]==0)&&(bits[i+5]==0)&&(bits[i+7]==1)&&(bits[i+9]==0)){
		symbols[j]  += I*((bits[i+1]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_7);
		}
		else if((bits[i+3]==0)&&(bits[i+5]==0)&&(bits[i+7]==0)&&(bits[i+9]==0)){
		symbols[j]  += I*((bits[i+1]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_6);
		}
		else if((bits[i+3]==0)&&(bits[i+5]==0)&&(bits[i+7]==0)&&(bits[i+9]==1)){
		symbols[j]  += I*((bits[i+1]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_5);
		}
		else if((bits[i+3]==0)&&(bits[i+5]==1)&&(bits[i+7]==0)&&(bits[i+9]==1)){
		symbols[j]  += I*((bits[i+1]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_4);
		}
		else if((bits[i+3]==0)&&(bits[i+5]==1)&&(bits[i+7]==0)&&(bits[i+9]==0)){
		symbols[j]  += I*((bits[i+1]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_3);
		}
		else if((bits[i+3]==0)&&(bits[i+5]==1)&&(bits[i+7]==1)&&(bits[i+9]==0)){
		symbols[j]  += I*((bits[i+1]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_2);
		}
		else if((bits[i+3]==0)&&(bits[i+5]==1)&&(bits[i+7]==1)&&(bits[i+9]==1)){
		symbols[j]  += I*((bits[i+1]==1)?(-1.0):(+1.0)) * (QAM1024_LEVEL_1);
		}
		j++;
	}
	if((numbits/10) != j){
		printf("mod_1024QAM(): ERROR numbits=%d, j=%d\n", numbits, j);
		exit(0);
	}
	return j;
}





int soft_demod_1024QAM(_Complex float *symbols, int numinputsymb, float *softbits){
	int i, j=0;
	float in_real, in_imag;
	// Normalize input
	norm1024QAM(symbols, numinputsymb);
	// Demodulate
	for (i=0;i<numinputsymb;i=i+1){
		in_real = -creal(symbols[i]);
		in_imag = -cimag(symbols[i]);
		softbits[j++] = in_real; //bit0
		softbits[j++] = in_imag; //bit1
		//bit 2
		softbits[j++] = fabs(in_real)-QAM1024_THRESHOLD_8;
		//bit 3
		softbits[j++] = fabs(in_imag)-QAM1024_THRESHOLD_8;
		//bit 4
		if(fabs(in_real)<QAM1024_THRESHOLD_4 || fabs(in_real)>QAM1024_THRESHOLD_12){
			softbits[j++] = fabs(in_real);
		}
		else{
			softbits[j++] = -fabs(in_real); //Ponemos el negativo para que siempre sea 0
		}
		//bit 5
		if(fabs(in_imag)<QAM1024_THRESHOLD_4 || fabs(in_imag)>QAM1024_THRESHOLD_12){
			softbits[j++] = fabs(in_imag);
		}
		else{
			softbits[j++] = -fabs(in_imag); //Ponemos el negativo para que siempre sea 0
		}
		//bit 6		
		if((fabs(in_real)<QAM1024_THRESHOLD_2) || (fabs(in_real)>QAM1024_THRESHOLD_6 && fabs(in_real)<QAM1024_THRESHOLD_10) || (fabs(in_real)>QAM1024_THRESHOLD_14)){
			softbits[j++] = fabs(in_real); //Ponemos todos los bits positivos (a 1).
		}
		else{
			softbits[j++] = -fabs(in_real); //Ponemos todos los bits negativos (a 0).
		}
		//bit 7
		if((fabs(in_imag)<QAM1024_THRESHOLD_2) || (fabs(in_imag)>QAM1024_THRESHOLD_6 && fabs(in_imag)<QAM1024_THRESHOLD_10) || (fabs(in_imag)>QAM1024_THRESHOLD_14)){
			softbits[j++] = fabs(in_imag); //Ponemos todos los bits positivos (a 1).
		}
		else{
			softbits[j++] = -fabs(in_imag); //Ponemos todos los bits negativos (a 0).
		}
		//bit 8	
		if((fabs(in_real)<QAM1024_THRESHOLD_1) || (fabs(in_real)>QAM1024_THRESHOLD_3 && fabs(in_real)<QAM1024_THRESHOLD_5) || (fabs(in_real)>QAM1024_THRESHOLD_7 && fabs(in_real)<QAM1024_THRESHOLD_9) || (fabs(in_real)>QAM1024_THRESHOLD_11 && fabs(in_real)<QAM1024_THRESHOLD_13) || (fabs(in_real)>QAM1024_THRESHOLD_15)){
			softbits[j++] = fabs(in_real); //Ponemos todos los bits positivos (a 1).
		}
		else{
			softbits[j++] = -fabs(in_real); //Ponemos todos los bits negativos (a 0).
		}
		//bit 9	
		if((fabs(in_imag)<QAM1024_THRESHOLD_1) || (fabs(in_imag)>QAM1024_THRESHOLD_3 && fabs(in_imag)<QAM1024_THRESHOLD_5) || (fabs(in_imag)>QAM1024_THRESHOLD_7 && fabs(in_imag)<QAM1024_THRESHOLD_9) || (fabs(in_imag)>QAM1024_THRESHOLD_11 && fabs(in_imag)<QAM1024_THRESHOLD_13) || (fabs(in_imag)>QAM1024_THRESHOLD_15)){
			softbits[j++] = fabs(in_imag); //Ponemos todos los bits positivos (a 1).
		}
		else{
			softbits[j++] = -fabs(in_imag); //Ponemos todos los bits negativos (a 0).
		}
	}
	return j;
}


int norm1024QAM(_Complex float *inout, int length){
	int i;
	float auxR, auxI, averg=0.0, Q=0.0; 
	static float ratio=0.0;

	for(i=0; i<length; i++){
		auxR=fabs(__real__ inout[i]);
		auxI=fabs(__imag__ inout[i]);
		averg = averg + auxR + auxI;
	}
	averg=averg/(2.0*(float)length+0.00000001);
	printf("averg=%f\n", averg);
	ratio = (QAM1024_THRESHOLD_8)/(averg+Q);		
	for(i=0; i<length; i++){
		inout[i] = inout[i]*ratio;
	}
	return(1);
}

#endif

