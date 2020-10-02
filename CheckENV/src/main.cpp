#include <iostream>
#include <iomanip>
#include <fstream>
#include <omp.h>


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
  uint32_t num_threads = 0;
  input >> num_threads;
  #pragma omp parallel num_threads(num_threads)
  {
    #pragma omp critical
    output << "TEST" << std::endl;
  }
  // Prepare to exit
  output.close();
  input.close();
  return 0;
}
