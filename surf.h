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
  cv::Mat image;
  cv::Rect roi;
};

bool GetFeature(const std::string& image_path, SURFFeature* feature);

bool MatchFeature(const SURFFeature& query_feature,
                  const SURFFeature& train_feature);

void PrintFeature(const SURFFeature& feature);

}

#endif  // HACKDAY_SURF_H_
