#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "gst/app/app-prelude.h"
#include "gstnvdsmeta.h"

typedef struct gst_app_interface
{
    void* user_data;

    void (*on_detection)(void* user_data);
    const void* (*on_get_frame)(void* user_data);
} gst_app_interface;

void gst_app_run(gint width, gint height, gint fps, const std::string& config_path, bool multicast, const std::string& ip,  gst_app_interface* interface);