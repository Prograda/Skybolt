#pragma once

#include <QProxyStyle>

class DarkStyle : public QProxyStyle
{
 public:
	DarkStyle();
	explicit DarkStyle(QStyle* style);

	void polish(QPalette& palette) override;
	void polish(QApplication* app) override;
	QIcon standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption* option = nullptr, const QWidget* widget = nullptr) const override;
	QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption* option = nullptr, const QWidget* widget = nullptr) const override;
};
