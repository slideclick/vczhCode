#include "Lexer.h"
#include <map>

/*
label:
goto label
if value1 [opcode value2] goto label
	opcode=is is_not greater_than less_then greater_equal less_equal
set name value
opcode name value1 value2
	opcode=add minus mul div idiv mod
exit
*/

namespace script
{
	enum CommandType
	{
		ctCommand,
		ctLabel
	};

	class Command
	{
	public:
		CommandType Type;
		wstring Name;
		const wchar_t* Position;
		vector<LexerToken> Parameters;
	};

	extern Error GetCommand(const wchar_t*& Input , Command& aCommand);

	class Environment
	{
	protected:
		map<wstring , wstring> FVariables;
	public:
		Environment();
		~Environment();

		void Initialize();
		bool HasVariable(const wstring& Name);
		wstring GetVariable(const wstring& Name);
		void SetVariable(const wstring& Name , const wstring& Value);
		void GetVariableNames(vector<wstring>& Names);
	};

	enum PluginStatus
	{
		psSuccess,
		psFail,
		psGiveUp
	};

	class Plugin
	{
	public:
		virtual PluginStatus Execute(const Command& aCommand , Environment& aEnvironment , wstring& ErrorMessage)=0;
	};

	class Script
	{
	protected:
		vector<Plugin*> FPlugins;
		vector<Command> FCommands;
		map<wstring , size_t> FLabels;
		wstring FScript;
	public:
		Script();
		~Script();

		void InstallPlugin(Plugin* aPlugin);
		void RemovePlugin(Plugin* aPlugin);
		const wstring& GetScript();
		Error Load(const wstring& Script);
		Error Run(Environment& aEnvironment);
	};
}