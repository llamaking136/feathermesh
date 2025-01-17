#include <Arduino.h>


void setup() {
    Serial2.begin(115200);
    Serial1.begin(9600);
}

void loop() {
    if (Serial1.available() > 0)
    {
        Serial2.write(Serial1.read());
    }
}