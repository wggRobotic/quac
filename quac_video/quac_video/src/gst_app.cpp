#include "video_app/gst_app.hpp"

#define MUXER_BATCH_TIMEOUT_USEC 33000
#define CUSTOM_PTS 1

typedef struct _AppSrcData
{
  GstElement *app_source = NULL;
  long frame_size = 0;                  /* Pointer to the raw video file */
  gint appsrc_frame_num = 0;
  guint fps = 0;                    /* To set the FPS value */
  guint sourceid = 0;               /* To control the GSource */
  gst_app_interface interface;

  int frame_number = 0;
} AppSrcData;

static GstFlowReturn new_sample (GstElement * sink, AppSrcData * data)
{
  GstSample *sample;
  GstBuffer *buf = NULL;
  NvDsObjectMeta *obj_meta = NULL;
  NvDsMetaList *l_frame = NULL;
  NvDsMetaList *l_obj = NULL;
  unsigned long int pts = 0;

  sample = gst_app_sink_pull_sample (GST_APP_SINK (sink));
  if (gst_app_sink_is_eos (GST_APP_SINK (sink))) {
    g_print ("EOS received in Appsink********\n");
  }

  guint obj_count = 0;

  if (sample) {
    /* Obtain GstBuffer from sample and then extract metadata from it. */
    buf = gst_sample_get_buffer (sample);
    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta (buf);

    for (l_frame = batch_meta->frame_meta_list; l_frame != NULL;
        l_frame = l_frame->next) {
      NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) (l_frame->data);
      pts = frame_meta->buf_pts;
      for (l_obj = frame_meta->obj_meta_list; l_obj != NULL;
          l_obj = l_obj->next) {
        obj_meta = (NvDsObjectMeta *) (l_obj->data);

        data->interface.on_detection(data->interface.user_data);
      }
    }

    data->frame_number++;
    gst_sample_unref (sample);
    return GST_FLOW_OK;
  }
  return GST_FLOW_ERROR;
}

static gboolean bus_call (GstBus * bus, GstMessage * msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop *) data;
  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;
    case GST_MESSAGE_ERROR:{
      gchar *debug = NULL;
      GError *error = NULL;
      gst_message_parse_error (msg, &error, &debug);
      g_printerr ("ERROR from element %s: %s\n",
          GST_OBJECT_NAME (msg->src), error->message);
      if (debug)
        g_printerr ("Error details: %s\n", debug);
      g_free (debug);
      g_error_free (error);
      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }
  return TRUE;
}

static gboolean read_data (AppSrcData * data)
{
  GstBuffer *buffer;
  GstFlowReturn gstret;

  GstMapInfo map =  GST_MAP_INFO_INIT;
  buffer = gst_buffer_new_allocate (NULL, data->frame_size, NULL);

  gst_buffer_map (buffer, &map, GST_MAP_WRITE);

  const void* image_data = data->interface.on_get_frame(data->interface.user_data);
  if (image_data != NULL) memcpy(map.data, image_data, data->frame_size);

  map.size = data->frame_size;

  gst_buffer_unmap (buffer, &map);

  #if CUSTOM_PTS
    GST_BUFFER_PTS (buffer) =
    gst_util_uint64_scale (data->appsrc_frame_num, GST_SECOND, data->fps);
  #endif
  gstret = gst_app_src_push_buffer ((GstAppSrc *) data->app_source, buffer);
  if (gstret != GST_FLOW_OK) {
    g_print ("gst_app_src_push_buffer returned %d \n", gstret);
    return FALSE;
  }
  data->appsrc_frame_num++;

  return TRUE;
}

static void start_feed (GstElement * source, guint size, AppSrcData * data)
{
  if (data->sourceid == 0) {
    data->sourceid = g_idle_add ((GSourceFunc) read_data, data);
  }
}

/* This callback triggers when appsrc has enough data and we can stop sending.
 * We remove the idle handler from the mainloop */
static void stop_feed (GstElement * source, AppSrcData * data)
{
  if (data->sourceid != 0) {
    g_source_remove (data->sourceid);
    data->sourceid = 0;
  }
}

void gst_app_run(gint width, gint height, gint fps, const std::string& config_path, bool multicast, const std::string& ip,  gst_app_interface* interface)
{
  GMainLoop *loop = NULL;
  GstElement *pipeline = NULL, *nvvidconv1 = NULL, *caps_filter = NULL,
      *streammux = NULL, *pgie = NULL, *nvvidconv2 = NULL,
      *nvosd = NULL, *tee = NULL, *appsink = NULL, *nvvidconv3 = NULL, *encoder = NULL, *payloader = NULL, *udp_sink = NULL;
  GstBus *bus = NULL;
  guint bus_watch_id;
  AppSrcData data;
  GstCaps *caps = NULL;
  GstCapsFeatures *feature = NULL;
  GstPad *tee_source_pad1, *tee_source_pad2;
  GstPad *osd_sink_pad, *appsink_sink_pad;

  data.fps = fps;
  data.frame_size = width * height * 4;
  data.interface = *interface;

  gst_init (NULL, NULL);
  loop = g_main_loop_new (NULL, FALSE);

  #define GST_CHECK(cmd) if (!(cmd)) {g_printerr ("'%s' in line %d failed. Exiting.\n", #cmd, __LINE__); }

  GST_CHECK(pipeline = gst_pipeline_new ("dstest-appsrc-pipeline"));
  GST_CHECK(data.app_source = gst_element_factory_make ("appsrc", "app-source"));
  GST_CHECK(nvvidconv1 = gst_element_factory_make ("nvvideoconvert", "nvvideo-converter1"));
  GST_CHECK(caps_filter = gst_element_factory_make ("capsfilter", "capsfilter"));
  GST_CHECK(streammux = gst_element_factory_make ("nvstreammux", "stream-muxer"));
  GST_CHECK(pgie = gst_element_factory_make ( "nvinfer", "primary-nvinference-engine"));
  GST_CHECK(nvvidconv2 = gst_element_factory_make ("nvvideoconvert", "nvvideo-converter2"));
  GST_CHECK(nvosd = gst_element_factory_make ("nvdsosd", "nv-onscreendisplay"));
  GST_CHECK(tee = gst_element_factory_make ("tee", "tee"));
  GST_CHECK(nvvidconv3 = gst_element_factory_make("nvvideoconvert", "nvvideo-converter3"));
  GST_CHECK(encoder = gst_element_factory_make("x264enc", "h264-encoder"));
  GST_CHECK(payloader = gst_element_factory_make("rtph264pay", "rtp-pay"));
  GST_CHECK(udp_sink = gst_element_factory_make("udpsink", "udp-sink"));
  GST_CHECK(appsink = gst_element_factory_make ("appsink", "app-sink"));

  /* Configure appsrc */
  g_object_set (data.app_source, "caps",
      gst_caps_new_simple ("video/x-raw",
          "format", G_TYPE_STRING, "RGBA",
          "width", G_TYPE_INT, width,
          "height", G_TYPE_INT, height,
          "framerate", GST_TYPE_FRACTION, data.fps, 1, NULL), NULL);

  g_signal_connect (data.app_source, "need-data", G_CALLBACK (start_feed), &data);
  g_signal_connect (data.app_source, "enough-data", G_CALLBACK (stop_feed), &data);

  /* For Jetson, with copy-hw=1 and memory-type=nvbuf-mem-surface-array,
     cudaMemcopy fail is observed. This is a WAR till root cause is fixed */
  g_object_set(nvvidconv1, "copy-hw", 2, NULL);

  caps = gst_caps_new_simple ("video/x-raw", "format", G_TYPE_STRING, "RGBA", NULL);
  feature = gst_caps_features_new ("memory:NVMM", NULL);
  gst_caps_set_features (caps, 0, feature);
  g_object_set (G_OBJECT (caps_filter), "caps", caps, NULL);

  /* Set streammux properties */
  g_object_set (G_OBJECT (streammux), "width", width, "height",
    height, "batch-size", 1, "live-source", TRUE,
    "batched-push-timeout", MUXER_BATCH_TIMEOUT_USEC, NULL);


  g_object_set (G_OBJECT (pgie), "config-file-path", config_path.c_str(), NULL);

  /* Configure appsink to extract data from DeepStream pipeline */
  g_object_set (appsink, "emit-signals", TRUE, "async", FALSE, NULL);
  g_signal_connect (appsink, "new-sample", G_CALLBACK (new_sample), &data);

  g_object_set(encoder,
    "tune", 4,  // zerolatency
    "speed-preset", 1,  // ultrafast
    "key-int-max", 30,
     NULL);

  g_object_set(encoder,
    "preset-level", 1,         // 1 = ultrafast
    "iframeinterval", 30,      // keyframe every second
    "bitrate", 2000,           // kbps, tune as needed
    "bufapi-version", TRUE,    // newer buffer API
    "profile", 1,              // baseline profile
    NULL);

  g_object_set(payloader, "config-interval", 1, NULL);

  g_object_set(udp_sink,
             "host", ip.c_str(),
             "port", 5000,
             "sync", FALSE,     // don’t wait for pipeline clock
             "async", FALSE,    // start immediately
             "buffer-size", 2000000,  // optional, reduce buffering
             NULL);

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  gst_bin_add_many (GST_BIN (pipeline), data.app_source, nvvidconv1, caps_filter, streammux, pgie, nvvidconv2, nvosd, tee, nvvidconv3, encoder, payloader, udp_sink, appsink, NULL);

  GstPad *sinkpad, *srcpad;

  GST_CHECK(sinkpad = gst_element_request_pad_simple (streammux, "sink_0"));
  GST_CHECK(srcpad = gst_element_get_static_pad (caps_filter, "src"));

  GST_CHECK(gst_pad_link (srcpad, sinkpad) == GST_PAD_LINK_OK);

  gst_object_unref (sinkpad);
  gst_object_unref (srcpad);

  if (!gst_element_link_many (data.app_source, nvvidconv1, caps_filter, NULL) ||
      !gst_element_link_many(nvosd, nvvidconv3, encoder, payloader, udp_sink, NULL) ||
      !gst_element_link_many (streammux, pgie, nvvidconv2, tee, NULL)) {
    g_printerr ("Elements could not be linked: Exiting.\n");
    return;
  }

/* Manually link the Tee, which has "Request" pads.
 * This tee, in case of multistream usecase, will come before tiler element. */
  tee_source_pad1 = gst_element_request_pad_simple (tee, "src_0");
  osd_sink_pad = gst_element_get_static_pad (nvosd, "sink");
  tee_source_pad2 = gst_element_request_pad_simple (tee, "src_1");
  appsink_sink_pad = gst_element_get_static_pad (appsink, "sink");
  if (gst_pad_link (tee_source_pad1, osd_sink_pad) != GST_PAD_LINK_OK) {
    g_printerr ("Tee could not be linked to display sink.\n");
    gst_object_unref (pipeline);
    return;
  }
  if (gst_pad_link (tee_source_pad2, appsink_sink_pad) != GST_PAD_LINK_OK) {
    g_printerr ("Tee could not be linked to appsink.\n");
    gst_object_unref (pipeline);
    return;
  }
  gst_object_unref (osd_sink_pad);
  gst_object_unref (appsink_sink_pad);

  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  g_print ("Running...\n");
  g_main_loop_run (loop);

  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);
  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);
}