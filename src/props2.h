#pragma once

#if defined(ARDUINO)
#  include <Arduino.h>
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

class SharedStateWrapper {
public:
    Document *doc;
    int *realloc_check_counter;
};

class PropertyNode {

public:
    // Constructor.
    PropertyNode();
    PropertyNode(string abs_path, bool create=true);
    //PropertyNode(Value *v);

    bool hasChild(const char *name );
    PropertyNode getChild( const char *name, bool create );
    PropertyNode getChild( const char *name ) {
        return getChild( name, true );
    }

    bool isNull();		// return true if value pointer is NULL
    bool isParent(const char *name);
    bool isArray(const char *name);
    bool isValue(const char *name);
    bool isValue(const char *name, unsigned int index);
    int getLen( const char *name ); // return len if node is an array (else 0)

    vector<string> getChildren(bool expand=true); // return list of children

    // value getters
    bool getBool( const char *name );         // return value as a bool
    int getInt( const char *name );           // return value as an int
    unsigned int getUInt( const char *name ); // return value as an unsigned int
    int64_t getInt64( const char *name );     // return value as an int64_t
    uint64_t getUInt64( const char *name );   // return value as an uint64_t
    double getDouble( const char *name );     // return value as a double
    string getString( const char *name );     // return value as a string

    // indexed value getters
    bool getBool( const char *name, unsigned int index ); // return value as a bool
    int getInt( const char *name, unsigned int index ); // return value as an int
    unsigned int getUInt( const char *name, unsigned int index ); // return value as an unsigned int
    double getDouble( const char *name, unsigned int index ); // return value as a double
    string getString( const char *name, unsigned int index ); // return value as a string

    // value setters
    bool setBool( const char *name, bool b ); // returns true if successful
    bool setInt( const char *name, int n );     // returns true if successful
    bool setUInt( const char *name, unsigned int u ); // returns true if successful
    bool setInt64( const char *name, int64_t n );     // returns true if successful
    bool setUInt64( const char *name, uint64_t u ); // returns true if successful
    bool setDouble( const char *name, double x ); // returns true if successful
    bool setString( const char *name, string s ); // returns true if successful

    // indexed value setters
    bool setInt( const char *name, int n, unsigned int index ); // returns true if successful
    bool setUInt( const char *name, unsigned int u, unsigned int index ); // returns true if successful
    bool setDouble( const char *name, double x, unsigned int index ); // returns true if successful
    bool setString( const char *name, string s, unsigned int index ); // returns true if successful

    // load/merge json file under this node
    bool load( const char *file_path );

    // save contents of node as a json file
    bool save( const char *file_path );

    // void print();
    void pretty_print();
    string get_json_string();
    bool set_json_string(string message);

    SharedStateWrapper get_shared_state() {
        init_shared_state();
        SharedStateWrapper d;
        d.doc = doc;
        d.realloc_check_counter = realloc_check_counter;
        return d;
    }

    void set_shared_state( SharedStateWrapper d ) {
        doc = d.doc;
        realloc_check_counter = d.realloc_check_counter;
    }

private:
    // shared state instances
    static Document *doc;
    static Document tmpdoc; // can be statically allocated I think
    static int *realloc_check_counter;

    // pointer to rapidjson Object;
    Value *val = nullptr;
    string saved_path;
    int saved_realloc_check_counter;

    inline void init_shared_state() {
        // printf("init_shared_state()\n");
        if ( doc == nullptr ) {
            printf("  doc is nullptr, creating new document\n");
            doc = new Document;
        }
        if ( realloc_check_counter == nullptr ) {
            printf("  realloc_check_counter is nullptr, creating new realloc_check_counter\n");
            realloc_check_counter = new int;
            *realloc_check_counter = 0;
        }
    }
    bool extend_array(Value *node, int size);
    Value *find_node_from_path(Value *start_node, string path, bool create);
    void recursive_tree_copy(Value &src, Value &dst, string indent);
    void realloc_check();
    bool load_json( const char *file_path, Value *v );
    void recursively_expand_includes(string base_path, Value *v);
};
