/*
Copyright (c) 2021 Timothy Rule
MIT License
*/

#include <yaml>
#include <barrust/hashmap.h>


typedef struct SimpleYamlNode SimpleYamlNode;

typedef struct SimpleYamlNode  {
    const char*         name;
    yaml_node_type_t    node_type;
    /* Node storage. */
    const char*         value;
    HashMap             objects;
    /* Document structure. */
    SimpleYamlNode*     parent;
} SimpleYamlNode;


SimpleYamlNode* simple_yaml_create_node(const char* name, SimpleYamlNode* parent);
void simple_yaml_destroy_node(SimpleYamlNode* node);

void simple_yaml_set_scalar(SimpleYamlNode* node, const char* value);
void simple_yaml_set_mapping(SimpleYamlNode* node);
void simple_yaml_set_sequence(SimpleYamlNode* node);

SimpleYamlNode* simple_yaml_get_node(SimpleYamlNode* parent, const char* path);
