#include <QApplication>
#include "./dialogs/mainDlg.h"
//#include <pthread.h>
//#include "thread.h"

//pthread_t pthread;
USER userinfo={ {'\0'},{'\0'},{'\0'} };

int main(int argc, char **argv)
{
	//QFont f("ZYSong 18030",12);
	//QApplication::setFont(f);
	
	QApplication app(argc,argv);
	MainDlg mainDlg;
	mainDlg.show();
	
//	app.connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );
	return app.exec();
}
