#include "mongoose.h"
#include <cstdio>
#include <dirent.h>
#include <sys/dir.h>
#include "surf.h"

using namespace hackday;

void usage(char *prog);

static SURFFeatureManager manager;

static int MatchHandler(struct mg_connection *conn) {
  if (strcmp(conn->uri, "/match") == 0) {
    char filepath[256], type[2];
    mg_get_var(conn, "path", filepath, sizeof(filepath));
    mg_get_var(conn, "type", type, sizeof(type));
    SURFFeature feature;
    if (!manager.GetFeature(filepath, &feature)) {
      mg_send_status(conn, 400);
      return MG_REQUEST_PROCESSED;
    }
    SURFFeature *matched = NULL;
    std::string id;
    manager.FindMatchFeature(atoi(type),
                             feature,
                             std::map<std::string, std::string>(),
                             matched,
                             &id);
  }
  return MG_REQUEST_PROCESSED;
}

int main(int argc, char **argv) {
  if (argc != 1) usage(argv[0]);

  Timer timer;
  timer.Start();
  manager.LoadFeatureSet();
  printf("Load time elapsed: %ld", timer.elapsed_millis());
  StartHTTPServer(&MatchHandler);
  return 0;
}

void usage(char *prog) {
  printf("Usage: %s <image1> <image2>", prog);
  exit(1);
}
