/*
Copyright (c) 2021 Timothy Rule
MIT License
*/

#include <simple_yaml.h>


SimpleYamlNode* simple_yaml_create_node(const char* name, SimpleYamlNode* parent)
{
    SimpleYamlNode* node = calloc(1, sizeof(SimpleYamlNode));
    assert(node);
    node->parent = parent;
    node->node_type = YAML_NO_NODE;
    strdup(node->name, name);

    // TODO depending on the parent node_type, add the node to the storage.

    return node;
}

void simple_yaml_set_scalar(SimpleYamlNode* node, const char* value)
{
    assert(node->node_type == YAML_NO_NODE);
    node->node_type = YAML_SCALAR_NODE;
    strdup(node->value, value);
}

void simple_yaml_set_mapping(SimpleYamlNode* node)
{
    assert(node->node_type == YAML_NO_NODE);
    node->node_type = YAML_MAPPING_NODE;

    // TODO init the object storage.
}

void simple_yaml_set_sequence(SimpleYamlNode* node)
{
    assert(node->node_type == YAML_NO_NODE);
    node->node_type = YAML_SEQUENCE_NODE;

    // TODO init the object storage.
}

void simple_yaml_destroy_node(SimpleYamlNode* node)
{

}

SimpleYamlNode* simple_yaml_get_node(SimpleYamlNode* parent, const char* path)
{
    // TODO find the node using parent/foo/bar/key search pattern.
}
