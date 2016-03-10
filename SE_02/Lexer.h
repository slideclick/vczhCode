#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <vector>

namespace script
{
	using namespace std;

	typedef bool(*Lexfp_Char)(wchar_t);

	extern int LexerInt(const wchar_t*& Input);
	extern int LexerReal(const wchar_t*& Input);
	extern int LexerString(const wchar_t*& Input);
	extern int LexerName(const wchar_t*& Input);
	extern int LexerChar(const wchar_t*& Input , Lexfp_Char CallBack);
	extern int LexerChar(const wchar_t*& Input , wchar_t Char);
	extern int LexerChars(const wchar_t*& Input , Lexfp_Char CallBack);
	extern int LexerChars(const wchar_t*& Input , const wchar_t* Chars);

	extern bool lDigit(wchar_t Input);
	extern bool lLetterUnder(wchar_t Input);
	extern bool lLetterUnderDigit(wchar_t Input);
	extern bool lSpace(wchar_t Input);
	extern bool lSpaceCrLf(wchar_t Input);

	enum LexerType
	{
		ltInt,
		ltReal,
		ltString,
		ltName,
		ltColon
	};

	class LexerToken
	{
	public:
		LexerType Type;
		wstring Token;
		const wchar_t* Position;
	};

	class Error
	{
	public:
		const wchar_t* Position;
		wstring Message;
	};

	extern Error Lexer(const wchar_t*& Input , vector<LexerToken>& Tokens , bool StopAtLineBreak);

	extern bool IsInt(const wstring& String);
	extern bool IsReal(const wstring& String);
	extern int ToInt(const wstring& String);
	extern double ToReal(const wstring& String);
	extern wstring ToStr(int Value);
	extern wstring ToStr(double Value);
}