#include "radon.h"
#include <iostream>

using std::cout;
using std::cin;
using std::endl;

#define pi 3.1415926
#define MAXX(x,y) ((x) > (y) ? (x) : (y))
#define MINX(x,y) ((x) > (y) ? (y):(x))
//#define ROUND(X) (((X-(int)X)>=0.5)?((int)X+1):((int)X))
//#define floor(X) (X>0)?(int(X)):((int)X-1)

typedef struct _radon_data{
	_radon_data(){
		_pix_nums = 0.0;
		_theta1 = 0.0;
		_distance = 0.0;
	}

	_radon_data & operator=(const _radon_data &value){
		this->_pix_nums = value._pix_nums;
		this->_theta1 = value._theta1;
		this->_distance = value._distance;
		return *this;
	}

	double	 _pix_nums;
	double   _theta1;
	double   _distance;
}radon_data;

void swap(radon_data *a, radon_data *b){
	radon_data temp = *a;
	*a = *b;
	*b = temp;
}

int partition(radon_data*	value, int begin, int end){
	double key = value[end]._pix_nums;
	int retIndex = begin - 1;
	int index;
	for(index = begin; index < end; index++){
		if(value[index]._pix_nums < key){
			retIndex += 1;
			swap(&value[retIndex], &value[index]);
		}
	}
	retIndex +=1 ;
	if(end < 5055){
		cout<<"ret index = "<<retIndex<<",pix_num="<<value[retIndex]._pix_nums<<",distance="<<value[retIndex]._distance<<",thetal="<<value[retIndex]._theta1<<endl;
		cout<<"end index = "<<end<<",pix_num="<<value[end]._pix_nums<<",distance="<<value[end]._distance<<",thetal="<<value[end]._theta1<<endl;
	}
	swap(&value[retIndex], &value[end]);

	return retIndex;
}

void quick_sort(radon_data*	value, int begin, int end){
	if( begin >= end){
		return;
	}

	int pInt = partition(value, begin, end);
#ifdef _USER_DEBUG
	if(begin < pInt)
		cout<<"left begin="<<begin<<",end="<<pInt-1<<endl;
#endif
	quick_sort(value, begin, pInt-1);
#ifdef _USER_DEBUG
	if(pInt+1 < end)
		cout<<"right begin="<<pInt+1<<",end="<<end<<endl;
#endif
	quick_sort(value, pInt+1, end);
}

int ROUND(double x)
{
	int ret ;
	if(x>2.00e+08)
		x=2.00e+08 ;
	if(x<-2.00e+08)
		x=-2.00e+08 ;
	if(x>0) 
	{
		ret=((x-((int)x))>=0.5)?((int)x+1):((int)x) ;
	}else
	{
		ret=((x-(int)x)<=-0.5)?((int)x-1):((int)x) ;
	}
//	ret=cvRound(x) ;
	return ret ;
}



unsigned char max(unsigned char a,unsigned char b)
{
	return a>b?a:b ;
}

void incrementRadon(double *pr, double pixel, double r)
{
    int r1;
    double delta;

    r1 = (int) r;
    delta = r - r1;
    pr[r1] += pixel * (1.0 - delta);
    pr[r1+1] += pixel * delta;
}

 void radon1(double *pPtr, unsigned char *iPtr, double *thetaPtr, int M, int N, 
      int xOrigin, int yOrigin, int numAngles, int rFirst, int rSize)
{
    int k, m, n;              /* loop counters */
    double angle;             /* radian angle value */
    double cosine, sine;      /* cosine and sine of current angle */
    double *pr;               /* points inside output array */
    unsigned char *pixelPtr;         /* points inside input array */
    double pixel;             /* current pixel value */
    double *ySinTable, *xCosTable;
    /* tables for x*cos(angle) and y*sin(angle) */
    double x,y;
    double r, delta;
    int r1;

    /* Allocate space for the lookup tables */
    xCosTable = (double *) malloc(2*N*sizeof(double));
    ySinTable = (double *) malloc(2*M*sizeof(double));

    for (k = 0; k < numAngles; k++) {
        angle = thetaPtr[k];
        pr = pPtr + k*rSize;  /* pointer to the top of the output column */
        cosine = cos(angle); 
        sine = sin(angle);   

        /* Radon impulse response locus:  R = X*cos(angle) + Y*sin(angle) */
        /* Fill the X*cos table and the Y*sin table.                      */
        /* x- and y-coordinates are offset from pixel locations by 0.25 */
        /* spaced by intervals of 0.5. */
        for (n = 0; n < N; n++)
        {
            x = n - xOrigin;
            xCosTable[2*n]   = (x - 0.25)*cosine;
            xCosTable[2*n+1] = (x + 0.25)*cosine;
        }
        for (m = 0; m < M; m++)
        {
            y = yOrigin - m;
            ySinTable[2*m] = (y - 0.25)*sine;
            ySinTable[2*m+1] = (y + 0.25)*sine;
        }

        pixelPtr = iPtr;
        for (n = 0; n< N; n++)
        {
            for (m= 0; m< M; m++)
            {
                //pixel = abs((double)((unsigned char)(*pixelPtr++)));
				pixel=(double)(*pixelPtr++) ;
                if (pixel != 0.0)
                {

                    pixel *= 0.25;


                    r = xCosTable[2*n] + ySinTable[2*m] - rFirst;
                    incrementRadon(pr, pixel, r);

                    r = xCosTable[2*n+1] + ySinTable[2*m] - rFirst;
                    incrementRadon(pr, pixel, r);

                    r = xCosTable[2*n] + ySinTable[2*m+1] - rFirst;
                    incrementRadon(pr, pixel, r);

                    r = xCosTable[2*n+1] + ySinTable[2*m+1] - rFirst;
                    incrementRadon(pr, pixel, r);
                }
            }
        }
    }
                
    free((void *) xCosTable);
    free((void *) ySinTable);
}
void radonc(unsigned char *input,double* output,int mWidth,int mHeight,int numAngels,int rSize,int step) 
{
	int m,n,t,r ,rho;
	int rhomax;//max length of row  ;
	int rc;//the center of row ;
	double x,y;
	
	int mt=numAngels ;
	double deg2rad = 3.14159265358979 / 180.0;
	double costheta,sintheta ;
	double a,b;//y=ax+b ;
	int ymax,ymin,xmax,xmin,xtemp,ytemp,rhooffset ;
	double up,low ;
	int floor1=0 ;
	double uptemp,lowtemp ;
	double ltemp ,utemp;
	int mStep=step ;
	m=ROUND(mWidth/2) ;
	n=ROUND(mHeight/2) ;

	rhomax=rSize ;
	rc=ROUND(rhomax/2) ;
	for(t=1;t<46;t++)
	{
		costheta=cos(t*deg2rad) ;
		sintheta=sin(t*deg2rad) ;
		a=-costheta/sintheta ;//y=ax+b ;//note that here can rhoptimized ;
		for(r=0;r<rhomax;r++)
		{
			rho=r-rc +1;
			b=rho/sintheta ;
			ymax=MINX((int)(ROUND(-a*m+b)),(n-1)) ;
			ymin=MAXX((int)(ROUND(a*m+b)),(-n)) ;
			for(ytemp=ymin;ytemp<ymax+1;ytemp++)
			{
				x=(ytemp-b)/a ;
				xtemp=floor(x) ;
				floor1=xtemp ;
				up=x-floor1 ;
				low=1-up ;

				xtemp=MAXX(floor1,-m) ;
				xtemp=MINX(xtemp,(m-2)) ;
				output[(rhomax-r)*mt+mt-t]+=low*(double)(unsigned char)input[(ytemp+n)*mStep+xtemp+m]; ;
				output[(rhomax-r)*mt+mt-t]+=up*(double)(unsigned char)input[(ytemp+n)*mStep+xtemp+m+1] ;
			}
		}
	}
	for(t=46;t<91;t++)
	{
		costheta=cos(t*deg2rad) ;
		sintheta=sin(t*deg2rad) ;
		a=-costheta/sintheta ;//y=ax+b ;//note that here can rhoptimized ;
		for(r=0;r<rhomax;r++)
		{
			rho=r-rc+1 ;
			b=rho/sintheta ;
			xmax=MINX((int)(ROUND((-n-b)/a)),(m-1)) ;
			xmin=MAXX((int)(ROUND((n-b)/a)),(-m)) ;

			for(xtemp=xmin;xtemp<xmax+1;xtemp++)
			{
				y=(a*xtemp+b) ;
				ytemp=floor(y) ;
				floor1=ytemp ;
				up=y-floor1;

				low=1-up ;
				ytemp=MAXX(floor1,(-n)) ;
				ytemp=MINX(ytemp,(n-2)) ;
				output[(rhomax-r)*mt+mt-t]+=low*(double)(unsigned char)input[(ytemp+n)*mStep+xtemp+m];
				output[(rhomax-r)*mt+mt-t]+=up*(double)(unsigned char)input[(ytemp+n+1)*mStep+xtemp+m];
			}
		}
	}
	for( t=91;t<136;t++)
	{
		costheta=cos(t*deg2rad) ;
		sintheta=sin(t*deg2rad) ;
		a=-costheta/sintheta ;//y=ax+b ;//note that here can rhoptimized ;
		for(r=0;r<rhomax;r++)
		{
			rho=r-rc+1 ;
			b=rho/sintheta ;
			xmax=MINX((int)(ROUND((n-b)/a)),m-1) ;
			xmin=MAXX((int)(ROUND((-n-b)/a)),(-m)) ;
			for(xtemp=xmin;xtemp<xmax+1;xtemp++)
			{
				y=(a*xtemp+b) ;
				ytemp=floor(y) ;
				floor1=ytemp ;
				up=y-floor1 ;
				low=1-up ;

				ytemp=MAXX(floor1,(-n)) ;
				ytemp=MINX(ytemp,(n-2)) ;
				output[(rhomax-r)*mt+mt-t]+=low*(double)(unsigned char)input[(ytemp+n)*mStep+xtemp+m]; 
				output[(rhomax-r)*mt+mt-t]+=up*(double)(unsigned char)input[(ytemp+n+1)*mStep+xtemp+m]; 
			}
		}

	}
	for(t=136;t<180;t++)
	{
		costheta=cos(t*deg2rad) ;
		sintheta=sin(t*deg2rad) ;
		a=-costheta/sintheta ;//y=ax+b ;//note that here can rhoptimized ;
		for(r=0;r<rhomax;r++)
		{
			rho=r-rc+1 ;
			b=rho/sintheta ;
			ymax=MINX((ROUND(a*m+b)),(n-1)) ;
			ymin=MAXX((ROUND(-a*m+b)),(-n)) ;
			for(ytemp=ymin;ytemp<ymax+1;ytemp++)
			{
				x=(ytemp-b)/a ;
				xtemp=floor(x) ;
				
				floor1=xtemp ;
				up=x-floor1 ;
				low=1-up ;

				xtemp=MAXX(floor1,(-m)) ;
				xtemp=MINX(xtemp,(m-2)) ;
				output[(rhomax-r)*mt+mt-t]+=low*(double)(unsigned char)input[(ytemp+n)*mStep+xtemp+m];
				output[(rhomax-r)*mt+mt-t]+=up*(double)(unsigned char)input[(ytemp+n)*mStep+xtemp+m+1];
			}
		}
	}

		t=0 ;
		rhooffset=ROUND(((rhomax-mWidth)/2)) ;
		for(xtemp=0;xtemp<mWidth;xtemp++)
		{
			r=xtemp+rhooffset+1 ;
			r=rhomax-r+1 ;
			for(ytemp=0;ytemp<mHeight;ytemp++)
			{
				output[r*mt+t]+=(double)(unsigned char)input[ytemp*mStep+xtemp] ;
			}
		}
}
IplImage *RadonTest(IplImage *pSrc)
{
	int numAngles;          /* number of theta values */
    double *thetaPtr;       /* pointer to theta values in radians */
    double *pr1, *pr2;      /* double pointers used in loop */
    double deg2rad;         /* conversion factor */
    double temp;            /* temporary theta-value holder */
    int k;                  /* loop counter */
    int M, N;               /* input image size */
    int xOrigin, yOrigin;   /* center of image */
    int temp1, temp2;       /* used in output size computation */
    int rFirst, rLast;      /* r-values for first and last row of output */
    int rSize;              /* number of rows in output */
	IplImage *pRadon ;
    /* Get THETA values */
    deg2rad = 3.14159265358979 / 180.0;
    numAngles = 180 ;
    thetaPtr = (double *) malloc(numAngles*sizeof(double));
   // pr1 = mxGetPr(THETA);
    pr2 = thetaPtr;
    for (k = 0; k < numAngles; k++)
        *(pr2++) = k * deg2rad;
	N=pSrc->height ;
	M=pSrc->width ;

	    /* Where is the coordinate system's origin? */
    xOrigin = MAXX(0, (N-1)/2);
    yOrigin = MAXX(0, (M-1)/2);

	    /* How big will the output be? */
    temp1 = M - 1 - yOrigin;
    temp2 = N - 1 - xOrigin;
    rLast = (int) ceil(sqrt((double) (temp1*temp1+temp2*temp2))) + 1;
    rFirst = -rLast;
    rSize = rLast - rFirst + 1;
	pRadon=cvCreateImage(cvSize(rSize,numAngles),IPL_DEPTH_64F,1) ;
	
	radon1((double*)(pRadon->imageData), (unsigned char*)(pSrc->imageData), thetaPtr, M, N, xOrigin, yOrigin, 
              numAngles, rFirst, rSize);
	return pRadon ;
}
IplImage *Radon(IplImage *pSrc)
{
	IplImage *pRadon=NULL ;
	int mWidth ,mHeight,mStep  ;
	int len,numAngels ;
	double *pRadonData ;
	//unsigned char *pSrcData ;
	if(pSrc==NULL)
		return pRadon;
	if(pSrc->nChannels!=1) 
		return pRadon;
	mWidth=pSrc->width ;
	mHeight=pSrc->height ;
	mStep=pSrc->widthStep ;
	
	len=(int)ceil(sqrt((double)(mWidth*mWidth+mHeight*mHeight)))+1 ;
	numAngels=180 ;
	pRadon=cvCreateImage(cvSize(numAngels,len),IPL_DEPTH_64F,1) ;
	pRadonData=(double*)(pRadon->imageData) ;
	radonc((unsigned char*)(pSrc->imageData),pRadonData,mWidth,mHeight,180,len-1,mStep) ;
	return pRadon ;
}

void GetMax(IplImage *pRadon,double **info)
{
	double theta1,theta2,len1,len2 ;
	int mWidth,mHeight,mStep ;
	int i,j ;
	double temp1,temp2 ,temp;
	double *pData ;

	mWidth=pRadon->width ;
	mHeight=pRadon->height ;
	mStep=pRadon->widthStep ;
	pData=(double*)(pRadon->imageData) ;

	temp1=temp2=0.0 ;
	theta1=theta2=len1=len2=0.0 ;

	for(i=0;i<mHeight;i++)
	{
		for(j=0;j<mWidth/2;j++)
		{
			temp=abs(pData[i*mWidth+j]);
			if(temp>temp1)
			{
				temp1=temp ;
				theta1=j ;
				len1=i ;
			}
		}
		for(j=mWidth/2;j<mWidth;j++)
		{
			temp=abs(pData[i*mWidth+j]) ;
			if(temp>temp2)
			{
				temp2=temp;
				theta2=j ;
				len2=i ;
			}
		}
	}

	cout<<"GetMax==============="<<endl;
	cout<<"function: thetal="<<theta1<<",pix_num="<<temp1<<",distance="<<len1<<";"<<endl;
	cout<<"function: thetal="<<theta2<<",pix_num="<<temp2<<",distance="<<len2<<";"<<endl;

	if(nullptr != info){
		(*info)[0]=theta1 ;
		(*info)[1]=len1 ;
		(*info)[2]=theta2 ;
		(*info)[3]=len2 ;
		(*info)[4]=mHeight ;
		(*info)[5]=temp2 ;
	}
}
void GetMaxThetal(IplImage *pRadon, double *thetal)
{
	int mWidth,mHeight,mStep ;
	int i,j ;
	double *pData ;

	mWidth=pRadon->width ;
	mHeight=pRadon->height ;
	mStep=pRadon->widthStep ;
	pData=(double*)(pRadon->imageData) ;


	int total = mWidth * mHeight;
	radon_data *value = new radon_data[total];
	double tmp = 0;
	int cnt = 0;
	for(i=0;i<mHeight;i++)
	{
		for(j=0;j<mWidth;j++)
		{
			cnt = i * mWidth + j;
			tmp = abs(pData[cnt]);
			value[cnt]._pix_nums= tmp;
			value[cnt]._distance=i;
			value[cnt]._theta1=j;
		}
	}
#ifdef _USER_DEBUG
	cout<<"cnt = "<<cnt<<endl;
	cout<<"quick sort ilter = "<<total - 1<<endl;
#endif
	quick_sort(value, 0,  total - 1);

	cout<<"GetMaxThetal==============="<<endl;
	int num = total -1;
	cout<<"the first: thetal="<<value[num]._theta1<<",pix_num="<<value[num]._pix_nums<<",distance="<<value[num]._distance<<";"<<endl;
	num = num - 1;
	cout<<"the second: thetal="<<value[num]._theta1<<",pix_num="<<value[num]._pix_nums<<",distance="<<value[num]._distance<<";"<<endl;
	num = num - 1;
	cout<<"the third: thetal="<<value[num]._theta1<<",pix_num="<<value[num]._pix_nums<<",distance="<<value[num]._distance<<";"<<endl;
	num = num - 1;
	cout<<"the foruth: thetal="<<value[num]._theta1<<",pix_num="<<value[num]._pix_nums<<",distance="<<value[num]._distance<<";"<<endl;
	num = num - 1;
	cout<<"the fiveth: thetal="<<value[num]._theta1<<",pix_num="<<value[num]._pix_nums<<",distance="<<value[num]._distance<<";"<<endl;

	/*int t_height = 50;
	int t_width =  180;
	int t_total = t_width * t_height;
	int t_size = t_total * sizeof(radon_data);
	radon_data *t_value = new radon_data[t_total];
	double t_tmp = 0;
	int t_cnt = 0;
	for(i=0;i<t_height;i++)
	{
		for(j=0;j<t_width;j++)
		{
			t_cnt = i * t_width + j;
			t_tmp = i * t_width + j;
			t_value[t_cnt]._pix_nums= t_tmp;
			t_value[t_cnt]._distance=i;
			t_value[t_cnt]._theta1=j;
		}
	}

	quick_sort(t_value, 0,  t_total - 1);*/

	*thetal = 0;

	delete []value;
}
void GetMaxTest(IplImage *pRadon,double **info)
{
	double theta1,theta2,len1,len2 ;
	int mWidth,mHeight,mStep ;
	int i,j ;
	double temp1,temp2 ,temp;
	double *pData ;
	
	mWidth=pRadon->width ;
	mHeight=pRadon->height ;
	mStep=pRadon->widthStep ;
	pData=(double*)(pRadon->imageData) ;

	temp1=temp2=0.0 ;
	theta1=theta2=len1=len2=0.0 ;
	for(i=0;i<mHeight/2;i++)
	{
		for(j=0;j<mWidth;j++)
		{
			temp=abs(pData[i*mWidth+j]);
			if(temp>temp1)
			{
				temp1=temp ;
				theta1=i ;
				len1=j ;
			}
		}
	}
	for(i=mHeight/2;i<mHeight;i++)
	{
		for(j=0;j<mWidth;j++)
		{
			temp=abs(pData[i*mWidth+j]);
			if(temp>temp2)
			{
				temp2=temp ;
				theta2=i ;
				len2=j ;
			}
		}
	}

	cout<<"theta1="<<theta1<<",len1="<<len1<<",theta2="<<theta2<<",len2="<<len2<<",temp1="<<temp1<<",temp2="<<temp2<<endl;
	if(nullptr != info){
		(*info)[0]=theta1 ;
		(*info)[1]=len1 ;
		(*info)[2]=theta2 ;
		(*info)[3]=len2 ;
		(*info)[4]=temp1 ;
		(*info)[5]=temp2 ;
	}
}

bool radonTransformer(IplImage *pImage, float &fthetal)
{
	//IplImage *pRadon = Radon(pImage);
	//double max_theate = 0.0;
	//GetMaxTheta(pRadon,&max_theate);
	IplImage *pRadon = Radon(pImage);
	double theate = 0;
	GetMax(pRadon, nullptr);
	//GetMaxThetal(pRadon, &theate);

	fthetal = theate;
	//ftheate = -ftheate;

	cvReleaseImage(&pRadon);

	return true;
}