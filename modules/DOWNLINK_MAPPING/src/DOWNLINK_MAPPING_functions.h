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

#ifndef MAPPINGV_FUNCTIONS_H
#define MAPPINGV_FUNCTIONS_H

#include <fftw3.h>

#define PI 3.14159265358979323846
#define PSSLENGTH 		62				//Number of PSS Symbols in Zadoff-Chu sequence
#define PSSCELLID0 		25.0
#define PSSCELLID1 		29.0
#define PSSCELLID2 		34.0


#define LAB2			0
#define LAB3			1
#define NOT2SEND		7

#define NORMAL_NORS				0				// Normal Operation Mode
#define NORMAL_WITHRS			1			// Normal Operation Mode
#define BYPASS						2				// Bypass Mode: Output = Input
#define DEBUGG						3				// Debugg Mode according DEBUGG_XX value.

#define NODEBUGMODE				1			// NO debugging
#define DEBUGPRINTMODE		2			// Print Relevant Info
#define DEBUG_IN_RAMP			3			// Use a ramp signal as input
#define DEBUG_IN_ALLZERO	4			// Use all zeros signal as input
#define DEBUG_IN_ALLONES	5			// Use all ones signal as input

typedef struct MODparams{
    	int opMODE;
		int debugMODE;
    	int numchars7FFT;
    	int numchars13FFT;
    	int numchars14FFT;
		int numTSNOT2SEND;			// Number of TS not to sent due to JACK issues
		int numTSChainDelay;		// Number of Delayed Tslots due to modules chain
		int phylayerID;				// Cell identifier 0-504
		int FFTsz;					// FFT size
}MODparams_t;

#define S_INIT		 	0
#define S_WORK		 	1

#define A_NOTHING		0
#define A_ZEROSF		1
#define A_PSSZEROSF		2
#define A_14DATASF		3
#define A_13PSSDATASF	4
#define A_1STDATASF		5

typedef struct MODvars{
    int state;
	int action;					// Identifies which action to do
	int NUMSubframe;
	int numchars2rcv;
	int ActiveCarriers;				
}MODvars_t;

int control_SFrame(MODvars_t *oVars, MODparams_t *oParam, int rcv_data);
/*int generate_LTE_subframe(MODvars_t *oVars, MODparams_t *oParam, 
							_Complex float *inMQAMsymb, int rcv_samples, 
							_Complex float *PSSseq,
							_Complex float *outbuffer);
*/
int generate_LTE_subframe(MODvars_t *oVars, MODparams_t *oParam, 
							_Complex float *inMQAMsymb, int rcv_samples, 
							_Complex float *PSSseq,
							_Complex float *outbuffer,
							char *OBJname);

int setPSS(int CellID, _Complex float *PSSsymb, int TxRxMode);
int genPSStime_seq(int cellID, int FFTsize, fftwf_complex  *PSS_time, int TxRxmode);
int genPSSfreq_seq(int phylayerID, int FFTsize, fftwf_complex  *PSS_freq, int TxRxmode);
int create_LTEspectrumNORS(_Complex float *MQAMsymb, int FFTlength, 
						int datalength, _Complex float *out_spectrum);


_Complex float addRS();
int create_LTEspectrumWITHRS(_Complex float *MQAMsymb, int FFTlength, 
												int datalength, _Complex float *out_spectrum, int phylayerID);
void addRSinOFDMsymb(_Complex float *MQAMsymb, int FFTlength, 
										 _Complex float *out_data, int phylayerID);
int addRSinDataFlow(_Complex float *MQAMsymb, int FFTlength, 
												int datalength, _Complex float *out_data, 
												int phylayerID);



int sendzeros_SUBFRAME(int fftlength, _Complex float *outbuffer);
int sendzerosPSS_SUBFRAME(_Complex float *PSS, int fftlength, _Complex float *outbuffer);
int send_first_SUBFRAME(_Complex float *PSS, _Complex float *inbuffer, 
						int fftlength, int datasize, _Complex float *outbuffer);
int send_data_SUBFRAME(_Complex float *inbuffer, int fftlength, 
						int datasize, _Complex float *outbuffer);
int send_pss_data_SUBFRAME(_Complex float *PSS, _Complex float *inbuffer, 
					int fftlength, int datasize, _Complex float *outbuffer);

#endif
