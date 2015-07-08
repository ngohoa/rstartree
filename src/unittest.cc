/*
 Copyright (C) 2010 by The Regents of the University of California
 
 Redistribution of this file is permitted under
 the terms of the BSD license.
 
 Date: 20/06/2011
 Author: Sattam Alsubaiee <salsubai (at) ics.uci.edu>
 */

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "rstartree.h"
using namespace std;

#define RANGE 320

clock_t clock_start, clock_end;
double dist_time_cpu, dist_time_wall;
double total_time_cpu = 0.0;
double total_time_wall = 0.0;
struct timespec time_start, time_finish;

void timerstart(){
    clock_start = clock();
    clock_gettime(CLOCK_MONOTONIC, &time_start);
}

void timerend(){
    clock_gettime(CLOCK_MONOTONIC, &time_finish);
    clock_end = clock ();
    dist_time_cpu = (double) (clock_end - clock_start) / 
    (double) CLOCKS_PER_SEC;
    dist_time_wall = (time_finish.tv_sec - time_start.tv_sec);
    dist_time_wall += (time_finish.tv_nsec - time_start.tv_nsec) / 1000000000.0;

    total_time_cpu = total_time_cpu + dist_time_cpu;
    total_time_wall = total_time_wall + dist_time_wall;
}

bool parseLine(string &line, Rectangle &mbr)
{
	istringstream coordinatesStream(line);
	float coord;
	vector<float> coords;
	while(coordinatesStream >> coord)
	{
		coords.push_back(coord);
	}
	if(coords.size() == 2)
	{
		mbr.min.x = coords[0];
		mbr.min.y = coords[1];
		mbr.max.x = mbr.min.x;
		mbr.max.y = mbr.min.y;
	}
	else 
	{
		mbr.min.x = coords[0];
		mbr.min.y = coords[1];
		mbr.max.x = coords[2];
		mbr.max.y = coords[3];
	}
	return true;
}

void printObject(Object object){
	Point min = object.mbr.min;
	Point max = object.mbr.max;
	cout << min.x << " " << min.y << "|" << max.x << " " << max.y << "   |   ";
}

void printResults(vector<Object> &objects, unsigned queryNumber)
{
	cout << "printing the answer for query #" << queryNumber << endl;
	for(unsigned i = 0; i < objects.size(); ++i)
	{
		cout << objects[i].id << endl;
	}
}

string* extractIndexString(string s, int number) {
    string* container = new string[number];
    string temp = "";
    int space = 0;
    for (int i = 0; i < s.length(); i++) {
        if (s[i] == ' ') {
            container[space++] = temp;
            temp = "";
        }
        else
            temp += s[i];
    }
    container[space] = temp;    
    return container;
}

int main(int argc, char **argv)
{		
	string datasetFile = "../data/cali-points.txt";
	string queryWorkloadFile = "../data/cali-queries.txt";
		
	Storage st;

    RTree rtree(&st);
	
	// creating an R*-tree with a branch factor of 32, and a fill factor of 0.7
    rtree.create(32, 0.7);
	
	ifstream data(datasetFile.c_str());
	if (!data)
	{
		cerr << "cannot open dataset file" << endl;
		return -1;
	}
	ifstream queries(queryWorkloadFile.c_str());
	if (!queries)
	{
		cerr << "cannot open query workload file" << endl;
		return -1;
	}
	
    string line;
    unsigned count = 0;

	// inserting objects in the R*-tree
	while (getline(data, line))
	{	
		Rectangle mbr;
		Object obj;
		if(parseLine(line, mbr))
		{
			obj.mbr = mbr;
			obj.id = count;
			rtree.insertData(obj);
			// delete the object
			// rtree.deleteData(obj);
		}
		/*if ((count % 1000) == 0)
			cerr << count << endl;*/
		++count;
	}

	cout << "starting the queries" << endl;
    /*unsigned numOfQueries = 0;

	while (getline(queries, line))
	{
		vector<Object> objects;
		Rectangle range;
		parseLine(line, range);
		     
		rtree.rangeQuery(objects, range);
		
		cout << "number of objects that satisfy the query are: " << objects.size() << endl;
		++numOfQueries;
		for (int i = 0; i < objects.size(); i++) {
			printObject(objects[i]);
		}
	
	}*/

	// kNN query
	/*multimap<double, Object> object1s;
	Point point;
	multimap<double, Object>::iterator it;

	point.x = -114.186394;
	point.y = 34.30806;
	rtree.kNNQuery(object1s, point, 80);
	for (it=object1s.begin(); it!=object1s.end(); it++) {
		printObject(it->second);
		cout << it->first << endl;
	}*/
	double minx = -124.48111, miny = 32.53722;
  	double maxx = -114.13694, maxy = 42.16;
  	double unit_x = (maxx-minx)/1024;
  	double unit_y = (maxy-miny)/1024;
	double sum = 0;

	timerstart();
	srand (time(NULL));
	while (getline(queries, line)) {
		string *lineIndex;
		multimap<double, Object> objects;
		Point point;
		multimap<double, Object>::iterator it;

        lineIndex = extractIndexString(line, 2);
        point.x = strtod(lineIndex[0].c_str(), NULL);
        point.y = strtod(lineIndex[1].c_str(), NULL);

        rtree.kNNQuery(objects, point, RANGE);
        int pos = rand() % RANGE;
        //cout << pos << endl;
        int count = 0;
        multimap<double, Object> o;
        for (it=objects.begin(); it!=objects.end(); it++) {
        	if (pos == count) {
        		Point p;
        		p.x = (it->second).mbr.min.x;
				p.y = (it->second).mbr.min.y;
				rtree.kNNQuery(o, p, RANGE-1);
				break;
			}
			count++;
		}

		double min_x = point.x, min_y = point.y, max_x = point.x, max_y = point.y;
		multimap<double, Object>::iterator itn;
		for (itn=o.begin(); itn!=o.end(); itn++) {
		int i = 0;
		//for (itn=objects.begin(); itn!=objects.end(); itn++) {
			i++;
			double first = (itn->second).mbr.min.x;
			double second = (itn->second).mbr.max.y;
			if (first <= min_x)
	            min_x = first;
	        if (first >= max_x)
	            max_x = first;
	        if (second <= min_y)
	            min_y = second;
	        if (second >= max_y)
	            max_y = second;
	    }
	    //cout << i << endl;

	    /*double x_range = floor((max_x - minx)/unit_x)-floor((min_x - minx)/unit_x)+1;
	    double y_range = floor((max_y - miny)/unit_y)-floor((min_y - miny)/unit_y)+1;*/
	    double x_range = floor((max_x-min_x)/unit_x)+1;
	    double y_range = floor((max_y-min_y)/unit_y)+1;
	    sum = sum + x_range*y_range;

	    /*for (int i = 0; i < 1000; i++){
	    	cout << i;	
	    }*/
	}
	timerend();
	printf("%f sec cpu, %f sec wall\n", 
       total_time_cpu, total_time_wall);
	cout << sum << endl;
	return 0;
}
