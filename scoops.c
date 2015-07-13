// Scoops
// lewis@lewissaunders.com

#include "half.h"
#include "/usr/discreet/presets/2016/sparks/spark.h"

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
			int vert = frontpix[colour] * 255;
			if(vert > h - 1) vert = h - 1;
			half *resultpix = (half *) (resultbuf + vert * onerow + x * onepix);
			resultpix[colour] += 0.2;
		}
	}

}

unsigned long *SparkProcess(SparkInfoStruct si) {
	SparkMemBufStruct result, front;

	if(!getbuf(1, &result)) return(NULL);
	if(!getbuf(2, &front)) return(NULL);

	memset(result.Buffer, 0, result.BufSize);
	sparkMpFork((void(*)())scopeThread, 3, &front, &result);

	return(result.Buffer);
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
