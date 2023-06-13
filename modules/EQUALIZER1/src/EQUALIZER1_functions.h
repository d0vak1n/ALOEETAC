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

#define SLENGTH			32
#define DEBUGG			0
#define BYPASS			1
#define NORMAL			2

#define MAXOPERATIONS	20000

typedef struct MODparams{
    int opMODE;
		int datalength;
    int num_operations;
    float constant;
    char datatext[SLENGTH];
}MODparams_t;



int bypass_CPLX(_Complex float *datin, int datalength, _Complex float *datout);
int equalize(_Complex float *datain, int datalength, _Complex float *dataout, _Complex float *channel);
#endif
