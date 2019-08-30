/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <Sprocket/EditorPlugin.h>
#include <KGL/Widgets/QCodeEditor.hpp>

#include <boost/config.hpp>
#include <boost/dll/alias.hpp>
#include <assert.h>
#include <deque>
#include <memory>
#include <sstream>

#pragma push_macro("slots")
#undef slots
#include <pybind11/embed.h>
#include <pybind11/iostream.h>
#pragma pop_macro("slots")

namespace py = pybind11;

//! Redirects python stdout to a std::ostream
class ScopedPyStdoutRedirect
{
public:
	ScopedPyStdoutRedirect(std::ostream& os)
	{
		mOut = PySys_GetObject("stdout");
		mOldWriteFn = PyObject_GetAttrString(mOut, "write");

		std::string output;
		PyObject_SetAttrString(mOut, "write", pybind11::cpp_function([&](const char* w) {
			os << w;
		}, py::arg("string")).ptr());
	}

	~ScopedPyStdoutRedirect()
	{
		PyObject_SetAttrString(mOut, "write", mOldWriteFn);
	}

private:
	PyObject* mOut;
	PyObject* mOldWriteFn;
};

class ConsoleWidget : public kgl::QCodeEditor
{
public:
	ConsoleWidget(QWidget* parent = nullptr) :
		kgl::QCodeEditor(parent)
	{
		setUndoRedoEnabled(false);
		startNewLine();

		connect(this, &QPlainTextEdit::cursorPositionChanged, this, [this] {onCursorPositionChanged(); });
	}

private:
	void startNewLine()
	{
		appendTextWithoutNewline(mInputLinePrefix);
	}

	void onCursorPositionChanged()
	{
	}

	int getInputLineStartPosition() const
	{
		QTextCursor c = textCursor();
		c.movePosition(QTextCursor::End);
		c.movePosition(QTextCursor::StartOfLine);
		return c.position() + mInputLinePrefix.length();
	}

	void keyPressEvent(QKeyEvent* event) override
	{
		QTextCursor c = textCursor();
		const int inputLineStartPosition = getInputLineStartPosition();

		if (c.position() < inputLineStartPosition)
		{
			c.movePosition(QTextCursor::End);
			setTextCursor(c);
		}

		if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
		{
			QString text = textAtLine(lineCount() - 1).mid(mInputLinePrefix.length()).remove(QRegExp("[\\n\\t\\r]"));
			if (!text.isEmpty())
			{
				mHistoryLines.push_front(text);
				mHistoryIndex = -1;

				try
				{
					std::ostringstream ss;
					ScopedPyStdoutRedirect redirect(ss);
					py::eval<py::eval_single_statement>(text.toStdString());

					appendTextWithoutNewline(QString::fromStdString("\n" + ss.str()));
				}
				catch (const std::exception& e)
				{
					appendPlainText(QString(e.what()) + "\n");
				}
			}
			else
			{
				appendPlainText("");
			}

			startNewLine();
		}
		else if (event->key() == Qt::Key_Backspace && c.position() <= inputLineStartPosition)
		{
			// ignore backspace if at/before start of input line
		}
		else if (event->key() == Qt::Key_Up)
		{
			if (!mHistoryLines.empty())
			{
				mHistoryIndex = std::min((int)mHistoryLines.size() - 1, mHistoryIndex + 1);
				setInputLineText(mHistoryLines[mHistoryIndex]);
			}
		}
		else if (event->key() == Qt::Key_Down)
		{
			if (!mHistoryLines.empty())
			{
				mHistoryIndex = std::max(0, mHistoryIndex - 1);
				setInputLineText(mHistoryLines[mHistoryIndex]);
			}
		}
		else
		{
			kgl::QCodeEditor::keyPressEvent(event);

			QTextCursor c = textCursor();
			int inputLineStartPosition = getInputLineStartPosition();
			if (c.position() < inputLineStartPosition)
			{
				c.setPosition(inputLineStartPosition);
				setTextCursor(c);
			}
		}
	}

	void setInputLineText(const QString& str)
	{
		QTextCursor c = textCursor();
		c.setPosition(getInputLineStartPosition(), QTextCursor::MoveAnchor);
		c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
		c.removeSelectedText();
		setTextCursor(c);

		appendTextWithoutNewline(str);
	}

	void appendTextWithoutNewline(const QString& str)
	{
		moveCursor(QTextCursor::End);
		insertPlainText(str);
		moveCursor(QTextCursor::End);
	}

private:
	const QString mInputLinePrefix = ">>> ";
	int mHistoryIndex = -1;
	std::deque<QString> mHistoryLines;
};

class PythonConsolePlugin : public EditorPlugin
{
public:
	PythonConsolePlugin(const EditorPluginConfig& config)
	{
		auto console = new ConsoleWidget();
		kgl::QCodeEditorDesign design;
		design.setLineColumnVisible(false);
		design.setEditorBackColor(console->palette().color(QPalette::Base));
		design.setEditorTextColor(console->palette().color(QPalette::Text));
		console->setDesign(design);

		mToolWindow.name = "Python Console";
		mToolWindow.widget = console;
	}

	~PythonConsolePlugin()
	{
		delete mToolWindow.widget;
	}

	std::vector<ToolWindow> getToolWindows() override
	{
		return { mToolWindow };
	}

private:
	ToolWindow mToolWindow;
};

namespace plugins {

	std::shared_ptr<EditorPlugin> createEditorPlugin(const EditorPluginConfig& config)
	{
		return std::make_shared<PythonConsolePlugin>(config);
	}

	BOOST_DLL_ALIAS(
		plugins::createEditorPlugin,
		createEditorPlugin
	)
}
