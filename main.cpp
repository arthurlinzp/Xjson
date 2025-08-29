#include <iostream>
#include "Xjson.h"
#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#endif

void print_header(const std::string& title) {
    std::cout << "\n\n--- " << title << " ---\n";
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);   // or SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(65001);
#endif

    // ======================================================================
    // 1. Creation and Initialization
    // ======================================================================
    print_header("1. Creation and Initialization");
    
    // a. Create from basic types
    Xjson j_null;                    // Default constructed as null
    Xjson j_int = 123;
    Xjson j_double = 3.14159;
    Xjson j_bool = true;
    Xjson j_string = "Hello, World!";
    Xjson j_nullptr = nullptr;

    std::cout << "Null: " << j_null << std::endl;
    std::cout << "Int: " << j_int << std::endl;
    std::cout << "Double: " << j_double << std::endl;
    std::cout << "Bool: " << j_bool << std::endl;
    std::cout << "String: " << j_string << std::endl;
    std::cout << "From nullptr: " << j_nullptr << std::endl;

    // b. Use static factory functions to create complex objects
    Xjson j_obj = Xjson::object({
        {"Name", "Zhang San"},
        {"Age", 30},
        {"IsStudent", false},
        {"Scores", Xjson::array({ 88, 95, 72.5 })},
        {"ContactInfo", Xjson::object({
            {"Email", "zhangsan@example.com"},
            {"Phone", nullptr} // Phone missing
        })}
    });
    std::cout << "\nCreated complex object:\n" << j_obj.dump(4) << std::endl;


    // ======================================================================
    // 2. Parsing and Serialization
    // ======================================================================
    print_header("2. Parsing and Serialization");
    
    std::string raw_json_str = R"({"id": "s001", "active": true, "permissions": ["read", "write"]})";
    Xjson parsed_json = Xjson::parse(raw_json_str);
    std::cout << "Parsed object from string: " << parsed_json << std::endl;
    
    std::cout << "Compact serialization: " << parsed_json.dump() << std::endl;
    std::cout << "Pretty serialization (2-space indent):\n" << parsed_json.dump(2) << std::endl;


    // ======================================================================
    // 3. Data Access (Reading)
    // ======================================================================
    print_header("3. Data Access (Reading)");

    // a. Type checking
    std::cout << "j_obj.is_object(): " << std::boolalpha << j_obj.is_object() << std::endl;
    std::cout << "j_obj[\"Scores\"].is_array(): " << std::boolalpha << j_obj["Scores"].is_array() << std::endl;
    
    // b. Read using operator[]
    std::string name = j_obj["Name"];
    int age = j_obj["Age"];
    double first_score = j_obj["Scores"][0];
    std::cout << "Name: " << name << ", Age: " << age << ", First score: " << first_score << std::endl;

    // c. Chained access
    std::string email = j_obj["ContactInfo"]["Email"];
    std::cout << "Email: " << email << std::endl;
    
    // d. Explicitly get using get<T>()
    bool is_student = j_obj["IsStudent"].get<bool>();
    assert(is_student == false);

    // e. Check if member exists
    std::cout << "Has 'Phone' key: " << std::boolalpha << j_obj["ContactInfo"].has_member("Phone") << std::endl;
    std::cout << "Has 'Address' key: " << std::boolalpha << j_obj["ContactInfo"].has_member("Address") << std::endl;


    // ======================================================================
    // 4. Modifying Data (Writing)
    // ======================================================================
    print_header("4. Modifying Data (Writing)");

    // a. Modify existing values
    j_obj["Age"] = 31;
    j_obj["ContactInfo"]["Phone"] = "13800138000";
    std::cout << "Modified age: " << (int)j_obj["Age"] << std::endl;
    std::cout << "Modified phone: " << (std::string)j_obj["ContactInfo"]["Phone"] << std::endl;
    
    // b. Add new members
    j_obj["City"] = "Beijing";
    
    // c. Operate on array
    Xjson scores = j_obj["Scores"];
    scores.push_back(99.0);
    scores[0] = 90; // Modify first element
    std::cout << "\nModified scores array: " << scores << std::endl;

    // d. Chain to create nested objects
    j_obj["AdditionalInfo"]["Note"]["Content"] = "This is a note";
    
    std::cout << "\nFully modified object:\n" << j_obj.dump(4) << std::endl;

    // e. Clear container
    Xjson to_clear = Xjson::array({1, 2, 3});
    std::cout << "Array before clear: " << to_clear << ", Size: " << to_clear.size() << std::endl;
    to_clear.clear();
    std::cout << "Array after clear: " << to_clear << ", Size: " << to_clear.size() << std::endl;


    // ======================================================================
    // 5. Iterators
    // ======================================================================
    print_header("5. Iterators");
    
    // a. Iterate over object
    std::cout << "\nIterating over object j_obj:" << std::endl;
    for (auto it = j_obj.begin(); it != j_obj.end(); ++it) {
        std::cout << "  Key: \"" << it.key() << "\", Value: " << it.value() << std::endl;
        // Modify value during iteration
        if (it.key() == "City") {
            it.value() = "Shanghai";
        }
    }
    std::cout << "After modifying city during iteration: " << (std::string)j_obj["City"] << std::endl;
    
    // b. Iterate over array (using range-based for loop)
    std::cout << "\nIterating over array j_obj[\"Scores\"]:" << std::endl;
    for (auto score : j_obj["Scores"]) {
        std::cout << "  " << (double)score;
    }
    std::cout << std::endl;

    // c. const iterator
    const Xjson const_obj = j_obj;
    std::cout << "\nUsing const iterator to traverse object:" << std::endl;
    for (auto it = const_obj.cbegin(); it != const_obj.cend(); ++it) {
        // it.value() = "new value"; // <-- This line would fail to compile, as it.value() returns a temporary proxy
        std::cout << "  Key: \"" << it.key() << "\", Is number: " << it.value().is_number() << std::endl;
    }


    // ======================================================================
    // 6. Copy and Proxy Behavior
    // ======================================================================
    print_header("6. Copy and Proxy Behavior");
    
    // a. Proxy (view) behavior
    Xjson original = Xjson::object({{"a", 1}});
    Xjson proxy = original["a"]; // proxy is a view/proxy to 'a'
    proxy = 2;                  // Modify through proxy
    std::cout << "After modifying via proxy, original: " << original << std::endl; // original is also modified
    assert((int)original["a"] == 2);

    // b. Copy behavior
    Xjson copy = original;       // copy is a deep copy of original
    copy["a"] = 3;              // Modify copy
    std::cout << "After modifying copy, original: " << original << std::endl; // original unchanged
    std::cout << "After modifying copy, copy: " << copy << std::endl;
    assert((int)original["a"] == 2);
    assert((int)copy["a"] == 3);
    

    std::cout << "\nAll tests completed!" << std::endl;

    return 0;
}