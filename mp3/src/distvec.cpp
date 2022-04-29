#include <fstream>
#include <set>
#include <vector>
#include <iostream>
#include <unordered_map>

	
using namespace std;
	
// use unordered_map for topo
unordered_map<int, unordered_map<int, int> > topo;

//use a set for all nodes
set<int> nodes;

//define a struct for  message
typedef struct message {
 int src;
 int dest;
 string contents;
 //this function is to create a message	
 message(int src, int dest, string contents) : src(src), dest(dest), contents(contents) {}
} message;


//use a vector for all message
vector<message> message_all;
	
// files
string topofile, messagefile, changesfile;
ofstream outputfile;


// create a forwaord table
//this table will include source, destination, nexthop and cost
unordered_map<int, unordered_map<int, pair<int, int> > > forward_table;

//this funciton if sto check whether a value exist if x,y are given
//In another word, it can check wheter a src and a dest are connected
//return true or false	
bool check_connection(int x, int y, unordered_map<int, unordered_map<int, int> > map) {
 auto it = map.find(x);
 auto res = it->second.find(y);
 return !(res == it->second.end());
}

//this function is to read the contents in the topofile
//Then, initiate a forward table based on it	
void readTopo(string file, unordered_map<int, unordered_map<int, int> > &topo, unordered_map<int, unordered_map<int, pair<int, int> > > &forward_table) {

 ifstream topoin;
 topoin.open(file, ios::in);

	
 int src, dest, cost;
 
 //this loop is to put what we read from the file into the map of topo
 while (topoin >> src >> dest >> cost) {
  topo[src][dest] = cost;
  topo[dest][src] = cost;
  
  //put nodes in the set of nodes
  if (nodes.find(src) == nodes.end()) {
  
   nodes.insert(src);
  }
  
  if (nodes.find(dest) == nodes.end()) {
   nodes.insert(dest);
  }
 }
 
 
 // initiate the first forward table
 
 for (auto i = nodes.begin(); i != nodes.end(); i++) {
 
  src = *i;
  for (auto j = nodes.begin(); j != nodes.end(); j++) {
	
   dest = *j;
   
   //if source=destination, set cost to 0
   if (src == dest) {
   
    topo[src][dest] = 0;
   }
   //if source is not connected to a destination, set cost to -999
   if (!check_connection(src, dest, topo)) {
	 topo[src][dest] = -999;
   }
   
   //here just assume that the path from src to dest is the shortest path
   //this cost is put in the forward table
   //we can update it later in another function
   forward_table[src][dest] = make_pair(dest, topo[src][dest]);
  }
 }
 topoin.close();
}
	
//This function is to get the forward table
void getforwardtable(unordered_map<int, unordered_map<int, pair<int, int> > > &forward_table) {

 int num = nodes.size();
 int src, dest, next_hop, min_Cost;
 
 //start to create a forward table
 //we will try every node as source and destination to check whether there can be new shorter path.
 //everytime we find a shorter path, we need to redo the whole work because of the new path information
 //to guarantee we can the final best forward table, we need to calculate n^3 times so here are 3 for loops 
 for (int i = 0; i < num; i++) {
 
  for (auto j = nodes.begin(); j != nodes.end(); j++) {
  
   src = *j;
   for (auto k = nodes.begin(); k != nodes.end(); k++) {

    dest = *k;

    next_hop = forward_table[src][dest].first;
    min_Cost = forward_table[src][dest].second;
    
    //check whether there is a node to make a path shorter
    for (auto l = nodes.begin(); l != nodes.end(); l++) {
    
     if (topo[src][*l] >= 0 && forward_table[*l][dest].second >= 0) {
      int new_Cost = topo[src][*l] + forward_table[*l][dest].second;
      
      //reset next hop and minimium cost
      if (min_Cost < 0 || new_Cost < min_Cost || (new_Cost==min_Cost && *l< next_hop && *l !=src)) {
      
       next_hop = *l;
       min_Cost = new_Cost;
      }
     }
    }
    //reset forward table for this pair src and dest
    forward_table[src][dest] = make_pair(next_hop, min_Cost);
    
   }
  }
 }
 
 outputfile << endl;
 
 //output
 for (auto l = nodes.begin(); l != nodes.end(); l++) {
  src = *l;
  for (auto i = nodes.begin(); i != nodes.end(); i++) {
   dest = *i;
   outputfile << dest << " " << forward_table[src][dest].first << " " << forward_table[src][dest].second << endl;
  }
 }
 
}
	
//this function is to read the message from the file
void readMessage(string file) {
 ifstream msgfile(file);
 string line, contents;
 
 if (msgfile.is_open()) {
  //read the message line by line
  while (getline(msgfile, line)) {
	
   int src, dest;
   //scan the information
   sscanf(line.c_str(), "%d %d %*s", &src, &dest);
   contents = line.substr(line.find(" "));
   contents = contents.substr(line.find(" ") + 1);
   //set new meassage and put it into the vector
   message newMessage(src, dest, contents);
   message_all.push_back(newMessage);
  }
 }
 
 msgfile.close();
}

//This function is to send the message	
void sendMessage() {
 
 int src, dest, cost, next_hop;
 
 for (int i = 0; i < message_all.size(); i++) {
	 
  src = message_all[i].src;
  dest = message_all[i].dest;
  next_hop = src;
  outputfile << "from " << src << " to " << dest << " cost ";
  cost = forward_table[src][dest].second;
	 
  if (cost == -999) {
   outputfile << "infinite hops unreachable ";
  } else {
   outputfile << cost << " hops ";
   //before reaching the destination, we need to print the next hop
   while (next_hop != dest) {
   
    outputfile << next_hop << " ";
    //to find the next hop, we just need to set previous "next_hop" as src
    next_hop = forward_table[next_hop][dest].first;
   }
  }
  outputfile << "message " << message_all[i].contents << endl;
 }
}


//This function is to initiate a forward table	
void initForwardTable() {
 int src, dest;
 
 for (auto i = nodes.begin(); i != nodes.end(); i++) {
  src = *i;
  for (auto j = nodes.begin(); j != nodes.end(); j++) {
   dest = *j;
   forward_table[src][dest] = make_pair(dest, topo[src][dest]);
  }
 }
}

//THis function is to deal with the topo change	
void doChanges(string file) {
 ifstream change(file);
 int src, dest, cost;
 
 if (change.is_open()) {
  while (change >> src >> dest >> cost) {
   
   //put new information into topo
   topo[src][dest] = cost;
   topo[dest][src] = cost;
   //initiate a new forward table
   initForwardTable();
   //update the forward table
   getforwardtable(forward_table);
   //send message again
   sendMessage();
  }
 }
}
	
//main function	
int main(int argc, char **argv) {

 if (argc != 4) {
  printf("Usage: ./linkstate topofile messagefile changesfile\n");
  return -1;
 }
	
 topofile = argv[1];
 messagefile = argv[2];
 changesfile = argv[3];
 
 //read topofile
 readTopo(topofile, topo, forward_table);
 
 outputfile.open("output.txt");

 //get the forward table
 getforwardtable(forward_table);
 
 //read the message
 readMessage(messagefile);

 //send message
 sendMessage();

 //read the change file
 doChanges(changesfile);
	
 outputfile.close();
 return 0;
}


