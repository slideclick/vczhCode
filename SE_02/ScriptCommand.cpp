#include "ScriptCommand.h"
#include <algorithm>

namespace script
{
	Error GetCommand(const wchar_t*& Input , Command& aCommand)
	{
		aCommand.Position=Input;
		vector<LexerToken> Tokens;
		Error aError=Lexer(Input,Tokens,true);
		if(aError.Position)return aError;
		aCommand.Parameters.clear();

		if(Tokens.size()==0)
		{
			aError.Position=Input;
			aError.Message=L"缺少字符。";
			return aError;
		}
		else if(Tokens[0].Type!=ltName)
		{
			aError.Position=Tokens[0].Position;
			aError.Message=Tokens[0].Token+L"不是一个合法的名称。";
			return aError;
		}
		aCommand.Name=Tokens[0].Token;

		if(Tokens.size()==1)
		{
			aCommand.Type=ctCommand;
		}
		else if(Tokens[1].Type==ltColon)
		{
			aCommand.Type=ctLabel;
		}
		else
		{
			aCommand.Type=ctCommand;
			size_t Index=1;

			while(Index<Tokens.size())
			{
				switch(Tokens[Index].Type)
				{
				case ltInt:case ltReal:case ltString:case ltName:
					aCommand.Parameters.push_back(Tokens[Index]);
					break;
				default:
					aError.Position=Tokens[Index].Position;
					aError.Message=Tokens[Index].Token+L"不是一个合法的参数。";
					return aError;
				}
				Index++;
			}
		}

		return aError;
	}

	/******************************************************************************************/

	Environment::Environment()
	{
	}

	Environment::~Environment()
	{
	}

	void Environment::Initialize()
	{
		FVariables.clear();
	}

	bool Environment::HasVariable(const wstring& Name)
	{
		return FVariables.find(Name)!=FVariables.end();
	}

	wstring Environment::GetVariable(const wstring& Name)
	{
		map<wstring , wstring>::iterator i=FVariables.find(Name);
		return i->second;
	}

	void Environment::SetVariable(const wstring& Name , const wstring& Value)
	{
		map<wstring , wstring>::iterator i=FVariables.find(Name);
		if(i==FVariables.end())
		{
			FVariables.insert(pair<wstring , wstring>(Name,Value));
		}
		else
		{
			i->second=Value;
		}
	}

	void Environment::GetVariableNames(vector<wstring>& Names)
	{
		Names.clear();
		for(map<wstring , wstring>::iterator i=FVariables.begin();i!=FVariables.end();i++)
		{
			Names.push_back(i->first);
		}
	}

	/******************************************************************************************/

	Script::Script()
	{
	}

	Script::~Script()
	{
	}

	void Script::InstallPlugin(Plugin* aPlugin)
	{
		vector<Plugin*>::iterator i=find(FPlugins.begin(),FPlugins.end(),aPlugin);
		if(i==FPlugins.end())
		{
			FPlugins.push_back(aPlugin);
		}
	}

	void Script::RemovePlugin(Plugin* aPlugin)
	{
		remove(FPlugins.begin(),FPlugins.end(),aPlugin);
	}

	const wstring& Script::GetScript()
	{
		return FScript;
	}

	Error Script::Load(const wstring& Script)
	{
		Error aError;
		aError.Position=0;
		FScript=Script;
		FCommands.clear();
		FLabels.clear();
		const wchar_t* Input=Script.c_str();
		while(true)
		{
			LexerChars(Input,lSpaceCrLf);
			if(*Input)
			{
				Command aCommand;
				aError=GetCommand(Input,aCommand);
				if(aError.Position)
				{
					break;
				}
				else if(aCommand.Type==ctLabel)
				{
					map<wstring , size_t>::iterator i=FLabels.find(aCommand.Name);
					if(i==FLabels.end())
					{
						FLabels.insert(pair<wstring , size_t>(aCommand.Name,FCommands.size()));
					}
					else
					{
						aError.Position=aCommand.Position;
						aError.Message=aCommand.Name+L"标号已经存在。";
					}
				}
				else
				{
					FCommands.push_back(aCommand);
				}
			}
			else
			{
				break;
			}
		}
		return aError;
	}

	Error Script::Run(Environment& aEnvironment)
	{
#define __GetObject(Variable,Index)								\
	do{															\
		if(aCommand.Parameters[Index].Type==ltName)				\
		{														\
			Variable=aCommand.Parameters[Index].Token;			\
			if(aEnvironment.HasVariable(Variable))				\
			{													\
				Variable=aEnvironment.GetVariable(Variable);	\
			}													\
			else												\
			{													\
				aError.Position=aCommand.Position;				\
				aError.Message=L"变量"+Variable+L"不存在。";	\
				return aError;									\
			}													\
		}														\
		else													\
		{														\
			Variable=aCommand.Parameters[Index].Token;			\
		}														\
	}while(false)

		Error aError;
		aError.Position=0;
		size_t CommandIndex=0;
		while(CommandIndex<FCommands.size())
		{
			size_t NextCommandIndex=CommandIndex+1;
			Command& aCommand=FCommands[CommandIndex];
			if(aCommand.Name==L"exit")
			{
				if(aCommand.Parameters.size()!=0)
				{
					aError.Position=aCommand.Position;
					aError.Message=L"exit命令不能有参数。";
					return aError;
				}
				else
				{
					break;
				}
			}
			else if(aCommand.Name==L"goto")
			{
				if(aCommand.Parameters.size()!=1)
				{
					aError.Position=aCommand.Position;
					aError.Message=L"goto命令需要一个跳转标号作为参数。";
					return aError;
				}
				else if(aCommand.Parameters[0].Type!=ltName)
				{
					aError.Position=aCommand.Position;
					aError.Message=L"goto命令需要一个跳转标号作为参数。";
					return aError;
				}
				else
				{
					map<wstring , size_t>::iterator i=FLabels.find(aCommand.Parameters[0].Token);
					if(i==FLabels.end())
					{
						aError.Position=aCommand.Position;
						aError.Message=L"标号"+aCommand.Parameters[0].Token+L"不存在。";
						return aError;
					}
					else
					{
						NextCommandIndex=i->second;
					}
				}
			}
			else if(aCommand.Name==L"set")
			{
				if(aCommand.Parameters.size()!=2)
				{
					aError.Position=aCommand.Position;
					aError.Message=L"set命令需要一个变量名和一个对象作为参数。";
					return aError;
				}
				else if(aCommand.Parameters[0].Type!=ltName)
				{
					aError.Position=aCommand.Position;
					aError.Message=L"set命令需要一个变量名和一个对象作为参数。";
					return aError;
				}
				else
				{
					wstring Value;
					__GetObject(Value,1);
					aEnvironment.SetVariable(aCommand.Parameters[0].Token,Value);
				}
			}
			else if(aCommand.Name==L"add" || aCommand.Name==L"minus" || aCommand.Name==L"mul" || aCommand.Name==L"div")
			{
				if(aCommand.Parameters.size()!=3)
				{
					aError.Position=aCommand.Position;
					aError.Message=L"add命令需要一个变量名和两个操作数作为参数。";
					return aError;
				}
				else if(aCommand.Parameters[0].Type!=ltName)
				{
					aError.Position=aCommand.Position;
					aError.Message=L"add命令需要一个变量名和两个操作数作为参数。";
					return aError;
				}
				else
				{
					wstring Param1;
					wstring Param2;
					__GetObject(Param1,1);
					__GetObject(Param2,2);
					if(IsReal(Param1))
					{
						if(IsReal(Param2))
						{
							if(ToReal(Param2)==0.0)
							{
								aError.Position=aCommand.Position;
								aError.Message=L"除数是0。";
								return aError;
							}
							else if(aCommand.Name==L"add")
							{
								if(IsInt(Param1) && IsInt(Param2))
								{
									aEnvironment.SetVariable(aCommand.Parameters[0].Token,ToStr(ToInt(Param1)+ToInt(Param2)));
								}
								else
								{
									aEnvironment.SetVariable(aCommand.Parameters[0].Token,ToStr(ToReal(Param1)+ToReal(Param2)));
								}
							}
							else if(aCommand.Name==L"minus")
							{
								if(IsInt(Param1) && IsInt(Param2))
								{
									aEnvironment.SetVariable(aCommand.Parameters[0].Token,ToStr(ToInt(Param1)-ToInt(Param2)));
								}
								else
								{
									aEnvironment.SetVariable(aCommand.Parameters[0].Token,ToStr(ToReal(Param1)-ToReal(Param2)));
								}
							}
							else if(aCommand.Name==L"mul")
							{
								if(IsInt(Param1) && IsInt(Param2))
								{
									aEnvironment.SetVariable(aCommand.Parameters[0].Token,ToStr(ToInt(Param1)*ToInt(Param2)));
								}
								else
								{
									aEnvironment.SetVariable(aCommand.Parameters[0].Token,ToStr(ToReal(Param1)*ToReal(Param2)));
								}
							}
							else if(aCommand.Name==L"div")
							{
								aEnvironment.SetVariable(aCommand.Parameters[0].Token,ToStr(ToReal(Param1)/ToReal(Param2)));
							}
						}
						else
						{
							aError.Position=aCommand.Position;
							aError.Message=Param1+L"不是数值。";
							return aError;
						}
					}
					else
					{
						aError.Position=aCommand.Position;
						aError.Message=Param1+L"不是数值。";
						return aError;
					}
				}
			}
			else if(aCommand.Name==L"idiv" || aCommand.Name==L"mod")
			{
				if(aCommand.Parameters.size()!=3)
				{
					aError.Position=aCommand.Position;
					aError.Message=L"add命令需要一个变量名和两个操作数作为参数。";
					return aError;
				}
				else if(aCommand.Parameters[0].Type!=ltName)
				{
					aError.Position=aCommand.Position;
					aError.Message=L"add命令需要一个变量名和两个操作数作为参数。";
					return aError;
				}
				else
				{
					wstring Param1;
					wstring Param2;
					__GetObject(Param1,1);
					__GetObject(Param2,2);
					if(IsInt(Param1))
					{
						if(IsInt(Param2))
						{
							if(ToInt(Param2)==0)
							{
								aError.Position=aCommand.Position;
								aError.Message=L"除数是0。";
								return aError;
							}
							else if(aCommand.Name==L"idiv")
							{
								aEnvironment.SetVariable(aCommand.Parameters[0].Token,ToStr(ToInt(Param1)/ToInt(Param2)));
							}
							else if(aCommand.Name==L"mod")
							{
								aEnvironment.SetVariable(aCommand.Parameters[0].Token,ToStr(ToInt(Param1)%ToInt(Param2)));
							}
						}
						else
						{
							aError.Position=aCommand.Position;
							aError.Message=Param1+L"不是整数。";
							return aError;
						}
					}
					else
					{
						aError.Position=aCommand.Position;
						aError.Message=Param1+L"不是整数。";
						return aError;
					}
				}
			}
			else if(aCommand.Name==L"if")
			{
				wstring Goto;
				bool WantGoto=false;
				if(aCommand.Parameters.size()==3)
				{
					wstring Param1;
					__GetObject(Param1,0);
					if(IsInt(Param1))
					{
						if(aCommand.Parameters[1].Type==ltName && aCommand.Parameters[2].Type==ltName && aCommand.Parameters[1].Token==L"goto")
						{
							Goto=aCommand.Parameters[2].Token;
							WantGoto=ToInt(Param1)!=0;
						}
						else
						{
							aError.Position=aCommand.Position;
							aError.Message=L"if命令的格式是if value1 [opcode value2] goto label。";
							return aError;
						}
					}
					else
					{
						aError.Position=aCommand.Position;
						aError.Message=Param1+L"不是整数。";
						return aError;
					}
				}
				else if(aCommand.Parameters.size()==5)
				{
					wstring Param1;
					wstring Param2;
					__GetObject(Param1,0);
					__GetObject(Param2,2);
					if(IsReal(Param1))
					{
						if(IsReal(Param2))
						{
							if(aCommand.Parameters[1].Type==ltName)
							{
								if(aCommand.Parameters[1].Token==L"is")
								{
									WantGoto=ToReal(Param1)==ToReal(Param2);
								}
								else if(aCommand.Parameters[1].Token==L"is_not")
								{
									WantGoto=ToReal(Param1)!=ToReal(Param2);
								}
								else if(aCommand.Parameters[1].Token==L"greater_than")
								{
									WantGoto=ToReal(Param1)>ToReal(Param2);
								}
								else if(aCommand.Parameters[1].Token==L"less_then")
								{
									WantGoto=ToReal(Param1)<ToReal(Param2);
								}
								else if(aCommand.Parameters[1].Token==L"greater_equal")
								{
									WantGoto=ToReal(Param1)>=ToReal(Param2);
								}
								else if(aCommand.Parameters[1].Token==L"less_equal")
								{
									WantGoto=ToReal(Param1)<=ToReal(Param2);
								}
								else
								{
									aError.Position=aCommand.Position;
									aError.Message=L"if命令支持的比较操作有is、is_not、greater_than、less_then、greater_equal和less_equal。";
									return aError;
								}
							}
							else
							{
								aError.Position=aCommand.Position;
								aError.Message=L"if命令支持的比较操作有is、is_not、greater_than、less_then、greater_equal和less_equal。";
								return aError;
							}
							if(WantGoto)
							{
								if(aCommand.Parameters[3].Type==ltName && aCommand.Parameters[4].Type==ltName && aCommand.Parameters[3].Token==L"goto")
								{
									Goto=aCommand.Parameters[4].Token;
								}
								else
								{
									aError.Position=aCommand.Position;
									aError.Message=L"if命令的格式是if value1 [opcode value2] goto label。";
									return aError;
								}
							}
						}
						else
						{
							aError.Position=aCommand.Position;
							aError.Message=Param2+L"不是整数。";
							return aError;
						}
					}
					else
					{
						aError.Position=aCommand.Position;
						aError.Message=Param1+L"不是整数。";
						return aError;
					}
				}
				else
				{
					aError.Position=aCommand.Position;
					aError.Message=L"if命令的格式是if value1 [opcode value2] goto label。";
					return aError;
				}
				if(WantGoto)
				{
					map<wstring , size_t>::iterator i=FLabels.find(Goto);
					if(i==FLabels.end())
					{
						aError.Position=aCommand.Position;
						aError.Message=L"标号"+Goto+L"不存在。";
						return aError;
					}
					else
					{
						NextCommandIndex=i->second;
					}
				}
			}
			else
			{
				bool Done=false;
				for(vector<Plugin*>::iterator i=FPlugins.begin();i!=FPlugins.end();i++)
				{
					switch((*i)->Execute(aCommand,aEnvironment,aError.Message))
					{
					case psSuccess:
						Done=true;
						break;
					case psFail:
						aError.Position=aCommand.Position;
						return aError;
					}
				}
				if(!Done)
				{
					aError.Position=aCommand.Position;
					aError.Message=L"命令"+aCommand.Name+L"不可识别。";
					return aError;
				}
			}
			CommandIndex=NextCommandIndex;
		}
		return aError;

#undef __GetObject
	}
}