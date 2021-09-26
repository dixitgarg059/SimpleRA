#include <bits/stdc++.h>

using namespace std;
#include <fstream>
using std::ofstream;


int main()
{
	
	int n = 1000;
    int a[n][n];
    // int cnt = n*n;
    int cnt = (n*n)/20;
	cout << rand()%100;
    ofstream infile;
    infile.open("../data/CHECK.csv");
    for (int i = 0; i < n; ++i)
    {
    	for (int j = 0; j< n; ++j)
    	{
    		if(cnt>=0)
    		{
                cnt--;
                int x = rand()%20+1;
                a[i][j] = x;
    			infile << x;
    		}
    		else
    		{
    			infile << 0;
                a[i][j] = 0;
    		}
    		if(j!=n-1)
    		infile << ",";
    	}
    	infile << "\n";
    }
    ofstream opfile;
    opfile.open("../data/transposed.csv");
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            opfile << a[j][i];
            if(j!=n-1)
            opfile << ",";
        }
        opfile << "\n";
    }

    return 0;
}