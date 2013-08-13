#ifndef LEAFPARSE_EMP_H
#define LEAFPARSE_EMP_H
#include"Leafparse.h"

using namespace cv;
namespace Leaf {
	vector<Point> preprocess(const Mat img);
	Mat extract(const Mat,const vector<Point>);
	vector<double> contour_analysis(const vector<Point>);

	vector<double> vein_analysis(const Mat);
	Mat thresholding(const Mat img);
	vector<Point> get_contour(const Mat img);
};




#endif