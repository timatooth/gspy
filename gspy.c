#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <gst/gst.h>
#include "audiosamples.h"

#define GSPY_VERSION_MAJOR 1
#define GSPY_VERSION_MINOR 0
#define GSPY_VERSION_MICRO 0

/* Error handler */
static gboolean on_bus_error(GstBus *bus, GstMessage *msg, gpointer data){
  gchar *debug;
  GError *error;

  gst_message_parse_error(msg, &error, &debug);
  g_free(debug);
  g_printerr("Error: %s\n", error->message);
  g_error_free(error);

  return TRUE;
}

static gboolean on_sample_timer(gpointer data){
  AudioSamples as = (AudioSamples) data;
  guint count = audiosamples_size(as);
  g_print("Timer called %d times.\n", count);
  /* Only keep collecting samples to get the average for x times */
  if(count < 10){
    audiosamples_add(as, count++);
    return TRUE;
  }
  
  g_print("Finished sampling output follows...\n");
  audiosamples_print(as);
  g_print("Sample standard deviation is: %.2f\n", 
	  audiosamples_sd(as));
  g_print("Sample average is: %.2f\n", audiosamples_average(as));
  return FALSE;
}

static gboolean on_level_message(GstBus *bus, GstMessage *msg, gpointer data){
  g_print("message!\n");
  if(msg->type == GST_MESSAGE_ELEMENT){
    const GstStructure *s = gst_message_get_structure(msg);
    const gchar *name = gst_structure_get_name(s);
    if(0 == strcmp(name, "level")){
      g_print("Got level message on bus\n");
    }
  }
  return TRUE;
}

int main(int argc, char *argv[]){
  GMainLoop *loop;
  /* GStreamer version information */
  const gchar *nano_str;
  guint major, minor, micro, nano;
  /* Bus information for error detection and messages */
  GstBus *bus;
  /* Elements in the pipeline */
  GstElement *pipeline, *audiosrc, *level, *oggmux, *vorbisenc, *filesink;
  /* Configuration */
  GKeyFile *config;
  /* Store audio sampling history */
  AudioSamples audioSamples;
  guint as_bus_watch_id;

  gst_init(&argc, &argv);
  gst_version(&major, &minor, &micro, &nano);
  if(nano == 1)
    nano_str = "(CVS)";
  else if (nano == 2)
    nano_str = "(Pre-release)";
  else 
    nano_str = "";
  printf("gSpy version: %d.%d.%d compiled against GStreamer: %d.%d.%d\n",
	 GSPY_VERSION_MAJOR, GSPY_VERSION_MINOR, GSPY_VERSION_MICRO,
	 GST_VERSION_MAJOR, GST_VERSION_MINOR, GST_VERSION_MICRO);
  printf("gSpy is currently linked against GStreamer: %d.%d.%d %s\n",
	 major, minor, micro, nano_str);


  /* Setup / Load configuration file */
  config = g_key_file_new();
  g_key_file_set_integer(config, "audio-detection", "sample-rate", 1000);
  g_key_file_set_integer(config, "audio-detection", "sample-count", 10);
  g_key_file_set_double(config, "audio-detection", "record-peak", 1);
  g_key_file_set_string(config,
			"saving",
			"save-dir",
			"/home/tim/src/gspy/output/out.ogg");
  gchar *file_data = g_key_file_to_data(config, NULL, NULL);
  printf("Generated default config data:\n%s\n", file_data);
  g_free(file_data);
  printf("User configuration dir is: \n %s\n", g_get_user_config_dir());

  audioSamples = 
    audiosamples_new(g_key_file_get_integer(
					    config,
					    "audio-detection",
					    "sample-count",
					    NULL));
  
  /* Initilise GST Elements 
   */
  loop = g_main_loop_new(NULL, FALSE);
  pipeline = gst_pipeline_new("gspy-pipeline");
  bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
  gst_bus_add_signal_watch(bus);
  g_signal_connect(bus, "message::error", 
			   G_CALLBACK(on_bus_error),
			   loop);
  as_bus_watch_id = gst_bus_add_watch(bus, on_level_message, NULL);

  gst_object_unref(bus);

  audiosrc    = gst_element_factory_make("autoaudiosrc",      "audio-source");
  level       = gst_element_factory_make("level",             "audio-level");
  oggmux      = gst_element_factory_make("oggmux",            "ogg-mux");
  vorbisenc   = gst_element_factory_make("vorbisenc",         "vorbis-enc");
  filesink    = gst_element_factory_make("filesink",          "file-sink");
  if(!audiosrc){ g_printerr("Er. Can't make autoaudiosrc\n"); return -1; }
  else if(!level){ g_printerr("Er. Cant make level\n"); return -1; }
  else if(!oggmux){ g_printerr("Er. Cant make oggmux\n"); return -1; }
  else if(!vorbisenc){ g_printerr("Er. Cant make vorbisenc\n"); return -1; }
  else if(!filesink){ g_printerr("Er. Cant make filesink\n"); return -1; }

  /* Settings for pipeline */
  g_object_set(G_OBJECT(filesink),
	       "location", 
	       g_key_file_get_string(config, "saving", "save-dir", NULL), NULL);

  g_object_set (G_OBJECT (level), "post-messages", TRUE, NULL);

  gst_bin_add_many(GST_BIN(pipeline),
		   audiosrc, level, oggmux, vorbisenc, filesink, NULL);

  gst_element_link_many(audiosrc, level, vorbisenc, oggmux, filesink, NULL);

  /* Set up timer for creating initial sound samples */
  /*g_timeout_add(
		g_key_file_get_integer(config,
				       "audio-detection",
				       "sample-rate",
				       NULL),
		on_sample_timer,
		audioSamples); */
  

  g_print("Enabling pipeline...\n");
  gst_element_set_state(pipeline, GST_STATE_PLAYING);


  /*Fire up the engine vroooom... */
  g_main_loop_run(loop);


  /* Program has exited loop, clean up. */
  g_print("Loop terminated\n");
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(pipeline);
  g_main_loop_unref(loop);
  g_key_file_free(config);

  return 0;
}
