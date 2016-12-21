#include "computervisionsystem.h"

#include <QFileDialog>
#include <QByteArray>

#include "ImgPreprocess.h"

ComputerVisionSystem::ComputerVisionSystem(QWidget *parent)
	: QMainWindow(parent)
{
	m_pImgPreprocess = nullptr;
	ui.setupUi(this);

	LoadImgprocess();
	CreateUi();
}

ComputerVisionSystem::~ComputerVisionSystem()
{
	UncreateUi();
	UnloadImgprocess();
}

int ComputerVisionSystem::CreateUi()
{
	connect(ui.OpenImg_PB,&QPushButton::clicked, this, &ComputerVisionSystem::openimg_slot);
	connect(ui.Discern_PB,&QPushButton::clicked, this, &ComputerVisionSystem::discern_slot);
	connect(ui.Train_PB,&QPushButton::clicked, this, &ComputerVisionSystem::train_slot);
	return 0;
}

void ComputerVisionSystem::UncreateUi()
{
	disconnect(ui.OpenImg_PB,&QPushButton::clicked, this, &ComputerVisionSystem::openimg_slot);
	disconnect(ui.Discern_PB,&QPushButton::clicked, this, &ComputerVisionSystem::discern_slot);
	disconnect(ui.Train_PB,&QPushButton::clicked, this, &ComputerVisionSystem::train_slot);
}

int ComputerVisionSystem::LoadImgprocess()
{
	if(nullptr == m_pImgPreprocess){
		m_pImgPreprocess = new CImgPreprocess;
	}

	return 0;
}

void ComputerVisionSystem::UnloadImgprocess()
{
	if(m_pImgPreprocess){
		delete m_pImgPreprocess;
		m_pImgPreprocess = nullptr;
	}
}

int ComputerVisionSystem::StartUpImgprocess()
{
	return 0;
}

void ComputerVisionSystem::openimg_slot()
{
	QString file = QFileDialog::getOpenFileName(this, tr("Open Img"),
				"./", tr("img (*.png *.jpg *.bmp)"));

	if(file.isEmpty()){
		ui.DebugInfo_TB->setPlainText("file not be found!\n");
		return;
	}

	QByteArray ba = file.toLatin1();
	char *pImg = ba.data();
	//char *pImg = file.toLocal8Bit().data(); //一步完成可能会出错

	if(m_pImgPreprocess){
		m_pImgPreprocess->excutePreprocess(pImg);
	}
}

void ComputerVisionSystem::discern_slot()
{

}

void ComputerVisionSystem::train_slot()
{

}