#ifndef REGISTRY_H
#define REGISTRY_H

#define SUBKEY "Software\\EvoXDTSR\\"

class REG
{
	HKEY hKey;
	void Open();	//Opens key for reading/writing
	void Close();		//Closes reg
public:
	REG();
	~REG();
	void WriteInt(char valname[],int i);	//Writes int value to reg
	void WriteInt(char valname[],DWORD i);
	int ReadInt(char valname[]);			//Reads int value from reg
	DWORD ReadDword(char valname[]);
	void WriteStr(char valname[],char buf[]);	//Writes \0 str to reg
	void ReadStr(char valname[],char buf[],DWORD bufsize);		//Reads \0 str from reg
};
#endif