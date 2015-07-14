// Scoops
// lewis@lewissaunders.com

#include "half.h"
#include "/usr/discreet/presets/2016/sparks/spark.h"

unsigned long *scopeUICallback(int n, SparkInfoStruct si);

SparkFloatStruct SparkFloat7 = {
	0.15,						// Value
	-INFINITY,					// Min
	+INFINITY,					// Max
	0.002,						// Increment
	0,							// Flags
	(char *) "Intensity %.4f",	// Title
	scopeUICallback				// Callback
};

SparkFloatStruct SparkFloat8 = {
	255.0,						// Value
	0,							// Min
	+INFINITY,					// Max
	1.0,						// Increment
	0,							// Flags
	(char *) "Height %.2f",		// Title
	scopeUICallback				// Callback
};

// Check a buffer
int getbuf(int n, SparkMemBufStruct *b) {
	if(!sparkMemGetBuffer(n, b)) {
		printf("Failed to get buffer %d\n", n);
		return(0);
	}
	if(!(b->BufState & MEMBUF_LOCKED)) {
		printf("Failed to lock buffer %d\n", n);
		return(0);
	}
	return(1);
}

// Process one slice of the input, run multiple times by sparkMpFork() below
void scopeThread(SparkMemBufStruct *front, SparkMemBufStruct *result) {
	char *frontbuf = (char *) front->Buffer;
	char *resultbuf = (char *) result->Buffer;
	int onepix = front->Inc;
	int onerow = front->Stride;
	int w = front->BufWidth;
	int h = front->BufHeight;

	unsigned long offset, pixels;
	sparkMpInfo(&offset, &pixels);
	
	for(int i = offset; i < offset + pixels; i++) {
		int y = i / w;
		int x = i % w;

		half *frontpix = (half *) (frontbuf + y * onerow + x * onepix);
		for(int colour = 0; colour < 3; colour++) {
			int vert = frontpix[colour] * SparkFloat8.Value;
			if(vert > h - 1) vert = h - 1;
			half *resultpix = (half *) (resultbuf + vert * onerow + x * onepix);
			resultpix[colour] += SparkFloat7.Value;
		}
	}

}

// Per-frame work
unsigned long *SparkProcess(SparkInfoStruct si) {
	SparkMemBufStruct result, front;

	if(!getbuf(1, &result)) return(NULL);
	if(!getbuf(2, &front)) return(NULL);

	memset(result.Buffer, 0, result.BufSize);
	sparkMpFork((void(*)())scopeThread, 3, &front, &result);

	return(result.Buffer);
}

// UI click-drag callback
unsigned long *scopeUICallback(int n, SparkInfoStruct si) {
	unsigned long *r = SparkProcess(si);
	sparkViewingDraw();
	return(r);
}

// Number of clips required
int SparkClips(void) {
	return(1);
}

// New memory interface to keep Batch happy
void SparkMemoryTempBuffers(void) {
}

// Module level, not desktop
unsigned int SparkInitialise(SparkInfoStruct si) {
	sparkControlTitle(SPARK_CONTROL_1, (char *) "Scopes");
	return(SPARK_MODULE);
}

// Bit depths
int SparkIsInputFormatSupported(SparkPixelFormat fmt) {
	switch(fmt) {
		case SPARKBUF_RGB_48_3x16_FP:
			return(1);
		default:
			return(0);
	}
}

// Stop
void SparkUnInitialise(SparkInfoStruct sparkInfo) {
}
