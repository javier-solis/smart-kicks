from typing import Tuple, List
import numpy as np
import argparse

# Inspiration: https://stackoverflow.com/questions/7427101/simple-argparse-example-wanted-1-argument-3-results

parser = argparse.ArgumentParser(description='''
    Returns the destination point from a starting (lat,lon) point having travelled a 
    given distance on a given bearing (degrees clockwise from North)''')

# Potential update: https://stackoverflow.com/questions/68020846/python3-argparse-nargs-get-number-of-arguments
# parser.add_argument("--test", type=float, nargs=4, required=False)
# args = parser.parse_args()
# print(args.test[0], [1], etc)

parser.add_argument('lat', help='Latitude', type=float)
parser.add_argument('lon', help='Longitude', type=float)
parser.add_argument('bearing', help='Bearing, degrees clockwise from North', type=float)
parser.add_argument('distance', help='Distance, in meters', type=float)

args = parser.parse_args()

earth_radius = 6371*1000 # km -> m

def distancePoint(start_lat: float, start_lon: float, bearing: float, distance: float) -> tuple[float, float]:
    '''
    Derivation inspired by https://www.movable-type.co.uk/scripts/latlong.html
    '''

    delta = distance / earth_radius  # angular distance in radians
    theta = np.radians(bearing)

    phi1 = np.radians(start_lat)
    lambda1 = np.radians(start_lon)

    sin_phi2 = np.sin(phi1)*np.cos(delta) + np.cos(phi1)*np.sin(delta)*np.cos(theta)
    
    phi2 = np.arcsin(sin_phi2)

    y = np.sin(theta)*np.sin(delta)*np.cos(phi1)
    x = np.cos(delta) - np.sin(phi1) * sin_phi2
    lambda2 = lambda1 + np.arctan2(y,x)

    new_lat = np.degrees(phi2)
    new_lon = np.degrees(lambda2)
    return (new_lat, new_lon)

answer = distancePoint(args.lat, args.lon, args.bearing, args.distance)
print(str(answer[0])+"\t"+str(answer[1]))
