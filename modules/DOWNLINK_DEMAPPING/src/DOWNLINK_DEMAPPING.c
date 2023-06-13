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

#include "DOWNLINK_DEMAPPING_interfaces.h"
#include "DOWNLINK_DEMAPPING_functions.h"
#include "DOWNLINK_DEMAPPING.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="DOWNLINK_DEMAPPING";
int Tslot=-1;
int FFTsize=128;
int DATAsize=0;

#define NORMAL_NORS				0				// Normal Operation Mode
#define NORMAL_WITHRS			1			// Normal Operation Mode
int opMODE=NORMAL_NORS;
int PHYLayerID=0;




//Module User Defined Parameters

//Global Variables
_Complex float bufferA[2048];
float bufferB[2048];

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
	param_get_int("opMODE", &opMODE);
	param_get_int("FFTsize", &FFTsize);
	param_get_int("PHYLayerID", &PHYLayerID);
	

	/* Verify control parameters */
	if (FFTsize != 128) {
		// ONLY FFTsize=128 CHECKED
		/*Include the file name and line number when printing*/
		printf("ERROR: FFTsize=%d != 128\n", FFTsize);
		moderror_msg("ERROR: FFTsize=%d != 128\n", FFTsize);
		return -1;
	}

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName()); 
	//mname=GetObjectName();
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      opMODE=%d [0-NORMAL_NORS, 1-NORMAL_WITHRS], FFTsize=%d, PHYLayerID=%d\n", opMODE, FFTsize, PHYLayerID);
	printf("O--------------------------------------------------------------------------------------------O\n");

	/* do some other initialization stuff */
	//Number of carriers per FFTsize
	if(FFTsize==128)DATAsize=72;
	if(FFTsize==256)DATAsize=180;
	if(FFTsize==512)DATAsize=300;
	if(FFTsize==1024)DATAsize=600;
	if(FFTsize==1536)DATAsize=900;
	if(FFTsize==2048)DATAsize=1200;

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
	int i, j=0, numFFTs, snd_samples=0;
	static int numSubframe=-1;
	static int firstSubframe=0;
	
	Tslot++;
	// Check if data received
	if (rcv_samples == 0)return(0);

	// Check if data exceed maximum expected data
	numFFTs=rcv_samples/FFTsize;
	if (rcv_samples != numFFTs*FFTsize) {
		printf("%s| ERROR: Received samples=%d is not multiple of FFTsize=%d\n", rcv_samples, FFTsize);
		return(-1);
	}
	// UPDATE NUMBER OF SUBFRAME
	numSubframe++;
	if(numSubframe==5)numSubframe=0;

	// NO RS SIGNALS INCLUDED
	if(opMODE==NORMAL_NORS){
		// FIRST SUBFRAME: DOUBLE PSS
		if(firstSubframe==0){
			for(i=numFFTs/2; i<numFFTs; i++){
				getDATAfromLTE_DOWNLINKspectrum(&inp[FFTsize*i], FFTsize, DATAsize, &out[DATAsize*j]);
				snd_samples += DATAsize;
				j++;
			}
			firstSubframe=1;
		} // NORMAL SUBFRAME: EVERY 5 SUBFRAMES PSS RECEIVED		
		else{
			for(i=0; i<numFFTs; i++){
				if(numSubframe != 0 || i != 6){
					getDATAfromLTE_DOWNLINKspectrum(&inp[FFTsize*i], FFTsize, DATAsize, &out[DATAsize*j]);
					snd_samples += DATAsize;
					j++;
				}
			}
		}
	}
	if(opMODE==NORMAL_WITHRS){
		//snd_samples=extractDataFromOFDMflowWithRS(inp, rcv_samples, FFTsize, out, PHYLayerID);
	}


	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


