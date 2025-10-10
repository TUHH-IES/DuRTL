/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/design.hpp>

#include <vcd-parser/VCDFile.hpp>
#include <vcd-parser/VCDTypes.hpp>

#include <memory>

namespace ducode {
const VCDScope& find_root_scope(const std::shared_ptr<VCDFile>& vcd_file, const Design& design);
}// namespace ducode