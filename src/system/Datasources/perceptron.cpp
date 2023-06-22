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

#include <Arduino.h>

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "perceptron.hpp"
#include "../../app/LogView.hpp"

// Constants
const double TRAINING_RATE = 0.2;
const int TRAINING_ITERATIONS = 16;

#ifdef __cplusplus
extern "C" {
#endif

// Definir el número de Euler
#define E 2.71828182845904523536

// Definir la función sigmoide
double sigmoid(double x) {
  return 1.0 / (1.0 + pow(E, -x));
}

// Definir la función sigmoide rápida
double fast_sigmoid(double x) {
  return x / (1.0 + fabs(x));
}

Perceptron *Perceptron_new(unsigned numOfInputs, double trainingRate) {
	unsigned i;
	static int initializedRandomization_ = 0;
	if ( numOfInputs < 1 ) {  numOfInputs = 1; }
	//size_t contiguousSize = sizeof(Perceptron)+(numOfInputs*sizeof(double));
	//Perceptron *perc = (Perceptron*)ps_malloc(contiguousSize); // data packed in only one malloc/free
	Perceptron *perc = (Perceptron*)ps_malloc(sizeof(Perceptron));
	perc->numInputs_ = numOfInputs;
	perc->trainingRate_ = trainingRate;
	perc->weights_ = (double*)ps_malloc(perc->numInputs_ * sizeof(double));
	//perc->weights_ = (double*)(perc+sizeof(Perceptron)); // packed data (only one free neded)
	if (! initializedRandomization_) {
		srand(time(NULL));
		initializedRandomization_ = 1;
	}	
	for (i = 0 ; i < perc->numInputs_ ; i++) {
		perc->weights_[i] = _Perceptron_getRandomDouble();
	}
	perc->threshold_ = _Perceptron_getRandomDouble();
	perc->trainedTimes = 0;
	return perc;	
}

double Perceptron_getValue(const Perceptron *perceptron, const double inputs[]) {
	unsigned i;
	double ans = 0;
	
	for (i = 0 ; i < perceptron->numInputs_ ; i++) {
		delay(1);
		ans += perceptron->weights_[i] * inputs[i];
	}	
	return ans;
}

void Perceptron_setTrainingRate(Perceptron *perceptron, double trainingRate) {
	perceptron->trainingRate_ = trainingRate;
}

void Perceptron_train(Perceptron *perceptron, const double inputs[], int expectedResult) {
	int result = Perceptron_getResult(perceptron, inputs);
	if (result == expectedResult) {
		//lLog("Perceptron: %p already trained\n",perceptron);
		return;
	}
	_Perceptron_changeWeights(perceptron, result, expectedResult, inputs);
	perceptron->trainedTimes++;
}

int Perceptron_getResult(const Perceptron *perceptron, const double inputs[]) {
	return (Perceptron_getValue(perceptron, inputs) >= perceptron->threshold_);
}

double Perceptron_getWeightAt(const Perceptron *perceptron, unsigned index) {
	return perceptron->weights_[index];
}

const double *Perceptron_getWeights(const Perceptron *perceptron) {
	return perceptron->weights_;
}

unsigned Perceptron_getNumOfInputs(const Perceptron *perceptron) {
	return perceptron->numInputs_;
}

double Perceptron_getThreshold(const Perceptron *perceptron) {
	return perceptron->threshold_;
}

double Perceptron_getTrainingRate(const Perceptron *perceptron) {
	return perceptron->trainingRate_;
}

void Perceptron_setWeightAt(Perceptron *perceptron, unsigned index, double weight) {
	perceptron->weights_[index] = weight;
}

void Perceptron_setWeights(Perceptron *perceptron, const double *weights) {
	unsigned i;
	for (i = 0 ; i < perceptron->numInputs_ ; i++) {
		delay(1);
		perceptron->weights_[i] = weights[i];
	}
}

void Perceptron_setThreshold(Perceptron *perceptron, double threshold) {
	perceptron->threshold_ = threshold;
}

void _Perceptron_changeWeights(Perceptron *perceptron, int actualResult, int desiredResult, const double inputs[]) {
	unsigned i;
	
	for (i = 0 ; i < perceptron->numInputs_ ; i++) {
		delay(1);
		perceptron->weights_[i] += perceptron->trainingRate_ * (desiredResult - actualResult) * inputs[i];
	}
	perceptron->threshold_ -= perceptron->trainingRate_ * (desiredResult - actualResult);
}

double _Perceptron_getRandomDouble() {
	double randValue = ((double)rand() / (double)RAND_MAX);
	double negativeRand = ((double)rand() / (double)RAND_MAX);
	if (negativeRand < 0.5) {
		randValue *= -1.0;
	}
	return randValue;
}


#ifdef __cplusplus
}
#endif


// https://sourceforge.net/p/ccperceptron/wiki/Home/
int PerceptronTest() {
	const int NUM_OF_INPUTS = 2;

	const double ZERO_ZERO[] = {0, 0};
	const double ZERO_ONE[]  = {0, 1};
	const double ONE_ZERO[]  = {1, 0};
	const double ONE_ONE[]   = {1, 1};
	
	int i;
	// Creating Perceptron instances
	Perceptron *pAND = (Perceptron *)Perceptron_new(NUM_OF_INPUTS, TRAINING_RATE);
	Perceptron *pOR  = (Perceptron *)Perceptron_new(NUM_OF_INPUTS, TRAINING_RATE);

	// Printing the results of the randomly generated perceptrons (BEFORE TRAINING)
	lLog("Results for 'OR' perceptron, BEFORE training:\n");
	lLog("Input: (0,0). Result: %d\n", Perceptron_getResult(pOR, ZERO_ZERO));
	lLog("Input: (0,1). Result: %d\n", Perceptron_getResult(pOR, ZERO_ONE));
	lLog("Input: (1,0). Result: %d\n", Perceptron_getResult(pOR, ONE_ZERO));
	lLog("Input: (1,1). Result: %d\n", Perceptron_getResult(pOR, ONE_ONE));

	lLog("Results for 'AND' perceptron, BEFORE training:\n");
	lLog("Input: (0,0). Result: %d\n", Perceptron_getResult(pAND, ZERO_ZERO));
	lLog("Input: (0,1). Result: %d\n", Perceptron_getResult(pAND, ZERO_ONE));
	lLog("Input: (1,0). Result: %d\n", Perceptron_getResult(pAND, ONE_ZERO));
	lLog("Input: (1,1). Result: %d\n", Perceptron_getResult(pAND, ONE_ONE));
	
	// Training each of the perceptrons
	for (i = 0 ; i < TRAINING_ITERATIONS ; i++) {
		delay(80);
		Perceptron_train(pAND, ZERO_ZERO, 0);
		Perceptron_train(pAND, ZERO_ONE,  0);
		Perceptron_train(pAND, ONE_ZERO,  0);
		Perceptron_train(pAND, ONE_ONE,   1);
	
		Perceptron_train(pOR, ZERO_ZERO,  0);
		Perceptron_train(pOR, ZERO_ONE,   1);
		Perceptron_train(pOR, ONE_ZERO,   1);
		Perceptron_train(pOR, ONE_ONE,    1);
	}
	
	// Printing the results of the trained perceptrons (AFTER TRAINING)
	lLog("Results for 'OR' perceptron, AFTER training:\n");
	lLog("Input: (0,0). Result: %d\n", Perceptron_getResult(pOR, ZERO_ZERO));
	lLog("Input: (0,1). Result: %d\n", Perceptron_getResult(pOR, ZERO_ONE));
	lLog("Input: (1,0). Result: %d\n", Perceptron_getResult(pOR, ONE_ZERO));
	lLog("Input: (1,1). Result: %d\n", Perceptron_getResult(pOR, ONE_ONE));
		
	lLog("Results for 'AND' perceptron, AFTER training:\n");
	lLog("Input: (0,0). Result: %d\n", Perceptron_getResult(pAND, ZERO_ZERO));
	lLog("Input: (0,1). Result: %d\n", Perceptron_getResult(pAND, ZERO_ONE));
	lLog("Input: (1,0). Result: %d\n", Perceptron_getResult(pAND, ONE_ZERO));
	lLog("Input: (1,1). Result: %d\n", Perceptron_getResult(pAND, ONE_ONE));
	
	free(pAND);
	free(pOR);

	return 0;
}

char *PerceptronSnapshoot(Perceptron *perceptron, size_t &size) {
	/*
	unsigned numInputs_;
	double *weights_;
	double threshold_;
	double trainingRate_;
	*/
	size_t sizeWeights=sizeof(double)*perceptron->numInputs_;
	lLog("Perceptron: Allocating %u bytes\n",sizeWeights);
	char * buffer=(char*)ps_malloc(sizeWeights);
	double *bufferPtr;
	bufferPtr=(double *)buffer;
	size=sizeWeights;
	for(size_t off=0;off<perceptron->numInputs_;off++) {
		//lLog("Weight off: %u Value: %lf\n", off, perceptron->weights_[off]);
		*bufferPtr=perceptron->weights_[off];
		bufferPtr++;
	}
	return buffer;
}