#include <iostream>
#include <omp.h>
#include <cassert>
#include <cstdlib>

#define P 12
#define LOGP 4
#define N 10000000
#define T 1000 //number of testcases
#define N1 1

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
int main(int argc, char **argv){
  srand(time(NULL));
  // assert(N % P == 0);
  cout<<"N = "<<N<<"\nP = "<<P<<endl;

  char *bits;
  int correct_result = 0;
  bits = new char[N];
  omp_set_num_threads(P);
  correct_result = 0;

  #pragma omp parallel for
  for(int i = 0; i < N; ++i)
  {
    auto p = N*float(rand())/RAND_MAX;
    if(p <= N1){
      bits[i] = 1;
      correct_result = 1;
    }else
      bits[i] = 0;
  }
  cout<<"correct_result = "<<correct_result<<endl;
  double results[3]={0,0,0};
  double times[3]={0,0,0};

  for(int t=0;t<T;t++){
    int result = 0;
    timer();
    for(int i=0;i<N;i++)
    {
      result |= bits[i];
    }
    times[0] += timer();
    results[0] += result;
    // cout<<"result = "<<result<<" (SERIAL)"<<endl;

    result = 0;
    timer();
    #pragma omp parallel for
    for(int i=0; i < N;i++){
      if(bits[i] == 1){
        // #pragma omp atomic
        result = 1;
      }
      // result |= bits[i];
    }
    times[1] += timer();
    results[1] += result;
    // cout<<"result = "<<result<<" (CW)"<<endl;
    result = 0;

    int tmp_result[P] = {0};
    timer();

    #pragma omp parallel
    {
      int j = omp_get_thread_num();
      int local_result = 0;
      #pragma omp for
      for(int i = 0; i < N;i++){
        local_result |= bits[i];
      }
      tmp_result[j] = local_result;
      for(int r=1;r<=LOGP;r++){
        #pragma omp barrier
        if((j & ((1<<r)-1)) == (1<<(r-1))){
          tmp_result[j - (1<<(r-1))] |= tmp_result[j];
        }
      }
    }
    result = tmp_result[0];
    times[2] += timer();
    results[2] += result;
  }
  cout<<"result = "<<results[0]/T<<", t = "<<times[0]/T<<" (SERIAL)"<<endl;
  cout<<"result = "<<results[1]/T<<", t = "<<times[1]/T<<" (CW)"<<endl;
  cout<<"result = "<<results[2]/T<<", t = "<<times[2]/T<<" (BINTREE)"<<endl;
  return 0;
}