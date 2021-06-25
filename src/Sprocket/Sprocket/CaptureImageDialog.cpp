/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CaptureImageDailog.h"
#include "PropertyEditor.h"
#include "QDialogHelpers.h"

#include <QProgressDialog>
#include <filesystem>

static QString specializeImageSequenceFilenameTemplate(QString str, int frameNumber)
{
	int endIndex = str.lastIndexOf('#');
	if (endIndex >= 0)
	{
		str.replace('#', '0');

		QString numberStr = QString::number(frameNumber);
		int startIndex = std::max(0, 1 + endIndex - int(numberStr.size()));
		str.replace(startIndex, numberStr.size(), numberStr);
	}

	return str;
}

void showCaptureImageSequenceDialog(const FrameImageWriter& frameImageWriter, const QString& defaultSequenceName, QWidget* parent)
{
	auto filenameTemplate = PropertiesModel::createVariantProperty("Filename", "Video/" + defaultSequenceName + "/" + defaultSequenceName + ".####.jpg");
	auto startTime = PropertiesModel::createVariantProperty("Start Time", 0.0);
	auto endTime = PropertiesModel::createVariantProperty("End Time", 1.0);
	auto frameRate = PropertiesModel::createVariantProperty("Frame Rate", 30.0);
	auto properties = std::make_shared<PropertiesModel>(std::vector<QtPropertyPtr>({filenameTemplate, startTime, endTime, frameRate}));

	PropertyEditor* editor = new PropertyEditor({});
	editor->setModel(properties);

	std::shared_ptr<QDialog> dialog = createDialog(editor, "Capture Image Sequence");
	if (dialog->exec() == QDialog::Accepted)
	{
		int frameCount = frameRate->value.toDouble() * std::max(0.0, (endTime->value.toDouble() - startTime->value.toDouble()));
		QProgressDialog progress("Capturing Sequence...", "Cancel", 0, frameCount, parent);
		progress.setWindowModality(Qt::WindowModal);

		for (int frame = 0; frame < frameCount; ++frame)
		{
			double time = startTime->value.toDouble() + (double)frame / frameRate->value.toDouble();

			QString frameStr = specializeImageSequenceFilenameTemplate(filenameTemplate->value.toString(), frame);
			std::filesystem::path directory = std::filesystem::path(frameStr.toStdString()).parent_path();
			std::filesystem::create_directories(directory);
			frameImageWriter(time, frameStr);

			progress.setValue(frame);

			if (progress.wasCanceled())
				break;
		}
	}
}