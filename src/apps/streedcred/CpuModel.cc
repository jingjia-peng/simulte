/*
 * CpuModel.cpp
 *
 *  Created on: May 6, 2024
 *      Author: veins
 */
#include "CpuModel.h"
#include <vector>
#include <random>
#include <iostream>

CpuModel::CpuModel(int numCores) {
    init(numCores);
}

CpuModel::CpuModel() {
    CpuModel(1);
}

void CpuModel::init(int numCores) {
    this->numCores = numCores;
    coreLoads = std::vector<double>(numCores);
}

double CpuModel::randomGaussian(double mean, double stddev) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> d(mean, stddev);
    double sample = d(gen);
    return sample;
}

// return a pair of CPU queuing latency and computation latency
std::pair<double,double> CpuModel::getLatency(double currentTime, double mean, double std) {
    int selectCoreId = -1;
    double earliestTime = 1000000.0;
    for (int i = 0; i < coreLoads.size(); i++) {
        if (coreLoads[i] < currentTime) coreLoads[i] = currentTime;
        if (coreLoads[i] < earliestTime) {
            earliestTime = coreLoads[i];
            selectCoreId = i;
        }
    }

    double queueLatency = coreLoads[selectCoreId] - currentTime;
    // avoid negative delay
    double computeLatency;
    do {
        computeLatency = randomGaussian(mean, std);
    } while (computeLatency <= 0);
    coreLoads[selectCoreId] = coreLoads[selectCoreId] + computeLatency;
    return std::make_pair(queueLatency,computeLatency);
}

//double CpuModel::getLatency(double currentTime, double mean, double std) {
//    int selectCoreId = -1;
//    double earliestTime = 1000000.0;
//    for (int i = 0; i < coreLoads.size(); i++) {
//        if (coreLoads[i] < currentTime) coreLoads[i] = currentTime;
//        if (coreLoads[i] < earliestTime) {
//            earliestTime = coreLoads[i];
//            selectCoreId = i;
//        }
//    }
//    double computeLatency = randomGaussian(mean, std);
//    double queueLatency = coreLoads[selectCoreId] - currentTime;
//    coreLoads[selectCoreId] = coreLoads[selectCoreId] + computeLatency;
//    // coreLoads[selectCoreId] - currentTime = queueLatency + computeLatency
////    std::cout << "[CPU] queueLatency = "<< queueLatency << " computeLatency = "<< computeLatency << std::endl;
//    return coreLoads[selectCoreId] - currentTime;
//}


