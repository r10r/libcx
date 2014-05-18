#ifndef CX_RPC_EXAMPLE_SERVICE_H
#define CX_RPC_EXAMPLE_SERVICE_H

#include "rpc_json.h"

int
ExampleService_call(const char* method_name, Param* params, int num_params, Value* result, ParamFormat format);

typedef struct person_t Person;
typedef void F_PersonFree (Person* person);

/* service domain objects + methods */

struct person_t
{
	char* firstname;
	char* lastname;
	int age;
	F_PersonFree* f_free;
};

json_t*
Person_to_json(Person* person);

void
Person_from_json(Person* person, json_t* json);

extern MethodMap EXAMPLE_SERVICE_METHODS[];

#endif
