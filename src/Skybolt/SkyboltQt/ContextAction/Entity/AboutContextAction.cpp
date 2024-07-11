/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "AboutContextAction.h"
#include "SkyboltQt/QtUtil/QtDialogUtil.h"
#include <SkyboltSim/Components/AssetDescriptionComponent.h>

#include <QComboBox>
#include <QMessageBox>

using namespace skybolt;
using namespace skybolt::sim;

bool AboutContextAction::handles(const Entity& entity) const
{
	return entity.getFirstComponent<AssetDescriptionComponent>() != nullptr;
}

void AboutContextAction::execute(Entity& entity) const
{
	auto desc = entity.getFirstComponent<AssetDescriptionComponent>()->getDescription();
	QString text;
	text += QString::fromStdString(desc.description);
	if (!desc.sourceUrl.empty())
	{
		text += "\n\n" + QString::fromStdString(desc.sourceUrl);
	}

	if (desc.authors.size() > 0)
	{
		text += "\n\n";
		text += (desc.authors.size() > 1) ? "Authors:" : "Author:";
	}
	for (const auto & author : desc.authors)
	{
		text += "\n" + QString::fromStdString(author);
	}

	QMessageBox::about(nullptr, "About Asset", text);
}
