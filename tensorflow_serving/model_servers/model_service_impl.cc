/* Copyright 2018 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow_serving/model_servers/model_service_impl.h"

#include "tensorflow_serving/model_servers/get_model_status_impl.h"
#include "tensorflow_serving/model_servers/grpc_status_util.h"
#include "tensorflow_serving/util/status_util.h"

namespace tensorflow {
  namespace serving {

    ::grpc::Status ModelServiceImpl::GetModelStatus(::grpc::ServerContext *context,
                                                    const GetModelStatusRequest *request,
                                                    GetModelStatusResponse *response) {
      const ::grpc::Status status = tensorflow::serving::ToGRPCStatus(
          GetModelStatusImpl::GetModelStatus(core_, *request, response));
      if (!status.ok()) {
        VLOG(1) << "GetModelStatus failed: " << status.error_message();
      }
      return status;
    }

    ::grpc::Status ModelServiceImpl::HandleReloadConfigRequest(::grpc::ServerContext *context,
                                                               const ReloadConfigRequest *request,
                                                               ReloadConfigResponse *response) {
      ModelServerConfig server_config = request->config();
      Status status;
      switch (server_config.config_case()) {
        case ModelServerConfig::kModelConfigList: {
          const ModelConfigList list = server_config.model_config_list();

          for (int index = 0; index < list.config_size(); index++) {
            const ModelConfig config = list.config(index);
            LOG(INFO) << "\nConfig entry"
                      << "\n\tindex : " << index
                      << "\n\tpath : " << config.base_path()
                      << "\n\tname : " << config.name()
                      << "\n\tplatform : " << config.model_platform();
          }
          status = core_->ReloadConfig(server_config);
          break;
        }
        default:
          status = errors::InvalidArgument(
              "ServerModelConfig type not supported by HandleReloadConfigRequest. Only ModelConfigList is currently supported");
      }

      if (!status.ok()) {
        LOG(ERROR) << "ReloadConfig failed: " << status.error_message();
      }

      const StatusProto status_proto = ToStatusProto(status);
      *response->mutable_status() = status_proto;
      return ToGRPCStatus(status);
    }

    ::grpc::Status ModelServiceImpl::HandleGetConfigRequest(::grpc::ServerContext *context,
                                                            const GetConfigRequest *request,
                                                            GetConfigResponse *response) {
      ModelServerConfig config = core_->GetConfig();
      *response->mutable_config() = config;
      return ToGRPCStatus(Status::OK());
    }

    ::grpc::Status ModelServiceImpl::HandleAddModelRequest(::grpc::ServerContext *context,
                                                           const AddModelRequest *request,
                                                           AddModelResponse *response) {
      const Status status = core_->AddModelConfigEntry(request->config());
      const StatusProto status_proto = ToStatusProto(status);
      *response->mutable_status() = status_proto;
      return ToGRPCStatus(status);
    }

    ::grpc::Status ModelServiceImpl::HandleRemoveModelRequest(::grpc::ServerContext *context,
                                                              const RemoveModelRequest *request,
                                                              RemoveModelResponse *response) {
      const Status status = core_->RemoveModelConfigEntry(request->model_name());
      const StatusProto status_proto = ToStatusProto(status);
      *response->mutable_status() = status_proto;
      return ToGRPCStatus(status);
    }

    ::grpc::Status ModelServiceImpl::HandleUpdateModelConfigRequest(::grpc::ServerContext *context,
                                                                    const UpdateModelConfigRequest *request,
                                                                    UpdateModelConfigResponse *response) {
      const Status status = core_->UpdateModelConfig(request->model_name(), request->config());
      const StatusProto status_proto = ToStatusProto(status);
      *response->mutable_status() = status_proto;
      return ToGRPCStatus(status);
    }
  }  // namespace serving
}  // namespace tensorflow
