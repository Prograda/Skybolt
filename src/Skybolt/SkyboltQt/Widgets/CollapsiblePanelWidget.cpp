#include "CollapsiblePanelWidget.h"

#include <QScrollArea>
#include <QStringLiteral>

CollapsiblePanelWidget::CollapsiblePanelWidget(const QString& title, QWidget* parent) :
    QWidget(parent),
    mContentWidget(new QWidget),
    mToggleButton(new QToolButton)
{
    mToggleButton->setText(title);
    mToggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mToggleButton->setCheckable(true);
    mToggleButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(mToggleButton);
    mainLayout->addWidget(mContentWidget);

    connect(mToggleButton, &QAbstractButton::toggled, this, &CollapsiblePanelWidget::setExpanded);
    setExpanded(true);
}

void CollapsiblePanelWidget::setExpanded(bool expanded)
{
    mToggleButton->setChecked(expanded);
    mToggleButton->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
    mContentWidget->setVisible(expanded);

    Q_EMIT expandedStateChanged(expanded);
}
