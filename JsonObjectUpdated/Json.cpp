#include "Json.h"
#include <iostream>
#include <fstream>

std::set<char> WHITESPACE_CHARS{
	' ',
	'\n',
	'\t',
	'\r'
};

Json::Json(const Json& other)
	:var_(new Var)
{
	var_->Copy(other.var_);
}

Json::Json(Json&& other) noexcept
{
	var_ = std::move(other.var_);
}

Json::Json(const Type type)
	:var_(new Var)
{
	var_->type = type;
	switch (type)
	{
	case Type::String:
		var_->stringVal = new std::string();
		break;
	case Type::Array:
		var_->arrayVal = new std::vector<Json>;
		break;
	case Type::Object:
		var_->objectVal = new std::map<std::string, Json>;
		break;
	default:
		break;
	}
}

Json::Json(const bool bval)
	:var_(new Var)
{
	var_->type = Type::Bool;
	var_->boolVal = bval;
}

Json::Json(const int val)
	:var_(new Var)
{
	var_->type = Type::Int;
	var_->intVal = val;
}

Json::Json(const float val)
	:var_(new Var)
{
	var_->type = Type::Float;
	var_->floatVal = val;
}

Json::Json(const char* str)
	:var_(new Var)
{
	var_->type = Type::String;
	var_->stringVal = new std::string(str);
}

Json::Json(const std::string& str)
	:var_(new Var)
{
	var_->type = Type::String;
	var_->stringVal = new std::string(str);
}

Json::Json(std::initializer_list<std::pair<const std::string, const Json>> args)
{
	*this = JObject(args);
}

Json::Json(JsonArrayWrapper args)
{
	*this = JArray(args.jArray);
}

Json::~Json() noexcept = default;

const bool Json::Compare(const std::vector<Json>& a, const std::vector<Json>& b)
{
	if (a.size() != b.size())
		return false;

	for (size_t i = 0; i < a.size(); i++)
	{
		if (a[i] != b[i])
			return false;
	}
	return true;
}

const bool Json::Compare(const std::map<std::string, Json>& a, const std::map<std::string, Json>& b)
{
	std::set<std::string> keys;
	for (const auto& aPair : a)
		(void)keys.insert(aPair.first);

	for (const auto& bPair : b)
	{
		const auto aVal = keys.find(bPair.first);
		if (aVal == keys.end())
			return false;
		(void)keys.erase(bPair.first);
	}
	if (!keys.empty())
		return false;
	for (auto it = a.begin(); it != a.end(); it++)
	{
		const auto bVal = b.find(it->first);
		if (it->second != bVal->second)
			return false;
	}
	return true;
}

size_t Json::FindFirstNotOf(const std::string& str, std::set<char> del, const bool bAsc)
{
	if (bAsc)
		for (size_t i = 0; i < str.length(); i++)
		{
			const auto it = del.find(str[i]);
			if (it == del.end())
				return i;
		}
	else
		for (size_t i = str.length()-1; i >= 0; i--)
		{
			const auto it = del.find(str[i]);
			if (it == del.end())
				return i;
		}
	return 0;
}

const Json::Type Json::GetType() const
{
	return var_->type;
}

const size_t Json::Size() const
{
	switch (GetType())
	{
	case Type::Array:
		return var_->arrayVal->size();
	case Type::Object:
		return var_->objectVal->size();
	default:
		return 0;
	}
}

Json& Json::Set(const std::string& key, const Json& value)
{
	assert(GetType() == Type::Object);
	auto& objRef = (*var_->objectVal)[key];
	objRef = value;
	return objRef;
}

bool Json::Contains(const std::string& key) const
{
	if (GetType() != Object)
		return false;
	return var_->objectVal->find(key) != var_->objectVal->end();
}

Json Json::JObject(std::initializer_list<std::pair<const std::string, const Json>> args)
{
	Json result(Json::Type::Object);
	for (const auto &arg : args)
	{ 
		result.Set(arg.first, arg.second);
	}
	return result;
}

Json Json::JArray(std::initializer_list<const Json> args)
{
	Json result(Type::Array);
	for (const auto& arg: args)
	{
		result.Add(arg);
	}
	return result;
}

size_t Json::FindExt(const std::string& text, const std::string& delimiter)
{
	for (size_t i = 0; i < text.length(); i++)
	{
		bool bResult = true;
		const auto ch1 = text[i];
		const auto ch2 = delimiter[0];
		if (ch1 != ch2)
			continue;
		for (size_t j = 0, r = i; j < delimiter.length() || r < text.length(); j++, r++)
		{
			const auto chA = delimiter[j];
			const auto chB = text[r];
			if (chA != chB)
			{
				bResult = false;
				i = r-1;
				break;
			}
		}
		if (!bResult)
			continue;
		return i;
	}
	return 0;
}

void Json::Save(const std::string& path)
{
	std::ofstream os;
	std::string newPath = path;
	if (!Json::FindExt(newPath, ".json"))
		newPath += ".json";
	os.open(newPath);
	assert(os.is_open());
	const auto& ref = (*this).Stringify();
	os.write(ref.c_str(), strlen(ref.c_str()));
	os.close();
}

Json Json::Load(const std::string& path)
{
	std::ifstream is;
	is.open(path);
	assert(is.is_open());
	std::string buffer{ "" };
	std::string text{ "" };
	while (!is.eof())
	{
		std::getline(is, buffer);
		text += buffer;
	}
	is.close();
	return Json::Parse(text);
}

void Json::Print() const
{
	std::string Text{ "" }; size_t nextOff{ 2 };
	switch (GetType())
	{
	case Type::Null:
		Text += "null";
		break;
	case Type::Int:
		Text += std::to_string(var_->intVal);
		break;
	case Type::Float:
		Text += std::to_string(var_->floatVal);
		break;
	case Type::Bool:
		Text += var_->boolVal ? "true" : "false";
		break;
	case Type::String:
		Text += '\"' + *var_->stringVal + '\"';
		break;
	case Type::Array:
		Text += "[\n";
		for (const auto& val: *this)
		{ 
			Text += "  ";
			Json::PrintS(Text, nextOff, val.Value());
			Text += ",\n";
		}
		Text.pop_back();
		Text.pop_back();
		Text += "\n]";
		break;
	case Type::Object:
		Text += "{\n";
		for (const auto& val : *this)
		{
			Text += "  \"" + val.Key() + '\"' + ": ";
			Json::PrintS(Text, nextOff, val.Value());
			Text += ",\n";
		}
		Text.pop_back();
		Text.pop_back();
		Text += "\n}";
		break;
	default:
		break;
	}
	std::cout << Text << std::endl;
}

void Json::PrintS(std::string& text, size_t& offset, const Json& json)
{
	size_t off = offset;
	size_t nextOffset = off + 2;
	switch (json.GetType())
	{
	case Type::Null:
		text += "null";
		break;
	case Type::Int:
		text += std::to_string(json.var_->intVal);
		break;
	case Type::Float:
		text += std::to_string(json.var_->floatVal);
		break;
	case Type::Bool:
		text += json.var_->boolVal ? "true" : "false";
		break;
	case Type::String:
		text += '\"' + *json.var_->stringVal + '\"';
		break;
	case Type::Array:
		text += '[';
		for (const auto& val : json)
		{		
			text += '\n';
			for (size_t i = 0; i < off + 2; i++) text.push_back(' ');
			PrintS(text, nextOffset, val.Value()); text += ",";
		}
		text += '\n'; for (size_t i = 0; i < off; i++) text.push_back(' '); text += "]";	
		break;
	case Type::Object:
		text += '{';
		for (auto& val : json)
		{
			text += '\n';
			for (size_t i = 0; i < off+2; i++) text.push_back(' ');
			text += '\"' + val.Key() + '\"' + ": ";
			PrintS(text, nextOffset, val.Value()); text += ',';
		}
		if (text[text.length() - 1] == ',')
			text.pop_back();
		text += '\n'; for (size_t i = 0; i < off; i++) text.push_back(' ');	text += '}';
		break;
	default:
		break;
	}	
}

auto Json::begin() const -> Iterator
{
	if (GetType() == Json::Type::Array)
		return Iterator(this, var_->arrayVal->begin());
	else
		return Iterator(this, var_->objectVal->begin());
}

auto Json::end() const -> Iterator
{
	if (GetType() == Json::Type::Array)
		return Iterator(this, var_->arrayVal->end());
	else
		return Iterator(this, var_->objectVal->end());
}

const std::string Json::Stringify()
{
	std::string result;
	switch (GetType())
	{
	case Type::Null:
		result += "null";
		break;
	case Type::Int:
		for (const auto &ch: std::to_string(var_->intVal))
		{
			result.push_back(ch);
		}	
		break;
	case Type::Float:
		for (const auto& ch : std::to_string(var_->floatVal))
		{
			result.push_back(ch);
		}
		break;
	case Type::Bool:
	{
		for (const auto& ch : std::string(var_->boolVal ? "true":"false"))
		{
			result.push_back(ch);
		}
		break;
	}
	case Type::String:
		result.push_back('\"');
		for (const auto ch : *var_->stringVal)
		{
			result.push_back(ch);
		}
		result.push_back('\"');
		break;
	case Type::Array:
		result.push_back('[');
		for (auto val : *var_->arrayVal)
		{
			const auto encVal = val.Stringify();
			result += encVal;
			result.push_back(',');
		}
		if (result[result.length() - 1] == ',')
			result.pop_back();
		result.push_back(']');
		break;
	case Type::Object:
		result.push_back('{');
		for (auto& obj : *var_->objectVal)
		{
			result += '\"'+obj.first+'\"'+':';
			result += obj.second.Stringify();
			result.push_back(',');
		}
		if (result[result.length()-1] == ',')
			result.pop_back();
		result.push_back('}');
		break;
	default:
		break;
	}
	return result;
}

Json Json::Parse(const std::string& js)
{
	Json result;
	const auto beg = Json::FindFirstNotOf(js, WHITESPACE_CHARS, true);
	const auto end = Json::FindFirstNotOf(js, WHITESPACE_CHARS, false) + 1;
	const std::string fixed(js.begin() + beg, js.begin() + end);
	const auto ch = fixed[0];
	const auto chL = fixed[fixed.length() - 1];

	std::string str{};
	if (fixed.length() > 1)
		str = std::string(fixed.begin() + 1, fixed.end() - 1);
	else str = ch;

	switch (ch)
	{
		case '{': if (chL == '}') result.var_->ParseAsObject(str); break;	
		case '[': if (chL == ']') result.var_->ParseAsArray(str); break;		
		case '"': if (chL == '"' && js.length() > 1) 
			result.var_->type = Type::String; 
			result.var_->stringVal = new std::string(str); break;		
		default:
		if (fixed == "true")
		{
			result.var_->type = Type::Bool;
			result.var_->boolVal = true;
		}
		else if (fixed == "false")
		{
			result.var_->type = Type::Bool;
			result.var_->boolVal = false;
		}
		else if (fixed == "null")
		{
			result.var_->type = Type::Null;
		}
		else if (fixed.find_first_of('.') != UINT32_MAX)
		{
			result.var_->ParseAsFloat(fixed);
		}
		else
			result.var_->ParseAsInt(fixed);
		break;
	}
	return result;
}

Json& Json::operator=(const Json& other)
{
	//assert(GetType() == other.GetType());
	if (this != &other)
	{
		var_.reset(new Var());
		var_->Copy(other.var_);
	}
	return *this;
}

Json& Json::operator=(Json&& other) noexcept
{
	//assert(GetType() == other.GetType());
	if (this != &other)
		var_ = std::move(other.var_);
	return *this;
}

Json& Json::operator[](const std::string& key)
{
	assert(GetType() == Type::Object);
	const auto it = var_->objectVal->find(key);
	const auto end = var_->objectVal->end();

	assert(it != end);
	return it->second;
}

const Json& Json::operator[](const std::string& key) const
{
	assert(GetType() == Type::Object);
	const auto it = var_->objectVal->find(key);
	const auto end = var_->objectVal->end();
	if (it != end)
		return it->second;
}

Json& Json::operator[](const char* key)
{
	return (*this)[std::string(key)];
}

const Json& Json::operator[](const char* key) const
{
	return (*this)[std::string(key)];
}

bool Json::operator==(const Json& other) const
{
	if (GetType() != other.GetType())
		return false;
	switch (GetType())
	{
		case Type::Bool:	return var_->boolVal == other.var_->boolVal;
		case Type::Int:		return var_->intVal == other.var_->intVal;
		case Type::Float:	return var_->floatVal == other.var_->floatVal;
		case Type::String:	return *var_->stringVal == *other.var_->stringVal;
		case Type::Array:	return Compare(*var_->arrayVal, *other.var_->arrayVal);
		case Type::Object:	return Compare(*var_->objectVal, *other.var_->objectVal);
		default:			return true;
	}
}

bool Json::operator!=(const Json& other) const
{
	return !(*this==other);
}

const Json& Json::operator[](size_t i) const
{
	assert(GetType() == Type::Array);
	return (*var_->arrayVal)[i];
}

const Json& Json::operator[](int i) const
{
	return (*this)[(size_t)i];
}

Json& Json::operator[](size_t i)
{
	assert(GetType() == Type::Array);
	return (*var_->arrayVal)[i];
}

Json& Json::operator[](int i)
{
	return (*this)[(size_t)i];
}

Json::operator bool() const
{
	if (GetType() != Type::Bool)
		return false;
	return var_->boolVal;
}

Json::operator int() const
{
	if (GetType() != Type::Int)
		return 0;
	return var_->intVal;
}

Json::operator float() const
{
	if (GetType() != Type::Float)
		return 0.f;
	return var_->floatVal;
}

Json::operator std::string() const
{
	if (GetType() != Type::String)
		return std::string("");
	return *var_->stringVal;
}


bool Json::operator==(Json& other)
{
	if (GetType() != other.GetType())
						return false;
	switch (GetType())
	{
	case Type::Bool:	return var_->boolVal == other.var_->boolVal;
	case Type::Int:		return var_->intVal == other.var_->intVal;
	case Type::Float:	return var_->floatVal == other.var_->floatVal;
	case Type::String:	return *var_->stringVal == *other.var_->stringVal;
	case Type::Array:	return Compare(*var_->arrayVal, *other.var_->arrayVal);
	case Type::Object:	return Compare(*var_->objectVal, *other.var_->objectVal);
	default:			return true;
	}
}

bool Json::operator!=(Json& other)
{
	return !(*this == other);
}

Json& Json::Insert(const Json& val, const size_t index)
{
	assert(GetType() == Type::Array);
	const auto	beg = var_->arrayVal->begin();
	const auto	result = var_->arrayVal->insert(beg + index, val);
	return		*result;
}

Json& Json::Insert(Json&& val, const size_t index)
{
	assert(GetType() == Type::Array);
	const auto	beg = var_->arrayVal->begin();
	auto		result = var_->arrayVal->insert(beg + var_->arrayVal->size(), std::move(val));
	return		*result;
}

Json& Json::Add(const Json& val)
{
	assert(GetType() == Type::Array);
	auto&	result = Insert(val, var_->arrayVal->size());
	return	result;
}

Json& Json::Add(Json&& val)
{
	if (this == &val)
		return Add(val);
	assert(GetType() == Type::Array);
	auto	&result = Insert(std::move(val), var_->arrayVal->size());
	return	result;
}

std::vector<std::string> Json::GetKeys() const
{
	std::vector<std::string> keys;
	if (GetType() == Type::Object)
	{
		for (const auto& key : *var_->objectVal)
			keys.push_back(key.first);		
		return keys;
	}
}

void Json::Var::Copy(const std::unique_ptr<Var>& src)
{
	type = src->type;
	switch (type)
	{
	case Json::Bool:
		boolVal = src->boolVal;
		break;
	case Json::Int:
		intVal = src->intVal;
		break;
	case Json::Float:
		floatVal = src->floatVal;
		break;
	case Json::String:
		stringVal = new std::string(*src->stringVal);
		break;
	case Json::Array:
		arrayVal = new std::vector<Json>;
		arrayVal->reserve(src->arrayVal->size());
		for (const auto& srcElements : *src->arrayVal)
			arrayVal->emplace_back(srcElements);		
		break;
	case Json::Object:
		objectVal = new std::map<std::string, Json>;
		for (const auto& srcElement : *src->objectVal)
			objectVal->insert({srcElement.first, srcElement.second});		
		break;
	default:
		break;
	}
}

std::string Json::Var::RemoveWS(const std::string& text)
{
	std::string result{ "" };
	for (size_t i = 0; i < text.length(); i++)
	{
		const auto& ch = text[i];
		if (WHITESPACE_CHARS.find(ch) == WHITESPACE_CHARS.end())
			continue;
		result += ch;
	}
	return result;
}

void Json::Var::ParseAsInt(const std::string& text)
{
	int val{ 0 }; size_t bNegative = text[0] == '-';
	for (size_t i = bNegative; i < text.length(); i++)
	{
		const auto& ch = text[i];
		assert(ch >= '0' && ch <= '9');
		val *= 10;
		val += ch - 48;
	}
	if (bNegative)
		val *= -1;
	type = Type::Int;
	intVal = val;
}

void Json::Var::ParseAsFloat(const std::string& text)
{
	type = Type::Float;
	floatVal = (float)atof(text.c_str());
}

void Json::Var::ParseAsObject(const std::string& text)
{
	std::map<std::string, Json> result;
	size_t offset{ 0 };
	while (offset < text.length())
	{
		const auto encKey = ParseKV(text, offset, ':');	
		if (encKey.empty())
			return;
		const auto key = Json::Parse(encKey);
		assert(key.GetType() == Type::String);
		const auto encVal = ParseKV(text, offset, ',');
		if (encVal.empty())
			return;
		result[(std::string)key] = Json::Parse(encVal);
	};
	type = Type::Object;
	objectVal = new decltype(result)(result);
}

void Json::Var::ParseAsArray(const std::string& text)
{
	std::vector<Json> result;
	size_t offset{ 0 };
	while (offset< text.length())
	{
		const auto encVal = ParseKV(text, offset, ',');
		if (encVal.empty())
			return;
		result.emplace_back(Json::Parse(encVal));
	}
	type = Type::Array;
	arrayVal = new decltype(result)(result);
}

std::string Json::Var::ParseKV(const std::string& text, size_t& offset, const char delimiter)
{
	std::stack<char> deli;
	std::string encoded;
	std::string encoding(text.begin() + offset, text.end());

	for (const auto cp : encoding)
	{
		encoded.push_back(cp);
		if (!deli.empty() && cp == deli.top())
		{ 
			deli.pop();
			continue;
		}

		switch (cp)
		{
			case '\"':
				deli.push('\"');
			break;
			case '[':
				deli.push(']');
			break;
			case '{':
				deli.push('}');
			break;
			default:			
			break;
		}
		if (cp == delimiter && deli.empty())
			break;	
	}
	assert(deli.empty());

	offset += encoded.length();
	if (encoded.back() == delimiter)
		encoded.pop_back();

	return encoded;
}

Json::Iterator::Iterator(const Json* obj, std::vector<Json>::const_iterator&& nextArrayEntry)
	:container(obj), nextArrayEntry(nextArrayEntry)
{
}
Json::Iterator::Iterator(const Json* obj, std::map<std::string, Json>::const_iterator&& nextObjectEntry)
	: container(obj), nextObjectEntry(nextObjectEntry)
{
}

Json::Iterator* Json::Iterator::operator++()
{
	if (container->GetType() == Type::Array)
	{ 
		nextArrayEntry++;
	}
	else 
	{ 
		nextObjectEntry++;	
	}
	counter++;
	return this;
}

Json::Iterator::operator size_t() const
{
	return counter;
}

bool Json::Iterator::operator!=(const Iterator& other) const
{
	if (container->GetType() == Array)
		return nextArrayEntry != other.nextArrayEntry;
	else
		return nextObjectEntry != other.nextObjectEntry;

}

Json::Iterator& Json::Iterator::operator*()
{
	counter = 0;
	return *this;
}

const std::string& Json::Iterator::Key() const
{
	if (container->GetType() == Type::Array)
		return *nextArrayEntry;
	else
		return nextObjectEntry->first;
}

const Json& Json::Iterator::Value() const
{
	if (container->GetType() == Type::Array)
		return *nextArrayEntry;
	else
		return nextObjectEntry->second;
}

Json::JsonArrayWrapper::JsonArrayWrapper(std::initializer_list<const Json> args)
	:jArray(args)
{
}
