/*
	Output after 100 trials:
	...
	Total Time Spent with no bikes during trial 30.389
	Total Time Spent with no bikes during trial 26.5569
	Total Time Spent with no bikes during trial 28.264
	Average amount of money over 100 iterations : 360.05
	Average time spent with no bikes over 100 iterations : 28.8555
	Average cost of dissatisfaction over 100 iterations : -93.7804 +-2.9717

	Output after 10000 trials:
	...
	Total Time Spent with no bikes during trial 35.6788
	Total Time Spent with no bikes during trial 28.8811
	Total Time Spent with no bikes during trial 33.8833
	Average amount of money over 10000 iterations : 361.017
	Average time spent with no bikes over 10000 iterations : 29.0693
	Average cost of dissatisfaction over 10000 iterations : -94.4752 +-0.296064
*/

#include <iostream>
#include <random>
#include <map>
#include <time.h>

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

	//poisson
	std::exponential_distribution<double> bikeClock(bikeArrivalRate);
	std::exponential_distribution<double> type1Clock(clientRates[1]);
	std::exponential_distribution<double> type2Clock(clientRates[2]);
	std::exponential_distribution<double> type3Clock(clientRates[3]);

	const int numberOfTrials = 100;
	double averageMoneyAmount = 0;
	unsigned long numberOfEvents = 0;

	double averageCostOfDissatisfaction = 0;
	double averageTimeWithNoBikes = 0;

	std::vector<double> costValues;

	std::cout << "Starting the trials" << std::endl;

	for (int t = 0; t < numberOfTrials; t++)
	{
		//we can assume total money starts at 0 + the deterministic annual prorated charge of clients classes 1 and 2
		double totalMoney = (0.5 * clientRates[1]) + (0.1 * clientRates[2]);
		int bikeCount = 10; //we start with 10 bikes at X(0)

		double timeSpentWithNoBikes = 0;
		double startOfNoBikes = -1;

		//events
		std::map<double, int> events; //holds arrival time, type

		//generate first set of events
		events.insert(std::make_pair(bikeClock(generator), 0));
		events.insert(std::make_pair(type1Clock(generator), 1));
		events.insert(std::make_pair(type2Clock(generator), 2));
		events.insert(std::make_pair(type3Clock(generator), 3));

		//while the next event is <= T
		while (events.begin()->first <= T)
		{
			//{0: Bike Arrival, 1: Class1, 2: Class2, 3: Class3)
			int eventType = events.begin()->second;
			double eventTime = events.begin()->first;

			//consume the event
			events.erase(events.begin());

			//generate the next event
			if (eventType == 0) events.insert(std::make_pair(eventTime + bikeClock(generator), 0));
			else if (eventType == 1) events.insert(std::make_pair(eventTime + type1Clock(generator), 1));
			else if (eventType == 2) events.insert(std::make_pair(eventTime + type2Clock(generator), 2));
			else if (eventType == 3) events.insert(std::make_pair(eventTime + type3Clock(generator), 3));

			//handle the current event
			if (eventType == 0) //a bike has arrived
			{
				bikeCount++; //increment bike amount
			
				//if was in state where no bikes, end the interval and record the delta
				if (startOfNoBikes != -1)
				{
					timeSpentWithNoBikes += eventTime - startOfNoBikes; //record the time interval with no bikes
					startOfNoBikes = -1; //set to -1 to indicate that we are not in a state of no bikes
				}
			}
			else //a client has arrived
			{
				//if no more bikes, add to queue, else decrement bike count
				if (bikeCount > 0)
				{
					//if eventType 3 gain $1.25
					if (eventType == 3)
					{
						totalMoney += 1.25;
					}
					//decrement the bike count
					bikeCount--;

					//if no more bikes start the timer
					if (bikeCount == 0)
					{
						startOfNoBikes = eventTime;
					}
				}
				else
				{
					//we apply a penalty for waiting in line, for class3 penalty is 0
					totalMoney += clientPenalty[eventType];
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