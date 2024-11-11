#include "decodeJSON.h"

void printVector(std::vector<std::any> vec, int indentationLevel, int level = 0) {
	// Opening brackets;
	std::cout << '[';
	if (vec.size()) {  // Check if there is anything inside the vector.
		for (int i = 0; i < vec.size(); ++i) {
			displayValue(vec.at(i), indentationLevel, level + 1);
			if (i != vec.size() - 1) {
				std::cout << ", ";
			}
		}
	}
	std::cout << ']';
}

void displayValue(std::any val, int indentationLevel, int level = 0) {
	// First, determine the type of cast.
	std::string typeName = val.type().name();
	// Then, determine whether it is a primitive, a vector, or a JSON. This can easily be done by checking 
	// the length of the string, which will always prove correct assuming valid values.
	switch (typeName.size()) {
	case(3):  // "int" has 3 characters.
		std::cout << std::any_cast<int>(val);
		break;
	case(4):  // "char" has 4 characters.
		std::cout << std::any_cast<char>(val);
		break;
	case(6):  // "double" has 6 characters.
		std::cout << std::any_cast<double>(val);
		break;
	case(87):  // "class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >" (yes, that is the type returned) has 87 characters (including spaces).
		std::cout << std::any_cast<std::string>(val);
		break;
	case(12):  // "struct CPPSON" has 12 characters (including the space).
		std::cout << std::endl;
		std::any_cast<CPPON>(val).print(indentationLevel, level + 1);
		break;
	default:  // The only other possibility is vector; all vectors must contain std::any; below that they may contain another vector w/ std::any or it contains a primitive type. 
		auto vec = std::any_cast<std::vector<std::any>>(val);
		printVector(vec, indentationLevel, level + 1);
		break;
	};
}

CPPON decodeJSONFile(const char* filepath) {
	return CPPON();
}