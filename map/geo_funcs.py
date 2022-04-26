import numpy as np

def get_bearing(pointA, pointB) -> float:
    """
    Calculates the bearing between two points on a sphere.

    @param pointA: must be a tuple representing (lat, lon)
    @param pointB: same as pointA
    @returns the bearing's value in degrees

    Inspired by: https://gist.github.com/jeromer/2005586
    """

    lat1 = np.radians(pointA[0])
    lat2 = np.radians(pointB[0])

    diffLong = np.radians(pointB[1] - pointA[1])

    x = np.sin(diffLong) * np.cos(lat2)
    y = np.cos(lat1) * np.sin(lat2) - (np.sin(lat1)
            * np.cos(lat2) * np.cos(diffLong))

    initial_bearing = np.arctan2(x, y)

    initial_bearing = np.degrees(initial_bearing)
    compass_bearing = (initial_bearing + 360) % 360

    return compass_bearing


def get_distance(pointA, pointB):
    """
    Calculate the great circle distance between two points on Earth.
    Based off of the haversine formula

    @param pointA: must be a tuple representing (lat, lon)
    @param pointB: same as pointA
    @returns distance in members

    Inspired by https://stackoverflow.com/a/29546836
    """
    
    lat1, lon1 = pointA
    lat2, lon2 = pointB
    
    lon1, lat1, lon2, lat2 = map(np.radians, [lon1, lat1, lon2, lat2])

    dlon = lon2 - lon1
    dlat = lat2 - lat1

    a = np.sin(dlat/2.0)**2 + np.cos(lat1) * np.cos(lat2) * np.sin(dlon/2.0)**2

    c = 2 * np.arcsin(np.sqrt(a))
    meters = 6371 * c * 1000
    return meters