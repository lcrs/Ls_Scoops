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

unsigned long *SparkProcess(SparkInfoStruct si) {
	SparkMemBufStruct result, front;

	if(!getbuf(1, &result)) return(NULL);
	if(!getbuf(2, &front)) return(NULL);

	memset(result.Buffer, 0, result.BufSize);

	int row = result.Stride;
	int pixel = result.Inc;
	int w = si.FrameWidth;
	int h = si.FrameHeight;
	char *fbuf = (char *) front.Buffer;
	char *rbuf = (char *) result.Buffer;
	half *f, *r;

	for(int y = 0; y < h - 1; y+=2) {
		for(int x = 0; x < w - 1; x+=2) {
			f = (half *) (fbuf + y * row + x * pixel);
			for(int c = 0; c < 3; c++) {
				int v = f[c] * (h - 1);
				if(v > h - 1) v = h - 1;
				r = (half *) (rbuf + v * row + x * pixel);
				r[c] += 0.1;
			}
		}
	}
	
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
