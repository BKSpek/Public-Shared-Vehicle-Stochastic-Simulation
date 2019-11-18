/*
	Two residual clocks: poisson arrival generator, gamma service time generator
	Per time unit, once we generate number of arrivals, we determine exact arrival time via uniform distr.
	Per time unit, we check to see if next in line is ready for service, if so service them
	We experimented with T (run time of experiment), and found that as T increases the average length of queue does as well
	We found the theta to be 0.5 meaning on average for every 2 arrivals 1 person is serviced

	1.2 average
*/

#include <iostream>
#include <random>
#include <map>
#include <time.h>

int main()
{
	//create and seed the generator
	std::default_random_engine generator;
	generator.seed(time(0));

	std::gamma_distribution<double> serviceTimesGenerator(3, 0.25);
	std::poisson_distribution<int> arrivalGenerator(1);

	//since the range is inclusive, semi arbitrary max value set to avoid using rejection method
	std::uniform_real_distribution<double> uniformDistributionGenerator(0.0, 1.0);

	int numTrials = 100;
	double overallAverageQueueLength = 0;
	
	for (int trial = 0; trial < numTrials; trial++)
	{
		int T = 20000; //time intervals
		std::map<double, double> line; //holds arrival time, servicetime, sorted

		double nextPossibleServiceTime = 0; //time when the next client can be seen

		double averageQueueLength = 0;

		//for every time unit
		for (int i = 0; i <= T; i++)
		{
			int arrivals = arrivalGenerator(generator);

			//for every arrival, use uniform distribution to determine exact time of arrival
			for (int j = 0; j < arrivals; j++)
			{
				double arrivalTime = i + uniformDistributionGenerator(generator);
				//generate a service time at the same time as the arrival time, and insert into ordered map
				line.insert(std::make_pair(arrivalTime, serviceTimesGenerator(generator)));
			}

			//try to service clients
			for (auto client = line.begin(); client != line.end();)
			{
				double delta = std::max(nextPossibleServiceTime - i, 0.0); //the elapsed time since start of X[i]
				//client is in queue earlier or at nextPossibleTime and next possible time is in current range
				if (nextPossibleServiceTime < i + 1)
				{
					//i + delta is to account for the case that the NPST is not a whole number
					//effectively i + delta = the current time when the next customer is serviced 
					nextPossibleServiceTime = (i + delta) + client->second;
					line.erase(client++);
				}
				else
				{
					break;
				}
			}

			averageQueueLength += line.size();
			//std::cout << "line size at time " << i << " : " << line.size() << std::endl;
		}

		//std::cout << "Average Queue Length : " << averageQueueLength / T << std::endl;
		overallAverageQueueLength += averageQueueLength / T;
	}
	
	std::cout << "Overall Average Queue Length : " << overallAverageQueueLength / numTrials << std::endl;

	system("pause");

	return 0;
}