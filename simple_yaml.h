/*
Copyright (c) 2021 Timothy Rule
MIT License
*/

#include <stdbool.h>
#include <stdint.h>
#include <yaml.h>
#include <hashmap.h>
#include <hashlist.h>


typedef struct SimpleYamlNode SimpleYamlNode;

typedef struct SimpleYamlNode  {
    const char*         name;
    yaml_node_type_t    node_type;
    /* Node storage. */
    const char*         value;
    HashMap             mapping;
    HashList            sequence;
    /* Document structure. */
    SimpleYamlNode*     parent;
} SimpleYamlNode;


SimpleYamlNode* simple_yaml_create_node(const char* name, SimpleYamlNode* parent);
void simple_yaml_destroy_node(SimpleYamlNode* node);

void simple_yaml_set_scalar(SimpleYamlNode* node, const char* value);
void simple_yaml_set_mapping(SimpleYamlNode* node);
void simple_yaml_set_sequence(SimpleYamlNode* node);

SimpleYamlNode* simple_yaml_find_node(SimpleYamlNode* parent, const char* path);
int simple_yaml_get_value_as_bool(SimpleYamlNode* node, bool* value);
int simple_yaml_get_value_as_int(SimpleYamlNode* node, int32_t* value);
int simple_yaml_get_value_as_uint(SimpleYamlNode* node, uint32_t* value);

HashList* simple_yaml_parse_file(const char* filename, HashList* doc_list);
