#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <conio.h>
#include "ScriptCommand.h"

using namespace std;
using namespace script;

class ConsolePlugin : public Plugin
{
public:
	PluginStatus Execute(const Command& aCommand , Environment& aEnvironment , wstring& ErrorMessage)
	{
		if(aCommand.Name==L"write")
		{
			for(vector<LexerToken>::const_iterator i=aCommand.Parameters.begin();i!=aCommand.Parameters.end();i++)
			{
				if(i->Type==ltName)
				{
					if(aEnvironment.HasVariable(i->Token))
					{
						wcout<<aEnvironment.GetVariable(i->Token);
					}
					else
					{
						ErrorMessage=L"变量"+i->Token+L"不存在。";
						return psFail;
					}
				}
				else
				{
					wcout<<i->Token;
				}
			}
			return psSuccess;
		}
		else if(aCommand.Name==L"writeln")
		{
			for(vector<LexerToken>::const_iterator i=aCommand.Parameters.begin();i!=aCommand.Parameters.end();i++)
			{
				if(i->Type==ltName)
				{
					if(aEnvironment.HasVariable(i->Token))
					{
						wcout<<aEnvironment.GetVariable(i->Token);
					}
					else
					{
						ErrorMessage=L"变量"+i->Token+L"不存在。";
						return psFail;
					}
				}
				else
				{
					wcout<<i->Token;
				}
			}
			wcout<<endl;
			return psSuccess;
		}
		else if(aCommand.Name==L"read")
		{
			for(vector<LexerToken>::const_iterator i=aCommand.Parameters.begin();i!=aCommand.Parameters.end();i++)
			{
				if(i->Type==ltName)
				{
					wstring Value;
					wcin>>Value;
					aEnvironment.SetVariable(i->Token,Value);
				}
				else
				{
					ErrorMessage=i->Token+L"不是变量。";
					return psFail;
				}
			}
			return psSuccess;
		}
		else
		{
			return psGiveUp;
		}
	}
} PluginObject;

int wmain(int argc , wchar_t* argv[])
{
	{
		size_t len=wcslen(argv[0]);
		argv[0][len-3]=L't';
		argv[0][len-2]=L'x';
		argv[0][len-1]=L't';
	}
	setlocale(LC_ALL,"Chinese-simplified");
	wstring Code;
	{
		FILE* f=_wfopen(argv[0],L"rb");
		fpos_t fsize;
		size_t size;
		fseek(f,0,SEEK_END);
		fgetpos(f,&fsize);
		size=(size_t)fsize;
		fseek(f,0,SEEK_SET);
		char* AnsiBuffer=new char[size+1];
		fread(AnsiBuffer,1,size,f);
		AnsiBuffer[size]='\0';
		fclose(f);

		size_t wsize=mbstowcs(0,AnsiBuffer,size);
		wchar_t* Buffer=new wchar_t[wsize+1];
		mbstowcs(Buffer,AnsiBuffer,size);
		Code=Buffer;

		delete[] Buffer;
		delete[] AnsiBuffer;
	}
	Error aError;
	Script aScript;
	aScript.InstallPlugin(&PluginObject);
	aError=aScript.Load(Code);
	if(aError.Position)
	{
		wcout<<aError.Message<<endl;
		wcout<<aError.Position<<endl;
	}
	else
	{
		Environment aEnvironment;
		aError=aScript.Run(aEnvironment);
		if(aError.Position)
		{
			wcout<<aError.Message<<endl;
			wcout<<aError.Position<<endl;
		}
	}
	_getch();
	return 0;
}