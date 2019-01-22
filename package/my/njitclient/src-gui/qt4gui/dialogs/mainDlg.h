#ifndef __MAINDLG_H__
#define __MAINDLG_H__

#include "main.h"

class MainDlg : public QDialog
{
	Q_OBJECT
public:
	MainDlg(QWidget *parent = 0,Qt::WindowFlags f=0);
	~MainDlg();
	void getInfo();
public:
	QLabel *userLabel;
	QLineEdit *userEdit;
	QLabel *passwdLabel;
	QLineEdit *passwdEdit;
	QCheckBox *autoStart;
	QCheckBox *savePasswdBox;
	
	QLabel *netcardLabel;
	QComboBox *netcardEdit;
	QLabel *stateLabel;
	QTextEdit *stateText;
	QLabel *copyRight;
	
	QLabel *labelIcon;
	QPushButton *start;
	QPushButton *stop;
	QPushButton *reset;
	QPushButton *version;
private:
	QGridLayout *leftLayout;
	QVBoxLayout *rightLayout;
	QHBoxLayout *bottomLayout;
	QGridLayout *mainLayout;
	
public:
	void initDlg();
	void createLeftLayout();
	void createRightLayout();
	void createBottomLayout();
	void createMainLayout();
public slots:
	void startNet();
	void stopNet();
	void showVersion();
	void resetInfo();
	void saveInfo();
};

#endif // __MAINDLG_H__
