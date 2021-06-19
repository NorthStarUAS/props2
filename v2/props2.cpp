#include <string>
using std::string;

#include "strutils.h"

#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

#include "props2.h"

PropertyNode::PropertyNode() {
}

PropertyNode::PropertyNode(string abs_path, bool create) {
    Pointer p = Pointer(abs_path.c_str());
    if ( create ) {
        p.Create(d);
    }
    v = p.Get(d);
    if ( v != NULL ) {
        v->SetObject();
    }
}

PropertyNode::PropertyNode(Value *value) {
    v = value;
}

bool PropertyNode::hasChild( const char *name ) {
    if ( v->IsObject() ) {
        if ( v->HasMember(name) ) {
            return true;
        }
    }
    return false;
}

PropertyNode PropertyNode::getChild( const char *name, bool create ) {
    if ( v->IsObject() ) {
        if ( !v->HasMember(name) and create ) {
            Value key(name, d.GetAllocator());
            Value node;
            node.SetObject();
            v->AddMember(key, node, d.GetAllocator());
        }
        Value &child = (*v)[name];
        return PropertyNode(&child);
    }
    printf("%s not an object...\n", name);
    return PropertyNode();
}

int PropertyNode::getLen( const char *name) {
    if ( v->IsObject() and v->IsArray() ) {
        return v->Size();
    } else {
        return 0;
    }
}

vector<string> PropertyNode::getChildren(bool expand) {
    vector<string> result;
    if ( v->IsObject() ) {
        for (Value::ConstMemberIterator itr = v->MemberBegin(); itr != v->MemberEnd(); ++itr) {
            string name = itr->name.GetString();
            if ( expand and itr->value.IsArray() ) {
                for ( int i = 0; i < itr->value.Size(); i++ ) {
                    string ename = name + "[" + std::to_string(i) + "]";
                    result.push_back(ename);
                }
            } else {
                result.push_back(name);
            }
        }
    }
    return result;
}

static bool getValueAsBool( Value &v ) {
    if ( v.IsBool() ) {
        return v.GetBool();
    } else if ( v.IsInt() ) {
        return v.GetInt();
    } else if ( v.IsUint() ) {
        return v.GetUint();
    } else if ( v.IsInt64() ) {
        return v.GetInt64();
    } else if ( v.IsUint64() ) {
        return v.GetUint64();
    } else if ( v.IsFloat() ) {
        return v.GetFloat() == 0.0;
    } else if ( v.IsDouble() ) {
        return v.GetDouble() == 0.0;
    } else if ( v.IsString() ) {
        string s = v.GetString();
        if ( s == "true" or s == "True" or s == "TRUE" ) {
            return true;
        } else {
            return false;
        }
    } else {
        printf("Unknown type in getValueAsBool()\n");
    }
    return false;
}

static int getValueAsInt( Value &v ) {
    if ( v.IsBool() ) {
        return v.GetBool();
    } else if ( v.IsInt() ) {
        return v.GetInt();
    } else if ( v.IsUint() ) {
        return v.GetUint();
    } else if ( v.IsInt64() ) {
        return v.GetInt64();
    } else if ( v.IsUint64() ) {
        return v.GetUint64();
    } else if ( v.IsFloat() ) {
        return v.GetFloat();
    } else if ( v.IsDouble() ) {
        return v.GetDouble();
    } else if ( v.IsString() ) {
        string s = v.GetString();
        return std::stoi(s);
    } else {
        printf("Unknown type in getValueAsBool()\n");
    }
    return 0;
}

static float getValueAsFloat( Value &v ) {
    if ( v.IsBool() ) {
        return v.GetBool();
    } else if ( v.IsInt() ) {
        return v.GetInt();
    } else if ( v.IsUint() ) {
        return v.GetUint();
    } else if ( v.IsInt64() ) {
        return v.GetInt64();
    } else if ( v.IsUint64() ) {
        return v.GetUint64();
    } else if ( v.IsFloat() ) {
        return v.GetFloat();
    } else if ( v.IsDouble() ) {
        return v.GetDouble();
    } else if ( v.IsString() ) {
        string s = v.GetString();
        return std::stof(s);
    } else {
        printf("Unknown type in getValueAsBool()\n");
    }
    return 0.0;
}

static double getValueAsDouble( Value &v ) {
    if ( v.IsBool() ) {
        return v.GetBool();
    } else if ( v.IsInt() ) {
        return v.GetInt();
    } else if ( v.IsUint() ) {
        return v.GetUint();
    } else if ( v.IsInt64() ) {
        return v.GetInt64();
    } else if ( v.IsUint64() ) {
        return v.GetUint64();
    } else if ( v.IsFloat() ) {
        return v.GetFloat();
    } else if ( v.IsDouble() ) {
        return v.GetDouble();
    } else if ( v.IsString() ) {
        string s = v.GetString();
        return std::stod(s);
    } else {
        printf("Unknown type in getValueAsBool()\n");
    }
    return 0.0;
}

static string getValueAsString( Value &v ) {
    if ( v.IsBool() ) {
        if ( v.GetBool() ) {
            return "true";
        } else {
            return "false";
        }
    } else if ( v.IsInt() ) {
        return std::to_string(v.GetInt());
    } else if ( v.IsUint() ) {
        return std::to_string(v.GetUint());
    } else if ( v.IsInt64() ) {
        return std::to_string(v.GetInt64());
    } else if ( v.IsUint64() ) {
        return std::to_string(v.GetUint64());
    } else if ( v.IsFloat() ) {
        return std::to_string(v.GetFloat());
    } else if ( v.IsDouble() ) {
        return std::to_string(v.GetDouble());
    } else if ( v.IsString() ) {
        return v.GetString();
    }
    return "unhandled value type";
}

bool PropertyNode::getBool( const char *name ) {
    if ( v->IsObject() ) {
        if ( v->HasMember(name) ) {
            return getValueAsBool((*v)[name]);
        }
    }
    return false;
}

int PropertyNode::getInt( const char *name ) {
    if ( v->IsObject() ) {
        if ( v->HasMember(name) ) {
            return getValueAsInt((*v)[name]);
        }
    }
    return 0;
}

float PropertyNode::getFloat( const char *name ) {
    if ( v->IsObject() ) {
        if ( v->HasMember(name) ) {
            return getValueAsFloat((*v)[name]);
        }
    }
    return 0.0;
}

double PropertyNode::getDouble( const char *name ) {
    if ( v->IsObject() ) {
        if ( v->HasMember(name) ) {
            return getValueAsDouble((*v)[name]);
        }
    }
    return 0.0;
}

string PropertyNode::getString( const char *name ) {
    if ( v->IsObject() ) {
        if ( v->HasMember(name) ) {
            return getValueAsString((*v)[name]);
        } else {
            return (string)name + ": not a member";
        }
    }
    return (string)name + ": not an object";
}

bool PropertyNode::setInt( const char *name, long val ) {
    if ( !v->IsObject() ) {
        v->SetObject();
    }
    Value newval(val);
    if ( !v->HasMember(name) ) {
        printf("creating %s\n", name);
        Value key(name, d.GetAllocator());
        v->AddMember(key, newval, d.GetAllocator());
    } else {
        printf("%s already exists\n");
    }
    (*v)[name] = val;
    return true;
}

bool PropertyNode::setDouble( const char *name, double val ) {
    if ( !v->IsObject() ) {
        v->SetObject();
    }
    Value newval(val);
    if ( !v->HasMember(name) ) {
        printf("creating %s\n", name);
        Value key(name, d.GetAllocator());
        v->AddMember(key, newval, d.GetAllocator());
    } else {
        printf("%s already exists\n");
    }
    (*v)[name] = val;
    return true;
}

bool PropertyNode::setString( const char *name, string val ) {
    if ( !v->IsObject() ) {
        v->SetObject();
    }
    if ( !v->HasMember(name) ) {
        Value newval("");
        printf("creating %s\n", name);
        Value key(name, d.GetAllocator());
        v->AddMember(key, newval, d.GetAllocator());
    } else {
        printf("%s already exists\n");
    }
    (*v)[name].SetString(val.c_str(), val.length());
    return true;
}

void PropertyNode::pretty_print() {
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    v->Accept(writer);
    //const char* output = buffer.GetString();
    printf("%s\n", buffer.GetString());
}

Document d;

#if 0
int main() {
   // suck in all the input
    string input_buf = "";
    while ( true ) {
        char c = getchar();
        if ( c == EOF ) {
            break;
        }
        input_buf += c;
    }

    // d.Parse(input_buf.c_str());

    PropertyNode n1 = PropertyNode("/a/b/c", true);
    n1.setInt("curt", 53);
    printf("%ld\n", n1.getInt("curt"));
    n1.setInt("curt", 55);
    printf("%ld\n", n1.getInt("curt"));
    printf("As bool: %d\n", n1.getBool("curt"));
    printf("As double: %.2f\n", n1.getDouble("curt"));
    string s = n1.getString("curt");
    printf("As string: %s\n", s.c_str());
    n1.setString("foo", "1.2345");
    printf("As double: %.2f\n", n1.getDouble("foo"));
    printf("As int: %d\n", n1.getInt("foo"));
}
#endif
