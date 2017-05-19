#include <algorithm>

inline int min(const int &a, const int &b){
  return a>b?b:a;
}

inline int max(const int &a, const int &b){
  return a>b?a:b;
}

void merge(int *data, int N, int P){
  if(N == 1)
    return;
  if(P == 1){
    int i=0,j=N/2;
    int *tmp = new int[N/2];

    for(int k=0;k<N/2;k++)
      tmp[k] = data[k];

    for(int k=0;k<N;k++){
      if(i != N/2 && j != N)
      {
        if(tmp[i] < data[j]){
          data[k] = tmp[i];
          i++;
        }else{
          data[k] = data[j];
          j++;
        }
      }else if(j != N){
        data[k] = data[j];
        j++;
      }else{
        data[k] = tmp[i];
        i++;
      }
    }

    return;
  }

  // #pragma omp critical
  // std::cout << "merge("<< N <<","<< P <<")\n";

  int *odds = new int[N/2];
  int *evens = new int[N/2];
  #pragma omp parallel for num_threads(P)
  for(int i=0;i<N/2;i++){
    evens[i] = data[i*2];
    odds[i] = data[i*2+1];
  }

  #pragma omp parallel sections num_threads(2)
  {
    merge(odds, N/2, P/2);
    #pragma omp section
    merge(evens, N/2, P/2);
  }

  data[0] = evens[0];
  #pragma omp parallel for num_threads(P)
  for(int i=0;i < N/2 - 1;i++){
    data[i*2+1] = min(odds[i], evens[i+1]);
    data[i*2+2] = max(odds[i], evens[i+1]);
  }
  data[N-1] = odds[N/2 - 1];
  delete []odds;
  delete []evens;
}
void merge_sort(int *data, int N, int P){
  if(N == 1)
    return;
  if(P==1){
    std::sort(data, data+N);
    return;
  }
  #pragma omp parallel sections num_threads(2)
  {
    merge_sort(data, N/2, P/2);
    #pragma omp section
    merge_sort(data+N/2, N/2, P/2);
  }
  merge(data, N, P);
}