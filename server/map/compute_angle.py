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

    final_forward_azimuth = round(90 - forward_azimuth) % 360
    return f'{final_forward_azimuth},{dist}'