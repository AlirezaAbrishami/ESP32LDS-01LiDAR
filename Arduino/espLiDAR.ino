uint8_t raw_bytes[2520];
uint16_t ranges[360];
uint16_t intensities[360];
bool gotScan = false;
long blinkMillis;

void poll() {
    Serial.println("poll");
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

                        int intensity = (byte1 << 8) + byte0;
                        int range = (byte3 << 8) + byte2;
                        ranges[index] = range / 10;
                        intensities[index] = intensity;
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
}

void loop() {
    delay(5);
    yield();
    if (blinkMillis && millis() - blinkMillis >= 300) {
        digitalWrite(14, 0);
        blinkMillis = 0;
    }
    raw_bytes[0] = 0;
    raw_bytes[1] = 0;
    poll();
    if (gotScan) {
        for (int i = 0; i < 360; i++) {
            Serial.println("angle " + String(i) + ": " + ranges[i] + "  " + intensities[i]);
        }
        digitalWrite(14, 1);
        blinkMillis = millis();
        gotScan = false;
    }
}
