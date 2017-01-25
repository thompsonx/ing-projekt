BASE_HEADERS = tracelog.h event.h
BASE_CLASSES = tracelog.cpp event.cpp
MPI_HEADERS = mpitracelog.h
MPI_CLASSES = mpitracelog.cpp main_mpi.cpp
CFLAGS = -std=c++11

mpi: $(BASE_HEADERS) $(BASE_CLASSES) $(MPI_HEADERS) $(MPI_CLASSES)
	mpic++ -o mpi_sync $(CFLAGS) $(BASE_CLASSES) $(MPI_CLASSES)
