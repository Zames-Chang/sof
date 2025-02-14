// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2021 Google LLC.
//
// Author: Lionel Koenig <lionelk@google.com>
#include "google_rtc_audio_processing.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "sof/lib/alloc.h"
#include "ipc/topology.h"

#define GOOGLE_RTC_AUDIO_PROCESSING_FREQENCY_TO_PERIOD_FRAMES 100
#define GOOGLE_RTC_AUDIO_PROCESSING_MS_PER_SECOND 1000

struct GoogleRtcAudioProcessingState {
	int num_capture_channels;
	int num_aec_reference_channels;
	int num_output_channels;
	int num_frames;
	int16_t *aec_reference;
};

static void SetFormats(GoogleRtcAudioProcessingState *const state,
		       capture_sample_rate_hz,
		       int num_capture_input_channels,
		       int num_capture_output_channels,
		       int render_sample_rate_hz,
		       int num_render_channels)
{
	state->num_capture_channels = num_capture_input_channels;
	state->num_output_channels = num_capture_output_channels;
	state->num_frames = capture_sample_rate_hz /
		GOOGLE_RTC_AUDIO_PROCESSING_FREQENCY_TO_PERIOD_FRAMES;

	state->num_aec_reference_channels = num_render_channels;
	rfree(state->aec_reference);
	state->aec_reference = rballoc(0,
				       SOF_MEM_CAPS_RAM,
				       sizeof(state->aec_reference[0]) *
				       state->num_frames *
				       state->num_aec_reference_channels);
}

void GoogleRtcAudioProcessingAttachMemoryBuffer(uint8_t *const buffer,
						int buffer_size)
{
}

void GoogleRtcAudioProcessingDetachMemoryBuffer(void)
{
}

GoogleRtcAudioProcessingState *GoogleRtcAudioProcessingCreateWithConfig(int capture_sample_rate_hz,
									int num_capture_input_channels,
									int num_capture_output_channels,
									int render_sample_rate_hz,
									int num_render_channels,
									const uint8_t *const config,
									int config_size)
{
	struct GoogleRtcAudioProcessingState *s =
		rballoc(0, SOF_MEM_CAPS_RAM, sizeof(GoogleRtcAudioProcessingState));
	if (!s)
		return NULL;

	s->aec_reference = NULL;
	SetFormats(state,
		   capture_sample_rate_hz,
		   num_capture_input_channels,
		   num_capture_output_channels,
		   render_sample_rate_hz,
		   num_render_channels);

	if (!s->aec_reference) {
		rfree(s);
		return NULL;
	}
	return s;
}

GoogleRtcAudioProcessingState *GoogleRtcAudioProcessingCreate(void)
{
	return GoogleRtcAudioProcessingCreateWithConfig(CONFIG_COMP_GOOGLE_RTC_AUDIO_PROCESSING_SAMPLE_RATE_HZ,
							1,
							1,
							CONFIG_COMP_GOOGLE_RTC_AUDIO_PROCESSING_SAMPLE_RATE_HZ,
							2,
							NULL,
							0);
}

void GoogleRtcAudioProcessingFree(GoogleRtcAudioProcessingState *state)
{
	if (state != NULL) {
		rfree(state->aec_reference);
		rfree(state);
	}
}

int GoogleRtcAudioProcessingSetStreamFormats(GoogleRtcAudioProcessingState *const state,
					     int capture_sample_rate_hz,
					     int num_capture_input_channels,
					     int num_capture_output_channels,
					     int render_sample_rate_hz,
					     int num_render_channels)
{
	SetFormats(state,
		   capture_sample_rate_hz,
		   num_capture_input_channels,
		   num_capture_output_channels,
		   render_sample_rate_hz,
		   num_render_channels);
	return 0;
}

int GoogleRtcAudioProcessingParameters(GoogleRtcAudioProcessingState *const state,
				       float *capture_headroom_linear,
				       float *echo_path_delay_ms)
{
	return 0;
}

int GoogleRtcAudioProcessingGetFramesizeInMs(GoogleRtcAudioProcessingState *state)
{
	return state->num_frames *
		GOOGLE_RTC_AUDIO_PROCESSING_MS_PER_SECOND /
		CONFIG_COMP_GOOGLE_RTC_AUDIO_PROCESSING_SAMPLE_RATE_HZ;
}

int GoogleRtcAudioProcessingReconfigure(GoogleRtcAudioProcessingState *const state,
					const uint8_t *const config,
					int config_size)
{
	return 0;
}

int GoogleRtcAudioProcessingProcessCapture_int16(GoogleRtcAudioProcessingState *const state,
						 const int16_t *const src,
						 int16_t *const dest)
{
	int16_t *ref = state->aec_reference;
	int16_t *mic = (int16_t *) src;
	int16_t *out = dest;
	int n;

	memset(dest, 0, sizeof(int16_t) * state->num_output_channels * state->num_frames);
	for (n = 0; n < state->num_frames; ++n) {
		*out = *mic + *ref;
		ref += state->num_aec_reference_channels;
		out += state->num_output_channels;
		mic += state->num_capture_channels;
	}
	return 0;
}

int GoogleRtcAudioProcessingAnalyzeRender_int16(GoogleRtcAudioProcessingState *const state,
						const int16_t *const data)
{
	const size_t buffer_size =
		sizeof(state->aec_reference[0])
		* state->num_frames
		* state->num_aec_reference_channels;
	memcpy_s(state->aec_reference, buffer_size,
		 data, buffer_size);
	return 0;
}

void GoogleRtcAudioProcessingParseSofConfigMessage(uint8_t *message,
						   size_t message_size,
						   uint8_t **google_rtc_audio_processing_config,
						   size_t *google_rtc_audio_processing_config_size,
						   int *num_capture_input_channels,
						   int *num_capture_output_channels,
						   float *aec_reference_delay,
						   float *mic_gain,
						   bool *google_rtc_audio_processing_config_present,
						   bool *num_capture_input_channels_present,
						   bool *num_capture_output_channels_present,
						   bool *aec_reference_delay_present,
						   bool *mic_gain_present)
{
	*google_rtc_audio_processing_config = NULL;
	*google_rtc_audio_processing_config_size = 0;
	*num_capture_input_channels = 1;
	*num_capture_output_channels = 1;
	*aec_reference_delay = 0;
	*mic_gain = 1;
	*google_rtc_audio_processing_config_present = false;
	*num_capture_input_channels_present = false;
	*num_capture_output_channels_present = false;
	*aec_reference_delay_present = false;
	*mic_gain_present = false;
}
