#include <iostream>
#include <assert.h>
#include "Json.h"
#include <ctime>

std::set<char> WHITESPACE_CHARS1{
	' ',
	'\n',
	'\t',
	'\r'
};
int main()
{
	Json j(Json::Type::Object);
	j.Set("Events", Json::Type::Object);
	j["Events"].Set("Shoot", { 3, 4 });
	j["Events"].Set("ShipLocations", Json::Type::Array);
	j["Events"]["ShipLocations"].Add({ 6.f,5.f });
	j["Events"]["ShipLocations"].Add({ 45.f,6.f });
	j["Events"]["ShipLocations"].Add({ 45646.f,435646.f });
	j["Events"]["ShipLocations"].Add({ true,false });
	for (const auto& val: j["Events"]["ShipLocations"])
	{
		float x = val.Value()[0];
		float y = val.Value()[1];
		std::cout << x << " " << y << std::endl;
	}
	j.Save("Table");
	j.Load("Table.json");
	j.Print();
}

