#ifndef __JSON_H__
#define __JSON_H__

#include <map>
#include <sstream>
#include <string>
#include <vector>

class JsonValue {
  public:
    virtual std::string serialize() const = 0;

    virtual JsonValue *clone() const = 0;
};

template<typename T>
class JsonNumber : public JsonValue {
  public:
    JsonNumber(const T& value) : value(value) {}

    std::string serialize() const {
        std::ostringstream ss;
        ss << value;
        return ss.str();
    }

    JsonNumber* clone() const { return new JsonNumber<T>(value); }

  private:
    T value;
};

class JsonBool : public JsonValue {
  public:
    JsonBool(bool value) : value(value) {}

    std::string serialize() const { return value?"true":"false"; }

    JsonBool* clone() const { return new JsonBool(value); }

  private:
    bool value;
};

class JsonNull : public JsonValue {
  public:
    std::string serialize() const { return "null"; }

    JsonNull* clone() const { return new JsonNull; }
};

class JsonString : public JsonValue {
  public:
    JsonString(const std::string& value) : value(value) {}

    std::string serialize() const {
        return "\"" + escape() + "\"";
    }

    JsonString* clone() const { return new JsonString(value); }

	static std::string escape(const std::string& json) {
        std::string new_str;
        for (int i = 0; i < json.size(); i++) {
            switch (json[i]) {
                case '\\': new_str += "\\\\"; break;
                case '"': new_str += "\\\""; break;
                case '/': new_str += "\\/"; break;
                case '\b': new_str += "\\b"; break;
                case '\f': new_str += "\\f"; break;
                case '\n': new_str += "\\n"; break;
                case '\r': new_str += "\\r"; break;
                case '\t': new_str += "\\t"; break;
                default: new_str += json[i]; break;
            }
        }
        return new_str;
    }

  private:
    std::string escape() const {
		return escape(value);
    }

    std::string value;
};

class JsonObject : public JsonValue {
  public:
    typedef std::map<JsonString*, JsonValue*>::const_iterator const_iterator;

	JsonObject() {}
	JsonObject(const JsonObject& o) {
		for (const_iterator it = o.map_.begin(); it != o.map_.end(); ++it) {
			insert(*it->first, *it->second);
		}
	}

    // This is for convenience.
    void insert(const std::string& str, const JsonValue& value) {
        insert(JsonString(str), value);
    }

    void insert(const JsonString& key, const JsonValue& value) {
        map_[key.clone()] = value.clone();
    }

	std::string serialize() const {
		return "{" + serializeWithoutBraces() + "}";
	}

    std::string serializeWithoutBraces() const {
        if (map_.empty())
            return "";

        const_iterator it = map_.begin();
        std::string value = "";
        value += it->first->serialize() + ":" + it->second->serialize();
        ++it;

        while (it != map_.end()) {
            value += ",";
            value += it->first->serialize() + ":" + it->second->serialize();
            ++it;
        }

		return value;
    }

    JsonObject *clone() const {
        JsonObject *new_object = new JsonObject;
        for (const_iterator it = map_.begin(); it != map_.end(); ++it) {
            new_object->insert(*it->first, *it->second);
        }

        return new_object;
    }

    ~JsonObject() {
        for (const_iterator it = map_.begin(); it != map_.end(); ++it) {
            delete it->first;
            delete it->second;
        }
    }

  private:
    std::map<JsonString*, JsonValue*> map_;
};

class JsonArray : public JsonValue {
  public:
    std::string serialize() const {
        if (values_.empty())
            return "[]";

        std::string str = "[" + values_[0]->serialize();
        for (int i = 1; i < values_.size(); i++) {
            str += ",";
            str += values_[i]->serialize();
        }
        str += "]";

        return str;
    }

    JsonArray* clone() const {
        JsonArray* new_array = new JsonArray;

        for (int i = 0; i < values_.size(); i++)
            new_array->append(*values_[i]);

        return new_array;
    }

    void append(const JsonValue& value) {
        values_.push_back(value.clone());
    }

    ~JsonArray() {
        for (int i = 0; i < values_.size(); i++)
            delete values_[i];
    }

  private:
    std::vector<JsonValue*> values_;
};

#endif
