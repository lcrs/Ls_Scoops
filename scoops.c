// Scoops
// lewis@lewissaunders.com

#include "half.h"

#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include "/usr/discreet/presets/2016/sparks/spark.h"
#else
	#include <GL/gl.h>
	#include "/usr/discreet/flame_2013.0.2/sparks/spark.h"
#endif

typedef struct {
	float r, g, b;
} colour;


float luma(colour c) {
	return(0.2126 * c.r + 0.7152 * c.g + 0.0722 * c.b);
}

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
	char *fbuf = (char *) front->Buffer;
	char *rbuf = (char *) result->Buffer;
	int pixel = front->Inc;
	int row = front->Stride;
	int w = front->BufWidth;
	int h = front->BufHeight;

	unsigned long offset, pixels;
	sparkMpInfo(&offset, &pixels);
	
	for(int i = offset; i < offset + pixels; i++) {
		int y = i / w;
		int x = i % w;

		half *f = (half *) (fbuf + y * row + x * pixel);
		for(int colour = 0; colour < 3; colour++) {
			int vert = f[colour] * 255;
			if(vert > h - 1) vert = h - 1;
			half *r = (half *) (rbuf + vert * row + x * pixel);
			r[colour] += 0.2;
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

// Bit depths yo
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
