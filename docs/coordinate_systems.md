# Coordinate Systems

## Earth Centered Earth Fixed (ECEF)
Skybolt 3D object world positions use ECEF coordinates with:
* Origin at planet center
* +X through the equator and prime meridian
* +Y perpendicular to X and Z, based on right-handed coordinate system
* +Z axis through the North Pole

## Latitude and Longitude
Skybolt also represents positions as either `LatLon` or `LatLonAlt` when more natural than ECEF. 

## North East Down (NED)
Local Tangent Plane (LTP) coordinates are represented as:
* +X is north
* +Y is east
* +Z is down

## Body Axes
Body local coordinates are represented as:
* +X is forward
* +Y is right
* +Z is down
