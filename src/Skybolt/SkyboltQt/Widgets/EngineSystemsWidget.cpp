#include "EngineSystemsWidget.h"

#include <SkyboltSim/System/System.h>

#include <QTextEdit>

QWidget* createEngineSystemsWidget(const skybolt::sim::SystemRegistry& registry, QWidget* parent)
{
	QString str;
	for (const auto& system : registry)
	{
		str += QString(typeid(*system).name()) + "\n";
	}

	auto textEdit = new QTextEdit(parent);
	textEdit->setPlainText(str);
	textEdit->setReadOnly(true);
	return textEdit;
}