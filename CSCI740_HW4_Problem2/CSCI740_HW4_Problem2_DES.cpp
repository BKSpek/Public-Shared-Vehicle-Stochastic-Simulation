/***
Program to calculate confidence interval of M/GI/1 queue with predetermined error tolerance
***/

#include <iostream>
#include <iomanip>
#include <random>
#include <map>
#include <time.h>
#include <vector>
#include <numeric>
#include <math.h>

int main()
{
	//Parameters for Batch Mean Method
	std::vector<double> batchAverages;    //vector to store mean values for each batch
	double batchSize = 50;                //batch size
	int batchQueueSize;                   //queue size for each batch
	int batchNumber;                      //number to keep track of current batch run
	int warmupPeriods = 5;                //number of batches "ignored"
	int totalIterationCount;              //total number of runs
	double sampleVariance;                //sample variance relative to batched trials
	double globalMean;                    //variable to keep track of global mean of trials
	double confidenceIntervalZ = 1.96;    //z-value for alpha = 0.05
	double precision = 0.01;              //desired precision
	int numTrials = 1;                    //number of independent trials
	double upperConfidenceBound;          //upper bound of confidence interval
	double lowerConfidenceBound;          //lower bound of confidence interval
	double nextPossibleServiceTime;       //time when the next client can be seen
	std::map<double, double> line;        //holds arrival time, servicetime, sorted


	//Initiate generators for distributions
	const double arrivalRate = 1;
	std::default_random_engine generator;
	generator.seed(time(0));
	std::exponential_distribution<double> arrivalClock(arrivalRate);

	std::gamma_distribution<double> serviceTimesGenerator(3, 0.25);
	std::poisson_distribution<int> arrivalGenerator(1);
	std::uniform_real_distribution<double> uniformDistributionGenerator(0.0, 1.0);

	for (int trial = 0; trial < numTrials; trial++)
	{
		//Initialize/reset values for new trial run
		totalIterationCount = 0;
		globalMean = 0;
		batchAverages.clear();
		sampleVariance = 0;
		line.clear();
		nextPossibleServiceTime = 0; 
		batchNumber = -1;

		

		while (true) {
			//initialize batch queue size
			batchQueueSize = 0;
			++batchNumber;
			for (int i = 0; i < batchSize; ++i) {

				//generate arrival & service time
				line.insert(std::make_pair(arrivalClock(generator), 0));
				double arrivalTime = line.begin()->first;
				double serviceTime = line.begin()->second;

				//Process arrival
				line.erase(line.begin());

				//generate the next event
				line.insert(std::make_pair(arrivalTime + arrivalClock(generator), 0));

				//Calculating batch size and updating total number of runs
				batchQueueSize += line.size();
				totalIterationCount++;
			};

			//Insert batch average into vector and calculate global mean up to current iteration
			batchAverages.push_back(batchQueueSize/batchSize);
			globalMean = (std::accumulate(batchAverages.begin(), batchAverages.end(), 0.0))/batchAverages.size();

			//Calculating sum of differences squared for sample vs global mean
			std::vector<double>::iterator ptr;
			double sumOfDeltaSquare = 0; //stores sum of differences quared between batch averages and global average.
			for (ptr = batchAverages.begin(); ptr < batchAverages.end(); ++ptr) {
				sumOfDeltaSquare += pow((*ptr - globalMean), 2);
			};

			//Calculate sample variance and upper/lower bounds for confidence intervals
			sampleVariance = pow(((1 / ((double)totalIterationCount - 1))*sumOfDeltaSquare), 0.5);
			upperConfidenceBound = globalMean + (confidenceIntervalZ*sampleVariance*(1 / (pow(totalIterationCount, 0.5))));
			lowerConfidenceBound = globalMean - (confidenceIntervalZ*sampleVariance*(1 / (pow(totalIterationCount, 0.5))));

			//Only check exit condition if batch run is after warmup period
			if (batchNumber >= warmupPeriods) {
				//If confidence interval is within precision, exit.
				if ((upperConfidenceBound - lowerConfidenceBound) <= precision) {
					break;
				};
			};
		};
	};

	std::cout << "Overall Average Queue Length : " << std::setprecision(3) << std::fixed << globalMean << std::endl;
	std::cout << "Total Number of Runs : " << totalIterationCount << std::endl;
	system("pause");
	return 0;
};