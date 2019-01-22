#include <iostream>
#include "mainDlg.h"
//#include "main.h"
//#include "auth.h"
//#include "thread.h"
//#include "debug.h"

//extern pthread_t pthread;
extern USER userinfo;
MainDlg::MainDlg(QWidget *parent, Qt::WindowFlags f):QDialog(parent,f)
{
	setWindowTitle(tr("xClient 802.1x"));
	
	Qt::WindowFlags flags = Qt::Window;
	flags |= Qt::WindowMinimizeButtonHint;
	flags |= Qt::WindowCloseButtonHint;
	setWindowFlags(flags);

	initDlg();
	createLeftLayout();
	createRightLayout();
	createBottomLayout();
	createMainLayout();
	setLayout(mainLayout);
	
	getInfo();
	
	connect(version,SIGNAL(clicked()),this,SLOT(showVersion()));
	connect(reset,SIGNAL(clicked()),this,SLOT(resetInfo()));
	connect(savePasswdBox,SIGNAL(toggled(bool)),\
			this,SLOT(saveInfo()));
	connect(start,SIGNAL(clicked()),this,SLOT(startNet()));
	connect(stop,SIGNAL(clicked()),this,SLOT(stopNet()));
}

MainDlg::~MainDlg()
{
	
}

void MainDlg::getInfo()
{
	QFile file("info.dat");
	file.open(QIODevice::ReadOnly);
	QDataStream in(&file);
	
	in.setVersion(QDataStream::Qt_4_0);
	qint32 magic;
	in>>magic;
	if (magic != (qint32)0xa1a2a3a4)
	{
		QMessageBox::information(this,"Tips",tr("Invalide file format"));
		return;
	}
	
	QString userName;
	QString passwd;
	int netcard;
	int startAuto;
	int savePasswd;
	in>>userName>>passwd>>netcard>>startAuto>>savePasswd;
	
	userEdit->setText(userName);
	passwdEdit->setText(passwd);
	//passwdEdit->setEchoMode(QLineEdit::Password);
	netcardEdit->setCurrentIndex(netcard);
	autoStart->setCheckState((startAuto != 0)?Qt::Checked:Qt::Unchecked);
	savePasswdBox->setCheckState((savePasswd != 0)?Qt::Unchecked:Qt::Checked);
	
}

void MainDlg::initDlg()
{
	userLabel = new QLabel(tr("UserName"));
	passwdLabel = new QLabel(tr("Password"));
	netcardLabel = new QLabel(tr("NetCard"));
	stateLabel = new QLabel(tr("Status"));

	userEdit = new QLineEdit;
	passwdEdit = new QLineEdit;
	passwdEdit->setEchoMode(QLineEdit::Password);
	autoStart = new QCheckBox(tr("Auto Start"));
	savePasswdBox = new QCheckBox(tr("Save Passwd"));
	netcardEdit = new QComboBox;
	netcardEdit->insertItem(0,tr("eth0"));
	netcardEdit->insertItem(1,tr("eth1"));
	stateText = new QTextEdit;
	stateText->setText("Net state...");
	copyRight = new QLabel(tr("Author:WangDiwen(From njit) Date:2011-09-01"));
	
	labelIcon = new QLabel();
	QPixmap pic("./imags/pic.png");

	labelIcon->resize(pic.width(),pic.height());
	labelIcon->setPixmap(pic);
	
	start = new QPushButton();
	start->setText(tr("StartNet"));
	stop = new QPushButton();
	stop->setText(tr("StopNet"));
	stop->setEnabled(false);
	reset = new QPushButton();
	reset->setText(tr("ResetNet"));
	version = new QPushButton();
	version->setText(tr("Version"));
}

void MainDlg::createLeftLayout()
{
	leftLayout = new QGridLayout();
	int labelCol = 0;
	int contentCol1 = 1;

	leftLayout->addWidget(userLabel,0,labelCol);
	leftLayout->addWidget(userEdit,0,contentCol1);

	
	leftLayout->addWidget(passwdLabel,1,labelCol);
	leftLayout->addWidget(passwdEdit,1,contentCol1);

	
	leftLayout->addWidget(netcardLabel,2,labelCol);
	leftLayout->addWidget(netcardEdit,2,contentCol1);

	
	leftLayout->addWidget(autoStart,3,labelCol);
	leftLayout->addWidget(savePasswdBox,3,contentCol1);

	
	leftLayout->addWidget(stateLabel,4,labelCol,Qt::AlignTop);
	leftLayout->addWidget(stateText,4,contentCol1);
	
	leftLayout->setColumnStretch(0,1);
	leftLayout->setColumnStretch(1,3);

}

void MainDlg::createRightLayout()
{
	rightLayout = new QVBoxLayout();
	
	rightLayout->setMargin(10);
	rightLayout->setSpacing(10);
	
	rightLayout->addWidget(start);
	rightLayout->addWidget(stop);
	rightLayout->addWidget(reset);
	rightLayout->addWidget(version);
	rightLayout->addStretch();
	rightLayout->addWidget(labelIcon);
	//rightLayout->setSizeConstraint(QLayout::SetFixedSize);
	//rightLayout->setAlignment(Qt::AlignTop);
}


void MainDlg::createBottomLayout()
{
	bottomLayout = new QHBoxLayout();
	bottomLayout->addWidget(copyRight);
}

void MainDlg::createMainLayout()
{
	mainLayout = new QGridLayout(this);
	
	mainLayout->setMargin(10);
	mainLayout->setSpacing(10);
	mainLayout->addLayout(leftLayout,0,0);
	mainLayout->addLayout(rightLayout,0,1);
	mainLayout->addLayout(bottomLayout,1,0);
	mainLayout->setSizeConstraint(QLayout::SetFixedSize);
}

void MainDlg::startNet()
{
	QString user = userEdit->text();
	QString pass = passwdEdit->text();
	QString net = netcardEdit->currentText();

	strcpy(userinfo.username,user.toLatin1().data());
	strcpy(userinfo.passwd,pass.toLatin1().data());
	strcpy(userinfo.netcard,net.toLatin1().data());
	
	//DPRINTF("user: %s\n",userinfo.username);
	//DPRINTF("pass: %s\n",userinfo.passwd);
	//DPRINTF("net: %s\n",userinfo.netcard);
	//Modified by WangDiwen(njit) for test;
	/**	
	USER userinfo;
	userinfo.username = "s-208080236";
	userinfo.passwd = "126333";
	userinfo.netcard = "eth0";
	*/
	int ret = -1/*pthread_create(&(pthread), NULL, thread_func, (void *)&userinfo)*/;
	if (ret != 0)
	{
		//exit(ret);
		std::cerr<<"create thread failed...\n";
	}
	std::clog<<"create thread success...\n";
	stateText->setText("Connect Server Success!\nIf your net has not connected yet,\nPlease clicked the <startNet> button!\n");
	start->setEnabled(false);
	stop->setEnabled(true);
	userEdit->setEnabled(false);
	passwdEdit->setEnabled(false);
	netcardEdit->setEnabled(false);
	//pthread_join(pthread,NULL);
}

void MainDlg::stopNet()
{
	//pthread_cancel(pthread);
	start->setEnabled(true);
	userEdit->setEnabled(true);
	passwdEdit->setEnabled(true);
	netcardEdit->setEnabled(true);
	stateText->setText("Stop Connect!");
}

void MainDlg::showVersion()
{
	QMessageBox::information(this,"Version",tr("xClient 802.1x for Linux!\nBased on QT4.\n\nAuthor:WangDiwen(From njit)\nEmail:dw_wang126@126.com"));
	return;
}
void MainDlg::resetInfo()
{
	userEdit->setText(tr(""));
	passwdEdit->setText(tr(""));
	netcardEdit->setCurrentIndex(0);
	stateText->setText(tr(""));
	return;
}
void MainDlg::saveInfo()
{
	QString userName = userEdit->text();
	QString passwd = passwdEdit->text();
	int netcard = netcardEdit->currentIndex();
	int startAuto = autoStart->isChecked()? 1 : 0;
	int savePasswd = savePasswdBox->isChecked()? 1 : 0;
	
	QFile file("info.dat");
	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);
	
	out.setVersion(QDataStream::Qt_4_0);
	out<<(qint32)0xa1a2a3a4;
	out<<userName<<passwd<<netcard<<startAuto<<savePasswd;
}


void *thread_func(void *arg)
{	
//	struct USER *user = (struct USER *)arg;
	int ret = -1/*Authentication(user->username,user->passwd,user->netcard)*/;
	if (ret == -1)
	{
		std::cerr<<"Authentication failed...\n";
	}
	std::clog<<"Authentication quit...\n";
//	pthread_exit(NULL);
	return NULL;
}

