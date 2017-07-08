//############################################################
//			GatherData
//Retrieve telegrams from IR-Head
//############################################################

bool gatherData() {
	while (Serial.available() > 0) {
		int b = Serial.read();
		char buf[2];
		sprintf(buf, "%02x", b);
		inbuffer.concat(buf);
		if (b == ENDCHAR && inbuffer.endsWith(POSTFIX)) {
			if (config.debugLevel >= debugInfo) {
				swSer.println("Received: " + inbuffer);
			}
			return true;
		}
	}
	return false;
}
