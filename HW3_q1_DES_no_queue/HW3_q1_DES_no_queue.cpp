/*
	A)
	Output after 100 runs:

	not actuall DES atm, poisson generator for number events over period
	exponential generator with some rate = 1 / lambda or something like that (distance between events)
	...
	Total Money at the end of experiment 365.85
	Total Money at the end of experiment 384.1
	Total Money at the end of experiment 375.35
	Average amount of money over 10000 iterations : 361.763
*/

#include <iostream>
#include <random>
#include <map>
#include <time.h>

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

	const int numberOfTrials = 10000;
	double averageMoneyAmount = 0;
	unsigned long numberOfEvents = 0;

	std::cout << "Starting the trials" << std::endl;

	for (int t = 0; t < numberOfTrials; t++)
	{
		//we can assume total money starts at 0 + the deterministic annual prorated charge of clients classes 1 and 2
		double totalMoney = (0.5 * clientRates[1]) + (0.1 * clientRates[2]);
		int bikeCount = 10; //we start with 10 bikes at X(0)

		//events
		std::map<double, int> events; //holds arrival time, type

		//generate first set of events
		events.insert(std::make_pair(bikeClock(generator),0));
		events.insert(std::make_pair(type1Clock(generator),1));
		events.insert(std::make_pair(type2Clock(generator),2));
		events.insert(std::make_pair(type3Clock(generator),3));

		//while the next event is <= T
		while (events.begin()->first <= T)
		{
			//{0: Bike Arrival, 1: Class1, 2: Class2, 3: Class3)
			int eventType = events.begin()->second;
			double eventTime = events.begin()->first;

			//consume the event
			events.erase(events.begin());

			//generate the next event
			if (eventType == 0) events.insert(std::make_pair(eventTime + bikeClock(generator),0));
			else if (eventType == 1) events.insert(std::make_pair(eventTime + type1Clock(generator),1));
			else if (eventType == 2) events.insert(std::make_pair(eventTime + type2Clock(generator),2));
			else if (eventType == 3) events.insert(std::make_pair(eventTime + type3Clock(generator),3));

			//handle the current event
			if (eventType == 0) //a bike has arrived
			{
				bikeCount++; //increment bike amount
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
				}
				else
				{
					//we apply a penalty for waiting in line, for class3 penalty is 0
					totalMoney += clientPenalty[eventType];
				}
			}
		}

		std::cout << "Total Money at the end of experiment " << totalMoney << std::endl;
		averageMoneyAmount += totalMoney;
	}

	std::cout << "Average amount of money over " << numberOfTrials << " iterations" << " : "
		<< (averageMoneyAmount / numberOfTrials) << std::endl;

	std::cout << "Average amount of events over " << numberOfTrials << " iterations" << " : "
		<< (numberOfEvents / numberOfTrials) << std::endl;

	std::getchar();

	return 0;
}