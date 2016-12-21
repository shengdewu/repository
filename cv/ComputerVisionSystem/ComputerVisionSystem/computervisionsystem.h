#ifndef COMPUTERVISIONSYSTEM_H
#define COMPUTERVISIONSYSTEM_H

#include <QtWidgets/QMainWindow>
#include "ui_computervisionsystem.h"

class CImgPreprocess;
class ComputerVisionSystem : public QMainWindow
{
	Q_OBJECT

public:
	ComputerVisionSystem(QWidget *parent = 0);
	~ComputerVisionSystem();

private:
	Ui::ComputerVisionSystemClass ui;

	CImgPreprocess *m_pImgPreprocess;

	int CreateUi(void);
	void UncreateUi(void);

	int LoadImgprocess(void);
	void UnloadImgprocess(void);

	int StartUpImgprocess(void);

private slots:
	void openimg_slot(void);
	void discern_slot(void);
	void train_slot(void);

};

#endif // COMPUTERVISIONSYSTEM_H
