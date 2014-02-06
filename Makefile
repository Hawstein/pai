CPPFLAGS = -Wall -v
LDFLAGS = -lopencv_core -lopencv_highgui -lopencv_features2d -lopencv_nonfree
TARGETS = entry

all: $(TARGETS)

$(TARGETS): surf.o

clean:
	rm -rf *.dSYM $(TARGETS) *.o
