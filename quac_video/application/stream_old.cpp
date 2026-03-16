#include <librealsense2/rs.hpp>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <csignal>
#include <cstring>
#include <cstdlib>

std::atomic<bool> keep_running{true};

void handle_signal(int signum) {
    keep_running.store(false);
    std::cout << "[Realsense] Received signal " << signum << ". Stopping..." << std::endl;
}

GstElement *pipeline_color = nullptr, *appsrc_color = nullptr;
GstElement *pipeline_depth = nullptr, *appsrc_depth = nullptr;

void push_frame(rs2::frameset &frames, bool is_color, int width, int height, int fps) {
    int frame_size = (is_color ? 3 : 2) * width * height;
    GstElement *appsrc = is_color ? appsrc_color : appsrc_depth;

    if (!appsrc) return;

    // Allocate GstBuffer
    GstBuffer *buffer = gst_buffer_new_allocate(nullptr, frame_size, nullptr);
    if (!buffer) {
        std::cerr << "[Realsense] Failed to allocate GstBuffer" << std::endl;
        return;
    }

    GstMapInfo map;
    if (!gst_buffer_map(buffer, &map, GST_MAP_WRITE)) {
        std::cerr << "[Realsense] Failed to map GstBuffer" << std::endl;
        gst_buffer_unref(buffer);
        return;
    }

    if (is_color) {
        rs2::video_frame color_frame = frames.get_color_frame();
        if (!color_frame) {
            gst_buffer_unmap(buffer, &map);
            gst_buffer_unref(buffer);
            return;
        }
        memcpy(map.data, color_frame.get_data(), frame_size);
        GST_BUFFER_PTS(buffer) = gst_util_uint64_scale(color_frame.get_timestamp(), GST_MSECOND, 1);
    } else {
        rs2::depth_frame depth_frame = frames.get_depth_frame();
        if (!depth_frame) {
            gst_buffer_unmap(buffer, &map);
            gst_buffer_unref(buffer);
            return;
        }
        memcpy(map.data, depth_frame.get_data(), frame_size);
        GST_BUFFER_PTS(buffer) = gst_util_uint64_scale(depth_frame.get_timestamp(), GST_MSECOND, 1);
    }

    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale(1000 / fps, GST_MSECOND, 1);

    gst_buffer_unmap(buffer, &map);

    GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(appsrc), buffer);
    if (ret != GST_FLOW_OK) {
        std::cerr << "[Realsense] Error pushing buffer" << std::endl;
        gst_buffer_unref(buffer);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "[Realsense] Usage: " << argv[0] << " <IP_ADDRESS> <COLOR_PORT> <DEPTH_PORT>" << std::endl;
        return -1;
    }

    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);

    std::string ip_address = argv[1];
    std::string color_port = argv[2];
    std::string depth_port = argv[3];

    int fps = 30;
    int width = 640;
    int height = 480;

    gst_init(&argc, &argv);

    // Low-latency optimized pipelines
    std::string pipeline_color_desc =
        "appsrc name=color_src format=time max-bytes=" + std::to_string(width*height*3*2) + " "
        "! video/x-raw,format=RGB,width=" + std::to_string(width) +
        ",height=" + std::to_string(height) + ",framerate=" + std::to_string(fps) + "/1 "
        "! queue max-size-buffers=1 leaky=downstream "
        "! videoconvert "
        "! x264enc speed-preset=ultrafast tune=zerolatency key-int-max=15 threads=1 "
        "! rtph264pay config-interval=1 "
        "! udpsink host=" + ip_address + " port=" + color_port + " sync=false";

    std::string pipeline_depth_desc =
        "appsrc name=depth_src format=time max-bytes=" + std::to_string(width*height*2*2) + " "
        "! video/x-raw,format=GRAY16_LE,width=" + std::to_string(width) +
        ",height=" + std::to_string(height) + ",framerate=" + std::to_string(fps) + "/1 "
        "! queue max-size-buffers=1 leaky=downstream "
        "! udpsink host=" + ip_address + " port=" + depth_port + " sync=false";

    GError *error = nullptr;
    pipeline_color = gst_parse_launch(pipeline_color_desc.c_str(), &error);
    pipeline_depth = gst_parse_launch(pipeline_depth_desc.c_str(), &error);

    if (!pipeline_color || !pipeline_depth || error) {
        std::cerr << "[Realsense] Failed to create GStreamer pipelines: " << (error ? error->message : "Unknown error") << std::endl;
        return -1;
    }

    appsrc_color = gst_bin_get_by_name(GST_BIN(pipeline_color), "color_src");
    appsrc_depth = gst_bin_get_by_name(GST_BIN(pipeline_depth), "depth_src");

    if (!appsrc_color || !appsrc_depth) {
        std::cerr << "[Realsense] Failed to get appsrc elements" << std::endl;
        return -1;
    }

    gst_element_set_state(pipeline_color, GST_STATE_PLAYING);
    gst_element_set_state(pipeline_depth, GST_STATE_PLAYING);

    rs2::pipeline rs_pipeline;
    rs2::config cfg;
    cfg.enable_stream(RS2_STREAM_COLOR, width, height, RS2_FORMAT_RGB8, fps);
    cfg.enable_stream(RS2_STREAM_DEPTH, width, height, RS2_FORMAT_Z16, fps);
    rs_pipeline.start(cfg);

    std::cout << "[Realsense] Streaming color to " << ip_address << ":" << color_port
              << " and depth to " << ip_address << ":" << depth_port << std::endl;

    while (keep_running.load()) {
        try {
            rs2::frameset frames = rs_pipeline.wait_for_frames();
            push_frame(frames, true, width, height, fps);
            push_frame(frames, false, width, height, fps);
        } catch (const rs2::error &e) {
            std::cerr << "[Realsense] RS error: " << e.what() << std::endl;
        }
    }

    rs_pipeline.stop();

    gst_element_set_state(pipeline_color, GST_STATE_NULL);
    gst_element_set_state(pipeline_depth, GST_STATE_NULL);
    gst_object_unref(appsrc_color);
    gst_object_unref(appsrc_depth);
    gst_object_unref(pipeline_color);
    gst_object_unref(pipeline_depth);

    std::cout << "[Realsense] Closed" << std::endl;
    return 0;
}