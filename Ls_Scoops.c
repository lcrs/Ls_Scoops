// Scoops
// lewis@lewissaunders.com

#include "half.h"
#include "/usr/discreet/presets/2016/sparks/spark.h"

unsigned long *scopeUICallback(int n, SparkInfoStruct si);

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
	float aspect = (float)w/(float)h;
	float border = SparkFloat9.Value / 100.0;

	unsigned long offset, pixels;
	sparkMpInfo(&offset, &pixels);

	// Image window
	if(SparkPup6.Value == 0) {
		float xscale, yscale, x0, y0;
		// 4-up
		xscale = 0.5;
		yscale = 0.5;
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
			xscale = 0.5 - 2.0 * border;
			yscale = 0.5 - 2.0 * border * aspect;
			x0 = 0.0 + border;
			y0 = 0.5 + border * aspect;
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
			xscale = 0.5 - 2.0 * border;
			yscale = 0.5 - 2.0 * border * aspect;
			x0 = 0.5 + border;
			y0 = 0.5 + border * aspect;
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
				half *resultpix = (half *) (resultbuf + (int)(y0 * h) * onerow + vert * onerow + (int)((x0 * w) + ((colour/3.0) * xscale * w)) * onepix + (int)(x * xscale * (1.0/3.0)) * onepix);
				resultpix[colour] += SparkFloat7.Value;
			}
		}
	}

	// Vectorscope
	if(SparkPup6.Value == 0 || SparkPup6.Value == 2) {
		float xscale, yscale, x0, y0;
		if(SparkPup6.Value == 0) {
			// 4-up
			xscale = 0.5 - 2.0 * border;
			yscale = 0.5 - 2.0 * border * aspect;
			x0 = 0.5 + border * aspect;
			y0 = 0.0 + border * aspect;
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
	sparkControlTitle(SPARK_CONTROL_1, (char *) "CPU Scopes");
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
