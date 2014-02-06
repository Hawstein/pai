#include "surf.h"
#include <cstdio>

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s image_path", argv[0]);
    return 1;
  }
  hackday::SURFFeature feature;
  hackday::GetFeature(argv[1], &feature);
  printf("Width: %d, Height: %d", feature.roi.width, feature.roi.height);
  return 0;
}
