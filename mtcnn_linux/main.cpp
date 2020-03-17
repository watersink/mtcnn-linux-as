#include "mtcnn.h"
#include <opencv2/opencv.hpp>

int main(){
    MTCNN mtcnn("../assets");
    std::vector<Bbox> finalBbox;


	cv::Mat frame = cv::imread("../beauty.jpg");
    ncnn::Mat inmat = ncnn::Mat::from_pixels(frame.data, ncnn::Mat::PIXEL_BGR2RGB, frame.cols, frame.rows);


    mtcnn.detect(inmat,finalBbox);

	std::cout<<finalBbox.size()<<std::endl;

    return 0;
}
