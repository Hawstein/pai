#include <cstdio>
#include <dirent.h>
#include <sys/dir.h>
#include "surf.h"

using namespace hackday;

void usage(char *prog);

int main(int argc, char **argv) {
  if (argc != 3) usage(argv[0]);

  Timer timer;
  timer.Start();
  SURFFeatureManager manager;
  manager.CalculateFeatureSet(kVideo, argv[1]);
  printf("Calculate takes %ld ms", timer.elapsed_millis());

  return 0;
}

void usage(char *prog) {
  printf("Usage: %s folder type", prog);
  exit(1);
}
