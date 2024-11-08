// decodeJson.h - Converts JSON into a map which either contains primitive data, a vector containing primitive data, or a pointer to a new map.

#include <string>
#include <vector>
#include <map>
#include <any>

// Note: Might not be implemented

struct JSON {
	std::map<std::string, std::any> values;
};
