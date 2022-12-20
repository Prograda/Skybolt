/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MoveToAirportContextAction.h"
#include "Sprocket/QDialogHelpers.h"
#include <SkyboltEngine/TemplateNameComponent.h>
#include <SkyboltSim/Components/DynamicBodyComponent.h>
#include <SkyboltSim/Spatial/GreatCircle.h>
#include <SkyboltSim/Spatial/LatLon.h>
#include <SkyboltSim/Spatial/Orientation.h>
#include <SkyboltSim/Spatial/Position.h>

#include <QComboBox>
#include <QVBoxLayout>

using namespace skybolt;
using namespace skybolt::sim;

class AirportChooser : public QWidget
{
public:
	AirportChooser(const AirportsMap& airports) :
		mAirports(airports)
	{
		mComboBox = new QComboBox();
		setLayout(new QVBoxLayout);
		layout()->addWidget(mComboBox);

		for (const auto& v : mAirports)
		{
			mComboBox->addItem(QString::fromStdString(v.first));
		}
	}

	skybolt::mapfeatures::AirportPtr getAirport() const
	{
		std::string name = mComboBox->currentText().toStdString();
		auto i = mAirports.find(name);
		if (i != mAirports.end())
		{
			return i->second;
		}
		return nullptr;
	}

private:
	const AirportsMap& mAirports;
	QComboBox* mComboBox;
};

bool MoveToAirportContextAction::handles(const Entity& entity) const
{
	return entity.getFirstComponent<TemplateNameComponent>() != nullptr;
}

void MoveToAirportContextAction::execute(Entity& entity) const
{
	AirportChooser* airportChooser = new AirportChooser(mAirports);
	auto dialog = createDialog(airportChooser, "Move to Airport");

	if (dialog->exec() == QDialog::Accepted)
	{
		mapfeatures::AirportPtr airport = airportChooser->getAirport();
		if (airport)
		{
			const mapfeatures::Airport::Runway& runway = airport->runways.front();
			sim::LatLon latLon = runway.start;

			double bearing = calcBearing(runway.start, runway.end);

			const double startOffset = 50.0;
			latLon = moveDistanceAndBearing(latLon, startOffset, bearing);

			static const double aircraftHeight = 5.0;
			setPosition(entity, toGeocentric(LatLonAltPosition(sim::toLatLonAlt(latLon, airport->altitude + aircraftHeight))).position);

			setOrientation(entity, sim::toGeocentric(LtpNedOrientation(glm::angleAxis(bearing, Vector3(0, 0, 1))), latLon).orientation);

			DynamicBodyComponent* body = entity.getFirstComponent<DynamicBodyComponent>().get();
			if (body)
			{
				body->setLinearVelocity(math::dvec3Zero());
				body->setAngularVelocity(math::dvec3Zero());
			}
		}
	}
}
