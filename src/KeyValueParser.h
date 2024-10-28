//
//  KeyValueParser.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 28.10.24.
//

#ifndef KeyValueParser_hpp
#define KeyValueParser_hpp

struct entity_field
{
    char* key;
    char* value;
};

struct entity_field** entities;
int num_entities;

void parse_entities(char* data);

struct entity_field* entity_by_classname(char* classname);
char* entity_get(struct entity_field* entity, char* key);

#endif /* KeyValueParser_hpp */
