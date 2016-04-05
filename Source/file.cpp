#include "file.h"

#include "Psapi.h"
#pragma comment (lib,"Psapi.lib")

#include <vector>
#include <iostream>

#include "boost\\lexical_cast.hpp"
#include "boost\\filesystem.hpp"
using namespace boost::filesystem;
using namespace boost::filesystem3;

#include "misc.h"

#include "program.h"

#include "Commdlg.h"

HMODULE GetCurrentModule()
{
	HMODULE hModule = NULL;
	GetModuleHandleEx (GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)GetCurrentModule, &hModule);
	return hModule;
}


HANDLE OpenToWrite(const char* Filename)
{
    HANDLE file = CreateFile(Filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(file==INVALID_HANDLE_VALUE)
    {
		MessageBox(NULL, Filename, "Could not create file.", MB_ICONERROR|MB_OK);
		file=NULL;
    }
    return file;
}

HANDLE OpenToRead(const char* Filename)
{
    HANDLE file = CreateFile(Filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(file==INVALID_HANDLE_VALUE)
    {
		MessageBox(NULL, Filename, "Could not open file.", MB_ICONERROR|MB_OK);
		file=NULL;
    }
    return file;
}


void WriteByte(HANDLE file, int iValue)
{
    DWORD bytes;

    // Write to file
    unsigned char data = (unsigned char)iValue;
    if(WriteFile(file, &data, sizeof(data), &bytes, NULL) == 0)
	{
		MessageBox(NULL, "Could not write to file.", "Error", MB_ICONERROR|MB_OK);
	}
}

int ReadByte(HANDLE file, bool* eof)
{
	int iResult = 0;
	DWORD bytes;

	// Read from file
	unsigned char data;
	if(ReadFile(file, &data, sizeof(data), &bytes, NULL) == 0)
	{
		MessageBox(NULL, "Could not read from file.", "Error", MB_ICONERROR|MB_OK);
	}
	if (bytes==0&&eof!=NULL)
		*eof=true;
	iResult = data;
	return iResult;
}

void WriteWord(HANDLE file, int iValue)
{
    DWORD bytes;

    // Write to file
	WORD data = (WORD)iValue;
    if(WriteFile(file, &data, sizeof(data), &bytes, NULL) == 0)
	{
		MessageBox(NULL, "Could not write to file.", "Error", MB_ICONERROR|MB_OK);
	}
}

int ReadWord(HANDLE file)
{
	int iResult = 0;
	DWORD bytes;

	// Read from file
	WORD data;
	if(ReadFile(file, &data, sizeof(data), &bytes, NULL) == 0)
	{
		MessageBox(NULL, "Could not read from file.", "Error", MB_ICONERROR|MB_OK);
	}
	iResult = data;
	return iResult;
}

void WriteInt(HANDLE file, int iValue)
{
    DWORD bytes;

    // Write to file
	int data = (int)iValue;
    if(WriteFile(file, &data, sizeof(data), &bytes, NULL) == 0)
	{
		MessageBox(NULL, "Could not write to file.", "Error", MB_ICONERROR|MB_OK);
	}
}

int ReadInt(HANDLE file)
{
	int iResult = 0;
	DWORD bytes;

	// Read from file
	int data;
	if(ReadFile(file, &data, sizeof(data), &bytes, NULL) == 0)
	{
		MessageBox(NULL, "Could not read from file.", "Error", MB_ICONERROR|MB_OK);
	}
	iResult = data;
	return iResult;
}

void WriteLong(HANDLE file, int iValue)
{
    DWORD bytes;

    // Write to file
	DWORD data = (DWORD)iValue;
    if(WriteFile(file, &data, sizeof(data), &bytes, NULL) == 0)
	{
		MessageBox(NULL, "Could not write to file.", "Error", MB_ICONERROR|MB_OK);
	}
}

int ReadLong(HANDLE file)
{
	int iResult = 0;
	DWORD bytes;

	// Read from file
	DWORD data;
	if(ReadFile(file, &data, sizeof(data), &bytes, NULL) == 0)
	{
		MessageBox(NULL, "Could not read from file.", "Error", MB_ICONERROR|MB_OK);
	}
	iResult = data;
	return iResult;
}


void WriteString(HANDLE file, const char* str)
{
	DWORD pString = (DWORD)str;

    char carriage[3];
    carriage[0]=13;
    carriage[1]=10;
    carriage[2]=0;

    DWORD bytes;

	if (pString)
	{
		LPSTR string = (char*)pString;
		DWORD stringlength=strlen(string);

		if (stringlength)
		{
			WriteFile(file, string, stringlength, &bytes, NULL);
		}
	}
	// Write Carriage-Return to file
	WriteFile(file, carriage, 3, &bytes, NULL);
}





LPSTR GetReturnStringFromWorkString(char* WorkString)
{
	LPSTR pReturnString=NULL;
	if(WorkString)
	{
			DWORD dwSize=strlen(WorkString);
			//g_pCreateDeleteStringFunction((DWORD*)&pReturnString, dwSize+1);
			strcpy(pReturnString, WorkString);
	}
	return pReturnString;
}

DWORD ReadStringDWORD(HANDLE file, DWORD pDestStr)
{
	LPSTR pReturnString=0;

	if(file)
	{
		unsigned char c=0;
		DWORD bytes;
		std::vector<char> WorkString;

		do
		{
			ReadFile(file, &c, 1, &bytes, NULL);
			if(c>=32 || c==9)
				WorkString.push_back(c);
		} while((c>=32 || c==9));

		WorkString.push_back(0);

		if(c==13)
		{
				ReadFile(file, &c, 1, &bytes, NULL);
		}
		// Create and return string
		pReturnString=GetReturnStringFromWorkString( &WorkString[0] );
	}
    return (DWORD)pReturnString;
}

//char* ReadString(HANDLE file)
//{
//	static char* szReturn = NULL;
//	DWORD dwReturn = ReadStringDWORD ( file, NULL );
//	szReturn = ( char* ) dwReturn;
//	return szReturn;
//
//}

std::string ReadString(HANDLE file)
{
	std::string pReturnString="";
	if(file)
	{
		unsigned char c=0;
		DWORD bytes;

		bool eof=false;
		do
		{
			ReadFile(file, &c, 1, &bytes, NULL);
			if (bytes==0||c==13)
				eof=true;
			if(c>=32 || c==9 &&!eof)
				pReturnString.push_back(c);
		} while((c>=32 || c==9) && !eof);

		//pReturnString.push_back(0);

		if(c==13)
		{
				ReadFile(file, &c, 1, &bytes, NULL);
				if (bytes==0)
					eof=true;
		}
	}
    return pReturnString;
}

//char* ReadString(HANDLE file)
//{
//	char* pReturnString=0;
//	std::vector<char> WorkString;
//	if(file)
//	{
//		unsigned char c=0;
//		DWORD bytes;
//
//		//cout << "'";
//
//		bool eof=false;
//		do
//		{
//			ReadFile(file, &c, 1, &bytes, NULL);
//			if (bytes==0||c==13)
//			{	
//				eof=true;
//				//cout << "'\nA: '" << "EOF" << "'\n"; //
//			}
//			if(c>=32 || c==9 &&!eof)
//			{	
//				WorkString.push_back(c);
//				//cout << c << " "; //
//			}
//		} while((c>=32 || c==9) && !eof);
//
//		WorkString.push_back(0);
//
//		//cout << "B: '" << &WorkString << "'\n"; //
//
//		if(c==13)
//		{
//				ReadFile(file, &c, 1, &bytes, NULL);
//				if (bytes==0)
//					eof=true;
//		}
//		// Create and return string
//		pReturnString=&WorkString[0];
//
//		char* b = &WorkString[0];
//		cout << b << endl;
//		cout << pReturnString << endl;
//		
//		//cout << "C: '" << pReturnString << "'\n"; //
//	}
//    return pReturnString;
//}

std::string getfilename (std::string dir, std::string prefix)
{
	std::string ret="";
	path p(dir);
	if (exists(p))
	{
		if (is_directory(p))
		{
			unsigned int count=0;
			directory_iterator end_iter;
			for (directory_iterator dir_itr(p); dir_itr!=end_iter; ++dir_itr)
			{
				if ( is_regular_file(dir_itr->status()) )
				{
					std::string file = dir_itr->path().filename().string();
					if (file.find(prefix)!=std::string::npos)
						ret = file;
				}
			}
		}
	}
	if (ret!="")
		ret.insert(0, p.string());
	else
	{
		if (program->debug)
		{
			color(11);
			std::cout << "Could not std::string getfilename - FIX" << std::endl;
		}
	}
	return ret;
}

void openfile(std::string* ret)
{
	OPENFILENAME ofn={0};
	char szFileName[MAX_PATH]={0};
	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.Flags=OFN_ALLOWMULTISELECT|OFN_EXPLORER;
	ofn.lpstrFilter="All Files (*.*)\0*.*\0";
	ofn.lpstrFile=szFileName;
	ofn.nMaxFile=MAX_PATH;
	::GetOpenFileName(&ofn);
	*ret = boost::lexical_cast<std::string>(ofn.lpstrFile);
}


std::string ExePath() 
{
    char buffer[MAX_PATH];
    GetModuleFileName( NULL, buffer, MAX_PATH );
	std::string::size_type pos = std::string( buffer ).find_last_of( "\\/" );
    return std::string( buffer ).substr( 0, pos);
}

std::ifstream::pos_type filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::in | std::ifstream::binary);
    in.seekg(0, std::ifstream::end);
    return in.tellg(); 
}

int GetLineCount (const char* filename)
{
	return 0;
	//std::ifstream myfile(filename);
	//if(myfile.is_open())
	//{
	//	myfile.ignore ( 999999, '\n' );

	//	unsigned currentLine = 0;
	//	std::string lineStr;
	//	
	//	while( getline(myfile, lineStr) ) 
	//	{
	//		bool is_empty = true;
	//		for (unsigned int i = 0; i < lineStr.size(); i++) 
	//		{
	//			char ch = lineStr[i];
	//			is_empty = is_empty && isspace(ch);
	//		}
	//		if (!is_empty) 
	//		{
	//			currentLine++;
	//			program->last_line = program->line;
	//		}
	//	}
	//	myfile.close();
	//	&lastline=currentLine;
	//}
	//else 
	//{
	//	color(11);
	//	std::cout << "Unable to open file in void readOnce! - FIX" << std::endl;
	//}
}


















int Compare( const void * Val1, const void * Val2 )
{
    if ( *(PDWORD)Val1 == *(PDWORD)Val2 )
    return 0;

    return *(PDWORD)Val1 > *(PDWORD)Val2 ? 1 : -1;
}

DWORD dWorkingSetPages[ 1024 * 128 ]; // hold the working set 
				// information get from QueryWorkingSet()
DWORD dPageSize = 0x1000;

extern "C" __declspec(dllexport) DWORD CalculateWSPrivate(DWORD processID)
{
    DWORD dSharedPages = 0;
    DWORD dPrivatePages = 0; 
    DWORD dPageTablePages = 0;
 
    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | 
		PROCESS_VM_READ, FALSE, processID );

    if ( !hProcess )
    return 0;

    __try
    { 
        char szBuffer[MAX_PATH * 4]="";

        if ( !QueryWorkingSet(hProcess, dWorkingSetPages, sizeof(dWorkingSetPages)) )
        __leave;

        DWORD dPages = dWorkingSetPages[0];

        qsort( &dWorkingSetPages[1], dPages, sizeof(DWORD), Compare );

        for ( DWORD i = 1; i <= dPages; i++ )
        {
            DWORD dCurrentPageStatus = 0; 
            DWORD dCurrentPageAddress;
            DWORD dNextPageAddress;
            DWORD dNextPageFlags;
            DWORD dPageAddress = dWorkingSetPages[i] & 0xFFFFF000;
            DWORD dPageFlags = dWorkingSetPages[i] & 0x00000FFF;

            while ( i <= dPages ) // iterate all pages
            {
                dCurrentPageStatus++;

                if ( i == dPages ) //if last page
                break;
 
                dCurrentPageAddress = dWorkingSetPages[i] & 0xFFFFF000;
                dNextPageAddress = dWorkingSetPages[i+1] & 0xFFFFF000;
                dNextPageFlags = dWorkingSetPages[i+1] & 0x00000FFF;
 
                //decide whether iterate further or exit
                //(this is non-contiguous page or have different flags) 
                if ( (dNextPageAddress == (dCurrentPageAddress + dPageSize)) 
			&& (dNextPageFlags == dPageFlags) )
                {
                    i++;
                }
                else
                break;
            }
 
            if ( (dPageAddress < 0xC0000000) || (dPageAddress > 0xE0000000) )
            {
                if ( dPageFlags & 0x100 ) // this is shared one
                dSharedPages += dCurrentPageStatus;

                else // private one
                dPrivatePages += dCurrentPageStatus;
            }
            else
            dPageTablePages += dCurrentPageStatus; //page table region 
        } 

        DWORD dTotal = dPages * 4;
        DWORD dShared = dSharedPages * 4;
        DWORD WSPrivate = dTotal - dShared;

        return WSPrivate;
    }
    __finally
    {
        CloseHandle( hProcess );
    }
	return 0;
}