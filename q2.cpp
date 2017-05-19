#include <iostream>
#include <omp.h>
#include <cassert>
#include <cstdlib>
#include <algorithm>

#include "merge.h"
#include "partition.h"

const int P = 2;
const int N = 16000;
const int T = 20; //number of testcases


using namespace std;

inline double timer(bool output=false)
{
  static double prev=0;
  double current = omp_get_wtime();
  double delta = current - prev;
  prev = current;
  if(output){
    cout<<"Time taken = "<<delta<<endl;
  }
  return delta;
}

bool verify(int *data, int N){
  for(int i=1;i<N;i++){
    if(data[i] < data[i-1])
      return false;
  }
  return true;
}

int main(int argc, char **argv){
  srand(0);//time(NULL));
  omp_set_nested(1);
  // assert(N % P == 0);
  cout<<"N = "<<N<<"\nP = "<<P<<endl;

  int *data,*test_data;

  test_data = new int[N];
  data = new int[N];
  omp_set_num_threads(P);

  double times[3]={0,0,0};

  for(int t=0;t<T;t++){
    #pragma omp parallel for
    for(int i = 0; i < N; ++i)
    {
      test_data[i] = (rand() % N) + 1; //N*float(rand())/RAND_MAX;
    }

    #pragma omp parallel for
    for(int i = 0; i < N; ++i)
    {
      data[i] = test_data[i];
    }
    timer();
    std::sort(data, data+N);
    times[0] += timer();
    // cout<<"result = "<<result<<" (SERIAL)"<<endl;

    #pragma omp parallel for
    for(int i = 0; i < N; ++i)
    {
      data[i] = test_data[i];
    }
    timer();
    merge_sort(data, N, P);
    times[1] += timer();
    assert(verify(data, N));
    // cout<<"result = "<<result<<" (CW)"<<endl;

    #pragma omp parallel for
    for(int i = 0; i < N; ++i)
    {
      data[i] = test_data[i];
    }
    timer();
    partition_sort(data, N, P);
    assert(verify(data, N));
    times[2] += timer();
    cerr<<".";
  }
  cout<<endl;
  cout<<"t = "<<times[0]/T<<" (SERIAL)"<<endl;
  cout<<"t = "<<times[1]/T<<" (MERGE)"<<endl;
  cout<<"t = "<<times[2]/T<<" (PARTITION)"<<endl;
  return 0;
}