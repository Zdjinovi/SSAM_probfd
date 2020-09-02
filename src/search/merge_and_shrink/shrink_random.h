#ifndef MERGE_AND_SHRINK_SHRINK_RANDOM_H
#define MERGE_AND_SHRINK_SHRINK_RANDOM_H

#include "shrink_bucket_based.h"

namespace merge_and_shrink {

class ShrinkRandom : public ShrinkBucketBased
{
protected:
    virtual void partition_into_buckets(
        const Abstraction &abs, std::vector<Bucket> &buckets) const;

    virtual std::string name() const;
public:
    ShrinkRandom(const options::Options &opts);
    virtual ~ShrinkRandom();
};

}

#endif
