#ifndef PERCEPTRON_H
#define PERCEPTRON_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Perceptron {
	unsigned numInputs_;
	double *weights_;
	double threshold_;
	double trainingRate_;
} Perceptron;

// Public
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

#endif

