//
//                           SimuLTE
// Copyright (C) 2015 Giovanni Nardini
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "stack/mac/scheduler/LcgSchedulerRealistic.h"
#include "stack/mac/buffer/LteMacBuffer.h"

LcgSchedulerRealistic::LcgSchedulerRealistic(LteMacUe* mac)
    :LcgScheduler(mac)
{
}

LcgSchedulerRealistic::~LcgSchedulerRealistic()
{
    // TODO Auto-generated destructor stub
}

ScheduleList&
LcgSchedulerRealistic::schedule(unsigned int availableBytes, Direction grantDir)
{
    /* clean up old schedule decisions
     for each cid, this map will store the the amount of sent data (in SDUs)
     */
    scheduleList_.clear();

    /*
     * Clean up scheduling support status map
     */
    statusMap_.clear();

    // amount of time passed since last scheduling operation
    simtime_t timeInterval = NOW - lastExecutionTime_;

    // these variables will contain the flow identifier and the priority of the lowest
    // priority flow encountered during the scheduling process
    int lowestBackloggedFlow = -1;
    int lowestBackloggedPriority = -1;

    // these variables will contain the flow identifier and the priority of the highest
    // priority flow encountered during the scheduling process
    int highestBackloggedFlow = -1;
    int highestBackloggedPriority = -1;

    // Considering 3GPP TS 36.321:
    //  - ConnDesc::parameters_.minReservedRate_    --> Prioritized Bit Rate (PBR) in Bytes/s
    //  - ConnDesc::parameters_.maximumBucketSizeurst_         --> Maximum Bucket size (MBS) = PBR*BSD product
    //  - ConnDesc::parameters_.bucket_         --> is the B parameter in Bytes

    // If true, assure a minimum reserved rate to all connection (LCP first
    // phase), if false, provide a best effort service (LCP second phase)
    bool priorityService = true;

    LcgMap& lcgMap = mac_->getLcgMap();

    if (lcgMap.empty())
        return scheduleList_;

    // for all traffic classes
    for (unsigned short i = 0; i < UNKNOWN_TRAFFIC_TYPE; ++i)
    {
        // Prepare the iterators to cycle the entire scheduling set
        std::pair<LcgMap::iterator, LcgMap::iterator> it_pair;
        it_pair = lcgMap.equal_range((LteTrafficClass) i);
        LcgMap::iterator it = it_pair.first, et = it_pair.second;

        EV << NOW << " LcgSchedulerRealistic::schedule - Node  " << mac_->getMacNodeId() << ", Starting priority service for traffic class " << i << endl;

        //! FIXME Allocation of the same resource to flows with same priority not implemented - not suitable with relays
        for (; it != et; ++it)
        {
            // processing all connections of same traffic class

            // get the connection virtual buffer
            LteMacBuffer* vQueue = it->second.second;

            // get the buffer size
            unsigned int queueLength = vQueue->getQueueOccupancy(); // in bytes

            // connection id of the processed connection
            MacCid cid = it->second.first;

            // get the Flow descriptor
            LteControlInfo connDesc = mac_->getConnDesc().at(cid);
            // TODO get the QoS parameters

//            IPHACK: no matter how I set the controlInfo in upper layer message sending, the connection direction is UL but not D2D_MULTI, strange
//            IPHACK: I temporarily ignore this check to let package scheduling go through, so package can be deliver down to PHY
//            IPHACK: roll back since this problem is solved by changing the RLC package direction setting to D2D_MULTI
            // connection must have the same direction of the grant
            if (connDesc.getDirection() != grantDir){
                EV << " LcgSchedulerRealistic::schedule - connection direction: " << dirToA((Direction)connDesc.getDirection()) << "\tgrant direction: " << dirToA(grantDir) << endl;
                continue;
            }

            unsigned int toServe = queueLength;
            // Check whether the virtual buffer is empty
            if (queueLength == 0)
            {
                EV << "LcgSchedulerRealistic::schedule scheduled connection is no more active " << endl;
                continue; // go to next connection
            }
            else
            {
                // we need to consider also the size of RLC and MAC headers
                if (connDesc.getRlcType() == UM)
                    toServe += RLC_HEADER_UM;
                else if (connDesc.getRlcType() == AM)
                    toServe += RLC_HEADER_AM;
                toServe += MAC_HEADER;
            }

            // get a pointer to the appropriate status element: we need a tracing element
            // in order to store information about connections and data transmitted. These
            // information may be consulted at the end of the LCP algorithm
            StatusElem* elem;

            if (statusMap_.find(cid) == statusMap_.end())
            {
                // the element does not exist, initialize it
                elem = &statusMap_[cid];
                elem->occupancy_ = vQueue->getQueueLength();
                elem->sentData_ = 0;
                elem->sentSdus_ = 0;
                // TODO set bucket from QoS parameters
                elem->bucket_ = 1000;
            }
            else
            {
                elem = &statusMap_[cid];
            }

            EV << NOW << " LcgSchedulerRealistic::schedule Node " << mac_->getMacNodeId() << " , Parameters:" << endl;
            EV << "\t Logical Channel ID: " << MacCidToLcid(cid) << endl;
            EV << "\t CID: " << cid << endl;
//                fprintf(stderr, "\tGroup ID: %d\n", desc->parameters_.groupId_);
//                fprintf(stderr, "\tPriority: %d\n", desc->parameters_.priority_);
//                fprintf(stderr, "\tMin Reserved Rate: %.0lf bytes/s\n", desc->parameters_.minReservedRate_);
//                fprintf(stderr, "\tMax Burst: %.0lf bytes\n", desc->parameters_.maxBurst_);

            if (priorityService)
            {
                // Update bucket value for this connection

                // get the actual bucket value and the configured max size
                double bucket = elem->bucket_; // TODO parameters -> bucket ;
                double maximumBucketSize = 10000.0; // TODO  parameters -> maxBurst;

                EV << NOW << " LcgSchedulerRealistic::schedule Bucket size: " << bucket << " bytes (max size " << maximumBucketSize << " bytes) - BEFORE SERVICE " << endl;

                // if the connection started before last scheduling event , use the
                // global time interval
                if (lastExecutionTime_ > 0)
                { // TODO desc->parameters_.startTime_) {
                    // PBR*(n*TTI) where n is the number of TTI from last update
                    bucket += /* TODO desc->parameters_.minReservedRate_*/ 100.0 * TTI;
                }
                // otherwise, set the bucket value accordingly to the start time
                else
                {
                    simtime_t localTimeInterval = NOW - 0/* TODO desc->parameters_.startTime_ */;
                    if (localTimeInterval < 0)
                        localTimeInterval = 0;

                    bucket = /* TODO desc->parameters_.minReservedRate_*/ 100.0 * localTimeInterval.dbl();
                }

                // do not overflow the maximum bucket size
                if (bucket > maximumBucketSize)
                    bucket = maximumBucketSize;

                // update connection's bucket
// TODO                desc->parameters_.bucket_ = bucket;

                // update the tracing element accordingly
                elem->bucket_ = 100.0; // TODO desc->parameters_.bucket_;
                EV << NOW << " LcgSchedulerRealistic::schedule Bucket size: " << bucket << " bytes (max size " << maximumBucketSize << " bytes) - AFTER SERVICE " << endl;
            }

            EV << NOW << " LcgSchedulerRealistic::schedule - Node " << mac_->getMacNodeId() << ", remaining grant: " << availableBytes << " bytes " << endl;
            EV << NOW << " LcgSchedulerRealistic::schedule - Node " << mac_->getMacNodeId() << " buffer Size: " << toServe << " bytes " << endl;


            // If priority service: (availableBytes>0) && (desc->buffer_.occupancy() > 0) && (desc->parameters_.bucket_ > 0)
            // If best effort service: (availableBytes>0) && (desc->buffer_.occupancy() > 0)
            if ((availableBytes > 0) && (toServe > 0)
                && (!priorityService || 1 /*TODO (desc->parameters_.bucket_ > 0)*/))
            {
                // Check if it is possible to serve the sdu, depending on the constraint
                // of the type of service
                // Priority service:
                //    ( sdu->size() <= availableBytes) && ( sdu->size() <= desc->parameters_.bucket_)
                // Best Effort service:
                //    ( sdu->size() <= availableBytes) && (!priorityService_)

                if ((toServe <= availableBytes) /*&& ( !priorityService || ( sduSize <= 0/*TODO desc->parameters_.bucket_) )*/)
                {
                    // remove SDU from virtual buffer
                    vQueue->popFront();

                    if (priorityService)
                    {
//    TODO                        desc->parameters_.bucket_ -= sduSize;
//                        // update the tracing element accordingly
//    TODO                    elem->bucket_ = 100.0 /* TODO desc->parameters_.bucket_*/;
                    }


                    // update the tracing element
                    elem->occupancy_ = vQueue->getQueueOccupancy();
                    elem->sentData_ += toServe;

                    // check if there is space for a SDU
                    int alloc = toServe;
                    alloc -= MAC_HEADER;
                    if (connDesc.getRlcType() == UM)
                        alloc -= RLC_HEADER_UM;
                    else if (connDesc.getRlcType() == AM)
                        alloc -= RLC_HEADER_AM;

                    if (alloc > 0)
                        elem->sentSdus_++;

                    availableBytes -= toServe;

                    while (!vQueue->isEmpty())
                    {
                        // remove SDUs from virtual buffer
                        vQueue->popFront();
                    }

                    toServe = 0;

                    EV << NOW << " LcgSchedulerRealistic::schedule - Node " << mac_->getMacNodeId() << ",  SDU of size " << elem->sentData_ << " selected for transmission" << endl;
                    EV << NOW << " LcgSchedulerRealistic::schedule - Node " << mac_->getMacNodeId() << ", remaining grant: " << availableBytes << " bytes" << endl;
                    EV << NOW << " LcgSchedulerRealistic::schedule - Node " << mac_->getMacNodeId() << " buffer Size: " << toServe << " bytes" << endl;
                }
                else
                {

                    if (priorityService)
                    {
//    TODO                        desc->parameters_.bucket_ -= sduSize;
//                        // update the tracing element accordingly
//    TODO                    elem->bucket_ = 100.0 /* TODO desc->parameters_.bucket_*/;
                    }


                    // update the tracing element
                    elem->occupancy_ = vQueue->getQueueOccupancy();
                    elem->sentData_ += availableBytes;

                    int alloc = availableBytes;
                    alloc -= MAC_HEADER;
                    if (connDesc.getRlcType() == UM)
                        alloc -= RLC_HEADER_UM;
                    else if (connDesc.getRlcType() == AM)
                        alloc -= RLC_HEADER_AM;

                    // check if there is space for a SDU
                    if (alloc > 0)
                        elem->sentSdus_++;

                    // update buffer
                    while (alloc > 0)
                    {
                        // update pkt info
                        PacketInfo newPktInfo = vQueue->popFront();
                        if (newPktInfo.first > alloc)
                        {
                            newPktInfo.first = newPktInfo.first - alloc;
                            vQueue->pushFront(newPktInfo);
                            alloc = 0;
                        }
                        else
                        {
                            alloc -= newPktInfo.first;
                        }

                    }

                    toServe -= availableBytes;
                    availableBytes = 0;

                    EV << NOW << " LcgSchedulerRealistic::schedule - Node " << mac_->getMacNodeId() << ",  SDU of size " << elem->sentData_ << " selected for transmission" << endl;
                    EV << NOW << " LcgSchedulerRealistic::schedule - Node " << mac_->getMacNodeId() << ", remaining grant: " << availableBytes << " bytes" << endl;
                    EV << NOW << " LcgSchedulerRealistic::schedule - Node " << mac_->getMacNodeId() << " buffer Size: " << toServe << " bytes" << endl;
                }
            }

            // check if flow is still backlogged
            if (availableBytes > 0)
            {
                // TODO the priority is higher when the associated integer is lower ( e.g. priority 2 is
                // greater than 4 )
//
//                if ( desc->parameters_.priority_ >= lowestBackloggedPriority_ ) {
//
//                    if(LteDebug::trace("LcgSchedulerRealistic::schedule"))
//                        fprintf(stderr,"%.9f LcgSchedulerRealistic::schedule - Node %d, this flow priority: %u (old lowest priority %u) - LOWEST FOR NOW\n", NOW, nodeId_, desc->parameters_.priority_, lowestBackloggedPriority_);
//
//                    // store the new lowest backlogged flow and its priority
//                    lowestBackloggedFlow_ = fid;
//                    lowestBackloggedPriority_ = desc->parameters_.priority_;
            }
//
//
//                if ( highestBackloggedPriority_ == -1 || desc->parameters_.priority_ <= highestBackloggedPriority_ ) {
//
//                    if(LteDebug::trace("LcgSchedulerRealistic::schedule"))
//                        fprintf(stderr,"%.9f LcgSchedulerRealistic::schedule - Node %d, this flow priority: %u (old highest priority %u) - HIGHEST FOR NOW\n", NOW, nodeId_, desc->parameters_.priority_, highestBackloggedPriority_);
//
//                    // store the new highest backlogged flow and its priority
//                    highestBackloggedFlow_ = fid;
//                    highestBackloggedPriority_ = desc->parameters_.priority_;
//                }
//
//            }
//
            // update the last schedule time
            lastExecutionTime_ = NOW;

            // signal service for current connection
            unsigned int* servicedSdu = NULL;

            if (scheduleList_.find(cid) == scheduleList_.end())
            {
                // the element does not exist, initialize it
                servicedSdu = &scheduleList_[cid];
                *servicedSdu = elem->sentSdus_;
            }
            else
            {
                // connection already scheduled during this TTI
                servicedSdu = &scheduleList_.at(cid);
            }

            // If the end of the connections map is reached and we were on priority and on last traffic class
            if (priorityService && (it == et) && ((i + 1) == (unsigned short) UNKNOWN_TRAFFIC_TYPE))
            {
                // the first phase of the LCP algorithm has completed ...
                // ... switch to best effort allocation!
                priorityService = false;
                //  reset traffic class
                i = 0;
                EV << "LcgSchedulerRealistic::schedule - Node" << mac_->getMacNodeId() << ", Starting best effort service" << endl;
            }
        } // END of connections cycle
    } // END of Traffic Classes cycle

    return scheduleList_;
}
