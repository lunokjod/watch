//
//    LunokWatch, a open source smartwatch software
//    Copyright (C) 2022,2023  Jordi Rubió <jordi@binarycell.org>
//    This file is part of LunokWatch.
//
// LunokWatch is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software 
// Foundation, either version 3 of the License, or (at your option) any later 
// version.
//
// LunokWatch is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
// details.
//
// You should have received a copy of the GNU General Public License along with 
// LunokWatch. If not, see <https://www.gnu.org/licenses/>. 
//

#ifndef PERCEPTRON_H
#define PERCEPTRON_H


#include <Arduino.h>

#include <stdlib.h>
#include <time.h>
#include <stdio.h>



#ifdef __cplusplus
extern "C" {
#endif


typedef struct Perceptron {
	unsigned numInputs_;
	double *weights_;
	double threshold_;
	double trainingRate_;
	uint32_t trainedTimes;
} Perceptron;

// Public
char *PerceptronSnapshoot(Perceptron *perceptron, size_t &size);
Perceptron *Perceptron_new(unsigned numOfInputs, double trainingRate);
double Perceptron_getValue(const Perceptron *perceptron, const double inputs[]);
void Perceptron_setTrainingRate(Perceptron *perceptron, double trainingRate);
void Perceptron_train(Perceptron *perceptron, const double inputs[], int expectedResult);
int Perceptron_getResult(const Perceptron *perceptron, const double inputs[]);
double Perceptron_getWeightAt(const Perceptron *perceptron, unsigned index);
const double *Perceptron_getWeights(const Perceptron *perceptron);
unsigned Perceptron_getNumOfInputs(const Perceptron *perceptron);
double Perceptron_getThreshold(const Perceptron *perceptron);
double Perceptron_getTrainingRate(const Perceptron *perceptron);
void Perceptron_setWeightAt(Perceptron *perceptron, unsigned index, double weight);
void Perceptron_setWeights(Perceptron *perceptron, const double *weights);
void Perceptron_setThreshold(Perceptron *perceptron, double threshold);

// Private
double _Perceptron_getRandomDouble();
void _Perceptron_changeWeights(Perceptron *perceptron, int actualResult, int desiredResult, const double inputs[]);


#ifdef __cplusplus
}
#endif
int PerceptronTest();

#endif

