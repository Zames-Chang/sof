# SPDX-License-Identifier: BSD-3-Clause

# Array of "input-file-name;output-file-name;comma separated pre-processor variables"
set(TPLGS
# CAVS HDMI only topology with passthrough pipelines
"sof-hda-generic\;sof-hda-generic-idisp\;USE_CHAIN_DMA=true,DEEPBUFFER_FW_DMA_MS=100"
# CAVS HDA topology with mixer-based pipelines for HDA and passthrough pipelines for HDMI
"sof-hda-generic\;sof-hda-generic\;HDA_CONFIG=mix,USE_CHAIN_DMA=true,DEEPBUFFER_FW_DMA_MS=100"
# If the alsatplg plugins for NHLT are not available, the NHLT blobs will not be added to the
# topologies below.
"sof-hda-generic\;sof-hda-generic-2ch\;\
HDA_CONFIG=mix,NUM_DMICS=2,PREPROCESS_PLUGINS=nhlt,NHLT_BIN=nhlt-sof-hda-generic-2ch.bin,USE_CHAIN_DMA=true,\
DEEPBUFFER_FW_DMA_MS=100"
"sof-hda-generic\;sof-hda-generic-4ch\;\
HDA_CONFIG=mix,NUM_DMICS=4,PDM1_MIC_A_ENABLE=1,PDM1_MIC_B_ENABLE=1,\
PREPROCESS_PLUGINS=nhlt,NHLT_BIN=nhlt-sof-hda-generic-4ch.bin,USE_CHAIN_DMA=true,DEEPBUFFER_FW_DMA_MS=100"
# CAVS SDW topology with passthrough pipelines
"cavs-sdw\;cavs-sdw\;DEEPBUFFER_FW_DMA_MS=100"

# IPC4 topology for TGL rt711 Headset + rt1316 Amplifier + rt714 DMIC
"cavs-sdw\;sof-tgl-rt711-rt1316-rt714\;NUM_SDW_AMPS=2,SDW_DMIC=1"

# IPC4 topology for TGL rt711 Headset + rt1308 Amplifier + rt715 DMIC
"cavs-sdw\;sof-tgl-rt715-rt711-rt1308-mono\;NUM_SDW_AMPS=1,SDW_DMIC=1,\
SDW_JACK_OUT_STREAM=SDW1-Playback,SDW_JACK_IN_STREAM=SDW1-Capture,\
SDW_SPK_STREAM=SDW2-Playback,SDW_DMIC_STREAM=SDW0-Capture"

# IPC4 topology for TGL rt711 Headset + rt1308 Amplifier + PCH DMIC
"cavs-sdw\;sof-tgl-rt711-rt1308-4ch\;NUM_SDW_AMPS=1,NUM_DMICS=4,DMIC0_ID=3,\
DMIC1_ID=4,PDM1_MIC_A_ENABLE=1,PDM1_MIC_B_ENABLE=1"

# CAVS SDW with SRC gain and mixer support
"cavs-sdw-src-gain-mixin\;cavs-sdw-src-gain-mixin"

# SDW + HDMI topology with passthrough pipelines
"cavs-sdw\;cavs-sdw-hdmi\;NUM_HDMIS=4,DEEPBUFFER_FW_DMA_MS=100"

# CAVS SSP topology for TGL
"cavs-nocodec\;sof-tgl-nocodec\;NUM_DMICS=4,PDM1_MIC_A_ENABLE=1,PDM1_MIC_B_ENABLE=1,\
PREPROCESS_PLUGINS=nhlt,NHLT_BIN=nhlt-sof-tgl-nocodec.bin,DEEPBUFFER_FW_DMA_MS=100"
"cavs-nocodec\;sof-adl-nocodec\;NUM_DMICS=4,PDM1_MIC_A_ENABLE=1,PDM1_MIC_B_ENABLE=1,\
PREPROCESS_PLUGINS=nhlt,NHLT_BIN=nhlt-sof-adl-nocodec.bin,DEEPBUFFER_FW_DMA_MS=100"
)

add_custom_target(topology2_cavs)

foreach(tplg ${TPLGS})
	set(defines "")
	list(LENGTH tplg length)
	list(GET tplg 0 input)
	list(GET tplg 1 output)

	math(EXPR last_index "${length}-1")

	# Get the pre-processor definitions from the 3rd item in the list for each topology
	# ex: "hda-generic\;hda-generic-2ch\;HDA_CONFIG=gain,DMIC_CH=2", "defines" would contain "HDA_CONFIG=gain,DMIC_CH=2"
	if (${last_index} EQUAL 2)
		list(GET tplg ${last_index} defines)
	endif()

	add_alsatplg2_command("${CMAKE_CURRENT_BINARY_DIR}/../abi.conf" abi_target
	  "${CMAKE_CURRENT_SOURCE_DIR}/../${input}" "${output}"
	  "${CMAKE_CURRENT_SOURCE_DIR}/../" "${defines}")

	add_custom_target(topology2_avs_${output} DEPENDS ${output}.tplg)
	add_dependencies(topology2_cavs topology2_avs_${output})
endforeach()
