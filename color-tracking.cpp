// g++ color-tracking.cpp -o color-tracking `pkg-config --cflags --libs opencv`
// ./color-tracking

/*
Ce programme cherche tous les pixels de teinte et saturation (HS de HSV) proches du pixel de référence
(à définir en cliquant sur l'image).
La valeur (V de HSV) est volontairement ignorée, car on veut que ça ne dépende pas de l'éclairage.
On calcule le barycentre des points qui ressemblent au pixel de référence, et on l'affiche avec un rond. 
*/
 
#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

// here put the number of the webcam stream to use 
// with Linux, a webcam stream is a file beginning by /dev/video
#define WEBCAM 0
 
// Maths methods
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b)) 
#define abs(x) ((x) > 0 ? (x) : -(x))
#define sign(x) ((x) > 0 ? 1 : -1)
 
// Step mooving for object min & max
#define STEP_MIN 5
#define STEP_MAX 100
 
IplImage *image;
 
// Position of the object we overlay
CvPoint objectPos = cvPoint(-1, -1);
// Color tracked and our tolerance towards it
int h = 0, s = 0, v = 0, tolerance = 10;

CvPoint binarisation(IplImage* image, int *nbPixels); // return the barycenter of the color tracked
void addObjectToVideo(IplImage* image, CvPoint objectNextPos, int nbPixels); // add circle to image
void getObjectColor(int event, int x, int y, int flags, void *param);  // updates h, s, v


int main() {
 
    // link to the current image
    IplImage *hsv;

    // link to webcam video streams
    CvCapture *capture;

    // Key for keyboard event
    char key;
 
    // Number of tracked pixels
    int nbPixels;
    // Next position of the object we overlay
    CvPoint objectNextPos;
 
    // Initialize the video Capture (200 => CV_CAP_V4L2)
    capture = cvCreateCameraCapture( WEBCAM );
 
    // Check if the capture is ok
    if (capture == NULL) {
        printf("Can't initialize the video capture.\n");
        return -1;
    }
 
    // Create the windows
    cvNamedWindow("Color Tracking", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("Binary Mask", CV_WINDOW_AUTOSIZE);
    // to set the position of the window
    cvMoveWindow("Color Tracking", 0, 100);
    cvMoveWindow("Binary Mask", 650, 100);
 
    // Mouse event to select the tracked color on the original image
    cvSetMouseCallback("Color Tracking", getObjectColor);
 
    // While we don't want to quit
    while(key != 'Q' && key != 'q') {
 
        // We get the current image
        image = cvQueryFrame(capture);
 
        // If there is no image, we exit the loop
        if(!image)
            continue;
 
        objectNextPos = binarisation(image, &nbPixels); // we get the barycenter of the color to track
        addObjectToVideo(image, objectNextPos, nbPixels); // draw a circle on the video
 
        // We wait 10 ms
        key = cvWaitKey(10);
 
    }
 
    // Destroy the windows we have created
    cvDestroyWindow("Color Tracking");
    cvDestroyWindow("Binary Mask");
 
    // release the webcam
    cvReleaseCapture(&capture);
 
    return 0;
 
}

/*
 * Transform the image into a two colored image, one color for the color we want to track, another color for the others colors
 * From this image, we get two datas : the number of pixel detected, and the center of gravity of these pixel
 */
CvPoint binarisation(IplImage* image, int *nbPixels) {
 
    int x, y;
    CvScalar pixel;
    IplImage *hsv, *mask;
    IplConvKernel *kernel;
    int sommeX = 0, sommeY = 0;
    *nbPixels = 0;
 
    // Create the mask &initialize it to white (no color detected)
    mask = cvCreateImage(cvGetSize(image), image->depth, 1);
 
    // Create the hsv image
    hsv = cvCloneImage(image);
    cvCvtColor(image, hsv, CV_BGR2HSV);
 
    // We create the mask
    cvInRangeS(hsv, cvScalar(h - tolerance -1, s - tolerance, 0), cvScalar(h + tolerance -1, s + tolerance, 255), mask);
 
    // Create kernels for the morphological operation
    kernel = cvCreateStructuringElementEx(5, 5, 2, 2, CV_SHAPE_ELLIPSE);
 
    // Morphological opening (inverse because we have white pixels on black background)
    cvDilate(mask, mask, kernel, 1);
    cvErode(mask, mask, kernel, 1); 
 
    // We go through the mask to look for the tracked object and get its gravity center
    for(x = 0; x < mask->width; x++) {
        for(y = 0; y < mask->height; y++) {
 
            // If its a tracked pixel, count it to the center of gravity's calcul
            if(((uchar *)(mask->imageData + y*mask->widthStep))[x] == 255) { // if pixel is white (?)
                sommeX += x;
                sommeY += y;
                (*nbPixels)++;
            }
        }
    }
 
    // Show the result of the mask image
    cvShowImage("Binary Mask", mask);
 
    // We release the memory of kernels
    cvReleaseStructuringElement(&kernel);
 
    // We release the memory of the mask
    cvReleaseImage(&mask);
    // We release the memory of the hsv image
    cvReleaseImage(&hsv);
 
    // If there is no pixel, we return a center outside the image, else we return the center of gravity
    if(*nbPixels > 0)
        return cvPoint((int)(sommeX / (*nbPixels)), (int)(sommeY / (*nbPixels)));
    else
        return cvPoint(-1, -1);
}
 
/*
 * Add a circle on the video that fellow your colored object
 */
void addObjectToVideo(IplImage* image, CvPoint objectNextPos, int nbPixels) {
 
    int objectNextStepX, objectNextStepY;
 
    // Calculate circle next position (if there is enough pixels)
    if (nbPixels > 10) {
 
        // Reset position if no pixel were found
        if (objectPos.x == -1 || objectPos.y == -1) {
            objectPos.x = objectNextPos.x;
            objectPos.y = objectNextPos.y;
        }
 
        // Move step by step the object position to the desired position
        if (abs(objectPos.x - objectNextPos.x) > STEP_MIN) {
            objectNextStepX = max(STEP_MIN, min(STEP_MAX, abs(objectPos.x - objectNextPos.x) / 2));
            objectPos.x += (-1) * sign(objectPos.x - objectNextPos.x) * objectNextStepX;
        }
        if (abs(objectPos.y - objectNextPos.y) > STEP_MIN) {
            objectNextStepY = max(STEP_MIN, min(STEP_MAX, abs(objectPos.y - objectNextPos.y) / 2));
            objectPos.y += (-1) * sign(objectPos.y - objectNextPos.y) * objectNextStepY;
        }
 
    // -1 = object isn't within the camera range
    } else {
 
        objectPos.x = -1;
        objectPos.y = -1;
 
    }
 
    // Draw an object (circle) centered on the calculated center of gravity
    if (nbPixels > 10)
        cvDrawCircle(image, objectPos, 15, CV_RGB(255, 0, 0), -1);
 
    // We show the image on the window
    cvShowImage("Color Tracking", image);
 
}
 
/*
 * Get the color of the pixel where the mouse has clicked
 * We put this color as model color (the color we want to tracked)
 * (this function updates the global variables h, s and v)
 */
void getObjectColor(int event, int x, int y, int flags, void *param = NULL) {
 
    // Vars
    CvScalar pixel;  // to keep the values of a pixel (with maximum 4 channels)
    IplImage *hsv;
 
    if(event == CV_EVENT_LBUTTONUP) {
 
        // Get the hsv image
        hsv = cvCloneImage(image);
        cvCvtColor(image, hsv, CV_BGR2HSV);
 
        // Get the selected pixel
        pixel = cvGet2D(hsv, y, x);
 
        // Change the value of the tracked color with the color of the selected pixel
        h = (int)pixel.val[0];
        s = (int)pixel.val[1];
        v = (int)pixel.val[2];
 
        // Release the memory of the hsv image
        cvReleaseImage(&hsv);
 
    }
 
}