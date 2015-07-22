// Scoops
// Does... image analysis things ^_^
// lewis@lewissaunders.com

#include "half.h"
#include "/usr/discreet/presets/2016/sparks/spark.h"

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

int sampling = 0;
int nextmode = 1;
GLuint prog, vshad;
half *ramp;
const char * vshadsrc = "void main() {\
		vec4 v;\
		v.y = gl_Color.g * 400.0 + 600.0;\
		v.x = gl_Vertex.x * 0.4 + 20.0;\
		v.z = 0.0;\
		v.w = 1.0;\
		gl_Position = gl_ModelViewProjectionMatrix * v;\
    	gl_FrontColor = vec4(0.1, 0.06, 0.02, 1.0);\
	}\
	";

unsigned long *cbPick(int v, SparkInfoStruct i);
unsigned long *scopeUICallback(int n, SparkInfoStruct si);

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
// CPU scopes
SparkPupStruct SparkPup6 = {
	0,							// Value
	4,							// Count
	scopeUICallback,			// Callback
	{"4-up",					// Titles
	 "Waveform",
	 "Vectorscope",
	 "RGB Parade"}
};
SparkFloatStruct SparkFloat7 = {
	0.03,						// Value
	-INFINITY,					// Min
	+INFINITY,					// Max
	0.002,						// Increment
	0,							// Flags
	(char *) "Intensity %.4f",	// Title
	scopeUICallback				// Callback
};
SparkFloatStruct SparkFloat8 = {
	1.0,						// Value
	0.0,						// Min
	+INFINITY,					// Max
	0.01,						// Increment
	0,							// Flags
	(char *) "Height %.2f",		// Title
	scopeUICallback				// Callback
};
SparkFloatStruct SparkFloat9 = {
	2.0,						// Value
	0.0,						// Min
	+INFINITY,					// Max
	0.01,						// Increment
	0,							// Flags
	(char *) "Borders %.2f",		// Title
	scopeUICallback				// Callback
};
SparkFloatStruct SparkFloat10 = {
	0.5,						// Value
	0.01,						// Min
	0.99,						// Max
	0.01,						// Increment
	0,							// Flags
	(char *) "Quadness %.2f",	// Title
	scopeUICallback				// Callback
};

// GPU scopes
SparkFloatColorStruct SparkFloatColor39 = {
	0.016, 0.010, 0.024, NULL	// RGB, callback
};
SparkIntStruct SparkInt41 = {
	1,							// Initial
	1,							// Min
	128,						// Max
	1,							// Increment
	SPARK_FLAG_X,				// Flags
	(char *) "Downres %d",		// Name
	NULL						// Callback
};

// Sampler
SparkBooleanStruct SparkBoolean64 = {
	0,
	(char *) "Pick sample location...",
	cbPick
};
SparkIntStruct SparkInt65 = {
	0,							// Initial
	0,							// Min
	32768,						// Max
	1,							// Increment
	SPARK_FLAG_X,				// Flags
	(char *) "Sample at X %d",	// Name
	NULL						// Callback
};
SparkIntStruct SparkInt66 = {
	0,							// Initial
	0,							// Min
	32768,						// Max
	1,							// Increment
	SPARK_FLAG_Y,				// Flags
	(char *) "Sample at Y %d",	// Name
	NULL						// Callback
};
SparkFloatStruct SparkFloat67 = {
	0.0,						// Initial
	-INFINITY,					// Min
	+INFINITY,					// Max
	0.001,						// Increment
	SPARK_FLAG_NO_INPUT ,		// Flags
	(char *) "R %f",			// Name
	NULL						// Callback
};
SparkFloatStruct SparkFloat68 = {
	0.0,						// Initial
	-INFINITY,					// Min
	+INFINITY,					// Max
	0.001,						// Increment
	SPARK_FLAG_NO_INPUT ,		// Flags
	(char *) "G %f",			// Name
	NULL						// Callback
};
SparkFloatStruct SparkFloat69 = {
	0.0,						// Initial
	-INFINITY,					// Min
	+INFINITY,					// Max
	0.001,						// Increment
	SPARK_FLAG_NO_INPUT ,		// Flags
	(char *) "B %f",			// Name
	NULL						// Callback
};
SparkFloatStruct SparkFloat70 = {
	0.0,						// Initial
	-INFINITY,					// Min
	+INFINITY,					// Max
	0.001,						// Increment
	SPARK_FLAG_NO_INPUT ,		// Flags
	(char *) "Luma %f",			// Name
	NULL						// Callback
};

// Slicer
SparkIntStruct SparkInt93 = {
	0,							// Initial
	0,							// Min
	32768,						// Max
	1,							// Increment
	SPARK_FLAG_X,				// Flags
	(char *) "From X %d",		// Name
	NULL						// Callback
};
SparkIntStruct SparkInt94 = {
	0,							// Initial
	0,							// Min
	32768,						// Max
	1,							// Increment
	SPARK_FLAG_Y,				// Flags
	(char *) "From Y %d",		// Name
	NULL						// Callback
};
SparkIntStruct SparkInt95 = {
	0,							// Initial
	0,							// Min
	32768,						// Max
	1,							// Increment
	SPARK_FLAG_X,				// Flags
	(char *) "To X %d",			// Name
	NULL						// Callback
};
SparkIntStruct SparkInt96 = {
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
	if(SparkBoolean64.Value == 0) {
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
	if(si.Context == SPARK_MODE_CONTROL3 && SparkBoolean64.Value == 1) {
		// Sampler mode
		if(pressure > 0.0) {
			// Pick sample point
			sampling = 1;
			SparkInt65.Value = vx;
			SparkInt93.Value = vy;
			sparkControlUpdate(65);
			sparkControlUpdate(66);
			SparkAnalyse(si);
		} else {
			if(sampling) {
				sampling = 0;
				SparkBoolean64.Value = 0;
				sparkControlUpdate(64);
				cbPick(0, si);
				sparkChClear(1, 67);
				sparkChClear(1, 68);
				sparkChClear(1, 69);
				sparkChClear(1, 70);
				SparkAnalyse(si);
			}
		}
		sparkViewingDraw();
	}
	if(si.Context == SPARK_MODE_CONTROL4) {
		// Slicing
		if(sampling) {
			SparkInt95.Value = vx;
			SparkInt96.Value = vy;
			sparkControlUpdate(73);
			sparkControlUpdate(74);
			if(pressure == 0.0) {
				sampling = 0;
			}
		} else {
			if(pressure > 0.0) {
				sampling = 1;
				SparkInt93.Value = vx;
				SparkInt94.Value = vy;
				sparkControlUpdate(66);
				sparkControlUpdate(67);
				SparkInt95.Value = vx;
				SparkInt96.Value = vy;
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
			b += buf.Stride * SparkInt93.Value;
			b += buf.Inc * SparkInt65.Value;
			pix = (half *) b;
			sampled.r = (float) *pix;
			sampled.g = (float) *(pix + 1);
			sampled.b = (float) *(pix + 2);
			break;
		default:
			printf("Unhandled pixel format what the hell?\n");
			sampled = (colour) {0.0, 0.0, 0.0};
	}
	sparkSetCurveKey(SPARK_UI_CONTROL, 67, si.FrameNo, sampled.r);
	sparkSetCurveKey(SPARK_UI_CONTROL, 68, si.FrameNo, sampled.g);
	sparkSetCurveKey(SPARK_UI_CONTROL, 69, si.FrameNo, sampled.b);
	sparkSetCurveKey(SPARK_UI_CONTROL, 70, si.FrameNo, luma(sampled));
	sparkControlUpdate(67);
	sparkControlUpdate(68);
	sparkControlUpdate(69);
	sparkControlUpdate(70);
}

// Process one slice of the input, run multiple times by sparkMpFork() below
void scopeThread(SparkMemBufStruct *front, SparkMemBufStruct *result) {
	char *frontbuf = (char *) front->Buffer;
	char *resultbuf = (char *) result->Buffer;
	int onepix = front->Inc;
	int onerow = front->Stride;
	int w = front->BufWidth;
	int h = front->BufHeight;
	float aspect = (float)w/(float)h;
	float border = SparkFloat9.Value / 100.0;
	float quad1 = SparkFloat10.Value;
	float quad2 = 1.0 - SparkFloat10.Value;
	if(quad2 < border + 0.01) {
		quad2 = border + 0.01;
		quad1 = 1.0 - quad2;
	} else if(quad1 < border + 0.01) {
		quad1 = border + 0.01;
		quad2 = 1.0 - quad1;
	}

	unsigned long offset, pixels;
	sparkMpInfo(&offset, &pixels);

	// Image window
	if(SparkPup6.Value == 0) {
		float xscale, yscale, x0, y0;
		// 4-up
		xscale = quad1;
		yscale = quad1;
		x0 = 0.0;
		y0 = 0.0;

		for(int i = offset; i < offset + pixels; i++) {
			int y = i / w;
			int x = i % w;

			half *frontpix = (half *) (frontbuf + y * onerow + x * onepix);
			half *resultpix = (half *) (resultbuf + (int)(y0 * h) * onerow + (int)(y * yscale) * onerow + (int)(x0 * w) * onepix + (int)(x * xscale) * onepix);

			memcpy(resultpix, frontpix, 3 * sizeof(half));
		}
	}

	// Waveform
	if(SparkPup6.Value == 0 || SparkPup6.Value == 1) {
		float xscale, yscale, x0, y0;
		if(SparkPup6.Value == 0) {
			// 4-up
			xscale = quad1 - 2.0 * border;
			yscale = quad2 - 2.0 * border * aspect;
			x0 = 0.0 + border;
			y0 = quad1 + border * aspect;
		} else {
			// Waveform
			xscale = 1.0 - 2.0 * border;
			yscale = 1.0 - 2.0 * border * aspect;
			x0 = 0.0 + border;
			y0 = 0.0 + border * aspect;
		}
		int maxvert = yscale * (h - 1);
		int minvert = y0;

		for(int i = offset; i < offset + pixels; i++) {
			int y = i / w;
			int x = i % w;

			half *frontpix = (half *) (frontbuf + y * onerow + x * onepix);

			for(int colour = 0; colour < 3; colour++) {
				int vert = frontpix[colour] * SparkFloat8.Value * yscale * h;
				if(vert > maxvert) vert = maxvert;
				if(vert < minvert) vert = minvert;
				half *resultpix = (half *) (resultbuf + (int)(y0 * h) * onerow + vert * onerow + (int)(x0 * w) * onepix + (int)(x * xscale) * onepix);
				resultpix[colour] += SparkFloat7.Value;
			}
		}
	}

	// RGB Parade
	if(SparkPup6.Value == 0 || SparkPup6.Value == 3) {
		float xscale, yscale, x0, y0;
		if(SparkPup6.Value == 0) {
			// 4-up
			xscale = quad2 - 2.0 * border;
			yscale = quad1 - 2.0 * border * aspect;
			x0 = quad1 + border;
			y0 = 0.0 + border * aspect;
		} else {
			// RGB Parade
			xscale = 1.0 - 2.0 * border;
			yscale = 1.0 - 2.0 * border * aspect;
			x0 = 0.0 + border;
			y0 = 0.0 + border * aspect;
		}
		int maxvert = yscale * (h - 1);
		int minvert = y0;

		for(int i = offset; i < offset + pixels; i++) {
			int y = i / w;
			int x = i % w;

			half *frontpix = (half *) (frontbuf + y * onerow + x * onepix);

			for(int colour = 0; colour < 3; colour++) {
				int vert = frontpix[colour] * SparkFloat8.Value * yscale * h;
				if(vert > maxvert) vert = maxvert;
				if(vert < minvert) vert = minvert;
				half *resultpix = (half *) (resultbuf + (int)(y0 * h) * onerow + vert * onerow + (int)((x0 * w) + ((colour/(3.0-border*6)) * xscale * w)) * onepix + (int)(x * xscale * (1.0/(3.0+border*12))) * onepix);
				resultpix[colour] += SparkFloat7.Value;
			}
		}
	}

	// Vectorscope
	if(SparkPup6.Value == 0 || SparkPup6.Value == 2) {
		float xscale, yscale, x0, y0;
		if(SparkPup6.Value == 0) {
			// 4-up
			xscale = quad2 - 2.0 * border;
			yscale = quad2 - 2.0 * border * aspect;
			x0 = quad1 + border * aspect;
			y0 = quad1 + border * aspect;
		} else {
			// Vectorscope
			xscale = 1.0 - 2.0 * border;
			yscale = 1.0 - 2.0 * border * aspect;
			x0 = 0.0 + border * aspect;
			y0 = 0.0 + border * aspect;
		}

		for(int i = offset; i < offset + pixels; i++) {
			int y = i / w;
			int x = i % w;

			half *frontpix = (half *) (frontbuf + y * onerow + x * onepix);
			half r = frontpix[0];
			half g = frontpix[1];
			half b = frontpix[2];

			float yy =  0.299 * r + 0.587 * g + 0.114  * b;
			float cb = -0.169 * r - 0.331 * g + 0.499  * b;
			float cr =  0.499 * r - 0.418 * g - 0.0813 * b;
			cr *= (float)h/(float)w;
			if(cb >  0.5) cb =  0.5;
			if(cb < -0.5) cb = -0.5;
			if(cr >  0.5) cr =  0.5;
			if(cr < -0.5) cr = -0.5;
			cb *= -0.95;
			cr *= 0.95;
			cb += 0.5;
			cr += 0.5;

			char *o = resultbuf;
			o += (int)(((y0 * h) + (cb * yscale * h))) * onerow;
			o += (int)(((x0 * w) + (cr * xscale * w))) * onepix;

			half *resultpix = (half *) o;
			resultpix[0] += SparkFloat7.Value * r;
			resultpix[1] += SparkFloat7.Value * g;
			resultpix[2] += SparkFloat7.Value * b;
		}
	}
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
		case SPARK_EVENT_CONTROL4:
			nextmode = 4;
			sparkReprocess();
			break;
		default:
			break;
	}
}

// Per-frame work
unsigned long *SparkProcess(SparkInfoStruct si) {
	SparkMemBufStruct result, input;

	if(!getbuf(1, &result)) return(NULL);
	if(!getbuf(2, &input)) return(NULL);

	if(nextmode == 1) {
		// CPU scopes
		memset(result.Buffer, 0, result.BufSize);
		sparkMpFork((void(*)())scopeThread, 2, &input, &result);
		return(result.Buffer);
	} else {
		// Pass through front
		sparkCopyBuffer(input.Buffer, result.Buffer);
		return(result.Buffer);
	}
}

// Sample per frame when Analyze clicked
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

// Draw OpenGL overlay per frame
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
		// CPU Scopes
		return;
	}
	if(si.Context == SPARK_MODE_CONTROL2) {
		// GPU Scopes
		SparkMemBufStruct input;
		if(!getbuf(2, &input)) return;

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(3, GL_HALF_FLOAT, 0, input.Buffer);
		glVertexPointer(2, GL_HALF_FLOAT, 0, ramp);

		glUseProgram(prog);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDrawArrays(GL_LINE_STRIP, 0, input.BufWidth * input.BufHeight);
		glFlush();
		glFinish();
		glDisable(GL_BLEND);
		glUseProgram(0);

		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
	if(si.Context == SPARK_MODE_CONTROL3) {
		// Sampler
		glBegin(GL_LINE_LOOP);
		glColor3f(1.0, 0.0, 0.0);
		glVertex2f(o.x + (ratio * zoom * (float)SparkInt65.Value - 10.0), o.y + (zoom * (float)SparkInt93.Value) - 10.0);
		glVertex2f(o.x + (ratio * zoom * (float)SparkInt65.Value + 10.0), o.y + (zoom * (float)SparkInt93.Value) - 10.0);
		glVertex2f(o.x + (ratio * zoom * (float)SparkInt65.Value + 10.0), o.y + (zoom * (float)SparkInt93.Value) + 10.0);
		glVertex2f(o.x + (ratio * zoom * (float)SparkInt65.Value - 10.0), o.y + (zoom * (float)SparkInt93.Value) + 10.0);
		glEnd();
		return;
	}
	if(si.Context == SPARK_MODE_CONTROL4) {
		// Slicer
		glBegin(GL_LINES);
		glColor3f(1.0, 1.0, 1.0);
		glVertex2f(o.x + (ratio * zoom * (float)SparkInt93.Value), o.y + (zoom * (float)SparkInt94.Value));
		glVertex2f(o.x + (ratio * zoom * (float)SparkInt95.Value), o.y + (zoom * (float)SparkInt96.Value));
		glEnd();

		// Now the hard messy bit...
		SparkMemBufStruct input;
		if(!getbuf(2, &input)) return;

		float dx = SparkInt95.Value - SparkInt93.Value;
		float dy = SparkInt96.Value - SparkInt94.Value;
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
				float x = SparkInt93.Value + i * dx;
				float y = SparkInt94.Value + i * dy;
				float pix = closest(&input, x, y, colour);
				float graphx = x + (100.0/len) * rightx * pix;
				float graphy = y + (100.0/len) * righty * pix;
				glVertex2f(o.x + (ratio * zoom * graphx), o.y + (zoom * graphy));
			}
			glEnd();
		}

		return;
	}
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
	// One-time init for GPU scopes
	ramp = (half *) malloc(si.FrameWidth * si.FrameHeight * sizeof(half) * 2);
	for(int i = 0; i < si.FrameHeight; i++) {
		for(int j = 0; j < si.FrameWidth; j++) {
			ramp[si.FrameWidth * 2 * i + j * 2] = j;
			ramp[si.FrameWidth * 2 * i + j * 2 + 1] = i;
		}
	}
	prog = glCreateProgram();
	vshad = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshad, 1, &vshadsrc, NULL);
	glCompileShader(vshad);
	glAttachShader(prog, vshad);
	glLinkProgram(prog);

	sparkControlTitle(SPARK_CONTROL_1, (char *) "CPU Scopes"); // controls 6 to 34
	sparkControlTitle(SPARK_CONTROL_2, (char *) "GPU Scopes"); // controls 35 to 63
	sparkControlTitle(SPARK_CONTROL_3, (char *) "Sampler");    // controls 64 to 92
	sparkControlTitle(SPARK_CONTROL_4, (char *) "Slicer");     // controls 93 to 121
	return(SPARK_MODULE);
}

// UI click-drag callback
unsigned long *scopeUICallback(int n, SparkInfoStruct si) {
	unsigned long *r = SparkProcess(si);
	sparkViewingDraw();
	return(r);
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
	free(ramp);
}
