#include <WiFi.h>
#include <ros.h>
#include <std_msgs/UInt16.h>
#include <sensor_msgs/LaserScan.h>
#include <geometry_msgs/Twist.h>
#include <pthread.h>

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
bool spinning = false;

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
                        fullRanges[index] = range / 1000.0;

                        yield();
                    }
                    if (i == 2478)
                        gotScan = true;
                }
            }
        }
    }
}

void motorCallBack(const geometry_msgs::Twist& cmd) {
    Serial.println(cmd.linear.x);
    Serial.println(cmd.linear.y);
    if (cmd.linear.y == 0) {
        ledcWrite(4, 0);
        ledcWrite(3, 0);
        ledcWrite(10, 0);
        ledcWrite(5, 0);
    } else if (cmd.linear.y > 0) {
        ledcWrite(10, 0);
        ledcWrite(5, 0);
        ledcWrite(4, (cmd.linear.x < 0 ? -(cmd.linear.x) * 225 : cmd.linear.y * 225));
        ledcWrite(3, (cmd.linear.x > 0 ? cmd.linear.x * 225 : cmd.linear.y * 225));
    } else if (cmd.linear.y < 0) {
        ledcWrite(4, 0);
        ledcWrite(3, 0);
        ledcWrite(10, (cmd.linear.x < 0 ? -(cmd.linear.x) * 225 : -(cmd.linear.y) * 225));
        ledcWrite(5, (cmd.linear.x > 0 ? cmd.linear.x * 225 : -(cmd.linear.y) * 225));
    }
}

void *spinThread(void *a) {
    ros::Subscriber<geometry_msgs::Twist> sub("cmd_vel", motorCallBack);
    nh.subscribe(sub);
    while (true) {
        delay(5);
        if (!spinning)
            nh.spinOnce();
        yield();
    }
}

void setup() {
    Serial.begin(230400);
    pinMode(14, OUTPUT);
    pinMode(13, OUTPUT);
    pinMode(2, OUTPUT);
    pinMode(15, OUTPUT);
    pinMode(12, OUTPUT);

    ledcSetup(3, 2000, 8); // 2000 hz PWM, 8-bit resolution
    ledcSetup(4, 2000, 8);
    ledcSetup(5, 2000, 8);
    ledcSetup(10, 2000, 8);
    delay(25);
    ledcAttachPin(13, 3);
    ledcAttachPin(12, 4);
    ledcAttachPin(2, 5);
    ledcAttachPin(15, 10);
    digitalWrite(14, 1);
    delay(500);
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
    delay(20);
    pthread_t thread;
    int returnValue = pthread_create(&thread, NULL, spinThread, (void*)1);

    if (returnValue) {
        Serial.println("An error has occurred");
    }

    delay(500);
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
                spinning = true;
                nh.spinOnce();
                spinning = false;
                delay(5);
            }
        }
    }
    delay(2);
}
