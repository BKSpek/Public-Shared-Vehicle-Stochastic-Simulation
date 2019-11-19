/*
	Output after 100 trials:
	...
	Total Time Spent with no bikes during trial 29.2989
	Total Time Spent with no bikes during trial 21.3961
	Total Time Spent with no bikes during trial 36.3279
	Average amount of money over 100 iterations : 365.07
	Average time spent with no bikes over 100 iterations : 28.7345
	Average cost of dissatisfaction over 100 iterations : -93.3872 +-3.46096

	Output after 10000 trials:
	...
	Total Time Spent with no bikes during trial 30.2822
	Total Time Spent with no bikes during trial 28.4952
	Total Time Spent with no bikes during trial 29.666
	Average amount of money over 10000 iterations : 361.833
	Average time spent with no bikes over 10000 iterations : 29.0333
	Average cost of dissatisfaction over 10000 iterations : -94.3581 +-0.299109
*/

#include <iostream>
#include <random>
#include <queue>
#include <time.h>
#include <set>

double calculateSampleVariance(std::vector<double> & costValues, double globalMean)
{
	double sampleVariance = 0;
	for (auto cost : costValues)
	{
		sampleVariance += pow(cost - globalMean, 2);
	}

	sampleVariance = sampleVariance / (costValues.size() - 1);

	sampleVariance = sqrt(sampleVariance);

	return sampleVariance;
}

int main()
{
	const int T = 120;
	int X[121] = { 0 }; //There are T+1 events
	const double bikeArrivalRate = 6;
	//clients have rate r1 = 3, r2 = 1, r3 = 4
	const double clientRates[4] = { 0, 3.0, 1.0, 4.0 };

	//client class 1/2 pay annually (K1 = 0.5, k2 = .1), total amount is (K1*r1 + K2*r2)
	//class 3 pays per ride amount k3 = 1.25

	//when annual members (class 1/2) arrive at empty station, there is penalty c1 = 1.0, c2 = 0.25, c3 = 0
	const double clientPenalty[4] = { 0, -1.0, -0.25, 0 };

	//create and seed the generator
	std::default_random_engine generator;
	generator.seed(time(0));

	//aggregate poisson
	std::cout << "Aggregate Lambda is : " << bikeArrivalRate + clientRates[1] + clientRates[2] + clientRates[3] << std::endl;
	std::poisson_distribution<int> poissonRandomVariableGenerator(bikeArrivalRate + clientRates[1] + clientRates[2] + clientRates[3]);

	/*  std::discrete_distribution produces random integers on the interval [0, n),
		where the probability of each individual integer i is defined as the weight of
		the ith integer divided by the sum of all n weights. */
	std::discrete_distribution<> weightedDistributionEventGenerator({ bikeArrivalRate, clientRates[1], clientRates[2], clientRates[3] });
	std::uniform_real_distribution<double> uniformRealGenerator(0, 1);

	const int numberOfTrials = 10000;
	double averageMoneyAmount = 0;
	double averageCostOfDissatisfaction = 0;
	double averageTimeWithNoBikes = 0;
	unsigned long numberOfEvents = 0;

	std::vector<double> costValues;

	std::cout << "Starting the trials" << std::endl;

	for (int t = 0; t < numberOfTrials; t++)
	{
		//we can assume total money starts at 0 + the deterministic annual prorated charge of clients classes 1 and 2
		double totalMoney = (0.5 * clientRates[1]) + (0.1 * clientRates[2]);
		X[0] = 10; //we start with 10 bikes at X(0)
		double timeSpentWithNoBikes = 0;
		double startOfNoBikes = -1;

		//for every X[i] to X[T]
		for (int i = 1; i <= T; i++)
		{
			X[i] = X[i - 1]; //new time interval starts with bike amount from prev interval
			int generatedValue = poissonRandomVariableGenerator(generator);
			numberOfEvents += generatedValue;
			//std::cout << "Generated p.r.v : " << generatedValue << std::endl;

			//generate the times of the events
			std::vector<double> eventTimes;
			for (int e = 0; e < generatedValue; e++)
				eventTimes.push_back(i + uniformRealGenerator(generator));

			//sort them in ascending order
			std::sort(eventTimes.begin(), eventTimes.end());

			//classify each event + handle it
			for (int rEvent = 0; rEvent < generatedValue; rEvent++)
			{
				//generate a u.r.v. {0: Bike Arrival, 1: Class1, 2: Class2, 3: Class3)
				int eventType = weightedDistributionEventGenerator(generator);

				if (eventType == 0) //a bike has arrived
				{
					X[i]++; //increment bike amount

					//if was in state where no bikes, end the interval and record the delta
					if (startOfNoBikes != -1)
					{
						timeSpentWithNoBikes += eventTimes[rEvent] - startOfNoBikes; //record the time interval with no bikes
						startOfNoBikes = -1; //set to -1 to indicate that we are not in a state of no bikes
					}
				}
				else  //a client has arrived
				{
					//if no more bikes, add to queue, else decrement bike count
					if (X[i] > 0)
					{
						//if eventType 3 gain $1.25
						if (eventType == 3)
						{
							totalMoney += 1.25;
						}
						//decrement the bike count
						X[i]--;

						//if no more bikes start the timer
						if (X[i] == 0)
						{
							startOfNoBikes = eventTimes[rEvent];
						}
					}
					else
					{
						//we apply a penalty for waiting in line, for class3 penalty is 0
						totalMoney += clientPenalty[eventType];
					}
				}
			}
		}

		//std::cout << "Total Money at the end of experiment " << totalMoney << std::endl;
		averageMoneyAmount += totalMoney;

		//record the cost for sample variance calculation
		costValues.push_back((timeSpentWithNoBikes * clientRates[1] * clientPenalty[1]) + (timeSpentWithNoBikes * clientRates[2] * clientPenalty[2]));

		//record for global mean calculation [further down]
		std::cout << "Total Time Spent with no bikes during trial " << timeSpentWithNoBikes << std::endl;
		averageTimeWithNoBikes += timeSpentWithNoBikes;
	}

	std::cout << "Average amount of money over " << numberOfTrials << " iterations" << " : "
		<< (averageMoneyAmount / numberOfTrials) << std::endl;

	/*std::cout << "Average amount of events over " << numberOfTrials << " iterations" << " : "
		<< (numberOfEvents / numberOfTrials) << std::endl;*/

	std::cout << "Average time spent with no bikes over " << numberOfTrials << " iterations" << " : "
		<< (averageTimeWithNoBikes / numberOfTrials) << std::endl;
	
	double globalMeanOfCost = (((averageTimeWithNoBikes / numberOfTrials) * clientRates[1] * clientPenalty[1]) + ((averageTimeWithNoBikes / numberOfTrials) * clientRates[2] * clientPenalty[2]));

	double sampleVariance = calculateSampleVariance(costValues, globalMeanOfCost);

	double z = 1.96;

	double CI = z * (sampleVariance / sqrt(costValues.size()));
	
	std::cout << "Average cost of dissatisfaction over " << numberOfTrials << " iterations" << " : "
		<< globalMeanOfCost << " +-" << CI << std::endl;

	std::getchar();

	return 0;
}