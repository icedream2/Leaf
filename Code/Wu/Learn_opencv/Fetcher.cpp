#include"Fetcher.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/legacy/legacy.hpp>
#include<queue>
using namespace Leaf;
using namespace std;
using namespace cv;

static const double Wz=0.43,Wd=0.43,Wg=0.14,genhao2=1.414;
static const float pi=3.1415926;

static char mm[]={-1,-1,-1,0,0,1,1,1};
static char nn[]={-1,0,1,-1,1,-1,0,1};
static char table[][3]={{0,1,2},{3,-1,4},{5,6,7}};
static const cv::Scalar RED = cv::Scalar(0,0,255);
static const cv::Scalar PINK = cv::Scalar(230,130,255);
static const cv::Scalar BLUE = cv::Scalar(255,0,0);
static const cv::Scalar LIGHTBLUE = cv::Scalar(255,255,160);
static const cv::Scalar WHITE=cv::Scalar(255,255,255);
static const cv::Scalar GREEN = cv::Scalar(0,255,0);
static const double bigThreTR=500,smallThreTR=100;
static const double bigThre=150,smallThre=30;
static const int snakeIter=2,grabIter=2;

const int BGD_KEY = CV_EVENT_FLAG_CTRLKEY;
const int FGD_KEY = CV_EVENT_FLAG_SHIFTKEY;

struct Route{
	cv::Point dot;
	cv::Point pre;
	double distance;
};

struct Route_comp {
	bool operator()(const Route& left, const Route& right) const 
	{
		return left.distance > right.distance;
	}
};


Fetcher::Fetcher(){
}

Fetcher::Fetcher(string name,string url){
	load(name,url);
}

Fetcher::Fetcher(string name,Mat img){
	load(name,img);
}

Fetcher::~Fetcher(){}


void Fetcher::load(string name,string url){
	origin=imread(url);
	winname=name;
	stat=NOT_SET;
	mark.clear();
}

void Fetcher::load(string name ,Mat img){
	origin=img;
	winname=name;
	stat=NOT_SET;
	mark.clear();
}

void Fetcher::set_contour(vector<Point>contour){
	mark=contour;
	stat=IN_PROCESS;
}

void Fetcher::wait(){
	waitKey();
}

Unitedfetcher::Unitedfetcher(){
}

Unitedfetcher::Unitedfetcher(string name,string url)
	:Fetcher(name,url){}

Unitedfetcher::Unitedfetcher(string name,Mat img)
	:Fetcher(name,img){}

Unitedfetcher::~Unitedfetcher(){}

void Unitedfetcher::load(string name,string url){
	Fetcher::load(name,url);
	contour.clear();
	tata.clear();
	cost=cal_cost();
}

void Unitedfetcher::load(string name,Mat img){
	Fetcher::load(name,img);
	contour.clear();
	tata.clear();
	cost=cal_cost();
}

Mat Unitedfetcher::cal_cost(){
	int row=origin.rows,col=origin.cols;
	Mat gray(row,col,CV_32F);
	Mat lap(row,col,CV_32F);
	Mat mag(row,col,CV_32F);
	Mat dx(row,col,CV_32F);
	Mat dy(row,col,CV_32F);
	Mat c(row,col,CV_32FC(8));
	cvtColor(origin,gray,CV_BGR2GRAY);

	//计算拉普拉斯算子
	Laplacian(gray,lap,gray.depth());
	for(int i=0;i<col;i++){
		for(int j=0;j<row;j++){
			if(lap.at<uchar>(Point(i,j))!=0)
				lap.at<uchar>(Point(i,j))=1;
		}
	}

	Sobel(gray,dx,CV_32F,1,0);
	Sobel(gray,dy,CV_32F,0,1);

	//计算 Gradient magnitude
	assert(dx.type() == dy.type());
	assert(dx.depth() == dy.depth());
	assert(dx.size() == dy.size());
	assert(dx.depth() == CV_32F || dx.depth() == CV_64F);
	magnitude(dx, dy,mag);


	Mat t1(row,col,CV_8SC(8));
	Mat t2(row,col,CV_8SC(8));
	Sobel(gray,dx,CV_32F,2,0);
	Sobel(gray,dy,CV_32F,0,2);

	/*for(int i=0;i<col;i++){
		for(int j=0;j<row;j++){
			for(int k=0;k<8;k++){
				int m=-mm[k];
				int n=-nn[k];
				if((dy.at<float>(Point(i,j))*m-dx.at<float>(Point(i,j))*n)>=0){
					t1.at<Vec<char, 8>>(Point(i,j))[k]=m;
					t2.at<Vec<char, 8>>(Point(i,j))[k]=n;
				}
				else{
					t1.at<Vec<char, 8>>(Point(i,j))[k]=-m;
					t2.at<Vec<char, 8>>(Point(i,j))[k]=-n;
				}
			}
		}
	}

	for(int i=0;i<col;i++){
		for(int j=0;j<row;j++){
			for(int k=0;k<8;k++){
				int m=mm[k];
				int n=nn[k];
				if(i+m>=0&&i+m<col&&j+n>=0&&j+n<row)
					c.at<Vec<float,8>>(Point(i,j))[k]=1/pi*(cos(1/(dy.at<float>(Point(i,j))*t1.at<Vec<char, 8>>(Point(i,j))[k]-dx.at<float>(Point(i,j))*t2.at<Vec<char, 8>>(Point(i,j))[k]))
						+cos(1/(dy.at<float>(Point(i+m,j+n))*t1.at<Vec<char, 8>>(Point(i,j))[k]-dx.at<float>(Point(i+m,j+n))*t2.at<Vec<char, 8>>(Point(i,j))[k])));

			}
		}
	}

	for(int i=0;i<col;i++){
		for(int j=0;j<row;j++){
			for(int k=0;k<8;k++){
               cout<<c.at<Vec<float,8>>(i,j)[k]<<endl;
			}
		}
	}*/

	float max=*std::max_element(mag.begin<float>(),mag.end<float>());

	for(int i=0;i<col;i++){
		for(int j=0;j<row;j++){
			for(int k=0;k<8;k++){
				int m=mm[k];
				int n=nn[k];
				if(i+m>=0&&i+m<col&&j+n>=0&&j+n<row){
				    if(m*n==0)
					    c.at<Vec<float,8>>(Point(i,j))[k]=Wz*lap.at<uchar>(Point(i,j))+(1-1.0*mag.at<float>(Point(i,j))/max)/genhao2*Wg/*+Wd*c.at<Vec<float,8>>(Point(i,j))[k]*/;
				    else
					    c.at<Vec<float,8>>(Point(i,j))[k]=Wz*lap.at<uchar>(Point(i,j))+(1-1.0*mag.at<float>(Point(i,j))/max)*Wg/*+Wd*c.at<Vec<float,8>>(Point(i,j))[k]*/;
				}
			}
		}
	}

	return c;
}

double Unitedfetcher::Lcost(int x1,int y1,int x2,int y2){
	if(x1==x2&&y1==y2)
		return 0;
	if(abs(x1-x2)<=1&&abs(y1-y2)<=1){
		int m=x1-x2;
		int n=y1-y2;
		//assert(x1+m>=0&&x1+m<origin.cols&&y1+n>=0&&y1+n<origin.rows);
		if(m*n==0)
			return cost.at<Vec<float,8>>(Point(x1,y1))[table[m+1][n+1]];
		else
			return genhao2*cost.at<Vec<float,8>>(Point(x1,y1))[table[m+1][n+1]];
	}
	return -1;
}

static double getcost(Route a){
	return a.distance;
}

void Unitedfetcher::mouse(int event, int x, int y, int flags, void* param){
	if(event==CV_EVENT_LBUTTONDOWN && stat==IN_PROCESS){
		stat=IN_PROCESS;
		vector<Point>tmp=find_contour(mark[mark.size()-1].x,mark[mark.size()-1].y,x,y);
		contour.insert(contour.end(),tmp.begin(),tmp.end());
		mark.push_back(Point(x,y));
	}
	else if(event==CV_EVENT_LBUTTONDOWN && stat==NOT_SET){
		stat=IN_PROCESS;
		mark.push_back(Point(x,y));
	}
	else if(event==CV_EVENT_MOUSEMOVE &&stat==IN_PROCESS){
		tata=find_contour(mark[mark.size()-1].x,mark[mark.size()-1].y,x,y);
	}
}

void Unitedfetcher::show_image(){
	Mat rec(origin.clone());

	if(stat!=NOT_SET){
		vector<vector<Point>>tmp;
		tmp.push_back(contour);
		drawContours(rec,tmp,-1,RED, 2);
		tmp.clear();
		tmp.push_back(tata);
		drawContours(rec,tmp,-1,RED, 2);
		tmp.clear();
		for(int i=0;i<mark.size();i++)
			circle(rec,mark[i],1,BLUE,2);

	}

	imshow(winname,rec);
}

vector<Point> Unitedfetcher::get_contour(){
	if(stat==SET)
		return contour;
	else
		return  vector<Point>();
}

vector<Point> Unitedfetcher::fetch_ok(){
	stat=SET;
	return get_contour();
}

vector<Point> Unitedfetcher::find_contour(int x1,int y1,int x2,int y2){
	int col=origin.cols,row=origin.rows;
	Point first(x1,y1);
	Point dest(x2,y2);
	//初始化所需要的数据结构
	map<int,Route>ma;
	Point pre;
	priority_queue<Route, std::vector<Route>, Route_comp>que;
	vector<Point>path;
	double cost=0;
    //算法运行主段
	//if(first==dest)
		//return vector<Point>();
	while(first!=dest){
		if(ma.count(first.y*row+first.x)==0){
			Route rr={first,pre,cost};
			ma[first.y*row+first.x]=rr;
			for(int k=0;k<8;k++){
				int m=mm[k];
				int n=nn[k];
				int sfa=ma.count((first.y+n)*row+first.x+m);
				
				if(first.x+m>=0&&first.x+m<col&&first.y+n>=0&&first.y+n<row&&ma.count((first.y+n)*row+first.x+m)==0){
					Route rrr={Point(first.x+m,first.y+n),first,cost+Lcost(first.x,first.y,first.x+m,first.y+n)};
					que.push(rrr);
				}
			}
		}
		if(que.empty()){
			return vector<Point>();
		}
		Route rr=que.top();
		que.pop();
		first=rr.dot;
		cost=rr.distance;
		pre=rr.pre;

	}
	path.push_back(dest);
	while(ma.count(pre.y*row+pre.x)&&ma[pre.y*row+pre.x].distance!=0){
		path.push_back(pre);
		pre=ma[pre.y*row+pre.x].pre;
	}
	//path.push_back(pre);
	return path;

}

void Unitedfetcher::reset(){
	stat=NOT_SET;
	mark.clear();
	tata.clear();
	contour.clear();
};


Snakefetcher::Snakefetcher(){
}

Snakefetcher::Snakefetcher(string name,string url)
	:Fetcher(name,url){
}

Snakefetcher::Snakefetcher(string name,Mat img)
	:Fetcher(name,img){
}

Snakefetcher::~Snakefetcher(){}

vector<Point >Snakefetcher:: fetch_ok(){
	stat=SET;
	vector<Point>result= snake(get_contour());
	for(int i=0;i<snakeIter;i++)
		result=snake(result);
	return result;
}

void Snakefetcher::reset(){
	mark.clear();
	stat=NOT_SET;
}

void Snakefetcher::show_image(){
	Mat rec(origin.clone());

	if(stat!=NOT_SET){
		for(int i=0;i<mark.size()-1;i++)
			line(rec,mark[i],mark[i+1],RED,2);

	}

	imshow(winname,rec);
}

void Snakefetcher::mouse(int event, int x, int y, int flags, void* param){
	if(event==CV_EVENT_MOUSEMOVE&&(flags&CV_EVENT_FLAG_CTRLKEY)&&stat!=SET){
		stat=IN_PROCESS;
		mark.push_back(Point(x,y));
	}
}

vector<Point> Snakefetcher::get_contour(){
	int row=origin.rows,col=origin.cols;
	Mat rec(row,col,CV_8UC1);
	rec.setTo(Scalar::all(0));
	vector<vector<Point>>cts;
	assert(mark.size()>1);
	for(int i=0;i<mark.size()-1;i++)
	    line(rec,mark[i],mark[i+1],WHITE,2);
	line(rec,mark[mark.size()-1],mark[0],WHITE,2);
	//imshow("fdsa",rec);
	//waitKey();
	findContours(rec, cts, CV_RETR_EXTERNAL|RETR_LIST, CHAIN_APPROX_NONE);
	assert(cts.size()>1);
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

vector<Point> Snakefetcher::snake(vector<Point>contour){

 
    Mat gray;
	cvtColor(origin,gray,CV_BGR2GRAY);
	Canny(gray,gray,bigThre,smallThre);

	int length =contour.size();	
	CvPoint* point = new CvPoint[length]; //分配轮廓点
	for (int i = 0; i < length; i++)
	{
		point[i]=contour[i];
	}
	float alpha=10/100.0f; 
	static float beta=10/100.0f; 
	float gamma=80/100.0f; 
	CvSize size; 
	size.width=3; 
	size.height=3; 
	CvTermCriteria criteria; 
	criteria.type=CV_TERMCRIT_ITER; 
	criteria.max_iter=2000; 
	criteria.epsilon=0.1;
	auto sss=IplImage(gray);
	cvSnakeImage(&sss, point,length,&alpha,&beta,&gamma,CV_VALUE,size,criteria, 0);

	for(int i=0;i<length;i++){
		contour[i]=point[i];
	}
	int row=origin.rows,col=origin.cols;
	Mat rec(row,col,CV_8UC1);
	rec.setTo(Scalar::all(0));
	vector<vector<Point>>cts;
	assert(contour.size()>1);
	for(int i=0;i<contour.size()-1;i++)
	    line(rec,contour[i],contour[i+1],WHITE,2);
	line(rec,contour[contour.size()-1],contour[0],WHITE,2);
	findContours(rec, cts, CV_RETR_EXTERNAL|RETR_LIST, CHAIN_APPROX_NONE);
	assert(cts.size()>1);
	delete []point;
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
	//mark=cts[index2];
	return cts[index2];
}


Mattingfetcher::Mattingfetcher(){
}

Mattingfetcher::Mattingfetcher(string name,string url)
	:Fetcher(name,url){
}

Mattingfetcher::Mattingfetcher(string name,Mat img)
	:Fetcher(name,img){
}

Mattingfetcher::~Mattingfetcher(){}

void Mattingfetcher::reset(){
	stat=NOT_SET;
	mark.clear();
}

void Mattingfetcher::show_image(){
	Mat rec(origin.clone());
	rectangle(rec,rect,BLUE);
	if(stat!=NOT_SET){
		rectangle(rec,rect,BLUE);
		for(int i=0;i<mark.size();i++)
			circle(rec,mark[i],radius,RED,thickness);

	}

	imshow(winname,rec);
}

vector<Point> Mattingfetcher::fetch_ok(){
	stat=SET;
	return get_contour();
}
void Mattingfetcher::set_rect(Rect rec){
	stat=IN_PROCESS;
	rect=rec;
}

void Mattingfetcher::mouse(int event ,int x,int y,int flags,void *param){
	if(event==CV_EVENT_LBUTTONDOWN&&stat==NOT_SET)
		rect=Rect(x,y,1,1);
	else if(event==CV_EVENT_LBUTTONUP&&stat==NOT_SET){
		stat=IN_PROCESS;
		rect = Rect( Point(rect.x, rect.y), Point(x,y) );
	}
	else if(event==CV_EVENT_MOUSEMOVE&&stat==NOT_SET&&flags==CV_EVENT_FLAG_LBUTTON)
		rect = Rect( Point(rect.x, rect.y), Point(x,y) );
	else if(event==CV_EVENT_LBUTTONDOWN&&stat==IN_PROCESS)
		mark.push_back(Point(x,y));
	else if(event==CV_EVENT_MOUSEMOVE&&stat==IN_PROCESS&&flags==CV_EVENT_FLAG_LBUTTON)
		mark.push_back(Point(x,y));
}

vector<Point> Mattingfetcher::get_contour(){
	Mat mask(origin.rows,origin.cols,CV_8UC1);
	mask.setTo(Scalar::all(GC_BGD));
	(mask(rect)).setTo( Scalar(GC_PR_FGD) );
	for(int i=0;i<mark.size();i++)
		mask.at<uchar>(mark[i])=GC_FGD;
	Mat bgdModel, fgdModel;
	grabCut(origin, mask, rect, bgdModel,fgdModel, grabIter, GC_INIT_WITH_MASK|GC_INIT_WITH_RECT );

	Mat rec;
	mask&=1;
	origin.copyTo(rec,mask);
	imshow(winname,rec);
	waitKey();
	for(int i=0;i<rec.rows;i++){
		for(int j=0;j<rec.cols;j++){
			if(rec.at<Vec3b>(Point(j,i))[0]==0&&rec.at<Vec3b>(Point(j,i))[1]==0&&rec.at<Vec3b>(Point(j,i))[2]==0){
				rec.at<Vec3b>(Point(j,i))[0]=255;
				rec.at<Vec3b>(Point(j,i))[1]=255;
				rec.at<Vec3b>(Point(j,i))[2]=255;
			}
		}
	}
	Mat gray(origin.rows, origin.cols,CV_8UC1);
	cvtColor(rec,gray,CV_BGR2GRAY);
	equalizeHist( gray, gray );

	threshold(gray, gray, 0, 255, THRESH_BINARY | THRESH_OTSU);

	Mat tmp2(gray.rows+20,gray.cols+20,gray.depth());
	copyMakeBorder(gray,tmp2,10,10,10,10,BORDER_ISOLATED,Scalar(0xff,0xff,0xff,0));
	vector<vector<Point>> cts;
	findContours(tmp2, cts, CV_RETR_EXTERNAL|RETR_LIST, CHAIN_APPROX_NONE);
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