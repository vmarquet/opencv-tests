// g++ k-mean.cpp -o k-mean -ggdb `pkg-config --cflags --libs opencv`
// ./k-mean

// from http://stackoverflow.com/questions/9575652/opencv-using-k-means-to-posterize-an-image

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>

using namespace cv;
using namespace std;

int K = 2;  // number of clusters to split the set by

int main() {
    Mat src, blurred, bestLabels1, bestLabels2, centers, src_clustered, blurred_clustered;
    int display_margin_x = 400;  // margin beetween windows
    int display_margin_y = 80;

    // we display source image
    src = imread("images/fruits.jpg");
    imshow("original", src);
    cvMoveWindow("original", 0, 30);

    // we blur source image
    blurred = Mat(src.rows, src.cols, CV_32F);
    blur(src, blurred, Size(15,15));  // param 3 is kernel size
    imshow("blurred", blurred);
    cvMoveWindow("blurred", 0+display_margin_x, 30+display_margin_y);

    Mat p1 = Mat::zeros(src.cols*src.rows, 5, CV_32F);
    vector<Mat> bgr1;
    cv::split(src, bgr1);
    // i think there is a better way to split pixel bgr color
    for(int i=0; i<src.cols*src.rows; i++) {
        p1.at<float>(i,0) = (i/src.cols) / src.rows;
        p1.at<float>(i,1) = (i%src.cols) / src.cols;
        p1.at<float>(i,2) = bgr1[0].data[i] / 255.0;
        p1.at<float>(i,3) = bgr1[1].data[i] / 255.0;
        p1.at<float>(i,4) = bgr1[2].data[i] / 255.0;
    }

    Mat p2 = Mat::zeros(blurred.cols*blurred.rows, 5, CV_32F);
    vector<Mat> bgr2;
    cv::split(blurred, bgr2);
    // i think there is a better way to split pixel bgr color
    for(int i=0; i<blurred.cols*blurred.rows; i++) {
        p2.at<float>(i,0) = (i/blurred.cols) / blurred.rows;
        p2.at<float>(i,1) = (i%blurred.cols) / blurred.cols;
        p2.at<float>(i,2) = bgr2[0].data[i] / 255.0;
        p2.at<float>(i,3) = bgr2[1].data[i] / 255.0;
        p2.at<float>(i,4) = bgr2[2].data[i] / 255.0;
    }

    // k-mean on source image, not blurred
    cv::kmeans(p1,  // floating-point matrix of input samples, one row per sample
               K,  // number of clusters to split the set by
               bestLabels1,  // the input/output integer array that will store the cluster indices for every sample
               TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 1.0),
               3, // How many times the algorithm is executed using different initial labelings. 
                  // The algorithm returns the labels that yield the best compactness
               KMEANS_PP_CENTERS,  // to select how initial centers are choosen
               centers);  // The output matrix of the cluster centers, one row per each cluster center

    // k-mean on blurred image
    cv::kmeans(p2,  // floating-point matrix of input samples, one row per sample
               K,  // number of clusters to split the set by
               bestLabels2,  // the input/output integer array that will store the cluster indices for every sample
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
    src_clustered = Mat(src.rows, src.cols, CV_32F);
    for(int i=0; i<src.cols*src.rows; i++) {
        src_clustered.at<float>(i/src.cols, i%src.cols) = (float)(colors[bestLabels1.at<int>(0,i)]);
        // cout << bestLabels.at<int>(0,i) << " " << 
        //          colors[bestLabels.at<int>(0,i)] << " " << 
        //          src_clustered.at<float>(i/src.cols, i%src.cols) << " " <<
        //          endl;
    }

    // we fill clustered mat with grey shade, depending on what area the pixel belongs to
    blurred_clustered = Mat(src.rows, src.cols, CV_32F);
    for(int i=0; i<src.cols*src.rows; i++) {
        blurred_clustered.at<float>(i/src.cols, i%src.cols) = (float)(colors[bestLabels2.at<int>(0,i)]);
        // cout << bestLabels.at<int>(0,i) << " " << 
        //          colors[bestLabels.at<int>(0,i)] << " " << 
        //          blurred_clustered.at<float>(i/src.cols, i%src.cols) << " " <<
        //          endl;
    }

    // we display source clustered image
    src_clustered.convertTo(src_clustered, CV_8U);
    imshow("source clustered not blurred", src_clustered);
    cvMoveWindow("source clustered not blurred", 0+2*display_margin_x, 30);

    // we display blurred clustered image
    blurred_clustered.convertTo(blurred_clustered, CV_8U);
    imshow("blurred before clustered", blurred_clustered);
    cvMoveWindow("blurred before clustered", 0+2*display_margin_x, 30+2*display_margin_y);

    waitKey();
    return 0;
}