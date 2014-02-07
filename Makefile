CPPFLAGS = -Wall -O2 -Ithirdparty/mongoose
LDFLAGS = -lopencv_core -lopencv_calib3d -lopencv_highgui \
	-lopencv_features2d -lopencv_nonfree -lopencv_flann -lopencv_imgproc
TARGETS = entry preprocess server

all: $(TARGETS)

$(TARGETS): surf.o thirdparty/mongoose/mongoose.o

clean:
	rm -rf *.dSYM $(TARGETS) *.o
