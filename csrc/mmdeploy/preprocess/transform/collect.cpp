// Copyright (c) OpenMMLab. All rights reserved.

#include "collect.h"

#include "mmdeploy/archive/json_archive.h"
#include "mmdeploy/core/logger.h"

namespace mmdeploy {

CollectImpl::CollectImpl(const Value &args) {
  if (!args.contains("keys") || !args["keys"].is_array()) {
    throw std::invalid_argument("'keys' is missed in arguments, or it is not an array as expected");
  }
  if (args.contains("meta_keys") && !args["meta_keys"].is_array()) {
    throw std::invalid_argument("'meta_keys' has to be an array");
  }

  for (auto &v : args["keys"]) {
    arg_.keys.push_back(v.get<std::string>());
  }
  if (args.contains("meta_keys")) {
    for (auto &v : args["meta_keys"]) {
      arg_.meta_keys.push_back(v.get<std::string>());
    }
  }
}

Result<Value> CollectImpl::Process(const Value &input) {
  MMDEPLOY_DEBUG("input: {}", to_json(input).dump(2));
  Value output;

  // collect 'ori_img' and 'attribute' from `input`, because those two fields
  // are given by users, not generated by transform ops
  if (input.contains("ori_img")) {
    output["ori_img"] = input["ori_img"];
  }
  if (input.contains("attribute")) {
    output["attribute"] = input["attribute"];
  }

  for (auto &meta_key : arg_.meta_keys) {
    if (input.contains(meta_key)) {
      output["img_metas"][meta_key] = input[meta_key];
    }
  }
  for (auto &key : arg_.keys) {
    if (!input.contains(key)) {
      MMDEPLOY_INFO("missed key '{}' in input", key);
      return Status(eInvalidArgument);
    } else {
      output[key] = input[key];
    }
  }

  MMDEPLOY_DEBUG("output: {}", to_json(output).dump(2));
  return output;
}

Collect::Collect(const Value &args, int version) : Transform(args) {
  impl_ = Registry<CollectImpl>::Get().GetCreator("cpu", version)->Create(args);
}

Result<Value> Collect::Process(const Value &input) { return impl_->Process(input); }

class CollectCreator : public Creator<Transform> {
 public:
  const char *GetName() const override { return "Collect"; }
  int GetVersion() const override { return version_; }
  ReturnType Create(const Value &args) override {
    return std::make_unique<Collect>(args, version_);
  }

 private:
  int version_{1};
};

REGISTER_MODULE(Transform, CollectCreator);

MMDEPLOY_DEFINE_REGISTRY(CollectImpl);

}  // namespace mmdeploy
