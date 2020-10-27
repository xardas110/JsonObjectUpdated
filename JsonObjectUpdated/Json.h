#pragma once
//For now the only supported format is Ascii2
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <assert.h>
#include <initializer_list>
#include <stack>

class Json
{
private:
	struct JsonArrayWrapper
	{
		JsonArrayWrapper(std::initializer_list<const Json> args);
		std::initializer_list<const Json> jArray;
	};

public:
	enum Type { Null, Bool, Int, Float, String, Array, Object };

	struct Iterator
	{
		Iterator(const Json* obj, std::vector<Json>::const_iterator&& nextArrayEntry);
		Iterator(const Json* container, std::map<std::string, Json>::const_iterator&& nextObjectEntry);
		Iterator* operator++();
		operator size_t() const;
		bool operator!=(const Iterator& other) const;
		Iterator& operator*();
		const std::string& Key() const;
		const Json& Value() const;
	private:
		size_t counter{ 0 };
		const Json* container = nullptr;		
		std::vector<Json>::const_iterator nextArrayEntry;
		std::map<std::string, Json>::const_iterator nextObjectEntry;
	};

	Json(const Json&);
	Json(Json&&) noexcept;
	Json(const Type type = Type::Null);
	Json(const bool bval);
	Json(const int val);
	Json(const float val);
	Json(const char* str);
	Json(const std::string& str);
	template<typename ARG, typename ... R>
	Json(ARG arg, const R& ... rest); //Initializer list works too for array, but since I use it for objects then I cant do the same for arrays cuz of constructor parameters
	
	Json(std::initializer_list<std::pair<const std::string, const Json>> args);
	Json(JsonArrayWrapper args);
	~Json() noexcept;

	Json& operator=(const Json&);
	Json& operator=(Json&&) noexcept;
	Json& operator[](const std::string& key);
	const Json& operator[](const std::string& key) const;
	Json& operator[](const char* key);
	const Json& operator[](const char* key) const;
	bool operator==(Json& other);
	bool operator!=(Json& other);
	bool operator==(const Json& other) const;
	bool operator!=(const Json& other) const;
	const Json& operator[](size_t i) const;
	const Json& operator[](int i) const;
	Json& operator[](size_t i);
	Json& operator[](int i);

	operator bool() const;
	operator int() const;
	operator float() const;
	operator std::string() const;

	Json& Insert(const Json& val, const size_t index);
	Json& Insert(Json&& val, const size_t index);
	Json& Add(const Json& val);
	Json& Add(Json&& val);
	std::vector<std::string> GetKeys() const;
	const Type GetType() const;
	const size_t Size() const;
	Json& Set(const std::string& key, const Json& value);
	bool Contains(const std::string& key) const;
	static Json JObject(std::initializer_list<std::pair<const std::string, const Json>> args);
	static Json JArray(std::initializer_list<const Json> args);
	static size_t FindExt(const std::string& text, const std::string& delimiter);

	void Save(const std::string& path);
	Json Load(const std::string& path);

	void Print() const;
	
	Iterator begin() const;
	Iterator end() const;

	const std::string Stringify();
	static Json Parse(const std::string& js);
	static const bool Compare(const std::vector<Json>& a, const std::vector<Json>& b);
	static const bool Compare(const std::map<std::string, Json>& a, const std::map<std::string, Json>& b);
	
private:
	static void PrintS(std::string& text, size_t& offset, const Json& json);
	static size_t FindFirstNotOf(const std::string& str, std::set<char> del, const bool bAsc);
	void EllipArray(Json& self) {};
	template<typename ARG, typename ... R>
	void EllipArray(Json& self, ARG arg, const R& ... rest);
	struct Var
	{
		Type type = Type::Null;
		union
		{
			bool boolVal;
			int intVal;
			float floatVal;
			std::string* stringVal;
			std::vector <Json>* arrayVal;
			std::map<std::string, Json>* objectVal;
		};
		~Var() noexcept
		{
			switch (type)
			{
			case Json::String:
				delete stringVal;
				break;
			case Json::Array:
				delete arrayVal;
				break;
			case Json::Object:
				delete objectVal;
				break;
			default:
				break;
			}
		}
		Var(const Var&) = delete;
		Var(Var&&) noexcept = delete;
		Var& operator=(const Var&) = delete;
		Var& operator=(Var&&) noexcept = delete;
		Var() = default;

		void Copy(const std::unique_ptr<Var>& src);
		std::string RemoveWS(const std::string & text);
		void ParseAsInt(const std::string& text);
		void ParseAsFloat(const std::string& text);
		void ParseAsObject(const std::string& text);
		void ParseAsArray(const std::string& text);
		std::string ParseKV(const std::string& text, size_t& offset, const char delimiter);

	};
	std::unique_ptr<Var> var_;	
};

template<typename ARG, typename ...R>
inline void Json::EllipArray(Json& self, ARG arg, const R & ...rest)
{
	decltype(arg) val = arg;
	self.Add(val);
	EllipArray(self, rest...);
};

template<typename ARG, typename ...R>
inline Json::Json(ARG arg, const R & ...rest)
	:var_(new Var)
{
	var_->type = Type::Array;
	var_->arrayVal = new std::vector<Json>;
	EllipArray(*this, arg, rest...);
};
