#include "mongoose.h"
#include <cstdio>
#include <dirent.h>
#include <sys/dir.h>
#include "surf.h"
#include "filter.h"

using namespace hackday;

void usage(char *prog);

static SURFFeatureManager manager;
static std::vector<Data> dataset;

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


    std::map<std::string, std::string> candidates =
      Filter::GetCandidates(filepath, dataset);
    if (candidates.empty()) {
      printf("Candidate not found\n");
      mg_send_status(conn, 400);
      mg_printf_data(conn, "");
      return MG_REQUEST_PROCESSED;
    }
    std::vector<std::string> ids;
    if (manager.FindMatchFeature(atoi(type),
                                 feature,
                                 candidates,
                                 &ids)) {
      mg_send_status(conn, 200);
      for (std::vector<std::string>::iterator it = ids.begin();
           it != ids.end();
           ++it) {
        printf("Find match, id is %s\n", it->c_str());
        mg_printf_data(conn, it->c_str());
        if (it != (ids.end() - 1)) {
          mg_printf_data(conn, ",");
        }
      }
    } else {
      mg_send_status(conn, 400);
      mg_printf_data(conn, "");
    }
  }
  printf("Process complete\n");
  return MG_REQUEST_PROCESSED;
}

int main(int argc, char **argv) {
  if (argc != 1) usage(argv[0]);

  Timer timer;
  timer.Start();
  //manager.CalculateFeatureSet(kVideo, "/Users/jiluo/projects/movie");
  const string data_name = "data.hd";
  Filter::LoadData(data_name, dataset);
  manager.LoadFeatureSet();
  printf("Load time elapsed: %ld\n", timer.elapsed_millis());
  StartHTTPServer(&MatchHandler);
  return 0;
}

void usage(char *prog) {
  printf("Usage: %s <image1> <image2>", prog);
  exit(1);
}
