# Final Project OS/Parallel | campwill & LandanJH

  # File Structure
```
./web_scraper
├── README.md
├── web_seq.c
├── web_omp.c
├── web_mpi.c
└── helper.sh
```

The ```helper.sh``` files are there to compile the programs

# Web Scraper

### Description
The web scraper is a program designed to gather information from websites such as email or phone numbers and collect them in a file for research purposes. The web_seq.c file is the sequential implementation of the program, the web_omp.c file is the OpenMP version of the program, and finally the web_mpi.c file is the MPI implementation.

### How to compile
Sequential Implementation
``` bash
gcc web_seq.c -o seq.bin -lcurl
```
MPI Implementation
``` bash
mpicc web_mpi.c -o mpi.bin -lcurl
```
OpenMP Implementation
``` bash
gcc web_omp.c -o omp.bin -lcurl -fopenmp
```
or
``` bash
chmod +x helper.sh
./helper.sh
```

### How to run

Note: you will need to give it the link to the full url with the path to the sitemap.xml eg. https://EXAMPLE.EXAMPLE/sitemap.xml

Sequential Implementation
``` bash
./seq.bin <url>
```
MPI Implementation
``` bash
mpirun -np 4 ./mpi.bin <url>
```
OpenMP Implementation
``` bash
./omp.bin <url>
```

### Notes
- There may be bugs this was a final project for a class that turned into a tool that I use every once in awhile
