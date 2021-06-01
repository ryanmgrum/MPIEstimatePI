/* Author: Ryan McAllister-Grum
 * Assignment: HW-03 (Option-B)
 * Class: CSC484A Intro to Parallel Processing
 *
 * Description: The Estimate-Pi program uses a Monte Carlo (meaning random)
 * method to estimate the value of Pi by simulating a dart board and expecting
 * a normal distribution of darts that land inside an inner circle versus
 * on the outer dart board square. The total amount inside the circle divided
 * by the total number of tosses, multiplied by 4, should be approximately pi.
 *
 * For this program, we need to first read in the number of tosses from
 * the user using process 0; after determining that the number of tosses is
 * greater than 0, distribute the number of tosses to all the processes using MPI_Bcast.
 *
 * After the above comes the main for-loop that all the processes will use to
 * compute their tosses using the provided for-loop in the Project Description
 * to randomly calculate their number of darts that land in the circle.
 *
 * After the above for-loop, the total will be printed out by process 0 by first
 * using MPI_Reduce to retrieve the total number of darts that landed in each
 * process's circle, and then calculating and printing the final result. Finally,
 * call MPI_Finalize and exit.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int main() {
	// number_of_tosses holds the total number of tosses entered in by the user.
	long long int number_of_tosses;
	
	// Seed the random number generator to get different results each time.
    srand(time(NULL)); 
	
	// Initialize MPI.
	int comm_sz, my_rank;
	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	// Fetch the user input.
	if (my_rank == 0) {
		printf("Please enter the total number of tosses:\n");
		scanf("%lli", &number_of_tosses);
	}
	
	/* Wait until all processes reach this point
	 * and the value is ready to broadcast before
	 * continuing.
	 */
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Bcast(&number_of_tosses, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
	
	// Check whether the user entered an invalid value.
	if (number_of_tosses < 1) {
		if (my_rank == 0)
			fprintf(stderr, "Error: The number of tosses input is less than 1.\n");
		MPI_Finalize();
		return 0;
	}
	
	// Now, find the number of tosses that land in the circle for each process.
	long long int number_in_circle = 0;
	long long int my_total_tosses = number_of_tosses / comm_sz +
		// Spreads the remainder out over the processes whose rank is smaller than the remainder.
		(my_rank <= (number_of_tosses - (number_of_tosses / comm_sz) * comm_sz) - 1 ? 1 : 0);
	double x, y, distance_squared;
	long long int toss;
	for (toss = 0; toss < my_total_tosses; toss++) {
		/* Generate a random number between -1 and 1.
		 * Formula for generating a random number within the range found at the following reference,
		 * in particular i486's post (direct URL is https://stackoverflow.com/a/33389635).
		 *
		 * References:
		 *   “Random Number Generator between -1 and 1 in C++,” October 28, 2015. https://stackoverflow.com/questions/33389553/random-number-generator-between-1-and-1-in-c.
		 */
		x = ((double) rand() / RAND_MAX) * 2 - 1;
		y = ((double) rand() / RAND_MAX) * 2 - 1;
		distance_squared = x*x + y*y;
		if (distance_squared <= 1)
			number_in_circle++;
	}
	
	// Aggregate the number_in_circle from each process.
	long long int total_number_in_circle = 0;
	MPI_Reduce(&number_in_circle, &total_number_in_circle, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	
	// Output the result.
	if (my_rank == 0)
		printf("PI estimate: %g\n", 4*total_number_in_circle/((double) number_of_tosses));
	
	// Clean up MPI and return.
	MPI_Finalize();
	return 0;
}