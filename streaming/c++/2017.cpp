#include<iostream>
#include<stdio.h>
#include<string.h>
#include <algorithm>
#include <chrono>
#include <ctime>  
#include <math.h>
#include <fstream>
#include <list>
#include <vector>
#include <map>
#define FR(i,a,b) for(i=a;i<b;++i)
#define FRS(i,a,b,s) for(i=a;i<b;i+=s)
#define FRE(i,a,b) for(i=a;i<=b;++i)
#define FRES(i,a,b,s) for(i=a;i<=b;i+=s)
#define FRR(i,a,b) for(i=a-1;i>=b;--i)
// [0,tt)     FR(i, 0, tt) printf(" 1");
// [0,2,4,...,tt) FRS(i, 0, tt, 2) printf(" 2");
// [0,tt]    FRE(i, 0, tt) printf(" 3");
// [0,2,4,...,tt]   FRES(i, 0, tt, 2) printf(" 4");
using namespace std;
// loop
int i,j,k,x,y,z,u,v,w,p,q,r,s,t;
char ch; 

void outputCurrentTime(){
	auto end = std::chrono::system_clock::now();
	std::time_t end_time = std::chrono::system_clock::to_time_t(end);
	cout << std::ctime(&end_time); 
}

int numV,numEnd,numReq,numSer,cap;	// para
int vs[10001]; // video size
int lat[1001]; 
struct edge{
	int ser;
	int lat;
};
int numEdge[1001];
edge end2SerL[1001][1001]; 
struct request{
	int video;
	int end;
	int num;
};
request req[1000001];
long long denom;
void readInput() {
	cin >> numV >> numEnd >> numReq >> numSer >> cap;
	FR(i, 0, numV) cin >> vs[i];
	FR(i,0,numEnd){
		cin >> lat[i] >> numEdge[i];
		FR(j,0,numEdge[i]){
			cin >> end2SerL[i][j].ser >> end2SerL[i][j].lat;
		}
	}
	FR(i,0,numReq) cin >> req[i].video >> req[i].end >> req[i].num;
	s = 0;
	FR(i,0,numReq) s += req[i].num;
	printf("%d %d %d %d %d\n", numV, numEnd, numReq, numSer, cap);
}


// answer
long long score = 0;
long long bestScore = 0;
int numSerV[1001];
int ser2VL[1001][10001]; // cache, # => video

bool b[1001][10001]; // server, video, cached?
long long calc(){
	// max score = (1,000,000 req * 10,000 num * 4,000 lat) / (1,000,000 req * 10,000 num)
	long long sum = 0;
	/* // too slow
	FR(i,0,numEnd) FR(j,0,numV) distEV[i][j] = 0;
	FR(i,0,numEnd) FR(j,0,numEdge[i]) {
		s = end2SerL[i][j].ser;
		FR(k,0,numSerV[s]) if(distEV[i][ser2VL[s][k]] < lat[i] - end2SerL[i][j].lat) distEV[i][ser2VL[s][k]] = lat[i] - end2SerL[i][j].lat;
	}
	FR(i,0,numReq) {
		sum += req[i].num * distEV[req[i].end][req[i].video];
	}
	///*/
	FR(i,0,numSer) FR(j,0,numV) b[i][j] = false;
	FR(i,0,numSer) FR(j,0,numSerV[i]) b[i][ser2VL[i][j]] = true;
	FR(i,0,numReq){
		int minn = lat[req[i].end];
		FR(j,0,numEdge[req[i].end]) if(b[end2SerL[req[i].end][j].ser][req[i].video] && minn > end2SerL[req[i].end][j].lat) minn = end2SerL[req[i].end][j].lat;
		sum += req[i].num * (lat[req[i].end] - minn);
	}
	// verify server cap
	FR(i,0,numSer){
		int ss = 0;
		FR(j,0,numSerV[i]) ss += vs[ser2VL[i][j]];
		if(ss > cap) throw;
	}
	return sum;
}

//{ output solution
char fName[50];
void writeSol(){
	double readableScore = score * 1000.0 / denom;
	double readableBestScore = bestScore * 1000.0 / denom;
	//sprintf(fName, ".\\output\\%d\\%lld_%d.out",cap,score,time(NULL));
	sprintf(fName, "./output/%d/%011.3f_%d.out",cap,readableScore,time(NULL));
	FILE* fout = fopen(fName, "w");
	fprintf(fout,"%d\n",numSer);
	FR(i,0,numSer){
		fprintf(fout, "%d", i);
		FR(j,0,numSerV[i]) fprintf(fout, " %d", ser2VL[i][j]);
		fprintf(fout, "\n");
	}
	fclose(fout);
	//printf("======Score: %lld %lld %f %f\n", score, bestScore, score *1000.0 / denom, bestScore * 1000.0 / denom);
	printf("======Score: %011.3f %011.3f\n", readableScore, readableBestScore);
	outputCurrentTime();
	
}
//}

void init(){
	score = bestScore = 0;
	FR(i,0,numSer) numSerV[i] = 0;
	denom = 0;
	FR(i,0,numReq) denom += req[i].num;
	// test
	//FR(i,0,numSer) numSerV[i] = numV;
	//FR(i,0,numSer) FR(j,0,numV) ser2VL[i][j] = j;
}

// cache from 0 to max
void greedy_0(){
	FR(i,0,numSer){
		numSerV[i] = 0;
		int total = 0;
		FR(j,0,numV) if(total + vs[j] <= cap){
			ser2VL[i][numSerV[i]] = j;
			numSerV[i]++;
			total += vs[j];
		}
	}
	score = calc();
	if(score > bestScore) bestScore = score;
}

bool compareVS(int i, int j) {return vs[i] < vs[j];}
// cache from smallest V to largest
void greedy_1(){
	int sortedV[10001];
	FR(i,0,numV) sortedV[i] = i;
	sort(sortedV, sortedV + numV, compareVS);
	FR(i,0,numSer){
		numSerV[i] = 0;
		int total = 0;
		FR(j,0,numV) if(total + vs[sortedV[j]] <= cap){
			ser2VL[i][numSerV[i]] = sortedV[j];
			numSerV[i]++;
			total += vs[sortedV[j]];
		}
	}
	score = calc();
	if(score > bestScore) bestScore = score;
}

// round robin cache
void greedy_2(){
	FR(i,0,numSer){
		numSerV[i] = 0;
		int total = 0;
		FR(j,0,numV) if(total + vs[(j+i)%numV] <= cap){
			ser2VL[i][numSerV[i]] = (j+i)%numV;
			numSerV[i]++;
			total += vs[(j+i)%numV];
		}
	}
	score = calc();
	if(score > bestScore) bestScore = score;
}

int distEV[1001][10001];
void greedy_3(){
	// for cache video max save/size
	// for request?
	FR(i,0,numEnd) FR(j,0,numV) distEV[i][j] = lat[i];
	int used[1001];
	FR(i,0,numSer) used[i] = 0;
	FR(i,0,numSer) numSerV[i] = 0;
	long long pg = 0;
	map<int, long long> cacheVBonus[1001];
	while(true){
		FR(i,0,numSer) cacheVBonus[i].clear();
		FR(i,0,numReq){
			FR(j,0,numEdge[req[i].end]){
				x = end2SerL[req[i].end][j].ser;
				y = end2SerL[req[i].end][j].lat;
				if(y < distEV[req[i].end][req[i].video]){
					cacheVBonus[x][req[i].video] += req[i].num * (distEV[req[i].end][req[i].video] - y);
				}
			}
		}
		bool picked = false;
		double maxx = 0;
		int pickSer, pickV;
		FR(i,0,numSer){
			//printf("Count: %d\n",cacheVBonus[i].size());
			for(auto const& kvp : cacheVBonus[i]){
				if(kvp.second * 1.0 / vs[kvp.first] > maxx && used[i] + vs[kvp.first] <= cap){
					maxx = kvp.second * 1.0 / vs[kvp.first];
					pickSer = i;
					pickV = kvp.first;
					picked = true;
				}
			}
		}
		if(picked){
			FR(i,0,numEnd){
				FR(j,0,numEdge[i]){
					if(end2SerL[i][j].ser != pickSer) continue;
					y = end2SerL[i][j].lat;
					if(y < distEV[i][pickV]){
						distEV[i][pickV] = y;
					}
				}
			}
			used[pickSer] += vs[pickV];
			ser2VL[pickSer][numSerV[pickSer]++] = pickV;
			pg += cacheVBonus[pickSer][pickV];
			z = 0;
			FR(i,0,numSer) z += cap - used[i];
			printf("progress: %lld %f %d %d %d\n", pg, pg * 1000.0 / denom, pickSer, pickV, z);
			outputCurrentTime();
		}else break;
	}
	score = calc();
	if(score > bestScore) bestScore = score;
}
long long cacheVBonus[1001][10001];
void greedy_3_1(){
	// for cache video max save/size
	// for request?
	FR(i,0,numEnd) FR(j,0,numV) distEV[i][j] = lat[i];
	int used[1001];
	FR(i,0,numSer) used[i] = 0;
	FR(i,0,numSer) numSerV[i] = 0;
	long long pg = 0;
	while(true){
		FR(i,0,numSer) FR(j,0,numV) cacheVBonus[i][j] = 0;
		FR(i,0,numReq){
			FR(j,0,numEdge[req[i].end]){
				x = end2SerL[req[i].end][j].ser;
				y = end2SerL[req[i].end][j].lat;
				if(y < distEV[req[i].end][req[i].video]){
					cacheVBonus[x][req[i].video] += req[i].num * (distEV[req[i].end][req[i].video] - y);
				}
			}
		}
		bool picked = false;
		double maxx = 0;
		int pickSer, pickV;
		FR(i,0,numSer){
			FR(j,0,numV){
				if(cacheVBonus[i][j] * 1.0 / vs[j] > maxx && used[i] + vs[j] <= cap){
					maxx = cacheVBonus[i][j] * 1.0 / vs[j];
					pickSer = i;
					pickV = j;
					picked = true;
				}
			}
		}
		if(picked){
			used[pickSer] += vs[pickV];
			FR(i,0,numEnd){
				FR(j,0,numEdge[i]){
					if(end2SerL[i][j].ser != pickSer) continue;
					y = end2SerL[i][j].lat;
					if(y < distEV[i][pickV]){
						distEV[i][pickV] = y;
					}
				}
			}
			ser2VL[pickSer][numSerV[pickSer]++] = pickV;
			pg += cacheVBonus[pickSer][pickV];
			z = 0;
			FR(i,0,numSer) z += cap - used[i];
			printf("progress: %lld %f %d %d %d\n", pg, pg * 1000.0 / denom, pickSer, pickV, z);
			outputCurrentTime();
		}else break;
	}
	score = calc();
	if(score > bestScore) bestScore = score;
}

// iteratively, (for each video, pick the most valuable server to cache at)
void greedy_3_2(){
	// for cache video max save/size
	// for request?
	FR(i,0,numEnd) FR(j,0,numV) distEV[i][j] = lat[i];
	int used[1001];
	FR(i,0,numSer) used[i] = 0;
	FR(i,0,numSer) numSerV[i] = 0;
	long long pg = 0;
	while(true){
		FR(i,0,numSer) FR(j,0,numV) cacheVBonus[i][j] = 0;
		FR(i,0,numReq){
			FR(j,0,numEdge[req[i].end]){
				x = end2SerL[req[i].end][j].ser;
				y = end2SerL[req[i].end][j].lat;
				if(y < distEV[req[i].end][req[i].video]){
					cacheVBonus[x][req[i].video] += req[i].num * (distEV[req[i].end][req[i].video] - y);
				}
			}
		}
		bool pickedOnce = false;
		double maxx = 0;
		int pickSer, pickV;
		FR(i,0,numV){
			maxx = 0;
			bool picked = false;
			FR(j,0,numSer){
				if(cacheVBonus[j][i] * 1.0 / vs[i] > maxx && used[j] + vs[i] <= cap){
					maxx = cacheVBonus[j][i] * 1.0 / vs[i];
					pickSer = j;
					pickV = i;
					picked = true;
				}
			}
			if(picked){
				used[pickSer] += vs[pickV];
				FR(p,0,numEnd){
					FR(q,0,numEdge[p]){
						if(end2SerL[p][q].ser != pickSer) continue;
						y = end2SerL[p][q].lat;
						if(y < distEV[p][pickV]){
							distEV[p][pickV] = y;
						}
					}
				}
				ser2VL[pickSer][numSerV[pickSer]++] = pickV;
				pg += cacheVBonus[pickSer][pickV];
				pickedOnce = true;
			}
		}
		if(pickedOnce){
			z = 0;
			FR(i,0,numSer) z += cap - used[i];
			printf("progress: %lld %f %d %d %d\n", pg, pg * 1000.0 / denom, pickSer, pickV, z);
			outputCurrentTime();
		}else break;
	}
	score = calc();
	if(score > bestScore) bestScore = score;
}

void readSol(const char* path){
	ifstream fin(path);
	FR(i,0,numSer) numSerV[i] = 0;

	score = calc();
	bestScore = score;
}

int main()
{
	srand(time(NULL));
	readInput(); printf("Done intput\n");
	//
	init();	printf("Done init\n");
	greedy_0(); printf("Done greedy 0\n"); writeSol();
	greedy_1(); printf("Done greedy 1\n"); writeSol();
	greedy_2(); printf("Done greedy 2\n"); writeSol();
	greedy_3(); printf("Done greedy 3_2\n"); writeSol();
	
	//greedyWithRand(); printf("Done greedy with rand\n");
	//readSol(".\\output\\0903146_1550509764.out");
	//
	//simAnn();
	writeSol(); printf("Finished\n");
	return 0;
}