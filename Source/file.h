#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <string>
#include <fstream>

HMODULE GetCurrentModule();
HANDLE OpenToWrite(const char* Filename);
HANDLE OpenToRead(const char* Filename);
void WriteByte(HANDLE file, int iValue);
int ReadByte(HANDLE file, bool* eof=NULL);
void WriteWord(HANDLE file, int iValue);
int ReadWord(HANDLE file);
void WriteInt(HANDLE file, int iValue);
int ReadInt(HANDLE file);
void WriteLong(HANDLE file, int iValue);
int ReadLong(HANDLE file);
void WriteString(HANDLE handle, const char* str);
std::string ReadString(HANDLE hanlde);

std::string getfilename (std::string dir, std::string prefix);

void openfile(std::string* ret);

std::string ExePath();

std::ifstream::pos_type filesize(const char* filename);

int GetLineCount (const char* filename);











int Compare( const void * Val1, const void * Val2 );
extern "C" __declspec(dllexport) DWORD CalculateWSPrivate(DWORD processID);