/* 
 * Copyright (c) 2012
 * This file is part of ALOE++ (http://flexnets.upc.edu/)
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


/* 
 * This file containts the functions related with the capture of module
 * parameter configuration from the external file "object_name.params".
 * 
 * ALL parameters to be captured must be listed in the parameters[] variable 
 * definition located at the module_params.h file
 */


#include <complex.h>
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <swapi_utils.h>
#include <phal_sw_api.h>

//Skeleton includes
#include "print_utils.h"
#include "params.h"
#include "skeleton.h"

//Module includes
#include "EQUALIZER1_functions.h"

extern MODparams_t oParam;
extern char mname[];
extern const int nof_input_itf;
extern const int nof_output_itf;

void cleanstring(char *string);
void printCOLORline(char *printformat, int position, char *data2print, char *fore, char *ground);
void printCOLORtext(int position, char *fore, char *ground,  int end, char *data2print);
                                                                                                                                         
#define BACKGROUNDLEN	100
char background[BACKGROUNDLEN]=""; 
char backgroundREF[BACKGROUNDLEN]="                                                                                                 ";
char lineREF[BACKGROUNDLEN]="=================================================================================================";
char aux[BACKGROUNDLEN];
                                                                                
//http://www.bitmote.com/index.php?post/2012/11/19/Using-ANSI-Color-Codes-to-Colorize-Your-Bash-Prompt-on-Linux
char format[BACKGROUNDLEN]="\33[1m\33[38;5;021;48;5;226m";
char reset[10]="\033[0m";


int get_config_params(){

	/* Get control parameters value from modulename.params file*/
	// Params: local to module control parameters
	param_get_int("opMODE", &oParam.opMODE);							//Initialized by hand or .params file
//	param_get_int("datalength", &oParam.datalength);					//Initialized by hand or .params file
//	param_get_int("num_operations", &oParam.num_operations);			//Initialized by hand or .params file
//	param_get_float("constant", &oParam.constant);
//	param_get_string("datatext", &oParam.datatext[0]);
	// Please, add here the variables to be setup from the external parameters file. 
	return(0);
}

#define END 97
#define BACK0	"231"
#define BACK1	"155"
#define TEXT0	"196"
#define TEXT1	"000"
#define TEXT2	"215"
#define TEXT3	"027"

void print_params(char *intype, char *outtype){
	int init, end;

//void printCOLORtext(int position, char *fore, char *ground,  int end, char *data2print)

	init=2,
	end=init+strlen(mname)+2;
	printCOLORtext(init, TEXT0, BACK0, end, mname);
	init=0;
	end=END-end;
	printCOLORtext(init, TEXT1, BACK1, end, "==================================================================================================");
	printf("\n");
	sprintf(aux, "Interfaces");
	printCOLORtext(3, TEXT2, BACK1, END, aux);
	printf("\n");
	cleanstring(aux);

	sprintf(aux, "Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s", 
		       												nof_input_itf, intype, nof_output_itf, outtype);
	printCOLORtext(4, TEXT1, BACK1, END, aux);
	printf("\n");
	cleanstring(aux);

	sprintf(aux, "Params"); 
	printCOLORtext(3, TEXT3, BACK1, END, aux);
	printf("\n");
	cleanstring(aux);

	sprintf(aux, "opMODE=%d (0: DEBUGG, 1: BYPASS, 2: NORMAL)", oParam.opMODE); 
	printCOLORtext(4, TEXT1, BACK1, END, aux);
	printf("\n");
	cleanstring(aux);
	printCOLORtext(0, TEXT1, BACK1, END, "==================================================================================================");
	printf("\n");
}

void printCOLORtext(int position, char *fore, char *ground,  int end, char *data2print){
	
	int length=(int)BACKGROUNDLEN;
	char formatt[BACKGROUNDLEN]="\33[1m\33[38;5;021;48;5;226m";

	if(strlen(data2print)>length){
		printf("EQUALIZER0:printCOLORtext() ERROR!!! data2print length = %d > BACKGROUNDLEN=%d\n", strlen(data2print), length);
		exit(0);
	}

	memcpy(formatt+11, fore, strlen(fore));
	memcpy(formatt+20, ground, strlen(ground));
	memcpy(background, backgroundREF, strlen(backgroundREF));	
	memcpy(background+position, data2print, strlen(data2print));
//	background[position+strlen(data2print)+1]=NULL;
	background[end]=NULL;
	printf("%s%s%s", formatt, background, reset);
}


void cleanstring(char *string){
	*string=NULL;
}

/**
 * @check_config_params(). Check any value that should be checked and act accordingly before RUN phase
 * @param.
 * @oParaml Refers to the struct that containts the module parameters
 * @return: Void
 */
void check_config_params(){

	if (oParam.num_operations > MAXOPERATIONS) {
		printf("ERROR: p_num_operations=%d > MAXOPERATIONS=%d\n", oParam.num_operations, MAXOPERATIONS);
		printf("Check your module_interfaces.h file\n");
		exit(EXIT_FAILURE);
	}
}




/*
#define MAXLINES	20
char printINITtext[MAXLINES][BACKGROUNDLEN];
int noflines=0;



void printCOLORline(char *printformat, int position, char *data2print, char *fore, char *ground){

	if(strlen(data2print)>BACKGROUNDLEN){
		printf("EQUALIZER0:printCOLORlone() ERROR!!! data2print length = %d > BACKGROUNDLEN=%d\n", strlen(data2print), (int)BACKGROUNDLEN);
		exit(0);
	}
	memcpy(printformat+11, fore, strlen(fore));
	memcpy(printformat+20, ground, strlen(ground));
	memcpy(background, backgroundREF, strlen(backgroundREF));	
	memcpy(background+position, data2print, strlen(data2print)-1);
//	printf("A0_strlen(background)=%d\n", strlen(background));
	printf("%s%s%s\n", printformat, background, reset);
}




void print_itfs(char *intype, char *outtype){

	int i;
	int init, end;
                                     
	sprintf(&printINITtext[0][0], "    =============================================================================================");
	memcpy(&printINITtext[0][4], mname, strlen(mname));
	memcpy(&printINITtext[0][4+strlen(mname)], " ", 1);
	noflines++;
	sprintf(&printINITtext[noflines][0], "     INTERFACES\n");
	noflines++;
	sprintf(&printINITtext[noflines][0], "      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       												nof_input_itf, intype, nof_output_itf, outtype);
	noflines++;
	sprintf(&printINITtext[noflines][0], "     PARAMS\n");
	noflines++;
	sprintf(&printINITtext[noflines][0], "      opMODE=%d (), datalength=%d\n", oParam.opMODE, oParam.datalength);
	noflines++;
	// ADD here other lines if needed
	sprintf(&printINITtext[noflines][0], "    =============================================================================================");
	noflines++;
	for(i=0; i<noflines; i++)printCOLORline(format,0, &printINITtext[i][0], "000", "115");
}*/
