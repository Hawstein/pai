#include "surf.h"

#include <sys/stat.h>
#include <sys/dir.h>
#include <dirent.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "mongoose.h"
#include "opencv2/core/core.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"

namespace hackday {

static const int kMinHessian = 400;
static const int kMinDistanceBetweenPoints = 3;
static const double kResizeFactor = 0.6;
static const int kMaxWidth = 200;
static const int kMaxHeight = 120;
static const int kOrginMaxWidth = (int) kMaxWidth / kResizeFactor;
static const int kOrginMaxHeight = (int) kMaxHeight / kResizeFactor;
static const char *kHTTPServerPort = "8090";
static const char *kDataFolder = "/tmp/feature_set";

static void SplitFilename(const std::string& filename,
                          std::vector<std::string> *arr);

static void ResizeImage(const cv::Mat& src_image, cv::Mat& dst_image);

static bool IsQuadrangle(cv::Point2f p1,
                         cv::Point2f p2,
                         cv::Point2f p3,
                         cv::Point2f p4);

static bool IsOverlapped(cv::Point2f p1, cv::Point2f p2);


void Timer::Start() {
  start_ = clock();
}

void Timer::Stop() {
  start_ = 0;
}

void Timer::Restart() {
  Start();
}

long Timer::elapsed_millis() {
  return (clock() - start_) * 1000 / CLOCKS_PER_SEC;
}

void StartHTTPServer(mg_handler_t handler) {
  struct mg_server *server = mg_create_server(NULL);
  mg_set_option(server, "listening_port", kHTTPServerPort);
  mg_set_request_handler(server, handler);
  for (;;) mg_poll_server(server, 1000);  // Infinite loop, Ctrl-C to stop
  mg_destroy_server(&server);
}

bool SURFFeatureManager::FindMatchFeature(
    int media_type,
    const SURFFeature& train,
    const std::map<std::string, std::string>& range,
    SURFFeature *matched,
    std::string *id) {
  FeatureMap::iterator fit = feature_map_.find(media_type);
  if (fit == feature_map_.end()) return false;
  std::map<std::string, SURFFeature*>& features = fit->second;
  for (std::map<std::string, std::string>::const_iterator it = range.begin();
       it != range.end();
       ++it) {
    std::map<std::string, SURFFeature*>::iterator sit = features.find(it->first);
    if (sit == features.end()) return false;
    SURFFeature *feature = sit->second;
    if (MatchFeature(*feature, train)) {
      matched = feature;
      *id = it->second;
      return true;
    }
  }
  return false;
}

bool SURFFeatureManager::LoadFeatureSet() {
  DIR *dir = opendir(kDataFolder);
  struct dirent *dir_ent = NULL;
  char folder[64];
  char filepath[128];
  while ((dir_ent = readdir(dir)) != NULL) {
    if (strcmp(".", dir_ent->d_name) == 0 ||
        strcmp("..", dir_ent->d_name) == 0) continue;
    sprintf(folder, "%s/%s", kDataFolder, dir_ent->d_name);
    printf("In folder %s\n", folder);
    DIR *category_dir = opendir(folder);
    struct dirent *file_ent = NULL;
    while ((file_ent = readdir(category_dir)) != NULL) {
      if (strcmp(".", file_ent->d_name) == 0 ||
          strcmp("..", file_ent->d_name) == 0) continue;
      sprintf(filepath, "%s/%s", folder, file_ent->d_name);
      std::vector<std::string> arr;
      SplitFilename(file_ent->d_name, &arr);
      cv::FileStorage fs(filepath, cv::FileStorage::READ);
      SURFFeature *feature = new SURFFeature;
      fs["D"] >> feature->descriptor;
      fs["I"] >> feature->image;
      cv::FileNode roi_node = fs["R"];
      feature->roi.x = roi_node["x"];
      feature->roi.y = roi_node["y"];
      feature->roi.width = roi_node["width"];
      feature->roi.height = roi_node["height"];
      cv::FileNode kp_node = fs["K"];
      for (cv::FileNodeIterator fit = kp_node.begin();
           fit != kp_node.end();
           ++fit) {
        cv::FileNode kp = *fit;
        cv::KeyPoint point;
        point.pt.x = kp["x"];
        point.pt.y = kp["y"];
        point.size = kp["size"];
        point.angle = kp["angle"];
        point.octave = kp["octave"];
        point.response = kp["response"];
        point.class_id = kp["class_id"];
        feature->key_points.push_back(point);
        int media_type = atoi(dir_ent->d_name);
        FeatureMap::iterator it;
        if ((it = feature_map_.find(media_type)) == feature_map_.end()) {
          feature_map_.insert(make_pair(media_type,
                                        std::map<std::string, SURFFeature*>()));
        } else {
          it->second.insert(make_pair(arr[1], feature));
        }
      }
    }
  }
  for (FeatureMap::iterator it = feature_map_.begin();
       it != feature_map_.end();
       ++it) {
    printf("%d: %lu\n", it->first, it->second.size());
  }
  return true;
}

bool SURFFeatureManager::CalculateFeatureSet(const int media_type,
                                             const std::string& image_folder) {
  std::string folder = image_folder;
  if (image_folder[image_folder.size() - 1] != '/') {
    folder += "/";
  }
  DIR *dir = opendir(folder.c_str());
  struct dirent *ent = NULL;
  while ((ent = readdir(dir)) != NULL) {
    if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0) continue;
    std::vector<std::string> arr;
    SplitFilename(ent->d_name, &arr);
    SURFFeature *feature = new SURFFeature;
    GetFeature(folder + ent->d_name, feature);

    char data_folder[64];
    sprintf(data_folder, "%s/%d", kDataFolder, media_type);
    mkdir(kDataFolder, S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir(data_folder, S_IRWXU | S_IRWXG | S_IRWXO);
    char filepath[128];
    sprintf(filepath, "%s/%s_%s_.yaml",
            data_folder, arr[0].c_str(), arr[1].c_str());
    cv::FileStorage fs(filepath,
                       cv::FileStorage::WRITE);
    fs << "D" << feature->descriptor;
    fs << "I" << feature->image;
    fs << "K" << "[";
    for (std::vector<cv::KeyPoint>::iterator it = feature->key_points.begin();
         it != feature->key_points.end();
         ++it) {
      fs << "{"
         << "x" << it->pt.x
         << "y" << it->pt.y
         << "size" << it->size
         << "angle" << it->angle
         << "response" << it->response
         << "octave" << it->octave
         << "class_id" << it->class_id
         << "}";
    }
    fs << "]";
    fs << "R" << "{"
       << "x" << feature->roi.x
       << "y" << feature->roi.y
       << "width" << feature->roi.width
       << "height" << feature->roi.height
       << "}";
    fs.release();
    FeatureMap::iterator it;
    if ((it = feature_map_.find(media_type)) == feature_map_.end()) {
      feature_map_.insert(make_pair(
        media_type,
        std::map<std::string, SURFFeature*>()));
    } else {
      it->second.insert(make_pair(arr[1], feature));
    }
  }
  return true;
}

bool SURFFeatureManager::GetFeature(const std::string& image_path,
                                    SURFFeature *feature) {
  cv::Mat original_image = cv::imread(image_path, CV_LOAD_IMAGE_GRAYSCALE);
  if (!original_image.data) return false;

  ResizeImage(original_image, feature->image);

  // Detect key points
  cv::SurfFeatureDetector(kMinHessian).detect(feature->image,
                                              feature->key_points);
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

bool SURFFeatureManager::MatchFeature(const SURFFeature& query_feature,
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

  // Two images are same
  if (good_matches.empty()) return true;

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

void SURFFeatureManager::PrintFeature(const SURFFeature& feature) {
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

static void ResizeImage(const cv::Mat& src_image,
                        cv::Mat& dst_image) {
  if (src_image.cols <= kMaxWidth && src_image.rows <= kMaxHeight) {
    dst_image = src_image;
    return;
  }
  if (src_image.cols <= kOrginMaxWidth && src_image.rows <= kOrginMaxHeight) {
    cv::resize(src_image, dst_image, cv::Size(), kResizeFactor, kResizeFactor);
    return;
  }
  double width_factor = kMaxWidth / (double)src_image.cols;
  double height_factor = kMaxHeight / (double)src_image.rows;
  double factor = std::min(width_factor, height_factor);
  cv::resize(src_image, dst_image, cv::Size(), factor, factor);
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

static void SplitFilename(const std::string& filename,
                          std::vector<std::string> *arr) {
  std::vector<std::string>::size_type start_pos = 0, find_pos;
  while ((find_pos = filename.find("_", start_pos)) != -1) {
    arr->push_back(filename.substr(start_pos, find_pos - start_pos));
    start_pos = find_pos + 1;
  }
}

}
