#include <cmath>
#include <algorithm>
#include <vector>
#include <omp.h>
#include <iostream>

using namespace std;
// #ifndef rand
// int rand(){return 0;}
// #define RAND_MAX 1
// #endif

bool is_prime(int n){
  int sq = sqrt((float)n);
  for(int i=2;i<=sq;i+=2)
    if(n % i == 0)
      return false;
  return true;
}

int find_table_sz(int N){
  int p = pow(float(N),0.6f);
  if(p % 2 == 0)
    p++;
  while(!is_prime(p))
    p+=2;
  return p;
}

int compact_serial(int *data, int N, int P=1){
  int s=0;
  for(int i=0;i<N;i++){
    if(data[i]){
      data[s] = data[i];
      s++;
    }
  }
  if(s<N)
    data[s] = 0;
  return s;
}
int prefix_sum_serial(int *data, int N, int P=1){
  int sum=0;
  for(int i=0;i<N;i++){
    sum += data[i];
    data[i] = sum;
  }
  return sum;
}
int prefix_sum(int *data, int N, int P){
  std::vector<int> t_sum(P);
  #pragma omp parallel num_threads(P)
  {
    int t_id = omp_get_thread_num();
    #pragma omp for
    for(int i=0;i<N;i++){
      t_sum[t_id] += data[i];
    }
    for(int r=1;r<P;r*=2){
      int res;
      if(t_id >= r)
        res = t_sum[t_id] + t_sum[t_id-r];
      else
        res = t_sum[t_id];
      #pragma omp barrier
      t_sum[t_id] = res;
      #pragma omp barrier
    }

    int local_sum = 0;
    #pragma omp for
    for(int i=0;i<N;i++){
      local_sum += data[i];
      data[i] = t_sum[t_id - 1] + local_sum;
    }
  }
  return data[N-1];
}


int compact(int *data, int N, int P){
  int *psums = new int[N];
  #pragma omp parallel for num_threads(P)
  for(int i=0;i<N;i++){
    psums[i] = (data[i]==0?0:1);
  }
  prefix_sum_serial(psums, N, P);
  #pragma omp parallel for num_threads(P)
  for(int i=0;i<N;i++){
    if(data[i])
      data[psums[i]-1] = data[i];
  }
  int sum = psums[N-1];
  delete [] psums;
  if(sum<N)
    data[sum] = 0;
  return sum;
}

void sort_samples(int *data, int N, int P){
  std::sort(data, data+N);
  return;
  #pragma omp parallel for num_threads(P)
  for(int i=0;i<N;i++){
    //sort ~sqrt(N) values in O(logN) time
    int *compares = new int[N];
    #pragma omp parallel for num_threads(P)
    for(int j=0;j<N;j++){
      compares[j] = (data[i] > data[j]);
    }
  }
}
int binsearch(int *data, int N, int val){ //returns pos, s.t. val lies in data[pos-1],data[pos]
  // cerr<<"bin "<<N<<endl;
  if(N==0)
    return 0;
  if(N==1){
    if(val >= data[0])
      return 1;
    else
      return 0;
  }
  int mid = N/2;
  if(val < data[mid])
    return binsearch(data, mid, val);
  return mid + binsearch(data+mid, N - mid, val);
}

void partition_sort(int *data, int N, int P){
  // #pragma omp critical
  // cerr<<"part(data[0],N,P): "<<data[0]<<" "<<N<<" "<<P<<endl;
  if(N<=1)
    return;
  if(P==1){
    std::sort(data, data+N);
    return;
  }
  int sqrtN = sqrt((float)N);

  int table_sz = find_table_sz(N); // hash table size
  vector<int> table(table_sz);

  // table[0] = data[0];
  int a = rand() % (table_sz-1) + 1, b = rand() % table_sz; //for universal hashing
  #pragma omp parallel for num_threads(P)
  for(int i=0;i<N;i++){
    // sample ~ sqrt(N) values in O(1) time
    if(float(rand())/RAND_MAX < 1.f/sqrtN){
      int h = (a*data[i] + b) % table_sz;
      if(h<0)
        h += table_sz;
      assert(h >= 0 && h < table_sz);
      table[h] = data[i];
    }
  }
  // cerr << "+";
  int samples_sz = compact_serial(table.data(), table_sz, P); // O(logN) time
  sort_samples(table.data(), samples_sz, P); // O(logN) time

  std::vector< std::vector<int> > parts(samples_sz+1);
  #pragma omp parallel for num_threads(P)
  for(int i=0;i<N;i++){ // distribute to proper partition in O(log(sqrtN))) time
    int part = binsearch(table.data(), samples_sz, data[i]);
    assert(part>=0 && part <= samples_sz);
    #pragma omp critical (parts)
    parts[part].push_back(data[i]);
  }
  int *part_sizes = new int[samples_sz+1];
  #pragma omp parallel for num_threads(P)
  for(int i=0;i<samples_sz+1;i++){
    part_sizes[i] = parts[i].size();
  }

  int parts_sum = prefix_sum_serial(part_sizes, samples_sz+1, P);

  // cerr<<"szs: ";
  // for(int i=0;i<samples_sz;i++)
  //   cerr<<table[i]<<" ";
  // cerr<<"\n";

  assert(parts_sum == N);

  // compact buckets into contiguous array in O(log(sqrtN)) time
  #pragma omp parallel for num_threads(P)
  for(int i=0;i<N;i++){
    int part_idx = binsearch(part_sizes, samples_sz+1, i);
    int idx = i;
    assert(part_idx != samples_sz+1);
    if(part_idx>0)
      idx -= part_sizes[part_idx-1];
    data[i] = parts[part_idx][idx];
  }

  // #pragma omp critical
  // for(int i=0;i<N;i++)
  //   cerr<<data[i]<<" ";
  // cerr<<endl;

  parts.clear();
  // return;

  #pragma omp parallel for num_threads(int(sqrt(P)))
  for(int i=0;i<samples_sz+1;i++){
    int start_idx = 0;
    int sz = part_sizes[i];
    if(i>0){
      start_idx = part_sizes[i-1];
      sz -= part_sizes[i-1];
    }
    // #pragma omp critical
    // cerr<<"si:"<<start_idx<<",sz:"<<sz<<endl;
    if(sz)
      partition_sort(data+start_idx,sz,1);
  }
  delete [] part_sizes;
}