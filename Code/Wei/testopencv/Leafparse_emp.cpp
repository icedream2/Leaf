#include"Leafparse_emp.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include<iostream>
using namespace cv;
static const double pi=3.1415926;
static const int thre=170;

Mat Leaf::thresholding(const Mat img){
	Mat tmp(img.clone());
	Mat tmp3(img.size(),CV_8U);
	int t=thre;

	while(1){
		threshold(tmp, tmp3, t, 255, THRESH_BINARY);//二值化
		t-=2;
		if(tmp3.at<uchar>(Point(0,0))!=0)
			break;
	}

	tmp=tmp3;
	Mat tmp2(tmp.rows+20,tmp.cols+20,tmp.depth());
	copyMakeBorder(tmp,tmp2,10,10,10,10,BORDER_ISOLATED,Scalar(0xff,0xff,0xff,0));
	return tmp2;
}

vector<Point> Leaf::get_contour(const Mat img){
	Mat tmp2(img.clone());
	vector<vector<Point>> cts;
	findContours(tmp2, cts, CV_RETR_EXTERNAL|RETR_LIST, CHAIN_APPROX_NONE);
	//drawContours(tmp2, cts, -1, cv::Scalar(0xff, 0x00, 0xff, 0));
	int index1=0,index2=0;
	double size1=0.0,size2=0.0;
	for(int i=0;i<cts.size();i++){//由于有多个轮廓取第二大的

		if(size1<abs(contourArea(cts[i]))){
			size2=size1;
			size1=abs(contourArea(cts[i]));
			index2=index1;
			index1=i;
		}

		else if(size2<abs(contourArea(cts[i]))){
			size2=abs(contourArea(cts[i]));
			index2=i;
		}
	}
	return cts[index2];
}
vector<Point> Leaf::preprocess(const Mat img){
    return  get_contour(thresholding(img));
}

vector<double> Leaf::contour_analysis(const vector<Point>contour){

	vector<double>feature;

	vector<Point> convex;
    convexHull(contour, convex);

	auto bounding_box=minAreaRect(contour);
    
	Point2f center;
	float radius;
	minEnclosingCircle(contour,center,radius);
	auto ell = cv::fitEllipse(contour);

    
	auto rect = std::minmax(bounding_box.size.height, bounding_box.size.width);
    auto ell2 = std::minmax(ell.size.height, ell.size.width);
    auto contour_area = contourArea(contour);
    auto convex_area = contourArea(convex);
    auto contour_perimeter = arcLength(contour, true);
    auto convex_perimeter = arcLength(convex, true);

    // get shape related
    auto aspect_ratio = double(rect.second) / rect.first;
    auto rectangularity = contour_area / (bounding_box.size.height*bounding_box.size.width);
    auto area_convexity = contour_area / convex_area;
    auto perimeter_convexity =  contour_perimeter / convex_perimeter;
    auto sphericity = (contour_area * 4 * pi) / (convex_perimeter * convex_perimeter);
        // circularity cannot be calculated without inscribed circle.
    auto eccentricity = ell2.second / ell2.first;
    auto form_factor = (contour_area * 4 * pi) / (contour_perimeter * contour_perimeter);

    feature.push_back(aspect_ratio);
    feature.push_back(rectangularity);
    feature.push_back(area_convexity);
    feature.push_back(perimeter_convexity);
    feature.push_back(sphericity);
    feature.push_back(eccentricity);
    feature.push_back(form_factor);

    // get humoments
    double h[7] = {};
    cv::HuMoments(cv::moments(contour), h);
    feature.insert(feature.end(), std::begin(h), std::end(h));

    return feature;

}


Mat Leaf::extract(const Mat img,const vector<Point>con){

	if(img.type()==CV_8UC1){
		Mat tmp(img.rows,img.cols,CV_8U);
		vector<vector<Point>>cts;
		cts.push_back(con);
		tmp.setTo(Scalar::all(255));
		drawContours(tmp,cts,-1,Scalar(0,0,0),-1);

		img|=tmp;
	}
	else if(img.type()==CV_8UC3){
		Mat tmp(img.rows,img.cols,CV_8UC3);
		vector<vector<Point>>cts;
		cts.push_back(con);
		tmp.setTo(Scalar::all(255));
		drawContours(tmp,cts,-1,Scalar(0,0,0),-1);

		img|=tmp;
	}

	return img;
}

vector<double> Leaf::vein_analysis(const Mat img){
	Mat rec(img.clone());
	Mat tmp;
	vector<double>feature;

	return feature;
}
