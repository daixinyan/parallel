#include<mpi.h>
#include<stdio.h>
int main(int argc ,char * argv[])
{



	MPI_Request req[2];
	MPI_Request sec_req[2];

	int s1,r1;

	MPI_Status sta[2];
  MPI_Status sec_sta[2];

	int nprocs,rank;
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);

	s1=5;

	if(rank==0){
		//MPI_Send(&s1,1,MPI_DOUBLE,2,1,MPI_COMM_WORLD);
		MPI_Isend(&s1,1,MPI_DOUBLE,2,1,MPI_COMM_WORLD,req);
		MPI_Isend(&s1,1,MPI_DOUBLE,2,1,MPI_COMM_WORLD,sec_req);
		MPI_Wait(req,sta);
		MPI_Wait(sec_req,sec_sta);
	}

	if(rank==2){
		//MPI_Recv(&r1,1,MPI_DOUBLE,0,1,MPI_COMM_WORLD,sta);
		MPI_Irecv(&r1,1,MPI_DOUBLE,0,1,MPI_COMM_WORLD,&req[1]);
		MPI_Wait(&req[1],&sta[1]);

		MPI_Irecv(&r1,1,MPI_DOUBLE,0,1,MPI_COMM_WORLD,&sec_req[1]);
		MPI_Wait(&sec_req[1],&sec_sta[1]);
 		printf("%d\n",r1);
	}
	printf("rank %d process is running\n",rank);
	MPI_Finalize();




}
