#include "surf.h"
#include "opencv2/core/core.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"

namespace hackday {

static const int kMinHessian = 400;

void GetFeature(const std::string& image_path,
                SURFFeature *feature) {
  cv::Mat image_mat = cv::imread(image_path);
  cv::SurfFeatureDetector detector(kMinHessian);
  detector.detect(image_mat, feature->key_points);
  cv::SurfDescriptorExtractor extractor;
  extractor.compute(image_mat, feature->key_points, feature->descriptor);
  cv::Size size = image_mat.size();
  feature->roi.x = 0;
  feature->roi.y = 0;
  feature->roi.width = size.width;
  feature->roi.height = size.height;
}

}
