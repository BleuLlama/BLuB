# Makefile for msf-dump
#
TARGA := blub

SRCSA := src/main.cpp \
	 src/eesim.cpp \
	 src/BLuB.cpp

INCS += -Isrc -Icontrib

OBJSA := $(SRCSA:%.cpp=%.o)

CFLAGS += -g -DDESKTOP

xx += -Wall -pedantic

LDFLAGS +=
LIBS += 

################################################################################

all: $(TARGA)

%.o: %.cpp
	@echo $(CXX) $<
	@$(CXX) $(CFLAGS) $(DEFS) $(INCS) -c -o $@ $<

%.cpp: %.ino
	@echo link INO to CPP
	@ln $< $@

$(TARGA): $(OBJSA) 
	@echo Link $@
	@$(CXX) $(CFLAGS) $(OBJSA) $(LDFLAGS) $(LIBS) -o $@

################################################################################

clean:
	@echo Remove build files
	-rm -f $(OBJSA) $(TARGA) $(TARGA).exe src/BLuB.cpp
.PHONY: clean

test: $(TARGA)
	./$(TARGA)
.PHONY: testa

full: clean
	make test
.PHONY: full
