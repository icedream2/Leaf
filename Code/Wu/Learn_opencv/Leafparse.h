#ifndef LEAFPARSE_H
#define LEAFPARSE_H
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <string>

namespace Leaf {
  std::vector<cv::Point> limit_contour(const cv::Mat& image,const std::vector<cv::Point>& contour);

  bool is_leaf(const cv::Mat& image,const std::vector<cv::Point>& contour);

  std::vector<cv::Point > refine_contour(const cv::Mat& image,const std::vector<cv::Point>& contour);

  std::vector<double> get_feature(const cv::Mat& image,const std::vector<cv::Point>& contour);
};

#endif