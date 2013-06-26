# Makefile for blub (or other arduino code for desktop)
#
#  2013-June  Scott Lawrence  yorgle@gmail.com
#

# Main target

TARG := blub

SRCS := Arduino/BLuB/BLuB.cpp

OTHER := \
	 src/EEExplorer.cpp \
	src/TinyBasicPlus.cpp

INCS += -Isrc


########################################
# arduino simulator for desktop

SRCSIM := ardsim/main.cpp \
	  ardsim/eesim.cpp \
	  ardsim/sersim.cpp \
	  ardsim/miscsim.cpp \
	  ardsim/iosim.cpp 

INCS += -Iardsim
	

########################################
# build settings and such

OBJS := $(SRCS:%.cpp=%.o) $(SRCSIM:%.cpp=%.o)

CFLAGS += -g -DDESKTOP -Wall -pedantic

LDFLAGS +=

LIBS += 


################################################################################
# main build rules

all: $(TARG)


%.o: %.cpp
	@echo $(CXX) $<
	@$(CXX) $(CFLAGS) $(DEFS) $(INCS) -c -o $@ $<


%.cpp: %.ino
	@echo use INO as CPP
	@ln $< $@


$(TARG): $(OBJS) 
	@echo Link $@
	@$(CXX) $(CFLAGS) $(OBJS) $(LDFLAGS) $(LIBS) -o $@


################################################################################

clean:
	@echo Remove build files
	-rm -f $(OBJS) $(TARG) $(TARG).exe src/BLuB.cpp
.PHONY: clean


test: $(TARG)
	./$(TARG)
.PHONY: testa


full: clean
	make test
.PHONY: full

install:
	  cp Arduino/BLuB/BLuB.ino ~/Dropbox/ArduinoSketches/BLuB/BLuB.ino
