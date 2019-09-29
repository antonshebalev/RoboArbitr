#include <Windows.h>

//=== Необходимые для Lua константы ============================================================================//
#define LUA_LIB
#define LUA_BUILD_AS_DLL

//=== Заголовочные файлы LUA ===================================================================================//
extern "C" {
#include "Lua\lauxlib.h"
#include "Lua\lua.h"
}


//=== Получает указатель на выделенную именованную память =====================================================//
// Имя для выделенной памяти
TCHAR Name1[] = TEXT("MyMemory");
// Создаст, или подключится к уже созданной памяти с таким именем
HANDLE hFileMapMyMemory = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 256, Name1);


//=== Стандартная точка входа для DLL ==========================================================================//
BOOL APIENTRY DllMain(HANDLE hModule, DWORD  fdwReason, LPVOID lpReserved)
{
	//Каждому событию соответствует свое значение аргумента fdwReason, передаваемого функции DllMain при его возникновении   
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH: // Подключение DLL          
		break;
	case DLL_PROCESS_DETACH: // Отключение DLL
		break;
	case DLL_THREAD_ATTACH:  // Создание нового потока
		break;
	case DLL_THREAD_DETACH:  // Завершение потока
		break;
	}
	return TRUE;
}

//=== Реализация функций, вызываемых из LUA ====================================================================//
static int forLua_TestFunc(lua_State *L)
{
	boolean B;
	int I;
	double D;
    const char *S = "";
	// Получает из стека переданные скриптом данные
	if (lua_isboolean(L, 1)) B = lua_toboolean(L, 1);
	if (lua_isnumber(L, 2)) I = lua_tointeger(L, 2);
	if (lua_isnumber(L, 3)) D = lua_tonumber(L, 3);
	if (lua_isstring(L, 4)) S = lua_tostring(L, 4);

	// Очищает стек Lua
	lua_settop(L, 0);

	// Добавляет в стек полученные ранее значения
	lua_pushboolean(L, B);
	lua_pushinteger(L, I);
	lua_pushnumber(L, D);
	lua_pushstring(L, S);

	// Возвращает значения в скрипт
	return(1, 2, 3, 4);
}

//=== Реализация функций, вызываемых из LUA ====================================================================//
static int forLua_SendArray(lua_State *L) // Получает массив из QLua, вычисляет сумму значений, отправляет сумму в C#
{
	// Если указатель на память получен
	if (hFileMapMyMemory)
	{
		// Получает доступ (представление) непосредственно к чтению/записи байт
		PBYTE pb = (PBYTE)(MapViewOfFile(hFileMapMyMemory, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 256));

		// Если доступ получен
		if (pb != NULL)
		{
			// Если запись пустая (либо первая отправка, либо C# очистил память, подтвердив получение)
			if (pb[0] == 0)
			{
				// Проверяет, является-ли первый элемент стека массивом (таблицей Lua)
				if (lua_istable(L, 1))
				{
					int ArraySum = 0;

					lua_pushnil(L); //Первый ключ
									//Функция lua_next перебирает все пары "ключ"-"значение" в таблице,
									//вторым параметром указывается индекс в стеке, по которому расположен массив (таблица Lua)
					while (lua_next(L, 1) != 0)
					{
						// в паре "ключ" находится по индексу -2, "значение" находится по индексу -1
						lua_tonumber(L, -2); // Так можно получить числовое значение ключа
						ArraySum = ArraySum + lua_tonumber(L, -1); // Считает сумму значений массива
																   // освобождает стек для следующей итерации
						lua_pop(L, 1);
					}

					char Str[10] = ""; // массив символов для строкового представления суммы значений
					_itoa_s(ArraySum, Str, 10); // конвертирует число в строку

					memcpy(pb, Str, strlen(Str)); // записывает строку в именованную память
				}
			}

			// Закрывает представление
			UnmapViewOfFile(pb);
			// Закрывает указатель на память
			CloseHandle(hFileMapMyMemory);
		}
	}
	return(0);
}

// Флаг необходимости отправлять сообщение
bool Run = true;

//=== Реализация функций, вызываемых из LUA ====================================================================//
static int forLua_StartSendHi(lua_State *L) // Отправляет сообщения для C#
{
	// Если указатель на память получен
	if (hFileMapMyMemory)
	{
		// Получает доступ (представление) непосредственно к чтению/записи байт
		PBYTE pb = (PBYTE)(MapViewOfFile(hFileMapMyMemory, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 256));

		// Если доступ получен
		if (pb != NULL)
		{
			// Очищает память при первом обращении
			for (int i = 0; i < 256; i++)pb[i] = '\0';

			// Бесконечный цикл пока Run == true
			while (Run)
			{
				// Если запись пустая (либо первая отправка, либо C# очистил память, подтвердив получение)
				if (pb[0] == 0)
				{
					// Записывает текст сообщения в память
					char *Str = "Привет из C/C++";
					memcpy(pb, Str, strlen(Str));
				}
				// Пауза в 1 секунду
				Sleep(1000);
			}

			// Закрывает представление
			UnmapViewOfFile(pb);
			// Закрывает указатель на память
			CloseHandle(hFileMapMyMemory);
		}
	}
	return(0);
}

static int forLua_StopSendHi(lua_State *L) // Прекращает отправку сообщений для C#
{
	Run = false; // Выключает флаг

	return(0);
}

// Имя для выделенной памяти
TCHAR Name[] = TEXT("QUIKCommand");
// Создаст, или подключится к уже созданной памяти с таким именем
HANDLE hFileMapQUIKCommand = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 256, Name);

static int forLua_GetCommand(lua_State *L)
{
	//Если указатель на память получен
	if (hFileMapQUIKCommand)
	{
		//Получает доступ к байтам памяти
		PBYTE pb = (PBYTE)(MapViewOfFile(hFileMapQUIKCommand, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 256));

		//Если доступ к байтам памяти получен
		if (pb != NULL)
		{
			//Если память чиста
			if (pb[0] == 0)
			{
				//Записывает в Lua-стек пустую строку
				lua_pushstring(L, "");
			}
			else //Если в памяти есть команда
			{
				//Записывает в Lua-стек полученную команду
				lua_pushstring(L, (char*)(pb));
				//Стирает запись, чтобы повторно не выполнить команду
				for (int i = 0; i < 256; i++)pb[i] = '\0';
			}

			//Закрывает представление
			UnmapViewOfFile(pb);
		}
		else lua_pushstring(L, "");//Если доступ к байтам памяти не был получен, записывает в Lua-стек пустую строку
	}
	else //Указатель на память не был получен
	{
		//Записывает в Lua-стек пустую строку
		lua_pushstring(L, "");
	}
	//Функция возвращает записанное значение из Lua-стека (пустую строку, или полученную команду)
	return(1);
}


// Quote
// Имя для выделенной памяти
TCHAR NameQuote[] = TEXT("TerminalQuote");
// Создаст, или подключится к уже созданной памяти с таким именем
HANDLE hFileMapTerminalQuote = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1400, NameQuote);

//Проверяет получил-ли робот последний СТАКАН
static int forLua_CheckGotQuote(lua_State *L)
{
	//Если указатель на память получен
	if (hFileMapTerminalQuote)
	{
		//Получает доступ к байтам памяти
		PBYTE pb = (PBYTE)(MapViewOfFile(hFileMapTerminalQuote, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 1400));

		//Если доступ к байтам памяти получен
		if (pb != NULL)
		{
			//проверяет на пустую запись (сигнал, что можно отправлять стакан)
			if (pb[0] == 0)
			{
				lua_pushboolean(L, true);
			}
			else
			{
				lua_pushboolean(L, false);
			}
			//закрывает представление
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

//Отправляет новые изменения стакана
static int forLua_SendQuote(lua_State *L)
{
	//Если указатель на память получен
	if (hFileMapTerminalQuote)
	{
		//Получает доступ к байтам памяти
		PBYTE pb = (PBYTE)(MapViewOfFile(hFileMapTerminalQuote, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 1400));

		//Если доступ к байтам памяти получен
		if (pb != NULL)
		{
			//Получает из Lua-стека переданное значение
			const char *Quote = lua_tostring(L, 1);
			int Size = 0;
			//считает количество символов в строке
			for (int i = 0; i < 1400; i++)
			{
				if (Quote[i] == 0)break;
				Size++;
			}
			//записывает стакан в память
			memcpy(pb, Quote, Size);
			//lua_pushstring(L, (char*)pb);//возвращает то, что записалось (если раскомментировать) (может пригодиться при отладке)
			//закрывает представление
			UnmapViewOfFile(pb);
		}
		else lua_pushstring(L, "");
	}
	else lua_pushstring(L, "");

	return(1);
}

//=== Регистрация реализованных в dll функций, чтобы они стали "видимы" для Lua ================================//
static struct luaL_reg ls_lib[] = {
	{ "TestFunc", forLua_TestFunc }, // из скрипта Lua эту функцию можно будет вызывать так: QluaCSharpConnector.TestFunc(); здесь можно указать любое другое название
	{ "SendArray", forLua_SendArray },
	{ "StartSendHi", forLua_StartSendHi }, // из скрипта Lua эту функцию можно будет вызывать так: QluaCSharpConnector.StartSendHi(); здесь можно указать любое другое название
	{ "StopSendHi", forLua_StopSendHi }, // соответственно
	{ "GetCommand", forLua_GetCommand },
	{ "CheckGotQuote", forLua_CheckGotQuote },
	{ "SendQuote", forLua_SendQuote },
	{ NULL, NULL }
};

//=== Регистрация названия библиотеки, видимого в скрипте Lua ==================================================//
extern "C" LUALIB_API int luaopen_QluaCSharpConnector(lua_State *L) {
	luaL_openlib(L, "QluaCSharpConnector", ls_lib, 0);
	return 0;
}