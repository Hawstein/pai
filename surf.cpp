#include "surf.h"

#include <cstdio>

#include "opencv2/core/core.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"

namespace hackday {

static const int kMinHessian = 400;

void GetFeature(const std::string& image_path,
                SURFFeature *feature) {
  cv::Mat image_mat = cv::imread(image_path);

  // Detect key points
  cv::SurfFeatureDetector(kMinHessian).detect(image_mat, feature->key_points);

  // Extract descriptor
  cv::SurfDescriptorExtractor().compute(image_mat,
                                        feature->key_points,
                                        feature->descriptor);

  // Set ROI
  cv::Size size = image_mat.size();
  feature->roi.x = 0;
  feature->roi.y = 0;
  feature->roi.width = size.width;
  feature->roi.height = size.height;
}

void PrintFeature(const SURFFeature& feature) {
  printf("KeyPoints:\n");
  for (std::vector<cv::KeyPoint>::const_iterator it = feature.key_points.begin();
       it != feature.key_points.end();
       it++) {
    printf("size: %f, x: %f, y: %f\n", it->size, it->pt.x, it->pt.y);
  }
  printf("\nMatrix:\n");
  const cv::Mat *mat = &feature.descriptor;
  switch(mat->channels()) {
  case 1:
    for (cv::MatConstIterator_<uchar> it = mat->begin<uchar>();
         it != mat->end<uchar>();
         it++) {
      printf("%d ", *it);
    }
    break;
  case 3:
    for (cv::MatConstIterator_<cv::Vec3b> it = mat->begin<cv::Vec3b>();
         it != mat->end<cv::Vec3b>();
         ++it) {
      printf("<%d %d %d> ",
             (*it)[0],
             (*it)[1],
             (*it)[2]);
    }
  }
  printf("\n\nWidth: %d, Height: %d\n", feature.roi.width, feature.roi.height);
}

}
