#include <WiFi.h>
#include <ros.h>
#include <std_msgs/UInt16.h>
#include <sensor_msgs/LaserScan.h>

ros::NodeHandle nh;
sensor_msgs::LaserScan scan;
ros::Publisher laser_pub("scan_segments", &scan);

char frame_id[] = "laser";

const char* ssid = "Alireza iPhone";
const char* password = "qazwsxedc735";

// Set the rosserial socket server IP address
IPAddress server(172, 20, 10, 3);
// Set the rosserial socket server port
const uint16_t serverPort = 11411;

uint8_t raw_bytes[2520];
float* ranges = new float[92];
float fullRanges[360];
bool gotScan = false;
long blinkMillis;

void poll() {
    Serial.println("poll");

    //LaserScan sensormsg init
    scan.angle_increment = (2.0 * M_PI / 360.0);
    scan.angle_min = 0.0;
    scan.angle_max = 2.0 * M_PI - scan.angle_increment;
    scan.range_min = 0.0;
    scan.range_max = 3.5;
    scan.ranges_length = 100;
    bool foundStart = false;
    int startCount = 0;
    Serial.flush();
    while (!foundStart) {
        Serial.readBytes(&raw_bytes[startCount], 1);
        if (startCount == 0 && raw_bytes[startCount] == 0xFA) {
            startCount = 1;
            Serial.println("Found 0xFA.");
        }
        if (startCount == 1 && raw_bytes[startCount] == 0xA0) {
            foundStart = true;
            Serial.println("Found 0xA0.");
            startCount = 2;
            Serial.readBytes(&raw_bytes[startCount], 2518);
            for (uint16_t i = 0; i < 2520; i += 42) {
                if (raw_bytes[i] == 0xFA && raw_bytes[i + 1] == (0xA0 + i / 42)) {  //CRC check
                    for (uint16_t j = i + 4; j < i + 40; j += 6) {
                        int index = 6 * (i / 42) + (j - 4 - i) / 6;

                        uint8_t byte0 = raw_bytes[j];
                        uint8_t byte1 = raw_bytes[j + 1];
                        uint8_t byte2 = raw_bytes[j + 2];
                        uint8_t byte3 = raw_bytes[j + 3];

                        int range = (byte3 << 8) + byte2;
                        fullRanges[359 - index] = range / 1000.0;
                        yield();
                    }
                    if (i == 2478)
                        gotScan = true;
                }
            }
        }
    }
}

void setup() {
    Serial.begin(230400);
    pinMode(14, OUTPUT);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Set the connection to rosserial socket server
    nh.getHardware()->setConnection(server, serverPort);
    nh.initNode();

    nh.advertise(laser_pub);

}

void loop() {
    yield();
    if (blinkMillis && millis() - blinkMillis >= 300) {
        digitalWrite(14, 0);
        blinkMillis = 0;
    }
    raw_bytes[0] = 0;
    raw_bytes[1] = 0;
    scan.header.frame_id = frame_id;
    poll();
    if (gotScan) {
        digitalWrite(14, 1);
        blinkMillis = millis();
        gotScan = false;
        if (nh.connected()) {
            for (int i = 0; i < 4; i++) {
                ranges[0] = 0xFA;
                ranges[1] = 0xA0 + i;
                for (int j = 2; j < 92; j++) {
                    uint16_t index = i * 90 + j - 2;
                    ranges[j] = fullRanges[index];
                }
                scan.ranges = ranges;
                laser_pub.publish(&scan);
                delay(20);
            }
        }
        nh.spinOnce();
        delay(20);
    }
}
