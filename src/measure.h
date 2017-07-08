//############################################################
//			Measure
//############################################################

#ifndef measure_h
#define measure_h

typedef struct {
	uint id;
	uint timestamp;
	uint patternId;
	ulong value;
} Measure;

//ringbuffer
Measure* measures;
uint measure_maxSize = 9;
uint measure_size = 0;
uint measure_writeIndex = 0;
uint measure_readIndex = 0;

//save last measures to know if new data should be processed
int lastTimeProcessed[MAXPATTERN] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
Measure lastMeasure[MAXPATTERN];

//unique id for each measurement
uint idcounter = 0;

void addMeasure(Measure* m) {
	if (measure_size >= measure_maxSize) {
		//choice: keep old or overwrite?
		return;
	} else {
		measure_size++;
	}

	memcpy(&measures[measure_writeIndex], m, sizeof(Measure));
	memcpy(&lastMeasure[m->patternId], m, sizeof(Measure));
	lastTimeProcessed[m->patternId] = m->timestamp;
	measures[measure_writeIndex].id = idcounter++;
	Measure* measure = &measures[measure_writeIndex];

	measure_writeIndex = (measure_writeIndex + 1) % measure_maxSize;
	free(m);
}

Measure* getNextMeasure() {
	if (measure_size == 0) {
		return NULL;
	}
	Measure* m = &measures[measure_readIndex];
	measure_readIndex = (measure_readIndex + 1) % measure_maxSize;

	return m;
}

void popMeasure() {
	if (measure_size > 0) { //leave last ref item
		measure_size--;
	}
}

#endif
