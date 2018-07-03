# -*- coding: utf-8 -*-
import sys
import json
import re
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import pymap3d as pm

from LaunchSiteData.Noshiro_land_3rd import NoshiroAsanai
from LaunchSiteData.Noshiro_sea import NoshiroOchiai
from LaunchSiteData.Taiki_land import TaikiLand

# =====↓↓↓↓ USER INPUT ↓↓↓↓====
receiving_point_LLH = np.array([40.249892, 140.012233, 0.0])

# =====↑↑↑↑ USER INPUT ↑↑↑↑====

# config file arg
argv = sys.argv
if len(argv) < 3:
    print('Error!! argument is missing')
    print('Usage: python ForRocket.py configFileName.json ResultDirectory')
    sys.exit()   
if '.json' in argv[1]: 
    config_file = argv[1]
    result_dir = argv[2]
elif '.json' in argv[2]:
    config_file = argv[2]
    result_dir = argv[1]

# config file to json
json = json.load(open(config_file))

# make launch site instance
launch_site_name = json.get('Launch Pad').get('Site')
noshiro = True if re.search(r'noshiro|nosiro', launch_site_name, re.IGNORECASE) else False
taiki = True if re.search(r'taiki', launch_site_name, re.IGNORECASE) else False
land = True if re.search(r'land', launch_site_name, re.IGNORECASE) else False
sea = True if re.search(r'sea', launch_site_name, re.IGNORECASE) else False
asanai = True if re.search(r'asanai|asauchi|asauti', launch_site_name, re.IGNORECASE) else False
field3rd = True if re.search(r'3rd', launch_site_name, re.IGNORECASE) else False
ochiai = True if re.search(r'ochiai|otiai', launch_site_name, re.IGNORECASE) else False

if noshiro and (land or asanai or field3rd):
    launch_site = NoshiroAsanai()
elif noshiro and (sea or oshiai):
    launch_site = NoshiroOchiai(3000.0)
elif taiki:
    launch_site = TaikiLand()
else:
    print('Error!! Not found launch site: ', launch_site_name)
    sys.exit()

lat0 = launch_site.launch_point_LLH[0]
lon0 = launch_site.launch_point_LLH[1]
h0 = 0.0

# read log
df = pd.read_csv(result_dir+'/log.csv', index_col=False)

pos_east = df['Pos_East']
pos_north = df['Pos_North']
pos_up = df['Pos_Up']
lat, lon, h = pm.enu2geodetic(pos_east, pos_north, pos_up, lat0, lon0, h0)

az, el, range = pm.geodetic2aer(lat, lon, h, receiving_point_LLH[0], receiving_point_LLH[1], receiving_point_LLH[2])

df = df.assign(dist_Receive = range)
df = df.assign(azi_Receive = az)
df = df.assign(elv_Receive = el)
df.to_csv(result_dir+'/log.csv')

plt.figure()
plt.plot(df['time'], range)
plt.xlabel('Time [sec]')
plt.ylabel('Distance - 3D [m]')
plt.grid()
plt.savefig(result_dir + '/Distance_ReceivePoint.png')

plt.figure()
plt.plot(df['time'], el)
plt.xlabel('Time [sec]')
plt.ylabel('Elevation [deg]')
plt.grid()
plt.savefig(result_dir + '/Elevation_ReceivePoint.png')

plt.figure()
plt.plot(df['time'], az)
plt.xlabel('Time [sec]')
plt.ylabel('Azimuth [deg]')
plt.grid()
plt.savefig(result_dir + '/Azimuth_ReceivePoint.png')
