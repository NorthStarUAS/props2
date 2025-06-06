#if defined(ARDUINO)
#  include <SD.h>
   extern FS *configfs;  // SD or LittleFS_Program (or other) defined in the top level sketch
#elif defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <fcntl.h>            // open()
#  include <io.h>               // open() for MSVC-2022
//#  include <unistd.h>           // read()
#else
#  include <sys/stat.h>
#  include <sys/statfs.h>
#  include <sys/types.h>
#  include <fcntl.h>            // open()
#  include <unistd.h>           // read()
#endif

#include <math.h>
#include <stdio.h>

#include <vector>
#include <string>
using std::vector;
using std::string;

#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "util/strutils.h"

#include "props2.h"

static bool is_integer(const string val) {
    for ( unsigned int i = 0; i < val.length(); i++ ) {
        if ( val[i] < '0' || val[i] > '9' ) {
            return false;
        }
    }
    return true;
}

bool PropertyNode::extend_array(Value *node, int size) {
    if ( !node->IsArray() ) {
        node->SetArray();
    }
    for ( int i = node->Size(); i < size; i++ ) {
        // printf("    extending: %d\n", i);
        Value newobj(kObjectType);
        node->PushBack(newobj, doc->GetAllocator());
    }
    return true;
}

PropertyNode::PropertyNode() {
    init_shared_state();
}

Value *PropertyNode::find_node_from_path(Value *start_node, string path, bool create) {
    Value *node = start_node;
    // printf("PropertyNode(%s)\n", path.c_str());
    if ( !node->IsObject() ) {
        node->SetObject();
        if ( !node->IsObject() ) {
            printf("  still not object after setting to object.\n");
        }
    }
    vector<string> tokens = split(path, "/");
    for ( unsigned int i = 0; i < tokens.size(); i++ ) {
        if ( tokens[i].length() == 0 ) {
            continue;
        }
        // printf("  token: %s\n", tokens[i].c_str());
        if ( is_integer(tokens[i]) ) {
            // array reference
            int index = std::stoi(tokens[i].c_str());
            extend_array(node, index+1);
            // printf("Array size: %d\n", node->Size());
            node = &(*node)[index];
            //PropertyNode(node).pretty_print();
        } else {
            if ( node->HasMember(tokens[i].c_str()) ) {
                // printf("    has %s\n", tokens[i].c_str());
                node = &(*node)[tokens[i].c_str()];
            } else if ( create ) {
                // printf("    creating %s (%d)\n", tokens[i].c_str(), *realloc_check_counter);
                Value key;
                key.SetString(tokens[i].c_str(), tokens[i].length(), doc->GetAllocator());
                Value newobj(kObjectType);
                (*realloc_check_counter)++;
                node->AddMember(key, newobj, doc->GetAllocator());
                node = &(*node)[tokens[i].c_str()];
                // printf("  new node: %p\n", node);
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
    // printf(" found/create node->%p\n", node);
    return node;
}

PropertyNode::PropertyNode(string abs_path, bool create) {
    init_shared_state();
    // printf("PropertyNode(%s) %d\n", abs_path.c_str(), (unsigned long)&doc);
    if ( abs_path[0] != '/' ) {
        printf("  not an absolute path\n");
        return;
    }
    val = find_node_from_path(doc, abs_path, create);
    saved_path = abs_path;
    saved_realloc_check_counter = *realloc_check_counter;
    // pretty_print();
}

//PropertyNode::PropertyNode(Value *v) {
//    init_shared_state();
//    val = v;
//}

void PropertyNode::realloc_check() {
    if ( saved_realloc_check_counter != *realloc_check_counter ) {
        // printf("realloc possible, updating pointer to %s\n", saved_path.c_str());
        // printf(" saved %d  new %d\n", saved_realloc_check_counter, *realloc_check_counter);
        // Value *tmp = val;
        val = find_node_from_path(doc, saved_path, true);
        // printf("  orig %p -> new %p\n", tmp, val);
        saved_realloc_check_counter = *realloc_check_counter;
    }
}

bool PropertyNode::hasChild( const char *name ) {
    realloc_check();
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            return true;
        }
    }
    return false;
}

PropertyNode PropertyNode::getChild( const char *name, bool create ) {
    realloc_check();
    // printf("  get child of %s\n", saved_path.c_str());
    string child_path = saved_path + "/" + name;
    // printf("    new path: %s\n", child_path.c_str());
    if ( val->IsObject() ) {
        // Value *child = find_node_from_path(val, name, create);
        return PropertyNode(child_path);
    }
    printf("%s not an object...\n", name);
    return PropertyNode();
}

bool PropertyNode::isNull() {
    return val == nullptr;
}

bool PropertyNode::isParent(const char *name) {
    realloc_check();
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            Value &v = (*val)[name];
            return v.IsObject();
        }
    }
    return false;
}

bool PropertyNode::isArray(const char *name) {
    realloc_check();
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            Value &v = (*val)[name];
            return v.IsArray();
        }
    }
    return false;
}

bool PropertyNode::isValue(const char *name) {
    realloc_check();
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            Value &v = (*val)[name];
            return !v.IsObject() && !v.IsArray();
        }
    }
    return false;
}

bool PropertyNode::isValue(const char *name, unsigned int index) {
    realloc_check();
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            Value &v = (*val)[name];
            if ( v.IsArray() && index < v.Size() ) {
                return !v[index].IsObject() && !v[index].IsArray();
            }
        }
    }
    return false;
}

int PropertyNode::getLen( const char *name ) {
    realloc_check();
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            Value &v = (*val)[name];
            if ( v.IsArray() ) {
                return v.Size();
            }
        }
    }
    return 0;
}

vector<string> PropertyNode::getChildren(bool expand) {
    realloc_check();
    vector<string> result;
    if ( val->IsObject() ) {
        for (Value::ConstMemberIterator itr = val->MemberBegin(); itr != val->MemberEnd(); ++itr) {
            string name = itr->name.GetString();
            if ( expand && itr->value.IsArray() ) {
                for ( unsigned int i = 0; i < itr->value.Size(); i++ ) {
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
    } else if ( v.IsDouble() ) {
	return fabs(v.GetDouble()) < 0.0000001;
    } else if ( v.IsString() ) {
        string s = v.GetString();
        if ( s == "true" || s == "True" || s == "TRUE" ) {
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
    } else if ( v.IsDouble() ) {
        return v.GetDouble();
    } else if ( v.IsString() ) {
        string s = v.GetString();
        return std::stoi(s);
    } else {
        printf("Unknown type in getValueAsInt()\n");
    }
    return 0;
}

static unsigned int getValueAsUInt( Value &v ) {
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
    } else if ( v.IsDouble() ) {
        return v.GetDouble();
    } else if ( v.IsString() ) {
        string s = v.GetString();
        return std::stoi(s);
    } else {
        printf("Unknown type in getValueAsUInt()\n");
    }
    return 0;
}

static int64_t getValueAsInt64( Value &v ) {
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
    } else if ( v.IsDouble() ) {
        return v.GetDouble();
    } else if ( v.IsString() ) {
        string s = v.GetString();
        return std::stoi(s);
    } else {
        printf("Unknown type in getValueAsInt64()\n");
    }
    return 0;
}

static uint64_t getValueAsUInt64( Value &v ) {
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
    } else if ( v.IsDouble() ) {
        return v.GetDouble();
    } else if ( v.IsString() ) {
        string s = v.GetString();
        return std::stoi(s);
    } else {
        printf("Unknown type in getValueAsUInt64()\n");
    }
    return 0;
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
    } else if ( v.IsDouble() ) {
        return v.GetDouble();
    } else if ( v.IsString() ) {
        string s = v.GetString();
        return std::stod(s);
    } else {
        printf("Unknown type in getValueAsDouble()\n");
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
    } else if ( v.IsDouble() ) {
        return std::to_string(v.GetDouble());
    } else if ( v.IsString() ) {
        return v.GetString();
    }
    printf("Unknown type in getValueAsString()\n");
    return "unhandled value type";
}

bool PropertyNode::getBool( const char *name ) {
    realloc_check();
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            return getValueAsBool((*val)[name]);
        }
    }
    return false;
}

int PropertyNode::getInt( const char *name ) {
    realloc_check();
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            return getValueAsInt((*val)[name]);
        }
    }
    return 0;
}

unsigned int PropertyNode::getUInt( const char *name ) {
    realloc_check();
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            return getValueAsUInt((*val)[name]);
        }
    }
    return 0;
}

int64_t PropertyNode::getInt64( const char *name ) {
    realloc_check();
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            return getValueAsInt64((*val)[name]);
        }
    }
    return 0;
}

uint64_t PropertyNode::getUInt64( const char *name ) {
    realloc_check();
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            return getValueAsUInt64((*val)[name]);
        }
    }
    return 0;
}

double PropertyNode::getDouble( const char *name ) {
    realloc_check();
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            return getValueAsDouble((*val)[name]);
        }
    }
    return 0.0;
}

string PropertyNode::getString( const char *name ) {
    realloc_check();
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            return getValueAsString((*val)[name]);
        } else {
            return "";
        }
    }
    return "";
}

int PropertyNode::getInt( const char *name, unsigned int index ) {
    realloc_check();
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            Value &v = (*val)[name];
            if ( v.IsArray() ) {
                if ( index < v.Size() ) {
                    return getValueAsInt(v[index]);
                } else {
                    printf("index out of bounds: %s\n", name);
                }
            } else {
                printf("not an array: %s\n", name);
            }
        } else {
            // printf("no member in getUInt(%s, %d)\n", name, index);
        }
    } else {
        printf("v is not an object\n");
    }
    return 0;
}

unsigned int PropertyNode::getUInt( const char *name, unsigned int index ) {
    realloc_check();
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            Value &v = (*val)[name];
            if ( v.IsArray() ) {
                if ( index < v.Size() ) {
                    return getValueAsUInt(v[index]);
                } else {
                    printf("index out of bounds: %s\n", name);
                }
            } else {
                printf("not an array: %s\n", name);
            }
        } else {
            // printf("no member in getUInt(%s, %d)\n", name, index);
        }
    } else {
        printf("v is not an object\n");
    }
    return 0;
}

double PropertyNode::getDouble( const char *name, unsigned int index ) {
    realloc_check();
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            Value &v = (*val)[name];
            if ( v.IsArray() ) {
                if ( index < v.Size() ) {
                    return getValueAsDouble(v[index]);
                } else {
                    printf("index out of bounds: %s\n", name);
                }
            } else {
                printf("not an array: %s\n", name);
            }
        } else {
            // printf("no member in getDouble(%s, %d)\n", name, index);
        }
    } else {
        printf("v is not an object\n");
    }
    return 0.0;
}

string PropertyNode::getString( const char *name, unsigned int index ) {
    realloc_check();
    if ( val->IsObject() ) {
        if ( val->HasMember(name) ) {
            Value &v = (*val)[name];
            if ( v.IsArray() ) {
                if ( index < v.Size() ) {
                    return getValueAsString(v[index]);
                } else {
                    printf("index out of bounds: %s\n", name);
                }
            } else {
                printf("not an array: %s\n", name);
            }
        } else {
            // printf("no member in getString(%s, %d)\n", name, index);
        }
    } else {
        printf("v is not an object\n");
    }
    return "";
}

bool PropertyNode::setBool( const char *name, bool b ) {
    realloc_check();
    if ( !val->IsObject() ) {
        val->SetObject();
    }
    Value newval(b);
    if ( !val->HasMember(name) ) {
        // printf("creating %s\n", name);
        Value key(name, doc->GetAllocator());
        (*realloc_check_counter)++;
        val->AddMember(key, newval, doc->GetAllocator());
    } else {
        // printf("%s already exists\n", name);
    }
    (*val)[name] = b;
    return true;
}

bool PropertyNode::setInt( const char *name, int n ) {
    realloc_check();
    if ( !val->IsObject() ) {
        val->SetObject();
    }
    Value newval(n);
    if ( !val->HasMember(name) ) {
        // printf("creating %s\n", name);
        Value key(name, doc->GetAllocator());
        (*realloc_check_counter)++;
        val->AddMember(key, newval, doc->GetAllocator());
    } else {
        // printf("%s already exists\n", name);
    }
    (*val)[name] = n;
    return true;
}

bool PropertyNode::setUInt( const char *name, unsigned int u ) {
    realloc_check();
    if ( !val->IsObject() ) {
        val->SetObject();
    }
    Value newval(u);
    if ( !val->HasMember(name) ) {
        // printf("creating %s\n", name);
        Value key(name, doc->GetAllocator());
        (*realloc_check_counter)++;
        val->AddMember(key, newval, doc->GetAllocator());
    } else {
        // printf("%s already exists\n", name);
    }
    (*val)[name] = u;
    return true;
}

bool PropertyNode::setInt64( const char *name, int64_t n ) {
    realloc_check();
    if ( !val->IsObject() ) {
        val->SetObject();
    }
    Value newval(n);
    if ( !val->HasMember(name) ) {
        printf("creating %s\n", name);
        Value key(name, doc->GetAllocator());
        (*realloc_check_counter)++;
        val->AddMember(key, newval, doc->GetAllocator());
    } else {
        // printf("%s already exists\n", name);
    }
    (*val)[name] = n;
    return true;
}

bool PropertyNode::setUInt64( const char *name, uint64_t u ) {
    realloc_check();
    if ( !val->IsObject() ) {
        val->SetObject();
    }
    Value newval(u);
    if ( !val->HasMember(name) ) {
        // printf("creating %s\n", name);
        Value key(name, doc->GetAllocator());
        (*realloc_check_counter)++;
        val->AddMember(key, newval, doc->GetAllocator());
    } else {
        // printf("%s already exists\n", name);
    }
    (*val)[name] = u;
    return true;
}

bool PropertyNode::setDouble( const char *name, double x ) {
    realloc_check();
    if ( !val->IsObject() ) {
        val->SetObject();
    }
    Value newval(x);
    if ( !val->HasMember(name) ) {
        // printf("creating %s\n", name);
        Value key(name, doc->GetAllocator());
        (*realloc_check_counter)++;
        val->AddMember(key, newval, doc->GetAllocator());
    } else {
        // printf("%s already exists\n", name);
    }
    (*val)[name] = x;
    return true;
}

bool PropertyNode::setString( const char *name, string s ) {
    // printf("setString in %s of %s to %s\n", saved_path.c_str(), name, s.c_str());
    realloc_check();
    if ( !val->IsObject() ) {
        val->SetObject();
    }
    if ( !val->HasMember(name) ) {
        Value newval("");
        // printf("creating %s\n", name);
        Value key(name, doc->GetAllocator());
        (*realloc_check_counter)++;
        val->AddMember(key, newval, doc->GetAllocator());
    } else {
        // printf("%s already exists\n", name);
    }
    (*val)[name].SetString(s.c_str(), s.length(), doc->GetAllocator());
    return true;
}

bool PropertyNode::setInt( const char *name, int n, unsigned int index ) {
    realloc_check();
    if ( !val->IsObject() ) {
        printf("  converting value to object\n");
        // hal.scheduler->delay(100);
        val->SetObject();
    }
    if ( !val->HasMember(name) ) {
        // printf("creating %s\n", name);
        Value key(name, doc->GetAllocator());
        Value a(kArrayType);
        (*realloc_check_counter)++;
        val->AddMember(key, a, doc->GetAllocator());
    } else {
        // printf("%s already exists\n", name);
        Value &a = (*val)[name];
        if ( ! a.IsArray() ) {
            printf("converting member to array: %s\n", name);
            a.SetArray();
        }
    }
    Value &a = (*val)[name];
    extend_array(&a, index+1);    // protect against out of range
    a[index] = n;
    return true;
}

bool PropertyNode::setUInt( const char *name, unsigned int u, unsigned int index ) {
    realloc_check();
    if ( !val->IsObject() ) {
        printf("  converting value to object\n");
        // hal.scheduler->delay(100);
        val->SetObject();
    }
    if ( !val->HasMember(name) ) {
        // printf("creating %s\n", name);
        Value key(name, doc->GetAllocator());
        Value a(kArrayType);
        (*realloc_check_counter)++;
        val->AddMember(key, a, doc->GetAllocator());
    } else {
        // printf("%s already exists\n", name);
        Value &a = (*val)[name];
        if ( ! a.IsArray() ) {
            printf("converting member to array: %s\n", name);
            a.SetArray();
        }
    }
    Value &a = (*val)[name];
    extend_array(&a, index+1);    // protect against out of range
    a[index] = u;
    return true;
}

bool PropertyNode::setDouble( const char *name, double x, unsigned int index ) {
    realloc_check();
    if ( !val->IsObject() ) {
        printf("  converting value to object\n");
        // hal.scheduler->delay(100);
        val->SetObject();
    }
    if ( !val->HasMember(name) ) {
        // printf("creating %s\n", name);
        Value key(name, doc->GetAllocator());
        Value a(kArrayType);
        (*realloc_check_counter)++;
        val->AddMember(key, a, doc->GetAllocator());
    } else {
        // printf("%s already exists\n", name);
        Value &a = (*val)[name];
        if ( ! a.IsArray() ) {
            printf("converting member to array: %s\n", name);
            a.SetArray();
        }
    }
    Value &a = (*val)[name];
    extend_array(&a, index+1);    // protect against out of range
    a[index] = x;
    return true;
}

bool PropertyNode::setString( const char *name, string s, unsigned int index ) {
    realloc_check();
    if ( !val->IsObject() ) {
        printf("  converting value to object\n");
        // hal.scheduler->delay(100);
        val->SetObject();
    }
    if ( !val->HasMember(name) ) {
        // printf("creating %s\n", name);
        Value key(name, doc->GetAllocator());
        Value a(kArrayType);
        (*realloc_check_counter)++;
        val->AddMember(key, a, doc->GetAllocator());
    } else {
        // printf("%s already exists\n", name);
        Value &a = (*val)[name];
        if ( ! a.IsArray() ) {
            printf("converting member to array: %s\n", name);
            a.SetArray();
        }
    }
    Value &a = (*val)[name];
    extend_array(&a, index+1);    // protect against out of range
    a[index].SetString(s.c_str(), s.length(), doc->GetAllocator());
    return true;
}

bool PropertyNode::load_json( const char *file_path, Value *v ) {
    realloc_check();
    printf("loading from %s\n", file_path);

    int file_size = 0;
#if !defined(ARDUINO)
    // posix systems: stat() before opening the file
    struct stat st;
    if ( stat(file_path, &st) < 0 ) {
        printf("Read stat failed: %s - %d\n", file_path, errno);
        printf("does it exist?");
        return false;
    }
    file_size = (int)st.st_size;
#endif

    // open a file in read mode
#if defined(ARDUINO)
    if ( ! configfs->exists(file_path) ) {
        printf("file does not exist: %s\n", file_path);
        return false;
    }
    File open_fd = configfs->open(file_path, FILE_READ);
    if ( !open_fd ) {
        printf("file open failed: %s\n", file_path);
        return false;
    }
    file_size = open_fd.size();
#else
    // posix systems
    const int open_fd = open(file_path, O_RDONLY);
    if (open_fd == -1) {
        printf("Open failed: %s - %d\n", file_path, errno);
        return false;
    }
#endif

    printf("File size: %s: %d\n", file_path, file_size);
    void *read_buf = malloc(file_size);
    if ( read_buf == nullptr ) {
        printf("unable to malloc enough bytes for read\n");
        return false;
    }

    // read from file
#if defined(ARDUINO)
    ssize_t read_len = open_fd.read(read_buf, file_size);
#else
    size_t read_len = read(open_fd, read_buf, file_size);
#endif
    if ( read_len == -1 ) {
        printf("Read failed: %s - %d\n", file_path, errno);
        return false;
    }

    // close file after reading
#if defined(ARDUINO)
    open_fd.close();
#else
    close(open_fd);
#endif

    // printf("Read %d bytes.\nstring: %s\n", read_len, read_buf);
    // hal.scheduler->delay(100);

    // Document tmpdoc(&(doc->GetAllocator()));
    tmpdoc.Parse<kParseCommentsFlag>((char *)read_buf, read_len);
    if ( tmpdoc.HasParseError() ){
        printf("json parse err: %d (%s)\n",
               tmpdoc.GetParseError(),
               GetParseError_En(tmpdoc.GetParseError()));
        free(read_buf);
        return false;
    }

    // merge each new top level member individually
    for (Value::ConstMemberIterator itr = tmpdoc.MemberBegin(); itr != tmpdoc.MemberEnd(); ++itr) {
        printf(" merging: %s\n", itr->name.GetString());
        Value key;
        key.SetString(itr->name.GetString(), itr->name.GetStringLength(), doc->GetAllocator());
        Value &newval = tmpdoc[itr->name.GetString()];
        v->AddMember(key, newval, doc->GetAllocator());
        (*realloc_check_counter)++;
    }

    free(read_buf);
    return true;
}

// hack pseudo-implementation of a rename function.  Loads the
// original file, saves as a new file, update the mtime, and finally
// unlink the original.
static bool rename_file(const char *current_name, const char *new_name) {
    printf("Renaming %s to %s\n", current_name, new_name);

#if defined(ARDUINO)
    if ( configfs != &SD ) {
        printf("Renaming only works on SD cards (SdFat)\n");
        return false;
    }
    if ( not SD.exists(current_name) ) {
        printf("File to rename does not exist: %s\n", current_name);
        return false;
    }
    FsFile orig = SD.sdfs.open(current_name);
    if ( not orig ) {
        printf("Cannont open %s before renaming.\n", current_name);
        return false;
    }
    orig.rename(new_name);
    orig.close();
#else
    rename(current_name, new_name);
#endif

    return true;
}

static bool save_json( const char *file_path, Value *v ) {

    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    v->Accept(writer);

    // check disk space
#if defined(ARDUINO)
    // SdFat sdcard;
    // long lFreeClusters = sdcard.vol()->freeClusterCount();
    // uint64_t free_bytes = lFreeClusters * 512;  // clusters are always 512k
    uint64_t free_bytes = configfs->totalSize() - configfs->usedSize();
#elif defined(_WIN32) || defined(_WIN64)
    int free_bytes = 1000000000;
#else
    struct statfs statfs_buf;
    uint64_t free_bytes;
    if (statfs("/", &statfs_buf) != 0) {
        printf("statfs() failed\n");
        free_bytes = 0;
    } else {
        free_bytes = statfs_buf.f_bavail * statfs_buf.f_bsize;
    }
#endif
    printf("Disk free: %dk needed: %dk\n",
           (unsigned int)free_bytes / 1024,
           (unsigned int)(buffer.GetSize() / 1024) + 1);

    // open a new file in write mode
#if defined(ARDUINO)
    string new_name;
    if ( configfs == &SD ) {
        // SdFat has rename
        new_name = (string)file_path + ".new";
    } else {
        // LittleFS does not have rename
        new_name = (string)file_path;
    }
    File open_fd = configfs->open(new_name.c_str(), O_WRONLY | O_CREAT);
    if ( !open_fd ) {
        printf("file open failed: %s\n", new_name.c_str());
        return false;
    }
#else
    string new_name = (string)file_path + ".new";
    const int open_fd = ::open(new_name.c_str(), O_WRONLY | O_CREAT, 0660);
    if (open_fd == -1) {
        printf("Open %s failed: %d\n", new_name.c_str(), errno);
        return false;
    }
#endif

    // write file
    std::size_t write_size;
#if defined(ARDUINO)
    write_size = open_fd.write(buffer.GetString(), buffer.GetSize());
#else
    write_size = write(open_fd, buffer.GetString(), buffer.GetSize());
#endif
    if ( write_size != buffer.GetSize() ) {
        printf("Write failed - %s\n", strerror(errno));
        return false;
    }

    // close file
#if defined(ARDUINO)
    open_fd.close();
#else
    close(open_fd);
#endif

    // rename existing file to .bak
    string bak = (string)file_path + ".bak";
    rename_file(file_path, bak.c_str());

    // rename new file to final file name
    rename_file(new_name.c_str(), file_path);

    return true;
}

// fixme: currently no mechanism to override include values
void PropertyNode::recursively_expand_includes(string base_path, Value *v) {
    realloc_check();
    if ( v->IsObject() ) {
        if ( v->HasMember("include") && (*v)["include"].IsString() ) {
            string full_path = base_path + "/" + (*v)["include"].GetString();
            printf("Need to include: %s\n", full_path.c_str());
            load_json( full_path.c_str(), v );
            v->RemoveMember("include");
        } else {
            for (Value::MemberIterator itr = v->MemberBegin(); itr != v->MemberEnd(); ++itr) {
                if ( itr->value.IsObject() || itr->value.IsArray() ) {
                    printf("expanding: %s\n", itr->name.GetString());
                    recursively_expand_includes(base_path, &itr->value );
                }
            }
        }
    } else if ( v->IsArray() ) {
        printf("Is an array\n");
        for (Value::ValueIterator itr = v->Begin(); itr != v->End(); ++itr) {
            if ( itr->IsObject() || itr->IsArray() ) {
                printf("recurse array\n");
                recursively_expand_includes(base_path, itr );
            }
        }
    }
}

bool PropertyNode::load( const char *file_path ) {
    realloc_check();
    if ( !load_json(file_path, val) ) {
        return false;
    }
    string full_path = file_path;
    size_t pos = full_path.rfind("/");
    string base_path = "";
    if ( pos > 0 && pos != string::npos ) {
        base_path = full_path.substr(0, pos);
    }
    recursively_expand_includes(base_path, val);

    // printf("Updated node contents:\n");
    // pretty_print();
    // printf("\n");

    return true;
}

bool PropertyNode::save( const char *file_path ) {
    realloc_check();
    if ( !save_json(file_path, val) ) {
        return false;
    }
    printf("json file saved: %s\n", file_path);
    return true;
}

// void PropertyNode::print() {
//     StringBuffer buffer;
//     Writer<StringBuffer> writer(buffer);
//     val->Accept(writer);
//     //const char* output = buffer.GetString();
//     printf("%s\n", buffer.GetString());
// }

void PropertyNode::pretty_print() {
    realloc_check();
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    //PrettyWriter<StringBuffer, UTF8<>, UTF8<>, CrtAllocator, kWriteNanAndInfFlag> writer(buffer);
    //Writer<StringBuffer, UTF8<>, UTF8<>, CrtAllocator, kWriteNanAndInfFlag> writer(buffer);

    // bool error = val->Accept(writer);
    val->Accept(writer);

    // work around size limitations
    // printf("buffer length: %d\n", buffer.GetSize());
    const char *ptr = buffer.GetString();
    for ( unsigned int i = 0; i < buffer.GetSize(); i++ ) {
        printf("%c", ptr[i]);
    }
    printf("\n");

    // if ( error ) {
    //     printf("json formating errro (nan or inf?)\n");
    // }
}

string PropertyNode::get_json_string() {
    realloc_check();
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    val->Accept(writer);
    // string result = "";
    // const char *ptr = buffer.GetString();
    // // printf("buffer length: %d\n", buffer.GetSize());
    // for ( unsigned int i = 0; i < buffer.GetSize(); i++ ) {
    //     result += ptr[i];
    // }
    // return result;
    return buffer.GetString();
}

// Note: this doesn't recursively update individual values in the whole subtree
// ... it really only works correctly for flat subnodes with no sub children
// nodes. Child nodes (subtrees) get overwritten by the new subtree.  This means
// local changes to the subtree will be lost and it's also a big ugly memory leak.
//
// That said, you can send whole trees if you understand the caveates.
//
// Todo make recursive
bool PropertyNode::set_json_string( string message ) {
    // Document tmpdoc(&(doc->GetAllocator()));
    tmpdoc.Parse<kParseCommentsFlag>(message.c_str(), message.length());
    if ( tmpdoc.HasParseError() ){
        printf("json parse err: %d (%s)\n",
               tmpdoc.GetParseError(),
               GetParseError_En(tmpdoc.GetParseError()));
        return false;
    }

    // // merge each new top level member individually
    // for (Value::ConstMemberIterator itr = tmpdoc.MemberBegin(); itr != tmpdoc.MemberEnd(); ++itr) {
    //     // printf(" merging: %s\n", itr->name.GetString());
    //     Value key;
    //     key.SetString(itr->name.GetString(), itr->name.GetStringLength(), doc->GetAllocator());
    //     Value &newval = tmpdoc[itr->name.GetString()];
    //     if ( val->HasMember(key) ) {
    //         if ( (*val)[itr->name.GetString()].IsObject() && newval->IsObject() ) {
    //             // recurse
    //         } else {
    //             // set val
    //             (*val)[itr->name.GetString()] = newval;
    //         }
    //     } else {
    //         val->AddMember(key, newval, doc->GetAllocator());
    //     }
    // }

    recursive_tree_copy(tmpdoc, (*val), "");

    (*realloc_check_counter)++;  // force refinding node paths

    return true;
}

void PropertyNode::recursive_tree_copy(Value &src, Value &dst, string indent) {
    // merge each new top level member individually
    for (Value::ConstMemberIterator itr = src.MemberBegin(); itr != src.MemberEnd(); ++itr) {
        // printf("%smerging: %s\n", indent.c_str(), itr->name.GetString());
        Value &newval = src[itr->name.GetString()];
        if ( dst.HasMember(itr->name.GetString()) ) {
            // printf("%s exists\n", indent.c_str());
            if ( dst[itr->name.GetString()].IsObject() && newval.IsObject() ) {
                // recurse
                // printf("%s recurse\n", indent.c_str());
                recursive_tree_copy(newval, dst[itr->name.GetString()], indent + " ");
            } else {
                // set value
                // printf("%s set val\n", indent.c_str());
                dst[itr->name.GetString()] = newval;
            }
        } else {
            // printf("%s new add\n", indent.c_str());
            Value key;
            key.SetString(itr->name.GetString(), itr->name.GetStringLength(), doc->GetAllocator());
            dst.AddMember(key, newval, doc->GetAllocator());
        }
    }
}

Document *PropertyNode::doc = nullptr;
int *PropertyNode::realloc_check_counter = nullptr;
Document PropertyNode::tmpdoc;

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
