/*
Copyright (c) 2021 Timothy Rule
MIT License
*/

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <yaml.h>
#include <simple_yaml.h>


SimpleYamlNode* simple_yaml_create_node(char* name, SimpleYamlNode* parent)
{
    SimpleYamlNode* node = calloc(1, sizeof(SimpleYamlNode));
    node->parent = parent;
    node->node_type = YAML_NO_NODE;
    if (name) node->name = strdup(name);
    if (parent) {
        if (parent->node_type == YAML_MAPPING_NODE) {
            assert(node->name);
            hashmap_set(&parent->mapping, node->name, node);
        } else if (parent->node_type == YAML_SEQUENCE_NODE) {
            hashlist_append(&parent->sequence, node);
        }
    }
    return node;
}

void simple_yaml_set_mapping(SimpleYamlNode* node)
{
    assert(node->node_type == YAML_NO_NODE);
    node->node_type = YAML_MAPPING_NODE;
    hashmap_init(&node->mapping);
}

void simple_yaml_set_sequence(SimpleYamlNode* node)
{
    assert(node->node_type == YAML_NO_NODE);
    node->node_type = YAML_SEQUENCE_NODE;
    hashlist_init(&node->sequence);
}

void simple_yaml_set_scalar(SimpleYamlNode* node, const char* value)
{
    assert(node->node_type == YAML_NO_NODE);
    node->node_type = YAML_SCALAR_NODE;
    node->value = strdup(value);
}

void simple_yaml_destroy_node(SimpleYamlNode* node)
{
    if (node == NULL) return;
    /* Destroy any contained nodes. */
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
    /* Destroy _this_ node. */
    free(node->name);
    free(node->value);
    free(node);
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
        doc_list = calloc(1, sizeof(HashList));
        if (doc_list == NULL) {
            perror("Error creating document list");
            return doc_list;  /* NULL */
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
                }
                doc = node = NULL;  /* Reset the document pointers. */
                break;
            /* Node events. */
            case YAML_SCALAR_EVENT:
                if (node->node_type == YAML_MAPPING_NODE
                        || node->node_type == YAML_SEQUENCE_NODE) {
                    /* Create a child node (will be attached to the collection),
                    and set the key. At this point the node_type is not known. */
                    node = simple_yaml_create_node(
                            (char*)event.data.scalar.value, node);
                } else {
                    /* The child node is scalar, set the node_type and value. */
                    simple_yaml_set_scalar(node, (char*)event.data.scalar.value);
                    node = node->parent;
                }
                break;
            case YAML_MAPPING_START_EVENT:
                if (node == NULL) {
                    /* This is the root node of the document. */
                    assert(doc == NULL);
                    node = simple_yaml_create_node(NULL, node);
                    doc = node;
                }
                if (node->node_type == YAML_SEQUENCE_NODE) {
                    /* This mapping is an item of the parent sequence, create
                    a node and append to the sequence. */
                    node = simple_yaml_create_node(NULL, node);
                }
                simple_yaml_set_mapping(node);
                break;
            case YAML_SEQUENCE_START_EVENT:
                if (node == NULL) {
                    /* This is the root node of the document. */
                    assert(doc == NULL);
                    node = simple_yaml_create_node(NULL, node);
                    doc = node;
                }
                simple_yaml_set_sequence(node);
                break;
            case YAML_MAPPING_END_EVENT:
            case YAML_SEQUENCE_END_EVENT:
                node = node->parent;
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
    char* _path = strdup(path);  /* strtok() is destructive, work from copy. */
    char* token = strtok(_path, "/");

    while (token && node) {
        if (node->node_type == YAML_SEQUENCE_NODE) {
            free(_path);
            return NULL;
        }
        if (node->node_type == YAML_MAPPING_NODE) {
            node = hashmap_get(&node->mapping, token);
        }
        token = strtok(NULL, "/");
    }
    free(_path);
    return node;
}

int simple_yaml_get_value_as_bool(SimpleYamlNode* node, bool* value)
{
    assert(node);
    assert(value);
    if (node->node_type != YAML_SCALAR_NODE) return EINVAL;
    const char** p;
    /* Bool true? */
    const char* bool_true[] = {
        "y", "Y", "yes", "Yes", "YES", "true", "True", "TRUE", "on", "On", "ON", NULL};
    for (p = bool_true; *p; p++) {
        if (strcmp(*p, node->value)) continue;
        *value = true;
        return 0;
    }
    /* Bool false? */
    const char* bool_false[] = {
        "n", "N", "no", "No", "NO", "false", "False", "FALSE", "off", "Off", "OFF", NULL};
    for (p = bool_false; *p; p++) {
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


int main(void)
{
    HashList* doc_list;
    doc_list = simple_yaml_parse_file("sample.yaml", NULL);
    for (uint32_t i = 0; i < hashlist_length(doc_list); i++) {
        SimpleYamlNode* doc = hashlist_get_at(doc_list, i);
        SimpleYamlNode* kind_node = simple_yaml_find_node(doc, "kind");
        SimpleYamlNode* name_node = simple_yaml_find_node(doc, "metadata/name");
        SimpleYamlNode* app_node = simple_yaml_find_node(doc, "spec/selector/app");
        printf("Document %d\n", i);
        printf("  %s = %s\n", kind_node->name, kind_node->value);
        printf("  %s = %s\n", name_node->name, name_node->value);
        if (app_node) printf("  %s = %s\n", app_node->name, app_node->value);

        SimpleYamlNode* ports_node = simple_yaml_find_node(doc, "spec/ports");
        if (ports_node == NULL) continue;
        if (ports_node->node_type != YAML_SEQUENCE_NODE) continue;
        for (uint32_t port_index = 0; port_index < hashlist_length(&ports_node->sequence); port_index++) {
            SimpleYamlNode* port_node = hashlist_get_at(&ports_node->sequence, port_index);
            SimpleYamlNode* _n = simple_yaml_find_node(port_node, "targetPort");
            if (_n) printf("  %s = %s\n", _n->name, _n->value);
        }
    }

    for (uint32_t i = 0; i < hashlist_length(doc_list); i++) {
        SimpleYamlNode* doc = hashlist_get_at(doc_list, i);
        simple_yaml_destroy_node(doc);
    }
    hashlist_destroy(doc_list);
    free(doc_list);

    exit(0);
}
