#include <obs-frontend-api.h>
#include <obs-module.h>
#include <QMainWindow>
#include <QAction>
#include <pulse/error.h>
#include <pulse/simple.h>
#include "pavirtualoutput.h"
#include "PAVirtualOutputProps.h"


OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("pavirtualsource", "en-US")

/* create PA context
 * find and destroy/create module-null-sink sink_name=OBSVirtualOutputSink sink_properties='device.description="OBSSink"'
 * find and destroy/create module-virtual-source source_name=OBSVirtualMic master=OBSVirtualOutputSink.monitor
 * create PA stream
 * pa_stream_connect_playback
 * on OBS data available, pa_stream_write
 * overflow/underflow handlers?
 * on close, pa_stream_disconnect
 */

struct pavirtualoutput_data {
    // config data about this instance of the output
    obs_output_t *output = nullptr;
    bool active = false;
    pa_simple *pa_ctx = nullptr;
};

PAVirtualOutputProps* prop;
obs_output_t* pavirtualoutput;

const char *pavirtualoutput_get_name(void *unused)
{
    return "pavirtualoutput";
}

void *pavirtualoutput_create(obs_data_t *settings, obs_output_t *output)
{
    blog(LOG_INFO, "pavirtualoutput create...");
    // create PA context
    // create/find PA source/sink
    // stash in data object

    auto state_data = (pavirtualoutput_data*)bzalloc(sizeof(struct pavirtualoutput_data));
    state_data->output = output;
    return state_data;
}

void pavirtualoutput_destroy(void *data)
{
    blog(LOG_INFO, "pavirtualoutput destroy...");

    auto *state_data = (pavirtualoutput_data*)data;
    if (state_data){
        bfree(state_data);
    }
}

bool pavirtualoutput_start(void *data)
{
    blog(LOG_INFO, "pavirtualoutput starting...");

    pavirtualoutput_data *state_data = (pavirtualoutput_data*)data;
    audio_t *audio = obs_output_audio(state_data->output);

    auto format_info = audio_output_get_info(audio);
    auto sample_rate = audio_output_get_sample_rate(audio);

    // FIXME: create PA stream
    // connect stream for playback
//
//    // this might not work under the debugger
//    if(!obs_output_can_begin_data_capture(state_data->output,OBS_OUTPUT_AUDIO)){
//        // FIXME: signal self fail
//        blog(LOG_WARNING, "pavirtualoutput could not start");
//        return false;
//    }

    struct audio_convert_info conv = {};
    conv.format = AUDIO_FORMAT_16BIT;
    conv.samples_per_sec = 44100;
    conv.speakers = SPEAKERS_STEREO;

    obs_output_set_audio_conversion(state_data->output, &conv);

    struct pa_sample_spec ss = {};
    ss.format = PA_SAMPLE_S16LE;
    ss.rate = conv.samples_per_sec;
    ss.channels = get_audio_channels(conv.speakers);

    int error;
    auto *pa_ctx = pa_simple_new(NULL, "OBS PulseAudio Output Plugin", PA_STREAM_PLAYBACK, "OBSVirtualOutputSink", "playback", &ss, NULL, NULL, &error);
    if(!pa_ctx)
    {
        blog(LOG_ERROR, "couldn't connect to PulseAudio sink: %s", pa_strerror(error));
        return false;
    }
    state_data->pa_ctx = pa_ctx;

    bool started_ok = obs_output_begin_data_capture(state_data->output, 0);

    if(!started_ok)
    {
        blog(LOG_ERROR, "pavirtualoutput could not start");
    }

    state_data->active = started_ok;

    blog(LOG_INFO, "pavirtualoutput started ok");

    return started_ok;
}

void pavirtualoutput_stop(void *data, uint64_t ts)
{
    blog(LOG_INFO, "pavirtualoutput stopping...");

    auto *state_data = (pavirtualoutput_data*)data;

    if(state_data->active)
    {
        state_data->active = false;
        obs_output_end_data_capture(state_data->output);
        // FIXME: disconnect PA stream playback
        pa_simple_free(state_data->pa_ctx);
        state_data->pa_ctx = nullptr;
    }
}

void pavirtualoutput_raw_audio(void *data, struct audio_data *frames)
{
    auto *state_data = (pavirtualoutput_data*)data;
    if(!state_data->active)
        return;

    auto bytect = get_audio_size(AUDIO_FORMAT_16BIT, SPEAKERS_STEREO, frames->frames);

    int error;
    if(0 != pa_simple_write(state_data->pa_ctx, frames->data[0], bytect, &error))
        blog(LOG_ERROR, "bang %s", pa_strerror(error));
}

// no-op; due to (apparently?) a bug in OBS 25.8, raw audio only outputs don't get called
void pavirtualoutput_raw_video(void* data, struct video_data* frame)
{

}

// these are probably not necessary

obs_properties_t* pavirtualoutput_get_properties(void* data)
{
    auto props = obs_properties_create();
    obs_properties_set_flags(props, OBS_PROPERTIES_DEFER_UPDATE);
    obs_properties_add_text(props, "blah", "blah", OBS_TEXT_DEFAULT);

    return props;
}

void pavirtualoutput_get_defaults(obs_data_t* settings)
{
    obs_data_set_default_string(settings, "pavirtualoutput", "blah");
}

void pavirtualoutput_update(void* data, obs_data_t* settings)
{

}

struct obs_output_info create_output_info()
{
    struct obs_output_info output_info = {};
    output_info.id = "pavirtualoutput";
    output_info.flags = OBS_OUTPUT_AV;  // seems like a bug that we can't just do OBS_OUTPUT_AUDIO
    output_info.get_name = pavirtualoutput_get_name;
    output_info.create = pavirtualoutput_create;
    output_info.destroy = pavirtualoutput_destroy;
    output_info.start = pavirtualoutput_start;
    output_info.stop = pavirtualoutput_stop;
    output_info.raw_audio = pavirtualoutput_raw_audio;

    output_info.raw_video = pavirtualoutput_raw_video;
/*
    output_info.update = pavirtualoutput_update;
    output_info.get_properties = pavirtualoutput_get_properties;
    output_info.get_defaults = pavirtualoutput_get_defaults;
*/
    return output_info;
}

bool obs_module_load(void)
{
    obs_output_info output_info = create_output_info();
    obs_register_output(&output_info);

    obs_data_t *settings = obs_data_create();
    pavirtualoutput = obs_output_create("pavirtualoutput", "PulseAudio Virtual Output", settings, NULL);
    obs_data_release(settings);

    QMainWindow* main_window = (QMainWindow*)obs_frontend_get_main_window();
    QAction *action = (QAction*)obs_frontend_add_tools_menu_qaction("PulseAudio Virtual Output");

    prop = new PAVirtualOutputProps(main_window);

    auto menu_cb = []
    {
        prop->setVisible(!prop->isVisible());
    };

    action->connect(action, &QAction::triggered, menu_cb);

    return true;
}

void obs_module_unload()
{
}

void pavirtualoutput_enable()
{
    bool started = obs_output_start(pavirtualoutput);
}

void pavirtualoutput_disable()
{
    obs_output_stop(pavirtualoutput);
}