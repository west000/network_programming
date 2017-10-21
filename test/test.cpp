#include <iostream>
#include <string>
#include <list>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

pair<uint32_t, uint32_t> getPair(const string &str)
{
	size_t pos = str.find("/", 0);
	string ip = str.substr(0, pos);
	string len = str.substr(pos+1, str.size());
    
    uint32_t ipInt = ntohl(inet_addr(ip.c_str()));
    uint32_t length = atoi(len.c_str());
    
    return pair<uint32_t, uint32_t>(ipInt, length);
}

int main()
{
    int k;
    cin >> k;
    
    list<string> netaddrList;
    list<pair<uint32_t, uint32_t>> pairList;
    for(int i=0; i<k; i++)
    {
    	string addr;
        cin >> addr;
        netaddrList.push_back(addr);
        pairList.push_back(getPair(addr));
    }
    
	for(auto it=pairList.begin(); it!=pairList.end(); ++it)
    {
        auto it1 = netaddrList.begin();
        for(auto it2=pairList.begin(); it2!=pairList.end();)
        {
            if(it == it2)
	    {
		it1++;
		it2++;
		continue;
	    }           
 
            int len = min(it->second, it2->second);
            uint32_t mask = (len==32 ? ~0 : ~((1 << (32-len))-1) );
            uint32_t ip1 = it->first & mask;
            uint32_t ip2 = it2->first & mask;
            if(ip1 == ip2) 
            {
            	if(it->second < it2->second)
                {
                 	pairList.erase(it2++);   
                    netaddrList.erase(it1++);
                }
                else
                {
                    it2++;
                    it1++;
                }
                    
            }
            else 
            {
                it2++;
                it1++;
            }
        }
    }
    
    cout << netaddrList.size() << endl;
    for(const auto &addr : netaddrList)
        cout << addr << endl;
    
    return 0;
}
