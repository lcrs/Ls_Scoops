// Scoops
// Does... image analysis things ^_^
// lewis@lewissaunders.com

#include "half.h"

#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include "/usr/discreet/smoke_2013.2.53/sparks/spark.h"
#else
	#include <GL/gl.h>
	#include "/usr/discreet/flame_2013.0.2/sparks/spark.h"
#endif

typedef struct {
	float r, g, b;
} colour;

typedef struct {
	float x, y;
} vert;

static int sampling = 0;
static int nextmode = 0;

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
SparkFloatColorStruct SparkFloatColor39 = {
	0.016, 0.010, 0.024, NULL	// RGB, callback
};
SparkIntStruct SparkInt66 = {
	0,							// Initial
	0,							// Min
	32768,						// Max
	1,							// Increment
	SPARK_FLAG_X,				// Flags
	(char *) "From X %d",		// Name
	NULL						// Callback
};
SparkIntStruct SparkInt67 = {
	0,							// Initial
	0,							// Min
	32768,						// Max
	1,							// Increment
	SPARK_FLAG_Y,				// Flags
	(char *) "From Y %d",		// Name
	NULL						// Callback
};
SparkIntStruct SparkInt73 = {
	0,							// Initial
	0,							// Min
	32768,						// Max
	1,							// Increment
	SPARK_FLAG_X,				// Flags
	(char *) "To X %d",			// Name
	NULL						// Callback
};
SparkIntStruct SparkInt74 = {
	0,							// Initial
	0,							// Min
	32768,						// Max
	1,							// Increment
	SPARK_FLAG_Y,				// Flags
	(char *) "To Y %d",			// Name
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
unsigned long* SparkInteract(SparkInfoStruct si, int sx, int sy, float pressure, float vx, float vy, float vz) {
	if(si.Context == SPARK_MODE_CONTROL1 && SparkBoolean6.Value == 1) {
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
		sparkViewingDraw();
	}
	if(si.Context == SPARK_MODE_CONTROL3) {
		if(sampling) {
			SparkInt73.Value = vx;
			SparkInt74.Value = vy;
			sparkControlUpdate(73);
			sparkControlUpdate(74);
			if(pressure == 0.0) {
				sampling = 0;
			}
		} else {
			if(pressure > 0.0) {
				sampling = 1;
				SparkInt66.Value = vx;
				SparkInt67.Value = vy;
				sparkControlUpdate(66);
				sparkControlUpdate(67);
				SparkInt73.Value = vx;
				SparkInt74.Value = vy;
				sparkControlUpdate(73);
				sparkControlUpdate(74);
			}
		}
		sparkViewingDraw();
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

// Miscellaneous events callback
void SparkEvent(SparkModuleEvent e) {
	switch(e) {
		case SPARK_EVENT_CONTROL1:
			nextmode = 1;
			sparkReprocess();
			break;
		case SPARK_EVENT_CONTROL2:
			nextmode = 2;
			sparkReprocess();
			break;
		case SPARK_EVENT_CONTROL3:
			nextmode = 3;
			sparkReprocess();
			break;
		default:
			break;
	}
}

// Work
unsigned long *SparkProcess(SparkInfoStruct si) {
	SparkMemBufStruct result, input;

	if(!getbuf(1, &result)) return(NULL);

	if(nextmode == 2) {
		// Scopes mode
		memset(result.Buffer, 0, result.BufSize);
		return(result.Buffer);
	} else {
		if(!getbuf(2, &input)) return(NULL);
		sparkCopyBuffer(input.Buffer, result.Buffer);
		return(result.Buffer);
	}
}

// More work
unsigned long *SparkAnalyse(SparkInfoStruct si) {
	SparkMemBufStruct input;

	if(!getbuf(2, &input)) return(NULL);
	sample(si, input);
	return(NULL);
}

// Return closest pixel value from buffer
half closest(SparkMemBufStruct *in, float x, float y, int colour) {
	if(x > in->BufWidth || y > in->BufHeight) return(0.0);
	if(x < 0 || y < 0) return(0.0);

	char *b = (char *) in->Buffer;
	b += in->Stride * (int) y + in->Inc * (int) x + colour * sizeof(half);

	if((b > (char *) in->Buffer + in->BufSize) || (b < (char *) in->Buffer)) {
		// Safety!
		return(0.0);
	}
	return(*((half *) b));
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

	if(si.Context == SPARK_MODE_CONTROL1) {
		// Sampler
		glBegin(GL_LINE_LOOP);
		glColor3f(1.0, 0.0, 0.0);
		glVertex2f(o.x + (ratio * zoom * (float)SparkInt7.Value - 10.0), o.y + (zoom * (float)SparkInt8.Value) - 10.0);
		glVertex2f(o.x + (ratio * zoom * (float)SparkInt7.Value + 10.0), o.y + (zoom * (float)SparkInt8.Value) - 10.0);
		glVertex2f(o.x + (ratio * zoom * (float)SparkInt7.Value + 10.0), o.y + (zoom * (float)SparkInt8.Value) + 10.0);
		glVertex2f(o.x + (ratio * zoom * (float)SparkInt7.Value - 10.0), o.y + (zoom * (float)SparkInt8.Value) + 10.0);
		glEnd();
		return;
	}
	if(si.Context == SPARK_MODE_CONTROL3) {
		// Slicer
		glBegin(GL_LINES);
		glColor3f(1.0, 1.0, 1.0);
		glVertex2f(o.x + (ratio * zoom * (float)SparkInt66.Value), o.y + (zoom * (float)SparkInt67.Value));
		glVertex2f(o.x + (ratio * zoom * (float)SparkInt73.Value), o.y + (zoom * (float)SparkInt74.Value));
		glEnd();

		// Now the hard messy bit...
		SparkMemBufStruct input;
		if(!getbuf(2, &input)) return;

		float dx = SparkInt73.Value - SparkInt66.Value;
		float dy = SparkInt74.Value - SparkInt67.Value;
		float rightx = -dy;
		float righty = dx;
		float len = sqrt(dx * dx + dy * dy);

		for(int colour = 0; colour < 3; colour++) {
			glBegin(GL_LINE_STRIP);
			switch(colour) {
				case 0:
					glColor3f(1.0, 0.0, 0.0);
					break;
				case 1:
					glColor3f(0.0, 1.0, 0.0);
					break;
				case 2:
					glColor3f(0.0, 0.0, 1.0);
					break;
				default:
					break;
			}
			for(float i = 0.0; i <= 1.0; i += 1.0/len) {
				float x = SparkInt66.Value + i * dx;
				float y = SparkInt67.Value + i * dy;
				float pix = closest(&input, x, y, colour);
				float graphx = x + (100.0/len) * rightx * pix;
				float graphy = y + (100.0/len) * righty * pix;
				glVertex2f(o.x + (ratio * zoom * graphx), o.y + (zoom * graphy));
			}
			glEnd();
		}

		return;
	}

	// Scopes
	SparkMemBufStruct input;
	if(!getbuf(2, &input)) return;

	const char * shadsrc =
		"in vec4 vert;"
		"in int gl_VertexID;"
		"void main() {"
    		"gl_Position = gl_Vertex;"
    		"gl_Vertex.r = float(gl_VertexID);"
		"}";

	GLuint prog = glCreateProgram();
	GLuint shad = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shad, 1, &shadsrc, NULL);
	glCompileShader(shad);
	glAttachShader(prog, shad);
	glLinkProgram(prog);
	glUseProgram(prog);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColor3f(0.7, 0.4, 0.2);

	// Deep breath
	for(int j = 0; j < input.BufHeight; j++) {
		char *a = (char *) input.Buffer + input.Stride * j;
		glVertexPointer(2, GL_HALF_FLOAT, 0, a);
		glDrawArrays(GL_LINE_STRIP, 0, input.BufWidth);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
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
	sparkControlTitle(SPARK_CONTROL_1, (char *) "Sampler");
	sparkControlTitle(SPARK_CONTROL_2, (char *) "Scopes");
	sparkControlTitle(SPARK_CONTROL_3, (char *) "Slicing");
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
