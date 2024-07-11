/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <SkyboltSim/SkyboltSimFwd.h>

#include <QWidget>

class QGridLayout;

class EntityControllerWidget : public QWidget
{
public:
	EntityControllerWidget(QWidget* parent = nullptr);

	~EntityControllerWidget() override;

	//! @param object may be null
	void setEntity(skybolt::sim::Entity* entity);

private:
	void addItem(const QString& name, QWidget* widget);

private:
	QGridLayout* mLayout;
	std::vector<QWidget*> mUpdateWidgets;
	static const int mControlSizePixels = 100;
};