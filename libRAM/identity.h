
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
//    std::string mName;
    int mComputationId;
//    int mbBucketId;
    int circuitOffset;
    osuCrypto::Role mRole;
    
    Identity()
    :
//    mName("NA"),
    mComputationId(-1),
//    mbBucketId(-1),
    circuitOffset(-1)
    {
        
    }
    
    Identity(int computationId, int circuitId, osuCrypto::Role role)
    :
//    mName(name),
    mComputationId(computationId),
//    mbBucketId(-1),
    circuitOffset(circuitId),
    mRole(role)
    {
        
    }
    
    bool
    operator==(Identity &id)
    {
        return (id.mComputationId == mComputationId) && (id.circuitOffset == circuitOffset) && (id.mRole == mRole);
    }
};

#endif /* IDENTITY_H */

