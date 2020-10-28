#include <iostream>
#include <iomanip>
#include <fstream>
#include <omp.h>
#include <cmath>

double func(double x)
{
  return sin(x);
}

const int token_number = 100;

double calc(double x0, double x1, double dx, uint32_t num_threads)
{
  double sum    = 0;
  double* token = new double[token_number];

  int num_steps   = (x1 - x0) / dx;
  int token_size  = num_steps / token_number;

  #pragma omp parallel num_threads(num_threads)
  {
    #pragma omp for
    for(int i = 1; i < token_size * token_number; ++i)
    {
      int token_idx = i / token_size;
      token[token_idx] += (func(x0 + i * dx));
    }
  }

  for(int i(token_size * token_number); i < num_steps; ++i) sum += (func(x0 + i * dx));
  for(int i(0); i < token_number; ++i) sum += token[i];

  sum *= dx;

  delete token;

  return sum;
}

int main(int argc, char** argv)
{
  // Check arguments
  if (argc != 3)
  {
    std::cout << "[Error] Usage <inputfile> <output file>\n";
    return 1;
  }

  // Prepare input file
  std::ifstream input(argv[1]);
  if (!input.is_open())
  {
    std::cout << "[Error] Can't open " << argv[1] << " for write\n";
    return 1;
  }

  // Prepare output file
  std::ofstream output(argv[2]);
  if (!output.is_open())
  {
    std::cout << "[Error] Can't open " << argv[2] << " for read\n";
    input.close();
    return 1;
  }

  // Read arguments from input
  double x0 = 0.0, x1 =0.0, dx = 0.0;
  uint32_t num_threads = 0;
  input >> x0 >> x1 >> dx >> num_threads;

  // Calculation
  double res = calc(x0, x1, dx, num_threads);

  // Write result
  output << std::setprecision(13) << res << std::endl;
  // Prepare to exit
  output.close();
  input.close();
  return 0;
}