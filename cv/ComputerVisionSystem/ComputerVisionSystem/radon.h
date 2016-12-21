#pragma  once

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"

bool radonTransformer(IplImage *pImage, float &fthetal);
/*
IplImage *Radon(IplImage *pSrc);
IplImage *RadonTest(IplImage *pSrc);
void GetMax(IplImage *pRadon,double **info);
void GetMaxTest(IplImage *pRadon,double **info);
void GetMaxThetal(IplImage *pRadon, double *thetal);*/