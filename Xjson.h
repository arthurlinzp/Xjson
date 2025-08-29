/*
 __  __  _                    
 \ \/ / (_) ___   ___   _ __  
  \  /  | |/ __| / _ \ | '_ \ 
  /  \  | |\__ \| (_) || | | |
 /_/\_\_/ ||___/ \___/ |_| |_|
      |__/                    
@Author: ArthurLin
@email: 2236188747@qq.com
@Description: A modern C++ JSON library with nlohmann::json-like interface, powered by RapidJSON
*/

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include <initializer_list>
#include <cstdint>

// Include RapidJSON headers
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC__MINOR__ >= 8)
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

// Enumeration for error states
enum XjsonError
{
    Ok = 0,
    ParseError = 1,
    NotAnObject = 2,
    NotAnArray = 3,
    NotAContainer = 4,
    KeyNotFound = 5,
    IndexOutOfBounds = 6,
    WrongType = 7,
    ValueOutOfRange = 8,
    InvalidIteratorOp = 9
};

class Xjson
{
public:
    class iterator;
    class const_iterator;

    // --- Constructors ---
    Xjson()
    {
        init_new_doc();
        m_value->SetNull();
    }
    Xjson(std::nullptr_t) : Xjson() {}
    Xjson(int value)
    {
        init_new_doc();
        m_value->SetInt(value);
    }
    Xjson(unsigned value)
    {
        init_new_doc();
        m_value->SetUint(value);
    }
    Xjson(int64_t value)
    {
        init_new_doc();
        m_value->SetInt64(value);
    }
    Xjson(uint64_t value)
    {
        init_new_doc();
        m_value->SetUint64(value);
    }
    Xjson(double value)
    {
        init_new_doc();
        m_value->SetDouble(value);
    }
    Xjson(bool value)
    {
        init_new_doc();
        m_value->SetBool(value);
    }
    Xjson(const char *value)
    {
        init_new_doc();
        m_value->SetString(value, get_allocator());
    }
    Xjson(const std::string &value)
    {
        init_new_doc();
        m_value->SetString(value.c_str(), static_cast<rapidjson::SizeType>(value.length()), get_allocator());
    }

    // --- Copy and Move Semantics ---
    Xjson(const Xjson &other) { copy_from(other); }
    Xjson &operator=(const Xjson &other)
    {
        if (this != &other)
        {
            m_value->CopyFrom(other.get_rapidjson_value(), get_allocator());
        }
        return *this;
    }
    Xjson(Xjson &&other) NOEXCEPT : m_doc(std::move(other.m_doc)), m_value(other.m_value) { other.m_value = nullptr; }

    Xjson &operator=(Xjson &&other) NOEXCEPT
    {
        if (this != &other)
        {
            if (m_value == m_doc.get() && other.m_value == other.m_doc.get()) {
                // Efficient move if both are document roots
                m_doc = std::move(other.m_doc);
                m_value = m_doc.get();
                other.m_value = nullptr;
            } else {
                // Otherwise, perform a deep copy
                m_value->CopyFrom(other.get_rapidjson_value(), get_allocator());
            }
        }
        return *this;
    }

    // --- Static Factory Functions ---
    static Xjson object(std::initializer_list<std::pair<std::string, Xjson>> list)
    {
        Xjson obj;
        obj.m_value->SetObject();
        for (const auto &pair : list)
        {
            obj[pair.first] = pair.second;
        }
        return obj;
    }
    static Xjson array(std::initializer_list<Xjson> list)
    {
        Xjson arr;
        arr.m_value->SetArray();
        for (const auto &item : list)
        {
            arr.push_back(item);
        }
        return arr;
    }

    // --- Parsing and Serialization ---
    static Xjson parse(const std::string &json_str)
    {
        Xjson json;
        if (json.m_doc->Parse(json_str.c_str()).HasParseError())
        {
            json.m_error = XjsonError::ParseError;
            json.m_error_message = "JSON parse error at offset " + std::to_string(json.m_doc->GetErrorOffset());
        }
        return json;
    }
    std::string dump(int indent = -1) const
    {
        rapidjson::StringBuffer buffer;
        if (indent >= 0)
        {
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            writer.SetIndent(' ', indent);
            m_value->Accept(writer);
        }
        else
        {
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            m_value->Accept(writer);
        }
        return buffer.GetString();
    }

    // --- Type Checks ---
    bool is_null() const { return m_value->IsNull(); }
    bool is_bool() const { return m_value->IsBool(); }
    bool is_number() const { return m_value->IsNumber(); }
    bool is_int() const { return m_value->IsInt(); }
    bool is_int64() const { return m_value->IsInt64(); }
    bool is_uint() const { return m_value->IsUint(); }
    bool is_uint64() const { return m_value->IsUint64(); }
    bool is_double() const { return m_value->IsDouble(); }
    bool is_string() const { return m_value->IsString(); }
    bool is_array() const { return m_value->IsArray(); }
    bool is_object() const { return m_value->IsObject(); }

    // --- Value Accessors ---
    template <typename T>
    T get() const;
    operator int() const;
    operator int64_t() const;
    operator uint8_t() const;
    operator uint32_t() const;
    operator uint64_t() const;
    operator double() const;
    operator bool() const;
    operator std::string() const;

    // --- Object Operations ---
    Xjson operator[](const std::string &key)
    {
        if (has_error())
            return *this;
        if (is_null())
            m_value->SetObject();
        if (!is_object())
            return Xjson(XjsonError::NotAnObject, "Value is not an object, cannot use string key.");
        auto it = m_value->FindMember(key.c_str());
        if (it == m_value->MemberEnd())
        {
            m_value->AddMember(rapidjson::Value(key.c_str(), get_allocator()).Move(), rapidjson::Value().Move(), get_allocator());
            it = m_value->FindMember(key.c_str());
        }
        return Xjson(m_doc, &it->value);
    }
    Xjson operator[](const char *key) { return (*this)[std::string(key)]; }
    Xjson at(const std::string &key)
    {
        if (has_error())
            return *this;
        if (!is_object())
            return Xjson(XjsonError::NotAnObject, "Value is not an object.");
        auto it = m_value->FindMember(key.c_str());
        if (it == m_value->MemberEnd())
            return Xjson(XjsonError::KeyNotFound, "Key '" + key + "' not found.");
        return Xjson(m_doc, &it->value);
    }
    const Xjson at(const std::string &key) const
    {
        if (has_error())
            return *this;
        if (!is_object())
            return Xjson(XjsonError::NotAnObject, "Value is not an object.");
        auto it = m_value->FindMember(key.c_str());
        if (it == m_value->MemberEnd())
            return Xjson(XjsonError::KeyNotFound, "Key '" + key + "' not found.");
        return Xjson(m_doc, &it->value);
    }
    bool has_member(const std::string &key) const { return is_object() && m_value->HasMember(key.c_str()); }

    // --- Array Operations ---
    Xjson operator[](int index)
    {
        if (has_error())
            return *this;
        if (is_null())
            m_value->SetArray();
        if (!is_array())
            return Xjson(XjsonError::NotAnArray, "Value is not an array, cannot use integer index.");
        if (index >= m_value->Size())
        {
            m_value->Reserve(index + 1, get_allocator());
            while (index >= m_value->Size())
            {
                m_value->PushBack(rapidjson::Value(), get_allocator());
            }
        }
        return Xjson(m_doc, &(*m_value)[static_cast<rapidjson::SizeType>(index)]);
    }
    Xjson at(size_t index)
    {
        if (has_error())
            return *this;
        if (!is_array())
            return Xjson(XjsonError::NotAnArray, "Value is not an array.");
        if (index >= m_value->Size())
            return Xjson(XjsonError::IndexOutOfBounds, "Index " + std::to_string(index) + " is out of range.");
        return Xjson(m_doc, &(*m_value)[static_cast<rapidjson::SizeType>(index)]);
    }
    const Xjson at(size_t index) const
    {
        if (has_error())
            return *this;
        if (!is_array())
            return Xjson(XjsonError::NotAnArray, "Value is not an array.");
        if (index >= m_value->Size())
            return Xjson(XjsonError::IndexOutOfBounds, "Index " + std::to_string(index) + " is out of range.");
        return Xjson(m_doc, &(*m_value)[static_cast<rapidjson::SizeType>(index)]);
    }
    void push_back(const Xjson &value)
    {
        if (is_null())
            m_value->SetArray();
        if (!is_array())
        {
            m_error = XjsonError::NotAnArray;
            m_error_message = "JSON not an array error at offset " + std::to_string(m_doc->GetErrorOffset());
        }
        rapidjson::Value copied_value;
        copied_value.CopyFrom(value.get_rapidjson_value(), get_allocator());
        m_value->PushBack(copied_value, get_allocator());
    }

    // --- Generic Container Operations ---
    size_t size() const
    {
        if (is_array())
            return m_value->Size();
        if (is_object())
            return m_value->MemberCount();
        return 0;
    }
    bool empty() const { return size() == 0; }
    void clear()
    {
        if (is_array())
            m_value->Clear();
        else if (is_object())
            m_value->RemoveAllMembers();
    }

    // --- Iterator Support ---
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;

    bool has_error() const { return m_error != XjsonError::Ok; }
    XjsonError get_error_code() const { return m_error; }
    const std::string &get_error_message() const { return m_error_message; }

private:
    Xjson(std::shared_ptr<rapidjson::Document> doc, rapidjson::Value *value) : m_doc(doc), m_value(value) {}
    void init_new_doc()
    {
        m_doc = std::make_shared<rapidjson::Document>();
        m_value = m_doc.get();
    }
    void copy_from(const Xjson &other)
    {
        init_new_doc();
        m_value->CopyFrom(other.get_rapidjson_value(), get_allocator());
    }
    rapidjson::Document::AllocatorType &get_allocator() { return m_doc->GetAllocator(); }
    const rapidjson::Value &get_rapidjson_value() const { return *m_value; }

private:
    std::shared_ptr<rapidjson::Document> m_doc;
    rapidjson::Value *m_value;
    mutable XjsonError m_error{XjsonError::Ok};
    mutable std::string m_error_message;

    Xjson(XjsonError err, std::string msg)
        : m_doc(nullptr), m_value(nullptr), m_error(err), m_error_message(std::move(msg))
    {
    }
};

// Template specializations for get<T>()
template <>
inline int Xjson::get<int>() const
{
    if (!m_value->IsInt())
    {
        m_error = XjsonError::WrongType;
        m_error_message = "Type error: value is not an int.";
        return 0;
    }
    return m_value->GetInt();
}

template <>
inline int64_t Xjson::get<int64_t>() const
{
    if (!m_value->IsInt64())
    {
        m_error = XjsonError::WrongType;
        m_error_message = "Type error: value is not an int64.";
        return 0;
    }
    return m_value->GetInt64();
}

template <>
inline uint8_t Xjson::get<uint8_t>() const
{
    if (!m_value->IsUint())
    {
        m_error = XjsonError::WrongType;
        m_error_message = "Type error: value is not a uint.";
        return 0;
    }

    unsigned int val = m_value->GetUint();
    if (val > UINT8_MAX)
    {
        m_error = XjsonError::ValueOutOfRange;
        m_error_message = "Value error: value out of range for uint8_t.";
        return 0;
    }
    return static_cast<uint8_t>(val);
}

template <>
inline uint32_t Xjson::get<uint32_t>() const
{
    if (!m_value->IsUint())
    {
        m_error = XjsonError::WrongType;
        m_error_message = "Type error: value is not a uint.";
        return 0;
    }
    return m_value->GetUint();
}

template <>
inline uint64_t Xjson::get<uint64_t>() const
{
    if (!m_value->IsUint64())
    {
        m_error = XjsonError::WrongType;
        m_error_message = "Type error: value is not a uint64.";
        return 0;
    }
    return m_value->GetUint64();
}

template <>
inline double Xjson::get<double>() const
{
    if (!m_value->IsNumber())
    {
        m_error = XjsonError::WrongType;
        m_error_message = "Type error: value is not a number.";
        return 0.0;
    }
    return m_value->GetDouble();
}

template <>
inline bool Xjson::get<bool>() const
{
    if (!m_value->IsBool())
    {
        m_error = XjsonError::WrongType;
        m_error_message = "Type error: value is not a boolean.";
        return false;
    }
    return m_value->GetBool();
}

template <>
inline std::string Xjson::get<std::string>() const
{
    if (!m_value->IsString())
    {
        m_error = XjsonError::WrongType;
        m_error_message = "Type error: value is not a string.";
        return "";
    }
    return std::string(m_value->GetString(), m_value->GetStringLength());
}

// Conversion operators
inline Xjson::operator int() const { return get<int>(); }
inline Xjson::operator int64_t() const { return get<int64_t>(); }
inline Xjson::operator uint8_t() const { return get<uint8_t>(); }
inline Xjson::operator uint32_t() const { return get<uint32_t>(); }
inline Xjson::operator uint64_t() const { return get<uint64_t>(); }
inline Xjson::operator double() const { return get<double>(); }
inline Xjson::operator bool() const { return get<bool>(); }
inline Xjson::operator std::string() const { return get<std::string>(); }

// Iterator class
class Xjson::iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Xjson;
    using difference_type = std::ptrdiff_t;
    using pointer = Xjson*;
    using reference = Xjson&;

    iterator(Xjson* parent, rapidjson::Value::ValueIterator it)
        : m_parent(parent), m_arr_it(it), m_obj_it(), m_is_array(true) {}

    iterator(Xjson* parent, rapidjson::Value::MemberIterator it)
        : m_parent(parent), m_arr_it(), m_obj_it(it), m_is_array(false) {}

    iterator& operator++()
    {
        if (m_is_array)
            ++m_arr_it;
        else
            ++m_obj_it;
        return *this;
    }

    iterator operator++(int)
    {
        iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const iterator& other) const
    {
        return m_is_array == other.m_is_array &&
               (m_is_array ? m_arr_it == other.m_arr_it : m_obj_it == other.m_obj_it);
    }

    bool operator!=(const iterator& other) const
    {
        return !(*this == other);
    }

    value_type operator*()
    {
        if (m_is_array)
        {
            return value_type(m_parent->m_doc, &(*m_arr_it));
        }
        return value_type(m_parent->m_doc, &(m_obj_it->value));
    }

    std::string key() const
    {
        if (m_is_array)
            return "";
        return std::string(m_obj_it->name.GetString(), m_obj_it->name.GetStringLength());
    }

    value_type value() const
    {
        if (m_is_array)
            return "";
        return value_type(m_parent->m_doc, &(m_obj_it->value));
    }

private:
    Xjson* m_parent;
    rapidjson::Value::ValueIterator m_arr_it;
    rapidjson::Value::MemberIterator m_obj_it;
    bool m_is_array;
};

// Const iterator class
class Xjson::const_iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = const Xjson;
    using difference_type = std::ptrdiff_t;
    using pointer = const Xjson*;
    using reference = const Xjson&;

    const_iterator(const Xjson* parent, rapidjson::Value::ConstValueIterator it)
        : m_parent(parent), m_arr_it(it), m_obj_it(), m_is_array(true) {}

    const_iterator(const Xjson* parent, rapidjson::Value::ConstMemberIterator it)
        : m_parent(parent), m_arr_it(), m_obj_it(it), m_is_array(false) {}

    const_iterator& operator++()
    {
        if (m_is_array)
            ++m_arr_it;
        else
            ++m_obj_it;
        return *this;
    }

    const_iterator operator++(int)
    {
        const_iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const const_iterator& other) const
    {
        return m_is_array == other.m_is_array &&
               (m_is_array ? m_arr_it == other.m_arr_it : m_obj_it == other.m_obj_it);
    }

    bool operator!=(const const_iterator& other) const
    {
        return !(*this == other);
    }

    Xjson operator*() const
    {
        if (m_is_array)
        {
            return Xjson(m_parent->m_doc, const_cast<rapidjson::Value*>(&(*m_arr_it)));
        }
        return Xjson(m_parent->m_doc, const_cast<rapidjson::Value*>(&m_obj_it->value));
    }

    std::string key() const
    {
        if (m_is_array)
            return "";
        return std::string(m_obj_it->name.GetString(), m_obj_it->name.GetStringLength());
    }

    Xjson value() const
    {
        if (m_is_array)
            return "";
        return Xjson(m_parent->m_doc, const_cast<rapidjson::Value*>(&m_obj_it->value));
    }

private:
    const Xjson* m_parent;
    rapidjson::Value::ConstValueIterator m_arr_it;
    rapidjson::Value::ConstMemberIterator m_obj_it;
    bool m_is_array;
};

// Iterator member functions
inline Xjson::iterator Xjson::begin()
{
    if (has_error() || (!is_array() && !is_object()))
        return end();
    if (is_array())
        return iterator(this, m_value->Begin());
    return iterator(this, m_value->MemberBegin());
}

inline Xjson::iterator Xjson::end()
{
    if (has_error() || !is_object())
    {
        if (is_array())
            return iterator(this, m_value->End());
        return iterator(this, rapidjson::Value::MemberIterator());
    }
    if (is_array())
        return iterator(this, m_value->End());
    return iterator(this, m_value->MemberEnd());
}

inline Xjson::const_iterator Xjson::begin() const
{
    if (has_error() || !is_object())
    {
        if (is_array())
            return const_iterator(this, m_value->Begin());
        return const_iterator(this, rapidjson::Value::MemberIterator());
    }
    if (is_array())
        return const_iterator(this, m_value->Begin());
    return const_iterator(this, m_value->MemberEnd());
}

inline Xjson::const_iterator Xjson::end() const
{
    if (has_error() || !is_object())
    {
        if (is_array())
            return const_iterator(this, m_value->End());
        return const_iterator(this, rapidjson::Value::MemberIterator());
    }
    if (is_array())
        return const_iterator(this, m_value->End());
    return const_iterator(this, m_value->MemberEnd());
}

inline Xjson::const_iterator Xjson::cbegin() const { return begin(); }
inline Xjson::const_iterator Xjson::cend() const { return end(); }

// Stream output operator
inline std::ostream& operator<<(std::ostream& os, const Xjson& json)
{
    os << json.dump();
    return os;
}