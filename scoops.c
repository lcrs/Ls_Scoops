// Scoops
// Does... image analysis things ^_^
// lewis@lewissaunders.com

#include "/usr/discreet/smoke_2013.2.53/sparks/spark.h"
#include "half.h"

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

typedef struct {
	float r, g, b;
} colour;

typedef struct {
	float x, y;
} vert;

static int sampling = 0;

unsigned long *cbPick(int v, SparkInfoStruct i);

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

// UI
SparkBooleanStruct SparkBoolean6 = {
	0,
	(char *) "Pick sample location...",
	cbPick
};
SparkIntStruct SparkInt7 = {
	0,							// Initial
	0,							// Min
	32768,						// Max
	1,							// Increment
	SPARK_FLAG_X,				// Flags
	(char *) "Sample at X %d",	// Name
	NULL						// Callback
};
SparkIntStruct SparkInt8 = {
	0,							// Initial
	0,							// Min
	32768,						// Max
	1,							// Increment
	SPARK_FLAG_Y,				// Flags
	(char *) "Sample at Y %d",	// Name
	NULL						// Callback
};
SparkFloatStruct SparkFloat9 = {
	0.0,						// Initial
	-INFINITY,					// Min
	+INFINITY,					// Max
	0.001,						// Increment
	SPARK_FLAG_NO_INPUT ,		// Flags
	(char *) "R %f",			// Name
	NULL						// Callback
};
SparkFloatStruct SparkFloat10 = {
	0.0,						// Initial
	-INFINITY,					// Min
	+INFINITY,					// Max
	0.001,						// Increment
	SPARK_FLAG_NO_INPUT ,		// Flags
	(char *) "G %f",			// Name
	NULL						// Callback
};
SparkFloatStruct SparkFloat11 = {
	0.0,						// Initial
	-INFINITY,					// Min
	+INFINITY,					// Max
	0.001,						// Increment
	SPARK_FLAG_NO_INPUT ,		// Flags
	(char *) "B %f",			// Name
	NULL						// Callback
};
SparkFloatStruct SparkFloat12 = {
	0.0,						// Initial
	-INFINITY,					// Min
	+INFINITY,					// Max
	0.001,						// Increment
	SPARK_FLAG_NO_INPUT ,		// Flags
	(char *) "Luma %f",			// Name
	NULL						// Callback
};

// Pick button callback
unsigned long* cbPick(int v, SparkInfoStruct i) {
	if(SparkBoolean6.Value == 0) {
		// Back to normal
		sparkViewingCursor(SPARK_CURSOR_ARROW);
	} else {
		// Pickin' time
		sparkViewingCursor(SPARK_CURSOR_PICK);
	}
	return(NULL);
}

// Clicky draggy callback
unsigned long *SparkInteract(SparkInfoStruct si, int sx, int sy, float pressure, float vx, float vy, float vz) {
	if(SparkBoolean6.Value == 1) {
		if(pressure > 0.0) {
			// Pick sample point
			sampling = 1;
			SparkInt7.Value = vx;
			SparkInt8.Value = vy;
			sparkControlUpdate(7);
			sparkControlUpdate(8);
			SparkAnalyse(si);
		} else {
			if(sampling) {
				sampling = 0;
				SparkBoolean6.Value = 0;
				sparkControlUpdate(6);
				cbPick(0, si);
				sparkChClear(1, 9);
				sparkChClear(1, 10);
				sparkChClear(1, 11);
				sparkChClear(1, 12);
				SparkAnalyse(si);
			}
		}
	}
	return(NULL);
}

// Set keys
void sample(SparkInfoStruct si, SparkMemBufStruct buf) {
	colour sampled;

	switch(buf.BufDepth) {
		case SPARKBUF_RGB_48_3x16_FP:
			char *b;
			half *pix;
			b = (char *) buf.Buffer;
			b += buf.Stride * SparkInt8.Value;
			b += buf.Inc * SparkInt7.Value;
			pix = (half *) b;
			sampled.r = (float) *pix;
			sampled.g = (float) *(pix + 1);
			sampled.b = (float) *(pix + 2);
			break;
		default:
			printf("Unhandled pixel format what the hell?\n");
			sampled = (colour) {0.0, 0.0, 0.0};
	}
	sparkSetCurveKey(SPARK_UI_CONTROL, 9, si.FrameNo, sampled.r);
	sparkSetCurveKey(SPARK_UI_CONTROL, 10, si.FrameNo, sampled.g);
	sparkSetCurveKey(SPARK_UI_CONTROL, 11, si.FrameNo, sampled.b);
	sparkSetCurveKey(SPARK_UI_CONTROL, 12, si.FrameNo, luma(sampled));
	sparkControlUpdate(9);
	sparkControlUpdate(10);
	sparkControlUpdate(11);
	sparkControlUpdate(12);
}

// Work
unsigned long *SparkProcess(SparkInfoStruct si) {
	SparkMemBufStruct result, input;

	if(!getbuf(2, &input)) return(NULL);
	if(!getbuf(1, &result)) return(NULL);
	sparkCopyBuffer(input.Buffer, result.Buffer);
	memset(result.Buffer, 0, result.BufSize);
	return(result.Buffer);
}

// More work
unsigned long *SparkAnalyse(SparkInfoStruct si) {
	SparkMemBufStruct input;

	if(!getbuf(2, &input)) return(NULL);
	sample(si, input);
	return(NULL);
}

// Further working
void SparkOverlay(SparkInfoStruct si, float zoom) {
	float ratio = sparkGetViewerRatio();

	if(zoom < 1.0) {
	    zoom = 1.0 / (2.0 - zoom);
	}

	float w = si.FrameWidth * ratio * zoom;
	float h = si.FrameHeight * zoom;
	vert o = {si.FrameBufferX, si.FrameBufferY};
	vert p = {o.x + w, o.y + h};

	SparkMemBufStruct input;
	if(!getbuf(2, &input)) return;

	glColor3f(0.003, 0.002, 0.001);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.5);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	char *b;
	for(int j = 0; j < input.BufHeight; j++) {
		b = (char *) input.Buffer + input.Stride * j;
		glBegin(GL_LINE_STRIP);
			half *pix;
			for(int i = 0; i < input.BufWidth; i++) {
				b += input.Inc;
				pix = (half *) b;
				glVertex2f(o.x + (float) i * ratio * zoom, o.y + (float) *pix * h);
			}
		glEnd();
	}
	glLineWidth(1.0);
	glDisable(GL_BLEND);
}

// Number of clips required
int SparkClips(void) {
	return(1);
}

// New memory interface to keep Batch happy
void SparkMemoryTempBuffers(void) {
}

// Module level, not desktop
unsigned int SparkInitialise(SparkInfoStruct sparkInfo) {
	return(SPARK_MODULE);
}

// Bit depths, we love them all
int SparkIsInputFormatSupported(SparkPixelFormat fmt) {
	return(1);
}

// Stop
void SparkUnInitialise(SparkInfoStruct sparkInfo) {
}
