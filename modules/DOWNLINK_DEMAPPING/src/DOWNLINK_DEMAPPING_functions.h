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

//#define DOWNLINK 	0
//#define UPLINK 		1

//int getDATAfromLTEspectrum(_Complex float *LTEspect, int FFTlength, int datalength, _Complex float *QAMsymb);
int getDATAfromLTE_UPLINKspectrum(_Complex float *LTEspect, int FFTlength, int datalength, _Complex float *QAMsymb);
int getDATAfromLTE_DOWNLINKspectrum(_Complex float *LTEspect, int FFTlength, int datalength, _Complex float *QAMsymb);
int rmoveRSinOFDMsymb(_Complex float *OFDMsymb, int FFTlength, 
										 _Complex float *out_data, int phylayerID);
int extractDataFromOFDMflowWithRS(_Complex float *OFDMsymb, int rcv_samples, int FFTsize, 
										 _Complex float *out_data, int phylayerID);
#endif
