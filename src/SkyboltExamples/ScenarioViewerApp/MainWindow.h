#pragma once

#include <QMainWindow>
#include <QPointer>
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