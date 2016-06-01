#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
using std::ofstream;
using std::ifstream;
using std::ios_base;
using std::endl;

using std::string;
using std::vector;
using std::map;
using std::cout;



#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

using cv::Mat;

#define cvGetHistValue_1D( hist, idx0 )     ((float*)(cvPtr1D( (hist)->bins, (idx0), 0 ))) 

#define cvQueryHistValue_1D( hist, idx0 ) \
	((float)cvGetReal1D( (hist)->bins, (idx0)))


typedef struct _Location{
	_Location(){
		_start = 0;
		_end = 0;
	}

	unsigned int _start;
	unsigned int _end;

	void clear(){
		_start = 0;
		_end = 0;
	}

}Location;

typedef struct _Image{
	_Image(){
		_pData = nullptr;
		_width = 0;
		_height = 0;
	}

	unsigned char *_pData;
	unsigned int  _width;
	unsigned int  _height;
}Image;

#define SAMPLE_COUNT      36
#define SAMPLE_SON		  5
#define MAX_SAMPLES      (SAMPLE_COUNT * SAMPLE_SON)
#define MAX_TRAIN_COLS	  256
#define MAX_OBJ_COLS      36
class CImageProcess
{
public:
	CImageProcess(string sName);
	~CImageProcess(void);

	void ProcessImage();
	void Sample(string path, float (*trainData)[MAX_TRAIN_COLS], int row);
	void Sample(string path, float *obj, int cols);
private:
	float	m_trainImput[MAX_SAMPLES][MAX_TRAIN_COLS];
	float   m_trainObj[MAX_SAMPLES][MAX_OBJ_COLS];

	string m_sName;
	unsigned char *pSwellTemp;
	unsigned char* HorizontalSwell(char *pIn, int iWidth, int iHeight, int n);
	void VerticalSwell(char *pIn, int iWidth, int iHeight, int n);
	void OpencvSwell(IplImage *pIn, IplImage *pOut);
	void VerticalProjection(char *pIn, int iWidth, int iHeight, unsigned int *pOut);
	void HorizontalProjection(char *pIn, int iWidth, int iHeight, unsigned int *pOut);
	void Thinning(Image &source, Image &dst);
	void ThinningIteration(cv::Mat &img, int iter);
	vector<Location> Projection_Location(unsigned int *pValue, int iVlen, int iMax);
	void CutImage(Image &source, vector<Location> x, vector<Location> y, vector<Image> *dst);
	void CopyImage(Image &source, Image &dest, Location x, Location y);
	IplImage* CreateImage(Image &image, const char* cName=nullptr);
	void ReleaseUserImage(IplImage **img);
	vector<CvRect> CheckBounds(IplImage *img);
	void CutImage(IplImage *source, vector<CvRect> rect, vector<IplImage*> *dst);
	void CopyImage(IplImage *source, IplImage *dst, CvRect rect);
	void InverseImage(IplImage *source);
	void SIFTInstance(IplImage *pImage, const char *pName);
	void ZoomImage(Image &in, Image &out, const char *cName = nullptr);
	void ZoomImage(vector<Image> *in, vector<Image> *out, Location &zoo);
	void ZoomImage(IplImage *in, IplImage *out);
	void ZoomImage_16(vector<IplImage*> *source, vector<IplImage*> *out);
	Image find(IplImage *in, Image *out);
	void ExtractFeature(IplImage *source, int obj, int objW, ofstream *fsave);
	void ExcuteThreshold(IplImage *source, int iThreshold, bool invers = false);

	void FillImage(Image &in, Image &out);
	void FillImage(IplImage *in, IplImage *out);
	void IplImge2Image(IplImage *in, Image &out);
	void test(string path);
	Location  FindMaxLocation(vector<Location> &x, vector<Location> &y);
	int FindMax(vector<Location> &v);
	map<string, IplImage*>  m_mapImage;

	CvANN_MLP m_bpANN;
	void ANN_Train(float (*trainData)[MAX_TRAIN_COLS], int t_r, int t_c,float (*obj)[MAX_OBJ_COLS], int o_r, int o_c);
	void ANN_GetSample();
	void ANN_TrainReady();
	void ANN_Region();
	void AssignObjMat(float (*obj)[MAX_OBJ_COLS], int row, int col, int nSample); 
	void ANN_ExtractFeature(IplImage *source, float (*trainData)[MAX_TRAIN_COLS], int row, int cols, int curRow);
	void ANN_ExtractFeature(IplImage *source, float *trainData, int cols);
};

