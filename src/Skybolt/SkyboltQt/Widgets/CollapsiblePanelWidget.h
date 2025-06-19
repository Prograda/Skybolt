#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QToolButton>

class CollapsiblePanelWidget : public QWidget
{
    Q_OBJECT

public:
    CollapsiblePanelWidget(const QString& title = "", QWidget* parent = nullptr);

    QWidget* getContentWidget() const { return mContentWidget; }

public slots:
    void setExpanded(bool expanded);

signals:
    void expandedStateChanged(bool expanded);

private:
    QToolButton* mToggleButton;
    QWidget* mContentWidget;
};
