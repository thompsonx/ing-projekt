BASE_HEADERS = tracelog.h event.h
BASE_CLASSES = tracelog.cpp event.cpp
MPI_HEADERS = mpitracelog.h
MPI_CLASSES = mpitracelog.cpp main_mpi.cpp
CFLAGS = -std=c++11

mpi: mpi_sync

mpi_sync: $(BASE_HEADERS) $(BASE_CLASSES) $(MPI_HEADERS) $(MPI_CLASSES)
	mpic++ -o $@ $(CFLAGS) $(BASE_CLASSES) $(MPI_CLASSES)
