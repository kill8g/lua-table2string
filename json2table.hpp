#include <string>
#include <stack>
#include <map>
#include <set>
#include <vector>
using namespace std;

bool is_space(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool is_digit(char c) {
	return c >= '0' && c <= '9';
}

bool is_alpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// 一个枚举类型，表示json值的类型
enum class json_type {
	JSON_NULL,
	JSON_BOOL,
	JSON_NUMBER,
	JSON_INT,
	JSON_STRING,
	JSON_ARRAY,
	JSON_OBJECT
};

// 一个结构体，表示json值
struct json_value {
	json_type type;
	union {
		bool b;
		lua_Number n;
		lua_Integer i;
		string* s;
		vector<json_value*>* a;
		map<string, json_value*>* o;
	};
};

// 一个类，表示json解析器
class json_parser {
private:
	const char * input; // 输入字符串
	int pos; // 当前位置
	int len; // 字符串长度

	// 一个辅助函数，用于跳过空白字符
	void skip_space() {
		while (pos < len && is_space(input[pos]))
			pos++;
	}

	// 一个辅助函数，用于解析null值
	json_value * parse_null() {
		if (pos + 3 < len && input[pos] == 'n' && input[pos + 1] == 'u' && input[pos + 2] == 'l' && input[pos + 3] == 'l') {
			pos += 4;
			json_value * v = new json_value();
			v->type = json_type::JSON_NULL;
			return v;
		}
		else
			return nullptr;
	}

	// 一个辅助函数，用于解析布尔值
	json_value * parse_bool() {
		if (pos + 3 < len && input[pos] == 't' && input[pos + 1] == 'r' && input[pos + 2] == 'u' && input[pos + 3] == 'e') {
			pos += 4;
			json_value * v = new json_value();
			v->type = json_type::JSON_BOOL;
			v->b = true;
			return v;
		}
		else if (pos + 4 < len && input[pos] == 'f' && input[pos + 1] == 'a' && input[pos + 2] == 'l' && input[pos + 3] == 's' && input[pos + 4] == 'e') {
			pos += 5;
			json_value * v = new json_value();
			v->type = json_type::JSON_BOOL;
			v->b = false;
			return v;
		}
		else
			return nullptr;
	}

	// 一个辅助函数，用于解析数字值
	json_value * parse_number() {
		int start = pos; // 记录数字的起始位置
		bool is_float = false; // 记录数字是否是浮点数
		if (input[pos] == '-') // 跳过负号
			pos++;
		if (input[pos] == '0') // 跳过零
			pos++;
		else { // 跳过非零的整数部分
			while (pos < len && is_digit(input[pos]))
				pos++;
		}
		if (pos < len && input[pos] == '.') { // 跳过小数点和小数部分
			is_float = true;
			pos++;
			while (pos < len && is_digit(input[pos]))
				pos++;
		}
		if (pos < len && (input[pos] == 'e' || input[pos] == 'E')) { // 跳过指数部分
			is_float = true;
			pos++;
			if (pos < len && (input[pos] == '+' || input[pos] == '-')) // 跳过指数符号
				pos++;
			while (pos < len && is_digit(input[pos])) // 跳
				pos++;
		}
		if (pos > start) { // 如果解析到了一个数字
			json_value * v = new json_value();
			string num_str(input + start, pos - start); // 截取数字字符串
			if (is_float) { // 如果是浮点数，转换为double
				v->type = json_type::JSON_NUMBER;
				v->n = stod(num_str);
			}
			else { // 如果是整数，转换为int
				v->type = json_type::JSON_INT;
				v->i = stoi(num_str);
			}
			return v;
		}
		else
			return nullptr;
	}

	// 一个辅助函数，用于解析字符串值
	json_value * parse_string() {
		if (input[pos] == '"') { // 跳过双引号
			pos++;
			int start = pos; // 记录字符串的起始位置
			while (pos < len && input[pos] != '"') // 跳过字符串内容，暂不处理转义字符
				pos++;
			if (pos < len && input[pos] == '"') { // 跳过双引号
				pos++;
				json_value * v = new json_value();
				v->type = json_type::JSON_STRING;
				v->s = new string(input + start, pos - start - 1); // 截取字符串内容
				return v;
			}
			else
				return nullptr;
		}
		else
			return nullptr;
	}

	// 一个辅助函数，用于解析数组值
	json_value * parse_array() {
		if (input[pos] == '[') { // 跳过左中括号
			pos++;
			skip_space(); // 跳过空白字符
			json_value * v = new json_value();
			v->type = json_type::JSON_ARRAY;
			v->a = new vector<json_value*>();
			if (input[pos] == ']') { // 处理空数组的情况
				pos++;
				return v;
			}
			while (true) { // 循环解析数组元素
				json_value * elem = parse_value(); // 解析一个元素值
				if (elem) { // 如果解析成功，将元素添加到数组中
					v->a->push_back(elem);
					skip_space(); // 跳过空白字符
					if (input[pos] == ',') { // 如果遇到逗号，跳过并继续解析下一个元素
						pos++;
						skip_space();
					}
					else if (input[pos] == ']') { // 如果遇到右中括号，结束解析并返回数组值
						pos++;
						return v;
					}
					else { // 如果遇到其他字符，说明格式错误，返回空指针并释放内存
						delete v;
						return nullptr;
					}
				}
				else { // 如果解析失败，返回空指针并释放内存
					delete v;
					return nullptr;
				}
			}
		}
		else {
			return nullptr;
		}
	}

	// 一个辅助函数，用于解析对象值
	json_value * parse_object() {
		if (input[pos] == '{') { // 跳过左大括号
			pos++;
			skip_space(); // 跳过空白字符
			json_value * v = new json_value();
			v->type = json_type::JSON_OBJECT;
			v->o = new map<string, json_value*>();
			if (input[pos] == '}') { // 处理空对象的情况
				pos++;
				return v;
			}
			while (true) { // 循环解析对象成员
				json_value * key = parse_string(); // 解析一个键值，必须是字符串类型
				if (key) { // 如果解析成功，获取键值对应的字符串内容，并释放内存
					string key_str = *(key->s);
					delete key;
					skip_space(); // 跳过空白字符
					if (input[pos] == ':') { // 如果遇到冒号，跳过并继续解析值部分
						pos++;
						skip_space();
						json_value * value = parse_value();
						if (value) { // 如果解析成功，将键值和值添加到对象中
							v->o->insert({ key_str, value });
							skip_space(); // 跳过空白字符
							if (input[pos] == ',') { // 如果遇到逗号，跳过并继续解析下一个成员
								pos++;
								skip_space();
							}
							else if (input[pos] == '}') { // 如果遇到右大括号，结束解析并返回对象值
								pos++;
								return v;
							}
							else { // 如果遇到其他字符，说明格式错误，返回空指针并释放内存
								delete v;
								return nullptr;
							}
						}
						else { // 如果解析失败，返回空指针并释放内存
							delete v;
							return nullptr;
						}
					}
					else { // 如果遇到其他字符，说明格式错误，返回空指针并释放内存
						delete v;
						return nullptr;
					}
				}
				else { // 如果解析失败，返回空指针并释放内存
					delete v;
					return nullptr;
				}
			}
		}
		else
			return nullptr;
	}

	// 一个辅助函数，用于解析任意类型的json值
	json_value * parse_value() {
		skip_space(); // 跳过空白字符
		if (input[pos] == 'n') // 尝试解析null值
			return parse_null();
		else if (input[pos] == 't' || input[pos] == 'f') // 尝试解析布尔值
			return parse_bool();
		else if (input[pos] == '-' || is_digit(input[pos])) // 尝试解析数字值
			return parse_number();
		else if (input[pos] == '"') // 尝试解析字符串值
			return parse_string();
		else if (input[pos] == '[') // 尝试解析数组值
			return parse_array();
		else if (input[pos] == '{') // 尝试解析对象值
			return parse_object();
		else // 无法解析，返回空指针
			return nullptr;
	}

public:
	// 构造函数，初始化输入字符串和位置
	json_parser(const char * input) {
		this->input = input;
		this->pos = 0;
		this->len = (int)strlen(input);
	}

	// 一个辅助函数，用于释放json值占用的内存
	void free_json(json_value * v) {
		if (v == nullptr)
			return;
		switch (v->type) {
		case json_type::JSON_STRING: // 释放字符串内容的内存
			delete v->s;
			break;
		case json_type::JSON_ARRAY: // 释放数组元素的内存，并释放数组本身的内存
			for (auto & elem : *(v->a))
				free_json(elem);
			delete v->a;
			break;
		case json_type::JSON_OBJECT: // 释放对象成员的内存，并释放对象本身的内存
			for (auto & p : *(v->o))
				free_json(p.second);
			delete v->o;
			break;
		default:
			break;
		}
		delete v; // 释放json值本身的内存
	}
	// 解析函数，返回一个json值，如果解析失败，返回空指针
	json_value * parse() {
		return parse_value();
	}

	// 一个辅助函数，用于将json值转换为lua表，并压入栈中
	void json_to_lua(lua_State * L, json_value * v) {
		switch (v->type) {
		case json_type::JSON_NULL: // null值转换为nil
			lua_pushnil(L);
			break;
		case json_type::JSON_BOOL: // 布尔值转换为boolean
			lua_pushboolean(L, v->b);
			break;
		case json_type::JSON_NUMBER: // 数字值转换为number
			lua_pushnumber(L, v->n);
			break;
		case json_type::JSON_INT: // 数字值转换为number
			lua_pushinteger(L, v->i);
			break;
		case json_type::JSON_STRING: // 字符串值转换为string
			lua_pushstring(L, v->s->c_str());
			break;
		case json_type::JSON_ARRAY: // 数组值转换为table，并递归转换每个元素
			lua_newtable(L);
			for (size_t i = 0; i < v->a->size(); i++) {
				json_to_lua(L, v->a->at(i)); // 将元素转换为lua值，并压入栈中
				lua_rawseti(L, -2, i + 1); // 将栈顶的元素设置为表的第i+1个元素，并弹出栈顶的元素
			}
			break;
		case json_type::JSON_OBJECT: // 对象值转换为table，并递归转换每个成员
			lua_newtable(L);
			set<string> intKeys;
			typedef map<string, json_value*> OBJ;
			OBJ & obj = *(v->o);
			const OBJ::iterator & iter = obj.find("_int_keys_");
			if (iter != obj.end()) {
				vector<json_value*> & arr = *(iter->second->a);
				for (auto & member : arr) {
					string & name = *(member->s);
					auto k = obj.find(name);
					if (k != obj.end())
						intKeys.insert(name);
				}
			}
			for (auto & p : *(v->o)) {
				const std::string & name = p.first;
				if (name == "_int_keys_")
					continue;
				if (intKeys.count(name)) {
					lua_Integer i = stoll(name.c_str());
					lua_pushinteger(L, i);
				} else {
					lua_pushstring(L, name.c_str()); // 将键压入栈中
				}
				json_to_lua(L, p.second); // 将值转换为lua值，并压入栈中
				lua_rawset(L, -3); // 将栈顶的两个元素设置为表的一个键值对，并弹出栈顶的两个元素
			}
			break;
		}
	}
};
