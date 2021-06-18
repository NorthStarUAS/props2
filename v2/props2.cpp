#include <string>
using std::string;

#include "strutils.h"

//  #include "rapidjson/document.h"
// using namespace rapidjson;

#include "props2.h"

PropertyNode::PropertyNode() {
}

PropertyNode::PropertyNode(string abs_path, bool create) {
    vector<string> parts = split(abs_path, "/");
    Value& node = d;
    for ( int i = 0; i < parts.size(); i++ ) {
    }
    p = Pointer(abs_path.c_str());
    if ( create ) {
        p.Create(d);
    }
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

bool PropertyNode::getBool( const char *name ) {
    Value *parent = p.Get(d);
    if ( parent->IsObject() ) {
        if ( parent->HasMember(name) ) {
            return getValueAsBool((*parent)[name]);
        }
    }
    return false;
}

int PropertyNode::getInt( const char *name ) {
    Value *parent = p.Get(d);
    if ( parent->IsObject() ) {
        if ( parent->HasMember(name) ) {
            return getValueAsInt((*parent)[name]);
        }
    }
    return 0;
}

float PropertyNode::getFloat( const char *name ) {
    Value *parent = p.Get(d);
    if ( parent->IsObject() ) {
        if ( parent->HasMember(name) ) {
            return getValueAsFloat((*parent)[name]);
        }
    }
    return 0.0;
}

double PropertyNode::getDouble( const char *name ) {
    Value *parent = p.Get(d);
    if ( parent->IsObject() ) {
        if ( parent->HasMember(name) ) {
            return getValueAsDouble((*parent)[name]);
        }
    }
    return 0.0;
}

bool PropertyNode::setInt( const char *name, long val ) {
    Value *parent = p.Get(d);
    if ( !parent->IsObject() ) {
        parent->SetObject();
    }
    Value newval(val);
    if ( !parent->HasMember(name) ) {
        printf("creating %s\n", name);
        Value key(name, d.GetAllocator());
        parent->AddMember(key, newval, d.GetAllocator());
    } else {
        printf("%s already exists\n");
    }
    (*parent)[name] = val;
    return true;
}



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

}

Document d;
