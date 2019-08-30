/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CodeEditor.h"
#include <Sprocket/IconFactory.h>
#include <QBoxLayout>

CodeEditor::CodeEditor(QWidget* parent) :
	QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout();
	setLayout(layout);

	QToolBar* toolbar = new QToolBar();
	layout->addWidget(toolbar);

	mCompileButton = new QToolButton();
	mCompileButton->setIcon(getDefaultIconFactory().createIcon(IconFactory::Icon::Build));
	mCompileButton->setEnabled(false);
	mCompileButton->setToolTip("Compile");
	toolbar->addWidget(mCompileButton);

	mFilenameLabel = new QLabel();
	layout->addWidget(mFilenameLabel);

	mEditor = new kgl::QCodeEditor();

	kgl::QCodeEditorDesign design;
	design.setEditorBackColor(mEditor->palette().color(QPalette::Base));
	design.setEditorTextColor(mEditor->palette().color(QPalette::Text));
	design.setLineColumnBackColor(mEditor->palette().color(QPalette::Base));
	design.setLineColumnTextColor(mEditor->palette().color(QPalette::Text));
	design.setLineColumnSeparatorColor(QColor(80, 80, 80));
	mEditor->setDesign(design);

	layout->addWidget(mEditor);
	layout->setStretch(2, 3);

	mLog = new QTextEdit;
	mLog->setReadOnly(true);
	layout->addWidget(mLog);
	layout->setStretch(3, 1);

	connect(mCompileButton, &QToolButton::pressed, this, [this] { compile(); });

	connect(mEditor, &kgl::QCodeEditor::lineChanged, mCompileButton, [this] {
		mCompileButton->setEnabled(true);
	});
}

void CodeEditor::setFilename(const QString& filename)
{
	mFilenameLabel->setText(filename);
}

void CodeEditor::setCode(const QString& code, Compiler compiler)
{
	mEditor->setPlainText(code);
	mLog->clear();
	mCompiler = compiler;
	compile();
}

QString CodeEditor::getCode() const
{
	return mEditor->toPlainText();
}

void CodeEditor::compile()
{
	mCompileButton->setEnabled(false);
	if (mCompiler)
	{
		QString result = mCompiler(getCode());

		if (result.isEmpty())
		{
			mLog->append("Compiled successfully");
		}
		if (!result.isEmpty())
		{
			mLog->append(result);
		}
	}
}
