#########################################################################REPORT

object {
	obj_name=REPORT1
	exe_name=REPORT
	inputs {
		name=input_0
		remote_itf=output_2
		remote_obj=DATASOURCESINK
	}
	outputs {
		name=output_0
		remote_itf=input_1
		remote_obj=CHAN_NOISE
	}
}
#########################################################################REPORT
########################################DATASOURCE
object {
	obj_name=DATASOURCESINK
	exe_name=DATASOURCESINK_REPORTP21
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=UNCRC
	}

	inputs {
		name=input_2
		remote_itf=output_1
		remote_obj=UNCRC
	}
	outputs {
		name=output_2
		remote_itf=input_0
		remote_obj=REPORT1
	}
}
########################################DATASOURCE


#####################################################DISTOR_CHANNEL
object {
	obj_name=DISTOR_CHANNEL
	exe_name=CPLX_FILTER_REPORT
	force_pe=0

	inputs {
		name=input_1
		remote_itf=output_1
		remote_obj=CHAN_NOISE
	}
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=CHAN_NOISE
	}
	outputs {
		name=output_1
		remote_itf=input_1
		remote_obj=UNCRC
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=CIRC_BUFFER_RX
	}
}
####################################################DISTOR_CHANNEL
#########################################################################DAC_JACK0

object {
	obj_name=DAC_JACK0
	exe_name=DAC_JACK0
#	force_pe=0
	kopts=15
	outputs {
		name=output
		remote_itf=input_0
		remote_obj=CHAN_NOISE
	}
}
#########################################################################DAC_JACK0
####################################################################CHANNEL_NOISE
object {
	obj_name=CHAN_NOISE
	exe_name=CHANNEL_SUI_REPORT
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output
		remote_obj=DAC_JACK0
	}
	inputs {
		name=input_1
		remote_itf=output_0
		remote_obj=REPORT1
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DISTOR_CHANNEL	
	}
	outputs {
		name=output_1
		remote_itf=input_1
		remote_obj=DISTOR_CHANNEL
	}

}
####################################################################CHANNEL_NOISE
####################################################################CIR_BUFFER_RX
object {
	obj_name=CIRC_BUFFER_RX				
	exe_name=CIRC_BUFFER
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DISTOR_CHANNEL
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DDC17
	}
}
####################################################################CIR_BUFFER_RX
#######################################################################DDC17
object {
	obj_name=DDC17
	exe_name=DDC17
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=CIRC_BUFFER_RX
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=UPLINK_SYNCHRO
	}
}
####################################################################DDC17

######################################################################UPLINK_SYNCHRO
object {
	obj_name=UPLINK_SYNCHRO
	exe_name=UPLINK_SYNCHRO
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DDC17
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=FFT
	}
	outputs {
		name=output_3
		remote_itf=input_0
		remote_obj=CHANNEL_ESTIMATOR
	}

}
#####################################################################UPLINK_SYNCHRO
########################################FFT
object {
	obj_name=FFT
	exe_name=FFT_IFFT
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=UPLINK_SYNCHRO
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=UPLINK_DEMAPPING
	}
}
########################################FFT
####################################################################DEMAPPING
object {
	obj_name=UPLINK_DEMAPPING				
	exe_name=UPLINK_DEMAPPING
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=FFT
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=UPLINK_EQUALIZER
	}

}
####################################################################DEMAPPING
####################################################################CHANNEL_ESTIMATOR
object {
	obj_name=CHANNEL_ESTIMATOR				
	exe_name=CHANNEL_ESTIMATOR2
	inputs {
		name=input_0
		remote_itf=output_3
		remote_obj=UPLINK_SYNCHRO
	}
	outputs {
		name=output_0
		remote_itf=input_1
		remote_obj=UPLINK_EQUALIZER
	}

}
####################################################################CHANNEL_ESTIMATOR
####################################################################EQUALIZER
object {
	obj_name=UPLINK_EQUALIZER				
	exe_name=UPLINK_EQUALIZER
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=UPLINK_DEMAPPING
	}
	inputs {
		name=input_1
		remote_itf=output_0
		remote_obj=CHANNEL_ESTIMATOR
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=IDFT
	}

}
####################################################################EQUALIZER
########################################IDFT
object {
	obj_name=IDFT
	exe_name=FFT_IFFT
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=UPLINK_EQUALIZER
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DEMOD_16QAM
	}
}
########################################DFT
####################################################################DEMOD16QAM
object {
	obj_name=DEMOD_16QAM
	exe_name=MOD_QAM
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=IDFT
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DESCRAMBLING
	}
}
####################################################################DEMOD16QAM
################################################DESCRAMBLING
object {
	obj_name=DESCRAMBLING
	exe_name=SCRAMBLING
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DEMOD_16QAM
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=LTETURBORXFLOATS
	}
}
################################################DESCRAMBLING
#######################################################################TURBODECODER
object {
	obj_name=LTETURBORXFLOATS
	exe_name=LTEturboCOD2
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DESCRAMBLING
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=UNCRC
	}
}
#######################################################################TURBODECODER
#######################################################################UNCRC
object {
	obj_name=UNCRC
	exe_name=CRC_REPORT
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=LTETURBORXFLOATS
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DATASOURCESINK 
	}

	inputs {
		name=input_1
		remote_itf=output_1
		remote_obj=DISTOR_CHANNEL
	}
	outputs {
		name=output_1
		remote_itf=input_2
		remote_obj=DATASOURCESINK
	}
}
#######################################################################UNCRC

############################END


