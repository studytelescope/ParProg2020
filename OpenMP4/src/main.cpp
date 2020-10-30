#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <omp.h>
#include <cmath>

using namespace std;

double calc(uint32_t x_last, uint32_t num_threads)
{
	double global_result  		= 1;
	double current_coefficient 	= 1;

	vector<double> coefficients(num_threads);
	vector<double> partial_sums(num_threads);

	uint32_t range_size = x_last / num_threads;

	if (x_last % num_threads != 0)
		range_size++;

	x_last--;

	#pragma omp parallel num_threads(num_threads)
	{
		double result		= 0;
		double thread_coeff = 1;

		uint32_t thread_id = omp_get_thread_num();

		uint32_t first_element = range_size * thread_id + 1;
		uint32_t last_element  = range_size * thread_id + range_size;

		if (last_element > x_last)
			last_element = x_last;

		for (uint32_t i = first_element; i <= last_element; i++)
		{
			thread_coeff /= i;
			result += thread_coeff;
		}

		partial_sums[thread_id] = result;
		coefficients[thread_id] = thread_coeff;
	}

	for (uint32_t i = 0; i < num_threads; i++) {
		global_result  		+= current_coefficient * partial_sums[i];
		current_coefficient *= coefficients[i];
	}

	return global_result;
}

int main(int argc, char** argv)
{
  // Check arguments
  if (argc != 3)
  {
	cout << "[Error] Usage <inputfile> <output file>\n";
	return 1;
  }

  // Prepare input file
  ifstream input(argv[1]);
  if (!input.is_open())
  {
	cout << "[Error] Can't open " << argv[1] << " for write\n";
	return 1;
  }

  // Prepare output file
  ofstream output(argv[2]);
  if (!output.is_open())
  {
	cout << "[Error] Can't open " << argv[2] << " for read\n";
	input.close();
	return 1;
  }

// Read arguments from input
  int x_last = 0, num_threads = 0;
  input >> x_last >> num_threads;

  // Calculation
  double result = calc(x_last, num_threads);

  // Write resultult
  output << setprecision(16) << result << endl;
  // Prepare to exit
  output.close();
  input.close();
  return 0;
}