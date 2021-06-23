#pragma once

#if defined(ARDUPILOT_BUILD)
#  undef _GLIBCXX_USE_C99_STDIO   // vsnprintf() not defind
#endif

#include <stdio.h>

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "rapidjson/document.h"
#include "rapidjson/pointer.h"
using namespace rapidjson;

//
// property system style interface with a rapidjson document as the backend
//

extern Document doc;

class PropertyNode
{
public:
    // Constructor.
    PropertyNode();
    PropertyNode(string abs_path, bool create=true);
    PropertyNode(Value *v);

    // Destructor.
    // ~PropertyNode();

    // operator =
    // PropertyNode & operator= (const PropertyNode &node);

    bool hasChild(const char *name );
    PropertyNode getChild( const char *name, bool create=true );
    PropertyNode getChild( const char *name, int index, bool create=true );

    bool isNull();		// return true if pObj pointer is NULL
    
    int getLen( const char *name ); // return len if node is an array (else 0)
    void setLen(int size); // set array len of node, extend if necessary
    void setLen(int size, double init_val); // set array size, extend and init

    vector<string> getChildren(bool expand=true); // return list of children

    bool isLeaf( const char *name); // return true if pObj/name is leaf
    
    // value getters
    bool getBool( const char *name );	  // return value as a bool
    int getInt( const char *name );	  // return value as an int
    unsigned int getUInt( const char *name ); // return value as an unsigned int
    float getFloat( const char *name );   // return value as a float
    double getDouble( const char *name ); // return value as a double
    string getString( const char *name ); // return value as a string

    // indexed value getters
    bool getBool( const char *name, int index ); // return value as a bool
    int getInt( const char *name, int index ); // return value as an int
    unsigned int getUInt( const char *name, int index ); // return value as an unsigned int
    float getFloat( const char *name, int index ); // return value as a float
    double getDouble( const char *name, int index ); // return value as a double
    string getString( const char *name, int index ); // return value as a string

    // value setters
    bool setBool( const char *name, bool b ); // returns true if successful
    bool setInt( const char *name, int n );     // returns true if successful
    bool setUInt( const char *name, unsigned int u ); // returns true if successful
    bool setFloat( const char *name, float x ); // returns true if successful
    bool setDouble( const char *name, double x ); // returns true if successful
    bool setString( const char *name, string s ); // returns true if successful

    // indexed value setters
    bool setFloat( const char *name, int index, float x ); // returns true if successful

    // load/merge json file under this node
    bool load( const char *file_path );
    
    // void print();
    void pretty_print();

    Value *get_valptr() {  return val; }
    
private:
    // Pointer p;
    Value *val = nullptr;
};
