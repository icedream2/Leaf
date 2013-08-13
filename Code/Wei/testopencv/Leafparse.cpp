#include"Leafparse.h"
#include"Leafparse_emp.h"
#include"Fetcher.h"
#include <opencv2/highgui/highgui.hpp>
using namespace cv;
using namespace std;
static const int H=70,S=60;
static const double Ratio=0.855,TVariant=4000,BVariant=1000;
static const int frame=30;


vector<double> Leaf::get_feature(const Mat& image,const vector<Point> &contour){
	assert(image.type()==CV_8UC3);

	Mat img;
	cvtColor(image,img,CV_BGR2GRAY);

	vector<double>feature;
	vector<double>cfeature(contour_analysis(contour));
	feature.insert(feature.end(),cfeature.begin(),cfeature.end());
	vector<double>vfeature(vein_analysis(extract(img,contour)));
	feature.insert(feature.end(),vfeature.begin(),vfeature.end());
	return feature;
}

vector<Point> Leaf::limit_contour(const Mat& image,const vector<Point>& contour){
	if(image.type()!=CV_8UC3||image.empty())
		return vector<Point>();
	Mat aaa;
	bilateralFilter(image,aaa,5,50.0,2.0);


	Snakefetcher fetcher;
	fetcher.load("hehe",aaa);

	if(contour.size()<5){
		vector<Point>c;
		c.push_back(Point(frame,frame));
		c.push_back(Point(image.cols-frame,frame));
		c.push_back(Point(image.cols-frame,image.rows-frame));
		c.push_back(Point(frame,image.rows-frame));
		fetcher.set_contour(c);
	}
	else{
		fetcher.set_contour(contour);
	}
	return fetcher.fetch_ok();
}

bool Leaf::is_leaf(const Mat& image,const vector<Point>& contour){
    Mat hsv;
	assert(image.type()==CV_8UC3);
	Mat img;
	img=extract(image,contour);
	long long int psum=0,t=0,vsum=0;
	cvtColor(img,hsv,CV_BGR2HSV);
	for(int i=0;i<hsv.cols;i++){
		for(int j=0;j<hsv.rows;j++){
			if((int)hsv.at<Vec3b>(Point(i,j))[1]<S)
				continue;
			else{
				psum++;
				vsum=+(int)hsv.at<Vec3b>(Point(i,j))[0];
				if(hsv.at<Vec3b>(Point(i,j))[0]<H)
					t++;
			}
		}
	}
	if(1.0*t/psum<Ratio){
		//.std::cout<<1.0*t/psum<<std::endl;
		return false;
	}
	int avg=vsum/psum;
	if(psum==0)
		return false;
	vsum=0;
	for(int i=0;i<hsv.cols;i++){
		for(int j=0;j<hsv.rows;j++){
			if((int)hsv.at<Vec3b>(Point(i,j))[1]<S)
				continue;
			else{
				vsum+=(avg-(int)hsv.at<Vec3b>(Point(i,j))[0])*(avg-(int)hsv.at<Vec3b>(Point(i,j))[0]);
			}
		}
	}

	if(vsum/psum<BVariant||vsum/psum>TVariant){
		//std::cout<<vsum/psum<<std::endl;
		return false;
	}
	return true;
}

vector<Point> Leaf::refine_contour(const Mat& image,const vector<Point>& contour){
	assert(image.type()==CV_8UC3&&contour.size()>5);

	Mat img=extract(image,contour);
	Mat black=thresholding(img);
	return get_contour(black);

}
