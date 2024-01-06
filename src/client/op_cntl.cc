#include "op_cntl.h"

#include "glog/logging.h"

void OpCntl::Write(node_id_t node, const Range& w) {
  int ret = OK;
  if (node == node_id_t(-1)) {
    for (auto node : client_->ListAllNodes()) {
      ret = client_->Write(node, w);
      if (ret != OK) {
        LOG(ERROR) << "write to node:" << node << " failed, code:" << ret;
      }
    }
  } else {
    ret = client_->Write(node, w);
    if (ret != OK) {
      LOG(ERROR) << "write to node:" << node << " failed, code:" << ret;
    }
  }
}

void OpCntl::Read(node_id_t node) {
  std::string read_ret;
  int ret = OK;
  if (node == node_id_t(-1)) {
    for (auto node : client_->ListAllNodes()) {
      ret = client_->Read(node, &read_ret);
      if (ret != OK) {
        LOG(ERROR) << "read from node:" << node << " failed, code:" << ret;
      } else {
        LOG(INFO) << "read from node:" << node << " data:\n" << read_ret;
      }
    }
  } else {
    ret = client_->Read(node, &read_ret);
    if (ret != OK) {
      LOG(ERROR) << "read from node:" << node << " failed, code:" << ret;
    } else {
      LOG(INFO) << "read from node:" << node << " data:\n" << read_ret;
    }
  }
}

void OpCntl::Clear(node_id_t node) {
  int ret = OK;
  if (node == node_id_t(-1)) {
    for (auto node : client_->ListAllNodes()) {
      ret = client_->Clear(node);
      if (ret != OK) {
        LOG(ERROR) << "clear node:" << node << " failed, code:" << ret;
      } else {
        LOG(INFO) << "clear node:" << node;
      }
    }
  } else {
    ret = client_->Clear(node);
    if (ret != OK) {
      LOG(ERROR) << "clear node:" << node << " failed, code:" << ret;
    } else {
      LOG(INFO) << "clear node:" << node;
    }
  }
}
