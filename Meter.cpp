// Do not remove the include below
#include "Meter.h"

//#define POWER
#define GAS

#define NETWORKID 100 //the same on all nodes that talk to each other
#define GATEWAYID 1
#define FREQUENCY RF69_915MHZ
#define ENCRYPTKEY "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define FLASH_ADDR 1
#define BLINK_MS 50
#define LED_PIN 9
#define INTER 1

#define SLEEP_CYCLES 40

#ifdef GAS
#define NODEID 10 //unique for each node on same network
#define REPORT_PULSES 1
#endif

#ifdef POWER
#define NODEID 20 //unique for each node on same network
#define REPORT_PULSES 10
#endif

//#define DEBUG

#define D(input) {Serial.print(input); Serial.flush();}
#define Dln(input) {Serial.println(input); Serial.flush();}

RFM69 radio;

volatile unsigned long pulse_count = 0;
unsigned long last_sent_pulse_count = 0;

byte cycle = 0;
byte pulse_cycle = 0;

void blink(int cnt) {
        for (int i = 0; i < cnt; i++) {
                pinMode(LED_PIN, OUTPUT);
                digitalWrite(LED_PIN, HIGH);
                delay(BLINK_MS);
                digitalWrite(LED_PIN, LOW);
                delay(BLINK_MS);
                pinMode(LED_PIN, INPUT);
        }
}

void pulse() {
        noInterrupts();
        pulse_count++;
        interrupts();
}

void setup() {
#ifdef DEBUG
        Serial.begin(115200);
        D("Start node ");
        Dln(NODEID);
#endif

        radio.initialize(FREQUENCY, NODEID, NETWORKID);
        radio.encrypt(ENCRYPTKEY);
        radio.sleep();

        blink(4);

        attachInterrupt(INTER, pulse, RISING);
        interrupts();
}

void transmit() {
#ifdef DEBUG
        D("Transmit ");
        Dln(pulse_count);
#endif
        char buff[10];
        sprintf(buff, "%lu", pulse_count);
        boolean sent = radio.sendWithRetry(GATEWAYID, buff,(byte) strlen(buff));
        radio.sleep();
#ifdef DEBUG
        D("Sent ");
        Dln(sent?"OK":"Fail");
#endif

}

void loop() {
#ifdef DEBUG
        blink(1);
        D("pulse_count ");
        D(pulse_count);
        D(", last_sent_pulse_count ");
        Dln(last_sent_pulse_count);
#endif

        if (pulse_count != last_sent_pulse_count) pulse_cycle++;
        cycle++;

        if (pulse_cycle >= REPORT_PULSES || cycle >= SLEEP_CYCLES) {
                transmit();
                last_sent_pulse_count = pulse_count;
                cycle = 0;
                pulse_cycle = 0;
        } else {
#ifdef DEBUG
                D(cycle);
                Dln(" - Sleeping");
#endif
        }
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}

