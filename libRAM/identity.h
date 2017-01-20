
/* 
 * File:   identity.h
 * Author: aafshar
 *
 * Created on January 14, 2017, 5:23 PM
 */

#ifndef IDENTITY_H
#define IDENTITY_H


class Identity
{
public:
    std::string mName;
    int mComputationId;
    int mbBucketId;
    int circuitOffset;
    
    Identity()
    :
    mName("NA"),
    mComputationId(-1),
    mbBucketId(-1),
    circuitOffset(-1)
    {
        
    }
    
    Identity(std::string name, int computationId, int bucketId)
    :
    mName(name),
    mComputationId(computationId),
    mbBucketId(bucketId),
    circuitOffset(-1)
    {
        
    }
};

#endif /* IDENTITY_H */

