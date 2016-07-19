
#pragma once

#include <mgcom/rma/requester.hpp>
#include <mgcom/rma/allocator.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace ibv {

class endpoint;
class completer;

mgbase::unique_ptr<rma::requester> make_scheduled_rma_requester(endpoint&, completer&, rma::allocator&, mgcom::endpoint&);

} // namespace ibv
} // namespace mgcom

