#which files to compile that are part of the program
OBJS = gspy.c audiosamples.c

#compiler
CC = gcc

#compiler flags with options
# -w supresses all warnings.
COMPILER_FLAGS = -Wall `pkg-config --cflags gstreamer-1.0` -ansi -g -c

#linkerflags the libraries to link against eg GL...
LINKER_FLAGS = `pkg-config --libs gstreamer-1.0` -lm


#objname is the target/name of the executable
OBJ_NAME = gspy

all: gspy

gspy: audiosamples.o gspy.o
	$(CC) gspy.o audiosamples.o $(LINKER_FLAGS) -o $(OBJ_NAME) 

gspy.o: gspy.c
	$(CC) $(COMPILER_FLAGS) gspy.c

audiosamples.o: audiosamples.c
	$(CC) $(COMPILER_FLAGS) audiosamples.c

clean:
	rm -rf *.o gspy
