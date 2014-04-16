// g++ -ggdb `pkg-config --cflags opencv` camshiftdemo.cpp `pkg-config --libs opencv` -o camshiftdemo
// ./camshiftdemo

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;

// here put the number of the webcam stream to use 
// using Linux, a webcam stream is a file beginning by /dev/video
#define CAMERA 0

typedef enum tracking {
    NONE,  //  nothing to track (it is set to 0 when we push 'c', and the tracking is reset)
    INITIATING,  // rectangle selected by user, but camshift not initiated yet
    IN_PROGRESS  // tracking in progress
} tracking;

bool maskMode = false;      // to visualize the mask
bool selectingRect = false;   // it's true when we are selecting a rectangle on the screen, and false when we release mouse
tracking trackingMode = NONE;  // see tracking enum
bool showHist = true;  // boolean whether we want to show histogram or not
bool paused = false;   // whether the stream is paused or not
Point origin;
Rect selection;
int vmin = 10, vmax = 256, smin = 30;

static void onMouse( int event, int x, int y, int, void* );
static void help();

Mat image;
const char* keys = { "{1|  | 0 | camera number}" };

int main( int argc, const char** argv )
{
    help();  // print to console

    VideoCapture cap;
    Rect trackWindow;  // the rectangle selected by user

    int numberColsHisto = 16;  // number of columns in the histograms
    float hranges[] = {0,180};  // this means we will use pixel values from 0 to 180
    const float* phranges = hranges;

    CommandLineParser parser(argc, argv, keys);
    int camNum = parser.get<int>("1");

    cap.open( CAMERA );  // put camNum if we want to use command line arguments 

    if( !cap.isOpened() )
    {
        help();
        cout << "***Could not initialize capturing...***\n";
        cout << "Current parameter's value: \n";
        parser.printParams();
        return -1;
    }

    namedWindow( "Histogram", 0 );
    namedWindow( "CamShift Demo", 0 );
    setMouseCallback( "CamShift Demo", onMouse, 0 );
    // barres de défilement pour pouvoir régler les valeurs en live:
    createTrackbar( "Vmin", "CamShift Demo", &vmin, 256, 0 );
    createTrackbar( "Vmax", "CamShift Demo", &vmax, 256, 0 );
    createTrackbar( "Smin", "CamShift Demo", &smin, 256, 0 );

    Mat frame, hsv, hue, mask, hist, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;

    for(;;)  // infinite loop
    {
        if( !paused )  // we get next frame
        {
            cap >> frame;
            if( frame.empty() )
                break;
        }

        frame.copyTo(image);  // copy frame into image (both are Mat)

        if( !paused )
        {
            cvtColor(image, hsv, COLOR_BGR2HSV);  // we convert image from RGB to HSV, output in Mat hsv

            if( trackingMode != NONE )
            {
                // NB: l'objet Scalar permet de stocker les valeurs d'un pixel
                // c'est un vecteur à 3 ou 4 dimensions

                inRange(hsv, Scalar(0, smin, MIN(vmin,vmax)), Scalar(180, 256, MAX(vmin, vmax)), mask);
                // from hsv (arg1), we create a mask (arg4)
                // arg2 and arg3 are the conditions to choose the mask
                // arg2 -> minimum
                // arg3 -> maximum

                // so the HSV mask is:
                // 0 < H < 180
                // smin < S < 256
                // MIN(vmin,vmax) < V < MAX(vmin, vmax)

                int ch[] = {0, 0};
                // index pair -> le n° du channel de l'image d'entrée (&hsv)...
                // index impair -> ...le n° du channel dans l'image de sortie(&hue)

                hue.create(hsv.size(), hsv.depth());
                // we set Mat hue to the same size as Mat hsv

                mixChannels(&hsv, 1, &hue, 1, ch, 1);
                // meaning: the channel 0 from hsv (hue channel) becomes channel 0 of hue
                // the last arg is the number of index pairs in ch[]
                // (arg2 and arg4 means that hsv and hue contains only 1 matrice)

                // so Mat hue contains only one channel: the hue channel from hsv

                if( trackingMode == INITIATING )  // user just selected rectangle to track, so we must initiate tracking
                {
                    // roi = Region Of Interest

                    // we get a matrice containing only the rectangle selected by user with mouse 
                    Mat roi(hue, selection), maskroi(mask, selection);

                    // we compute the histogram (filled with H (hue) values)
                    calcHist(&roi, 1, 0, maskroi, hist, 1, &numberColsHisto, &phranges);
                    // reminder: numberColsHisto = number of columns
                    //           phranges = {0, 180}, pixel range of hue component (9)
                    // see http://docs.opencv.org/doc/tutorials/imgproc/histograms/histogram_calculation/histogram_calculation.html

                    // we normalize beetween 0 and 255
                    normalize(hist, hist, 0, 255, CV_MINMAX);

                    trackWindow = selection;
                    trackingMode = IN_PROGRESS;

                    histimg = Scalar::all(0);  // reset the histogram
                    int columnWidth = histimg.cols / numberColsHisto;
                    // column width = histogram width / number of columns

                    Mat buf(1, numberColsHisto, CV_8UC3);
                    // CV_8UC3 = 8 bit, unsigned char, 3 channels

                    for( int i = 0; i < numberColsHisto; i++ )
                        buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./numberColsHisto), 255, 255);
                    cvtColor(buf, buf, CV_HSV2BGR);

                    for( int i = 0; i < numberColsHisto; i++ )
                    {
                        int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows/255);
                        rectangle( histimg, Point(i*columnWidth,histimg.rows),
                                   Point((i+1)*columnWidth,histimg.rows - val),
                                   Scalar(buf.at<Vec3b>(i)), -1, 8 );
                    }
                }

                calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
                backproj &= mask;
                RotatedRect trackBox = CamShift(backproj, trackWindow,
                                    TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));

                if( trackWindow.area() <= 1 )
                {
                    int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
                    trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
                                       trackWindow.x + r, trackWindow.y + r) &
                                  Rect(0, 0, cols, rows);
                }

                if( maskMode )  // if we want to display the mask instead of webcam stream
                    cvtColor( backproj, image, COLOR_GRAY2BGR );
                    // we convert backproj to RGB format, and we put the result in image, to be displayed

                ellipse( image, trackBox, Scalar(0,0,255), 3, CV_AA ); // we draw the ellipse on image
            }
        }
        else if( trackingMode ==  INITIATING )  // if it's paused while initiating
            paused = false;

        // selectingRect==true => the user is selecting the rectangle but has not released yet
        if( selectingRect==true && selection.width > 0 && selection.height > 0 )
        {
            Mat roi(image, selection);  // we get the Region Of Interest
            bitwise_not(roi, roi);  // inverts every bit, 
            // to display with special colors the rectangle while user is selecting it
        }

        // we update images display in the windows
        imshow( "CamShift Demo", image );
        imshow( "Histogram", histimg );

        // we get the key pressed
        char c = (char)waitKey(10);
        if( c == 27 )  // 27 = ESCAPE
            break;
        switch(c)
        {
            case 'm':   // pour le masque en noir et blanc
                maskMode = !maskMode;
                break;
            case 't':   // to reset tracking
                trackingMode = NONE;
                histimg = Scalar::all(0);
                break;
            case 'h':   // to show histogram
                showHist = !showHist;
                if( !showHist )
                    destroyWindow( "Histogram" );
                else
                    namedWindow( "Histogram", 1 );
                break;
            case 'p':
                paused = !paused;
                break;
            default:
                break;
        }
    }

    return 0;
}

static void onMouse( int event, int x, int y, int, void* )
{
    if( selectingRect )
    {
        selection.x = MIN(x, origin.x);
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x);
        selection.height = std::abs(y - origin.y);

        selection &= Rect(0, 0, image.cols, image.rows);
    }

    switch( event )
    {
    case CV_EVENT_LBUTTONDOWN:  // when we press LEFT BUTTON
        origin = Point(x,y);
        selection = Rect(x,y,0,0);
        selectingRect = true;
        break;
    case CV_EVENT_LBUTTONUP:  // when we release LEFT BUTTON
        selectingRect = false;
        if( selection.width > 0 && selection.height > 0 )
            trackingMode = INITIATING;
        break;
    }
}

static void help()
{
    cout << "\nThis is a demo that shows mean-shift based tracking\n"
            "You select a color objects such as your face and it tracks it.\n"
            "This reads from video camera (0 by default, or the camera number the user enters\n"
            "Usage: \n"
            "   ./camshiftdemo [camera number]\n";

    cout << "\n\nHot keys: \n"
            "\t ESC - quit the program\n"
            "\t t - stop the tracking\n"
            "\t m - switch to/from mask view\n"
            "\t h - show/hide object histogram\n"
            "\t p - pause video\n"
            "To initialize tracking, select the object with mouse\n";
}