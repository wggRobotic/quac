/*
 * SPDX-FileCopyrightText: Copyright (c) 2022-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <glib.h>
#include <rclcpp/node.hpp>
#include <stdio.h>
#include <string.h>
#include "gstnvdsmeta.h"
#include <librealsense2/rs.hpp>

#include <rclcpp/rclcpp.hpp>


#include "ament_index_cpp/get_package_share_directory.hpp"

#define MAX_DISPLAY_LEN 64

gchar* class_names[] = {
  "blasting_agents",
  "corrosive",
  "dangerous_when_wet",
  "explosives",
  "flammable_gas",
  "flammable_solid",
  "fuel_oil",
  "inhalation_hazard",
  "non_flammable_gas",
  "organic_peroxide",
  "oxidizer",
  "oxygen",
  "poison",
  "radioactive",
  "spontaneously_combustible",
};

#define CLASS_COUNT sizeof(class_names)/sizeof(class_names[0])

#define CUSTOM_PTS 1

#define NVINFER_PLUGIN "nvinfer"
#define PGIE_CONFIG_FILE  "src/quac/quac_video/infer/config/hazmat_config.txt"

/* Muxer batch formation timeout, for e.g. 40 millisec. Should ideally be set
 * based on the fastest source's framerate. */
#define MUXER_BATCH_TIMEOUT_USEC 33000

gint frame_number = 0;

/* Structure to contain all our information for appsrc,
 * so we can pass it to callbacks */
typedef struct _AppSrcData
{
  GstElement *app_source = NULL;
  long frame_size = 0;                  /* Pointer to the raw video file */
  gint appsrc_frame_num = 0;
  guint fps = 0;                    /* To set the FPS value */
  guint sourceid = 0;               /* To control the GSource */
  rs2::pipeline pipe;
  rs2::config cfg;

  //rclcpp::Node node;
  //rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr point_cloud_publisher;
} AppSrcData;

/* new_sample is an appsink callback that will extract metadata received
 * tee sink pad and update params for drawing rectangle,
 *object information etc. */

guint last_class_counts[CLASS_COUNT] = {0};

static GstFlowReturn
new_sample (GstElement * sink, gpointer * data)
{
  GstSample *sample;
  GstBuffer *buf = NULL;
  NvDsObjectMeta *obj_meta = NULL;
  NvDsMetaList *l_frame = NULL;
  NvDsMetaList *l_obj = NULL;
  unsigned long int pts = 0;

  guint class_counts[CLASS_COUNT] = {0};

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
        if (obj_meta->class_id >= 0 && obj_meta->class_id < CLASS_COUNT) { class_counts[obj_meta->class_id]++; obj_count++;}
      }
    }

    if (memcmp(last_class_counts, class_counts, sizeof(class_counts)) != 0)
    {
      g_print("Frame Number = %d PTS = %" GST_TIME_FORMAT "   Found %d objects:  ", frame_number, GST_TIME_ARGS (pts), obj_count);
      for (int i = 0; i < CLASS_COUNT; i++) if (class_counts[i] > 0) g_print("%s %d   ", class_names[i], class_counts[i]); 
      g_print("\n");
    }
    

    memcpy(last_class_counts, class_counts, sizeof(class_counts));

    frame_number++;
    gst_sample_unref (sample);
    return GST_FLOW_OK;
  }
  return GST_FLOW_ERROR;
}

static gboolean
bus_call (GstBus * bus, GstMessage * msg, gpointer data)
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

/* This method is called by the idle GSource in the mainloop, 
 * to feed one raw video frame into appsrc.
 * The idle handler is added to the mainloop when appsrc requests us
 * to start sending data (need-data signal)
 * and is removed when appsrc has enough data (enough-data signal).
 */
static gboolean
read_data (AppSrcData * data)
{
  GstBuffer *buffer;
  GstFlowReturn gstret;

  size_t ret = 1;
  GstMapInfo map =  GST_MAP_INFO_INIT;
  buffer = gst_buffer_new_allocate (NULL, data->frame_size, NULL);

  gst_buffer_map (buffer, &map, GST_MAP_WRITE);
  rs2::frameset frames = data->pipe.wait_for_frames();
  rs2::video_frame color_frame = frames.get_color_frame();
  memcpy(map.data, color_frame.get_data(), data->frame_size);
  //ret = fread (map.data, 1, data->frame_size, data->file);
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

/* This signal callback triggers when appsrc needs data. Here,
 * we add an idle handler to the mainloop to start pushing
 * data into the appsrc */
static void
start_feed (GstElement * source, guint size, AppSrcData * data)
{
  if (data->sourceid == 0) {
    data->sourceid = g_idle_add ((GSourceFunc) read_data, data);
  }
}

/* This callback triggers when appsrc has enough data and we can stop sending.
 * We remove the idle handler from the mainloop */
static void
stop_feed (GstElement * source, AppSrcData * data)
{
  if (data->sourceid != 0) {
    g_source_remove (data->sourceid);
    data->sourceid = 0;
  }
}

int main (int argc, char *argv[])
{
  rclcpp::init(argc, argv);

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

  //data.node = rclcpp::Node("camera_app");

  long width = 640, height = 480;
  data.fps = 30;
  data.frame_size = width * height * 4;

  /* Standard GStreamer initialization */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  #define GST_CHECK(cmd) if (!(cmd)) {g_printerr ("'%s' in line %d failed. Exiting.\n", #cmd, __LINE__); return -1;}

  GST_CHECK(pipeline = gst_pipeline_new ("dstest-appsrc-pipeline"));
  GST_CHECK(data.app_source = gst_element_factory_make ("appsrc", "app-source"));
  GST_CHECK(nvvidconv1 = gst_element_factory_make ("nvvideoconvert", "nvvideo-converter1"));
  GST_CHECK(caps_filter = gst_element_factory_make ("capsfilter", "capsfilter"));
  GST_CHECK(streammux = gst_element_factory_make ("nvstreammux", "stream-muxer"));
  GST_CHECK(pgie = gst_element_factory_make ( NVINFER_PLUGIN, "primary-nvinference-engine"));
  GST_CHECK(nvvidconv2 = gst_element_factory_make ("nvvideoconvert", "nvvideo-converter2"));
  GST_CHECK(nvosd = gst_element_factory_make ("nvdsosd", "nv-onscreendisplay"));
  GST_CHECK(tee = gst_element_factory_make ("tee", "tee"));

  /* After nvosd */
  GST_CHECK(nvvidconv3 = gst_element_factory_make("nvvideoconvert", "nvvideo-converter3"));
  GST_CHECK(encoder = gst_element_factory_make("x264enc", "h264-encoder"));
  g_object_set(encoder,
             "tune", 4,             // zerolatency
             "speed-preset", 1,     // ultrafast
             "key-int-max", 30,
             NULL);
  g_object_set(encoder,
             "preset-level", 1,         // 1 = ultrafast
             "iframeinterval", 30,      // keyframe every second
             "bitrate", 2000,           // kbps, tune as needed
             "bufapi-version", TRUE,    // newer buffer API
             "profile", 1,              // baseline profile
             NULL);

  GST_CHECK(payloader = gst_element_factory_make("rtph264pay", "rtp-pay"));
  g_object_set(payloader, "config-interval", 1, NULL);

  GST_CHECK(udp_sink = gst_element_factory_make("udpsink", "udp-sink"));
  g_object_set(udp_sink,
             "host", "192.168.137.26",
             "port", 5000,
             "sync", FALSE,     // don’t wait for pipeline clock
             "async", FALSE,    // start immediately
             "buffer-size", 2000000,  // optional, reduce buffering
             NULL);


    /* For Jetson, with copy-hw=1 and memory-type=nvbuf-mem-surface-array,
     cudaMemcopy fail is observed. This is a WAR till root cause is fixed */
  g_object_set(nvvidconv1, "copy-hw", 2, NULL);

  GST_CHECK(appsink = gst_element_factory_make ("appsink", "app-sink"));

  /* Configure appsrc */
  g_object_set (data.app_source, "caps",
      gst_caps_new_simple ("video/x-raw",
          "format", G_TYPE_STRING, "RGBA",
          "width", G_TYPE_INT, width,
          "height", G_TYPE_INT, height,
          "framerate", GST_TYPE_FRACTION, data.fps, 1, NULL), NULL);
#if !CUSTOM_PTS
  g_object_set (G_OBJECT (data.app_source), "do-timestamp", TRUE, NULL);
#endif
  g_signal_connect (data.app_source, "need-data", G_CALLBACK (start_feed), &data);
  g_signal_connect (data.app_source, "enough-data", G_CALLBACK (stop_feed), &data);

  caps = gst_caps_new_simple ("video/x-raw", "format", G_TYPE_STRING, "RGBA", NULL);
  feature = gst_caps_features_new ("memory:NVMM", NULL);
  gst_caps_set_features (caps, 0, feature);
  g_object_set (G_OBJECT (caps_filter), "caps", caps, NULL);

  /* Set streammux properties */
  g_object_set (G_OBJECT (streammux), "width", width, "height",
    height, "batch-size", 1, "live-source", TRUE,
    "batched-push-timeout", MUXER_BATCH_TIMEOUT_USEC, NULL);

  /* Set all the necessary properties of the nvinfer element,
   * the necessary ones are : */

  std::string pkg_share_dir = ament_index_cpp::get_package_share_directory("quac_video");
  std::string model_path = pkg_share_dir + "/config/hazmat_config.txt";

  g_object_set (G_OBJECT (pgie), "config-file-path", model_path.c_str(), NULL);

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* Set up the pipeline */
  /* we add all elements into the pipeline */
  gst_bin_add_many (GST_BIN (pipeline), data.app_source, nvvidconv1, caps_filter, streammux, pgie, nvvidconv2, nvosd, tee, nvvidconv3, encoder, payloader, udp_sink, appsink, NULL);

  GstPad *sinkpad, *srcpad;

  GST_CHECK(sinkpad = gst_element_request_pad_simple (streammux, "sink_0"));
  GST_CHECK(srcpad = gst_element_get_static_pad (caps_filter, "src"));

  GST_CHECK(gst_pad_link (srcpad, sinkpad) == GST_PAD_LINK_OK);

  gst_object_unref (sinkpad);
  gst_object_unref (srcpad);

  /* we link the elements together */
  /* app-source -> nvvidconv -> caps filter ->
   * pgie -> nvvidconv -> nvosd -> video-renderer */

  if (!gst_element_link_many (data.app_source, nvvidconv1, caps_filter, NULL) ||
      !gst_element_link_many(nvosd, nvvidconv3, encoder, payloader, udp_sink, NULL) ||
      !gst_element_link_many (streammux, pgie, nvvidconv2, tee, NULL)) {
    g_printerr ("Elements could not be linked: Exiting.\n");
    return -1;
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
    return -1;
  }
  if (gst_pad_link (tee_source_pad2, appsink_sink_pad) != GST_PAD_LINK_OK) {
    g_printerr ("Tee could not be linked to appsink.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  gst_object_unref (osd_sink_pad);
  gst_object_unref (appsink_sink_pad);

  /* Configure appsink to extract data from DeepStream pipeline */
  g_object_set (appsink, "emit-signals", TRUE, "async", FALSE, NULL);

  /* Callback to access buffer and object info. */
  g_signal_connect (appsink, "new-sample", G_CALLBACK (new_sample), NULL);

  /* Set the pipeline to "playing" state */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  data.cfg = rs2::config();
  data.pipe = rs2::pipeline();
  data.cfg.enable_stream(RS2_STREAM_COLOR, width, height, RS2_FORMAT_RGBA8, data.fps);
  data.pipe.start(data.cfg);

  /* Wait till pipeline encounters an error or EOS */
  g_print ("Running...\n");
  g_main_loop_run (loop);

  data.pipe.stop();

  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);
  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);

  rclcpp::shutdown();
  return 0;
}
