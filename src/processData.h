//############################################################
//			processData
//Process retrieved Data
//############################################################

bool isTimeToSaveNewMeasure(int patternID) {
	int now = timeClient.getEpochTime();
	int delta = (now - lastTimeProcessed[patternID]);
	bool result = delta >= config.intervalParameter;
	if (config.debugLevel >= debugInfo) {
		swSer.println("[processData::isTimeToSaveNewMeasure]");
		swSer.println("patternID:" + String(patternID));
		swSer.println("lastTime: " + String(lastTimeProcessed[patternID]));
		swSer.println("getTime:  " + String(now));
		swSer.println("delta:    " + String(delta));
		swSer.println("result:   " + String(result));
	}
	return result;
}

bool isDeltaBetweenLastMeasuresBigEnough(Measure* m) {
	int mvalue = m->value;
	int lvalue = lastMeasure[m->patternId].value;
	int delta = abs(mvalue - lvalue);
	bool result = delta >= config.intervalParameter;
	if (config.debugLevel >= debugInfo) {
		swSer.println("[processData::isDeltaBetweenLastMeasuresBigEnough]");
		swSer.println("patternID:" + String(m->patternId));
		swSer.println("mvalue:   " + String(mvalue));
		swSer.println("lvalue:   " + String(lvalue));
		swSer.println("delta:    " + String(delta));
		swSer.println("result:   " + String(result));
	}
	return result;
}

void registerMeasure(Measure* newMeasure) {
	if (config.debugLevel >= debugInfo) {
		swSer.println("[processData::registerMeasure]");
	}

	if (newMeasure == NULL) {
		if (config.debugLevel > debugOff) {
			swSer.println("Register failed, ptr null");
		}
		return;
	} else {
		if (config.debugLevel >= debugInfo) {
			swSer.println("NewMeasure:");
			swSer.println("PatternID: " + String(newMeasure->patternId));
			swSer.println("Value:     " + String(newMeasure->value));
			swSer.println("Time:      " + String(newMeasure->timestamp));
		}
	}
	/*if (measure_size == 0) {
		if (config.debugLevel >= debugInfo)
			swSer.println("MeasureCatalog is empty.");
		addMeasure(newMeasure);
	} else
	*/
	if (config.intervalMode == DynamicOutputInterval
			&& isDeltaBetweenLastMeasuresBigEnough(newMeasure)) {
		if (config.debugLevel >= debugInfo) {
			swSer.println("DynamicOutputInterval-delta reached");
		}
		addMeasure(newMeasure);
	}
	else if (config.intervalMode == FixedOutputInterval
			&& isTimeToSaveNewMeasure(newMeasure->patternId)) {
		if (config.debugLevel >= debugInfo) {
			swSer.println("FixedOutputInterval-delta reached");
		}
		addMeasure(newMeasure);
	} else {
		if (config.debugLevel >= debugInfo) {
			swSer.println("Delta to low. Skipping measure.");
		}
		free(newMeasure);
	}
}

void processData() {
	int pos = inbuffer.indexOf(PREFIX);
	if (pos == -1)
		return;

	int procLines = 0;
	while (pos != -1 && procLines++ < PROCLINELIMIT) {
		String line;
		int end = inbuffer.indexOf(PREFIX, pos + 1);
		if (end == -1)
			line = inbuffer.substring(pos);
		else
			line = inbuffer.substring(pos, end);

		for (int i = 0; i < config.patternCount; i++) {
			if (line.indexOf(config.patternBegin[i]) > -1) {
				int skip = line.indexOf(config.patternBegin[i])
						+ config.patternBegin[i].length()
						+ config.patternOffset[i];
				line = line.substring(skip);
				int end = line.indexOf(config.patternEnd[i]);
				if (config.patternLength[i] > 0
						&& end > config.patternLength[i])
					end = config.patternLength[i];
				line = line.substring(0, end);

				char buffer[PROCESSBUFFERSIZE];
				line.toCharArray(buffer, PROCESSBUFFERSIZE);
				ulong value = strtol(buffer, NULL, 16);
				Measure* ptr = (Measure*) malloc(sizeof(Measure));
				if (ptr == NULL) {
					if (config.debugLevel > debugOff) {
						swSer.println(
								"[processData] Out of memory. Skipping measure.");
					}
				} else {
					ptr->value = value;
					ptr->patternId = i;
					ptr->timestamp = timeClient.getEpochTime();
					registerMeasure(ptr);
				}
			}
		}
		pos = inbuffer.indexOf(PREFIX, pos + 1);
	}

	//reset inbuffer
	inbuffer = "";
	return;
}
