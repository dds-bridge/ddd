# HOW TO COMPILE AND LINK THIS WITH THE LATEST VERSION OF DDS
#
# The following is suggested:
#
#   - obtain the latest .cpp and dll.h file for version 1.0x
#     of Bo Haglund's DDS
#   - rename them to dds10x.cpp and dds10x.h and copy them into this directory
#   - change the include below to reflect the version dds10x.h
#
# Linux: compile and link as follows:
#   g++ -O2 -Wall -o ./ddd ddd.cpp dds10x.cpp defs.cpp timer.cpp giblib.cpp rng.cpp
#
# Windows: compile and link as follows:
#   g++ -O2 -Wall -o ddd.exe ddd.cpp dds10x.cpp defs.cpp timer.cpp giblib.cpp rng.cpp
#
# for debugging change the switch '-O2' to '-g'
#
# Note: on MingW you must have _WIN32 defined to compile code in timer.cpp
#
# Using the Makefile:
# There is a Makefile supplied, edit it for the correct version of dds10x.
# Run 'make' (or 'mingw32-make') to produce the executable,
# note that the executable is written to ../exe/ddd.
# To make the debug executable run 'make debug',
# the debug executable is written to ../exe/dddd.
#
# The Makefile creates separate directories ../objr and ../objd for
# compiled object files, apart from the directory ../exe for executables.

CXX=g++
CXXFLAGS=-g -Wall -O2
LDFLAGS=-L..
LIBS=-ldds
OBJ=ddd.o defs.o timer.o giblib.o rng.o
PREFIX=/usr/local

ddd: dds11x.h $(OBJ)
	g++ $(LDFLAGS) -o $@ $^ $(LIBS)

dds11x.h:
	ln -s ../dll.h $@

install:
	install -d $(DESTDIR)$(PREFIX)/games
	install ddd $(DESTDIR)$(PREFIX)/games/dds

clean:
	rm -f $(OBJ) ddd dds11x.h
