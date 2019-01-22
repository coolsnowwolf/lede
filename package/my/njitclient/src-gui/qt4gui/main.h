#ifndef __MAIN_H__
#define __MAIN_H__

#include <QtGui>
//#include <QWidget>
//#include <QDialog>
//#include <QFont>
//#include <QPushButton>
//#include <QLabel>
//#include <QLineEdit>
//#include <QComboBox>
//#include <QCheckBox>
//#include <QTextEdit>
//#include <QGridLayout>
//#include <QVBoxLayout>

struct USER
{
	char username[20];
	char passwd[10];
	char netcard[10];
};
void *thread_func(void * arg);

#endif // __MAIN_H__
