#include "ImgPreprocess.h"
#include "radon.h"

CImgPreprocess::CImgPreprocess(void)
{
}


CImgPreprocess::~CImgPreprocess(void)
{
}

//3ͨ����1ͨ��
bool CImgPreprocess::excuteGray(IplImage* pSrc, IplImage* pDst, char method)
{
	if(nullptr == pSrc || nullptr == pDst){
		return false;
	}

	memset(pDst->imageData, 0, pDst->imageSize);

	int iPixlSize = pSrc->widthStep / pSrc->width; //ԭʼͼ��ÿ�����ص��ֽڴ�С
	unsigned char*	pData = reinterpret_cast<unsigned char*>(pSrc->imageData); //ԭʼͼƬ�ڴ�  bgr
	unsigned char*  pOutData = reinterpret_cast<unsigned char*>(pDst->imageData);
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;
	unsigned char gray = 0;

	for(int i=0; i<pSrc->height; i++){
		pData = reinterpret_cast<unsigned char*>(pSrc->imageData) + pSrc->widthStep * i; //ԭʼͼƬ�ڴ�  bgr
		pOutData = reinterpret_cast<unsigned char*>(pDst->imageData) + pDst->widthStep * i;
		for(int j=0; j<pSrc->width; j++){

			r = pData[iPixlSize * j + 2];
			g = pData[iPixlSize * j + 1];
			b = pData[iPixlSize * j];
			gray = 0;
			switch(method){
			case 'r': //������
				gray = r;
				break;
			case 'g'://������
				gray = g;
				break;
			case 'b'://������
				gray = b;
				break;
			case 'm'://���ֵ��
				gray = r > b ? r : b;
				gray = gray > g ? gray : g;
				break;
			case 'a': //��ֵ��
				gray =  (r + g + b) / 3;
				break;
			case 'o': //opencv 
				gray =  0.072169 * b+ 0.715160 * g+ 0.212671 * r;
				break;
			default://������������ѧ ѡȡy���� Y=0.3R+0.59G+0.11B
				//�ṩ�����ٶȣ�������ʹ��float
				gray = (11 * b + 59 * g + 30 * r) / 100;
				break;
			}

			//�Ҷ�ֵ
			if(gray > 255){
				gray = 255;
			}

			pOutData[j] = gray;
		}
	}
	return true;
}

void CImgPreprocess::GetHistogram(unsigned char* pImage,int iHeight, int iWidthStep, double* pHistogram)
{
	for(int i=0; i<iHeight; i++)
	{//����ֱ��ͼ
		for(int j=0;j<iWidthStep;j++)
		{
			unsigned int temp = *(pImage + i * iWidthStep + j);
			temp = temp<0? 0:temp;
			temp = temp>255? 255:temp;
			pHistogram[temp]++;
		} 
	}
}

int CImgPreprocess::OtsuThreshold(int iHeight, int iWidthStep, unsigned char* pImage)
{

	int T = 0;//��ֵ
	double gSum0;//��һ��Ҷ���ֵ
	double gSum1;//�ڶ���Ҷ���ֵ
	double N0 = 0;//ǰ��������
	double N1 = 0;//����������
	double u0 = 0;//ǰ������ƽ���Ҷ�
	double u1 = 0;//��������ƽ���Ҷ�
	double w0 = 0;//ǰ�����ص���ռ����ͼ��ı���Ϊ��0
	double w1 = 0;//�������ص���ռ����ͼ��ı���Ϊ��1
	double u = 0;//��ƽ���Ҷ�
	double tempg = -1;//��ʱ��䷽��
	double g = -1;//��䷽��
	double Histogram[256]={0};// = new double[256];//�Ҷ�ֱ��ͼ
	double N = iWidthStep*iHeight;//��������

	//��ȡֱ��ͼ
	GetHistogram(pImage, iHeight, iWidthStep, Histogram);

	//������ֵ
	for (int i = 0;i<256;i++)
	{
		gSum0 = 0;
		gSum1 = 0;
		N0 += Histogram[i];			
		N1 = N-N0;
		if(0==N1)break;//������ǰ�������ص�ʱ������ѭ��
		w0 = N0/N;
		w1 = 1-w0;
		for (int j = 0;j<=i;j++)
		{
			gSum0 += j*Histogram[j];
		}
		u0 = gSum0/N0;
		for(int k = i+1;k<256;k++)
		{
			gSum1 += k*Histogram[k];
		}
		u1 = gSum1/N1;
		//u = w0*u0 + w1*u1;
		g = w0*w1*(u0-u1)*(u0-u1);
		if (tempg<g)
		{
			tempg = g;
			T = i;
		}
	}
	return T; 
}

int CImgPreprocess::Otsu(IplImage *pImage)
{
	long N = pImage->height * pImage->widthStep;
	int h[256];
	double p[256],u[256],w[256];
	for(int i = 0; i < 256; i++)
	{
		h[i] = 0;
		p[i] = 0;
		u[i] = 0;
		w[i] = 0;
	}
	for(int i = 0; i < pImage->height; i++)
		for(int j = 0; j < pImage->widthStep; j++)
			for(int k = 0; k < 256; k++)
			{
				if(((uchar*)(pImage->imageData + pImage->widthStep*i))[j] == k)
					h[k]++;
			}

			for(int i = 0; i < 256; i++)
				p[i] = h[i] / double(N);

			int T = 0;
			double uT,thegma2fang;
			double thegma2fang_max = -10000;
			for(int k = 0; k < 256; k++)
			{
				uT = 0;
				for(int i = 0; i <= k; i++)
				{
					u[k] += i*p[i];
					w[k] += p[i];
				}

				for(int i = 0; i < 256; i++)
					uT += i*p[i];

				thegma2fang = (uT*w[k] - u[k])*(uT*w[k] - u[k]) / (w[k]*(1-w[k]));
				if(thegma2fang > thegma2fang_max)
				{
					thegma2fang_max = thegma2fang;
					T = k;
				}
			}
			return T;
}

//1Ϊǰ����0Ϊ����
void CImgPreprocess::expandImage(IplImage *pSource, IplImage *pDst, bool bZero)
{
	if(pSource == nullptr || pDst == nullptr){
		return;
	}

	if(pSource->widthStep > pDst->widthStep ||
		pSource->height > pDst->height){
			return;
	}

	if(bZero){
		memset(pDst->imageData, 0, pDst->imageSize);
	}
	else{
		memset(pDst->imageData, 255, pDst->imageSize);
	}

	for(int i= 0; i < pSource->height; i++){
		for(int j=0; j < pSource->widthStep; j++){
			*(pDst->imageData + i*pDst->widthStep + j) = 
				*(pSource->imageData + i*pSource->widthStep + j);
		}
	}
}

int CImgPreprocess::edgeDetection(IplImage *pImage, IplImage **pOut)
{
	if(nullptr == pImage){
		return -1;
	}

	//��ʴ����
	Mat src(pImage);
	Mat dst;
	erode(src, dst, Mat(2,2, CV_8U), Point(-1,-1),1);

	IplImage pTmp = (IplImage)dst;

	cvNamedWindow("pTmp");
	cvShowImage("pTmp", &pTmp);

	//�
	//A-B
	*pOut = cvCreateImage(cvGetSize(pImage),pImage->depth, 1);
	memcpy_s((*pOut)->imageData,(*pOut)->imageSize, pImage->imageData, pImage->imageSize);
	
	unsigned char *pOutData = reinterpret_cast<unsigned char*>((*pOut)->imageData);
	unsigned char *pSrcData = reinterpret_cast<unsigned char*>(pImage->imageData);
	unsigned char *pErode = reinterpret_cast<unsigned char*>(pTmp.imageData);
	unsigned char srcv = 0;
	unsigned char erov = 0;
	for(int i=0; i<pImage->height; i++){
		for(int j=0; j<pImage->widthStep; j++){
			//srcv = *(pSrcData + i * pImage->height + j);
			erov = *(pErode + i * pImage->height + j);
			if(255 == erov){
				//*(pOutData + i * pImage->height + j) = 255;
				*(pOutData + i * pImage->height + j) = 0;
			}
		}
	}

	return 0;
}

void CImgPreprocess::inverseImage(IplImage *source)
{
	for(int i=0; i<source->height; i++){
		for(int j=0; j<source->widthStep; j++)
		{
			*(source->imageData + i * source->widthStep + j) = 
				255 - *(source->imageData + i * source->widthStep + j);
		}
	}
}

void CImgPreprocess::roatImage(IplImage *pImage, float angle)
{
	cv::Point center(pImage->height/2, pImage->widthStep/2);
	cv::Mat rotMat = getRotationMatrix2D(center,angle,1.0);
	IplImage *pDstImg = cvCreateImage(cvGetSize(pImage), IPL_DEPTH_8U, 1);
	memset(pDstImg->imageData, 0, pDstImg->imageSize);
	inverseImage(pImage);
	cv::Mat dstImg(pDstImg);
	cv::Mat srcImg(pImage);
	warpAffine(srcImg, dstImg, rotMat, srcImg.size(),1,0,cv::Scalar(255,255,255));
	inverseImage(pDstImg);
	cvCopy(pDstImg,pImage);
	cvReleaseImage(&pDstImg);
}


void CImgPreprocess::segmenImg(IplImage *pImg, IplImage **pOut)
{
	//������ͨ��contoure
	inverseImage(pImg);
	cvXorS(pImg, cvScalarAll(255), pImg, 0);

	CvMemStorage *storage = cvCreateMemStorage();
	CvSeq *contour = nullptr;
	cvFindContours(pImg, storage, &contour, sizeof(CvContour),CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	//������ͨ��
	CvSeq *p = contour;
	while(p){
		CvRect rect = cvBoundingRect(p, 0);
		if(rect.height < 10){//ͼ��������Ҫ15���ظ߶�
			p = p->h_next;
			continue;
		}
		//���Ƹ���ͨ��character
		//cvZero(pImg);
		*pOut = cvCreateImage(cvSize(rect.width, rect.height), IPL_DEPTH_8U, 1);
		cvZero(*pOut);
		cvDrawContours(*pOut, p, CV_RGB(255, 255, 255), CV_RGB(0, 0, 0), -1, -1, 8, cvPoint(-rect.x, -rect.y));
		//inverseImage(*pOut);
		p = p->h_next;
		break;
	}

	cvReleaseMemStorage(&storage);
}


//                                        p3  p2  p1
//**********ʹ��zhang���п����㷨����ϸ�� p4  p   p0
//                                        p5  p6  p7
void CImgPreprocess::ZhangThinning(int w,int h,unsigned char *imgBuf)
{
    int neighbor[8];
 
    unsigned char *mark=new unsigned char[w*h];
    memset(mark,0,w*h);
 
    bool loop=true;
    int x,y,k;
    int markNum=0;
 
    while(loop)
    {
       loop=false;
 
       //��һ��
       markNum=0;
       for(y=1;y<h-1;y++)
       {
           for(x=1;x<w-1;x++)
           {
              //����1��p������ǰ����
              if(imgBuf[y*w+x]==0 ) continue;
 
              neighbor[0]= imgBuf[y*w+x+1] ;
              neighbor[1]= imgBuf[(y-1)*w+x+1];
              neighbor[2]= imgBuf[(y-1)*w+x];
              neighbor[3]= imgBuf[(y-1)*w+x-1];
              neighbor[4]= imgBuf[y*w+x-1];
              neighbor[5]= imgBuf[(y+1)*w+x-1];
              neighbor[6]= imgBuf[(y+1)*w+x];
              neighbor[7]= imgBuf[(y+1)*w+x+1];
 
              //����2��2<=N(p��<=6
              int np=(neighbor[0]+neighbor[1]+neighbor[2]+neighbor[3]+neighbor[4]+neighbor[5]+neighbor[6]+neighbor[7])/255;
              if(np<2 || np>6) continue;
 
              //����3��S(p��=1
              int sp=0;
              for(int i=1;i<8;i++)
              {
                  if(neighbor[i]-neighbor[i-1]==255)
                     sp++;
              }
              if(neighbor[0]-neighbor[7]==255)
                  sp++;            
              if(sp!=1) continue;
 
              //����4��p2*p0*p6=0
              if(neighbor[2]&neighbor[0]&neighbor[6]!=0)
                  continue;
                //����5��p0*p6*p4=0
              if(neighbor[0]&neighbor[6]&neighbor[4]!=0)
                  continue;
 
 
              //���ɾ��
              mark[w*y+x]=1;   
              markNum++;
              loop=true;
           }
       }
 
       //�����ɾ���ĵ���Ϊ����ɫ
       if(markNum>0)
       {
           for(y=0;y<h;y++)
           {
              for(x=0;x<w;x++)
              {
                  k=y*w+x;
                  if(mark[k]==1)
                  {
                     imgBuf[k]=0;
                  }
              }
           }
       }
      
 
       //�ڶ���
        markNum=0;
       for(y=1;y<h-1;y++)
       {
           for(x=1;x<w-1;x++)
           {
              //����1��p������ǰ����
              if(imgBuf[y*w+x]==0 ) continue;
 
              neighbor[0]= imgBuf[y*w+x+1] ;
              neighbor[1]= imgBuf[(y-1)*w+x+1];
              neighbor[2]= imgBuf[(y-1)*w+x];
              neighbor[3]= imgBuf[(y-1)*w+x-1];
              neighbor[4]= imgBuf[y*w+x-1];
              neighbor[5]= imgBuf[(y+1)*w+x-1];
              neighbor[6]= imgBuf[(y+1)*w+x];
              neighbor[7]= imgBuf[(y+1)*w+x+1];
 
              //����2��<=N(p)<=6
              int np=(neighbor[0]+neighbor[1]+neighbor[2]+neighbor[3]+neighbor[4]+neighbor[5]+neighbor[6]+neighbor[7])/255;
              if(np<2 || np>6) continue;
 
              //����3��S(p)=1
              int sp=0;
              for(int i=1;i<8;i++)
              {
                  if(neighbor[i]-neighbor[i-1]==255)
                     sp++;
              }
              if(neighbor[0]-neighbor[7]==255)
                  sp++;
              if(sp!=1) continue;
 
              //����4��p2*p0*p4==0
              if(neighbor[2]&neighbor[0]&neighbor[4]!=0)
                  continue;
              //����5��p2*p6*p4==0
              if(neighbor[2]&neighbor[6]&neighbor[4]!=0)
                  continue;
 
              //���ɾ��
              mark[w*y+x]=1;   
              markNum++;
              loop=true;
           }
       }
 
       //�����ɾ���ĵ���Ϊ����ɫ
       for(y=0;y<h;y++)
       {
           for(x=0;x<w;x++)
           {
              k=y*w+x;
              if(mark[k]==1)
              {
                  imgBuf[k]=0;
              }
           }
       }
 
    } 
}

int CImgPreprocess::excutePreprocess(const char *pFile)
{
	if(pFile == nullptr){
		return -1;
	}

	IplImage *pSrc = cvLoadImage(pFile);
	if(pSrc == nullptr){
		return -2;
	}

	cvRectangle(pSrc, cvPoint(0, 0), cvPoint(pSrc->width-1, pSrc->height-1), CV_RGB(255, 255, 255));

	IplImage *pGray = cvCreateImage(cvGetSize(pSrc), IPL_DEPTH_8U, 1);
	//cvCvtColor(pSourec, pGray, CV_BGR2GRAY);
	excuteGray(pSrc, pGray, 'a');
	//filterImg(pGray);

	/*IplImage *pHist = showHistImage(&pGray);
	cvNamedWindow("pHist");
	cvShowImage("pHist", pHist);*/

	//��ֵ�˲�
	IplImage *pImg = cvCreateImage(cvGetSize(pGray), IPL_DEPTH_8U, 1);
	Mat iMat(pGray);
	Mat oMat(pImg);
	medianBlur(iMat,oMat,3);
	//cvNamedWindow("pGray");
	//cvShowImage("pGray", pGray);
	//cvNamedWindow("medianBlur");
	//cvShowImage("medianBlur", pImg);

	int t = OtsuThreshold(pImg->height, pImg->widthStep, (unsigned char*)pImg->imageData);
	int t_ = Otsu(pGray);
	cvReleaseImage(&pImg);
	std::cout<<"threshold="<<t<<"��threshold2="<<t_<<std::endl;
	//opencv ��ֵ��
	//int t = OtsuThreshold(pGray->height, pGray->widthStep, (unsigned char*)pGray->imageData);
	//int t_ = Otsu(pGray);
	//cvThreshold(&pFilt, &pFilt, 0, 255, CV_THRESH_OTSU);
	cvThreshold(pGray, pGray, t, 255, CV_THRESH_BINARY_INV);
	//inverseImage(pGray);
	//std::cout<<"threshold="<<t<<std::endl;

	CvSize eSize;
	eSize.height = pGray->height;
	eSize.width = pGray->widthStep + 4;
	IplImage *pExpand = cvCreateImage(eSize, pGray->depth, pGray->nChannels);
	expandImage(pGray, pExpand, true);
	cvReleaseImage(&pGray);
	
	cvNamedWindow("pExpand");
	cvShowImage("pExpand", pExpand);

	IplImage *pEdge = nullptr;
	edgeDetection(pExpand, &pEdge);

	cvNamedWindow("pEdge");
	cvShowImage("pEdge", pEdge);

	float theate = 0.0;
	radonTransformer(pEdge, theate);

	roatImage(pEdge, theate);
	cvNamedWindow("rota");
	cvShowImage("rota", pEdge);

	IplImage *pSeg = nullptr;
	segmenImg(pExpand, &pSeg);

	cvNamedWindow("pSeg");
	cvShowImage("pSeg", pSeg);

	ZhangThinning(pSeg->widthStep,pSeg->height,
				 reinterpret_cast<unsigned char*>(pSeg->imageData));

	cvNamedWindow("thinning");
	cvShowImage("thinning", pSeg);

	//float theate = 0.0;
	//radonTransformer(pSeg, theate);

	//roatImage(pSeg, theate);
	//cvNamedWindow("rota");
	//cvShowImage("rota", pSeg);

	cvWaitKey();

	return 0;
}

