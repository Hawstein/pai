#ifndef HACKDAY_FILTER_H_
#define HACKDAY_FILTER_H_

#include "opencv2/core/core.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <iterator>
#include <cmath>
#include <cstring>

using namespace std;
using namespace cv;

namespace hackday {

const int COLOR_COUNT = 11;

struct Data {
	string videoId;
	string md5;
	string mainColorFeature;
	string pHashFeature;
	int pHashDistance;
	Data(string id, string md5, string mc, string phash): videoId(id), md5(md5), mainColorFeature(mc), pHashFeature(phash), pHashDistance(0) {}
};

struct Hsl {
    double h, s, l;
};

inline bool cmp(Data d1, Data d2) {
	return d1.pHashDistance < d2.pHashDistance;
}

class Filter {
public:
	static void LoadData(const string data_name, vector<Data> &dataset);
	static map<string, string> GetCandidates(const char* pic_path, const vector<Data> &dataset);
	static void WriteData(const string data_name);
	static string PHashValue(Mat &src);
	static int HanmingDistance(string &str1,string &str2);
	static Hsl GetHsl(int R, int G, int B);
	static int GetColorWeight(Hsl hslcolor);
	static string GetMainColor(Mat &src);
	static string GetVideoId(string pic_name);
	static string GetMD5(string pic_name);
};

} 

#endif  // HACKDAY_FILTER_H_






