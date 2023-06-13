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
// Primary synchronization signal definitions
#define PI 3.14159265358979323846
#define PSSLENGTH 	62	//Number of PSS Symbols in Zadoff-Chu sequence
#define PSSCELLID0 	25.0
#define PSSCELLID1 	29.0
#define PSSCELLID2 	34.0
#define MAXFFTSIZE	2048


#define NOFOFDMSYMBPERSUBFRAME	14

int setPSS(int phylayerID, _Complex float *PSSsymb, int TxRxMode);
int genPSStime_seq(int phylayerID, int FFTsize, _Complex float  *PSS_time, int TxRxmode);
int genPSSfreq_seq_rotated(int phylayerID, int FFTsize, _Complex float  *PSS_freq, int TxRxmode);

void conv_cc(_Complex float *input, _Complex float *filter, _Complex float *output, int input_len, int filter_len);
int detect_PSS(_Complex float *inout, int length, float *variance);
int detect_2onPSS(_Complex float *inout, int length, int pMAX, float *variance);
int stream_conv_CPLX(_Complex float *ccinput, int datalength,_Complex float *filtercoeff, int filterlength, _Complex float *ccoutput);
void process_correllation(_Complex float *inCorrel, int datalength, _Complex float *outCorrel);
void process_correllation2(_Complex float *inCorrel, int datalength, _Complex float *outCorrel);
//int cCORRscan(_Complex float *input, int outlength, int *scaninit, _Complex float *graph0);


float compute_in_gain(_Complex float *inout, int length, int pPSS);
float compute_in_gainAver(_Complex float *inout, int length, int pPSS);
int gain_vectorCPLX(_Complex float *in, _Complex float *out, int length, float gain);

//int write_subframe_buffer(_Complex float *datain, int pMAX, int rcv_samples);
//int read_subframe_buffer(_Complex float *datain);

int write_subframe_buffer(_Complex float *datain, int pMAX, int rcv_samples, int FFTsize);
int read_subframe_buffer(_Complex float *dataout, int FFTsize);

void rotateCvector(_Complex float *in, _Complex float *out, int length, float Adegrees);
float checkPSSphase(_Complex float *PSSreceived, _Complex float *PSSseqRX, int length);
float checkPhaseOffset(_Complex float *PSScorrelation);
void testPhaseOffset();

int channel_estimator(_Complex float *PSSreceived, _Complex float *PSSfreqseqTX, int FFTsize, _Complex float *CHestimation);
#endif
