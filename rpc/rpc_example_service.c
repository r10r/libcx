#include "rpc_example_service.h"
#include <jansson.h>    // FIXME service should not depend on jansson directly

static void
Person_free(Person* person)
{
	cx_free(person->firstname);
	cx_free(person->lastname);
	cx_free(person);
}

static Person*
get_person()
{
	Person* person = cx_alloc(sizeof(Person));

	person->firstname = cx_strdup("Max");
	person->lastname = cx_strdup("Mustermann");
	person->age = 33;
	person->f_free = &Person_free;
	return person;
}

static char*
print_person(Person* person)
{
	char buf[1024];

	snprintf(buf, sizeof(buf), "%s %s (age %d)", person->firstname, person->lastname, person->age);
	return cx_strdup(buf);
}

static bool
has_count(const char* string, size_t count)
{
	bool result = false;

	if (string)
	{
		printf("string:%s count:%zu\n", string, count);
		result = strlen(string) == count;
	}
	else
	{
		set_cx_errno(ERROR_PARAM_NULL);
	}

	return result;
}

static int
call_has_count(Param* params, int num_params, Value* result, ParamFormat format)
{
	UNUSED(format); // FIXME

	/* typesafe parameter deserialization with error checking */
	Value* p_string = Param_get(params, 0, "string", num_params);

	if (p_string == NULL)
	{
		set_cx_errno(ERROR_PARAM_MISSING);
		return -1;
	}

	if (p_string->type != TYPE_STRING)
	{
		set_cx_errno(ERROR_PARAM_TYPE);
		return -1;
	}

	Value* p_count = Param_get(params, 1, "count", num_params);

	if (p_count == NULL)
	{
		set_cx_errno(ERROR_PARAM_MISSING);
		return -1;
	}

	if (p_count->type != TYPE_INTEGER)
	{
		set_cx_errno(ERROR_PARAM_TYPE);
		return -1;
	}

	/* call method */
	bool out = has_count(p_string->value.string, (size_t)p_count->value.integer);

	/* prepare result */
	result->type = TYPE_BOOLEAN;
	result->value.boolean = out;

	return 1;
}

static int
call_get_person(Param* params, int num_params, Value* result, ParamFormat format)
{
	UNUSED(params);
	UNUSED(format); // FIXME
	UNUSED(num_params);

	Person* person = get_person();

	result->type = TYPE_OBJECT;
	result->value.object = person;
	result->f_free = (F_ValueFree*)&Person_free;

	return 1;
}

static void
simple_free(void* object)
{
	cx_free(object);
}

static int
call_print_person(Param* params, int num_params, Value* result, ParamFormat format)
{
	UNUSED(result);
	UNUSED(format); // FIXME

	/* typesafe parameter deserialization with error checking */
	Value* p_person = Param_get(params, 0, "person", num_params);

	if (p_person == NULL)
	{
		set_cx_errno(ERROR_PARAM_MISSING);
		return -1;
	}

	if (p_person->type != TYPE_OBJECT)
	{
		set_cx_errno(ERROR_PARAM_TYPE);
		return -1;
	}

	char* person_s = NULL;
	switch (format)
	{
	case FORMAT_JSON:
	{
		Person person;
		memset(&person, 0, sizeof(Person));
		Person_from_json(&person, (json_t*)p_person->value.object);
		person_s = print_person(&person);
		break;
	}
	case FORMAT_NATIVE:
	{
		Person* person = (Person*)p_person->value.object;
		person_s = print_person(person);
		break;
	}
//	default:
//		set_cx_errno(ERROR_PARAM_FORMAT);
//		return -1;
	}

	result->type = TYPE_STRING;
	result->value.string = person_s;
	result->f_free = &simple_free;

	return 0;
}

typedef int RPC_Method (Param* params, int num_params, Value* result, ParamFormat format);
typedef struct cx_rpc_method_map_t
{
	const char* name;
	RPC_Method* method;
} MethodMap;

static MethodMap METHOD_MAP[] =
{
	{ "has_count", call_has_count },
	{ "get_person", call_get_person },
	{ "print_person", call_print_person },
	{ NULL, NULL }
};

/* @return
 *      -1 on error see cx_errno
 *      0 method has void return,
 *      1 if return values is valid
 */
int
ExampleService_call(const char* method_name, Param* params, int num_params, Value* result, ParamFormat format)
{
	set_cx_errno(0); /* clear previous errors */
	int status = 1;

	MethodMap* method_map = METHOD_MAP;
	bool method_missing = true;

	while (method_map->name)
	{
		if (strcmp(method_map->name, method_name) == 0)
		{
			XFDBG("Calling method[%s] (wrapper:%p, params:%d, format:%d)",
			      method_name, (void*)method_map->method, num_params, format);
			status = method_map->method(params, num_params, result, format);
			method_missing = false;
			break;
		}
		method_map++;
	}

	if (method_missing)
		set_cx_errno(ERROR_METHOD_MISSING);

	/* free allocated param values */
	int i;
	Param* param = params;
	for (i = 0; i < num_params; i++)
	{
		if (param->value.f_free)
			param->value.f_free(param->value.value.object);

		param++;
	}

	if (cx_errno == 0)
		return status;
	else
	{
		XFERR("ERROR calling method '%s' (cx_errno %d)", method_name, cx_errno);
		return -1;
	}
}

json_t*
Person_to_json(Person* person)
{
	json_t* json = json_object();

	json_object_set_new(json, "firstname", json_string(person->firstname));
	json_object_set_new(json, "lastname", json_string(person->lastname));
	// FIXME requires same integer deserialization as simple params
	json_object_set_new(json, "age", json_integer(person->age));
	return json;
}

void
Person_from_json(Person* person, json_t* json)
{
	person->firstname = json_string_value(json_object_get(json, "firstname"));
	person->lastname = json_string_value(json_object_get(json, "lastname"));
	person->age = (int)json_integer_value(json_object_get(json, "age"));
}
