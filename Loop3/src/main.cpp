#include <iostream>
#include <iomanip>
#include <fstream>
#include <mpi.h>
#include <unistd.h>
#include <cmath>

void reversed_calc(double* arr, uint32_t ySize, uint32_t xSize, int rank, int size)
{
	MPI_Bcast(&xSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&ySize, 1, MPI_INT, 0, MPI_COMM_WORLD);
	uint32_t block_size = 0, start = 0, end = 0;

	block_size = ySize / size + 1;

	if (rank == 0 && size > 0)
	{
		for (int i = 1; i < size; i++)
		{
			start = i * block_size;
			end = start + block_size;

			if (end > ySize) end = ySize;
			if (start > ySize) start = ySize;
			
			MPI_Send (&(arr[start * xSize]), xSize * (end - start), MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
		}

		start = 0;
		end = block_size < ySize ? block_size : ySize;
	}
	else
	{
	    start = rank * block_size;
	    end = start + block_size;

	    if (end > ySize) end = ySize;
	    if (start > ySize) start = ySize;

	    arr = (double*) malloc (sizeof(double) * xSize * (end - start));

	    MPI_Status status;
	    MPI_Recv(arr, xSize * (end - start), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
	}

	for (uint32_t i = 0; i < end - start; i++)
		for (uint32_t j = 4; j < xSize; j++)
			arr[i * xSize + j] =  sin(arr[i * xSize + j - 4]);

	if (rank == 0 && size > 0)
	{
		for (int i = 1; i < size; i++)
		{
			start = i * block_size;
			end = start + block_size;

			if (end > ySize) end = ySize;
			if (start > ySize) start = ySize;

			MPI_Status status;
			MPI_Recv (&(arr[start * xSize]), xSize * (end - start), MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
		}
	}
	else
	{
		MPI_Send(arr, xSize * (end - start), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		free(arr);
	}
}

void calc(double* arr, uint32_t ySize, uint32_t xSize, int rank, int size)
{
	double* arr_t;

	if (rank == 0 && size > 0)
	{
		arr_t = (double*) malloc (sizeof(double) * xSize * ySize);

		for (uint32_t i = 0; i < ySize; i++)
		for (uint32_t j = 0; j < xSize; j++)
		arr_t[j * ySize + i] = arr[i * xSize + j];
	}

	  reversed_calc(arr_t, xSize, ySize, rank, size);

	if (rank == 0 && size > 0)
	{
		for (uint32_t i = 0; i < ySize; i++)
			for (uint32_t j = 0; j < xSize; j++)
				arr[i * xSize + j] = arr_t[j * ySize + i];

		free(arr_t);
	}
}

int main(int argc, char** argv)
{
  int rank = 0, size = 0, buf = 0;
  uint32_t ySize = 0, xSize = 0;
  double* arr = 0;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0)
  {
    // Check arguments
    if (argc != 3)
    {
      std::cout << "[Error] Usage <inputfile> <output file>\n";
      buf = 1;
      MPI_Bcast(&buf, 1, MPI_INT, 0, MPI_COMM_WORLD);
      return 1;
    }

    // Prepare input file
    std::ifstream input(argv[1]);
    if (!input.is_open())
    {
      std::cout << "[Error] Can't open " << argv[1] << " for write\n";
      buf = 1;
      MPI_Bcast(&buf, 1, MPI_INT, 0, MPI_COMM_WORLD);
      return 1;
    }

    // Read arguments from input
    input >> ySize >> xSize;
    MPI_Bcast(&buf, 1, MPI_INT, 0, MPI_COMM_WORLD);

    arr = new double[ySize * xSize];

    for (uint32_t y = 0; y < ySize; y++)
    {
     for (uint32_t x = 0; x < xSize; x++)
      {
        input >> arr[y*xSize + x];
      }
    }
    input.close();
  } else {
    MPI_Bcast(&buf, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (buf != 0)
    {
      return 1;
    }
  }

  calc(arr, ySize, xSize, rank, size);

  if (rank == 0)
  {
    // Prepare output file
    std::ofstream output(argv[2]);
    if (!output.is_open())
    {
      std::cout << "[Error] Can't open " << argv[2] << " for read\n";
      delete arr;
      return 1;
    }
    for (uint32_t y = 0; y < ySize; y++)
    {
      for (uint32_t x = 0; x < xSize; x++)
      {
        output << " " << arr[y*xSize + x];
      }
      output << std::endl;
    }
    output.close();
    delete arr;
  }

  MPI_Finalize();
  return 0;
}
