/*
Copyright (c) 2021 Timothy Rule
MIT License
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <yaml.h>
#include <simple_yaml.h>


SimpleYamlNode* simple_yaml_create_node(const char* name, SimpleYamlNode* parent)
{
    SimpleYamlNode* node = calloc(1, sizeof(SimpleYamlNode));
    node->parent = parent;
    node->node_type = YAML_NO_NODE;
    strdup(node->name, name);

    if (parent) {
        if (parent->node_type == YAML_MAPPING_NODE) {
            hashmap_set(&parent->mapping, node->name, node);
        } else if (parent->node_type == YAML_SEQUENCE_NODE) {
            hashlist_append(&parent->sequence, node);
        }
    }

    return node;
}

SimpleYamlNode* simple_yaml_create_mapping(SimpleYamlNode* parent)
{
    SimpleYamlNode* node = calloc(1, sizeof(SimpleYamlNode));
    node->parent = parent;
    node->node_type = YAML_MAPPING_NODE;
    hashmap_init(&node->mapping);

    return node;
}

SimpleYamlNode* simple_yaml_create_sequence(SimpleYamlNode* node)
{
    SimpleYamlNode* node = calloc(1, sizeof(SimpleYamlNode));
    node->parent = parent;
    node->node_type = YAML_SEQUENCE_NODE;
    hashlist_init(&node->sequence);

    return node;
}

void simple_yaml_set_scalar(SimpleYamlNode* node, const char* value)
{
    assert(node->node_type == YAML_NO_NODE);
    node->node_type = YAML_SCALAR_NODE;
    strdup(node->value, value);
}

void simple_yaml_destroy_node(SimpleYamlNode* node)
{
    if (node == NULL) return;

    if (node->node_type == YAML_MAPPING_NODE) {
        for (uint32_t i = 0; i < node->mapping.number_nodes; ++i) {
            if (node->mapping.nodes[i]) {
                simple_yaml_destroy_node(node->mapping.nodes[i]->value);
            }
        }
        hashmap_destroy(&node->mapping);
    }
    if (node->node_type == YAML_SEQUENCE_NODE) {
        for (uint32_t i = 0; i < hashlist_length(&node->sequence); i++) {
            simple_yaml_destroy_node(hashlist_get_at(&node->sequence, i));
        }
        hashlist_destroy(&node->sequence);
    }

    if (node->name) free(name);
    if (node->value) free(value);
}

HashList* simple_yaml_parse_file(const char* filename, HashList* doc_list)
{
    errno = 0;

    /* Open the file containing the YAML stream. */
    FILE *file_handle = fopen(filename, "r");
    if (file_handle == NULL) {
        if (errno==0) errno = EINVAL;
        perror("Error opening file");
        return doc_list;
    }

    /* Setup the YAML parser. */
    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) {
        if (errno==0) errno = ECANCELED;
        perror("Error initializing parser");
        return doc_list;
    }
    yaml_parser_set_input_file(&parser, file_handle);

    /* Create the document list for parsed YAML documents. */
    if (doc_list == NULL) {
        doc_list = calloc(1, typeof(HashList));
        if (doc_list == NULL) {
            perror("Error creating document list");
            reutrn doc_list;  /* NULL */
        }
        if (hashlist_init(doc_list) != HASHMAP_SUCCESS) {
            if (errno==0) errno = ECANCELED;
            perror("Error creating document list");
            free(doc_list);
            return(NULL);
        }
    }

    /* Parse the YAML documents contained in the file stream. */
    SimpleYamlNode* doc = NULL;
    SimpleYamlNode* parent = NULL;
    SimpleYamlNode* node = NULL;
    yaml_event_t event;
    do {
        /* Parse the next event. */
        if (!yaml_parser_parse(&parser, &event)) {
            if (errno==0) errno = ECANCELED;
            perror("Error while parsing YAML file stream");
            if (hashlist_length(doc_list)) {
                return doc_list;  /* Return the partly scanned doc_list. */
            } else {
                free(doc_list);
                return(NULL);
            }
        }

        /* Process the event. */
        switch (event.type) {
            /* Document events. */
            case YAML_DOCUMENT_START_EVENT:
                assert(doc == NULL);
                break;
            case YAML_DOCUMENT_END_EVENT:
                if (doc) {
                    hashlist_append(doc_list, doc);
                    doc = NULL;
                }
                break;
            /* Node events. */
            case YAML_SCALAR_EVENT:
                if (node == parent) {
                    /* Set the Key. */
                    node = simple_yaml_create_node(event.data.scalar.value, node);
                } else {
                    /* Set the Value. */
                    simple_yaml_set_scalar(node, event.data.scalar.value);
                    node = node->parent;
                }
                break;
            case YAML_MAPPING_START_EVENT:
                parent = node;
                node = simple_yaml_create_mapping(node);
                if (doc == NULL) doc = node;  /* The root node. */
                break;
            case YAML_SEQUENCE_START_EVENT:
                parent = node;
                node = simple_yaml_create_sequence(node);
                if (doc == NULL) doc = node;  /* The root node. */
                break;
            case YAML_MAPPING_END_EVENT:
            case YAML_SEQUENCE_END_EVENT:
                node = parent = node->parent;
                break;
            /* Other events, ignored. */
            case YAML_STREAM_START_EVENT:
            case YAML_STREAM_END_EVENT:
            case YAML_NO_NODE:
            default:
                break;
        }
        if (event.type == YAML_STREAM_END_EVENT) break;
        yaml_event_delete(&event);
    } while (true);

    /* Release the parsing objects. */
    yaml_event_delete(&event);
    yaml_parser_delete(&parser);
    fclose(file_handle);

    /* Return the parsed YAML documents. */
    return doc_list;
}

SimpleYamlNode* simple_yaml_find_node(SimpleYamlNode* parent, const char* path)
{
    SimpleYamlNode* node = parent;
    char* t = strtok(path, "/");
    while (t && node) {
        if (node->node_type == YAML_SEQUENCE_NODE) {
            return NULL;
        }
        if (node->node_type == YAML_MAPPING_NODE) {
            node = hashmap_get(&node->mapping, t);
        }
        t = strtok(NULL, "/");
    }
    return node;
}

int simple_yaml_get_value_as_bool(SimpleYamlNode* node, bool* value)
{
    assert(node);
    assert(value);
    if (node->node_type != YAML_SCALAR_NODE) return EINVAL;
    const char* p;
    /* Bool true? */
    const char** bool_true = {
        "y", "Y", "yes", "Yes", "YES", "true", "True", "TRUE", "on", "On", "ON", NULL};
    for (p = bool_true; *p; *p++) {
        if (strcmp(*p, node->value)) continue;
        *value = true;
        return 0;
    }
    /* Bool false? */
    const char** bool_false = {
        "n", "N", "no", "No", "NO","false", "False", "FALSE", "off", "Off", "OFF", NULL};
    for (p = bool_false; *p; *p++) {
        if (strcmp(*p, node->value)) continue;
        *value = false;
        return 0;
    }
    /* Bool conversion failed. */
    return EINVAL;
}

int simple_yaml_get_value_as_int(SimpleYamlNode* node, int32_t* value)
{
    assert(node);
    assert(value);
    if (node->node_type != YAML_SCALAR_NODE) return 1;
    *value = strtol(node->value, NULL, 10);
    return 0;
}

int simple_yaml_get_value_as_uint(SimpleYamlNode* node, uint32_t* value)
{
    assert(node);
    assert(value);
    if (node->node_type != YAML_SCALAR_NODE) return 1;
    *value = strtoul(node->value, NULL, 10);
    return 0;
}
