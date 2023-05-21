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
#include "print_utils.h"

#include "EQUALIZER1_interfaces.h"
#include "EQUALIZER1_functions.h"
#include "EQUALIZER1.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="EQUALIZER1";
MODparams_t oParam={BYPASS, 256, 100, 7.9, "BABY"};		// Initialize module params struct
unsigned long int numTS;			/* Time slot number */

//Global Variables
// Module variables: Modify according your needs
_Complex float CHANNEL[256];

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

#ifndef DEBUG_STANDALONE
	sprintf(mname, "%s", GetObjectName());
#endif

	/* Get control parameters value from modulename.params file*/
	get_config_params();

	/* Verify control parameters */
	check_config_params();

	/* Print Module Init Parameters */
	print_params(IN_TYPE, OUT_TYPE);					// Print Interfaces and Params configuration

	/* do some other initialization stuff */
	//printf("EQUALIZER: opMODE=%d\n", oParam.opMODE);



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
	int i;
	static int Tslot=0;
	int snd_samples0=0, rcv_samples0=0, snd_samples1=0, rcv_samples1=0;
	input_t *input0, *input1;
	output_t *output0, *output1;
		
#ifndef DEBUG_STANDALONE
//	printf(BOLD T_RED"WORK() IN %s: Tslot=%d///////////////////////////////////////////////////////////////////////////d"RESET"\n", GetObjectName(), Tslot);
#endif

	/* GET THE POINTER FOR INPUT AND OUTPUT DATA BUFFERS*/
	input0 = in(inp,0);
	input1 = in(inp,1);
	output0 = out(out,0);
	output1 = out(out,1);

	/* GET THE NUMBER OF SAMPLES RECEIVED */
	rcv_samples0=get_input_samples(0);
	rcv_samples1=get_input_samples(1);

	/* DO YOUR CHECKS BEFORE DSP TASKS*/
	if (rcv_samples0 == 0)return(1); 							

	/* PUT HERE YOUR DSP TASKS ///////////////////////////////*/
	if(oParam.opMODE==BYPASS){
//		printf("EQUALIZER: BYPASS\n");
		snd_samples0=bypass_CPLX(input0, rcv_samples0, output0);
		snd_samples1=bypass_CPLX(input1, rcv_samples1, output1);
	}
	if(oParam.opMODE==NORMAL){
		snd_samples0=equalize(input0, rcv_samples0, output0, input1);
		snd_samples1=bypass_CPLX(input1, rcv_samples1, output1);
	}

	/* INDICATE THE NUMBER OF OUTPUT SAMPLES AT EACH OUTPUT */
	set_output_samples(0, snd_samples0);								//	set_output_samples(Output_number, number_of_samples);
	set_output_samples(1, snd_samples1);

	// UPDATE TIME SLOT COUNTER
	Tslot++;
	return 1;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


