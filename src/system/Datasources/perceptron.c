#include <stdlib.h>
#include <time.h>

#include "perceptron.h"

#ifdef __cplusplus
extern "C" {
#endif

Perceptron *Perceptron_new(unsigned numOfInputs, double trainingRate) {
	unsigned i;
	static int initializedRandomization_ = 0;
	
	Perceptron *perc = (Perceptron*)malloc(sizeof(Perceptron));
	perc->numInputs_ = numOfInputs;
	if (perc->numInputs_ <= 0) {
		perc->numInputs_ = 1;
	}
	perc->trainingRate_ = trainingRate;
	perc->weights_ = (double*)malloc(perc->numInputs_ * sizeof(double));
	if (! initializedRandomization_) {
		srand(time(NULL));
		initializedRandomization_ = 1;
	}	
	for (i = 0 ; i < perc->numInputs_ ; i++) {
		perc->weights_[i] = _Perceptron_getRandomDouble();
	}
	perc->threshold_ = _Perceptron_getRandomDouble();
	
	return perc;	
}

double Perceptron_getValue(const Perceptron *perceptron, const double inputs[]) {
	unsigned i;
	double ans = 0;
	
	for (i = 0 ; i < perceptron->numInputs_ ; i++) {
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
		return;
	}
	_Perceptron_changeWeights(perceptron, result, expectedResult, inputs);
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
		perceptron->weights_[i] = weights[i];
	}
}

void Perceptron_setThreshold(Perceptron *perceptron, double threshold) {
	perceptron->threshold_ = threshold;
}

void _Perceptron_changeWeights(Perceptron *perceptron, int actualResult, int desiredResult, const double inputs[]) {
	unsigned i;
	
	for (i = 0 ; i < perceptron->numInputs_ ; i++) {
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

