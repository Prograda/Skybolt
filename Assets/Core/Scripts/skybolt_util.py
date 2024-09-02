import skybolt as sb


def set_entity_lla(entity: sb.Entity, lla: sb.LatLonAlt):
    """
    Sets the position of an entity to a latitude, longitude and altitude
    """
    entity.setPosition(sb.toGeocentricPosition(sb.LatLonAltPosition(lla)).position)


def set_entity_lla_rpy(entity: sb.Entity, lla: sb.LatLonAlt, rpy: sb.Vector3):
    """
    Sets the position of an entity to a latitude, longitude and altitude,
    and sets the orientation to a roll, pitch, yaw vector.
    """
    entity.setPosition(sb.toGeocentricPosition(sb.LatLonAltPosition(lla)).position)
    ltpNedOrientation = sb.quaternionFromEuler(rpy)
    orientation = sb.toGeocentricOrientation(sb.LtpNedOrientation(ltpNedOrientation), sb.toLatLon(lla))
    entity.setOrientation(orientation.orientation)


def lla_to_geocentric(lla: sb.LatLonAlt) -> sb.Vector3:
    """
    Converts a `LatLonAlt` to a geocentric position `Vector3`
    """
    return sb.toGeocentricPosition(sb.LatLonAltPosition(lla)).position


def geocentric_to_lla(xyz: sb.Vector3) -> sb.LatLonAlt:
    """
    Converts a geocentric position `Vector3` to a `LatLonAlt`
    """
    return sb.toLatLonAlt(sb.GeocentricPosition(xyz)).position


def rpy_to_geocentric_quat(rpy: sb.Vector3, lat_lon: sb.LatLonAlt) -> sb.Quaternion:
    """
    Converts a roll, pitch, yaw `Vector3` to a `Quartion` at the given `LatLonAlt` location
    """
    return sb.toGeocentricOrientation(sb.LtpNedOrientation(sb.quaternionFromEuler(rpy)), lat_lon).orientation
