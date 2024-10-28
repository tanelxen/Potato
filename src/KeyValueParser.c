//
//  KeyValueParser.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 28.10.24.
//

#include "KeyValueParser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --------------------------------------------------------------------- */

/*
 * tiny lexer for the quake 3 entity syntax
 *
 * {
 * "key1" "value1"
 * "key2" "value2"
 * }
 * {
 * "key1" "value1"
 * "key2" "value2"
 * }
 * ...
 */

struct vec_header
{
    int n;
    int cap;
};

/* cover your eyes, don't look at the macro abominations */

#define vec_hdr(v) (v ? ((struct vec_header*)(v) - 1) : 0)
#define vec_len(v) (v ? vec_hdr(v)->n : 0)
#define vec_cap(v) (v ? vec_hdr(v)->cap : 0)
#define vec_grow(v, n) (*(void**)&(v) = vec_fit(v, n, sizeof((v)[0])))
#define vec_reserve(v, n) ( \
    vec_grow(v, vec_len(v) + n), \
    &(v)[vec_len(v)] \
)
#define vec_append(v, x) (*vec_append_p(v) = (x))
#define vec_append_p(v) ( \
    vec_reserve(v, 1), \
    &(v)[vec_hdr(v)->n++] \
)
#define vec_cat(v, array, array_size) ( \
    memcpy(vec_reserve(v, array_size), array, array_size * sizeof(v)[0]), \
    vec_hdr(v)->n += array_size \
)
#define vec_clear(v) (v ? vec_hdr(v)->n = 0 : 0)
#define vec_free(v) (free(vec_hdr(v)), v = 0)

#define max(a, b) (a > b ? a : b)

void* vec_fit(void* v, int n, int element_size)
{
    struct vec_header* hdr;

    hdr = vec_hdr(v);

    if (!hdr || max(n, vec_len(v)) >= vec_cap(v))
    {
        int new_cap;
        int alloc_size;

        new_cap = max(n, vec_cap(v) * 2);
        alloc_size = sizeof(struct vec_header) + new_cap * element_size;

        if (hdr) {
            hdr = realloc(hdr, alloc_size);
        } else {
            hdr = malloc(alloc_size);
            hdr->n = 0;
        }

        hdr->cap = new_cap;
    }

    return hdr + 1;
}

enum entities_token
{
    ENTITIES_LAST_LITERAL_TOKEN = 255,
    ENTITIES_STRING,
    ENTITIES_LAST_TOKEN
};

void describe_entities_token(char* dst, int dst_size, int kind)
{
    switch (kind)
    {
    case ENTITIES_STRING:
        snprintf(dst, dst_size, "%s", "string");
        break;

    default:
        if (kind >= 0 && kind <= ENTITIES_LAST_LITERAL_TOKEN) {
            snprintf(dst, dst_size, "'%c'", (char)kind);
        } else {
            snprintf(dst, dst_size, "unknown (%d)", kind);
        }
    }
}

struct entities_lexer
{
    char* p;
    int token_kind;
    char* str;
    int n_lines;
};

int lex_entities(struct entities_lexer* lex)
{
again:
    switch (*lex->p)
    {
    case '\n':
        ++lex->n_lines;
    case '\t':
    case '\v':
    case '\f':
    case '\r':
    case ' ':
        ++lex->p;
        goto again;

    case '"':
        lex->token_kind = ENTITIES_STRING;
        ++lex->p;
        lex->str = lex->p;

        for (; *lex->p != '"'; ++lex->p)
        {
            if (!*lex->p)
            {
                printf("W: unterminated string \"%s\" at line %d",
                    lex->str, lex->n_lines);
                break;
            }
        }

        if (*lex->p) {
            *lex->p++ = 0;
        }
        break;

    default:
        lex->token_kind = (int)*lex->p;
        ++lex->p;
    }

    return lex->token_kind;
}

int entities_expect(struct entities_lexer* lex, int kind)
{
    if (lex->token_kind != kind)
    {
        char got[64];
        char exp[64];

        describe_entities_token(got, sizeof(got), lex->token_kind);
        describe_entities_token(exp, sizeof(exp), kind);

        printf("W: got %s, expected %s at line %d",
            got, exp, lex->n_lines);

        return 0;
    }

    lex_entities(lex);

    return 1;
}

/* --------------------------------------------------------------------- */


char* entity_get(struct entity_field* entity, char* key)
{
    int i;

    for (i = 0; i < vec_len(entity); ++i)
    {
        if (!strcmp(entity[i].key, key)) {
            return entity[i].value;
        }
    }

    return 0;
}

struct entity_field* entity_by_classname(char* classname)
{
    int i;

    for (i = 0; i < vec_len(entities); ++i)
    {
        char* cur_classname;

        cur_classname = entity_get(entities[i], "classname");

        if (cur_classname && !strcmp(cur_classname, classname)) {
            return entities[i];
        }
    }

    return 0;
}

void parse_entities(char* data)
{
    int i;
    struct entities_lexer lex;

    for (i = 0; i < vec_len(entities); ++i) {
        vec_free(entities[i]);
    }

    vec_clear(entities);
    
    num_entities = 0;

    memset(&lex, 0, sizeof(lex));
    lex.p = data;
    lex_entities(&lex);

    do
    {
        struct entity_field* fields = 0;

        if (!entities_expect(&lex, '{')) {
            return;
        }

        while (lex.token_kind == ENTITIES_STRING)
        {
            struct entity_field field;

            field.key = lex.str;
            lex_entities(&lex);

            if (lex.token_kind != ENTITIES_STRING) {
                return;
            }

            field.value = lex.str;
            vec_append(fields, field);

            lex_entities(&lex);
        }

        vec_append(entities, fields);
        num_entities += 1;

        if (!entities_expect(&lex, '}')) {
            return;
        }
    }
    while (lex.token_kind);
}
