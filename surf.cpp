#include "surf.h"

#include <cstdio>
#include <cstdlib>

#include "opencv2/core/core.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"

namespace hackday {

static const int kMinHessian = 400;

static const int kMinDistanceBetweenPoints = 3;

static bool IsQuadrangle(cv::Point2f p1,
                         cv::Point2f p2,
                         cv::Point2f p3,
                         cv::Point2f p4);

static bool IsOverlapped(cv::Point2f p1, cv::Point2f p2);

bool GetFeature(const std::string& image_path,
                SURFFeature *feature) {
  feature->image = cv::imread(image_path);
  if (!feature->image.data) return false;

  // Detect key points
  cv::SurfFeatureDetector(kMinHessian).detect(feature->image, feature->key_points);

  // Extract descriptor
  cv::SurfDescriptorExtractor().compute(feature->image,
                                        feature->key_points,
                                        feature->descriptor);

  // Set ROI
  cv::Size size = feature->image.size();
  feature->roi.x = 0;
  feature->roi.y = 0;
  feature->roi.width = size.width;
  feature->roi.height = size.height;
  return true;
}

bool MatchFeature(const SURFFeature& query_feature,
                  const SURFFeature& train_feature) {
  std::vector<cv::DMatch> matches;
  cv::FlannBasedMatcher matcher;
  matcher.match(query_feature.descriptor, train_feature.descriptor, matches);

  double max_dist = 0;
  double min_dist = 100;

  // Quick calculation of max and min distances between keypoints
  for (int i = 0; i < query_feature.descriptor.rows; i++) {
    double dist = matches[i].distance;
    if (dist < min_dist) min_dist = dist;
    if (dist > max_dist) max_dist = dist;
  }

  // Draw only "good" matches (i.e. whose distance is less than 3*min_dist)
  std::vector<cv::DMatch> good_matches;
  for (int i = 0; i < query_feature.descriptor.rows; i++) {
    if (matches[i].distance < 3 * min_dist) good_matches.push_back(matches[i]);
  }

  // cv::Mat img_matches;
  // drawMatches(query_feature.image,
  //             query_feature.key_points,
  //             train_feature.image,
  //             train_feature.key_points,
  //             good_matches,
  //             img_matches,
  //             cv::Scalar::all(-1),
  //             cv::Scalar::all(-1),
  //             std::vector<char>(),
  //             cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

  // Localize the object
  std::vector<cv::Point2f> src_points;
  std::vector<cv::Point2f> dst_points;

  for(int i = 0; i < good_matches.size(); i++)
  {
    // Get the keypoints from the good matches
    src_points.push_back(query_feature.key_points[good_matches[i].queryIdx].pt);
    dst_points.push_back(train_feature.key_points[good_matches[i].trainIdx].pt);
  }

  cv::Mat homography = findHomography(src_points, dst_points, CV_FM_RANSAC);

  // Get the corners from the query_image (the object to be "detected")
  std::vector<cv::Point2f> query_image_corners(4);
  query_image_corners[0] = cvPoint(0,0);
  query_image_corners[1] = cvPoint(query_feature.image.cols, 0);
  query_image_corners[2] = cvPoint(query_feature.image.cols,
                                   query_feature.image.rows);
  query_image_corners[3] = cvPoint(0, query_feature.image.rows);
  std::vector<cv::Point2f> train_image_corners(4);

  perspectiveTransform(query_image_corners, train_image_corners, homography);

  for (std::vector<cv::Point2f>::iterator it = train_image_corners.begin();
       it != train_image_corners.end();
       ++it) {
    printf("Corner: <%f %f>\n", it->x, it->y);
  }

  return IsQuadrangle(train_image_corners[0],
                      train_image_corners[1],
                      train_image_corners[2],
                      train_image_corners[3]);
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

static bool IsQuadrangle(cv::Point2f p1,
                         cv::Point2f p2,
                         cv::Point2f p3,
                         cv::Point2f p4) {
  return !(IsOverlapped(p1, p2) ||
           IsOverlapped(p1, p3) ||
           IsOverlapped(p1, p4) ||
           IsOverlapped(p2, p3) ||
           IsOverlapped(p2, p4) ||
           IsOverlapped(p3, p4));
}

static bool IsOverlapped(cv::Point2f p1, cv::Point2f p2) {
  return abs(((int)p1.x - (int)p2.x)) <= kMinDistanceBetweenPoints &&
    abs(((int)p1.y - (int)p2.y)) <= kMinDistanceBetweenPoints;
}

}
