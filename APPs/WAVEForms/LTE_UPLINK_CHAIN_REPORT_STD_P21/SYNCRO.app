
####################################################################GRAPH_SYNCHRO
object {
	obj_name=GRAPH_SYNCHRO
	exe_name=GRAPH
	force_pe=0
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=UPLINK_SYNCHRO
	}
}
####################################################################GRAPH_SYNCHRO

######################################################################UPLINK_SYNCHRO
object {
	obj_name=UPLINK_SYNCHRO
	exe_name=UPLINK_SYNCHRO
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=GRAPH_SYNCHRO
	}
	outputs {
		name=output_1
		remote_itf=input_0
		remote_obj=GRAPH_OUT1
	}

	
}
#####################################################################UPLINK_SYNCHRO

####################################################################GRAPH_OUT1
object {
	obj_name=GRAPH_OUT1
	exe_name=GRAPH		
	inputs {
		name=input_0
		remote_itf=output_1
		remote_obj=UPLINK_SYNCHRO
	}
}
######################################################################GRAPH_OUT1

############################END


