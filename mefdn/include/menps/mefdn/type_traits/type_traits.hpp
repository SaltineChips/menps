
#pragma once

#include <menps/mefdn/lang.hpp>
#include <type_traits>

namespace menps {
namespace mefdn {

using std::is_convertible;
using std::remove_const;
using std::remove_extent;
using std::remove_cv;
using std::result_of;
using std::decay;
using std::is_reference;
using std::conditional;
using std::remove_reference;

} // namespace mefdn
} // namespace menps

