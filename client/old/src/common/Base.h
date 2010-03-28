#ifndef  BASE_H_
#define  BASE_H_

#ifndef MACROS_H_
#include "Macros.h"
#endif

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

using namespace std;

typedef pair<string,string> pairss;  	 
typedef vector<pairss> vectorpairss;  	 
typedef vector<pairss>::iterator vectorpairssor;  	 
typedef vector<pairss>::const_iterator vectorpairssctor;  	 

typedef map<string,string> mapss;
typedef map<string,string>::iterator mapssor;
typedef map<string,string>::const_iterator mapssctor;


#include "Logging.h" 

enum YlemState{YLEM_OK=0,YLEM_ERR,YLEM_SOCKET_ERR,YLEM_OPENFILE_ERR,YLEM_WEBDAV_ERR};

#endif

