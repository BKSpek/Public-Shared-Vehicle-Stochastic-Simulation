/*
	A)
	Output after 100 runs:

	not actuall DES atm, poisson generator for number events over period
	exponential generator with some rate = 1 / lambda or something like that (distance between events)
	...
	Total Money at the end of experiment 313.85
	Total Money at the end of experiment 292.85
	Total Money at the end of experiment 294.6
	Average amount of money over 10000 iterations : 285.073
	Average amount of events over 10000 iterations : 1315
*/

#include <iostream>
#include <random>
#include <map>
#include <time.h>

struct Client
{
	int type;
};

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

	//poisson
	std::poisson_distribution<int> bikeClock(bikeArrivalRate);
	std::poisson_distribution<int> type1Clock(clientRates[1]);
	std::poisson_distribution<int> type2Clock(clientRates[2]);
	std::poisson_distribution<int> type3Clock(clientRates[3]);

	//uniform generator (0-1], used to determine time of event
	std::uniform_real_distribution<double> uniformNumberGenerator(0.0, 1.0);

	const int numberOfTrials = 10000;
	double averageMoneyAmount = 0;
	unsigned long numberOfEvents = 0;

	std::cout << "Starting the trials" << std::endl;

	for (int t = 0; t < numberOfTrials; t++)
	{
		//we can assume total money starts at 0 + the deterministic annual prorated charge of clients classes 1 and 2
		double totalMoney = (0.5 * clientRates[1]) + (0.1 * clientRates[2]);
		X[0] = 10; //we start with 10 bikes at X(0)

		//for every X[i] to X[T]
		for (int i = 1; i <= T; i++)
		{
			X[i] = X[i - 1]; //new time interval starts with bike amount from prev interval

			std::map<double, int> events; //holds arrival time, type

			//generate number of bike arrivals and then assign them event time via uniform distr.
			for (int j = 0; j < bikeClock(generator); j++)
			{
				events.insert(std::make_pair(uniformNumberGenerator(generator), 0));
			}

			//generate number of type1 arrivals and then assign them event time via uniform distr.
			for (int j = 0; j < type1Clock(generator); j++)
			{
				events.insert(std::make_pair(uniformNumberGenerator(generator), 1));
			}

			//generate number of type2 arrivals and then assign them event time via uniform distr.
			for (int j = 0; j < type2Clock(generator); j++)
			{
				events.insert(std::make_pair(uniformNumberGenerator(generator), 2));
			}

			//generate number of type3 arrivals and then assign them event time via uniform distr.
			for (int j = 0; j < type3Clock(generator); j++)
			{
				events.insert(std::make_pair(uniformNumberGenerator(generator), 3));
			}
			
			numberOfEvents += events.size();
			//std::cout << "Generated p.r.v : " << generatedValue << std::endl;
			for (auto & e : events)
			{
				//{0: Bike Arrival, 1: Class1, 2: Class2, 3: Class3)
				int eventType = e.second;

				//std::cout << "Event : " << eventType << std::endl;

				if (eventType == 0) //a bike has arrived
				{
					X[i]++; //increment bike amount
				}
				else //a client has arrived
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
					}
					else
					{
						//we apply a penalty for waiting in line, for class3 penalty is 0
						totalMoney += clientPenalty[eventType];
					}
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