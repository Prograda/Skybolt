/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QLabel>
#include <QToolBar>
#include <QToolButton>
#include <KGL/Widgets/QCodeEditor.hpp>

class CodeEditor : public QWidget
{
	Q_OBJECT
public:
	CodeEditor(QWidget* parent = nullptr);

	void setFilename(const QString& filename);

	//! @returns empty string on success, or error message on failure
	typedef std::function<QString(const QString& code)> Compiler;

	void setCode(const QString& code, Compiler compiler);
	QString getCode() const;

private:
	void compile();

private:
	QToolButton* mCompileButton;
	QLabel * mFilenameLabel;
	kgl::QCodeEditor* mEditor;
	Compiler mCompiler;
	QTextEdit* mLog;
};
