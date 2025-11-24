#include "MainWindow.h"

#include <QCloseEvent>
#include <QToolBar>

MainWindow::MainWindow(QPointer<QSettings> settings, QWidget *parent) :
	QMainWindow(parent), mSettings(settings)
{
	resize(1200, 800);
	readSettings();

	// Add dummy toolbar to create a bap between the file menu and the main window content
	QToolBar *spacer = new QToolBar(this);
	spacer->setObjectName("MainWindowToolBarSpacer");
	spacer->setStyleSheet("QToolBar { border: none; }");
	spacer->setFixedHeight(10); // Adjust for desired gap size
	spacer->setMovable(false);
	spacer->setFloatable(false);
	spacer->setAllowedAreas(Qt::TopToolBarArea);
	addToolBar(Qt::TopToolBarArea, spacer);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	writeSettings();
	event->accept();
}

void MainWindow::writeSettings()
{
	if (!mSettings) { return; }

	mSettings->beginGroup("MainWindow");
	mSettings->setValue("geometry", saveGeometry());
	mSettings->setValue("windowState", saveState());
	mSettings->endGroup();
}

void MainWindow::readSettings()
{
	if (!mSettings) { return; }

	mSettings->beginGroup("MainWindow");
	restoreGeometry(mSettings->value("geometry").toByteArray());
	restoreState(mSettings->value("windowState").toByteArray());
	mSettings->endGroup();
}
