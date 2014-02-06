#ifndef HACKDAY_SURF_H_
#define HACKDAY_SURF_H_

#include <vector>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"

namespace hackday {

struct SURFFeature {
  std::vector<cv::KeyPoint> key_points;
  cv::Mat descriptor;
  cv::Rect roi;
};

void GetFeature(const std::string& image_path, SURFFeature* feature);

}

#endif  // HACKDAY_SURF_H_
