
#pragma once

#include <mgcom/common.hpp>
#include <mgcom/rpc/client.hpp>
#include <mgcom/rpc/server.hpp>
#include <mgbase/callback.hpp>

namespace mgcom {
namespace rpc {

class requester
    : public client
    , public server
{
public:
    static requester& get_instance() MGBASE_NOEXCEPT {
        MGBASE_ASSERT(req_ != MGBASE_NULLPTR);
        return *req_;
    }
    
    static void set_instance(requester& req) {
        req_ = &req;
    }
    
private:
    static requester* req_;
};

} // namespace rpc
} // namespace mgcom

