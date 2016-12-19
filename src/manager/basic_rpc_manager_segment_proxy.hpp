
#pragma once

#include <mgbase/lang.hpp>
#include <vector>

namespace mgdsm {

template <typename Policy>
class basic_rpc_manager_segment_proxy
    : public Policy::interface_type
    , public mgbase::crtp_base<Policy>
{
    typedef typename Policy::derived_type   derived_type;
    typedef typename Policy::interface_type interface_type;
    
    typedef typename Policy::page_type      page_type;
    
    typedef typename Policy::space_type     space_type;
    
    typedef typename Policy::page_id_type   page_id_type;
    typedef typename Policy::segment_id_type segment_id_type;
    
    typedef typename interface_type::acquire_read_result    acquire_read_result_type;
    typedef typename interface_type::acquire_write_result   acquire_write_result_type;
    
    typedef typename Policy::handler_id_type    handler_id_type;
    
public:
    // local operations
    
    virtual mgbase::size_t get_page_size() const
    {
        auto& self = this->derived();
        
        auto sg_ac = self.get_accessor();
        
        return sg_ac.get_page_size();
    }
    
    virtual mgbase::size_t get_num_pages() const
    {
        auto& self = this->derived();
        
        auto sg_ac = self.get_accessor();
        
        return sg_ac.get_num_pages();
    }
    
public:
    // RPC operations
    
    static void register_handlers()
    {
        Policy::template register_handler<acquire_read_handler>();
        Policy::template register_handler<release_read_handler>();
        Policy::template register_handler<acquire_write_handler>();
        Policy::template register_handler<release_write_handler>();
    }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual acquire_read_result_type acquire_read(const page_id_type pg_id) MGBASE_OVERRIDE
    {
        auto& self = this->derived();
        auto& sp = self.get_space_proxy();
        
        const auto seg_id = self.get_segment_id();
        
        const auto man_proc = sp.get_manager_proc(seg_id);
        auto* man_sp = sp.get_manager_space(seg_id);
        
        const argument arg{ man_sp, seg_id, pg_id };
        
        const auto ret_msg =
            Policy::template call<acquire_read_handler>(
                man_proc
            ,   arg
            );
        
        return *ret_msg;
    }
    
    virtual void release_read(const page_id_type pg_id) MGBASE_OVERRIDE
    {
        auto& self = this->derived();
        auto& sp = self.get_space_proxy();
        
        const auto seg_id = self.get_segment_id();
        
        const auto man_proc = sp.get_manager_proc(seg_id);
        auto* man_sp = sp.get_manager_space(seg_id);
        
        const argument arg{ man_sp, seg_id, pg_id };
        
        Policy::template call<release_read_handler>(
            man_proc
        ,   arg
        );
    }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual acquire_write_result_type acquire_write(const page_id_type pg_id) MGBASE_OVERRIDE
    {
        auto& self = this->derived();
        auto& sp = self.get_space_proxy();
        
        const auto seg_id = self.get_segment_id();
        
        const auto man_proc = sp.get_manager_proc(seg_id);
        auto* man_sp = sp.get_manager_space(seg_id);
        
        const argument arg{ man_sp, seg_id, pg_id };
        
        const auto ret_msg =
            Policy::template call<acquire_write_handler>(
                man_proc
            ,   arg
            );
        
        return *ret_msg;
    }
    
    virtual void release_write(const page_id_type pg_id) MGBASE_OVERRIDE
    {
        auto& self = this->derived();
        auto& sp = self.get_space_proxy();
        
        const auto seg_id = self.get_segment_id();
        
        const auto man_proc = sp.get_manager_proc(seg_id);
        auto* man_sp = sp.get_manager_space(seg_id);
        
        const argument arg{ man_sp, seg_id, pg_id };
        
        Policy::template call<release_write_handler>(
            man_proc
        ,   arg
        );
    }
    
private:
    struct argument {
        space_type*         sp;
        segment_id_type     seg_id;
        page_id_type        pg_id;
    };
    
    struct acquire_read_handler
    {
        static const handler_id_type handler_id = Policy::acquire_read_handler_id;
        
        typedef argument                    request_type;
        typedef acquire_read_result_type    reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc)
        {
            auto& rqst = sc.request();
            
            auto& sp = *rqst.sp;
            const auto seg_id = rqst.seg_id;
            const auto pg_id = rqst.pg_id;
            
            const auto src_proc = sc.src_proc();
            
            // Look up the segment.
            auto sp_ac = sp.get_segment_accessor(seg_id);
            
            // Look up the page.
            auto pg_ac = sp_ac.get_page_accessor(pg_id);
            
            auto rep = sc.make_reply();
            
            auto ret = pg_ac.acquire_read(src_proc);
            
            // Call acquire_read() on the manager process.
            *rep = acquire_read_result_type{ ret.owner_plptr, ret.needs_flush };
            
            return rep;
        }
    };
    
    struct release_read_handler
    {
        static const handler_id_type handler_id = Policy::release_read_handler_id;
        
        typedef argument    request_type;
        typedef void        reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc)
        {
            auto& rqst = sc.request();
            
            auto& sp = *rqst.sp;
            const auto seg_id = rqst.seg_id;
            const auto pg_id = rqst.pg_id;
            
            const auto src_proc = sc.src_proc();
            
            // Look up the segment.
            auto sp_ac = sp.get_segment_accessor(seg_id);
            
            // Look up the page.
            auto pg_ac = sp_ac.get_page_accessor(pg_id);
            
            pg_ac.release_read(src_proc);
            
            return sc.make_reply();
        }
    };
    
    struct acquire_write_handler
    {
        static const handler_id_type handler_id = Policy::acquire_write_handler_id;
        
        typedef argument                    request_type;
        typedef acquire_write_result_type   reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc)
        {
            auto& rqst = sc.request();
            
            auto& sp = *rqst.sp;
            const auto seg_id = rqst.seg_id;
            const auto pg_id = rqst.pg_id;
            
            const auto src_proc = sc.src_proc();
            
            // Look up the segment.
            auto sp_ac = sp.get_segment_accessor(seg_id);
            
            // Look up the page.
            auto pg_ac = sp_ac.get_page_accessor(pg_id);
            
            auto rep = sc.make_reply();
            
            auto ret = pg_ac.acquire_write(src_proc);
            
            ret.inv.send_to_all(sp_ac, pg_id);
            
            // Call acquire_write() on the manager process.
            *rep = acquire_write_result_type{ ret.owner_plptr, ret.needs_diff };
            
            return rep;
        }
    };
    
    struct release_write_handler
    {
        static const handler_id_type handler_id = Policy::release_write_handler_id;
        
        typedef argument    request_type;
        typedef void        reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc)
        {
            auto& rqst = sc.request();
            
            auto& sp = *rqst.sp;
            const auto seg_id = rqst.seg_id;
            const auto pg_id = rqst.pg_id;
            
            const auto src_proc = sc.src_proc();
            
            // Look up the segment.
            auto sp_ac = sp.get_segment_accessor(seg_id);
            
            // Look up the page.
            auto pg_ac = sp_ac.get_page_accessor(pg_id);
            
            pg_ac.release_write(src_proc);
            
            return sc.make_reply();
        }
    };
};

} // namespace mgdsm

