#include "classifier.h"
#include <fstream>
#include <string>
#include <opencv2/highgui/highgui.hpp>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usage:\n  image_classification model_path image_path\n");
    return 1;
  }
  auto model_path = argv[1];
  auto image_path = argv[2];
  cv::Mat img = cv::imread(argv[2]);
  if (!img.data) {
    fprintf(stderr, "failed to load image: %s\n", image_path);
    return 1;
  }

  mm_handle_t classifier{};
  int status{};
  status = mmdeploy_classifier_create_by_path(model_path, "cpu", 0, &classifier);
  if (status != MM_SUCCESS) {
    fprintf(stderr, "failed to create classifier, code: %d\n", (int) status);
    return 1;
  }

  mm_mat_t mat{img.data, img.rows, img.cols, 3, MM_BGR, MM_INT8};

  mm_class_t *res{};
  int *res_count{};
  status = mmdeploy_classifier_apply(classifier, &mat, 1, &res, &res_count);
  if (status != MM_SUCCESS) {
    fprintf(stderr, "failed to apply classifier, code: %d\n", (int) status);
    return 1;
  }

  fprintf(stderr, "label: %d, score: %.4f\n", res->label_id, res->score);

  mmdeploy_classifier_release_result(res, res_count, 1);

  mmdeploy_classifier_destroy(classifier);

  return 0;
}