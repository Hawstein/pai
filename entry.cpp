#include <cstdio>
#include <dirent.h>
#include <sys/dir.h>
#include "surf.h"

using namespace hackday;

void usage(char *prog);

int main(int argc, char **argv) {
  if (argc != 3) usage(argv[0]);

  SURFFeatureManager manager;
  SURFFeature query_feature, train_feature;
  if (!manager.GetFeature(argv[1], &query_feature)) usage(argv[0]);
  if (!manager.GetFeature(argv[2], &train_feature)) usage(argv[0]);

  Timer timer;
  timer.Start();
  bool matched = manager.MatchFeature(query_feature, train_feature);
  printf("Matched: %d, time elapsed: %ld", matched, timer.elapsed_millis());
  return 0;
}

void usage(char *prog) {
  printf("Usage: %s <image1> <image2>", prog);
  exit(1);
}
