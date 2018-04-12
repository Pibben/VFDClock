/*
 * main.cpp
 *
 *  Created on: 15 feb 2010
 *      Author: Per Zetterlund
 */

#include "WProgram.h"
#include "Wire.h"
#include "OneWire.h"
#include "DallasTemperature.h"

extern "C" void __cxa_pure_virtual(void) {
    while(1);
}

static void getTimeHHMM(char* str) {
	strncpy(str, "2334", 5);
}

class Page {
public:
	virtual void draw() = 0;
};

class ClockPage : public Page {
private:
	OneWire oneWire;
	DallasTemperature sensors;
public:
	ClockPage() : oneWire(10), sensors(&oneWire) {
		sensors.begin();
	}
	virtual void draw() {
		char str[5];
		getTimeHHMM(str);

		Wire.beginTransmission(0x2e);
#if 0
		Wire.send(0xfe);
		Wire.send(0x6e); /* Big digit mode */

		/* Hours 1*/
		Wire.send(0xfe);
		Wire.send(0x23);
		Wire.send(0x00);
		Wire.send(str[0]-'0');
		/* Hours 2*/
		Wire.send(0xfe);
		Wire.send(0x23);
		Wire.send(0x04);
		Wire.send(str[1]-'0');

		/* Colon */
		Wire.send(0xfe);
		Wire.send(0x47);
		Wire.send(0x07);
		Wire.send(0x02);
		Wire.send(0x94);

		Wire.send(0xfe);
		Wire.send(0x47);
		Wire.send(0x07);
		Wire.send(0x03);
		Wire.send(0x94);

		/* Minutes 1*/
		Wire.send(0xfe);
		Wire.send(0x23);
		Wire.send(0x08);
		Wire.send(str[2]-'0');
		/* Minutes 2*/
		Wire.send(0xfe);
		Wire.send(0x23);
		Wire.send(0x0B);
		Wire.send(str[3]-'0');
#endif
		sensors.requestTemperatures();
		float tf = sensors.getTempCByIndex(0);
		int ti = (int)(tf*100.0f);

		Wire.send(0xfe);
		Wire.send(0x47);
		Wire.send(13);
		Wire.send(1);
		Wire.send('T');
		Wire.send('0'+ti/1000);
		Wire.send('0'+(ti%1000)/100);
		Wire.send(',');
		Wire.send('0'+(ti%100)/10);
		Wire.send('0'+ti%10);

		float tf2 = sensors.getTempCByIndex(1);
		int ti2 = (int)(tf2*100.0f);

		Wire.send(0xfe);
		Wire.send(0x47);
		Wire.send(13);
		Wire.send(2);
		Wire.send('T');
		Wire.send('0'+ti2/1000);
		Wire.send('0'+(ti2%1000)/100);
		Wire.send(',');
		Wire.send('0'+(ti2%100)/10);
		Wire.send('0'+ti2%10);

		Wire.endTransmission();
	}
};

class AquariumPage : public Page {
public:
	AquariumPage() {
		/* Create custom chars */
		Wire.beginTransmission(0x2e);
		Wire.send(0xfe);
		Wire.send(0x4e);
		Wire.send(0x00);
		Wire.send(B00011);
		Wire.send(B00100);
		Wire.send(B01010);
		Wire.send(B10000);
		Wire.send(B01000);
		Wire.send(B00111);
		Wire.send(B00000);
		Wire.send(B00000);
		Wire.endTransmission();

		Wire.beginTransmission(0x2e);
		Wire.send(0xfe);
		Wire.send(0x4e);
		Wire.send(0x01);
		Wire.send(B10001);
		Wire.send(B01011);
		Wire.send(B00101);
		Wire.send(B00101);
		Wire.send(B01011);
		Wire.send(B10001);
		Wire.send(B00000);
		Wire.send(B00000);
		Wire.endTransmission();

		for(int i = 0; i < NUM_FISH; ++i) {
			fish[i].x = random(18);
			fish[i].y = random(4);
			fish[i].speed = random(32)+1;
			fish[i].counter = fish[i].speed;
		}
	}
	virtual void draw() {
		animateFish();
		//bigClock();
	}
private:
	static const uint8_t NUM_FISH = 5;

	struct Fish {
		uint8_t x,y;
		uint8_t speed;
		uint8_t counter;

		void draw() {
			Wire.beginTransmission(0x2e);
			Wire.send(0xfe);
			Wire.send(0x47);
			Wire.send(x+1);
			Wire.send(y+1);
			Wire.send(0);
			Wire.send(1);
			Wire.endTransmission();
		}
	};

	Fish fish[NUM_FISH];

	void animateFish() {
		Wire.beginTransmission(0x2e);
		Wire.send(0xfe);
		Wire.send(0x58);
		Wire.endTransmission();

		for(int i = 0;i < NUM_FISH;++i){
			fish[i].draw();
			fish[i].counter--;
			if(fish[i].counter == 0){
				if(fish[i].x == 0){
					fish[i].x = 18;
					fish[i].y = random(4);
					fish[i].speed = random(32) + 1;
				}else{
					fish[i].x--;
				}
				fish[i].counter = fish[i].speed;
			}

		}
	}
};

int main(int argc, char** argv) {
	init();
	Wire.begin();

	AquariumPage aq;
	ClockPage clk;
	Page* page = &clk;

	Wire.beginTransmission(0x2e);
	Wire.send(0xfe);
	Wire.send(0x58); /* Clear screen */
	Wire.send(0xfe);
	Wire.send(0x54); /* Cursor off */
	Wire.endTransmission();

	for(;;) {
		page->draw();
		delay(100);
	}
}

