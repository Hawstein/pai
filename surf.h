#ifndef HACKDAY_SURF_H_
#define HACKDAY_SURF_H_

#include <ctime>
#include <vector>
#include <map>
#include <string>

#include "mongoose.h"
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"

namespace hackday {

static const int kVideo = 0;
static const int kEbook = 1;

struct SURFFeature {
  std::vector<cv::KeyPoint> key_points;
  cv::Mat descriptor;
  cv::Mat image;
  cv::Rect roi;
};

class Timer {
 public:
  Timer() : start_(0) {}
  void Start();
  void Stop();
  void Restart();
  long elapsed_millis();
 private:
  clock_t start_;
};

typedef std::map<int, std::map<std::string, SURFFeature*> > FeatureMap;

class SURFFeatureManager {
 public:
  bool LoadFeatureSet();
  bool CalculateFeatureSet(const int media_type,
                           const std::string& image_folder);
  bool GetFeature(const std::string& image_path, SURFFeature *feature);
  bool MatchFeature(const SURFFeature& query, const SURFFeature& train);
  bool FindMatchFeature(int media_type,
                        const SURFFeature& train,
                        const std::map<std::string, std::string>& range,
                        std::vector<std::string> *ids);
  void PrintFeature(const SURFFeature& feature);
 private:
  FeatureMap feature_map_;
};

void StartHTTPServer(mg_handler_t handler);

}

#endif  // HACKDAY_SURF_H_
