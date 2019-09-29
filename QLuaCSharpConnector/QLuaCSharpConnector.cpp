#include <Windows.h>

//=== ����������� ��� Lua ��������� ============================================================================//
#define LUA_LIB
#define LUA_BUILD_AS_DLL

//=== ������������ ����� LUA ===================================================================================//
extern "C" {
#include "Lua\lauxlib.h"
#include "Lua\lua.h"
}


//=== �������� ��������� �� ���������� ����������� ������ =====================================================//
// ��� ��� ���������� ������
TCHAR Name1[] = TEXT("MyMemory");
// �������, ��� ����������� � ��� ��������� ������ � ����� ������
HANDLE hFileMapMyMemory = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 256, Name1);


//=== ����������� ����� ����� ��� DLL ==========================================================================//
BOOL APIENTRY DllMain(HANDLE hModule, DWORD  fdwReason, LPVOID lpReserved)
{
	//������� ������� ������������� ���� �������� ��������� fdwReason, ������������� ������� DllMain ��� ��� �������������   
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH: // ����������� DLL          
		break;
	case DLL_PROCESS_DETACH: // ���������� DLL
		break;
	case DLL_THREAD_ATTACH:  // �������� ������ ������
		break;
	case DLL_THREAD_DETACH:  // ���������� ������
		break;
	}
	return TRUE;
}

//=== ���������� �������, ���������� �� LUA ====================================================================//
static int forLua_TestFunc(lua_State *L)
{
	boolean B;
	int I;
	double D;
    const char *S = "";
	// �������� �� ����� ���������� �������� ������
	if (lua_isboolean(L, 1)) B = lua_toboolean(L, 1);
	if (lua_isnumber(L, 2)) I = lua_tointeger(L, 2);
	if (lua_isnumber(L, 3)) D = lua_tonumber(L, 3);
	if (lua_isstring(L, 4)) S = lua_tostring(L, 4);

	// ������� ���� Lua
	lua_settop(L, 0);

	// ��������� � ���� ���������� ����� ��������
	lua_pushboolean(L, B);
	lua_pushinteger(L, I);
	lua_pushnumber(L, D);
	lua_pushstring(L, S);

	// ���������� �������� � ������
	return(1, 2, 3, 4);
}

//=== ���������� �������, ���������� �� LUA ====================================================================//
static int forLua_SendArray(lua_State *L) // �������� ������ �� QLua, ��������� ����� ��������, ���������� ����� � C#
{
	// ���� ��������� �� ������ �������
	if (hFileMapMyMemory)
	{
		// �������� ������ (�������������) ��������������� � ������/������ ����
		PBYTE pb = (PBYTE)(MapViewOfFile(hFileMapMyMemory, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 256));

		// ���� ������ �������
		if (pb != NULL)
		{
			// ���� ������ ������ (���� ������ ��������, ���� C# ������� ������, ���������� ���������)
			if (pb[0] == 0)
			{
				// ���������, ��������-�� ������ ������� ����� �������� (�������� Lua)
				if (lua_istable(L, 1))
				{
					int ArraySum = 0;

					lua_pushnil(L); //������ ����
									//������� lua_next ���������� ��� ���� "����"-"��������" � �������,
									//������ ���������� ����������� ������ � �����, �� �������� ���������� ������ (������� Lua)
					while (lua_next(L, 1) != 0)
					{
						// � ���� "����" ��������� �� ������� -2, "��������" ��������� �� ������� -1
						lua_tonumber(L, -2); // ��� ����� �������� �������� �������� �����
						ArraySum = ArraySum + lua_tonumber(L, -1); // ������� ����� �������� �������
																   // ����������� ���� ��� ��������� ��������
						lua_pop(L, 1);
					}

					char Str[10] = ""; // ������ �������� ��� ���������� ������������� ����� ��������
					_itoa_s(ArraySum, Str, 10); // ������������ ����� � ������

					memcpy(pb, Str, strlen(Str)); // ���������� ������ � ����������� ������
				}
			}

			// ��������� �������������
			UnmapViewOfFile(pb);
			// ��������� ��������� �� ������
			CloseHandle(hFileMapMyMemory);
		}
	}
	return(0);
}

// ���� ������������� ���������� ���������
bool Run = true;

//=== ���������� �������, ���������� �� LUA ====================================================================//
static int forLua_StartSendHi(lua_State *L) // ���������� ��������� ��� C#
{
	// ���� ��������� �� ������ �������
	if (hFileMapMyMemory)
	{
		// �������� ������ (�������������) ��������������� � ������/������ ����
		PBYTE pb = (PBYTE)(MapViewOfFile(hFileMapMyMemory, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 256));

		// ���� ������ �������
		if (pb != NULL)
		{
			// ������� ������ ��� ������ ���������
			for (int i = 0; i < 256; i++)pb[i] = '\0';

			// ����������� ���� ���� Run == true
			while (Run)
			{
				// ���� ������ ������ (���� ������ ��������, ���� C# ������� ������, ���������� ���������)
				if (pb[0] == 0)
				{
					// ���������� ����� ��������� � ������
					char *Str = "������ �� C/C++";
					memcpy(pb, Str, strlen(Str));
				}
				// ����� � 1 �������
				Sleep(1000);
			}

			// ��������� �������������
			UnmapViewOfFile(pb);
			// ��������� ��������� �� ������
			CloseHandle(hFileMapMyMemory);
		}
	}
	return(0);
}

static int forLua_StopSendHi(lua_State *L) // ���������� �������� ��������� ��� C#
{
	Run = false; // ��������� ����

	return(0);
}

// ��� ��� ���������� ������
TCHAR Name[] = TEXT("QUIKCommand");
// �������, ��� ����������� � ��� ��������� ������ � ����� ������
HANDLE hFileMapQUIKCommand = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 256, Name);

static int forLua_GetCommand(lua_State *L)
{
	//���� ��������� �� ������ �������
	if (hFileMapQUIKCommand)
	{
		//�������� ������ � ������ ������
		PBYTE pb = (PBYTE)(MapViewOfFile(hFileMapQUIKCommand, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 256));

		//���� ������ � ������ ������ �������
		if (pb != NULL)
		{
			//���� ������ �����
			if (pb[0] == 0)
			{
				//���������� � Lua-���� ������ ������
				lua_pushstring(L, "");
			}
			else //���� � ������ ���� �������
			{
				//���������� � Lua-���� ���������� �������
				lua_pushstring(L, (char*)(pb));
				//������� ������, ����� �������� �� ��������� �������
				for (int i = 0; i < 256; i++)pb[i] = '\0';
			}

			//��������� �������������
			UnmapViewOfFile(pb);
		}
		else lua_pushstring(L, "");//���� ������ � ������ ������ �� ��� �������, ���������� � Lua-���� ������ ������
	}
	else //��������� �� ������ �� ��� �������
	{
		//���������� � Lua-���� ������ ������
		lua_pushstring(L, "");
	}
	//������� ���������� ���������� �������� �� Lua-����� (������ ������, ��� ���������� �������)
	return(1);
}


// Quote
// ��� ��� ���������� ������
TCHAR NameQuote[] = TEXT("TerminalQuote");
// �������, ��� ����������� � ��� ��������� ������ � ����� ������
HANDLE hFileMapTerminalQuote = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1400, NameQuote);

//��������� �������-�� ����� ��������� ������
static int forLua_CheckGotQuote(lua_State *L)
{
	//���� ��������� �� ������ �������
	if (hFileMapTerminalQuote)
	{
		//�������� ������ � ������ ������
		PBYTE pb = (PBYTE)(MapViewOfFile(hFileMapTerminalQuote, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 1400));

		//���� ������ � ������ ������ �������
		if (pb != NULL)
		{
			//��������� �� ������ ������ (������, ��� ����� ���������� ������)
			if (pb[0] == 0)
			{
				lua_pushboolean(L, true);
			}
			else
			{
				lua_pushboolean(L, false);
			}
			//��������� �������������
			UnmapViewOfFile(pb);
		}
		else lua_pushboolean(L, false);
	}
	else
	{
		lua_pushboolean(L, false);
	}

	return(1);
}

//���������� ����� ��������� �������
static int forLua_SendQuote(lua_State *L)
{
	//���� ��������� �� ������ �������
	if (hFileMapTerminalQuote)
	{
		//�������� ������ � ������ ������
		PBYTE pb = (PBYTE)(MapViewOfFile(hFileMapTerminalQuote, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 1400));

		//���� ������ � ������ ������ �������
		if (pb != NULL)
		{
			//�������� �� Lua-����� ���������� ��������
			const char *Quote = lua_tostring(L, 1);
			int Size = 0;
			//������� ���������� �������� � ������
			for (int i = 0; i < 1400; i++)
			{
				if (Quote[i] == 0)break;
				Size++;
			}
			//���������� ������ � ������
			memcpy(pb, Quote, Size);
			//lua_pushstring(L, (char*)pb);//���������� ��, ��� ���������� (���� �����������������) (����� ����������� ��� �������)
			//��������� �������������
			UnmapViewOfFile(pb);
		}
		else lua_pushstring(L, "");
	}
	else lua_pushstring(L, "");

	return(1);
}

//=== ����������� ������������� � dll �������, ����� ��� ����� "������" ��� Lua ================================//
static struct luaL_reg ls_lib[] = {
	{ "TestFunc", forLua_TestFunc }, // �� ������� Lua ��� ������� ����� ����� �������� ���: QluaCSharpConnector.TestFunc(); ����� ����� ������� ����� ������ ��������
	{ "SendArray", forLua_SendArray },
	{ "StartSendHi", forLua_StartSendHi }, // �� ������� Lua ��� ������� ����� ����� �������� ���: QluaCSharpConnector.StartSendHi(); ����� ����� ������� ����� ������ ��������
	{ "StopSendHi", forLua_StopSendHi }, // ��������������
	{ "GetCommand", forLua_GetCommand },
	{ "CheckGotQuote", forLua_CheckGotQuote },
	{ "SendQuote", forLua_SendQuote },
	{ NULL, NULL }
};

//=== ����������� �������� ����������, �������� � ������� Lua ==================================================//
extern "C" LUALIB_API int luaopen_QluaCSharpConnector(lua_State *L) {
	luaL_openlib(L, "QluaCSharpConnector", ls_lib, 0);
	return 0;
}