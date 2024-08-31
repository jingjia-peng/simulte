/*
 * common.h
 *
 *  Created on: Apr 21, 2024
 *      Author: veins
 */

#ifndef COMMON_H_
#define COMMON_H_

// Enumeration types tracking the stage of protocol.
enum class CoinAssignmentStage {
    INIT,
    REQUESTED,
    SENT,
    FINISHED,
    FAILED
};

enum class CoinDepositStage {
    INIT,
    REQUESTED,
    SIGNATURE_REQUESTED,
    SIGNATURE_SENT,
    SUBMITTED,
    FINISHED,
    FAILED
};

#endif
