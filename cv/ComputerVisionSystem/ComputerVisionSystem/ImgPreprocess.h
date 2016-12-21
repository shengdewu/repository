#pragma once
#include "opencvheader.h"

class CImgPreprocess
{
public:
	CImgPreprocess(void);
	~CImgPreprocess(void);

	int excutePreprocess(const char *pFile);

private:
	bool excuteGray(IplImage* pSrc, IplImage* pDst, char method);
	int OtsuThreshold(int iHeight, int iWidthStep, unsigned char* pImage);
	void GetHistogram(unsigned char* pImage,int iHeight, int iWidthStep, double* pHistogram);
	int Otsu(IplImage *pImage);
	void expandImage(IplImage *pSource, IplImage *pDst, bool bZero);
	int edgeDetection(IplImage *pImage, IplImage **pOut);
	void roatImage(IplImage *pImage, float angle);
	void inverseImage(IplImage *source);
	void segmenImg(IplImage *pImg, IplImage **pOut);
	void ZhangThinning(int w,int h,unsigned char *imgBuf);
};

