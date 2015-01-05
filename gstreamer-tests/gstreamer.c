// gcc -ggdb `pkg-config --cflags opencv` gstreamer.c `pkg-config --libs opencv` -o gstreamer

// SOURCE: http://stackoverflow.com/questions/6022423/mjpeg-streaming-and-decoding

#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>


int main( int argc, char** argv){

    IplImage  *frame;
    int       key;

    /* supply the AVI file to play */
    assert( argc == 2 );

    /* load the AVI file */
    CvCapture *capture = cvCreateFileCapture(argv[1]) ; //cvCaptureFromAVI( argv[1] );

    /* always check */
    if( !capture ) {
        printf("ERROR: capture == NULL \n");
        return 1;    
    }

    /* get fps, needed to set the delay */
    int fps = ( int )cvGetCaptureProperty( capture, CV_CAP_PROP_FPS );

    int frameH    = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
    int frameW    = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);

    /* display video */
    cvNamedWindow( "video", CV_WINDOW_AUTOSIZE );

    while( key != 'q' ) {

    double t1=(double)cvGetTickCount();
    /* get a frame */
    frame = cvQueryFrame( capture );
    double t2=(double)cvGetTickCount();
    printf("time: %gms  fps: %.2g\n",(t2-t1)/(cvGetTickFrequency()*1000.), 1000./((t2-t1)/(cvGetTickFrequency()*1000.)));

    /* always check */
    if( !frame ) break;

    /* display frame */
    cvShowImage( "video", frame );

    /* quit if user press 'q' */
    key = cvWaitKey( 1000 / fps );
    }

    /* free memory */
    cvReleaseCapture( &capture );
    cvDestroyWindow( "video" );

    return 0;
}