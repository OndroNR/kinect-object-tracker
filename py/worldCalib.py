import numpy as np
import cv2

shape = (1, 11, 3)
world = np.zeros(shape, np.float32)
camera = np.zeros(shape, np.float32)

# [x, y, z]
world[0][0] = [0,0,0]
world[0][1] = [0,0,0.6]
world[0][2] = [0.6,0,0]
world[0][3] = [0.6,0,0.6]
world[0][4] = [1.2,0,0]
world[0][5] = [0.6,0,1.2]
world[0][6] = [1.2,0,1.2]
world[0][7] = [1.8,0,1.8]
world[0][8] = [1.8,0,0.6]
world[0][9] = [0.6,0,1.8]
world[0][10] = [2.9,1.2,0.5]

camera[0][0] = [683.491, 1421.46, 1073]
camera[0][1] = [324.551, 2610.22, 1496]
camera[0][2] = [374.971, 347.7, 1477]
camera[0][3] = [27.8199, 1669.19, 2009]
camera[0][4] = [104.023, -667.483, 1878]
camera[0][5] = [-245.445, 2956.49, 2417]
camera[0][6] = [-596.639, 1914.79, 3006]
camera[0][7] = [-1276.86, 2215.73, 4068]
camera[0][8] = [-558.53, -545.232, 2881]
camera[0][9] = [-733.301, 4586.73, 3115]
camera[0][10] = [-3923.94, -2679.4, 3172]

retval, M, inliers = cv2.estimateAffine3D(camera, world)

print(retval)
print(M)
print(inliers)
