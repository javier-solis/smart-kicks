from pyproj import Geod

def request_handler(request):
    values = request['values']
    try:
        current_lat = float(values['current_lat'])
        current_lon = float(values['current_lon'])
        dest_lat = float(values['dest_lat'])
        dest_lon = float(values['dest_lon'])
    except:
        return 'Value passed in is wrong'
    
    geo = Geod(ellps='WGS84')
    north_lon, north_lat, back_az = geo.fwd(lons=current_lon, lats=current_lat, az=0, dist=100)
    forward_azimuth, back_azimuth, dist = geo.inv(lons1=north_lon, lats1=north_lat, lons2=dest_lon, lats2=dest_lat)

    return round(90 - forward_azimuth) % 360


# lat, lon = 42.355370, -71.100120
# dest_lat, dest_lon = 42.3649281, -71.0910598
# geo = Geod(ellps='WGS84')

# forward_azimuth, back_azimuth, dist = geo.inv(lons1=lon, lats1=lat, lons2=dest_lon, lats2=dest_lat)
# print(forward_azimuth, dist)

# while( (current_angle := input()) != 'QUIT' ):
#     actual = (forward_azimuth - float(current_angle)) % 360.0
#     print(f'Actual angle: {actual}')