CFLAGS = -O3 -march=core2 -mtune=native -fPIC -DDL_LITTLE_ENDIAN
LDFLAGS = -fPIC

ifeq ($(shell uname), Darwin)
	CFLAGS += -D_DARWIN_USE_64_BIT_INODE
	LDFLAGS += -dynamiclib -undefined dynamic_lookup
	EXT = spark
endif
ifeq ($(shell uname), Linux)
	CFLAGS += -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
	LDFLAGS += -shared -Bsymbolic
	EXT = spark_x86_64
endif

all: Ls_Scoops.$(EXT)

Ls_Scoops.$(EXT): Ls_Scoops.o Makefile
	g++ $(LDFLAGS) Ls_Scoops.o -o Ls_Scoops.$(EXT)

Ls_Scoops.o: Ls_Scoops.c half.h halfExport.h spark.h Makefile
	g++ $(CFLAGS) -c Ls_Scoops.c -o Ls_Scoops.o

spark.h: Makefile
	ln -sf `ls /usr/discreet/presets/*/sparks/spark.h | head -n1` spark.h

clean:
	rm -f Ls_Scoops.$(EXT) Ls_Scoops.o spark.h
