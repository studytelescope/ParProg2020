#include <iostream>
#include <iomanip>
#include <fstream>
#include <mpi.h>
#include <unistd.h>
#include <cmath>

void calc(double* arr, uint32_t ySize, uint32_t xSize, int rank, int size)
{
	MPI_Bcast(&xSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&ySize, 1, MPI_INT, 0, MPI_COMM_WORLD);

	uint32_t start_point = 0, end_point = 0;
	uint32_t block_size = ySize / size + 1;

	if (rank == 0 && size > 0)
	{
		for (int i = 1; i < size; i++)
		{
			start_point = i * block_size;
			end_point = start_point + block_size + 1;

			if (end_point > ySize) end_point = ySize;
			if (start_point > ySize) start_point = ySize;

			MPI_Send(&(arr[start_point * xSize]), xSize * (end_point - start_point), MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
		}

		start_point = 0;
		end_point = block_size + 1 < ySize ? block_size + 1 : ySize;
		if(block_size < ySize)
			end_point = block_size + 1;
		else
			end_point = ySize;
	}
	else
	{
		start_point = rank * block_size;
		end_point = start_point + block_size + 1;

		if (end_point > ySize) end_point = ySize;
		if (start_point > ySize) start_point = ySize;

		arr = (double*) malloc (sizeof(double) * xSize * (end_point - start_point));
		MPI_Status status;
		MPI_Recv(arr, xSize * (end_point - start_point), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
	}

	for (int i = 0; i < (int)(end_point - start_point - 1); i++)
		for (int j = 3; j < (int)xSize; j++)	
			arr[i * xSize + j] = sin(0.00001 * arr[(i + 1) * xSize + j - 3]);
		 
	if (rank == 0 && size > 0)
	{
		for (int i = 1; i < size; i++)
		{
			MPI_Status status;

			start_point = i * block_size;
			end_point = start_point + block_size;

			if (end_point > ySize) end_point = ySize;
			if (start_point > ySize) start_point = ySize;

			MPI_Recv (&(arr[start_point * xSize]), xSize * (end_point - start_point), MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
		}
	}
	else
	{
		if(end_point != start_point)
			MPI_Send(arr, xSize * (end_point - start_point - 1), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		else
			MPI_Send(arr, 0, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);

		free(arr);
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
	}
	else
	{
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
