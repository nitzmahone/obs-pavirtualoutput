#include <obs-frontend-api.h>
#include <obs-module.h>
#include <QMainWindow>
#include <QAction>
#include "pavirtualoutput.h"
#include "PAVirtualOutputProps.h"


OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("pavirtualsource", "en-US")

/* create PA context
 * find and destroy/create module-null-sink sink_name=OBSVirtualOutputSink sink_properties='device.description="OBSSink"'
 * find and destroy/create module-virtual-source source_name=OBSVirtualMicrophone master=OBSVirtualOutputSink.monitor
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

    auto sample_rate = audio_output_get_sample_rate(audio);
    auto channels = audio_output_get_channels(audio);

    // FIXME: create PA stream
    // connect stream for playback
//
//    // this might not work under the debugger
//    if(!obs_output_can_begin_data_capture(state_data->output,OBS_OUTPUT_AUDIO)){
//        // FIXME: signal self fail
//        blog(LOG_WARNING, "pavirtualoutput could not start");
//        return false;
//    }

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
    }
}

void pavirtualoutput_raw_audio(void *data, struct audio_data *frames)
{
    blog(LOG_INFO, "pavirtualoutput got %d frames", frames->frames);
}

// these are probably not necessary

void pavirtualoutput_raw_video(void* data, struct video_data* frame)
{
    //blog(LOG_INFO, "pavirtualoutput VIDEO got stuff");
}

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
    output_info.flags = OBS_OUTPUT_AV;
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
    QAction *action = (QAction*)obs_frontend_add_tools_menu_qaction("Pulse Audio Virtual Output");

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