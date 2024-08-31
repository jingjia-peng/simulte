/*
 * CpuModel.h
 *
 *  Created on: May 5, 2024
 *      Author: veins
 */

#ifndef SRC_VEINS_MODULES_APPLICATION_RCS_CPUMODEL_H_
#define SRC_VEINS_MODULES_APPLICATION_RCS_CPUMODEL_H_

#include <vector>


class CpuModel {
public:
    int numCores;
    std::vector<double> coreLoads;

    CpuModel();
    CpuModel(int numCores);
    void init(int numCores);
    double randomGaussian(double mean, double std);
    std::pair<double,double> getLatency(double currentTime, double mean, double std);
//    double getLatency(double currentTime, double mean, double std);
};


#endif /* SRC_VEINS_MODULES_APPLICATION_RCS_CPUMODEL_H_ */
