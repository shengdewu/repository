#include "Train.h"
#include "opencvheader.h"

#include <iostream>
using std::cout;
using std::cin;
using std::endl;

//调用方法如下

/*char path[256] ={0};
	char c = 0;

	for(int i=0; i < CHARS_CLASS_COUNTS; i++){
		for(int j=0; j< CHARS_PER_COUNTS; j++){
			int row = j+i*CHARS_PER_COUNTS;
			memset(path, sizeof(path), 0);
			sprintf_s(path,"./source/sample/%d (%d).bmp",i,j+1);
			//sprintf_s(path,"./%d/%d (%d).png",i,i,j+1);
			preprocess(path,row, i);
		}
	}

	//train();
	train_1();*/
	
	/*char start_idenfien[256] = {0};
	cout<<"please start indefine:"<<endl;
	cin>>start_idenfien;
	indefine();*/


CTrain::CTrain(void)
{
	init();
}


CTrain::~CTrain(void)
{
}

void CTrain::init()
{
	memset(inputs, 0.0f, CHARS_ALL_COUNTS*INPUT_VECTOR_DIMS);
	memset(outputs, 0.0f, CHARS_ALL_COUNTS*OUT_VECTOR_DIMS);
}

int CTrain::excuteIndefine()
{
	char path[256] = {0};
	int i_c = 4;
	sprintf_s(path,"./sample/%d (%d).bmp",i_c,5);
	IplImage *pSourec = cvLoadImage(path);
	if(nullptr == pSourec){
		cout<<"features:"<<path<<" is not vailed"<<endl;
		return -1;
	}
	cout<<"start idenfie:" << path<<endl;
#ifdef Debug
	cvNamedWindow("source");
	cvShowImage("source", pSourec);
#endif

	//out
	IplImage *pOut = nullptr;
	//opencv  灰度化
	IplImage *gray = cvCreateImage(cvGetSize(pSourec), IPL_DEPTH_8U, 1);
	cvCvtColor(pSourec, gray, CV_BGR2GRAY);
	//opencv 二值化
	cvThreshold(gray, gray, 175, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

	//去边框
	cvRectangle(gray, cvPoint(0, 0), cvPoint(gray->width-1, gray->height-1), CV_RGB(255, 255, 255));

#ifdef Debug
	cvNamedWindow("cvRectangle");
	cvShowImage("cvRectangle", gray);
#endif

	//去噪
	//IplConvKernel *se = cvCreateStructuringElementEx(2, 2, 1, 1, CV_SHAPE_CROSS);
	//cvDilate(gray, gray, se);

#ifdef Debug
	//cvNamedWindow("IplConvKernel");
	//cvShowImage("IplConvKernel", gray);
#endif

	//计算连通域contoure
	cvXorS(gray, cvScalarAll(255), gray, 0);

#ifdef Debug
	cvNamedWindow("cvXorS");
	cvShowImage("cvXorS", gray);
#endif
	CvMemStorage *storage = cvCreateMemStorage();
	CvSeq *contour = nullptr;
	cvFindContours(gray, storage, &contour, sizeof(CvContour),CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	//分析连通域
	CvSeq *p = contour;
	while(p){
		CvRect rect = cvBoundingRect(p, 0);
		if(rect.height < 10){//图像文字需要15像素高度
			p = p->h_next;
			continue;
		}
		//绘制该连通到character
		cvZero(gray);
		IplImage *character = cvCreateImage(cvSize(rect.width, rect.height), IPL_DEPTH_8U, 1);
		cvZero(character);
		cvDrawContours(character, p, CV_RGB(255, 255, 255), CV_RGB(0, 0, 0), -1, -1, 8, cvPoint(-rect.x, -rect.y));

#ifdef Debug		
		cvNamedWindow("character");
		cvShowImage("character", character);
#endif
		// 归一化
		pOut = cvCreateImage(cvSize(16, 16), IPL_DEPTH_8U, 1);
		cvResize(character, pOut, CV_INTER_AREA);
#ifdef Debug
		cvNamedWindow("cvResize");
		cvShowImage("cvResize", pOut);
#endif
		// 修正
		cvThreshold(pOut, pOut, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

#ifdef Debug
		cvNamedWindow("show");
		cvShowImage("show", pOut);
#endif
		//cvReleaseImage(&character);
		p = p->h_next;
	}

	// 计算输入向量
	float input[256];
	for(int i=0; i<256; i++)
		input[i] = (pOut->imageData[i]==-1);

	// 识别
	CvANN_MLP mlp;
	mlp.load( "mpl.xml" );
	CvMat* output = cvCreateMat( 1, 36, CV_32F );
	CvMat inputMat = cvMat( 1, 256, CV_32F, input);
	mlp.predict( &inputMat, output );

	CvPoint max_loc = {0,0};
	cvMinMaxLoc( output, NULL, NULL, NULL, &max_loc, NULL );
	int best = max_loc.x;// 识别结果
	char c = (char)( best<10 ? '0'+best : 'A'+best-10 );
	cout<<"indefine="<<c<<"<=====>pratics="<<i_c<<endl;
	cvReleaseMat( &output );

	cvReleaseImage(&gray);
	cvReleaseImage(&pOut);
	cvReleaseImage(&pSourec);
	cvReleaseMemStorage(&storage);
	//cvReleaseStructuringElement(&se);
#ifdef Debug
	cvWaitKey(0);
	cvDestroyAllWindows();
#endif 

	return 0;
}

int CTrain::excuteTrain()
{
	// 读入结果responses 特征data
	FILE* f = fopen( "batch", "rb" );
	fseek(f, 0l, SEEK_END);
	long size = ftell(f);
	fseek(f, 0l, SEEK_SET);
	int count = size/4/(36+256);
	CvMat* batch = cvCreateMat( count, 36+256, CV_32F );
	fread(batch->data.fl, size-1, 1, f);
	CvMat outputs, inputs;
	cvGetCols(batch, &outputs, 0, 36);
	cvGetCols(batch, &inputs, 36, 36+256);

	fclose(f);
	// 新建MPL
	CvANN_MLP mlp;
	int layer_sz[] = { 256, 20, 36 };
	CvMat layer_sizes = cvMat( 1, 3, CV_32S, layer_sz );
	mlp.create( &layer_sizes );

	// 训练
	//system( "time" );
	mlp.train( &inputs, &outputs, NULL, NULL,
		CvANN_MLP_TrainParams(cvTermCriteria(CV_TERMCRIT_ITER,300,0.01), CvANN_MLP_TrainParams::RPROP, 0.01)
		);
	//system( "time" );

	// 存储MPL
	mlp.save( "mpl.xml" );

	// 测试
	int right = 0;
	CvMat* output = cvCreateMat( 1, 36, CV_32F );
	for(int i=0; i<count; i++)
	{
		CvMat input;
		cvGetRow( &inputs, &input, i );

		mlp.predict( &input, output );
		CvPoint max_loc = {0,0};
		cvMinMaxLoc( output, NULL, NULL, NULL, &max_loc, NULL );
		int best = max_loc.x;// 识别结果

		int ans = -1;// 实际结果
		for(int j=0; j<36; j++)
		{
			if( outputs.data.fl[i*(outputs.step/4)+j] == 1.0f )
			{
				ans = j;
				break;
			}
		}
		cout<<(char)( best<10 ? '0'+best : 'A'+best-10 );
		cout<<(char)( ans<10 ? '0'+ans : 'A'+ans-10 );
		if( best==ans )
		{
			cout<<"+";
			right++;
		}
		//cin.get();
		cout<<endl;
	}
	cvReleaseMat( &output );
	cout<<endl<<right<<"/"<<count<<endl;

	cvReleaseMat( &batch );

	system( "pause" );
	return 0;
}


void CTrain::saveFeatures(void *pSrc, int row, char c)
{
	IplImage *pIn = reinterpret_cast<IplImage*>(pSrc);
	//输出目标
	unsigned char outKey = 255;
	outKey = c;
	// 计算输入向量
	float input[INPUT_VECTOR_DIMS] = {0.0};
	for(int i=0; i<INPUT_VECTOR_DIMS; i++)
		input[i] = (pIn->imageData[i]==-1);

	// 转换成输出向量
	float output[OUT_VECTOR_DIMS];
	for(int i=0; i<OUT_VECTOR_DIMS; i++)
		output[i] = 0.0f;
	output[c] = 1.0f;

	// 存储到批处理文件
	FILE* batch = fopen("batch", "ab");
	fwrite(output, 4*36, 1, batch);
	fwrite(input, 4*256, 1, batch);
	fclose(batch);
	// 转换成输出向量

	//cout<<row/CHARS_PER_COUNTS<<"obj:"<<c<<endl;
	for(int i=0; i<OUT_VECTOR_DIMS; i++){
		outputs[row][i] = 0.0f;
		if(c == i){
			outputs[row][c] = 1.0f;
		}
	}

	// 计算输入向量
	for(int i=0; i<INPUT_VECTOR_DIMS; i++)
		inputs[row][i] = (pIn->imageData[i]==-1);
}

int CTrain::preprocess(const char *path, int row, char c){

	IplImage *pSourec = cvLoadImage(path);
	if(nullptr == pSourec){
		cout<<"features:"<<path<<" is not vailed"<<endl;
		return -1;
	}
#ifdef Debug
	cvNamedWindow("source");
	cvShowImage("source", pSourec);
#endif

	//out
	IplImage *pOut = nullptr;
	//opencv  灰度化
	IplImage *gray = cvCreateImage(cvGetSize(pSourec), IPL_DEPTH_8U, 1);
	cvCvtColor(pSourec, gray, CV_BGR2GRAY);
	//opencv 二值化
	cvThreshold(gray, gray, 175, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

	//去边框
	cvRectangle(gray, cvPoint(0, 0), cvPoint(gray->width-1, gray->height-1), CV_RGB(255, 255, 255));

#ifdef Debug
	cvNamedWindow("cvRectangle");
	cvShowImage("cvRectangle", gray);
#endif

	//去噪
	//IplConvKernel *se = cvCreateStructuringElementEx(2, 2, 1, 1, CV_SHAPE_CROSS);
	//cvDilate(gray, gray, se);

#ifdef Debug
	//cvNamedWindow("IplConvKernel");
	//cvShowImage("IplConvKernel", gray);
#endif

	//计算连通域contoure
	cvXorS(gray, cvScalarAll(255), gray, 0);

#ifdef Debug
	cvNamedWindow("cvXorS");
	cvShowImage("cvXorS", gray);
#endif
	CvMemStorage *storage = cvCreateMemStorage();
	CvSeq *contour = nullptr;
	cvFindContours(gray, storage, &contour, sizeof(CvContour),CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	//分析连通域
	CvSeq *p = contour;
	while(p){
		CvRect rect = cvBoundingRect(p, 0);
		if(rect.height < 10){//图像文字需要15像素高度
			p = p->h_next;
			continue;
		}
		//绘制该连通到character
		cvZero(gray);
		IplImage *character = cvCreateImage(cvSize(rect.width, rect.height), IPL_DEPTH_8U, 1);
		cvZero(character);
		cvDrawContours(character, p, CV_RGB(255, 255, 255), CV_RGB(0, 0, 0), -1, -1, 8, cvPoint(-rect.x, -rect.y));

#ifdef Debug		
		cvNamedWindow("character");
		cvShowImage("character", character);
#endif
		// 归一化
		pOut = cvCreateImage(cvSize(16, 16), IPL_DEPTH_8U, 1);
		cvResize(character, pOut, CV_INTER_AREA);
#ifdef Debug
		cvNamedWindow("cvResize");
		cvShowImage("cvResize", pOut);
#endif
		// 修正
		cvThreshold(pOut, pOut, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

#ifdef Debug
		cvNamedWindow("show");
		cvShowImage("show", pOut);
#endif
		//cvReleaseImage(&character);
		p = p->h_next;
	}

	if(pOut){
		saveFeatures(pOut, row, c);
		cout<<"features:"<<path<<"is success"<<":height ="<<pOut->height<<",width="<<pOut->width<<endl;
	}
	else
		cout<<"features:"<<path<<" is failed"<<endl;

	cvReleaseImage(&gray);
	cvReleaseImage(&pOut);
	cvReleaseImage(&pSourec);
	cvReleaseMemStorage(&storage);
	//cvReleaseStructuringElement(&se);
#ifdef Debug
	cvWaitKey(0);
	cvDestroyAllWindows();
#endif 

	return 0;
}