#include "StatusBar.h"

#include <QApplication>
#include <QBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QToolButton>
#include <QStatusBar>
#include <QStyle>
#include <QVariant>

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/trivial.hpp>

namespace bl = boost::log;

class LabelLogSink : public bl::sinks::basic_formatted_sink_backend<char, bl::sinks::synchronized_feeding>
{
public:
	LabelLogSink(std::function<void(const QString&)> fn) :
		mFn(fn)
	{
	}

    void consume(const bl::record_view& rec, const std::string& str) {
		mFn(QString::fromStdString(str));
    }

private:
	std::function<void(const QString&)> mFn;

private:
	QLabel* mLabel;
};

void addErrorLogStatusBar(QStatusBar& bar)
{
	auto widget = new QWidget(&bar);
	auto layout = new QHBoxLayout(widget);
	layout->setMargin(0);
	
	auto label = new QLabel(&bar);
	label->setStyleSheet("QLabel { color : yellow; }");
	layout->addWidget(label);

	QFontMetrics fm(label->font());

	QStyle* style = QApplication::style();

	auto infoButton = new QToolButton(&bar);
	infoButton->setIcon(style->standardIcon(QStyle::SP_TitleBarNormalButton));
	infoButton->setToolTip("Expand");
	infoButton->setFixedHeight(fm.height());
	infoButton->setVisible(false);
	layout->addWidget(infoButton);

	auto clearButton = new QToolButton(&bar);
	clearButton->setIcon(style->standardIcon(QStyle::SP_TitleBarCloseButton));
	clearButton->setToolTip("Close");
	clearButton->setFixedHeight(fm.height());
	clearButton->setVisible(false);
	layout->addWidget(clearButton);

	QObject::connect(infoButton, &QToolButton::pressed, [parent = &bar, infoButton] {
		QMessageBox::about(parent, "", infoButton->property("messageText").toString());
	});

	QObject::connect(clearButton, &QToolButton::pressed, [label, infoButton, clearButton] {
		label->setText("");
		infoButton->setVisible(false);
		clearButton->setVisible(false);
	});

	bar.addPermanentWidget(widget);

    auto sink = boost::make_shared<LabelLogSink>([label, infoButton, clearButton] (const QString& message) {
		QString singleLineMessage = message;
		singleLineMessage = singleLineMessage.replace('\n', ' ');
		singleLineMessage.resize(std::min(singleLineMessage.size(), 100));
		singleLineMessage += "...";

		label->setText(singleLineMessage);
		infoButton->setProperty("messageText", message);
		infoButton->setVisible(!message.isEmpty());
		clearButton->setVisible(!message.isEmpty());
	});

	using sink_t = bl::sinks::synchronous_sink<LabelLogSink>;
	auto sinkWrapper = boost::make_shared<sink_t>(sink);
	sinkWrapper->set_filter(bl::trivial::severity >= bl::trivial::error);
    bl::core::get()->add_sink(sinkWrapper);
}