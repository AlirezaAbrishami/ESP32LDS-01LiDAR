#include <ros/ros.h>
#include <std_msgs/UInt16.h>
#include <sensor_msgs/LaserScan.h>
#include <iostream>
#include <cstdlib>
#include <cstring>

void scanCallback(const sensor_msgs::LaserScanPtr scan) {

}

int main(int argc, char **argv) {
    ros::init(argc, argv, "listener");
    ros::NodeHandle n;
    ros::Subscriber sub = n.subscribe("scan_segments", 1000, scanCallback);
    ros::spin();

    return 0;
}