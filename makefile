# Makefile for msf-dump
#
TARGA := blub

SRCSAI := src/BLuB.ino
SRCSAC := src/main.cpp 

INCS += -Isrc -Icontrib

OBJSAC := $(SRCSAC:%.cpp=%.o)
OBJSAI := $(SRCSAI:%.ino=%.o)

CFLAGS += -g

xx += -Wall -pedantic

LDFLAGS += 
LIBS += 

################################################################################

all: $(TARGA)

%.o: %.cpp
	@echo $(CXX) $<
	@$(CXX) $(CFLAGS) $(DEFS) $(INCS) -c -o $@ $<

%.o: %.ino
	@echo $(CXX) $<
	@$(CXX) $(CFLAGS) $(DEFS) $(INCS) -c -o $@ $<

$(TARGA): $(OBJSAC) $(OBJSAI)
	@echo Link $@
	@$(CXX) $(CFLAGS) $(OBJSA) $(LDFLAGS) $(LIBS) -o $@

################################################################################

clean:
	@echo Remove build files
	-rm -f $(OBJSAC) $(OBJSAI) $(TARGA) $(TARGA).exe
.PHONY: clean

test: $(TARGA)
	./$(TARGA)
.PHONY: testa

full: clean
	make test
.PHONY: full
