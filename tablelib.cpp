#include <lua.hpp>
#include <set>
#include <string>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <unordered_set>

#define ADD_VALUE(L, type, index, format, str) {\
	char tmp[32] = {0};\
	sprintf(tmp, (format), lua_to##type((L), (index)));\
	(str) += tmp;\
}

#define ADD_INTEGER(L, index, str) {\
	char tmp[24] = {0};\
	char* result = tmp + 23;\
	llong2str(lua_tointeger((L), (index)), result);\
	(str) += result;\
}

const char hex_digits[] = "0123456789ABCDEF";
char * llong2str(long long n, char *& result) {
	unsigned long long abs_n = n;
	if (n < 0)
		abs_n = -n;
	do {
		char c = hex_digits[abs_n & 0xF];
		*(--result) = c;
		abs_n >>= 4;
	} while (abs_n != 0);
	*(--result) = 'X';
	*(--result) = '0';
	if (n < 0)
		*(--result) = '-';
	return result;
}

void table2string(lua_State * L, int index, std::string& str, std::set<const void*>& visited) {
	try {
		lua_pushnil(L);
		str.push_back('{');
		while (lua_next(L, index)) {
			int ktype = lua_type(L, -2);
			str += "[";
			if (ktype == LUA_TNUMBER) {
				if (lua_isinteger(L, -2)) {
					#if defined(_OPEN_HEX_)
						ADD_INTEGER(L, -2, str);
					#else
						ADD_VALUE(L, integer, -2, "%lld", str);
					#endif
				} else if (lua_isnumber(L, -2)) {
					ADD_VALUE(L, number, -2, "%0.6lf", str);
				}
			} else if (ktype == LUA_TSTRING) {
				str.push_back('"');
				str += lua_tostring(L, -2);
				str.push_back('"');
			} else if (ktype == LUA_TBOOLEAN) {
				str += lua_toboolean(L, -2) ? "true" : "false";
			} else if (ktype == LUA_TFUNCTION) {
				ADD_VALUE(L, pointer, -2, "\"%p:function\"", str);
			} else if (ktype == LUA_TUSERDATA) {
				ADD_VALUE(L, pointer, -2, "\"%p:userdata\"", str);
			} else if (ktype == LUA_TTHREAD) {
				ADD_VALUE(L, pointer, -2, "\"%p:thread\"", str);
			} else if (ktype == LUA_TLIGHTUSERDATA) {
				ADD_VALUE(L, pointer, -2, "\"%p:lightuserdata\"", str);
			} else {
				ADD_VALUE(L, pointer, -2, "\"%p:other\"", str);
			}
			str += "]=";

			int vtype = lua_type(L, -1);
			if (vtype == LUA_TTABLE) {
				const void * p = lua_topointer(L, -1);
				if (visited.count(p)) {
					ADD_VALUE(L, pointer, -1, "\"%p\"", str);
				} else {
					visited.insert(lua_topointer(L, -1));
					table2string(L, lua_gettop(L), str, visited);
				}
			} else if (vtype == LUA_TBOOLEAN) {
				bool b = lua_toboolean(L, -1);
				str += b ? "true" : "false";
			} else if (vtype == LUA_TSTRING) {
				str.push_back('"');
				str += lua_tostring(L, -1);
				str.push_back('"');	
			} else if (vtype == LUA_TNUMBER) {
				if (lua_isinteger(L, -1)) {
					#if defined(_OPEN_HEX_)
						ADD_INTEGER(L, -1, str);
					#else
						ADD_VALUE(L, integer, -1, "%lld", str);
					#endif
				} else if (lua_isnumber(L, -1)) {
					ADD_VALUE(L, number, -1, "%0.6lf", str);
				}
			} else if (vtype == LUA_TLIGHTUSERDATA) {
				ADD_VALUE(L, pointer, -1, "\"%p:lightuserdata\"", str);
			} else if (vtype == LUA_TFUNCTION) {
				ADD_VALUE(L, pointer, -1, "\"%p:function\"", str);
			} else if (vtype == LUA_TUSERDATA) {
				ADD_VALUE(L, pointer, -1, "\"%p:userdata\"", str);
			} else if (vtype == LUA_TTHREAD) {
				ADD_VALUE(L, pointer, -1, "\"%p:thread\"", str);
			} else {
				ADD_VALUE(L, pointer, -1, "\"%p:other\"", str);
			}
			str.push_back(',');
			lua_pop(L, 1);
		}
		str.push_back('}');
	}
	catch(const std::exception& e) {
		luaL_error(L, e.what());
	}
}

static int TableToString(lua_State * L) {
	int type = lua_type(L, 1);
	switch (type) {
	case LUA_TNIL:
		lua_pushnil(L);
		break;
	case LUA_TBOOLEAN:
		lua_pushstring(L, lua_toboolean(L, 1) ? "true" : "false");
		break;
	case LUA_TSTRING:
		lua_pushstring(L, lua_tostring(L, 1));
		break;
	case LUA_TNUMBER:
		if (lua_isinteger(L, -1)) {
			lua_pushstring(L, std::to_string(lua_tointeger(L, 1)).c_str());
		} else {
			lua_pushstring(L, std::to_string(lua_tonumber(L, 1)).c_str());
		}
		break;
	case LUA_TTABLE: {
		std::set<const void*> visited;
		visited.insert(lua_topointer(L, 1));

		std::string str;
		int capacity = (int)lua_tointeger(L, 2);
		if (capacity > 0)
			str.reserve(capacity);
		table2string(L, 1, str, visited);
		lua_pushstring(L, str.c_str());
		}
		break;
	default: {
		char buffer[32] = {0};
		sprintf(buffer, "\"%p\"", lua_topointer(L, 1));
		lua_pushstring(L, buffer);
		}
		break;
	}
	return 1;
}

static const struct luaL_Reg methods[] = {
	{ "tostring", TableToString }, 
	{ NULL, NULL }, 
};

extern "C"
int luaopen_tablelib(lua_State * L) {
	luaL_checkversion(L);
	luaL_newlib(L, methods);
	return 1;
}
