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

#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H

// Module Variables and Defines

#define SLENGTH			32
#define DEBUGG									0
#define FREQ_CHANNEL_ESTIMATION					1
#define BYPASS									2

#define MAXOPERATIONS	20000

typedef struct MODparams{
    int opMODE;
		int datalength;
    int num_operations;
    float constant;
    char datatext[SLENGTH];
}MODparams_t;




/*************************************************************************************************/
// Functions Predefinition
// Primary synchronization signal definitions
#define PI 3.14159265358979323846
#define DMRSLENGTH 	156	//Number of DMRS Symbols in Zadoff-Chu sequence
#define DMRSCELLID0 	25.0
#define DMRSCELLID1 	29.0
#define DMRSCELLID2 	34.0
#define MAXFFTSIZE	2048

int setDMRS(int phylayerID, _Complex float *DMRSsymb, int TxRxMode);
int genDMRStime_seq(int phylayerID, int FFTsize, _Complex float  *DMRS_time, int TxRxmode);
int genDMRSfreq_seq_rotated(int phylayerID, int FFTsize, _Complex float  *DMRS_freq, int TxRxmode);
int time2freq(int FFTsize, _Complex float *input, _Complex float *output);

// CHANNEL ESTIMATION
int channel_estimator_freq(_Complex float *DMRSreceived, 
														_Complex float *DMRSfreqseqTX, 
														int FFTsize, _Complex float *CHestimation);




int bypass_CPLX(_Complex float *input, int inlength, _Complex float *output);

float computeEVM_3GGP_LTE256(_Complex float *inputMeasured, _Complex float *inputReference, int nofsamples);
float computeEVM_3GGP_LTE256_2(_Complex float *inputMeasured, _Complex float *inputReference, int nofsamples);
int normA(_Complex float *inout, int length);
int normB(_Complex float *inout, int length);

#endif
