#pragma once
#include <vector>						//Required for connection list
#include "wsck_generic.h"
using namespace std;


class CLIENT : public GENSOCK {
private:
	vector<CONNFO>	ConList;	//list of connections
public:
	//CLIENT::CLIENT(const UINT major, const UINT minor);
	CLIENT::CLIENT(const int iMax);	//Constructer that sets max connections
	CLIENT::CLIENT();				//Regular constructer
	CLIENT::~CLIENT();
	SOCKET CLIENT::Connect(const HWND hwnd, const char szIP[30], const UINT iPort);	//Attempts to connect
	bool CLIENT::Close(SOCKET skt);			//Close a single connection
	bool CLIENT::CloseAll();				//Close all connections
	SOCKET skt;
};