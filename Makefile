CPPFLAGS = -Wall -v
LDFLAGS = -lopencv_core -lopencv_calib3d -lopencv_highgui \
	-lopencv_features2d -lopencv_nonfree -lopencv_flann
TARGETS = entry feature_homography

all: $(TARGETS)

$(TARGETS): surf.o

clean:
	rm -rf *.dSYM $(TARGETS) *.o
