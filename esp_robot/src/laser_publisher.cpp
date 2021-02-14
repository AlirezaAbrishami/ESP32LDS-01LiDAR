#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>
#include <iostream>

using namespace std;

sensor_msgs::LaserScanPtr fullScan(new sensor_msgs::LaserScan);
ros::Publisher laser_pub;

uint8_t in = 0;


void scanCallback(const sensor_msgs::LaserScanPtr scan) {
    auto scanIterator = scan->ranges.begin();
    if (*scanIterator == 0xFA && *++scanIterator == 0xA0 + in && in <= 3) {
        scanIterator ++;
        for (int i = 2; i < 92; i++) {
            uint16_t index = in * 90 + i - 2;
            fullScan->ranges[index] = *scanIterator;
            scanIterator++;
        }
        if (in == 3) {
            laser_pub.publish(fullScan);
            in = 0;
        } else {
            in++;
        }
    }
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "laser_publisher");
    ros::NodeHandle n;
    laser_pub = n.advertise<sensor_msgs::LaserScan>("scan", 1000);
    ros::Subscriber sub = n.subscribe("scan_segments", 1000, scanCallback);
    char frameId[] = "laser";
    fullScan->header.frame_id = frameId;
    fullScan->ranges.resize(360);
    fullScan->angle_increment = (2.0 * M_PI / 360.0);
    fullScan->angle_min = 0.0;
    fullScan->angle_max = 2.0 * M_PI - fullScan->angle_increment;
    fullScan->range_min = 0.0;
    fullScan->range_max = 3.5;
    ros::spin();
    return 0;
}