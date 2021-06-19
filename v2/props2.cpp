#if defined(ARDUINO_BUILD)
#  undef _GLIBCXX_USE_C99_STDIO   // vsnprintf() not defind
#  include "setup_board.h"
#endif

#include <stdio.h>

#include <vector>
#include <string>
using std::vector;
using std::string;

#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

#include "strutils.h"
#include "props2.h"

static void pretty_print_tree(Value *v) {
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    v->Accept(writer);
    //const char* output = buffer.GetString();
    printf("%s\n", buffer.GetString());
}

static bool is_integer(const string val) {
    for ( int i = 0; i < val.length(); i++ ) {
        if ( val[i] < '0' or val[i] > '9' ) {
            return false;
        }
    }
    return true;
}

static bool extend_array(Value *node, int size) {
    if ( !node->IsArray() ) {
        node->SetArray();
    }
    for ( int i = node->Size(); i < size; i++ ) {
        printf("    extemding: %d\n", i);
        Value newobj(kObjectType);
        node->PushBack(newobj, doc.GetAllocator());
    }
    pretty_print_tree(&doc);
    return true;
}

PropertyNode::PropertyNode() {
}

// PropertyNode::PropertyNode(string abs_path, bool create) {
//     printf("PropertyNode(%s)\n", abs_path.c_str());
//     p = Pointer(abs_path.c_str());
//     Value *val = p.Get(doc);
//     if ( val == nullptr and create ) {
//         printf("  creating\n");
//         p.Create(doc);
//     }
//     val = p.Get(doc);
//     if (val == nullptr) {
//         printf("  val is still null\n");
//     }
//     if ( !val->IsObject() ) {
//         printf("  setting as object\n");
//         val->SetObject();
//     }
// }

static Value *find_node_from_path(Value *node, string path, bool create) {
    printf("PropertyNode(%s)\n", path.c_str());
    if ( !node->IsObject() ) {
        node->SetObject();
    }
    printf("starting tree:\n");
    pretty_print_tree(&doc);
    vector<string> tokens = split(path, "/");
    for ( int i = 0; i < tokens.size(); i++ ) {
        if ( tokens[i].length() == 0 ) {
            continue;
        }
        printf("  token: %s\n", tokens[i].c_str());
        if ( is_integer(tokens[i]) ) {
            // array reference
            int index = std::stoi(tokens[i].c_str());
            extend_array(node, index+1);
            printf("Array size: %d\n", node->Size());
            node = &(*node)[index];
        } else {
            if ( node->HasMember(tokens[i].c_str()) ) {
                printf("    has %s\n", tokens[i].c_str());
                node = &(*node)[tokens[i].c_str()];
                pretty_print_tree(node);
            } else if ( create ) {
                printf("    creating\n");
                Value key;
                key.SetString(tokens[i].c_str(), tokens[i].length(), doc.GetAllocator());
                Value newobj(kObjectType);
                node->AddMember(key, newobj, doc.GetAllocator());
                pretty_print_tree(&doc);
                node = &(*node)[tokens[i].c_str()];
            } else {
                return nullptr;
            }
        }
    }
    if ( node->IsArray() ) {
        // when node is an array and no index specified, default to /0
        if ( node->Size() > 0 ) {
            node = &(*node)[0];
        }
    }
    return node;
}

PropertyNode::PropertyNode(string abs_path, bool create) {
    printf("PropertyNode(%s)\n", abs_path.c_str());
    if ( abs_path[0] != '/' ) {
        printf("  not an absolute path\n");
        return;
    }
    val = find_node_from_path(&doc, abs_path, create);
}

PropertyNode::PropertyNode(Value *v) {
    val = v;
}

bool PropertyNode::hasChild( const char *name ) {
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            return true;
        }
    }
    return false;
}

PropertyNode PropertyNode::getChild( const char *name, bool create ) {
    if ( val->IsObject() ) {
        Value *child = find_node_from_path(val, name, create);
        return PropertyNode(child);
    }
    printf("%s not an object...\n", name);
    return PropertyNode();
}

int PropertyNode::getLen() {
    printf("getLen()\n");
    pretty_print();
    if ( val->IsArray() ) {
        return val->Size();
    } else {
        return 0;
    }
}

vector<string> PropertyNode::getChildren(bool expand) {
    vector<string> result;
    if ( val->IsObject() ) {
        for (Value::ConstMemberIterator itr = val->MemberBegin(); itr != val->MemberEnd(); ++itr) {
            string name = itr->name.GetString();
            if ( expand and itr->value.IsArray() ) {
                for ( int i = 0; i < itr->value.Size(); i++ ) {
                    string ename = name + "/" + std::to_string(i);
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
#if defined(ARDUINO_BUILD)
        char buf[30];
        hal.util->snprintf(buf, 30, "%f", v.GetFloat());
        return buf;
#else
        return std::to_string(v.GetFloat());
#endif
    } else if ( v.IsDouble() ) {
#if defined(ARDUINO_BUILD)
        char buf[30];
        hal.util->snprintf(buf, 30, "%lf", v.GetDouble());
        return buf;
#else
        return std::to_string(v.GetDouble());
#endif
    } else if ( v.IsString() ) {
        return v.GetString();
    }
    return "unhandled value type";
}

bool PropertyNode::getBool( const char *name ) {
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            return getValueAsBool((*val)[name]);
        }
    }
    return false;
}

int PropertyNode::getInt( const char *name ) {
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            return getValueAsInt((*val)[name]);
        }
    }
    return 0;
}

float PropertyNode::getFloat( const char *name ) {
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            return getValueAsFloat((*val)[name]);
        } else {
            printf("no member in getFloat()\n");
        }
    } else {
        printf("v is not an object\n");
    }
    return 0.0;
}

double PropertyNode::getDouble( const char *name ) {
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            return getValueAsDouble((*val)[name]);
        }
    }
    return 0.0;
}

string PropertyNode::getString( const char *name ) {
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            return getValueAsString((*val)[name]);
        } else {
            return (string)name + ": not a member";
        }
    }
    return (string)name + ": not an object";
}

bool PropertyNode::setInt( const char *name, int n ) {
    if ( !val->IsObject() ) {
        val->SetObject();
    }
    Value newval(n);
    if ( !val->HasMember(name) ) {
        printf("creating %s\n", name);
        Value key(name, doc.GetAllocator());
        val->AddMember(key, newval, doc.GetAllocator());
    } else {
        printf("%s already exists\n", name);
    }
    (*val)[name] = n;
    return true;
}

bool PropertyNode::setFloat( const char *name, float x ) {
    //printf("setFloat(%s) = %f\n", name, val);
    // hal.scheduler->delay(100);
    if ( !val->IsObject() ) {
        printf("  converting value to object\n");
        // hal.scheduler->delay(100);
        val->SetObject();
    }
    // printf("  creating newval\n");
    // hal.scheduler->delay(100);
    Value newval(x);
    if ( !val->HasMember(name) ) {
        printf("creating %s\n", name);
        Value key(name, doc.GetAllocator());
        val->AddMember(key, newval, doc.GetAllocator());
    } else {
        printf("%s already exists\n", name);
    }
    // hal.scheduler->delay(100);
    (*val)[name] = x;
    return true;
}

bool PropertyNode::setDouble( const char *name, double x ) {
    if ( !val->IsObject() ) {
        val->SetObject();
    }
    Value newval(x);
    if ( !val->HasMember(name) ) {
        printf("creating %s\n", name);
        Value key(name, doc.GetAllocator());
        val->AddMember(key, newval, doc.GetAllocator());
    } else {
        printf("%s already exists\n", name);
    }
    (*val)[name] = x;
    return true;
}

bool PropertyNode::setString( const char *name, string s ) {
    if ( !val->IsObject() ) {
        val->SetObject();
    }
    if ( !val->HasMember(name) ) {
        Value newval("");
        printf("creating %s\n", name);
        Value key(name, doc.GetAllocator());
        val->AddMember(key, newval, doc.GetAllocator());
    } else {
        printf("%s already exists\n", name);
    }
    (*val)[name].SetString(s.c_str(), s.length());
    return true;
}

void PropertyNode::pretty_print() {
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    val->Accept(writer);
    //const char* output = buffer.GetString();
    printf("%s\n", buffer.GetString());
}

Document doc;

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

    // doc.Parse(input_buf.c_str());

    PropertyNode n1 = PropertyNode("/a/b/c/d", true);
    PropertyNode n2 = PropertyNode("/a/b/c/d", true);
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
    PropertyNode("/").pretty_print();
}
#endif
