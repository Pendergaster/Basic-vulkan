/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef OBJLOAD_H
#define OBJLOAD_H

#include "utils.h"
#include "vertex.h"
#include "cmath.h"
#include "dynamicArray.h"
#include "hash_table.h"

typedef struct Vertex {
    vec3    pos;
    vec3    color;
    vec2    uv;
} Vertex;

typedef struct VertexLoadData {
    Vertex*     vertexes;
    int*        indexes;
    int         numVertexes;
    int         numIndexes;
} VertexLoadData;

typedef struct ObjElementIndex {
    int vert;
    int uv;
} ObjElementIndex;

DECLARE_HASHTABLEKEY(Vertex, Vertex);
DECLARE_HASHTABLE( VertexKey, Vertex, vertex);

static VertexLoadData
obj_load(const char* name) {

    VertexLoadData ret = {};

    vec3* vertexBuffer = (vec3*)dynamicarray_create(sizeof(vec3));
    vec2* uvBuffer = (vec2*)dynamicarray_create(sizeof(vec2));
    ObjElementIndex* indexBuffer = (ObjElementIndex*)dynamicarray_create(sizeof(ObjElementIndex));

    FILE* f = fopen(name, "r");
    if(!f) {
        ABORT("Failed to open model file");
    }

    char buffer[255];
    int end = fscanf(f, "%s", buffer);
    while (end != EOF) {
        if (!strcmp("v", buffer)) {
            // read vertex
            vec3 temp;
            int matches = fscanf(f, "%f %f %f\n", &temp.x, &temp.y, &temp.z);
            ASSERT_MESSAGE(matches == 3,"Vertex does not have enough matches!");
            dynamicarray_push_back(vertexBuffer, &temp);
        }

        if (!strcmp("vt", buffer)) {
            // read vertex
            vec2 temp;
            int matches = fscanf(f, "%f %f\n", &temp.x, &temp.y);
            ASSERT_MESSAGE(matches == 2,"Uv does not have enough matches!");
            temp.y = 1.f - temp.y;
            dynamicarray_push_back(uvBuffer, &temp);
        }

        if (!strcmp("f", buffer)) {
            // read index buffer for 3 vertexes
            ObjElementIndex temp;
            for(u32 i = 0 ; i < 3; i++) {
                int matches = fscanf(f, "%d/%d\n", &temp.vert, &temp.uv);
                ASSERT_MESSAGE(matches == 2,"Indexes does not have enough matches!");
                dynamicarray_push_back(indexBuffer, &temp);
            }
        }
        end = fscanf(f, "%s", buffer);
    }
    u32 numIndexes = dynamicarray_size(indexBuffer);
    ret.vertexes = malloc(sizeof(Vertex) * numIndexes);
    ret.indexes = malloc(sizeof(int) * numIndexes);
    ret.numIndexes = numIndexes;

    VertexHashTable table = {};
    ASSERT_MESSAGE(hashmap_find_primeindex(500) == 4, "Failed to find map prime");
    ASSERT_MESSAGE(hashmap_find_primeindex(3000) == 6, "Failed to find map prime");
    ASSERT_MESSAGE(hashmap_find_primeindex(1300) == 5, "Failed to find map prime");
    vertex_hashtable_init(&table, sizeof(int), hashmap_find_primeindex(numIndexes));

    u32 numVertexes = 0;
#if 1
    for(u32 i = 0; i < numIndexes; i++) {
        Vertex vert = {
            .pos = vertexBuffer[indexBuffer[i].vert - 1],
            .uv = uvBuffer[indexBuffer[i].uv - 1]
        };
        int* index = vertex_hashtable_access(&table, vert);
        int realIndex;
        if(index == NULL) {
            // Not found yet and store for later findings
            ret.vertexes[numVertexes] = vert;
            realIndex = numVertexes;
            vertex_hashtable_insert(&table, vert, (u8*)&realIndex);
            numVertexes++;
        } else {
            realIndex = *index;
        }
        ret.indexes[i] = realIndex;
    }
#else
    for(u32 i = 0; i < numIndexes; i++) {
        //printf("index %d %d\n", indexBuffer[i].vert , indexBuffer[i].uv);fflush(stdout);
        //if(indexBuffer[i].vert == 0 || i > 100) ABORT("RE");
        ret.vertexes[i].pos = vertexBuffer[indexBuffer[i].vert - 1];
        ret.vertexes[i].uv = uvBuffer[indexBuffer[i].uv - 1];
        ret.indexes[i] = i;
    }
#endif
    ret.numVertexes = numVertexes;

    dynamicarray_dispose(vertexBuffer);
    dynamicarray_dispose(indexBuffer);
    dynamicarray_dispose(uvBuffer);

    fclose(f);
    return ret;
}

// TODO free obj memory
// TODO map memory
#endif /* OBJLOAD_H */
