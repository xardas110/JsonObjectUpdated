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
	Json j;
	j = Json::JObject({ {"Ali", 234}, {"abc", 456} });
	j = Json::JArray({ 465,"Ali",456,345.f, {{"Ali",456}} });
	//j = { 345, 4657, 47567657, "ali", {"Ali", 4564} };
	std::cout << j.Stringify();
}

