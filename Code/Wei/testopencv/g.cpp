#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include<cmath>
#include <sstream>
#include <cassert>
#include"Leafparse.h"
#include"Leafparse_emp.h"
#include"GCApplication.h"
#include"Fetcher.h"
using namespace Leaf;
using namespace std;
using namespace cv;
//const double pi=3.14159;
string file="tmp.jpg";
Mat img;
Leaf::Snakefetcher t;
int big=0;
int minaa(vector<Point> contour){
	Point2f center;
	float radius;
	minEnclosingCircle(contour,center,radius);
	cout<<radius<<endl;
	img.setTo(Scalar::all(0));
	vector<vector<Point>>cts;
	cts.push_back(contour);
	drawContours(img,cts,-1,Scalar(255,255,255),-1);
	int i=0;
	while(1){
		i++;
		erode(img,img,Mat(),Point(-1,-1),1);


		imshow("fdsa",img);
		waitKey(100);
		if(countNonZero(img)==0)
			break;
	}

	return i;
}

void mouse(int event,int x,int y,int flags,void * param){
	t.mouse(event,x,y,flags,param);
	t.show_image();
}

void on(int pos, void * d){
	Mat rec;
	threshold(img, rec, big ,255, THRESH_BINARY);
	imshow(file,rec);
}
int main(){
	img=imread(file,0);

	t.load(file,img);
	 namedWindow( file );
	setMouseCallback(file,mouse,0);
	createTrackbar(file,file,&big,200,on);
	on(0,0);
	imshow(file,img);
    while(1){
		char c=waitKey();
		if(c=='d')
			t.fetch_ok();
	}
	system("pause");
	return 0;
}

/*
int main(){

	img=imread(file);
	auto f=limit_contour(img,vector<Point>());

	vector<vector<Point>>cts;
	cts.push_back(f);
    
	Mat rec(img.clone());
	drawContours(rec,cts,-1,Scalar(255,0,0));
	imshow(file,rec);

	waitKey();

	system("pause");
	return 0;
}*/