
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/vector.hpp>
#include <queue>

namespace menps {
namespace medsm2 {

template <typename P>
class rd_set
{
    using blk_id_type = typename P::blk_id_type;
    using rd_ts_type = typename P::rd_ts_type;
    using wr_ts_type = typename P::wr_ts_type;
    
    using mutex_type = typename P::mutex_type;
    using unique_lock_type  = typename P::unique_lock_type;
    
    using size_type = typename P::size_type;
    
    using ult_itf_type = typename P::ult_itf_type;
    using spinlock_type = typename ult_itf_type::spinlock;
    
    struct entry {
        blk_id_type blk_id;
        rd_ts_type  rd_ts;
    };
    
    struct greater_ts {
        bool operator() (const entry& a, const entry& b) const noexcept {
            return P::is_greater_rd_ts(a.rd_ts, b.rd_ts);
        }
    };
    
    using pq_type =
        std::priority_queue<entry, mefdn::vector<entry>, greater_ts>;
    
public:
    void add_readable(const blk_id_type blk_id, const rd_ts_type rd_ts)
    {
        const unique_lock_type lk(this->mtx_);
        
        // Check that (min_wr_ts <= rd_ts).
        MEFDN_ASSERT(!P::is_greater_rd_ts(this->min_wr_ts_, rd_ts));
        
        // Add a new entry to the priority queue.
        this->pq_.push(entry{ blk_id, rd_ts });
    }
    
    template <typename Func>
    void self_invalidate(const rd_ts_type min_wr_ts, Func&& func)
    {
        mefdn::vector<blk_id_type> blk_ids;
        
        {
            const unique_lock_type lk(this->mtx_);
            
            const auto old_min_wr_ts = this->min_wr_ts_;
            if (!P::is_greater_rd_ts(min_wr_ts, old_min_wr_ts)) {
                // Because min_wr_ts <= old_min_wr_ts,
                // it is unnecessary to self-invalidate blocks for this signature.
                MEFDN_LOG_VERBOSE(
                    "msg:No need to self-invalidate.\t"
                    "old_min_wr_ts:{}\t"
                    "new_min_wr_ts:{}\t"
                ,   old_min_wr_ts
                ,   min_wr_ts
                );
                return;
            }
            
            // Update the minimum write timestamp.
            this->min_wr_ts_ = min_wr_ts;
            
            while (! this->pq_.empty()) {
                auto& e = this->pq_.top();
                if (!P::is_greater_rd_ts(min_wr_ts, e.rd_ts)) {
                    // e.rd_ts is still valid.
                    break;
                }
                
                // Add the block ID if the block is a candidate for self-invalidation.
                blk_ids.push_back(e.blk_id);
                
                this->pq_.pop();
            }
        }
        
        MEFDN_LOG_DEBUG(
            "msg:Start self-invalidating all old blocks.\t"
            "min_wr_ts:{}\t"
            "num_blk_ids:{}"
        ,   min_wr_ts
        ,   blk_ids.size()
        );
        
        mefdn::vector<entry> new_ents;
        spinlock_type new_ents_lock;
        
        //for (const auto& blk_id : blk_ids) {
        ult_itf_type::for_loop(
            ult_itf_type::execution::seq
            //ult_itf_type::execution::par
            // TODO: This can be parallelized, but tasks are fine-grained
        ,   0
        ,   blk_ids.size()
        ,   [&func, &blk_ids, &new_ents, &new_ents_lock, min_wr_ts] (const size_type i) {
                const auto blk_id = blk_ids[i];
                const auto ret = func(blk_id);
                if (!P::is_greater_rd_ts(min_wr_ts, ret.rd_ts)) {
                    mefdn::lock_guard<spinlock_type> lk(new_ents_lock);
                    new_ents.push_back(entry{ blk_id, ret.rd_ts });
                }
            }
        );
        
        if (!new_ents.empty()) {
            const unique_lock_type lk(this->mtx_);
            for (const auto& e : new_ents) {
                this->pq_.push(e);
            }
        }
        
        MEFDN_LOG_DEBUG(
            "msg:Finish self-invalidating all old blocks.\t"
            "min_wr_ts:{}\t"
            "num_old_blks:{}\t"
            "num_valid_blks:{}\t"
        ,   min_wr_ts
        ,   blk_ids.size()
        ,   new_ents.size()
        );
    }
    
    template <typename Func>
    void self_invalidate_all(Func&& func)
    {
        mefdn::vector<blk_id_type> blk_ids;
        
        {
            const unique_lock_type lk(this->mtx_);
            while (!this->pq_.empty()) {
                const auto& e = this->pq_.top();
                blk_ids.push_back(e.blk_id);
                this->pq_.pop();
            }
        }
        
        for (const auto& blk_id : blk_ids) {
            // TODO: Ignoring the returned value.
            func(blk_id);
        }
    }
    
    wr_ts_type make_new_wr_ts(const rd_ts_type old_rd_ts) const
    {
        const unique_lock_type lk(this->mtx_);
        return std::max(old_rd_ts + 1, this->min_wr_ts_);
    }
    
    rd_ts_type make_new_rd_ts(const wr_ts_type wr_ts, const rd_ts_type rd_ts)
    {
        const unique_lock_type lk(this->mtx_);
        // TODO: Provide a good prediction for a lease value of each block.
        return std::max(std::max(wr_ts, this->min_wr_ts_) + P::constants_type::lease_ts, rd_ts);
    }
    
    wr_ts_type get_min_wr_ts() const noexcept {
        const unique_lock_type lk(this->mtx_);
        return this->min_wr_ts_;
    }
    
    bool is_valid_rd_ts(const rd_ts_type rd_ts) const noexcept {
        const unique_lock_type lk(this->mtx_);
        // If rd_ts >= min_wr_ts, the block is still valid.
        return !P::is_greater_rd_ts(this->min_wr_ts_, rd_ts);
    }
    
private:
    mutable mutex_type  mtx_;
    pq_type             pq_;
    wr_ts_type          min_wr_ts_ = 0;
};

} // namespace medsm2
} // namespace menps

