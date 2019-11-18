/*
	Total time before complete infection : 933.907
	Total time before complete infection : 1969.86
	Total time before complete infection : 1031.89
	Total time before complete infection : 987.224
	ContactRate : 1 : InfectionRate : 0.5
	Average time over 10000 trials : 1037.55
*/

#include <iostream>
#include <vector>
#include <random>
#include <time.h>

bool checkAllInfected(std::vector<bool> & population)
{
	for (auto isInfected : population)
	{
		if (!isInfected)
		{
			return false;
		}
	}

	return true;
}

int main() 
{
	//create and seed the generator
	std::default_random_engine generator;
	generator.seed(time(0));

	//exponential generator
	double contactRate = 1, infectionRate = 0.5;
	std::exponential_distribution<double> exponentialGenerator(contactRate);
	std::uniform_int_distribution<int> uniformGenerator(0, 99);

	double totalTime = 0;

	int numTrials = 10000;
	for (int trial = 0; trial < numTrials; trial++)
	{
		std::vector<bool> population(100, false); // initialize population to healthy
		population[uniformGenerator(generator)] = true; // pick a random person to be infected as per the prompt

		double time = 0;

		// check the whole population if everybody is infected or not then repeat the simulation until everybody is
		while (!checkAllInfected(population))
		{
			// pick 2 people at random
			int person1 = uniformGenerator(generator);
			int person2 = -1;
			while (person2 < 0 && person2 != person1) { person2 = uniformGenerator(generator); } //make sure they are different people

			if (population[person1] != population[person2])
			{
				if (uniformGenerator(generator) < infectionRate * 100)
				{
					population[person1] = true;
					population[person2] = true;
				}
			}
			
			time += exponentialGenerator(generator);
		}

		totalTime += time;
		std::cout << "Total time before complete infection : " << time << std::endl;
	}

	std::cout << "ContactRate : " << contactRate << " : InfectionRate : " << infectionRate << std::endl;
	std::cout << "Average time over " << numTrials << " trials : " << (totalTime / numTrials) << std::endl;

	std::getchar();
	
	return 0;
}
