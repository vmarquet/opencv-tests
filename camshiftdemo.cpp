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
// with Linux, a webcam stream is a file beginning by /dev/video
#define CAMERA 1

Mat image;

bool backprojMode = false;   // pour voir en N&B, les pixels correspondant en blanc, le reste en noir
bool selectObject = false;   // it's true when we are selecting a rectangle on the screen, and false when we release mouse
int trackObject = 0;  // possible values: -1, 0, 1
// -1 = rectangle selected by user, but camshift not initiated yet 
// 0 = nothing to track (it is set to 0 when we push 'c', and the tracking is reset)
// 1 = tracking en cours
bool showHist = true;  // boolean whether we want to show histogram or not
bool paused = false;   // whether the stream is paused or not
Point origin;
Rect selection;
int vmin = 10, vmax = 256, smin = 30;

static void onMouse( int event, int x, int y, int, void* );
static void help();

const char* keys = { "{1|  | 0 | camera number}" };

int main( int argc, const char** argv )
{
    help();

    VideoCapture cap;
    Rect trackWindow;  // the rectangle selected by user
    int hsize = 16;
    float hranges[] = {0,180};
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

        frame.copyTo(image);  // copy frame into image

        if( !paused )
        {
            cvtColor(image, hsv, COLOR_BGR2HSV);  // we convert image from RGB to HSV, output in object hsv

            if( trackObject != 0 )  // trackObject == 0 => nothing to track, so here there is something to track
            {
                // NB: l'objet Scalar permet de stocker les valeurs d'un pixel
                // c'est un vecteur à 3 ou 4 dimensions

                inRange(hsv, Scalar(0, smin, MIN(vmin,vmax)), Scalar(180, 256, MAX(vmin, vmax)), mask);
                // from hsv (arg1), we create a mask (arg4)
                // arg2 and arg3 are the conditions to choose the mask
                // arg2 -> minimum
                // arg3 -> maximum

                // the HSV mask is:
                // 0 < H < 180
                // smin < S < 256
                // MIN(vmin,vmax) < V < MAX(vmin, vmax)

                int ch[] = {0, 0};
                // index pair -> le n° du channel de l'image d'entrée (&hsv)...
                // index impair -> ...le n° du channel dans l'image de sortie(&hue)

                hue.create(hsv.size(), hsv.depth());

                mixChannels(&hsv, 1, &hue, 1, ch, 1);
                // meaning: the channel 0 from hsv becomes channel 0 of hue
                // the last arg is the number of index pairs in ch[]
                // (arg2 and arg4 means that hsv and hue contains only 1 matrice)

                if( trackObject == -1 )  // user just selected rectangle to track, so we must initiate tracking
                {
                    // roi = Region Of Interest

                    // we get a matrice containing only the rectangle selected by user with mouse 
                    Mat roi(hue, selection), maskroi(mask, selection);

                    // we compute the histogram
                    calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);

                    // we normalize beetween 0 and 255
                    normalize(hist, hist, 0, 255, CV_MINMAX);

                    trackWindow = selection;
                    trackObject = 1;

                    histimg = Scalar::all(0);  // reset the histogram (?)
                    int binW = histimg.cols / hsize;
                    Mat buf(1, hsize, CV_8UC3);
                    for( int i = 0; i < hsize; i++ )
                        buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);
                    cvtColor(buf, buf, CV_HSV2BGR);

                    for( int i = 0; i < hsize; i++ )
                    {
                        int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows/255);
                        rectangle( histimg, Point(i*binW,histimg.rows),
                                   Point((i+1)*binW,histimg.rows - val),
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

                if( backprojMode )
                    cvtColor( backproj, image, COLOR_GRAY2BGR );
                ellipse( image, trackBox, Scalar(0,0,255), 3, CV_AA );
            }
        }
        else if( trackObject ==  -1 )
            paused = false;

        // selectObject==true => the user is selecting the rectangle but has not released yet (?)
        if( selectObject==true && selection.width > 0 && selection.height > 0 )
        {
            Mat roi(image, selection);  // we get the Region Of Interest
            bitwise_not(roi, roi);  // inverts every bit
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
            case 'b':   // pour le masque en noir et blanc
                backprojMode = !backprojMode;
                break;
            case 't':   // to reset tracking
                trackObject = 0;
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
    if( selectObject )
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
        selectObject = true;
        break;
    case CV_EVENT_LBUTTONUP:  // when we release LEFT BUTTON
        selectObject = false;
        if( selection.width > 0 && selection.height > 0 )
            trackObject = -1;
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
            "\tESC - quit the program\n"
            "\tc - stop the tracking\n"
            "\tb - switch to/from backprojection view\n"
            "\th - show/hide object histogram\n"
            "\tp - pause video\n"
            "To initialize tracking, select the object with mouse\n";
}