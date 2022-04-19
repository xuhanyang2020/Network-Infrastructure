#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unordered_map>
#include <vector>
#include <queue>
#include <unordered_set>

using namespace std;

void initConfig(string topoFile, string messFile, string changeFile);
void dijkstra(int start);
void displayInfo();

vector<vector<int> > globalPathInfo;
unordered_map<int, unordered_map<int, vector<int> > > nodePathMap;
unordered_map<int, unordered_map<int, int > > nodeCost;
unordered_map<int, unordered_map<int, string> > messageMap;
vector<pair<int, int> > messagePair;
vector<string> messageContent;
vector<pair<int, int> > changePair;
vector<int> changeContent;
int nodeNum = 0;
int changeRound = 0;
ofstream outFile;

// unordered_map<int, int> next

int main(int argc, char** argv) {
    //printf("Number of arguments: %d", argc);
    if (argc != 4) {
        printf("Usage: ./linkstate topofile messagefile changesfile\n");
        return -1;
    } 

    outFile.open("output.txt", ios_base::out);

    // load in all of the config files
    initConfig(argv[1], argv[2], argv[3]);

    // change round
    // for (int round = 0; round < changeRound; round++){
    //     // calculate the path each round
        for (int i = 1; i <= nodeNum; i++){
            dijkstra(i);
        }
    //     displayInfo();
    // }

    
    // getMessages(argv[2]);
    // getChanges(argv[3]);

    // priority_queue<pair<int, int> , vector<pair<int, int> >, greater<pair<int, int> > > pq;
    // pq.push(make_pair(3, 0));
    // pq.push(make_pair(0, 0));
    // pq.push(make_pair(4, 0));
    // cout<<pq.top().first<<endl;
    
    displayInfo();

    // FILE *fpOut;
    

    return 0;
}

void initConfig(string topoFile, string messFile, string changeFile){

    // get the topology of the graph
    ifstream inTopo(topoFile);
    string line;
   
    while (getline(inTopo, line)){
        stringstream sstr;
        sstr << line;

        string node1_s, node2_s, cost_s;
        getline(sstr, node1_s, ' ');
        getline(sstr, node2_s, ' ');
        getline(sstr, cost_s, ' ');

        int node1 = stoi(node1_s);
        int node2 = stoi(node2_s);
        int cost = stoi(cost_s);
        // update nodeNum
        nodeNum = max(nodeNum, node1);
        nodeNum = max(nodeNum, node2);
        // construct global path vector
        vector<int> group;
        group.push_back(node1);
        group.push_back(node2);
        group.push_back(cost);
        globalPathInfo.push_back(group);
    }

    // get content of message file
    ifstream inMess(messFile);
    string messLine;

    while (getline(inMess, messLine))
    {
        stringstream sstr;
        sstr << messLine;
        
        string node1_s, node2_s, m;
        getline(sstr, node1_s, ' ');
        getline(sstr, node2_s, ' ');
        getline(sstr, m,' ');

        int node1 = stoi(node1_s);
        int node2 = stoi(node2_s);

        messagePair.push_back(make_pair(node1, node2));
        messageContent.push_back(m);
    }
    
    // get the content of changeFile
    ifstream inChange(changeFile);
    string changeLine;

    while (getline(inChange, changeLine))
    {
        stringstream sstr;
        sstr << changeLine;
        
        string node1_s, node2_s, costs_s;
        getline(sstr, node1_s, ' ');
        getline(sstr, node2_s, ' ');
        getline(sstr, costs_s,' ');

        int node1 = stoi(node1_s);
        int node2 = stoi(node2_s);
        int cost = stoi(costs_s);

        changePair.push_back(make_pair(node1, node2));
        changeContent.push_back(cost);
    }
}

void dijkstra(int start){
    int matrix[nodeNum + 1][nodeNum + 1]; // 2D array to store local path cost
    int costs[nodeNum + 1]; // cost to all nodes

    for (int i = 1; i <= nodeNum; i++){
        for (int j = 1; j <= nodeNum; j++){
            matrix[i][j] = i == j ? 0 : INT_MAX;
        }
    }
    // store all visited node
    bool visited[nodeNum + 1];
    for (int i = 0; i <= nodeNum; i++) {
        visited[i] = false;
        costs[i] = INT_MAX;
    }
    costs[start] = 0;

    int lastHop[nodeNum + 1];
    lastHop[start] = start;

    for (int i = 0; i < globalPathInfo.size(); i++){
        int n1 = globalPathInfo.at(i)[0];
        int n2 = globalPathInfo.at(i)[1];
        int localCost = globalPathInfo.at(i)[2];
        matrix[n1][n2] = localCost;
        matrix[n2][n1] = localCost;
    }
    
    priority_queue<pair<int, int> , vector<pair<int, int> >, greater<pair<int, int> > > pq; // min heap
    pq.push(make_pair(0, start));

    while (!pq.empty()){
        pair<int, int> tmp = pq.top();
        int cost = tmp.first;
        int node = tmp.second;
        // cout<<"current node is" << node<<endl;
        pq.pop();
        if (visited[node]){continue;}
        visited[node] = true;
        // cout<<"current node is" << node<<endl;
        for (int i = 1; i <= nodeNum; i++){
            // cout<<"i == "<<i<<endl;
            // the node has not been visited and the distance is not infinity
            if (!visited[i] && matrix[node][i] != INT_MAX){
                // have to update the path cost
                if (cost + matrix[node][i] < costs[i]){
                    // cout<<"update node" << i<<endl;
                    costs[i] = cost + matrix[node][i];
                    lastHop[i] = node;
                    pq.push(make_pair(costs[i], i));
                }            
            }
        }
    }

    cout<< "topology starting from node "<<start<<endl;
    for (int i = 1; i <= nodeNum; i++){
        cout<< i << "   cost   " << costs[i] << "   lasthop  " << lastHop[i]<<endl;
    }
    
    // update costs vector
    for (int i = 1; i <= nodeNum; i++){
        nodeCost[start][i] = costs[i];
    }
    // calculate the path
    unordered_map<int, vector<int> > path;
    for (int i = 1; i <= nodeNum; i++){
        vector<int> tmp;
        tmp.push_back(i);
        int cur = lastHop[i];
        while (cur != start){
            tmp.insert(tmp.begin(), cur);
            cur = lastHop[cur];
        }
        if (i != start){
            tmp.insert(tmp.begin(), start);
        }
        path[i] = tmp;
    }
    nodePathMap[start] = path;
}

void displayInfo(){
    for (int node = 1; node <= nodeNum; node++){
        unordered_map<int, vector<int> > map1 = nodePathMap[node];
        outFile<<"starting from node "<<node<<endl;
        for (auto i = 1; i <= nodeNum; i++){
            vector<int> p = map1[i];
           
            int cost = nodeCost[node][i];
            outFile<<i<<" ";
            if (i == node){
                outFile<<i<<" ";
            } else {
                outFile<<p.at(1)<<" ";
                // cout<<p.at(1)<<" ";
            }
            outFile<<cost<<endl;
        }
    } 
}
