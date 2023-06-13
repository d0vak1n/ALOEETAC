########################################DATA_SOURCE_SINK
object {
	obj_name=DATASOURCESINK
	exe_name=DATASOURCESINK_REPORTP21
	
	inputs {
		name=input_1
		remote_itf=output_1
		remote_obj=UPLINK_MAPPING
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=CRC
	}
}
########################################DATA_SOURCE_SINK
########################################CRC
object {
	obj_name=CRC
	exe_name=CRC
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DATASOURCESINK
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=LTETURBOTX
	}
}
########################################CRC
########################################TURBOCODER_TX
object {
	obj_name=LTETURBOTX
	exe_name=LTEturboCOD2
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=CRC
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=SCRAMBLING
	}
}
########################################TURBOCODER_TX
########################################SCRAMBLING
object {
	obj_name=SCRAMBLING
	exe_name=SCRAMBLING
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=LTETURBOTX
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=MOD_16QAM
	}
}
########################################SCRAMBLING
########################################MODULATOR
object {
	obj_name=MOD_16QAM
	exe_name=MOD_QAM
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=SCRAMBLING
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DFT
	}
}
########################################MODULATOR
########################################DFT
object {
	obj_name=DFT
	exe_name=FFT_IFFT
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=MOD_16QAM
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=UPLINK_MAPPING
	}
}
########################################DFT
########################################UPLINK_MAPPING
object {
	obj_name=UPLINK_MAPPING				
	exe_name=UPLINK_MAPPING
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DFT
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=IFFT
	outputs {
		name=output_1
		remote_itf=input_1
		remote_obj=DATASOURCESINK
	}
}
####################################################################UPLINK_MAPPING
########################################IFFT
object {
	obj_name=IFFT
	exe_name=FFT_IFFT
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=UPLINK_MAPPING
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DUC17
	}
}
########################################IFFT
#####################################################DUC17
object {
	obj_name=DUC17
	exe_name=DUC17
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=IFFT
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=CIRC_BUFFER_TX
	}
}
####################################################################DUC
####################################################################CIR_BUFFER_TX
object {
	obj_name=CIRC_BUFFER_TX
	exe_name=CIRC_BUFFER
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DUC17
	}
	outputs {
		name=output_0
		remote_itf=input
		remote_obj=DAC_JACK0
	}
}
####################################################################CIR_BUFFER_TX
#########################################################################DAC_JACK0
object {
	obj_name=DAC_JACK0
	exe_name=DAC_JACK0
#	force_pe=0
	kopts=15
	inputs {
		name=input
		remote_itf=output_0
		remote_obj=CIRC_BUFFER_TX
	}
}
#########################################################################DAC_JACK0


