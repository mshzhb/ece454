rm mm.c
wget https://dl.dropboxusercontent.com/u/19920404/ece454_2/ece454/hw3/assn3-malloc/assn/mm.c
make clean
make
./mdriver -f ../traces/amptjp-bal.rep
