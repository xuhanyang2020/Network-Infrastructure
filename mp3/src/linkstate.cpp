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

#define INTEGER_MAX 2147483647

void initConfig(string topoFile, string messFile, string changeFile);
void dijkstra(int start);
void displayInfo();
void displayMessages();
void changePerRound(int round);

vector<vector<int> > globalPathInfo;
unordered_map<int, unordered_map<int, vector<int> > > nodePathMap;
unordered_map<int, unordered_map<int, int > > nodeCost;

// unordered_map<int, unordered_map<int, string> > messageMap;
vector<pair<int, int> > messagePair;
vector<string> messageContent;

vector<pair<int, int> > changePair;
vector<int> changeContent;

int nodeNum = 0;
int changeRoundIndex = 0;
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

    for (int round = 0; round < changeRoundIndex; round++){
        // calculate the path each round
        for (int i = 1; i <= nodeNum; i++){dijkstra(i);}
        displayInfo();
        displayMessages();
        changePerRound(round);
    }

    for (int i = 1; i <= nodeNum; i++){dijkstra(i);}
    displayInfo();
    displayMessages();

    return 0;
}

void changePerRound(int round){
    // cout<<"change round "<<round<<endl;
    int n1 = changePair.at(round).first;
    int n2 = changePair.at(round).second;
    // if exists in global path, update for delele the content
    for (int i = 0; i < globalPathInfo.size(); i++){
        if ((n1 == globalPathInfo.at(i).at(0) && n2 == globalPathInfo.at(i).at(1)) ||
        (n2 == globalPathInfo.at(i).at(0) && n1 == globalPathInfo.at(i).at(1))){
            if (changeContent.at(round) >= 0){
                // update the path
                globalPathInfo.at(i).at(2) = changeContent.at(round);
            } else {
                // delete the path
                globalPathInfo.erase(globalPathInfo.begin() + i);
            }
            return;
        }
    }
    // no exists, create a new vector
    if (changeContent.at(round) >= 0){
        vector<int> newGroup;
        newGroup.push_back(n1);
        newGroup.push_back(n2);
        newGroup.push_back(changeContent.at(round));
        globalPathInfo.push_back(newGroup);
    }
    
}

void displayMessages(){
    int messNum = messagePair.size();
    for (int i = 0; i < messNum; i++){
        int startNode = messagePair.at(i).first;
        int dest = messagePair.at(i).second;
        bool reachable = nodePathMap[startNode][dest].size() != 0;
        if (!reachable){
            outFile<<"from "<< startNode<< " to "<<dest<<" cost infinite hops unreachable message "<<messageContent.at(i)<<endl;
        } else {
            outFile<<"from "<< startNode<< " to "<<dest<<" cost ";
            // cost
            outFile<<nodeCost[startNode][dest]<<" hops ";
            for (int j = 0; j < nodePathMap[startNode][dest].size() - 1; j++){
                outFile<<nodePathMap[startNode][dest].at(j)<<" ";
            }
            outFile<<"message ";
            outFile<<messageContent.at(i)<<endl;
        }
    }
    outFile<<endl;

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
        getline(sstr, m);

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
        // cout<<"newline in change file"<<endl;
        string node1_s, node2_s, costs_s;
        getline(sstr, node1_s, ' ');
        getline(sstr, node2_s, ' ');
        getline(sstr, costs_s,' ');

        int node1 = stoi(node1_s);
        int node2 = stoi(node2_s);
        int cost = stoi(costs_s);

        changeRoundIndex++;
        changePair.push_back(make_pair(node1, node2));
        changeContent.push_back(cost);
    }
}

void dijkstra(int start){
    int matrix[nodeNum + 1][nodeNum + 1]; // 2D array to store local path cost
    int costs[nodeNum + 1]; // cost to all nodes
    int lastHop[nodeNum + 1]; // last hop before reaching target
    bool visited[nodeNum + 1]; // store visited nodes

    for (int i = 1; i <= nodeNum; i++){
        for (int j = 1; j <= nodeNum; j++){
            matrix[i][j] = i == j ? 0 : INTEGER_MAX;
        }
    }

    for (int i = 0; i <= nodeNum; i++) {
        visited[i] = false;
        costs[i] = INTEGER_MAX;
    }
    costs[start] = 0;
    lastHop[start] = start;

    for (int i = 0; i < globalPathInfo.size(); i++){
        int n1 = globalPathInfo.at(i)[0];
        int n2 = globalPathInfo.at(i)[1];
        int localCost = globalPathInfo.at(i)[2];
        matrix[n1][n2] = localCost;
        matrix[n2][n1] = localCost;
    }

    // for (int i = 1; i <= nodeNum; i++){
    //     for (int j = 1; j <= nodeNum; j++){
    //         cout<<matrix[i][j]<<" ";
    //     }
    //     cout<<endl;
    // }
    
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
            if (!visited[i] && matrix[node][i] != INTEGER_MAX){
                // have to update the path cost, here use || to break the tie
                if (cost + matrix[node][i] < costs[i] || (cost + matrix[node][i] == costs[i] && node < lastHop[i])){
                    // cout<<"update node" << i<<endl;
                    costs[i] = cost + matrix[node][i];
                    lastHop[i] = node;
                    pq.push(make_pair(costs[i], i));
                }            
            }
        }
    }
    // print the routing table
    // cout<< "topology entries for node "<<start<<endl;
    // for (int i = 1; i <= nodeNum; i++){
    //     cout<< i << "   cost   " << costs[i] << "   lasthop  " << lastHop[i]<<endl;
    // }
    // update costs vector
    for (int i = 1; i <= nodeNum; i++){
        nodeCost[start][i] = costs[i];
    }
    // calculate the path
    unordered_map<int, vector<int> > path;
    for (int i = 1; i <= nodeNum; i++){
        vector<int> tmp;
        // if the node is not reachable, add a void vector and continue
        if (costs[i] == INTEGER_MAX){
            path[i] = tmp;
            continue;
        } 
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
    // traverse all the nodes
    for (int node = 1; node <= nodeNum; node++){
        unordered_map<int, vector<int> > map1 = nodePathMap[node];
        // outFile<<"topology entries for node "<<node<<endl;
        outFile<<endl;
        // for each node, print out the topo
        for (int i = 1; i <= nodeNum; i++){
            vector<int> p = map1[i];
            // if the node is not reachable, do not need to print it out
            if (p.size() == 0) continue;
            // fileout the data according to the format
            int cost = nodeCost[node][i];
            outFile<<i<<" ";
            if (i == node){
                outFile<<i<<" ";
            } else {
                outFile<<p.at(1)<<" "; // next
            }
            outFile<<cost<<endl;
        }
    } 
}
