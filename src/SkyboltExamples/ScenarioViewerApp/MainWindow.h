#pragma once

#include <QMainWindow.h>
#include <QPointer.h>
#include <QSettings>

class MainWindow : public QMainWindow
{
public:
	MainWindow(QPointer<QSettings> settings, QWidget *parent = nullptr);

protected:
	void closeEvent(QCloseEvent *event) override;

	void writeSettings();

	void readSettings();

private:
	QPointer<QSettings> mSettings;
};