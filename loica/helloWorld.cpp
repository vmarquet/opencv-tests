// g++ helloWorld.cpp -o helloWorld `pkg-config --cflags opencv` `pkg-config --libs opencv`  

#include <cv.h>
#include <highgui.h>
#include <stdio.h>

using namespace cv; 
using namespace std;

int main(int argc, char** argv) {

	char key; 		// For keyboard interaction
		
	Mat frame; 		// Contener of the frame
	int frameSize; 	// Size of the frames
	int frameNumber = 0;
	
	stringstream frameName;
	
	int captNum = 0;
	int test = 0;
	
	
//	do {
		/* Open the default camera */	
		printf("Test:%d\n", captNum);
		VideoCapture capture(captNum); 
	
		captNum++;
		/* Check if openning the video has succeeded */
		if (!capture.isOpened()) {
			test = 0;
			printf("Video not openning :(\n");
			return -1;
		} else {
			test = 1;
		}
		
//	} while(test == 0);
	
	/* Create windows for display */
	namedWindow("video", CV_WINDOW_AUTOSIZE);
	
	capture >> frame;
	
	frameSize = frame.rows * frame.cols;
	 	
	/* Loop on the flux of frames */
	do {
		
		/* Grab the current frame */
		capture >> frame; 
		
	
	
		if (!frame.empty()) {
	
				
			// if (frameNumber%20 == 0) {		
			// 	frameName.str("");
			// 	frameName << "./video/frame" << frameNumber << ".jpg";
			// 
			// 	//cout << frameName.str() << endl;
			// 	if (imwrite(frameName.str(), frame)) {
			// 		printf("Write %d!\n", frameNumber);						
			// 	} else {
			// 		printf("Problem Writing %d!\n", frameNumber);						
			// 	}
			// 	
			// }
	
			/* Show the frames */
			imshow("Video", frame); 
		}
		
		frameNumber++;
	
		key = waitKey(10);	// Wait for a keystroke in the window 
	
		
	} while(key != 'q' && key != 'Q' && key != 27); 
	
	capture.release();
	
	return 0;

}
