#include "filter.h"

using namespace std;

namespace hackday {

//////////// 感知哈希算法 ////////////
//pHash算法
string Filter::PHashValue(Mat &src) {
	Mat clone = src.clone();
	Mat img, dst;
	if (clone.channels() == 3) {
		cvtColor(clone, clone, CV_BGR2GRAY);
		img = Mat_<double>(clone);
	} else {
		img = Mat_<double>(clone);
	}     
	 
    /* 第一步，缩放尺寸，可以为Size(32,32)或Size(8,8)，也可以更高，主要是为了提高计算效率*/
	resize(img, img, Size(32,32));

    // 顺时针旋转图像90度
    transpose(img, dst);  
    flip(dst, img, 1);
        
    /* 第二步，离散余弦变换，DCT系数求取*/
	dct(img, dst);	

    /* 第三步，求取DCT系数均值（左上角8*8区块的DCT系数，频率最低的部分，即图像信息的大部分）*/
    double dIdex[64];
    double mean = 0.0;
    int k = 0;
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			dIdex[k] = dst.at<double>(i, j);
			mean += dst.at<double>(i, j)/64;
			++k;
		}
	}
        
    /* 第四步，计算哈希值。*/
    string rst(64,' ');
	for (int i = 0; i < 64; ++i) {
		if (dIdex[i] >= mean) {
			rst[i] = '1';
		} else {
			rst[i] = '0';
		}
	}
	return rst;
}

//汉明距离计算
int Filter::HanmingDistance(string &str1,string &str2) {
 	if ((str1.size()!=64) || (str2.size()!=64))
 		return -1;
 	int difference = 0;
 	for (int i = 0; i < 64; ++i) {
 		if(str1[i] != str2[i])
 			difference++;
 	}
 	return difference;
}

Hsl Filter::GetHsl(int R, int G, int B) {
    Hsl temp;
    double r = R / 255.0;
    double g = G / 255.0;
    double b = B / 255.0;
    double max, min;
    temp.h = 0;
    temp.s = 0;
    temp.l = 0;
    max = std::max(r, std::max(g, b));
    min = std::min(r, std::min(g, b));
    temp.l = (max + min) / 2.0;
    if (max == min) {
        temp.s = 0;
        temp.h = 0;
        return temp;
    }
    if (temp.l < 0.5) {
        temp.s = (max - min) / (max + min);
    }
    else {
        temp.s = (max - min) / (2 - max - min);
    }
    if (r == max) {
        temp.h = (g - b) / (max - min);
    }
    else if (g == max) {
        temp.h = 2 + (b - r) / (max - min);
    }
    else if (b == max) {
        temp.h = 4 + (r - g) / (max - min);
    }
    temp.h = temp.h * 60;
    if (temp.h < 0) {
        temp.h += 360;
    }
    return temp;
}

int Filter::GetColorWeight(Hsl hslcolor) {
    int weight = -1;
    double H = hslcolor.h;
    double L = hslcolor.l;
    double S = hslcolor.s;

    if (L <= 0.20) {
        weight = 9; //black
    }
    else if (L >= 0.75 && S <= 0.2) {
        weight = 10; //white
    }
    else if (S <= 0.1) {
        weight = 8; //gray
    }
    else if ((H > 330 && H <= 360) || (H >= 0 && H <= 25)) {
        weight = 0;
    }
    else if (H > 25 && H <= 41) {
        weight = 1;
    }
    else if (H > 41 && H <= 75) {
        weight = 2;
    }
    else if (H > 75 && H <= 156) {
        weight = 3;
    }
    else if (H > 156 && H <= 201) {
        weight = 4;
    }
    else if (H > 201 && H <= 262) {
        weight = 5;
    }
    else if (H > 262 && H <= 285) {
        weight = 6;
    }
    else if (H > 285 && H <= 330) {
        weight = 7;
    }
    return weight;
}

string Filter::GetMainColor(Mat &src) {
	Mat img = src.clone();
	resize(img, img, Size(32, 32));
	int row = img.rows;
	int col = img.cols;
	int b = 0, g = 0, r = 0;
	uchar* pImg = img.data;
	int weights[COLOR_COUNT];
	memset(weights, 0, sizeof(weights));
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			b = (int)pImg[3*j];
			g = (int)pImg[3*j + 1];
			r = (int)pImg[3*j + 2];
			//cout<<(int)b<<" "<<(int)g<<" "<<(int)r<<endl;
			Hsl hsl = GetHsl(r, g, b);
			weights[GetColorWeight(hsl)]++;
		}
		pImg += img.step;
	}
	// get 2 main color except black, white, gray
	int max = 0, secondmax = 1;
	if (weights[1] > weights[0]) {
		max = 1;
		secondmax = 0;
	}
	for (int i = 2; i < COLOR_COUNT-2; ++i) {
		if (weights[i] > weights[secondmax]) {
			if (weights[i] > weights[max]) {
				secondmax = max;
				max = i;
			}
			else {
				secondmax = i;
			}
		}
	}
	if (secondmax < max) {
		int tmp = max;
		max = secondmax;
		secondmax = tmp;
	}
	string res = "";
    res = res + char(max + '0');
    res = res + char(secondmax + '0');
    return res;
}


// get picture path : 
// ls | sed "s:^:`pwd`/: " > ../coverlists.txt
string Filter::GetVideoId(string pic_name) {
	int pos = pic_name.find('_');
	return pic_name.substr(0, pos);
}

string Filter::GetMD5(string pic_name) {
	int pos1 = pic_name.find('_');
	int pos2 = pic_name.rfind('_');
	return pic_name.substr(pos1+1, pos2-pos1-1);
}

void Filter::WriteData(const string data_name) {
	ifstream in("coverlists.txt");  
	ofstream out(data_name.c_str());
    if (!in.is_open()) { 
    	cout<<"Error opening file"<<endl; 
		return; 
    }  
    string buffer;
    int num = 1;
    while (getline(in, buffer)) {
    	string id = GetVideoId(buffer);
    	string md5 = GetMD5(buffer);
    	buffer = "./movie/" + buffer;
    	Mat img = imread(buffer.c_str(), 1);
		string main_color = GetMainColor(img);
    	string phash_img = PHashValue(img);
    	cout<<num<<endl;
    	++num;
    	out<<id<<" "<<md5<<" "<<main_color<<" "<<phash_img<<endl;
    }
    in.close();
    out.close();
}

void Filter::LoadData(const string data_name, vector<Data> &dataset) {
	ifstream in(data_name.c_str()); 
	string line;
	int num = 1;
	while (getline(in, line)) {
		istringstream iss(line);
		vector<string> tokens;
		copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter<vector<string> >(tokens));
		dataset.push_back(Data(tokens[0], tokens[1], tokens[2], tokens[3]));
		cout<<num<<endl;
		++num;
		// test
		// cout<<tokens[0]<<" "<<tokens[1]<<" "<<tokens[2]<<" "<<tokens[3]<<endl;
	}

	in.close();
}

map<string, string> Filter::GetCandidates(const char* pic_path, const vector<Data> &dataset) {
	// Mat img = imread(pic_path);
	// string main_color = GetMainColor(img);
 //    string phash_img = PHashValue(img);
 //    // main color
 //    vector<Data> subSet;
 //    for (int i=0; i<dataset.size(); ++i) {
 //    	Data d = dataset[i];
 //    	if(d.mainColorFeature == main_color) {
 //    		subSet.push_back(d);
 //    	}
 //    }
 //    // calculate pHash distance
 //    for (int i=0; i<subSet.size(); ++i) {
 //    	subSet[i].pHashDistance = HanmingDistance(subSet[i].pHashFeature, phash_img);
 //    }
 //    // sort by pHash distance
 //    sort(subSet.begin(), subSet.end(), cmp);
 //    // return top 1000
 //    map<string, string> cadidates;
 //    for (int i=0; i<1000 && i<subSet.size(); ++i) {
 //    	cadidates[subSet[i].md5] = subSet[i].videoId;
 //    	cout<<subSet[i].md5<<" "<<subSet[i].videoId<<endl;
 //    }

    
    map<string, string> cadidates;
    for (int i=0; i<dataset.size(); ++i) {
        cadidates[dataset[i].md5] = dataset[i].videoId;
    }
    return cadidates;
}

}