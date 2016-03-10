#include "Lexer.h"

namespace script
{
	int LexerInt(const wchar_t*& Input)
	{
		return LexerChars(Input,lDigit);
	}

	int LexerReal(const wchar_t*& Input)
	{
		const wchar_t* Temp=Input;
		int Left=LexerInt(Temp);
		if(Left==0)return 0;
		if(LexerChar(Temp,L'.')==0)return 0;
		int Right=LexerInt(Temp);
		if(Right==0)return 0;
		Input=Temp;
		return Left+Right+1;
	}

	bool __NotDQM(wchar_t Input)
	{
		return Input!=L'\"';
	}

	int LexerString(const wchar_t*& Input)
	{
		const wchar_t* Temp=Input;
		if(LexerChar(Temp,L'\"')==0)return 0;
		int Count=LexerChars(Temp,__NotDQM);
		if(LexerChar(Temp,L'\"')==0)return 0;
		Input=Temp;
		return Count+2;
	}

	int LexerName(const wchar_t*& Input)
	{
		if(LexerChar(Input,lLetterUnder)==0)return 0;
		return 1+LexerChars(Input,lLetterUnderDigit);
	}

	int LexerChar(const wchar_t*& Input , Lexfp_Char CallBack)
	{
		if(CallBack(*Input))
		{
			Input++;
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int LexerChar(const wchar_t*& Input , wchar_t Char)
	{
		if(*Input==Char)
		{
			Input++;
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int LexerChars(const wchar_t*& Input , Lexfp_Char CallBack)
	{
		int Count=0;
		while(CallBack(*Input))
		{
			Input++;
			Count++;
		}
		return Count;
	}

	int LexerChars(const wchar_t*& Input , const wchar_t* Chars)
	{
		const wchar_t* Temp=Input;
		while(*Chars)
		{
			if(*Input++!=*Chars++)return 0;
		}
		Input=Temp;
		return (int)(Chars-Input);
	}

	bool lDigit(wchar_t Input)
	{
		return Input>=L'0' && Input<=L'9';
	}

	bool lLetterUnder(wchar_t Input)
	{
		return (Input>=L'a' && Input<=L'z') || (Input>=L'A' && Input<=L'Z') || (Input==L'_');
	}

	bool lLetterUnderDigit(wchar_t Input)
	{
		return lLetterUnder(Input) || lDigit(Input);
	}

	bool lSpace(wchar_t Input)
	{
		switch(Input)
		{
		case L' ':case L'\t':
			return 1;
		default:
			return 0;
		}
	}

	bool lSpaceCrLf(wchar_t Input)
	{
		switch(Input)
		{
		case L' ':case L'\t':case L'\r':case L'\n':
			return 1;
		default:
			return 0;
		}
	}

	LexerToken MakeToken(const wchar_t* Input , LexerType Type , int Count , int Left , int Right)
	{
		LexerToken Token;
		Token.Type=Type;
		Token.Position=Input-Count;
		Token.Token=wstring(Input-Count+Left,Count-Left-Right);
		return Token;
	}

	Error Lexer(const wchar_t*& Input , vector<LexerToken>& Tokens , bool StopAtLineBreak)
	{
		Error aError;
		aError.Position=0;
		Tokens.clear();
		while(true)
		{
			int Count=0;
			LexerChars(Input,lSpace);
			if(Count=LexerName(Input))
			{
				Tokens.push_back(MakeToken(Input,ltName,Count,0,0));
			}
			else if(Count=LexerString(Input))
			{
				Tokens.push_back(MakeToken(Input,ltString,Count,1,1));
			}
			else if(Count=LexerReal(Input))
			{
				Tokens.push_back(MakeToken(Input,ltReal,Count,0,0));
			}
			else if(Count=LexerInt(Input))
			{
				Tokens.push_back(MakeToken(Input,ltInt,Count,0,0));
			}
			else if(Count=LexerChar(Input,L':'))
			{
				Tokens.push_back(MakeToken(Input,ltColon,Count,0,0));
			}
			else if(Count=LexerChars(Input,lSpaceCrLf))
			{
				if(StopAtLineBreak)break;
			}
			else if(*Input==L'\0')
			{
				break;
			}
			else
			{
				aError.Position=Input;
				aError.Message=wstring(L"遇到不可识别字符：")+(*Input)+L"。";
				break;
			}
		}
		return aError;
	}

	bool IsInt(const wstring& String)
	{
		const wchar_t* Buffer=String.c_str();
		wcstol(Buffer,const_cast<wchar_t**>(&Buffer),10);
		return *Buffer==L'\0';
	}

	bool IsReal(const wstring& String)
	{
		const wchar_t* Buffer=String.c_str();
		wcstod(Buffer,const_cast<wchar_t**>(&Buffer));
		return *Buffer==L'\0';
	}

	int ToInt(const wstring& String)
	{
		const wchar_t* Buffer=String.c_str();
		return wcstol(Buffer,const_cast<wchar_t**>(&Buffer),10);
	}

	double ToReal(const wstring& String)
	{
		const wchar_t* Buffer=String.c_str();
		return wcstod(Buffer,const_cast<wchar_t**>(&Buffer));
	}

	wstring ToStr(int Value)
	{
		wchar_t Buffer[100];
		_itow(Value,Buffer,10);
		return Buffer;
	}

	wstring ToStr(double Value)
	{
		char AnsiBuffer[100];
		wchar_t Buffer[100];
		_gcvt(Value,18,AnsiBuffer);
		mbstowcs(Buffer,AnsiBuffer,100);
		return Buffer;
	}
}