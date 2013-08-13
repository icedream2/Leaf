#ifndef FETCHER_H
#define FETCHER_H
#include <opencv2/imgproc/imgproc.hpp>
#include<vector>
#include<string>
namespace Leaf{
	//~!!!!!!!!!!!!!!!!!!!!!!!目前使用snakefetcher
class Fetcher{
public:
	enum Status { NOT_SET = 0, IN_PROCESS = 1, SET = 2 };
    static const int radius = 2;
    static const int thickness = -1;
	/*以下函数使用方法，开始先load一个参数是窗口名，不要show_img随便写就好。
	 *然后set_contour然后fetch_ok直接返回精确的轮廓
	 *reset可以重置*/
	Fetcher();
	Fetcher(std::string,cv::Mat);
	Fetcher(std::string,std::string);
	virtual ~Fetcher();
	virtual void load(std::string,std::string) ;
	virtual void load(std::string,cv::Mat)  ;
	virtual void set_contour(std::vector<cv::Point>);
	virtual void reset() =0;
	virtual std::vector<cv::Point> fetch_ok()=0;

	virtual void show_image() =0;
	virtual void mouse( int event, int x, int y, int flags, void* param) =0;
	virtual void wait();
protected:
	Status stat;
	cv::Mat origin;
    std::string winname;
	std::vector<cv::Point>mark;
	virtual std::vector<cv::Point> get_contour()=0;


};

class Unitedfetcher:public Fetcher{
public:
	Unitedfetcher();
	Unitedfetcher(std::string,std::string);
	Unitedfetcher(std::string,cv::Mat);
	virtual ~Unitedfetcher() ;
	virtual void load(std::string,std::string) override;
	virtual void load(std::string,cv::Mat ) override;
	virtual void reset() override;
	virtual void show_image() override;
	virtual std::vector<cv::Point> fetch_ok()override;
	virtual void mouse( int event, int x, int y, int flags, void* param) override;
private:
	std::vector<cv::Point> find_contour(int,int,int,int);
	virtual std::vector<cv::Point> get_contour() override;
	cv::Mat cal_cost();
	void build_path(int,int);
	double Lcost(int,int,int,int);
	std::vector<cv::Point>contour;
	std::vector<cv::Point>tata;
	cv::Mat cost; 
};

class Snakefetcher:public Fetcher{
public:
	Snakefetcher();
	Snakefetcher(std::string,std::string);
	Snakefetcher(std::string,cv::Mat);
	virtual ~Snakefetcher() ; 

	virtual void reset() override;
	virtual void show_image() override;
	virtual std::vector<cv::Point> fetch_ok()override;
	virtual void mouse( int event, int x, int y, int flags, void* param) override;

private:
	virtual std::vector<cv::Point> get_contour() override;
	std::vector<cv::Point> snake(std::vector<cv::Point>);
	int x,y;
};
 
class Mattingfetcher:public Fetcher{
public:
	Mattingfetcher();
	Mattingfetcher(std::string,std::string);
	Mattingfetcher(std::string,cv::Mat);

	virtual ~Mattingfetcher() ;
	virtual void reset() override;
	virtual void show_image() override;
	virtual std::vector<cv::Point> fetch_ok()override;
	virtual void mouse( int event, int x, int y, int flags, void* param) override;
	void set_rect(cv::Rect rect);
private:
	virtual std::vector<cv::Point> get_contour() override;
	cv::Rect rect;
	//Mat bgdModel, fgdModel;
};
	

}

#endif