#include "/usr/discreet/smoke_2013.2.53/sparks/spark.h"

// Single control, integer at position 0
SparkIntStruct SparkInt6 = {
	0,							// Initial
	0,							// Min
	255,						// Max
	1,							// Increment
	SPARK_FLAG_NO_INPUT,		// Read-only
	"Bottom left pixel red %d",	// Label
	NULL						// Callback
};

// Number of clips required
int SparkClips(void) {
	return(1);
}

// New memory interface to keep Batch happy
void SparkMemoryTempBuffers(void) {
}

// Module level, no desktop
unsigned int SparkInitialise(SparkInfoStruct sparkInfo) {
	return(SPARK_MODULE);
}

// Return how many frames to output per input frame
int SparkProcessStart(SparkInfoStruct sparkInfo) {
	return(1);
}

// Work
unsigned long *SparkProcess(SparkInfoStruct sparkInfo) {
	SparkMemBufStruct resultBuffer, inputBuffer;
	char *result, *input;

	// Check result buffer is happy
	if(!sparkMemGetBuffer(1, &resultBuffer)) {
		return(NULL);
	}
	if(!(resultBuffer.BufState & MEMBUF_LOCKED)) {
		return(NULL);
	}

	// Check input buffer is happy
	if(!sparkMemGetBuffer(2, &inputBuffer)) {
		return(NULL);
	}
	if(!(inputBuffer.BufState & MEMBUF_LOCKED)) {
		return(NULL);
	}

	input = (char *)inputBuffer.Buffer;
	sparkSetCurveKey(SPARK_UI_CONTROL, 6, sparkInfo.FrameNo, input[0]);

	// Return buffer of whatever, probably garbage
	return(resultBuffer.Buffer);
}

// Stop
void SparkUnInitialise(SparkInfoStruct sparkInfo) {
}
