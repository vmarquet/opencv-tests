// g++ gstreamer_gademer.c -o gstreamer_gademer `pkg-config --cflags --libs opencv gstreamer-0.10`

/*
 * Code written by Vinz (GeckoGeek.fr)
 * Updated by UrU (atis-lab.fr :p )
 */
 
#include <stdio.h>
 
//Gstreamer
#include <gst/gst.h>
#include <glib.h>

#include <highgui.h>
#include <cv.h>

void opencvWrapper(GstElement *fakesink, GstBuffer *buffer, GstPad *pad, gpointer user_data) {
	IplImage *img = (IplImage*) user_data;
	img->imageData = (char*) GST_BUFFER_DATA(buffer);
	cvShowImage("window",img);
}

int main(int argc, char *argv[]) {

	int width = 640;
	int height = 480;

	GMainLoop *loop = NULL;
	GstElement *pipeline  = NULL, *source=NULL, *conv=NULL, *filter=NULL, *fakesink=NULL;
	GstBus *bus = NULL;
	
	// Image OpenCV
	IplImage *image;
	
	//Initialisation de l'image
	image = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U, 3);



	//Init Gstreamer
	gst_init(&argc,&argv);
	
	//Initialisation de la chaine de process
	loop = g_main_loop_new(NULL, FALSE);
	pipeline = gst_pipeline_new("camviewer");
	source = gst_element_factory_make("v4l2src","file-source");
	conv = gst_element_factory_make("ffmpegcolorspace","conv");
	filter = gst_element_factory_make("capsfilter","filter");
	fakesink = gst_element_factory_make("fakesink","video-output");
	
	printf("Choix de la source\n");
	//Choix de la source (Gstreamer)
	g_object_set(G_OBJECT(source), "device", "/dev/video0",NULL);
	printf("Choix du format\n");
	//Choix du format et redimensionnement
	g_object_set(G_OBJECT(filter),"caps", gst_caps_new_simple("video/x-raw-rgb","bpp", G_TYPE_INT, 24,"depth", G_TYPE_INT, 24, "width", G_TYPE_INT, width,"height", G_TYPE_INT, height, "red_mask", G_TYPE_INT, 0xff,	"green_mask", G_TYPE_INT, 0xff00, "blue_mask", G_TYPE_INT, 0xff0000, NULL), NULL);
	printf("Construction du pipeline\n");
	//On ajouter les éléments de la chaine au pipeline
	gst_bin_add_many(GST_BIN(pipeline), source, conv, filter, fakesink, NULL);
	//On lie les éléments de la chaine entre eux
	gst_element_link_many(source, conv, filter, fakesink, NULL);
	printf("Rattachement du wrapper opencv\n");
	//On configure le fake sink
	g_object_set(G_OBJECT(fakesink), "signal-handoffs", TRUE, NULL);
	//On connect le fake sink avec notre code source
	g_signal_connect(fakesink, "handoff", (GCallback)opencvWrapper, image);
	printf("Ouverture de la fenêtre\n");
	
	// Définition de la fenêtre
	cvNamedWindow("window", CV_WINDOW_AUTOSIZE);
	printf("Go go go !\n");
	gst_element_set_state(pipeline,GST_STATE_PLAYING);
	g_main_loop_run(loop);
	
	
	
	printf("Never been there. How to properly quit ?\n");
	//Nettoyage Gstreamer
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(pipeline));
	//Nettoyage Opencv 
	cvDestroyWindow("window");
	printf("Bye !\n");
 
	return 0;
 
}
