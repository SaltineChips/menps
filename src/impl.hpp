
#pragma once

#include <mgcom.hpp>
#include <mgbase/stdint.hpp>

namespace mgcom {

inline void notify(const notifier_t& notif) MGBASE_NOEXCEPT {
    switch (notif.op) {
        case MGCOM_LOCAL_ASSIGN_INT64:
            *static_cast<mgbase::int64_t*>(notif.pointer) = static_cast<mgbase::int64_t>(notif.value);
            break;
        case MGCOM_LOCAL_FAA_INT64:
            // FIXME
            break;
    }
}

}

