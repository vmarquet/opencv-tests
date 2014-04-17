// g++ -ggdb `pkg-config --cflags opencv` k-mean.cpp `pkg-config --libs opencv` -o k-mean
// ./k-mean

// from http://stackoverflow.com/questions/9575652/opencv-using-k-means-to-posterize-an-image

#include "cv.h"
#include "highgui.h"
#include <iostream>

using namespace cv;
using namespace std;

int K = 8;  // number of clusters to split the set by

int main() {
    Mat src, bestLabels, centers, clustered;
    int display_margin_x = 400;  // margin beetween windows
    int display_margin_y = 80;

    // we display source image
    src = imread("images/fruits.jpg");
    imshow("original", src);
    moveWindow("original", 0, 30);

    // we blur source image
    blur(src, src, Size(15,15));  // param 3 is kernel size
    imshow("blurred", src);
    moveWindow("blurred", 0+display_margin_x, 30+display_margin_y);

    Mat p = Mat::zeros(src.cols*src.rows, 5, CV_32F);
    vector<Mat> bgr;
    cv::split(src, bgr);
    // i think there is a better way to split pixel bgr color
    for(int i=0; i<src.cols*src.rows; i++) {
        p.at<float>(i,0) = (i/src.cols) / src.rows;
        p.at<float>(i,1) = (i%src.cols) / src.cols;
        p.at<float>(i,2) = bgr[0].data[i] / 255.0;
        p.at<float>(i,3) = bgr[1].data[i] / 255.0;
        p.at<float>(i,4) = bgr[2].data[i] / 255.0;
    }

    cv::kmeans(p,  // floating-point matrix of input samples, one row per sample
               K,  // number of clusters to split the set by
               bestLabels,  // the input/output integer array that will store the cluster indices for every sample
               TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 1.0),
               3, // How many times the algorithm is executed using different initial labelings. 
                  // The algorithm returns the labels that yield the best compactness
               KMEANS_PP_CENTERS,  // to select how initial centers are choosen
               centers);  // The output matrix of the cluster centers, one row per each cluster center

    // we convert the number of each one of the K areas to a grey shade, for the display 
    int colors[K];
    for(int i=0; i<K; i++) {
        colors[i] = 255/(i+1);
    }

    // we fill clustered mat with grey shade, depending on what area the pixel belongs to
    clustered = Mat(src.rows, src.cols, CV_32F);
    for(int i=0; i<src.cols*src.rows; i++) {
        clustered.at<float>(i/src.cols, i%src.cols) = (float)(colors[bestLabels.at<int>(0,i)]);
        // cout << bestLabels.at<int>(0,i) << " " << 
        //          colors[bestLabels.at<int>(0,i)] << " " << 
        //          clustered.at<float>(i/src.cols, i%src.cols) << " " <<
        //          endl;
    }

    // we display clustered image
    clustered.convertTo(clustered, CV_8U);
    imshow("clustered", clustered);
    moveWindow("clustered", 0+2*display_margin_x, 30+2*display_margin_y);

    waitKey();
    return 0;
}