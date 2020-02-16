PGMS = spl.exe spl.inf

OBJS = splbook.obj general.obj 

DEFS = -O

all: $(OBJS) $(PGMS)

spl.exe:  spl.cpp spl.h spl.rc splbook.obj general.obj
	icc $(DEFS) -B"/PM:PM" spl.cpp splbook.obj general.obj
	rc spl.rc

spl.inf: spl.ipf
	ipfc /inf spl.ipf

%.obj: %.cpp
	icc $(DEFS) -c $<
