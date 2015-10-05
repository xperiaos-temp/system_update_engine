//
// Copyright (C) 2013 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "update_engine/install_plan.h"

#include <base/format_macros.h>
#include <base/logging.h>
#include <base/strings/stringprintf.h>

#include "update_engine/payload_constants.h"
#include "update_engine/utils.h"

using std::string;

namespace chromeos_update_engine {

InstallPlan::InstallPlan(bool is_resume,
                         bool is_full_update,
                         const string& url,
                         uint64_t payload_size,
                         const string& payload_hash,
                         uint64_t metadata_size,
                         const string& metadata_signature,
                         const string& public_key_rsa)
    : is_resume(is_resume),
      is_full_update(is_full_update),
      download_url(url),
      payload_size(payload_size),
      payload_hash(payload_hash),
      metadata_size(metadata_size),
      metadata_signature(metadata_signature),
      hash_checks_mandatory(false),
      powerwash_required(false),
      public_key_rsa(public_key_rsa) {}


bool InstallPlan::operator==(const InstallPlan& that) const {
  return ((is_resume == that.is_resume) &&
          (is_full_update == that.is_full_update) &&
          (download_url == that.download_url) &&
          (payload_size == that.payload_size) &&
          (payload_hash == that.payload_hash) &&
          (metadata_size == that.metadata_size) &&
          (metadata_signature == that.metadata_signature) &&
          (source_slot == that.source_slot) &&
          (target_slot == that.target_slot) &&
          (partitions == that.partitions));
}

bool InstallPlan::operator!=(const InstallPlan& that) const {
  return !((*this) == that);
}

void InstallPlan::Dump() const {
  string partitions_str;
  for (const auto& partition : partitions) {
    partitions_str += base::StringPrintf(
        ", part: %s (source_size: %" PRIu64 ", target_size %" PRIu64 ")",
        partition.name.c_str(), partition.source_size, partition.target_size);
  }

  LOG(INFO) << "InstallPlan: "
            << (is_resume ? "resume" : "new_update")
            << ", payload type: " << (is_full_update ? "full" : "delta")
            << ", source_slot: " << BootControlInterface::SlotName(source_slot)
            << ", target_slot: " << BootControlInterface::SlotName(target_slot)
            << ", url: " << download_url
            << ", payload size: " << payload_size
            << ", payload hash: " << payload_hash
            << ", metadata size: " << metadata_size
            << ", metadata signature: " << metadata_signature
            << partitions_str
            << ", hash_checks_mandatory: " << utils::ToString(
                hash_checks_mandatory)
            << ", powerwash_required: " << utils::ToString(
                powerwash_required);
}

bool InstallPlan::LoadPartitionsFromSlots(SystemState* system_state) {
  bool result = true;
  for (Partition& partition : partitions) {
    if (source_slot != BootControlInterface::kInvalidSlot) {
      result = system_state->boot_control()->GetPartitionDevice(
          partition.name, source_slot, &partition.source_path) && result;
    } else {
      partition.source_path.clear();
    }

    if (target_slot != BootControlInterface::kInvalidSlot) {
      result = system_state->boot_control()->GetPartitionDevice(
          partition.name, target_slot, &partition.target_path) && result;
    } else {
      partition.target_path.clear();
    }
  }
  return result;
}

bool InstallPlan::Partition::operator==(
    const InstallPlan::Partition& that) const {
  return (name == that.name &&
          source_path == that.source_path &&
          source_size == that.source_size &&
          source_hash == that.source_hash &&
          target_path == that.target_path &&
          target_size == that.target_size &&
          target_hash == that.target_hash &&
          run_postinstall == that.run_postinstall);
}

}  // namespace chromeos_update_engine
