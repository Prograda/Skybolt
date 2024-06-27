#pragma once

#include <SkyboltSim/System/SystemRegistry.h>

class QWidget;

QWidget* createEngineSystemsWidget(const skybolt::sim::SystemRegistry& registry, QWidget* parent);