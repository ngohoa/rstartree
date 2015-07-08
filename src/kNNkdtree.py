import numpy as np
from scipy import spatial
import time
import math
import fileinput
from time import clock

order = 1024

def read_data(fn):
    ""
    with open(fn) as f:
        raw_data = np.loadtxt(f, delimiter=' ', dtype="float",skiprows=0, usecols=None)

    data = []

    for row in raw_data:
        data.append(row[0:])

    return np.array(data)

def build_KDtree(fn):
	data = read_data(fn)
	return spatial.KDTree(data)

def knn(fdata, ftest, k):
	minx = -124.48111
	miny = 32.53722
	maxx = -114.13694 
	maxy = 42.16
	unit_x = (maxx-minx)/1024
	unit_y = (maxy-miny)/1024
	sumA = 0;

	data = read_data(fdata)
	tree = spatial.KDTree(data)
	test = read_data(ftest)

	t0 = time.clock()
	results = tree.query(test, k)
	positions = results[1]
	count = 0
	for poss in positions: #each point
		min_x = test[count][0]
		min_y = test[count][1] 
		max_x = test[count][0] 
		max_y = test[count][1]
		x = y = 0
		for pos in poss: #each nearest neighbor
			x = data[pos][0]
			y = data[pos][1]

			if x <= min_x:
				min_x = x
			if x >= max_x:
				max_x = x
			if y <= min_y:
				min_y = y
			if y >= max_y:
				max_y = y
		sumA+= (math.floor((max_x-min_x)/unit_x)+1)*(math.floor((max_y-min_y)/unit_y)+1);
		count += 1
	t1 = time.clock()
	print (t1-t0), " sec cpu" 
	print sumA

knn("../data/cali-points.txt", "../data/cali-queries.txt", 320)