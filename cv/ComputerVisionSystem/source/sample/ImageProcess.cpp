#include "StdAfx.h"
#include "ImageProcess.h"
#include "Teatures.h"


CImageProcess::CImageProcess(string sName)
{
	m_sName.clear();
	m_sName.append(sName);
	m_mapImage.clear();
	pSwellTemp = nullptr;

	//ANN_TrainReady();
	ANN_Region();
}


CImageProcess::~CImageProcess(void)
{
	map<string, IplImage*>::iterator it=m_mapImage.begin();
	for(; it!=m_mapImage.end(); it++){
		cvReleaseImage(&(it->second));
	}

	if(pSwellTemp){
		delete []pSwellTemp;
		pSwellTemp = nullptr;
	}
}

void CImageProcess::Sample(string path, float *obj, int cols)
{
	IplImage*	pOSource = cvLoadImage(path.c_str());
	m_mapImage.insert(std::pair<string, IplImage*>(m_sName, pOSource));
	//opencv  灰度化
	IplImage* pOGray = cvCreateImage(cvGetSize(pOSource), IPL_DEPTH_8U, 1);
	m_mapImage.insert(std::pair<string, IplImage*>("gray", pOGray));
	cvCvtColor(pOSource, pOGray, CV_BGR2GRAY);
	//opencv 二值化
	IplImage*	pOBinary = cvCreateImage(cvGetSize(pOGray), IPL_DEPTH_8U, 1);
	m_mapImage.insert(std::pair<string, IplImage*>("binary", pOBinary));
	//cvThreshold(pOGray, pOBinary, 175, 255, CV_THRESH_BINARY_INV);
	ExcuteThreshold(pOGray, 175, true);

	vector<CvRect> rect = CheckBounds(pOGray);
	vector<IplImage*> image;
	CutImage(pOGray, rect, &image);

	vector<IplImage*>::iterator it = image.begin();
	for(int i=0; it != image.end(); it++,i++){
		InverseImage(*it);
	}

	vector<IplImage*> dst;
	ZoomImage_16(&image, &dst);

	it = image.begin();
	for(int i=0; it != image.end(); it++,i++){
		//char c[10]={0};
		//itoa(i,c,10);
		//cvNamedWindow(c,1); 
		//cvShowImage(c,*it);  
		cvReleaseImage(&(*it));
	}


	//ofstream fsave;
	//fsave.open("./sources/feature.txt", ios_base::out | ios_base::ate | ios_base::app);
	it=dst.begin();
	if(it != dst.end()){
		ANN_ExtractFeature(*it , obj, cols);
	}
}

void CImageProcess::Sample(string path, float (*trainData)[MAX_TRAIN_COLS], int row)
{
	//test(path);
	IplImage*	pOSource = cvLoadImage(path.c_str());
	m_mapImage.insert(std::pair<string, IplImage*>(m_sName, pOSource));
	//opencv  灰度化
	IplImage* pOGray = cvCreateImage(cvGetSize(pOSource), IPL_DEPTH_8U, 1);
	m_mapImage.insert(std::pair<string, IplImage*>("gray", pOGray));
	cvCvtColor(pOSource, pOGray, CV_BGR2GRAY);
	//opencv 二值化
	IplImage*	pOBinary = cvCreateImage(cvGetSize(pOGray), IPL_DEPTH_8U, 1);
	m_mapImage.insert(std::pair<string, IplImage*>("binary", pOBinary));
	//cvThreshold(pOGray, pOBinary, 175, 255, CV_THRESH_BINARY_INV);
	ExcuteThreshold(pOGray, 175, true);

	vector<CvRect> rect = CheckBounds(pOGray);
	vector<IplImage*> image;
	CutImage(pOGray, rect, &image);

	vector<IplImage*>::iterator it = image.begin();
	for(int i=0; it != image.end(); it++,i++){
		InverseImage(*it);
	}

	vector<IplImage*> dst;
	ZoomImage_16(&image, &dst);

	it = image.begin();
	for(int i=0; it != image.end(); it++,i++){
			//char c[10]={0};
			//itoa(i,c,10);
			//cvNamedWindow(c,1); 
			//cvShowImage(c,*it);  
		cvReleaseImage(&(*it));
	}
	

	//ofstream fsave;
	//fsave.open("./sources/feature.txt", ios_base::out | ios_base::ate | ios_base::app);
	it=dst.begin();
	for(; it!=dst.end(); it++){
		ANN_ExtractFeature(*it , trainData, MAX_SAMPLES, MAX_TRAIN_COLS, row);
	//	ExtractFeature(*it, 1, 24, &fsave);
	}
	//fsave.close();

	//ANN_GetSample();
	//cvNamedWindow("二值图像",1);    
	//cvShowImage("二值图像",pOBinary);   
	//cvWaitKey(0);  
	//cvDestroyAllWindows();  
	//cvReleaseImage(&pOBinary);  
}

void CImageProcess::ExcuteThreshold(IplImage *source, int iThreshold, bool invers)
{
	for(int i=0; i<source->height; i++)
	{
		for(int j=0; j<source->widthStep; j++)
		{
			unsigned char value = *(source->imageData + i * source->widthStep + j);
			if(value >= iThreshold){
				*(source->imageData + i * source->widthStep + j) = invers ? 0 : 255;
			}
			else{
				*(source->imageData + i * source->widthStep + j) = invers ? 255 : 0;
			}
		}
	}
}

void CImageProcess::InverseImage(IplImage *source)
{
	for(int i=0; i<source->height; i++){
		for(int j=0; j<source->widthStep; j++)
		{
			*(source->imageData + i * source->widthStep + j) = 
				255 - *(source->imageData + i * source->widthStep + j);
		}
	}
}

void CImageProcess::ZoomImage_16(vector<IplImage*> *source, vector<IplImage*> *out)
{
	vector<IplImage*>::iterator it = source->begin();
	for(; it != source->end(); it++){
		CvSize size;
		size.height = 16;
		size.width =16;
		IplImage *image = cvCreateImage(size, (*it)->depth, (*it)->nChannels);
		memset(image->imageData, 255, image->imageSize);

		size.width = 14;
		size.height = 10;
		IplImage*	pZoom = cvCreateImage(size, (*it)->depth, (*it)->nChannels);
		memset(pZoom->imageData, 255 ,pZoom->imageSize);
		cvResize(*it, pZoom, CV_INTER_LINEAR);

		FillImage(pZoom, image);
		cvReleaseImage(&pZoom);
		out->push_back(image);

	}
}
void CImageProcess::CutImage(IplImage *source, vector<CvRect> rect, vector<IplImage*> *dst)
{
	vector<CvRect>::iterator it = rect.begin();
	for(; it != rect.end(); it++){
		CvSize size;
		size.height = it->height;
		size.width = it->width;
		IplImage *image = cvCreateImage(size,source->depth, source->nChannels);
		memset(image->imageData, 0, image->imageSize);
		CopyImage(source,image, *it);
		dst->push_back(image);
	}
}

void CImageProcess::CopyImage(IplImage *source, IplImage *dst, CvRect rect)
{
	for(int i = 0; i<rect.height; i++){
		for(int j=0; j<rect.width; j++){
			*(dst->imageData + i*(dst->widthStep) + j) = 
				*(source->imageData + (i+ rect.y)*(source->widthStep) + j+rect.x);
		}
	}
}

vector<CvRect> CImageProcess::CheckBounds(IplImage *img)
{
	CvMemStorage* storage = cvCreateMemStorage( 0 );        
	CvSeq* contours = NULL;        
	IplImage *imgTemp = cvCloneImage( img );        
	cvFindContours( imgTemp, storage, &contours, sizeof( CvContour ), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE );  
	vector<CvRect> rect;
	for( ; contours != NULL; contours = contours -> h_next )        
	{                
		CvRect _rect = cvBoundingRect( contours, 0 );    
		rect.push_back(_rect);
	//	cvRectangle( img, cvPoint( rect.x, rect.y ),cvPoint( rect.x + rect.width, rect.y + rect.height ), cvScalar(0,0,0), 0 );        
	}  
	return rect;
}

void CImageProcess::ProcessImage()
{
	IplImage*	pOSource = cvLoadImage(m_sName.c_str());
	m_mapImage.insert(std::pair<string, IplImage*>(m_sName, pOSource));
	//opencv  灰度化
	IplImage* pOGray = cvCreateImage(cvGetSize(pOSource), IPL_DEPTH_8U, 1);
	m_mapImage.insert(std::pair<string, IplImage*>("gray", pOGray));

	cvCvtColor(pOSource, pOGray, CV_BGR2GRAY);

	//opencv 二值化
	IplImage*	pOBinary = cvCreateImage(cvGetSize(pOGray), IPL_DEPTH_8U, 1);
	m_mapImage.insert(std::pair<string, IplImage*>("binary", pOBinary));
	cvThreshold(pOGray, pOBinary, 175, 255, CV_THRESH_BINARY);

	//pSwellTemp = LevelSwell(pOBinary->imageData, pOBinary->widthStep, pOBinary->height,8);
	//VerticalSwell(pOBinary->imageData, pOBinary->widthStep, pOBinary->height, 0);
	//OpencvSwell(pOBinary, pOBinary);

	//投影法，分割
	unsigned int *pVHist = new unsigned int[pOBinary->widthStep];
	VerticalProjection(pOBinary->imageData, pOBinary->widthStep, pOBinary->height, pVHist);
	unsigned int *pHHist = new unsigned int[pOBinary->height];
	HorizontalProjection(pOBinary->imageData, pOBinary->widthStep, pOBinary->height, pHHist);

	vector<Location> v_lc = Projection_Location(pVHist,pOBinary->widthStep, pOBinary->height);
	vector<Location> h_lc = Projection_Location(pHHist,pOBinary->height, pOBinary->widthStep);

	//切图
	Image image;
	image._height = pOBinary->height;
	image._width = pOBinary->widthStep;
	image._pData = new unsigned char[image._height * image._width];
	memset(image._pData, 0 , image._height * image._width);
	memcpy_s(image._pData, image._height * image._width, pOBinary->imageData, image._height * image._width);
	vector<Image> dst;
	CutImage(image, v_lc, h_lc, &dst);
	delete []image._pData;

	//缩放
	//Location l = FindMaxLocation(v_lc, h_lc);
	Location l;
	l._start = 16;
	l._end = 16;
	vector<Image> zImage;
	ZoomImage(&dst,&zImage, l);

	//提取特征
	for(int i=0; i<zImage.size(); i++){
		char c[10];
		itoa(i, c, 10);
		IplImage *img = CreateImage(zImage.at(i));
		//m_mapImage.insert(std::pair<string, IplImage*>(c, img));
		SIFTInstance(img, c);
		ReleaseUserImage(&img);
	}
	
/*
	char*		cOTitle = {"opencv"};
	cvNamedWindow(cOTitle, CV_WINDOW_AUTOSIZE);
	cvShowImage(cOTitle,pOBinary);*/

	//test();
	cvWaitKey();

	cvDestroyAllWindows();
}

Location CImageProcess::FindMaxLocation(vector<Location> &x, vector<Location> &y)
{
	Location v;
	v._start = FindMax(x);
	v._end = FindMax(y);
	return v;
}

int CImageProcess::FindMax(vector<Location> &v)
{
	vector<Location>::iterator it = v.begin();
	int maxH = it->_end - it->_start;
	it ++;
	for(; it != v.end(); it++){
		int maxh = it->_end - it->_start;
		if(maxh > maxH){
			maxH = maxh;
		}
	}

	return maxH;
}

void CImageProcess::IplImge2Image(IplImage *in, Image &out)
{
	memset(out._pData, 0 ,in->imageSize);
	memcpy_s(out._pData, in->imageSize, in->imageData, in->imageSize);
	out._height = in->height;
	out._width = in->widthStep;
}

void CImageProcess::ZoomImage(IplImage *in, IplImage *out)
{
	CvSize size;
	size.width = 14;
	size.height = 14;
	IplImage*	pZoom = cvCreateImage(size, in->depth, in->nChannels);
	memset(pZoom->imageData, 0 ,pZoom->imageSize);
	cvResize(in, pZoom, CV_INTER_AREA);

	FillImage(pZoom, out);
	cvReleaseImage(&pZoom);
}

Image CImageProcess::find(IplImage *in, Image *out)
{
	vector<vector<Location>> res;
	res.clear();
	//投影法，分割
	unsigned int *pVHist = new unsigned int[in->widthStep];
	VerticalProjection(in->imageData, in->widthStep, in->height, pVHist);
	unsigned int *pHHist = new unsigned int[in->height];
	HorizontalProjection(in->imageData, in->widthStep, in->height, pHHist);

	vector<Location> v_lc = Projection_Location(pVHist,in->widthStep, in->height);
	vector<Location> h_lc = Projection_Location(pHHist,in->height, in->widthStep);

	//切图
	Image image;
	image._height = in->height;
	image._width = in->widthStep;
	image._pData = new unsigned char[image._height * image._width];
	memset(image._pData, 0 , image._height * image._width);
	memcpy_s(image._pData, image._height * image._width, in->imageData, image._height * image._width);
	vector<Image> dst;
	CutImage(image, v_lc, h_lc, &dst);
	delete []image._pData;

	
	res.push_back(v_lc);
	res.push_back(h_lc);

	delete []pVHist;
	delete []pHHist;
	pVHist = nullptr;
	pHHist = nullptr;

	return dst.at(0);
}

void CImageProcess::ZoomImage(vector<Image> *in, vector<Image> *out, Location &zoo)
{
	for(int i=0; i<in->size(); i++){
		char c[10];
		itoa(i+10, c, 10);

		Image zootmp;
		zootmp._width = zoo._start - 2;
		zootmp._height = zoo._end - 2;
		zootmp._pData = new unsigned char[zootmp._width * zootmp._height];
		memset(zootmp._pData, 0, zootmp._width * zootmp._height);
		ZoomImage(in->at(i), zootmp);
		
		Image dsttmp;
		dsttmp._width = zoo._start;
		dsttmp._height = zoo._end;
		dsttmp._pData = new unsigned char[dsttmp._width * dsttmp._height];
		FillImage(zootmp, dsttmp);
		delete []zootmp._pData;
		out->push_back(dsttmp);
		IplImage *img = CreateImage(dsttmp, c);
	}
}


void CImageProcess::ZoomImage(Image &in, Image &out, const char *cName)
{
	IplImage *image = CreateImage(in);
	IplImage *pDst = CreateImage(out);

	cvResize(image, pDst, CV_INTER_AREA );

	if(cName){
		cvNamedWindow(cName, CV_WINDOW_AUTOSIZE);
		cvShowImage(cName,pDst);
	}


	//cvSaveImage("1.png",pDst);
	IplImge2Image(pDst, out);
}

void CImageProcess::ExtractFeature(IplImage *source, int obj, int objW, ofstream *fsave)
{

	if(!fsave->is_open()){
		return;
	}
	for(int i=0; i<objW; i++){
		if(obj == i){
			*fsave<<(unsigned int)1;
		}
		*fsave<<(unsigned int)0;
	}
	*fsave<<":";
	unsigned char value;
	char tmp;
	for(int i=0; i<source->height; i++){
		for(int j=0; j<source->widthStep; j++){
			value = *(source->imageData + i * source->widthStep + j);
			*fsave<<(unsigned int)value<<",";
			/*if(255 == value)
			{
				*fsave <<255<<",";
			}
			else if(0 == value){
				*fsave<<0<<",";
			}
			else{
				char t = (char)value;
				atoi(&t);
				*fsave<<atoi(&t)<<",";
			}*/
			//*fsave<<value<<" ";
			
		}
	}
	*fsave<<"\n"<<endl;

}


void CImageProcess::FillImage(IplImage *in, IplImage *out)
{
	int x_start = (out->widthStep - in->widthStep) / 2;
	int y_start = (out->height - in->height) /2;
	memset(out->imageData, 255, out->height*out->widthStep);

	for(int i=0; i<in->height; i++){
		for(int j=0; j<in->widthStep; j++){
			*(out->imageData + (i+y_start) * (out->widthStep) + j+x_start) = *(in->imageData + i * in->widthStep + j);
		}
	}
}
void CImageProcess::FillImage(Image &in, Image &out)
{
	int x_start = (out._width - in._width) / 2;
	int y_start = (out._height - in._height) /2;
	memset(out._pData, 255, out._height*out._width);

	for(int i=0; i<in._height; i++){
		for(int j=0; j<in._width; j++){
			*(out._pData + (i+y_start) * out._width + j+x_start) = *(in._pData + i * in._width + j);
		}
	}
}

void CImageProcess::ReleaseUserImage(IplImage **img){
	delete [] (*img)->imageData;
	(*img)->imageData = nullptr;
	cvReleaseImage(img);
}

IplImage* CImageProcess::CreateImage(Image &image, const char* cName)
{
	CvSize size = {image._width, image._height};
	IplImage* pUserImage = cvCreateImage(size, IPL_DEPTH_8U, 1);
	pUserImage->height = image._height;
	pUserImage->width = image._width;
	pUserImage->widthStep = image._width;
	pUserImage->imageDataOrigin = nullptr;
	pUserImage->imageSize = image._height * image._width;
	pUserImage->imageData = new char[pUserImage->imageSize];
	memset(pUserImage->imageData, 0 ,pUserImage->imageSize);
	memcpy_s(pUserImage->imageData, pUserImage->imageSize, image._pData, pUserImage->imageSize);

	if(cName != nullptr){
		cvNamedWindow(cName, CV_WINDOW_AUTOSIZE);
		cvShowImage(cName,pUserImage);
	}

	return pUserImage;
}
void CImageProcess::OpencvSwell(IplImage *pIn, IplImage *pOut)
{
	IplConvKernel tmp;
	tmp.nCols = 8;
	tmp.nRows = 1;
	tmp.anchorX = tmp.anchorY =0;
	tmp.nShiftR = CV_SHAPE_RECT;
	//int values[8] = {255, 255, 255, 255,255,255,255,255};
	int values[8] = {1, 1, 1, 1,1,1,1,1};
	tmp.values = values;
	cvDilate(pIn, pOut,&tmp,1);
}

unsigned char* CImageProcess::HorizontalSwell(char *pIn, int iWidth, int iHeight, int n)
{
	unsigned char *pData = reinterpret_cast<unsigned char*>(pIn);
	unsigned char *pOut = new unsigned char [iWidth * iHeight];
	memset(pOut, 0, iWidth * iHeight);
	memcpy_s(pOut , iWidth * iHeight, pData, iWidth * iHeight);
	
	for(int i=0; i<iHeight; i++)
	{
		pData = reinterpret_cast<unsigned char*>(pIn) + i * iWidth;
		for(int j=(n-1)/2; j < iWidth-(n-1)/2; j++)
		{
			for(int k = -(n-1)/2; k < (n-1)/2; k++)
			{
				unsigned char value = *(pData + j + k);
				if(255 == value)
				{
					*(pOut + i * iWidth + j) = value;
					break;
				}
			}
		}
	}

	return pOut;
}

void CImageProcess::VerticalSwell(char *pIn, int iWidth, int iHeight, int n)
{
	unsigned char *pData = reinterpret_cast<unsigned char*>(pIn);
	if(nullptr == pSwellTemp){
		return;
	}

	if(0 == n){
		memcpy_s(pIn, iWidth * iHeight, pSwellTemp, iWidth * iHeight);
	}

	for(int i=(n-1)/2; i<iHeight-(n-1)/2; i++)
	{
		for(int j=0; j < iWidth; j++)
		{
			for(int k = -(n-1)/2; k < (n-1)/2; k++)
			{
				unsigned char value = *(pSwellTemp + iWidth * (i +k) + j);
				if(255 == value)
				{
					*(pData + i * iWidth + j) = value;
					break;
				}
			}
		}
	}
}


/*
pIn : 图像内存
iWidth : 图像字节宽度，
iHeight ：图像字节高度
       eg:垂直投影 len(pOut) = iWidth * sizeof(unsignd int)字节
		  水平投影 len(pOut) = iHeight * sizeof(unsignd int) 字节
*/
void CImageProcess::HorizontalProjection(char *pIn, int iWidth, int iHeight, unsigned int *pOut)
{
	memset(pOut, 0, iHeight* sizeof(unsigned int));
	unsigned char *pImage = reinterpret_cast<unsigned char*>(pIn);
	for(int i=0; i < iHeight; i++){
		for(int j=0; j < iWidth; j++){
			unsigned char value = *(pImage + i * iWidth + j);
			if(255 == value)
				pOut[i] ++; 
		}
	}
}

void CImageProcess::VerticalProjection(char *pIn, int iWidth, int iHeight, unsigned int *pOut)
{
	memset(pOut, 0, iWidth * sizeof(unsigned int));
	unsigned char *pImage = reinterpret_cast<unsigned char*>(pIn);
	for(int i=0; i < iWidth; i++){
		for(int j=0; j < iHeight; j++){ 
			unsigned char value = *(pImage + j * iWidth + i);
			if(255 == value)
				pOut[i] ++;
		}
	}
}

vector<Location> CImageProcess::Projection_Location(unsigned int *pValue, int iVlen, int iMax)
{
	vector<Location> res;
	res.clear();

	int vT = iMax - 2;

	char f = -1;
	unsigned int l = 0;
	Location lc;
	for(int i=0; i< iVlen; i++){
		unsigned int t = pValue[i];
		if(pValue[i] > vT & (0 == f)){//更新起始位置
			l = i;
		}
		else if(pValue[i] <= vT & (0 == f)){ //是起始位置
			lc._start = l;
			l = i;
			f = 1;
		}
		else if(pValue[i] <= vT & (1 == f)){ //更新终止位置
			l = i;
			f = 1;
		}
		else if(pValue[i] > vT & (1 == f)){//是终止位置
			lc._end = l;
			l = i;
			f = 0;
			res.push_back(lc);
			lc.clear();
		}
		else{
			f = (pValue[i] >= vT) ? 0 : 1;
		}

	}

	return res;
}

void CImageProcess::CutImage(Image &source, vector<Location> x, vector<Location> y, vector<Image> *dst)
{
	if(nullptr == source._pData){
		return;
	}

	for(int i=0; i<x.size(); i++){
		for(int j=0; j<y.size(); j++){
			Image image;
			Location _x = x.at(i);
			Location _y = y.at(j);	
			image._width = _x._end - _x._start;
			image._height = _y._end - _y._start;
			if((0 == image._height) || (0 == image._width)){
				continue;
			}
			image._pData = new unsigned char[image._width * image._height];
			memset(image._pData, 0, image._width * image._height);
			CopyImage(source, image, _x, _y);
			dst->push_back(image);
		}
	}
}

void CImageProcess::CopyImage(Image &source, Image &dest, Location x, Location y)
{
	for(int h=y._start; h<y._end; h++){
		for(int w=x._start; w<x._end; w++)
		{
			(dest._pData + (h-y._start) * dest._width)[w-x._start] = (source._pData + h * source._width)[w];
		}
	}
}

void CImageProcess::SIFTInstance(IplImage *pImage, const char *pName)
{
	cv::Mat src(pImage, 0);
	Vector<Keypoint> features;
	
	CTeatures sift;
	sift.Sift(src, features, 1.6);
	sift.DrawKeyPoints(src, features);
	sift.DrawSiftFeatures(src, features);

	char des[256] = {0};
	sprintf_s(des, sizeof(des), ".\\%s_descriptor.txt", pName);
	sift.write_features(features, des);

	IplImage *pImg = nullptr;
	pImg = &IplImage(src);
	

	/*cvNamedWindow(pName, CV_WINDOW_AUTOSIZE);
	cvShowImage(pName,pImg);*/
	//imshow(pName, src);
}

void CImageProcess::test(string path)
{
	IplImage * src=cvLoadImage(path.c_str(),0);  
	//  cvSmooth(src,src,CV_BLUR,3,3,0,0);  
	cvThreshold(src,src,175,255,CV_THRESH_BINARY);  
	IplImage* paintx=cvCreateImage( cvGetSize(src),IPL_DEPTH_8U, 1 );  
	IplImage* painty=cvCreateImage( cvGetSize(src),IPL_DEPTH_8U, 1 );  
	cvZero(paintx);  
	cvZero(painty);  
	int* v=new int[src->width];  
	int* h=new int[src->height];  
	memset(v,0,src->width*4);  
	memset(h,0,src->height*4);  

	int x,y;  
	CvScalar s,t;  
	for(x=0;x<src->width;x++)  
	{  
		for(y=0;y<src->height;y++)  
		{  
			s=cvGet2D(src,y,x);           
			if(s.val[0]==255)  
				v[x]++;                   
		}         
	}  

	for(x=0;x<src->width;x++)  
	{  
		for(y=0;y<v[x];y++)  
		{         
			t.val[0]=255;  
			cvSet2D(paintx,y,x,t);        
		}         
	}  

	for(y=0;y<src->height;y++)  
	{  
		for(x=0;x<src->width;x++)  
		{  
			s=cvGet2D(src,y,x);           
			if(s.val[0]==255)  
				h[y]++;       
		}     
	}  
	for(y=0;y<src->height;y++)  
	{  
		for(x=0;x<h[y];x++)  
		{             
			t.val[0]=255;  
			cvSet2D(painty,y,x,t);            
		}         
	}  
	cvNamedWindow("二值图像",1);  
	cvNamedWindow("垂直积分投影",1);  
	cvNamedWindow("水平积分投影",1);  
	cvShowImage("二值图像",src);  
	cvShowImage("垂直积分投影",paintx);  
	cvShowImage("水平积分投影",painty);  
	cvWaitKey(0);  
	cvDestroyAllWindows();  
	cvReleaseImage(&src);  
	cvReleaseImage(&paintx);  
	cvReleaseImage(&painty);  
}


//骨架化
void CImageProcess::Thinning(Image &source, Image &dst){
	memset(dst._pData, 0, dst._height * dst._width);
	IplImage *tmp = CreateImage(source);
	IplImage *tmp_d = CreateImage(dst);
	cv::Mat src(tmp, 0);
	cv::Mat	dst_t(tmp_d);
	dst_t = src.clone();
	dst_t /= 255;         // convert to binary image

	cv::Mat prev = cv::Mat::zeros(dst_t.size(), CV_8UC1);
	cv::Mat diff;

	do {
		ThinningIteration(dst_t, 0);
		ThinningIteration(dst_t, 1);
		cv::absdiff(dst_t, prev, diff);
		dst_t.copyTo(prev);
	} 
	while (cv::countNonZero(diff) > 0);

	dst_t *= 255;
	IplImage tmp_(dst_t);
	IplImge2Image(&tmp_, dst);
	ReleaseUserImage(&tmp);
	ReleaseUserImage(&tmp_d);
}

/**
 * Perform one thinning iteration.
 * Normally you wouldn't call this function directly from your code.
 *
 * Parameters:
 * 		im    Binary image with range = [0,1]
 * 		iter  0=even, 1=odd
 */
void CImageProcess::ThinningIteration(cv::Mat &img, int iter)
{
	CV_Assert(img.channels() == 1);
	CV_Assert(img.depth() != sizeof(uchar));
	CV_Assert(img.rows > 3 && img.cols > 3);

	cv::Mat marker = cv::Mat::zeros(img.size(), CV_8UC1);

	int nRows = img.rows;
	int nCols = img.cols;

	if (img.isContinuous()) {
		nCols *= nRows;
		nRows = 1;
	}

	int x, y;
	uchar *pAbove;
	uchar *pCurr;
	uchar *pBelow;
	uchar *nw, *no, *ne;    // north (pAbove)
	uchar *we, *me, *ea;
	uchar *sw, *so, *se;    // south (pBelow)

	uchar *pDst;

	// initialize row pointers
	pAbove = NULL;
	pCurr  = img.ptr<uchar>(0);
	pBelow = img.ptr<uchar>(1);

	for (y = 1; y < img.rows-1; ++y) {
		// shift the rows up by one
		pAbove = pCurr;
		pCurr  = pBelow;
		pBelow = img.ptr<uchar>(y+1);

		pDst = marker.ptr<uchar>(y);

		// initialize col pointers
		no = &(pAbove[0]);
		ne = &(pAbove[1]);
		me = &(pCurr[0]);
		ea = &(pCurr[1]);
		so = &(pBelow[0]);
		se = &(pBelow[1]);

		for (x = 1; x < img.cols-1; ++x) {
			// shift col pointers left by one (scan left to right)
			nw = no;
			no = ne;
			ne = &(pAbove[x+1]);
			we = me;
			me = ea;
			ea = &(pCurr[x+1]);
			sw = so;
			so = se;
			se = &(pBelow[x+1]);

			int A  = (*no == 0 && *ne == 1) + (*ne == 0 && *ea == 1) + 
				(*ea == 0 && *se == 1) + (*se == 0 && *so == 1) + 
				(*so == 0 && *sw == 1) + (*sw == 0 && *we == 1) +
				(*we == 0 && *nw == 1) + (*nw == 0 && *no == 1);
			int B  = *no + *ne + *ea + *se + *so + *sw + *we + *nw;
			int m1 = iter == 0 ? (*no * *ea * *so) : (*no * *ea * *we);
			int m2 = iter == 0 ? (*ea * *so * *we) : (*no * *so * *we);

			if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
				pDst[x] = 1;
		}
	}

	img &= ~marker;
}

void CImageProcess::ANN_GetSample()
{
	
	ifstream fin;
	fin.open("./sources/feature.txt");
	unsigned int value;
	if(fin.is_open()){
		cout <<"open"<<endl;
	}
	while(!fin.eof()){
		fin>>value;
		cout<<value<<endl;
	}
	fin.close();
}

void CImageProcess::ANN_Region()
{
	char path[512] = {0};
	float obj[MAX_TRAIN_COLS] = {0};
	Sample("./sources/0 (1).bmp", obj, MAX_TRAIN_COLS);

	CvANN_MLP bpANN;

	CvANN_MLP_TrainParams param;
	param.term_crit = cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS,5000,0.01);
	param.train_method = CvANN_MLP_TrainParams::BACKPROP;
	param.bp_dw_scale = 0.1;
	param.bp_moment_scale = 0.1;

	Mat	layerSize = (Mat_<int>(1,3)<<MAX_TRAIN_COLS ,MAX_OBJ_COLS,MAX_OBJ_COLS);
	bpANN.create(layerSize, CvANN_MLP::SIGMOID_SYM);


	//m_bpANN.load("./sources/mlp.xml");

	Mat input(1, MAX_TRAIN_COLS, CV_32FC1, obj);
	float _obj[MAX_OBJ_COLS] ={0};
	Mat out(1, MAX_OBJ_COLS, CV_32FC1, _obj);
	//Mat out;
	bpANN.predict(input,out);

	int i=0;
	i+=i;

}

void CImageProcess::ANN_TrainReady()
{
	char path[512] = {0};

	memset(m_trainObj, 0, MAX_SAMPLES * MAX_OBJ_COLS);
	memset(m_trainImput, 0, MAX_SAMPLES * MAX_TRAIN_COLS);
	AssignObjMat(m_trainObj, MAX_SAMPLES, MAX_OBJ_COLS, 5);
	
	for(int i=0; i < SAMPLE_COUNT; i++){
		for(int j=0; j< SAMPLE_SON; j++){
			int row = j+i*SAMPLE_SON;
			memset(path, sizeof(path), 0);
			sprintf_s(path,"./sources/%d (%d).bmp",i,j+1);
			Sample(path,m_trainImput, row);
		}
	}

	//Sample("./sources/3.bmp",m_trainImput, 3);
	ANN_Train(m_trainImput, MAX_SAMPLES, MAX_TRAIN_COLS, m_trainObj, MAX_SAMPLES, MAX_OBJ_COLS);
}

void CImageProcess::ANN_ExtractFeature(IplImage *source, float *trainData, int cols)
{
	if(source->widthStep * source->height != cols){
		return;
	}
	unsigned char value;
	int index = 0;
	for(int i=0; i<source->height; i++){
		for(int j=0; j<source->widthStep; j++){
			index = i * source->widthStep + j;
			value = *(source->imageData + i * source->widthStep + j);
			unsigned int tmp = (unsigned int)value;
			float tmp_1 = (float)tmp;
			if(index <= cols)
				trainData[index] = tmp;

		}
	}
}
void CImageProcess::ANN_ExtractFeature(IplImage *source, float (*trainData)[MAX_TRAIN_COLS], int row, int cols, int curRow)
{
	unsigned char value;
	int index = 0;
	for(int i=0; i<source->height; i++){
		for(int j=0; j<source->widthStep; j++){
			index = i * source->widthStep + j;
			value = *(source->imageData + i * source->widthStep + j);
			unsigned int tmp = (unsigned int)value;
			float tmp_1 = (float)tmp;
			if(index <= row * cols)
				trainData[curRow][index] = tmp;
			
		}
	}

}
void CImageProcess::AssignObjMat(float (*obj)[MAX_OBJ_COLS],int row, int col, int nSample)
{
	if(nSample != row / col){
		return;
	}
	int n=0;
	int nCol=0;
	for(int j=0; j< row; j++){
		for(int i=0; i< col; i++){
			obj[j][i] = 0;
		}
	}

	for(int j=0; j< row; j++){
		if(n >= nSample){
			n=0;
			nCol++;	
		}
		obj[j][nCol] = 1;
		n++;
	}

}

void CImageProcess::ANN_Train(float (*trainData)[MAX_TRAIN_COLS], int t_r, int t_c,float (*obj)[MAX_OBJ_COLS], int o_r, int o_c)
{
//	CvANN_MLP m_bpANN;
	

	CvANN_MLP_TrainParams param;
	param.term_crit = cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS,5000,0.01);
	param.train_method = CvANN_MLP_TrainParams::BACKPROP;
	param.bp_dw_scale = 0.1;
	param.bp_moment_scale = 0.1;

	Mat	layerSize = (Mat_<int>(1,3)<<t_c ,o_c,o_c);
	//Mat layerSize=(Mat_<int>(1,5) << 256,2,2,2,24);
	m_bpANN.create(layerSize, CvANN_MLP::SIGMOID_SYM);

	/*float labels[3][24] = {1};
	Mat labelsMat(3, 24, CV_32FC1, labels);
	float trainingData[3][256] = {1};
	Mat trainingDataMat(3, 256, CV_32FC1, trainingData);*/

	Mat labelsMat(o_r, o_c, CV_32FC1, obj);
	Mat trainingDataMat(t_r, t_c, CV_32FC1, trainData);

	//m_bpANN.train(input, obj, Mat(), Mat(), param);
	m_bpANN.train(trainingDataMat, labelsMat, Mat(), Mat(), param);
	m_bpANN.save("./sources/mlp.xml");

//	CvANN_MLP m_bpInden;
//	m_bpInden.load("mlp.xml");

//	m_bpInden.predict();
}


