#include <cstdio>
#include "surf.h"

using namespace hackday;

void usage(char *prog);

int main(int argc, char **argv) {
  if (argc != 3) usage(argv[0]);
  SURFFeature query_feature, train_feature;
  if (!GetFeature(argv[1], &query_feature)) usage(argv[0]);
  if (!GetFeature(argv[2], &train_feature)) usage(argv[0]);
  bool matched = MatchFeature(query_feature, train_feature);
  printf("Matched: %d", matched);
  return 0;
}

void usage(char *prog) {
  printf("Usage: %s <image1> <image2>", prog);
  exit(1);
}
