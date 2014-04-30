#!/bin/bash

# packages needeed:
# gstreamer-tools

# we create a named pipe
mkfifo test_gstreamer

# Prepare To Convert From MJPEG to MPEG4 using gstreamer (rate of incoming frames critical)
gst-launch -v souphttpsrc location="http://<ip>/cgi_bin/<mjpeg>.cgi" do-timestamp=true is_live=true ! multipartdemux ! jpegdec ! queue ! videoscale ! 'video/x-raw-yuv, width=640, height=480'! queue ! videorate ! 'video/x-raw-yuv,framerate=30/1' ! queue ! ffmpegcolorspace ! 'video/x-raw-yuv,format=(fourcc)I420' ! ffenc_mpeg4 ! queue ! filesink location=test_gstreamer

# we compile the launch program
gcc -ggdb `pkg-config --cflags opencv` gstreamer.c `pkg-config --libs opencv` -o gstreamer

# we launch the test program, we give the named pipe as an argument
./gstreamer test_gstreamer

exit 0
